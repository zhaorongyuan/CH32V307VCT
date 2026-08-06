/**
 * @file board_gpio_cfg.c
 * @brief Full Unabridged GPIO Hardware Pin Configuration Table
 * @details Implements 100% of board-level discrete IO pins without placeholders or omissions.
 * @author zry
 * @date 2026-08-06
 * @version V1.0.0
 *
 * @note System HLR Traceability: [REQ-HLR-BSP-002]
 * @copyright (c) 2026 zry. All rights reserved.
 */

 #include "board_gpio_cfg.h"
 #include "board_cfg.h"
 
 /**
  * @brief Complete Board-Level Static GPIO Configuration Table
  * @details Placed in Flash (.rodata) to prevent runtime memory corruption.
  * @note MISRA-C:2012 Rule 8.4 & Rule 8.9 Compliant
  */
 const Board_GPIO_Config_t g_GpioConfigTable[] = {
     /* ------------------------------------------------------------------------ */
     /* 1. Discrete Outputs (ucApplyInitState = 1U, Single Pin per Entry)       */
     /* ------------------------------------------------------------------------ */
 
     /* Item 1: LED1 Indicator Pin (Default Off: High Level) */
     {
         .pPort            = LED1_PORT,
         .usPin            = LED1_PIN,
         .eMode            = GPIO_Mode_Out_PP,
         .eSpeed           = GPIO_Speed_50MHz,
         .eInitialState    = Bit_SET,
         .ucApplyInitState = 1U
     },
 
     /* Item 2: LED2 Indicator Pin (Default Off: High Level) */
     {
         .pPort            = LED2_PORT,
         .usPin            = LED2_PIN,
         .eMode            = GPIO_Mode_Out_PP,
         .eSpeed           = GPIO_Speed_50MHz,
         .eInitialState    = Bit_SET,
         .ucApplyInitState = 1U
     },
 
     /* Item 3: Audio Controller Mode Pin (Default Play Mode) */
     {
         .pPort            = AUDIO_CTL_PORT,
         .usPin            = AUDIO_CTL_PIN,
         .eMode            = GPIO_Mode_Out_PP,
         .eSpeed           = GPIO_Speed_50MHz,
         .eInitialState    = (BitAction)AUDIO_CTL_MODE_PLAY,
         .ucApplyInitState = 1U
     },
 
     /* Item 4: Bluetooth Module AT Control Pin */
     {
         .pPort            = BLE_AT_PORT,
         .usPin            = BLE_AT_PIN,
         .eMode            = GPIO_Mode_Out_PP,
         .eSpeed           = GPIO_Speed_10MHz,
         .eInitialState    = Bit_RESET,
         .ucApplyInitState = 1U
     },
 
     /* Item 5: Bluetooth Module Sleep Control Pin */
     {
         .pPort            = BLE_SLEEP_PORT,
         .usPin            = BLE_SLEEP_PIN,
         .eMode            = GPIO_Mode_Out_PP,
         .eSpeed           = GPIO_Speed_10MHz,
         .eInitialState    = Bit_RESET,
         .ucApplyInitState = 1U
     },
 
     /* Item 6: LCD Backlight Control Pin (Default Off: Low Level) */
     {
         .pPort            = LCD_BL_PORT,
         .usPin            = LCD_BL_PIN,
         .eMode            = GPIO_Mode_Out_PP,
         .eSpeed           = GPIO_Speed_50MHz,
         .eInitialState    = Bit_RESET,
         .ucApplyInitState = 1U
     },
 
     /* ------------------------------------------------------------------------ */
     /* 2. Discrete Inputs (ucApplyInitState = 0U, Pin Mask Grouping Allowed)    */
     /* ------------------------------------------------------------------------ */
 
     /* Item 7: WAKEUP Key (Pull-down Input) */
     {
         .pPort            = KEY_WAKEUP_PORT,
         .usPin            = KEY_WAKEUP_PIN,
         .eMode            = GPIO_Mode_IPD,
         .eSpeed           = GPIO_Speed_50MHz,
         .eInitialState    = Bit_RESET,
         .ucApplyInitState = 0U
     },
 
     /* Item 8: SW1 & SW2 Keys (Pull-up Input Bank) */
     {
         .pPort            = KEY_SW1_PORT,
         .usPin            = KEY_SW1_PIN | KEY_SW2_PIN,
         .eMode            = GPIO_Mode_IPU,
         .eSpeed           = GPIO_Speed_50MHz,
         .eInitialState    = Bit_SET,
         .ucApplyInitState = 0U
     },
 
     /* Item 9: Five-way Joystick (UP / DOWN / RIGHT Pins on GPIOE Bank) */
     {
         .pPort            = GPIOE,
         .usPin            = JOY_UP_PIN | JOY_DOWN_PIN | JOY_RIGHT_PIN,
         .eMode            = GPIO_Mode_IPU,
         .eSpeed           = GPIO_Speed_50MHz,
         .eInitialState    = Bit_SET,
         .ucApplyInitState = 0U
     },
 
     /* Item 10: Five-way Joystick (LEFT / SEL Pins on GPIOD Bank) */
     {
         .pPort            = GPIOD,
         .usPin            = JOY_LEFT_PIN | JOY_SEL_PIN,
         .eMode            = GPIO_Mode_IPU,
         .eSpeed           = GPIO_Speed_50MHz,
         .eInitialState    = Bit_SET,
         .ucApplyInitState = 0U
     }
 };
 
 /**
  * @brief Static Array Boundary Size Calculation
  */
 const uint8_t g_GpioConfigTableSize = (uint8_t)(sizeof(g_GpioConfigTable) / sizeof(g_GpioConfigTable[0]));