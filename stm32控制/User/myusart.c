#include "stm32f10x.h"
#include "myusart.h"

/* 1ms tick from SysTick_Handler (stm32f10x_it.c), for send timeout */
extern volatile uint32_t system_ms;

#define SEND_TIMEOUT_MS  50  /* max wait for TX buffer to drain (ms) */
#define BT_RX_BUFFER_SIZE 128U

static volatile uint8_t bt_rx_buffer[BT_RX_BUFFER_SIZE];
static volatile uint16_t bt_rx_head = 0U;
static volatile uint16_t bt_rx_tail = 0U;
static volatile uint32_t bt_rx_overflow_count = 0U;

/**
  * @brief  USART3 initialization for Bluetooth: 115200-8N1,
  *         PB10(TX) -> Bluetooth RXD, PB11(RX) <- Bluetooth TXD.
  */
void usart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    /* PB10 - USART3 TX (alternate function push-pull) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* PB11 - USART3 RX (input pull-up) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    bt_rx_head = 0U;
    bt_rx_tail = 0U;

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART3, ENABLE);
}

/**
  * @brief  Send a single character via Bluetooth USART3.
  */
void my_send(char ch)
{
    uint32_t start = system_ms;
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
    {
        if (system_ms - start >= SEND_TIMEOUT_MS)
            return;  /* timeout: skip to prevent deadlock */
    }
    USART_SendData(USART3, (uint16_t)ch);
}

/**
  * @brief  Send a string of given length via Bluetooth USART3.
  * @param  str  Pointer to the string buffer.
  * @param  num  Number of characters to send.
  */
void my_send_string(const char *str, int num)
{
    while (num--)
    {
        my_send(*str++);
    }
}

int my_usart_read_byte(char *ch)
{
    if (bt_rx_head == bt_rx_tail)
    {
        return 0;
    }

    *ch = (char)bt_rx_buffer[bt_rx_tail];
    bt_rx_tail = (uint16_t)((bt_rx_tail + 1U) % BT_RX_BUFFER_SIZE);
    return 1;
}

void my_usart_irq_handler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uint8_t value = (uint8_t)(USART_ReceiveData(USART3) & 0xFFU);
        uint16_t next_head = (uint16_t)((bt_rx_head + 1U) % BT_RX_BUFFER_SIZE);
        if (next_head != bt_rx_tail)
        {
            bt_rx_buffer[bt_rx_head] = value;
            bt_rx_head = next_head;
        }
        else
        {
            bt_rx_overflow_count++;
        }
    }

    if (USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
    {
        (void)USART_ReceiveData(USART3);
        USART_ClearFlag(USART3, USART_FLAG_ORE);
    }
}

uint32_t my_usart_rx_overflow_count(void)
{
    return bt_rx_overflow_count;
}
