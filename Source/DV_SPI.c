/*-----------------------------------------------------------------------------
 *      Name:         DV_SPI.c
 *      Purpose:      SPI test cases
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#include "cmsis_dv.h" 
#include "DV_Config.h"
#include "DV_Framework.h"
#include "Driver_SPI.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

// SPI buffer type
#if (SPI_DATA_BITS>16U)
typedef uint32_t buf_t;
#elif (SPI_DATA_BITS>8U)
typedef uint16_t buf_t;
#else
typedef uint8_t buf_t;
#endif

// SPI buffer pointers
static buf_t *buffer_out; 
static buf_t *buffer_in; 

// SPI baudrates (kHz)
static const uint32_t SPI_BR[] =  {
#if (SPI_BR_1>0)
  SPI_BR_1,
#endif  
#if (SPI_BR_2>0)
  SPI_BR_2,
#endif 
#if (SPI_BR_3>0)
  SPI_BR_3,
#endif 
#if (SPI_BR_4>0)
  SPI_BR_4,
#endif 
#if (SPI_BR_5>0)
  SPI_BR_5,
#endif 
#if (SPI_BR_6>0)
  SPI_BR_6,
#endif 
};
static const uint32_t SPI_BR_NUM = ARRAY_SIZE(SPI_BR);
  
// Register Driver_SPI#
extern ARM_DRIVER_SPI CREATE_SYMBOL(Driver_SPI, DRV_SPI);
static ARM_DRIVER_SPI *drv = &CREATE_SYMBOL(Driver_SPI, DRV_SPI);
static ARM_SPI_CAPABILITIES capab;   

static char str[128];

// Event flags
static uint8_t volatile Event;    

// SPI event
static void SPI_DrvEvent (uint32_t event) {
  Event |= event;
}

// SPI transfer
int8_t SPI_RunTransfer (void *out, void *in, uint32_t cnt);
int8_t SPI_RunTransfer (void *out, void *in, uint32_t cnt) {
  uint32_t tick;
  
  Event &= ~ARM_SPI_EVENT_TRANSFER_COMPLETE;
  drv->Transfer (out, in, cnt);

  tick = GET_SYSTICK();
  do {
    if (Event & ARM_SPI_EVENT_TRANSFER_COMPLETE) {
      return 0;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(SPI_TRANSFER_TIMEOUT));

  drv->Control(ARM_SPI_ABORT_TRANSFER, 0);
  return -1;
}

// SPI send
int8_t SPI_RunSend (void *out, uint32_t cnt);
int8_t SPI_RunSend (void *out, uint32_t cnt) {
  uint32_t tick;
  
  Event &= ~ARM_SPI_EVENT_TRANSFER_COMPLETE;
  drv->Send (out, cnt);

  tick = GET_SYSTICK();
  do {
    if (Event & ARM_SPI_EVENT_TRANSFER_COMPLETE) {
      return 0;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(SPI_TRANSFER_TIMEOUT));

  drv->Control(ARM_SPI_ABORT_TRANSFER, 0);
  return -1;
}

// SPI send without callback
int8_t SPI_RunSend_NoCallback (void *out, uint32_t cnt);
int8_t SPI_RunSend_NoCallback (void *out, uint32_t cnt) {
  uint32_t tick;
    
  drv->Send (out, cnt);

  tick = GET_SYSTICK();
  do {
    if (drv->GetDataCount() == cnt) {
      return 0;
    } 
    if (drv->GetStatus().data_lost) {
      return -1;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(SPI_TRANSFER_TIMEOUT));

  drv->Control(ARM_SPI_ABORT_TRANSFER, 0);
  return -1;
}

// SPI receive
int8_t SPI_RunReceive (void *in, uint32_t cnt);
int8_t SPI_RunReceive (void *in, uint32_t cnt) {
  uint32_t tick;
  
  Event &= ~ARM_SPI_EVENT_TRANSFER_COMPLETE;
  drv->Receive (in, cnt);

  tick = GET_SYSTICK();
  do {
    if (Event & ARM_SPI_EVENT_TRANSFER_COMPLETE) {
      return 0;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(SPI_TRANSFER_TIMEOUT));

  drv->Control(ARM_SPI_ABORT_TRANSFER, 0);
  return -1;
}

// SPI receive
int8_t SPI_RunReceiveNoCallback (void *in, uint32_t cnt);
int8_t SPI_RunReceiveNoCallback (void *in, uint32_t cnt) {
  uint32_t tick;
  
  Event &= ~0x01;
  drv->Receive (in, cnt);

  tick = GET_SYSTICK();
  do {
    if (drv->GetDataCount() == cnt) {
      return 0;
    } 
    if (drv->GetStatus().data_lost) {
      return -1;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(SPI_TRANSFER_TIMEOUT));

  drv->Control(ARM_SPI_ABORT_TRANSFER, 0);
  return -1;
}

/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/
 
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup spi_funcs SPI Validation
\brief SPI test cases
\details
The SPI validation test performs the following checks:
- API interface compliance.
- Data communication with various transfer sizes and communication parameters.
- Transfer speed of the data communication.
- Loopback communication.

\anchor spi_loop_back_setup
Loopback Communication Setup
----------------------------

To perform loopback communication tests, it is required to connect the SPI's \b MISO signal to the \b MOSI signal (refer to
the schematics of your target hardware for detailed pinout information).
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: SPI_GetCapabilities
\details
The test case \b SPI_GetCapabilities verifies the function \b GetCapabilities.
*/
void SPI_GetCapabilities (void) {                    
  /* Get SPI capabilities */
  capab = drv->GetCapabilities();
  TEST_ASSERT(&capab != NULL); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_Initialization
\details
The test case \b SPI_Initialization verifies the SPI functions with the sequence:
  - \b Initialize without callback
  - \b Uninitialize
  - \b Initialize with callback
  - \b Uninitialize
*/
void SPI_Initialization (void) { 
    
  /* Initialize without callback */
  TEST_ASSERT(drv->Initialize(NULL) == ARM_DRIVER_OK); 
    
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK); 
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_CheckInvalidInit
\details
The test case \b SPI_CheckInvalidInit verifies the driver behaviour when receiving an invalid initialization sequence:
  - \b Uninitialize
  - \b PowerControl with Power off
  - \b PowerControl with Power on
  - \b Control 
  - \b PowerControl with Power off
  - \b Uninitialize
*/
void SPI_CheckInvalidInit (void) { 
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Try to power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) != ARM_DRIVER_OK); 
  
  /* Try to set configuration */
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(8) | ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_UNUSED, SPI_BR[0]*1000)
    != ARM_DRIVER_OK);
  
  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_PowerControl
\details
The test case \b SPI_PowerControl verifies the \b PowerControl function with the sequence:
 - Initialize with callback
 - Power on
 - Power low
 - Power off
 - Uninitialize
*/
void SPI_PowerControl (void) { 
  int32_t val;
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK); 
  
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
\brief  Test case: SPI_Config_PolarityPhase
\details
The test case \b SPI_Config_PolarityPhase verifies the \b Control function with the sequence:
 - Initialize with callback
 - Power on
 - Set basic SPI bus configuration
 - Change polarity
 - Change phase 
 - Change polarity and phase
 - Power off
 - Uninitialize
*/
void SPI_Config_PolarityPhase (void) { 
  
  /* Initialize with callback and power on*/
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK);   
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Set basic SPI bus configuration*/
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(SPI_DATA_BITS), SPI_BR[0]*1000) == ARM_DRIVER_OK);
  
  /* Change polarity */
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA0 | ARM_SPI_DATA_BITS(SPI_DATA_BITS), SPI_BR[0]*1000) == ARM_DRIVER_OK);
  
  /* Change phase */
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA1 | ARM_SPI_DATA_BITS(SPI_DATA_BITS), SPI_BR[0]*1000) == ARM_DRIVER_OK);
  
  /* Change polarity and phase */
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_DATA_BITS(SPI_DATA_BITS), SPI_BR[0]*1000) == ARM_DRIVER_OK);
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_Config_DataBits
\details
The test case \b SPI_Config_DataBits verifies the \b Control function with the sequence:
 - Initialize with callback
 - Power on
 - Data bits = \token{8}
 - Data bits = \token{16}
 - Power off
 - Uninitialize
*/
void SPI_Config_DataBits (void) { 
  int32_t val;  
  
  /* Initialize with callback and power on*/
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK);   
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Data bits = 8 */
  val = drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(8), SPI_BR[0]*1000);
  if (val == ARM_SPI_ERROR_DATA_BITS) { TEST_MESSAGE("[WARNING] Data Bits = 8 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  /* Data bits = 16 */
  val = drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(16), SPI_BR[0]*1000);
  if (val == ARM_SPI_ERROR_DATA_BITS) { TEST_MESSAGE("[WARNING] Data Bits = 16 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_Config_BitOrder
\details
The test case \b SPI_Config_BitOrder verifies the \b Control function with the sequence:
 - Initialize with callback
 - Power on
 - Bit order LSB_MSB
 - Bit order MSB_LSB
 - Power off
 - Uninitialize
*/
void SPI_Config_BitOrder (void) { 
  int32_t val;  
  
  /* Initialize with callback and power on*/
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK);   
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Bit order LSB_MSB */
  val = drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(8) | ARM_SPI_LSB_MSB, SPI_BR[0]*1000);
  if (val == ARM_SPI_ERROR_BIT_ORDER) { TEST_MESSAGE("[WARNING] Bit order LSB_MSB is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Bit order MSB_LSB */
  val = drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(8) | ARM_SPI_MSB_LSB, SPI_BR[0]*1000);
  if (val == ARM_SPI_ERROR_BIT_ORDER) { TEST_MESSAGE("[WARNING] Bit order MSB_LSB is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_Config_SSMode
\details
The test case \b SPI_Config_SSMode verifies the \b Control function with the sequence:
 - Initialize with callback
 - Power on
 - SS Mode MASTER_HW_OUTPUT
 - SS Mode MASTER_HW_INPUT 
 - SS Mode MASTER_SW
 - Power off
 - Uninitialize
*/
void SPI_Config_SSMode (void) { 
  int32_t val;  
  
  /* Initialize with callback and power on*/
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK);   
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 

  /* SS Mode MASTER_HW_OUTPUT */
  val = drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(8) | ARM_SPI_SS_MASTER_HW_OUTPUT, SPI_BR[0]*1000);
  if (val == ARM_SPI_ERROR_SS_MODE) { TEST_MESSAGE("[WARNING] Slave select MASTER_HW_OUTPUT is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }

  /* SS Mode MASTER_HW_INPUT */
  val = drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(8) | ARM_SPI_SS_MASTER_HW_INPUT, SPI_BR[0]*1000);
  if (val == ARM_SPI_ERROR_SS_MODE) { TEST_MESSAGE("[WARNING] Slave select MASTER_HW_INPUT is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
  
  /* SS Mode MASTER_SW */
  val = drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(8) | ARM_SPI_SS_MASTER_SW, SPI_BR[0]*1000);
  if (val == ARM_SPI_ERROR_SS_MODE) { TEST_MESSAGE("[WARNING] Slave select MASTER_SW is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }

  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_Config_CommonParams
\details
The test case \b SPI_Config_CommonParams verifies the \b Control function with the sequence:
 - Initialize with callback
 - Power on
 - Configure SPI bus
 - Power off
 - Uninitialize
*/
void  SPI_Config_CommonParams (void) {
  
  /* Initialize with callback and power on*/
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK);   
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  
  /* Configure SPI bus */
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(SPI_DATA_BITS) |
    ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_UNUSED, SPI_BR[0]*1000) == ARM_DRIVER_OK);
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}
  
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_Config_BusSpeed
\details
The test case \b SPI_Config_BusSpeed verifies the \b Control function and tests bus speeds with the sequence:
 - Initialize with callback
 - Power on
 - Change bus speed with specific control parameter
 - Read bus speed
 - Change bus speed with general control parameter
 - Read bus speed
 - Power off
 - Uninitialize

 \note Typically, the SPI bus speed is limited to a maximum number. This means that for a bus speed of 25 MHz, values below
 25 MHz are also acceptable. If this test returns a higher value than the one that is being set, you need to check the
 driver.
*/
void SPI_Config_BusSpeed (void) { 
  int32_t val;
  uint32_t speed;   
  
  /* Initialize with callback, power on and configure SPI bus */
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK);   
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(SPI_DATA_BITS) |
    ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_UNUSED, SPI_BR[0]*1000) == ARM_DRIVER_OK);
  
  /* Test bus speeds */
  for (speed=0; speed<SPI_BR_NUM; speed++) {

    /* Change bus speed with specific control parameter*/
    TEST_ASSERT (drv->Control(ARM_SPI_SET_BUS_SPEED, SPI_BR[speed]*1000) 
       == ARM_DRIVER_OK);
          
    /* Read bus speed */
    val = drv->Control(ARM_SPI_GET_BUS_SPEED,0)/1000;
    if ((uint32_t)val != SPI_BR[speed]) {
      snprintf(str,sizeof(str),"[WARNING] Specific control set speed: %dkHz, Get speed: %dkHz",SPI_BR[speed], val);
      TEST_MESSAGE(str);
    } else TEST_PASS();
    
    /* Change bus speed with general control parameter*/
    TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(8) | ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_SW, SPI_BR[speed]*1000) 
       == ARM_DRIVER_OK);
       
    /* Read bus speed */
    val = drv->Control(ARM_SPI_GET_BUS_SPEED,0)/1000;
    if ((uint32_t)val != SPI_BR[speed]) {
      snprintf(str,sizeof(str),"[WARNING] General control set speed: %dkHz, Get speed: %dkHz",SPI_BR[speed], val);
      TEST_MESSAGE(str);
    } else TEST_PASS();
   
  }
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_Send
\details
The test case \b SPI_Send verifies the function \b Send with the sequence:
 - Initialize with callback
 - Power on
 - Send using callback
 - Send without callback
 - Power off
 - Uninitialize
*/
void SPI_Send (void) { 
  uint16_t cnt;
  
  /* Allocate buffer */
  buffer_out = malloc(BUFFER[BUFFER_NUM-1]*sizeof(buf_t));
  TEST_ASSERT(buffer_out != NULL);
  
  /* Initialize with callback, power on and configure SPI bus */
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK);   
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(SPI_DATA_BITS) |
    ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_UNUSED, SPI_BR[0]*1000) == ARM_DRIVER_OK);
  
  /* Send data chunks */
  for (cnt = 0; cnt<BUFFER_NUM; cnt++) { 
    
    /* Send using callback */
    if (SPI_RunSend(buffer_out, BUFFER[cnt]) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fail to send %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();   
    
    /* Send without callback */
    if (SPI_RunSend_NoCallback(buffer_out, BUFFER[cnt]) != ARM_DRIVER_OK) {   
      snprintf(str,sizeof(str),"[FAILED] Fail to send without callback %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();      
  } 
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Free buffer */
  free(buffer_out);    
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_Receive
\details
The test case \b SPI_Receive verifies the function \b Receive with the sequence:
 - Initialize with callback
 - Power on
 - Receive using callback
 - Receive without callback
 - Power off
 - Uninitialize
*/
void SPI_Receive (void) { 
  uint16_t cnt;
  
  /* Allocate buffer */
  buffer_in = malloc(BUFFER[BUFFER_NUM-1]*sizeof(buf_t));
  TEST_ASSERT(buffer_in != NULL);
  
  /* Initialize with callback, power on and configure SPI bus */
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK);   
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(SPI_DATA_BITS) |
    ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_UNUSED, SPI_BR[0]*1000) == ARM_DRIVER_OK);
  
  /* Receive data chunks */
  for (cnt = 0; cnt<BUFFER_NUM; cnt++) { 
    
    /* Receive using callback */
    if (SPI_RunReceive(buffer_in, BUFFER[cnt]) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fail to receive %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();
    
    /* Receive without callback */
    if (SPI_RunReceiveNoCallback(buffer_in, BUFFER[cnt]) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fail to receive without callback %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();       
  } 
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Free buffer */
  free(buffer_in);    
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_Loopback_CheckBusSpeed
\details
The test case \b SPI_Loopback_CheckBusSpeed verifies the \b Control function, configures various bus speeds, and measures
the transfer time with this sequence:
 - Initialize with callback
 - Power on
 - Change bus speed with specific control parameter
 - Measure transfer time
 - Power off
 - Uninitialize
 
 \note If this test issues errors or warnings, refer to the \ref test_results section for more information.
*/
void SPI_Loopback_CheckBusSpeed (void) {
  uint16_t cnt;
  int32_t val;
  uint32_t speed;
  uint32_t ticks_measured;
  uint32_t ticks_expected;  
  double rate;
  
  /* Allocate buffer */
  buffer_out = malloc(BUFFER_SIZE_BR*sizeof(buf_t));
  TEST_ASSERT(buffer_out != NULL);
  buffer_in = malloc(BUFFER_SIZE_BR*sizeof(buf_t));
  TEST_ASSERT(buffer_in != NULL);

  /* Initialize with callback, power on and configure SPI bus */
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK);   
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(SPI_DATA_BITS) |
    ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_UNUSED, SPI_BR[0]*1000) == ARM_DRIVER_OK);
    
  /* Set Local Loopback */
  SPI_LOCAL_LOOPBACK();
  
  /* Test bus speeds */
  for (speed=0; speed<SPI_BR_NUM; speed++) {
  
    /* Set output buffer with random data */
    srand(GET_SYSTICK());
    for (cnt = 0; cnt<BUFFER_SIZE_BR; cnt++) { 
      buffer_out[cnt] = (buf_t)rand()&((1U<<SPI_DATA_BITS)-1U);
    } 
  
    /* Change bus speed with specific control parameter*/
    TEST_ASSERT (drv->Control(ARM_SPI_SET_BUS_SPEED, SPI_BR[speed]*1000) 
       == ARM_DRIVER_OK);
          
    /* Read bus speed */
    val = drv->Control(ARM_SPI_GET_BUS_SPEED,0)/1000;
    
    /* Measure transfer time */
    ticks_measured = GET_SYSTICK();
    SPI_RunTransfer(buffer_out, buffer_in, BUFFER_SIZE_BR);
    ticks_measured = GET_SYSTICK() - ticks_measured;  
    ticks_expected = SYSTICK_MICROSEC(BUFFER_SIZE_BR*1000/(uint32_t)val);
    ticks_expected *= SPI_DATA_BITS;
    
    rate = (double)ticks_measured/ticks_expected;
    
    if ((rate>(1.0+(double)MIN_BUS_SPEED/100))||(rate<1.0)) {
      snprintf(str,sizeof(str),"[WARNING] At %dkHz: measured time is %f x expected time", val, rate);
      TEST_MESSAGE(str);
    } else TEST_PASS();     
    
    /* Check received data against sent data*/
    if (memcmp(buffer_in, buffer_out, BUFFER_SIZE_BR)!=0) {
      snprintf(str,sizeof(str),"[FAILED] At %dkHz: fail to check block of %d bytes", val, BUFFER_SIZE_BR);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS(); 
  }
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Free buffer */
  free(buffer_out);   
  free(buffer_in); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: SPI_Loopback_Transfer
\details
The test case \b SPI_Loopback_Transfer verifies the function \b Transfer with the sequence:
 - Initialize with callback
 - Power on
 - Transfer data
 - Check received data against sent data
 - Power off
 - Uninitialize
*/
void SPI_Loopback_Transfer (void) { 
  uint16_t cnt, i;
  uint8_t pattern[] = BUFFER_PATTERN;
  
  /* Allocate buffer */
  buffer_out = malloc(BUFFER[BUFFER_NUM-1]*sizeof(buf_t));
  TEST_ASSERT(buffer_out != NULL);
  buffer_in = malloc(BUFFER[BUFFER_NUM-1]*sizeof(buf_t));
  TEST_ASSERT(buffer_in != NULL);
  
  /* Initialize with callback, power on and configure SPI bus */
  TEST_ASSERT(drv->Initialize(SPI_DrvEvent) == ARM_DRIVER_OK);   
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(SPI_DATA_BITS) |
    ARM_SPI_MSB_LSB | ARM_SPI_SS_MASTER_UNUSED, SPI_BR[0]*1000) == ARM_DRIVER_OK);

  /* Set Local Loopback */
  SPI_LOCAL_LOOPBACK();
  
  /* Set output buffer pattern*/
  for (cnt = 0; cnt<BUFFER[BUFFER_NUM-1];) {  
    for (i = 0; i<ARRAY_SIZE(pattern);) {
      buffer_out[cnt] = pattern[i++];
      if (SPI_DATA_BITS>8U) {
        buffer_out[cnt] |= pattern[i++]<<8U;
      }
      if (SPI_DATA_BITS>16U) {
        buffer_out[cnt] |= pattern[i++]<<16U;
        buffer_out[cnt] |= pattern[i++]<<24U;
      }
      buffer_out[cnt++] &= (1U<<SPI_DATA_BITS)-1U;      
    }
  } 

  /* Transfer data chunks */
  for (cnt = 0; cnt<BUFFER_NUM; cnt++) {      
    /* Clear input buffer */
    memset(buffer_in,0,BUFFER[cnt]);    
    if (SPI_RunTransfer(buffer_out, buffer_in, BUFFER[cnt]) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fail to transfer block of %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();     
    if (memcmp(buffer_in, buffer_out, BUFFER[cnt])!=0) {
      snprintf(str,sizeof(str),"[FAILED] Fail to check block of %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();     
  } 
  
  /* Set output buffer with random data */
  srand(GET_SYSTICK());
  for (cnt = 0; cnt<BUFFER[BUFFER_NUM-1]; cnt++) {  
    buffer_out[cnt] = (buf_t)rand()&((1U<<SPI_DATA_BITS)-1U);
  } 

  /* Transfer data chunks */
  for (cnt = 0; cnt<BUFFER_NUM; cnt++) {      
    /* Clear input buffer */
    memset(buffer_in,0,BUFFER[cnt]);    
    if (SPI_RunTransfer(buffer_out, buffer_in, BUFFER[cnt]) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fail to transfer block of %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();     
    if (memcmp(buffer_in, buffer_out, BUFFER[cnt])!=0) {
      snprintf(str,sizeof(str),"[FAILED] Fail to check block of %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();     
  } 
  
  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Free buffer */
  free(buffer_out);   
  free(buffer_in);     
}

/**
@}
*/ 
// end of group spi_funcs
