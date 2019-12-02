/*-----------------------------------------------------------------------------
 *      Name:         DV_USBH.c
 *      Purpose:      USB Host test cases
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#include "cmsis_os.h" 
#include "cmsis_dv.h" 
#include "DV_Config.h"
#include "DV_Framework.h"
#include "Driver_USBH.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

// Register Driver_USBH#
extern ARM_DRIVER_USBH CREATE_SYMBOL(Driver_USBH, DRV_USBH);
static ARM_DRIVER_USBH *drv = &CREATE_SYMBOL(Driver_USBH, DRV_USBH);
static ARM_USBH_CAPABILITIES capab;  

// Event flags
static uint8_t volatile PortEvent;  
static uint8_t volatile PipeEvent; 

// USB Port event
static void USB_PortEvent (uint8_t port, uint32_t event) {
  PortEvent |= event;
}

// USB Pipe event
static void USB_PipeEvent (ARM_USBH_PIPE_HANDLE pipe_hndl, uint32_t event) {
  PipeEvent |= event;
}


/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/
 
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup usbh_funcs USB Host Validation
\brief USB Host test cases
\details
The USB Host validation test checks the API interface compliance only.
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: USBH_GetCapabilities
\details
The test case \b USBH_GetCapabilities verifies the function \b GetCapabilities.
*/
void USBH_GetCapabilities (void) {                    
  /* Get USBH capabilities */
  capab = drv->GetCapabilities();
  TEST_ASSERT(&capab != NULL); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USBH_Initialization
\details
The test case \b USBH_Initialization verifies the USBH functions with the sequence:
  - \b Initialize without callback
  - \b Uninitialize
  - \b Initialize with callback
  - \b Uninitialize
*/
void USBH_Initialization (void) { 
    
  /* Initialize without callback */
  TEST_ASSERT(drv->Initialize(NULL, NULL) == ARM_DRIVER_OK); 
    
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(USB_PortEvent, USB_PipeEvent) == ARM_DRIVER_OK); 
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USBH_CheckInvalidInit
\details
The test case \b USBH_CheckInvalidInit verifies the driver behaviour when receiving an invalid initialization sequence:
  - \b Uninitialize
  - \b PowerControl with Power off
  - \b PowerControl with Power on
  - \b PowerControl with Power off
  - \b Uninitialize
*/
void USBH_CheckInvalidInit (void) { 

  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Try to power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) != ARM_DRIVER_OK); 
  
  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USBH_PowerControl
\details
The test case \b USBH_PowerControl verifies the \b PowerControl function with the sequence:
 - Initialize
 - Power on
 - Power low
 - Power off
 - Uninitialize 
*/
void USBH_PowerControl (void) { 
  int32_t val;
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(USB_PortEvent, USB_PipeEvent) == ARM_DRIVER_OK); 
  
  /* Power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);  
  
  /* Power low */
  val = drv->PowerControl (ARM_POWER_LOW);
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Low power is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
   
  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/**
@}
*/ 
// end of group usbh_funcs
