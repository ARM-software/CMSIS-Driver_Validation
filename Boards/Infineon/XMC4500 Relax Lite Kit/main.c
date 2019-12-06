/*----------------------------------------------------------------------------
 * Name:    main.c
 *----------------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "cmsis_dv.h"                   // ARM.API::CMSIS Driver Validation:Framework

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

  osKernelInitialize ();                // Initialize CMSIS-RTOS2
  osThreadNew (cmsis_dv, NULL, NULL);   // Create validation main thread
  osKernelStart ();                     // Start thread execution

  for (;;) {}
}
