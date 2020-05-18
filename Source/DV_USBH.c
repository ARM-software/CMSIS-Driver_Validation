/*
 * Copyright (c) 2015-2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * -----------------------------------------------------------------------------
 *
 * Project:     CMSIS-Driver Validation
 * Title:       Universal Serial Bus (USB) Host Driver Validation tests
 *
 * -----------------------------------------------------------------------------
 */


#include "cmsis_dv.h" 
#include "DV_USBH_Config.h"
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
 *      Tests
 *----------------------------------------------------------------------------*/
 
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup dv_usbh USB Host Validation
\brief USB Host driver validation
\details
The USB Host validation test checks the API interface compliance only.

\defgroup usbh_tests Tests
\ingroup dv_usbh

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: USBH_GetCapabilities
\details
The test function \b USBH_GetCapabilities verifies the function \b GetCapabilities.
*/
void USBH_GetCapabilities (void) {                    
  /* Get USBH capabilities */
  capab = drv->GetCapabilities();
  TEST_ASSERT(&capab != NULL); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Function: USBH_Initialization
\details
The test function \b USBH_Initialization verifies the USBH functions with the sequence:
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
\brief  Function: USBH_CheckInvalidInit
\details
The test function \b USBH_CheckInvalidInit verifies the driver behaviour when receiving an invalid initialization sequence:
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
\brief  Function: USBH_PowerControl
\details
The test function \b USBH_PowerControl verifies the \b PowerControl function with the sequence:
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
// end of group dv_usbh
