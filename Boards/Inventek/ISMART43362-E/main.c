/*------------------------------------------------------------------------------
 * Copyright (c) 2019 Arm Limited (or its affiliates). All
 * rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   1.Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   2.Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   3.Neither the name of Arm nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *------------------------------------------------------------------------------
 * Name:    main.c
 * Purpose: Main module
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "board.h"
#include "pin_mux.h"

#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_gint.h"
#include "fsl_inputmux.h"

#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"

#include "cmsis_dv.h"                   // ARM.API::CMSIS Driver Validation:Framework

extern void WiFi_ISM43362_Pin_DATARDY_IRQ (void);

static void gint_callback (void);


/*------------------------------------------------------------------------------
 * main function
 *----------------------------------------------------------------------------*/
int main (void) {

  /* Init board hardware. */
  /* attach main clock divide to FLEXCOMM0 (debug console) */
  CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

  BOARD_InitBootPins();
  BOARD_BootClockFROHF96M();
  BOARD_InitDebugConsole();

  GINT_Init(GINT0);                     // Initialize GINT

  // Setup GINT for edge trigger, "OR" mode
  GINT_SetCtrl(GINT0, kGINT_CombineOr, kGINT_TrigEdge, gint_callback);
  // Select pin P1_5 active high
  GINT_ConfigPins(GINT0, kGINT_Port1, (1U << 5), (1U << 5));
  // Enable callback for GINT
  GINT_EnableCallback(GINT0);

  SystemCoreClockUpdate();

  SDK_DelayAtLeastUs(5000);             // 5 seconds for debugger connect

  // CMSIS RTOS2 initialization and start
  osKernelInitialize();                 // Initialize CMSIS-RTOS
#if defined(RTE_CMSIS_RTOS2)
  osThreadNew(cmsis_dv, NULL, NULL);    // Create validation main thread
#else
  cmsis_dv(NULL);
#endif
  osKernelStart();                      // Start thread execution
  for (;;) {}
}

/**
  \fn          void pint_intr_callback (pint_pin_int_t pintr, uint32_t pmatch_status)
  \brief       Callback function called when pin interrupt is trigerred.
  \return      none
*/
static void gint_callback (void) {
//  printf("DATARDY positive edge\n");
  WiFi_ISM43362_Pin_DATARDY_IRQ();
}

uint32_t SPI8_GetFreq(void) {
  return CLOCK_GetFreq(kCLOCK_HsLspi);
}

void SPI8_InitPins(void) {
  // Pins are initialized in main function in BOARD_InitBootPins function
}

void SPI8_DeinitPins(void) {
  // Pins deinitialization is not implemented
}
