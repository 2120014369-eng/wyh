/**
  ******************************************************************************
  * @file    main.c
  * @brief   Bluetooth-controlled motor drive for STM32F103C8.
  *
   *          6-pin L298N full H-bridge: PB8/PB9 = enable, PB12-PB15 = IN1-IN4.
   *
  *          USART3 (115200-8N1, Bluetooth on PB10/PB11) commands:
   *            - "go"        : Forward  (both motors, full H-bridge)
   *            - "stop"/"S"  : Stop     (all 6 pins low)
   *            - "F"         : Forward  (same as "go")
   *            - "B"         : Backward (H-bridge reverse, both motors)
   *            - "L"         : Pivot left (differential tank turn)
   *            - "R"         : Pivot right (differential tank turn)
   *            - "V0".."V100": Set motor PWM speed percent
   *
  *          HC-SR04 ultrasonic:
  *            - PA11    : TRIG output
  *            - PA12    : ECHO input (must be level-shifted / resistor-divided to 3.3V)
  *
  *          Bluetooth runtime wiring:
  *            - PB10    : USART3 TX -> Bluetooth RX
  *            - PB11    : USART3 RX <- Bluetooth TX
  *
  *          USART1 PA9/PA10 is left free for STM32 ISP flashing.
  *
  *          PC13 LED: fast blink (3 flashes) on BT command received,
  *          slow blink when motor running, solid on when stopped.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "myusart.h"
#include "mycar.h"
#include "track.h"
#include "ultrasonic.h"
#include "string.h"

/* 1ms tick from SysTick_Handler (stm32f10x_it.c) */
extern volatile uint32_t system_ms;

/* Private define ------------------------------------------------------------*/
#define LED_PORT    GPIOC
#define LED_PIN     GPIO_Pin_13
#define BT_LINK_DIAGNOSTIC  0
#define OBSTACLE_STOP_DISTANCE_MM    180U
#define OBSTACLE_CLEAR_DISTANCE_MM   230U
#define ULTRASONIC_CHECK_INTERVAL_MS 120U
#define DISTANCE_REPORT_DELTA_MM     10U
#define OBSTACLE_CONFIRM_COUNT       3U
#define OBSTACLE_CLEAR_COUNT         2U

typedef enum
{
    CAR_MODE_MANUAL = 0,
    CAR_MODE_TRACKING,
    CAR_MODE_OBSTACLE_STOP,
    CAR_MODE_WAIT_USER_DECISION
} CarMode;

/* Private function prototypes -----------------------------------------------*/
static void LED_Init(void);
static void motor_hw_init(void);
static void motor_go(void);
static void motor_stop(void);
static void motor_forward(void);
static void motor_backward(void);
static void motor_left(void);
static void motor_right(void);
static void motor_set_speed(unsigned char speed);
static int parse_speed_command(const char *cmd, unsigned char *speed);
static int append_uint(char *buffer, int pos, uint32_t value);
static void send_speed_ok_status(unsigned char speed);
static int append_text(char *buffer, int pos, const char *text);
static void send_boot_status(void);
static void watchdog_init(void);
static void run_direction_command(char cmd, uint32_t *motor_running);
static int parse_track_command(const char *cmd);
static void restore_manual_speed_profile(void);
static int parse_user_continue_command(const char *cmd);
static int parse_user_snapshot_command(const char *cmd);
static int parse_status_command(const char *cmd);
static void send_control_string(const char *text);
static void send_obstacle_status(uint32_t distance_mm);
static void send_distance_status(uint32_t distance_mm);
static void send_track_status(uint8_t left, uint8_t mid, uint8_t right);
static void send_speed_diag_status(unsigned char speed);
static void send_bt_rx_overflow_status(void);
static void send_mode_status(CarMode mode);
static uint32_t abs_diff_u32(uint32_t a, uint32_t b);

/* Private functions ---------------------------------------------------------*/

static void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);

    /* PC13 LED is active low, default off */
    GPIO_SetBits(LED_PORT, LED_PIN);
}

#if BT_LINK_DIAGNOSTIC
static void motor_hw_init(void) {}
static void motor_go(void) {}
static void motor_stop(void) {}
static void motor_forward(void) {}
static void motor_backward(void) {}
static void motor_left(void) {}
static void motor_right(void) {}
static void motor_set_speed(unsigned char speed) { (void)speed; }
#else
static void motor_hw_init(void) { mycar_init(); }
static void motor_go(void) { go(); }
static void motor_stop(void) { stop(); }
static void motor_forward(void) { car_forward(); }
static void motor_backward(void) { car_backward(); }
static void motor_left(void) { car_left(); }
static void motor_right(void) { car_right(); }
static void motor_set_speed(unsigned char speed) { car_set_speed(speed); }
#endif

static int parse_speed_command(const char *cmd, unsigned char *speed)
{
    int i = 1;
    int value = 0;

    if (cmd[0] != 'V' && cmd[0] != 'v')
        return 0;
    if (cmd[1] == '\0')
        return -1;

    while (cmd[i] != '\0')
    {
        if (cmd[i] < '0' || cmd[i] > '9')
            return -1;
        value = value * 10 + (cmd[i] - '0');
        if (value > 100)
            return -1;
        i++;
    }

    *speed = (unsigned char)value;
    return 1;
}

static int parse_track_command(const char *cmd)
{
    if (strcmp(cmd, "track") == 0 || strcmp(cmd, "TRACK") == 0 ||
        strcmp(cmd, "track_on") == 0 || strcmp(cmd, "TRACK_ON") == 0)
    {
        return 1;
    }

    if (strcmp(cmd, "track_off") == 0 || strcmp(cmd, "TRACK_OFF") == 0)
    {
        return 0;
    }

    return -1;
}

static void restore_manual_speed_profile(void)
{
    unsigned char speed = car_get_speed();
    if (speed == 0U)
    {
        speed = car_get_default_speed();
    }
    motor_set_speed(speed);
}

static int parse_user_continue_command(const char *cmd)
{
    if (strcmp(cmd, "continue") == 0 || strcmp(cmd, "CONTINUE") == 0 ||
        strcmp(cmd, "resume") == 0 || strcmp(cmd, "RESUME") == 0)
    {
        return 1;
    }

    if (strcmp(cmd, "hold") == 0 || strcmp(cmd, "HOLD") == 0 ||
        strcmp(cmd, "wait") == 0 || strcmp(cmd, "WAIT") == 0)
    {
        return 0;
    }

    return -1;
}

static int parse_user_snapshot_command(const char *cmd)
{
    if (strcmp(cmd, "snap") == 0 || strcmp(cmd, "SNAP") == 0 ||
        strcmp(cmd, "photo") == 0 || strcmp(cmd, "PHOTO") == 0)
    {
        return 1;
    }

    if (strcmp(cmd, "kping") == 0 || strcmp(cmd, "KPING") == 0 ||
        strcmp(cmd, "pingk210") == 0 || strcmp(cmd, "PINGK210") == 0)
    {
        return 2;
    }

    return 0;
}

static int parse_status_command(const char *cmd)
{
    if (strcmp(cmd, "status") == 0 || strcmp(cmd, "STATUS") == 0 ||
        strcmp(cmd, "diag") == 0 || strcmp(cmd, "DIAG") == 0)
    {
        return 1;
    }

    return 0;
}

static void send_control_string(const char *text)
{
    int len = (int)strlen(text);
    my_send_string(text, len);
}

static void send_obstacle_status(uint32_t distance_mm)
{
    char msg[48];
    int pos = 0;

    pos = append_text(msg, pos, "OBS:");
    pos = append_uint(msg, pos, distance_mm);
    msg[pos++] = 'm';
    msg[pos++] = 'm';
    msg[pos++] = '\r';
    msg[pos++] = '\n';

    my_send_string(msg, pos);
}

static void send_distance_status(uint32_t distance_mm)
{
    char msg[48];
    int pos = 0;

    pos = append_text(msg, pos, "DIST:");
    pos = append_uint(msg, pos, distance_mm);
    msg[pos++] = 'm';
    msg[pos++] = 'm';
    msg[pos++] = '\r';
    msg[pos++] = '\n';

    my_send_string(msg, pos);
}

static void send_track_status(uint8_t left, uint8_t mid, uint8_t right)
{
    char msg[20];
    int pos = 0;

    pos = append_text(msg, pos, "TRACK:");
    msg[pos++] = (char)('0' + (left ? 1 : 0));
    msg[pos++] = ',';
    msg[pos++] = (char)('0' + (mid ? 1 : 0));
    msg[pos++] = ',';
    msg[pos++] = (char)('0' + (right ? 1 : 0));
    msg[pos++] = '\r';
    msg[pos++] = '\n';

    my_send_string(msg, pos);
}

static void send_speed_diag_status(unsigned char speed)
{
    char msg[20];
    int pos = 0;

    pos = append_text(msg, pos, "SPEED:");
    pos = append_uint(msg, pos, speed);
    msg[pos++] = '\r';
    msg[pos++] = '\n';

    my_send_string(msg, pos);
}

static void send_bt_rx_overflow_status(void)
{
    char msg[32];
    int pos = 0;

    pos = append_text(msg, pos, "BT_RX_OVF:");
    pos = append_uint(msg, pos, my_usart_rx_overflow_count());
    msg[pos++] = '\r';
    msg[pos++] = '\n';

    my_send_string(msg, pos);
}

static void send_mode_status(CarMode mode)
{
    switch (mode)
    {
    case CAR_MODE_MANUAL:
        send_control_string("MODE:MANUAL\r\n");
        break;
    case CAR_MODE_TRACKING:
        send_control_string("MODE:TRACKING\r\n");
        break;
    case CAR_MODE_OBSTACLE_STOP:
        send_control_string("MODE:OBSTACLE_STOP\r\n");
        break;
    case CAR_MODE_WAIT_USER_DECISION:
        send_control_string("MODE:WAIT_USER\r\n");
        break;
    default:
        break;
    }
}

static int append_uint(char *buffer, int pos, uint32_t value)
{
    char digits[10];
    int len = 0;

    if (value == 0U) {
        buffer[pos++] = '0';
        return pos;
    }

    while (value > 0U) {
        digits[len++] = (char)('0' + (value % 10U));
        value /= 10U;
    }

    while (len > 0) {
        buffer[pos++] = digits[--len];
    }
    return pos;
}

static int append_text(char *buffer, int pos, const char *text)
{
    while (*text != '\0')
    {
        buffer[pos++] = *text++;
    }
    return pos;
}

static void send_boot_status(void)
{
    char msg[80];
    int pos = 0;

    pos = append_text(msg, pos, "MCU:BOOT");
    if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) {
        pos = append_text(msg, pos, ":PIN");
    }
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) {
        pos = append_text(msg, pos, ":POR");
    }
    if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) != RESET) {
        pos = append_text(msg, pos, ":SFT");
    }
    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET) {
        pos = append_text(msg, pos, ":IWDG");
    }
    if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET) {
        pos = append_text(msg, pos, ":WWDG");
    }
    if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST) != RESET) {
        pos = append_text(msg, pos, ":LPWR");
    }
    msg[pos++] = '\r';
    msg[pos++] = '\n';

    my_send_string(msg, pos);
    RCC_ClearFlag();
}

static void watchdog_init(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload(0x0FFF);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

static uint32_t abs_diff_u32(uint32_t a, uint32_t b)
{
    return (a >= b) ? (a - b) : (b - a);
}

static void send_speed_ok_status(unsigned char speed)
{
    char msg[12];
    int pos = 0;
    msg[pos++] = 'O';
    msg[pos++] = 'K';
    msg[pos++] = ':';
    msg[pos++] = ' ';
    msg[pos++] = 'V';
    pos = append_uint(msg, pos, speed);
    msg[pos++] = '\r';
    msg[pos++] = '\n';
    my_send_string(msg, pos);
}

static void run_direction_command(char cmd, uint32_t *motor_running)
{
    if (cmd == 'F')
    {
        restore_manual_speed_profile();
        motor_forward();
        *motor_running = 1;
        my_send_string("OK: F\r\n", 7);
    }
    else if (cmd == 'B')
    {
        restore_manual_speed_profile();
        motor_backward();
        *motor_running = 1;
        my_send_string("OK: B\r\n", 7);
    }
    else if (cmd == 'L')
    {
        restore_manual_speed_profile();
        motor_left();
        *motor_running = 1;
        my_send_string("OK: L\r\n", 7);
    }
    else if (cmd == 'R')
    {
        restore_manual_speed_profile();
        motor_right();
        *motor_running = 1;
        my_send_string("OK: R\r\n", 7);
    }
    else if (cmd == 'S')
    {
        motor_stop();
        *motor_running = 0;
        GPIO_ResetBits(LED_PORT, LED_PIN);
        my_send_string("OK: S\r\n", 7);
    }
}

int main(void)
{
    /* Bluetooth command buffer */
    char cmd[32];
    char ch = 0;
    int cmd_len = 0;
    int overflow = 0;

    uint32_t last_led_toggle = 0;   /* last LED toggle timestamp (ms) */
    uint32_t led_state = 0;         /* current LED on/off */
    uint32_t motor_running = 0;     /* 1 = motor running */
    uint32_t bt_flash = 0;          /* remaining half-cycles for BT LED flash */
    uint32_t bt_flash_on = 0;       /* current flash phase: 1=on, 0=off */
    uint32_t track_mode = 0;
    uint32_t last_ultrasonic_check = 0;
    uint32_t obstacle_confirm_count = 0;
    uint32_t obstacle_clear_count = 0;
    uint32_t obstacle_distance_mm = 0;
    uint32_t last_reported_distance_mm = 0xFFFFFFFFU;
    uint8_t last_reported_track_left = 0xFFU;
    uint8_t last_reported_track_mid = 0xFFU;
    uint8_t last_reported_track_right = 0xFFU;
    CarMode car_mode = CAR_MODE_MANUAL;

    /* ISP burn compatibility delay: at least 500000 idle cycles before
       any serial/watchdog init, so FlyMcu can complete its handshake
       when using the on-board USB (CH340E) for ISP programming. */
    {
        volatile uint32_t isp_delay;
        for (isp_delay = 0U; isp_delay < 500000U; isp_delay++)
        {
            __NOP();
        }
    }

    watchdog_init();

    /* Use real pre-emption priority bits for interrupt-driven Bluetooth RX. */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* Initialize motor pins PB8, PB9 */
    motor_hw_init();

    /* Initialize LED indicator */
    LED_Init();

    /* Initialize HC-SR04 and tracking sensors. K210 is connected to the PC over USB. */
    ultrasonic_init();
    track_init();

    /* Initialize USART3 for Bluetooth commands on PB10/PB11. */
    usart_init();

    /* Configure SysTick: 1ms interrupt */
    SysTick_Config(SystemCoreClock / 1000);

    send_boot_status();
    send_mode_status(car_mode);
    send_speed_diag_status(car_get_speed());

    /* Motors start stopped, waiting for commands */
    motor_stop();
    GPIO_ResetBits(LED_PORT, LED_PIN);  /* LED solid on when stopped */

    while (1)
    {
        IWDG_ReloadCounter();

        if (track_mode && car_mode == CAR_MODE_TRACKING)
        {
            motor_running = track_run_step();
            if (track_should_hold())
            {
                track_mode = 0;
                car_mode = CAR_MODE_WAIT_USER_DECISION;
                motor_stop();
                motor_running = 0;
                track_clear_hold();
                send_mode_status(car_mode);
            }
        }

        if ((car_mode == CAR_MODE_TRACKING || car_mode == CAR_MODE_OBSTACLE_STOP) &&
            (system_ms - last_ultrasonic_check) >= ULTRASONIC_CHECK_INTERVAL_MS)
        {
            uint32_t distance_mm;
            uint8_t left_state;
            uint8_t mid_state;
            uint8_t right_state;
            last_ultrasonic_check = system_ms;
            distance_mm = ultrasonic_get_distance_mm();
            left_state = track_read_left();
            mid_state = track_read_mid();
            right_state = track_read_right();
            if (last_reported_distance_mm == 0xFFFFFFFFU ||
                distance_mm == 0U ||
                last_reported_distance_mm == 0U ||
                abs_diff_u32(distance_mm, last_reported_distance_mm) >= DISTANCE_REPORT_DELTA_MM)
            {
                send_distance_status(distance_mm);
                last_reported_distance_mm = distance_mm;
            }
            if (left_state != last_reported_track_left ||
                mid_state != last_reported_track_mid ||
                right_state != last_reported_track_right)
            {
                send_track_status(left_state, mid_state, right_state);
                last_reported_track_left = left_state;
                last_reported_track_mid = mid_state;
                last_reported_track_right = right_state;
            }
            if (car_mode == CAR_MODE_OBSTACLE_STOP)
            {
                motor_stop();
                motor_running = 0;
                if (distance_mm == 0U || distance_mm >= OBSTACLE_CLEAR_DISTANCE_MM)
                {
                    obstacle_clear_count++;
                    if (obstacle_clear_count >= OBSTACLE_CLEAR_COUNT)
                    {
                        track_clear_hold();
                        track_mode = 1;
                        car_mode = CAR_MODE_TRACKING;
                        obstacle_confirm_count = 0;
                        obstacle_clear_count = 0;
                        send_mode_status(car_mode);
                        my_send_string("OK: obstacle clear\r\n", 20);
                    }
                }
                else
                {
                    obstacle_clear_count = 0;
                }
            }
            else if (distance_mm > 0U && distance_mm <= OBSTACLE_STOP_DISTANCE_MM)
            {
                obstacle_confirm_count++;
                obstacle_distance_mm = distance_mm;
                if (obstacle_confirm_count >= OBSTACLE_CONFIRM_COUNT)
                {
                    track_mode = 0;
                    motor_stop();
                    motor_running = 0;
                    car_mode = CAR_MODE_OBSTACLE_STOP;
                    send_mode_status(car_mode);
                    send_obstacle_status(obstacle_distance_mm);
                    obstacle_confirm_count = 0;
                    obstacle_clear_count = 0;
                }
            }
            else
            {
                obstacle_confirm_count = 0;
                obstacle_clear_count = 0;
            }
        }
        else if (car_mode != CAR_MODE_TRACKING)
        {
            obstacle_confirm_count = 0;
            if (car_mode != CAR_MODE_OBSTACLE_STOP)
            {
                obstacle_clear_count = 0;
            }
        }

        /* ================================================================
         * USART3 (Bluetooth serial, 115200) command receive.
         * RX bytes are buffered by USART3_IRQHandler.
         * Commands: "go", "stop", F/B/L/R/S, V0..V100
         * ================================================================ */
        while (my_usart_read_byte(&ch) > 0)
        {
            if ((ch == 'F' || ch == 'B' || ch == 'L' || ch == 'R' || ch == 'S') && cmd_len == 0)
            {
                track_mode = 0;
                car_mode = CAR_MODE_MANUAL;
                send_mode_status(car_mode);
                run_direction_command(ch, &motor_running);
                if (bt_flash == 0U)
                {
                    bt_flash = 6U;
                }
            }
            else if (ch == '\n' || ch == '\r')
            {
                cmd[cmd_len] = '\0';

                if (overflow)
                {
                    my_send_string("Error: command too long\r\n", 25);
                }
                else if (cmd_len > 0)
                {
                     unsigned char speed = 0;
                     int speed_cmd = parse_speed_command(cmd, &speed);
                     int track_cmd = parse_track_command(cmd);
                     int continue_cmd = parse_user_continue_command(cmd);
                     int snap_cmd = parse_user_snapshot_command(cmd);
                     int status_cmd = parse_status_command(cmd);

                     if (track_cmd == 1)
                     {
                          track_clear_hold();
                          track_mode = 1;
                          car_mode = CAR_MODE_TRACKING;
                          obstacle_confirm_count = 0;
                          obstacle_clear_count = 0;
                          send_mode_status(car_mode);
                          my_send_string("OK: track\r\n", 11);
                     }
                     else if (track_cmd == 0)
                     {
                          track_mode = 0;
                          car_mode = CAR_MODE_MANUAL;
                          send_mode_status(car_mode);
                          motor_stop();
                          motor_running = 0;
                          GPIO_ResetBits(LED_PORT, LED_PIN);
                         my_send_string("OK: track off\r\n", 15);
                     }
                     else if (strcmp(cmd, "go") == 0)
                     {
                          track_mode = 0;
                          car_mode = CAR_MODE_MANUAL;
                          send_mode_status(car_mode);
                          restore_manual_speed_profile();
                          motor_go();
                         motor_running = 1;
                         my_send_string("OK: go\r\n", 8);
                     }
                     else if (strcmp(cmd, "stop") == 0)
                     {
                          track_mode = 0;
                          car_mode = CAR_MODE_MANUAL;
                          send_mode_status(car_mode);
                          motor_stop();
                         motor_running = 0;
                         GPIO_ResetBits(LED_PORT, LED_PIN);
                         my_send_string("OK: stop\r\n", 10);
                     }
                     /* Directional commands over Bluetooth USART3. */
                     else if (strcmp(cmd, "F") == 0)
                     {
                          track_mode = 0;
                          car_mode = CAR_MODE_MANUAL;
                          send_mode_status(car_mode);
                          run_direction_command('F', &motor_running);
                     }
                     else if (strcmp(cmd, "B") == 0)
                     {
                          track_mode = 0;
                          car_mode = CAR_MODE_MANUAL;
                          send_mode_status(car_mode);
                          run_direction_command('B', &motor_running);
                     }
                     else if (strcmp(cmd, "L") == 0)
                     {
                          track_mode = 0;
                          car_mode = CAR_MODE_MANUAL;
                          send_mode_status(car_mode);
                          run_direction_command('L', &motor_running);
                     }
                     else if (strcmp(cmd, "R") == 0)
                     {
                          track_mode = 0;
                          car_mode = CAR_MODE_MANUAL;
                          send_mode_status(car_mode);
                          run_direction_command('R', &motor_running);
                     }
                     else if (strcmp(cmd, "S") == 0)
                     {
                          track_mode = 0;
                          car_mode = CAR_MODE_MANUAL;
                          send_mode_status(car_mode);
                          run_direction_command('S', &motor_running);
                     }
                     else if (speed_cmd == 1)
                     {
                         track_mode = 0;
                         car_mode = CAR_MODE_MANUAL;
                         send_mode_status(car_mode);
                          motor_set_speed(speed);
                          send_speed_diag_status(speed);
                          send_speed_ok_status(speed);
                      }
                     else if (continue_cmd == 1)
                     {
                          if (car_mode == CAR_MODE_WAIT_USER_DECISION)
                          {
                              track_clear_hold();
                              track_mode = 1;
                              car_mode = CAR_MODE_TRACKING;
                              obstacle_confirm_count = 0;
                              obstacle_clear_count = 0;
                             send_mode_status(car_mode);
                             my_send_string("OK: continue\r\n", 14);
                         }
                         else
                         {
                             my_send_string("Error: no wait state\r\n", 22);
                         }
                     }
                     else if (continue_cmd == 0)
                     {
                         track_mode = 0;
                         car_mode = CAR_MODE_WAIT_USER_DECISION;
                         send_mode_status(car_mode);
                         motor_stop();
                         motor_running = 0;
                         my_send_string("OK: hold\r\n", 10);
                     }
                     else if (snap_cmd == 1)
                     {
                         my_send_string("Error: K210 uses USB\r\n", 22);
                     }
                     else if (snap_cmd == 2)
                     {
                         my_send_string("Error: K210 uses USB\r\n", 22);
                     }
                     else if (status_cmd == 1)
                     {
                         send_mode_status(car_mode);
                         send_speed_diag_status(car_get_speed());
                         send_distance_status(ultrasonic_get_distance_mm());
                         send_track_status(track_read_left(), track_read_mid(), track_read_right());
                         send_bt_rx_overflow_status();
                         my_send_string("OK: status\r\n", 12);
                     }
                     else if (speed_cmd < 0)
                     {
                          my_send_string("Error: speed 0-100\r\n", 20);
                     }
                     else
                     {
                         my_send_string("Error: unknown command\r\n", 24);
                     }
                     if (bt_flash == 0U)
                     {
                         bt_flash = 6U;
                     }
                }
                overflow = 0;
                cmd_len = 0;
            }
            else if (cmd_len < 31)
            {
                cmd[cmd_len++] = ch;
            }
            else
            {
                overflow = 1;
            }
        }

        /* ---- LED control (SysTick-based, non-blocking) ---- */
        /* Fast blink: 100ms per half-cycle. Slow blink: 500ms per half-cycle. */
        if (bt_flash > 0)
        {
            if (system_ms - last_led_toggle >= 100)
            {
                last_led_toggle = system_ms;
                bt_flash_on = !bt_flash_on;
                if (bt_flash_on)
                {
                    GPIO_ResetBits(LED_PORT, LED_PIN);  /* LED on (active low) */
                }
                else
                {
                    GPIO_SetBits(LED_PORT, LED_PIN);    /* LED off */
                    bt_flash--;
                }
            }
        }
        else if (motor_running)
        {
            /* Slow blink when motor running: 500ms toggle */
            if (system_ms - last_led_toggle >= 500)
            {
                last_led_toggle = system_ms;
                led_state = !led_state;
                if (led_state)
                    GPIO_ResetBits(LED_PORT, LED_PIN);
                else
                    GPIO_SetBits(LED_PORT, LED_PIN);
            }
        }
        else
        {
            GPIO_ResetBits(LED_PORT, LED_PIN);  /* solid on when stopped */
        }
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1)
    {
    }
}
#endif
