#ifndef __BOARD_SYSTICK_H
#define __BOARD_SYSTICK_H

void BSW_SysTick_Config(uint32_t sys_clk);
void BSP_SysTick_Inc(void);
uint32_t BSP_GetSysTick(void);

#endif