/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_it.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2024/03/06
* Description        : Main Interrupt Service Routines.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32v30x_it.h"
#include "ch32v30x_gpio.h"

/* 声明外部的全局滴答计数变量 */
extern volatile uint32_t sys_tick_ms;

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
  while (1)
  {
  }
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler(void)
{
  NVIC_SystemReset();
  while (1)
  {

  }
}

/**
 * @brief  SysTick 系统滴答中断服务程序
 */
/* 使用 WCH 硬件快速中断特有的免表跳转与硬件压栈属性 */
__attribute__((interrupt("WCH-Interrupt-fast"))) void SysTick_Handler(void)
{
    /* 清除 SysTick 比较寄存器状态标志位 */
    SysTick->SR &= ~(1 << 0);
    
    /* 累加时间基准（单位：毫秒） */
    sys_tick_ms++;
}


