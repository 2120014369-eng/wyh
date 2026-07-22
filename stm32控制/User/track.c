#include "track.h"
#include "mycar.h"

#define TRACK_LEFT_PORT   GPIOA
#define TRACK_LEFT_PIN    GPIO_Pin_4
#define TRACK_MID_PORT    GPIOA
#define TRACK_MID_PIN     GPIO_Pin_5
#define TRACK_RIGHT_PORT  GPIOB
#define TRACK_RIGHT_PIN   GPIO_Pin_1

#define TRACK_ACTIVE_LEVEL         1U
#define TRACK_SWAP_LEFT_RIGHT      1U
#define TRACK_DEBOUNCE_SAMPLES     3U
#define TRACK_HOLD_CONFIRM_MS      100U  /* 111 pattern must persist this long before hold */
#define TRACK_LIGHT_TURN_PERCENT   60U
#define TRACK_HARD_TURN_PERCENT    25U

extern volatile uint32_t system_ms;

typedef enum
{
    TRACK_DIR_STRAIGHT = 0,
    TRACK_DIR_LEFT,
    TRACK_DIR_RIGHT
} TrackDirection;

static uint8_t track_stable_pattern = 0U;
static uint8_t track_pending_pattern = 0U;
static uint8_t track_pending_count = 0U;
static TrackDirection track_last_direction = TRACK_DIR_STRAIGHT;
static uint8_t track_hold_requested = 0U;
static uint32_t track_hold_start_ms = 0U;
static unsigned char track_scale_speed(unsigned char base_speed, unsigned char percent)
{
    unsigned int scaled;

    if (base_speed == 0U)
    {
        return 0U;
    }

    scaled = ((unsigned int)base_speed * (unsigned int)percent) / 100U;
    if (scaled == 0U)
    {
        scaled = 1U;
    }

    return (unsigned char)scaled;
}

static uint8_t track_normalize_level(uint8_t raw_level)
{
    return (raw_level == TRACK_ACTIVE_LEVEL) ? 1U : 0U;
}

static uint8_t track_read_left_raw(void)
{
    return (uint8_t)GPIO_ReadInputDataBit(TRACK_LEFT_PORT, TRACK_LEFT_PIN);
}

static uint8_t track_read_mid_raw(void)
{
    return (uint8_t)GPIO_ReadInputDataBit(TRACK_MID_PORT, TRACK_MID_PIN);
}

static uint8_t track_read_right_raw(void)
{
    return (uint8_t)GPIO_ReadInputDataBit(TRACK_RIGHT_PORT, TRACK_RIGHT_PIN);
}

static uint8_t track_read_left_physical(void)
{
    return track_normalize_level(track_read_left_raw());
}

static uint8_t track_read_mid_physical(void)
{
    return track_normalize_level(track_read_mid_raw());
}

static uint8_t track_read_right_physical(void)
{
    return track_normalize_level(track_read_right_raw());
}

static uint8_t track_pattern_from_inputs(uint8_t left, uint8_t mid, uint8_t right)
{
    return (uint8_t)((left << 2) | (mid << 1) | right);
}

static uint8_t track_read_pattern_raw(void)
{
    uint8_t left = track_read_left_physical();
    uint8_t mid = track_read_mid_physical();
    uint8_t right = track_read_right_physical();

    if (TRACK_SWAP_LEFT_RIGHT != 0U)
    {
        uint8_t temp = left;
        left = right;
        right = temp;
    }

    return track_pattern_from_inputs(left, mid, right);
}

static uint8_t track_get_stable_pattern(void)
{
    uint8_t raw_pattern = track_read_pattern_raw();

    if (raw_pattern != track_pending_pattern)
    {
        track_pending_pattern = raw_pattern;
        track_pending_count = 1U;
    }
    else if (track_pending_count < TRACK_DEBOUNCE_SAMPLES)
    {
        track_pending_count++;
    }

    if (track_pending_count >= TRACK_DEBOUNCE_SAMPLES)
    {
        track_stable_pattern = track_pending_pattern;
    }

    return track_stable_pattern;
}

static void track_drive_forward(unsigned char left_speed,
                                unsigned char right_speed,
                                TrackDirection direction)
{
    car_set_speed_lr(left_speed, right_speed);
    go();
    track_last_direction = direction;
}

static unsigned char track_base_speed(void)
{
    return car_get_speed();
}

void track_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = TRACK_LEFT_PIN | TRACK_MID_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = TRACK_RIGHT_PIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    track_stable_pattern = track_read_pattern_raw();
    track_pending_pattern = track_stable_pattern;
    track_pending_count = TRACK_DEBOUNCE_SAMPLES;
    track_last_direction = TRACK_DIR_STRAIGHT;
    track_hold_requested = 0U;
    track_hold_start_ms = 0U;
}

uint8_t track_read_left(void)
{
    if (TRACK_SWAP_LEFT_RIGHT != 0U)
    {
        return track_read_right_physical();
    }

    return track_read_left_physical();
}

uint8_t track_read_mid(void)
{
    return track_read_mid_physical();
}

uint8_t track_read_right(void)
{
    if (TRACK_SWAP_LEFT_RIGHT != 0U)
    {
        return track_read_left_physical();
    }

    return track_read_right_physical();
}

uint8_t track_should_hold(void)
{
    return track_hold_requested;
}

void track_clear_hold(void)
{
    track_hold_requested = 0U;
    track_hold_start_ms = 0U;
}

uint8_t track_run_step(void)
{
    uint8_t pattern;
    unsigned char base_speed = track_base_speed();
    unsigned char light_turn_speed = track_scale_speed(base_speed, TRACK_LIGHT_TURN_PERCENT);
    unsigned char hard_turn_speed = track_scale_speed(base_speed, TRACK_HARD_TURN_PERCENT);

    if (base_speed == 0U)
    {
        stop();
        return 0U;
    }

    pattern = track_get_stable_pattern();

    switch (pattern)
    {
    case 0x02U: /* 010: centered on line */
    case 0x05U: /* 101: crossing / center gap */
        track_hold_start_ms = 0U;
        track_drive_forward(base_speed, base_speed, TRACK_DIR_STRAIGHT);
        return 1U;

    case 0x07U: /* 111: all three sensors on black, confirm then stop */
        if (track_hold_start_ms == 0U)
        {
            track_hold_start_ms = system_ms;
        }
        else if ((system_ms - track_hold_start_ms) >= TRACK_HOLD_CONFIRM_MS)
        {
            stop();
            track_hold_requested = 1U;
        }
        return 0U;

    case 0x06U: /* 110: line drifting to left */
        track_hold_start_ms = 0U;
        track_drive_forward(light_turn_speed, base_speed, TRACK_DIR_LEFT);
        return 1U;

    case 0x04U: /* 100: strong left correction */
        track_hold_start_ms = 0U;
        track_drive_forward(hard_turn_speed, base_speed, TRACK_DIR_LEFT);
        return 1U;

    case 0x03U: /* 011: line drifting to right */
        track_hold_start_ms = 0U;
        track_drive_forward(base_speed, light_turn_speed, TRACK_DIR_RIGHT);
        return 1U;

    case 0x01U: /* 001: strong right correction */
        track_hold_start_ms = 0U;
        track_drive_forward(base_speed, hard_turn_speed, TRACK_DIR_RIGHT);
        return 1U;

    case 0x00U: /* 000: stop immediately, same as legacy project */
        track_hold_start_ms = 0U;
        stop();
        return 0U;

    default:
        track_hold_start_ms = 0U;
        stop();
        return 0U;
    }
}
