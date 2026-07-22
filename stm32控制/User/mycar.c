#include "stm32f10x.h"
#include "mycar.h"

#define CAR_PWM_PERIOD       999
#define CAR_DEFAULT_SPEED    40

static unsigned char car_speed_percent = CAR_DEFAULT_SPEED;
static unsigned char car_left_speed_percent = CAR_DEFAULT_SPEED;
static unsigned char car_right_speed_percent = CAR_DEFAULT_SPEED;
static unsigned char car_motion_active = 0;

unsigned char car_get_default_speed(void)
{
    return CAR_DEFAULT_SPEED;
}

/*
 * Motor pin mapping (full H-bridge with direction control):
 *
 *   PB8  = 右电机使能 (ENB / TIM4_CH3 PWM)
 *   PB9  = 左电机使能 (ENA / TIM4_CH4 PWM)
 *   PB12 = 右电机 IN3 (方向 A)
 *   PB13 = 右电机 IN4 (方向 B)
 *   PB14 = 左电机 IN1 (方向 A)
 *   PB15 = 左电机 IN2 (方向 B)
 *
 *   L298N 驱动逻辑:
 *     PB14=0, PB15=1 → 左电机前进
 *     PB14=1, PB15=0 → 左电机后退
 *     PB12=0, PB13=1 → 右电机前进
 *     PB12=1, PB13=0 → 右电机后退
 */

/**
 * @brief  Initialize motor driver pins.
 *         PB8,PB9 = enable PWM (TIM4_CH3/CH4)
 *         PB12-PB15 = H-bridge direction control (push-pull)
  */
void mycar_init(void)
{
    GPIO_InitTypeDef GPIO_Structure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* Enable pins PB8, PB9: TIM4_CH3/CH4 PWM */
    GPIO_Structure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Structure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Structure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_Structure);

    /* Direction pins PB12-PB15 */
    GPIO_Structure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13
                            | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Structure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Structure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_Structure);

    /* 72 MHz / 72 / 1000 = 1 kHz PWM. */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = CAR_PWM_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM4, &TIM_OCInitStructure);
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);

    car_set_speed(CAR_DEFAULT_SPEED);
    stop();
}

static void car_apply_pwm(void)
{
    uint16_t left_pulse = 0;
    uint16_t right_pulse = 0;

    if (car_motion_active)
    {
        if (car_left_speed_percent > 0)
        {
            right_pulse = (uint16_t)(((uint32_t)(CAR_PWM_PERIOD + 1) * car_left_speed_percent) / 100);
            if (right_pulse > CAR_PWM_PERIOD)
            {
                right_pulse = CAR_PWM_PERIOD;
            }
        }
        if (car_right_speed_percent > 0)
        {
            left_pulse = (uint16_t)(((uint32_t)(CAR_PWM_PERIOD + 1) * car_right_speed_percent) / 100);
            if (left_pulse > CAR_PWM_PERIOD)
            {
                left_pulse = CAR_PWM_PERIOD;
            }
        }
    }

    TIM_SetCompare3(TIM4, left_pulse);
    TIM_SetCompare4(TIM4, right_pulse);
}

void car_set_speed(unsigned char speed_percent)
{
    if (speed_percent > 100) {
        speed_percent = 100;
    }
    car_speed_percent = speed_percent;
    car_left_speed_percent = speed_percent;
    car_right_speed_percent = speed_percent;
    car_apply_pwm();
}

void car_set_speed_lr(unsigned char left_speed_percent, unsigned char right_speed_percent)
{
    if (left_speed_percent > 100) {
        left_speed_percent = 100;
    }
    if (right_speed_percent > 100) {
        right_speed_percent = 100;
    }
    car_left_speed_percent = left_speed_percent;
    car_right_speed_percent = right_speed_percent;
    car_apply_pwm();
}

unsigned char car_get_speed(void)
{
    return car_speed_percent;
}

/**
  * @brief  Move forward (both motors forward).
  */
void go(void)
{
    car_motion_active = 0;
    car_apply_pwm();
    GPIO_ResetBits(GPIOB, GPIO_Pin_14);     /* left IN1=0 */
    GPIO_SetBits(GPIOB, GPIO_Pin_15);       /* left IN2=1 → left forward */
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);     /* right IN3=0 */
    GPIO_SetBits(GPIOB, GPIO_Pin_13);       /* right IN4=1 → right forward */
    car_motion_active = 1;
    car_apply_pwm();
}

/**
  * @brief  Stop all motors (disable + all IN low).
  */
void stop(void)
{
    car_motion_active = 0;
    car_apply_pwm();
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    GPIO_ResetBits(GPIOB, GPIO_Pin_13);
    GPIO_ResetBits(GPIOB, GPIO_Pin_14);
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);
}

/**
  * @brief  Move forward (alias for go()).
  */
void car_forward(void)
{
    go();
}

/**
  * @brief  Move backward (both motors reverse).
  *         Now functional with H-bridge direction pins.
  */
void car_backward(void)
{
    car_motion_active = 0;
    car_apply_pwm();
    GPIO_SetBits(GPIOB, GPIO_Pin_14);       /* left IN1=1 */
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);     /* left IN2=0 → left backward */
    GPIO_SetBits(GPIOB, GPIO_Pin_12);       /* right IN3=1 */
    GPIO_ResetBits(GPIOB, GPIO_Pin_13);     /* right IN4=0 → right backward */
    car_motion_active = 1;
    car_apply_pwm();
}

/**
  * @brief  Pivot left (left backward, right forward).
  *         Differential / tank turn in place.
  */
void car_left(void)
{
    car_motion_active = 0;
    car_apply_pwm();
    GPIO_SetBits(GPIOB, GPIO_Pin_14);       /* left IN1=1 */
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);     /* left IN2=0 → left backward */
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);     /* right IN3=0 */
    GPIO_SetBits(GPIOB, GPIO_Pin_13);       /* right IN4=1 → right forward */
    car_motion_active = 1;
    car_apply_pwm();
}

/**
  * @brief  Pivot right (left forward, right backward).
  *         Differential / tank turn in place.
  */
void car_right(void)
{
    car_motion_active = 0;
    car_apply_pwm();
    GPIO_ResetBits(GPIOB, GPIO_Pin_14);     /* left IN1=0 */
    GPIO_SetBits(GPIOB, GPIO_Pin_15);       /* left IN2=1 → left forward */
    GPIO_SetBits(GPIOB, GPIO_Pin_12);       /* right IN3=1 */
    GPIO_ResetBits(GPIOB, GPIO_Pin_13);     /* right IN4=0 → right backward */
    car_motion_active = 1;
    car_apply_pwm();
}
