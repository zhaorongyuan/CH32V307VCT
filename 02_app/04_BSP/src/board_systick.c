
#include "ch32v30x_conf.h"
#include "board_systick.h"

static volatile uint32_t s_sys_tick_ms = 0U;

/**
 * @brief  set SysTick is 1ms tickÖÐ¶Ï
 */
void BSW_SysTick_Config(uint32_t sys_clk)
{
    uint64_t s_systick_reload_val = sys_clk / 1000U;

    SysTick->CTLR &= ~(1U << 0);
    SysTick->CNT = 0;
    SysTick->SR = 0;
    SysTick->CMP = s_systick_reload_val - 1U;

    SysTick->CTLR |= (1U << 4) | (1U << 3) | (1U << 2);     
    SysTick->CTLR |= (1U << 1);
    SysTick->CTLR |= (1U << 0);

    NVIC_EnableIRQ(SysTick_IRQn);
}

void BSP_SysTick_Inc(void)
{
    s_sys_tick_ms++;
}

uint32_t BSP_GetSysTick(void)
{
  return s_sys_tick_ms;
}
