/*------------------------------------------------------------------------------
 * Example main module
 * Copyright (c) 2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    main.c
 * Purpose: Main module
 *----------------------------------------------------------------------------*/

#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "cmsis_dv.h"

#include "peripherals.h"                // Keil::Board Support:SDK Project Template:Project_Template
#include "pin_mux.h"                    // Keil::Board Support:SDK Project Template:Project_Template
#include "board.h"                      // Keil::Board Support:SDK Project Template:Project_Template


/*------------------------------------------------------------------------------
 * main function
 *----------------------------------------------------------------------------*/
int main(void) {

  // System initialization
  BOARD_InitBootPeripherals();
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_InitDebugConsole();

  // Update System Core Clock info
  SystemCoreClockUpdate();

  osKernelInitialize ();                        // Initialize CMSIS-RTOS2
  osThreadNew (cmsis_dv, NULL, NULL);           // Create application main thread
  osKernelStart ();                             // Start thread execution

  for (;;) {}
}
