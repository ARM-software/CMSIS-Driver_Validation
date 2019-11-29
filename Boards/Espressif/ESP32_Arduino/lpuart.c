/*------------------------------------------------------------------------------
 * Name:    lpuart.c
 * Purpose: fsl_lpuart_cmsis driver helper file
 *----------------------------------------------------------------------------*/

#include "clock_config.h"

uint32_t LPUART1_GetFreq(void) {
  return BOARD_BOOTCLOCKRUN_UART_CLK_ROOT;
}

uint32_t LPUART3_GetFreq(void) {
  return BOARD_BOOTCLOCKRUN_UART_CLK_ROOT;
}
