/**
  ******************************************************************************
  * @file    stm32f10x_it.c
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "myusart.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* 1ms tick counter for non-blocking timing, declared extern for other .c files */
volatile uint32_t system_ms = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  */
__NO_RETURN void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  */
__NO_RETURN void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  */
__NO_RETURN void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  */
__NO_RETURN void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  *         Increments system_ms at 1ms intervals.
  */
void SysTick_Handler(void)
{
    system_ms++;
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  Legacy EXTI handler retained; Bluetooth now uses USART3 on PB10/PB11.
  */
void EXTI15_10_IRQHandler(void)
{
}

/**
  * @brief  This function handles USART1 interrupt request.
  *         USART1 is reserved for system ISP/flashing on PA9/PA10.
  */
void USART1_IRQHandler(void)
{
}

/**
  * @brief  This function handles USART3 interrupt request.
  *         USART3 is assigned to Bluetooth/control UART on PB10/PB11.
  */
void USART3_IRQHandler(void)
{
  my_usart_irq_handler();
}

/**
  * @brief  This function handles PPP interrupt request.
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */
