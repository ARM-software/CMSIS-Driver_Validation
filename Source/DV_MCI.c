/*-----------------------------------------------------------------------------
 *      Name:         DV_MCI.c
 *      Purpose:      MCI test cases
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/ 
#include "cmsis_dv.h" 
#include "DV_Config.h"
#include "DV_Framework.h"
#include "Driver_MCI.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

// Register Driver_USBD#
extern ARM_DRIVER_MCI CREATE_SYMBOL(Driver_MCI, DRV_MCI);
static ARM_DRIVER_MCI *drv = &CREATE_SYMBOL(Driver_MCI, DRV_MCI);
static ARM_MCI_CAPABILITIES capab;  

// Event flags
static uint8_t volatile Event;    

// MCI event
static void MCI_DrvEvent (uint32_t event) {
  Event |= event;
}

/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/
 
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup mci_funcs MCI Validation
\brief MCI test cases
\details
The MCI validation test checks the API interface compliance only.
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: MCI_GetCapabilities
\details
The test case \b MCI_GetCapabilities verifies the function \b GetCapabilities.
*/
void MCI_GetCapabilities (void) {                    
  /* Get USBD capabilities */
  capab = drv->GetCapabilities();
  TEST_ASSERT(&capab != NULL); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: MCI_Initialization
\details
The test case \b MCI_Initialization verifies the MCI functions with the sequence:
  - \b Initialize without callback
  - \b Uninitialize
  - \b Initialize with callback
  - \b Uninitialize
*/
void MCI_Initialization (void) { 
    
  /* Initialize without callback */
  TEST_ASSERT(drv->Initialize(NULL) == ARM_DRIVER_OK); 
    
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(MCI_DrvEvent) == ARM_DRIVER_OK); 
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: MCI_CheckInvalidInit
\details
The test case \b MCI_CheckInvalidInit verifies the driver behaviour when receiving an invalid initialization sequence:
  - \b Uninitialize
  - \b PowerControl with Power off
  - \b PowerControl with Power on
  - \b PowerControl with Power off
  - \b Uninitialize
*/
void MCI_CheckInvalidInit (void) { 
  
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
\brief  Test case: MCI_PowerControl
\details
The test case \b MCI_PowerControl verifies the \b PowerControl function with the sequence:
 - Initialize
 - Power on
 - Power low
 - Power off
 - Uninitialize
*/
void MCI_PowerControl (void) { 
  int32_t val;
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(MCI_DrvEvent) == ARM_DRIVER_OK); 
  
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

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: MCI_SetBusSpeedMode
\details
The test case \b MCI_SetBusSpeedMode verifies the \b Control function and sets the bus speed with the sequence:
 - Initialize
 - Power on
 - default speed
 - high speed 
 - SDR12 speed
 - SDR25 speed
 - SDR50 speed
 - SDR104 speed
 - DDR50 speed
 - Power off
 - Uninitialize
*/
void MCI_SetBusSpeedMode (void) { 
  int32_t val;
  
  /* Initialize with callback and power on*/
  TEST_ASSERT(drv->Initialize(MCI_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);  
  
  /* Default speed */
  TEST_ASSERT(drv->Control (ARM_MCI_BUS_SPEED_MODE, ARM_MCI_BUS_DEFAULT_SPEED ) == ARM_DRIVER_OK);
  
  /* High speed */
  val = drv->Control (ARM_MCI_BUS_SPEED_MODE, ARM_MCI_BUS_DEFAULT_SPEED );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] High speed is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* SDR12 speed */
  val = drv->Control (ARM_MCI_BUS_SPEED_MODE, ARM_MCI_BUS_UHS_SDR12 );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] SDR12 is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* SDR25 speed */
  val = drv->Control (ARM_MCI_BUS_SPEED_MODE, ARM_MCI_BUS_UHS_SDR25 );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] SDR25 is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }

  /* SDR50 speed */
  val = drv->Control (ARM_MCI_BUS_SPEED_MODE, ARM_MCI_BUS_UHS_SDR50 );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] SDR50 is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }

  /* SDR104 speed */
  val = drv->Control (ARM_MCI_BUS_SPEED_MODE, ARM_MCI_BUS_UHS_SDR104 );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] SDR104 is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }

  /* DDR50 speed */
  val = drv->Control (ARM_MCI_BUS_SPEED_MODE, ARM_MCI_BUS_UHS_DDR50 );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] DDR50 is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: MCI_Config_DataWidth
\details
The test case \b MCI_Config_DataWidth verifies the \b Control function and set the data width with the sequence:
 - Initialize
 - Power on
 - Default data width \token{1}
 - Data width \token{4}
 - Data width \token{8}
 - Data width \token{4 DDR}
 - Data width \token{8 DDR}
 - Power off
 - Uninitialize
*/
void MCI_Config_DataWidth (void) { 
  int32_t val;
  
  /* Initialize with callback and power on*/
  TEST_ASSERT(drv->Initialize(MCI_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Default data width 1 */
  TEST_ASSERT(drv->Control (ARM_MCI_BUS_DATA_WIDTH, ARM_MCI_BUS_DATA_WIDTH_1 ) == ARM_DRIVER_OK);
  
  /* Data width 4 */
  val = drv->Control (ARM_MCI_BUS_DATA_WIDTH, ARM_MCI_BUS_DATA_WIDTH_4 );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Data width 4 is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Data width 8 */
  val = drv->Control (ARM_MCI_BUS_DATA_WIDTH, ARM_MCI_BUS_DATA_WIDTH_8 );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Data width 8 is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Data width 4 DDR */
  val = drv->Control (ARM_MCI_BUS_DATA_WIDTH, ARM_MCI_BUS_DATA_WIDTH_4_DDR );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Data width 4 DDR is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }

  /* Data width 8 DDR */
  val = drv->Control (ARM_MCI_BUS_DATA_WIDTH, ARM_MCI_BUS_DATA_WIDTH_8_DDR );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Data width 8 DDR is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: MCI_Config_CmdLineMode
\details
The test case \b MCI_Config_CmdLineMode verifies the \b Control function with the sequence:
 - Initialize
 - Power on
 - Default push-pull
 - Open Drain
 - Power off
 - Uninitialize
*/
void MCI_Config_CmdLineMode (void) { 
  int32_t val;
  
  /* Initialize with callback and power on*/
  TEST_ASSERT(drv->Initialize(MCI_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Default push-pull */
  TEST_ASSERT(drv->Control (ARM_MCI_BUS_CMD_MODE, ARM_MCI_BUS_CMD_PUSH_PULL  ) == ARM_DRIVER_OK);
  
  /* Open Drain */
  val = drv->Control (ARM_MCI_BUS_CMD_MODE, ARM_MCI_BUS_CMD_OPEN_DRAIN );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Open Drain Cmd is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: MCI_Config_DriverStrength
\details
The test case \b MCI_Config_DriverStrength verifies the \b Control function and sets the driver strength with the sequence:
 - Initialize
 - Power on
 - Type A
 - Type B
 - Type C
 - Type D
 - Power off
 - Uninitialize
*/
void MCI_Config_DriverStrength (void) { 
  int32_t val;
  
  /* Initialize with callback and power on*/
  TEST_ASSERT(drv->Initialize(MCI_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Type A */
  val = drv->Control ( ARM_MCI_DRIVER_STRENGTH, ARM_MCI_DRIVER_TYPE_A );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Type A is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Type B */
  val = drv->Control ( ARM_MCI_DRIVER_STRENGTH, ARM_MCI_DRIVER_TYPE_B );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Type B is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Type C */
  val = drv->Control ( ARM_MCI_DRIVER_STRENGTH, ARM_MCI_DRIVER_TYPE_C );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Type C is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Type D */
  val = drv->Control ( ARM_MCI_DRIVER_STRENGTH, ARM_MCI_DRIVER_TYPE_D );
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Type D is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/**
@}
*/ 
// end of group mci_funcs
