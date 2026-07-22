#include "ultrasonic.h"

extern volatile uint32_t system_ms;

#define ULTRASONIC_TRIG_PORT      GPIOA
#define ULTRASONIC_TRIG_PIN       GPIO_Pin_11
#define ULTRASONIC_ECHO_PORT      GPIOA
#define ULTRASONIC_ECHO_PIN       GPIO_Pin_12

#define ULTRASONIC_TIMEOUT_MS     30U
#define ULTRASONIC_MAX_PULSE_US   25000U

static void ultrasonic_delay_us(uint32_t us)
{
    uint32_t start = SysTick->VAL;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    uint32_t reload = SysTick->LOAD + 1U;
    uint32_t elapsed = 0U;

    while (elapsed < ticks)
    {
        uint32_t now = SysTick->VAL;
        if (start >= now)
        {
            elapsed += start - now;
        }
        else
        {
            elapsed += start + (reload - now);
        }
        start = now;
    }
}

void ultrasonic_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = ULTRASONIC_TRIG_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(ULTRASONIC_TRIG_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN);

    GPIO_InitStructure.GPIO_Pin = ULTRASONIC_ECHO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(ULTRASONIC_ECHO_PORT, &GPIO_InitStructure);
}

uint32_t ultrasonic_get_distance_mm(void)
{
    uint32_t start_ms;
    uint32_t pulse_us = 0U;

    GPIO_ResetBits(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN);
    ultrasonic_delay_us(2U);
    GPIO_SetBits(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN);
    ultrasonic_delay_us(12U);
    GPIO_ResetBits(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN);

    start_ms = system_ms;
    while (GPIO_ReadInputDataBit(ULTRASONIC_ECHO_PORT, ULTRASONIC_ECHO_PIN) == Bit_RESET)
    {
        if ((system_ms - start_ms) >= ULTRASONIC_TIMEOUT_MS)
        {
            return 0U;
        }
    }

    while (GPIO_ReadInputDataBit(ULTRASONIC_ECHO_PORT, ULTRASONIC_ECHO_PIN) == Bit_SET)
    {
        ultrasonic_delay_us(2U);
        pulse_us += 2U;
        if (pulse_us >= ULTRASONIC_MAX_PULSE_US)
        {
            return 0U;
        }
    }

    return pulse_us * 17U / 100U;
}
