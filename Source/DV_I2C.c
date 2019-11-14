/*-----------------------------------------------------------------------------
 *      Name:         DV_I2C.c
 *      Purpose:      I2C test cases
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#include "cmsis_dv.h" 
#include "DV_Config.h"
#include "DV_Framework.h"
#include "Driver_I2C.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

// Register Driver_I2C#
extern ARM_DRIVER_I2C CREATE_SYMBOL(Driver_I2C, DRV_I2C);
static ARM_DRIVER_I2C *drv = &CREATE_SYMBOL(Driver_I2C, DRV_I2C);
static ARM_I2C_CAPABILITIES capab;  

// Event flags
static uint8_t volatile Event;    

// I2C event
static void I2C_DrvEvent (uint32_t event) {
  Event |= event;
}

/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/
 
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup i2c_funcs I2C Validation
\brief I2C test cases
\details
The I2C validation test checks the API interface compliance only.
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: I2C_GetCapabilities
\details
The test case \b I2C_GetCapabilities verifies the function \b GetCapabilities.
*/
void I2C_GetCapabilities (void) {                    
  /* Get SPI capabilities */
  capab = drv->GetCapabilities();
  TEST_ASSERT(&capab != NULL); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: I2C_Initialization
\details
The test case \b I2C_Initialization verifies the I2C functions with the sequence:
  - \b Initialize  without callback
  - \b Uninitialize
  - \b Initialize with callback
  - \b Uninitialize
*/
void I2C_Initialization (void) { 
  
  /* Initialize without callback */
  TEST_ASSERT(drv->Initialize(NULL) == ARM_DRIVER_OK); 
    
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(I2C_DrvEvent) == ARM_DRIVER_OK); 
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: I2C_CheckInvalidInit
\details
The test case \b I2C_CheckInvalidInit verifies the driver behaviour when receiving an invalid initialization sequence:
  - \b Uninitialize
  - \b PowerControl with Power off
  - \b PowerControl with Power on
  - \b Control with bus speed fast 
  - \b PowerControl with Power off
  - \b Uninitialize
*/
void I2C_CheckInvalidInit (void) { 
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Try to power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) != ARM_DRIVER_OK); 
  
  /* Try to set configuration */
  TEST_ASSERT(drv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST) != ARM_DRIVER_OK);

  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: I2C_PowerControl
\details
The test case \b I2C_PowerControl verifies the \b PowerControl function with the sequence:
 - Initialize 
 - Power on
 - Power low
 - Power off 
 - Uninitialize 
*/
void I2C_PowerControl (void) { 
  int32_t val;
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(I2C_DrvEvent) == ARM_DRIVER_OK); 
  
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
\brief  Test case: I2C_SetBusSpeed
\details
The test case \b I2C_SetBusSpeed verifies the \b Control function with the sequence:
 - Initialize 
 - Power on
 - Set bus speed standard 
 - Set bus speed fast 
 - Set bus speed fast plus 
 - Set bus speed high 
 - Power off 
 - Uninitialize 
*/
void I2C_SetBusSpeed (void) { 
int32_t val;  
  
  /* Initialize with callback and power on */
  TEST_ASSERT(drv->Initialize(I2C_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Set bus speed standard */
  TEST_ASSERT(drv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD) == ARM_DRIVER_OK);
  
  /* Set bus speed fast */
  val = drv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Fast speed is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  /* Set bus speed fast plus */
  val = drv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST_PLUS);
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Fast plus speed is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  /* Set bus speed high */
  val = drv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_HIGH);
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] High speed is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: I2C_SetOwnAddress
\details
The test case \b I2C_SetOwnAddress verifies the \b Control function with the sequence:
 - Initialize 
 - Power on
 - Set slave own address \token{0x0000}
 - Set slave own address \token{0x0001}
 - Set slave own address \token{0x00FF}
 - Set slave own address \token{0x03FF}
 - Power off 
 - Uninitialize 
*/
void I2C_SetOwnAddress (void) { 
  
  /* Initialize with callback and power on */
  TEST_ASSERT(drv->Initialize(I2C_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Set slave own address */
  TEST_ASSERT(drv->Control(ARM_I2C_OWN_ADDRESS, 0x0000) == ARM_DRIVER_OK);
  
  /* Set slave own address */
  TEST_ASSERT(drv->Control(ARM_I2C_OWN_ADDRESS, 0x0001) == ARM_DRIVER_OK);
  
  /* Set slave own address */
  TEST_ASSERT(drv->Control(ARM_I2C_OWN_ADDRESS, 0x00FF) == ARM_DRIVER_OK);
  
  /* Set slave own address */
  TEST_ASSERT(drv->Control(ARM_I2C_OWN_ADDRESS, 0x03FF) == ARM_DRIVER_OK);
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);   
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: I2C_BusClear
\details
The test case \b I2C_BusClear verifies the \b Control function with the sequence:
 - Initialize 
 - Power on
 - Clear Bus
 - Power off 
 - Uninitialize 
*/
void I2C_BusClear (void) { 
  
  /* Initialize with callback and power on */
  TEST_ASSERT(drv->Initialize(I2C_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  
  /* Clear Bus */
  TEST_ASSERT(drv->Control(ARM_I2C_BUS_CLEAR, 0x0000) == ARM_DRIVER_OK);
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}
  
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: I2C_AbortTransfer
\details
The test case \b I2C_AbortTransfer verifies the \b Control function with the sequence:
 - Initialize 
 - Power on
 - Abort transfer
 - Power off 
 - Uninitialize 
*/
void I2C_AbortTransfer (void) { 
  
  /* Initialize with callback and power on */
  TEST_ASSERT(drv->Initialize(I2C_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  
  /* Abort transfer */
  TEST_ASSERT(drv->Control(ARM_I2C_ABORT_TRANSFER, 0x0000) == ARM_DRIVER_OK);
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/**
@}
*/ 
// end of group i2c_funcs

