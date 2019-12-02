/*-----------------------------------------------------------------------------
 *      Name:         DV_USBD.c
 *      Purpose:      USB Device test cases
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#include "cmsis_dv.h" 
#include "DV_Config.h"
#include "DV_Framework.h"
#include "Driver_USBD.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

// Register Driver_USBD#
extern ARM_DRIVER_USBD CREATE_SYMBOL(Driver_USBD, DRV_USBD);
static ARM_DRIVER_USBD *drv = &CREATE_SYMBOL(Driver_USBD, DRV_USBD);
static ARM_USBD_CAPABILITIES capab;  

// Event flags
static uint8_t volatile DeviceEvent;  
static uint8_t volatile EndpointEvent; 

// USB Device event
static void USB_DeviceEvent (uint32_t event) {
  DeviceEvent |= event;
}

// USB Endpoint event
static void USB_EndpointEvent (uint8_t endpoint, uint32_t event) {
  (void) endpoint;
  EndpointEvent |= event;
}


/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/
 
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup usbd_funcs USB Device Validation
\brief USB Device test cases
\details
The USB Device validation test checks the API interface compliance only. The section \ref usbd_comp_test explains how to run
the USB compliance tests. These tests check USB devices for conformance to the USB Device Framework which is required in
order to gain USB certification.
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: USBD_GetCapabilities
\details
The test case \b USBD_GetCapabilities verifies the function \b GetCapabilities.
*/
void USBD_GetCapabilities (void) {                    
  /* Get USBD capabilities */
  capab = drv->GetCapabilities();
  TEST_ASSERT(&capab != NULL); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USBD_Initialization
\details
The test case \b USBD_Initialization verifies the USBD functions with the sequence:
  - \b Initialize without callback
  - \b Uninitialize
  - \b Initialize with callback
  - \b Uninitialize
*/
void USBD_Initialization (void) { 
    
  /* Initialize without callback */
  TEST_ASSERT(drv->Initialize(NULL, NULL) == ARM_DRIVER_OK); 
    
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(USB_DeviceEvent, USB_EndpointEvent) == ARM_DRIVER_OK); 
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USBD_CheckInvalidInit
\details
The test case \b USBD_CheckInvalidInit verifies the driver behaviour when receiving an invalid initialization sequence:
  - \b Uninitialize
  - \b PowerControl with Power off
  - \b PowerControl with Power on
  - \b PowerControl with Power off
  - \b Uninitialize
*/
void USBD_CheckInvalidInit (void) { 

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
\brief  Test case: USBD_PowerControl
\details
The test case \b USBD_PowerControl verifies the \b PowerControl function with the sequence:
 - Initialize
 - Power on
 - Power low
 - Power off
 - Uninitialize 
*/
void USBD_PowerControl (void) { 
  int32_t val;
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(USB_DeviceEvent, USB_EndpointEvent) == ARM_DRIVER_OK); 
  
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
// end of group usbd_funcs
