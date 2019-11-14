/*-----------------------------------------------------------------------------
 *      Name:         DV_USART.c
 *      Purpose:      USART test cases
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#include "cmsis_dv.h" 
#include "DV_Config.h"
#include "DV_Framework.h"
#include "Driver_USART.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

// USART buffer type
#if (USART_DATA_BITS>8U)
typedef uint16_t buf_t;
#else
typedef uint8_t buf_t;
#endif

// USART buffer pointers
static buf_t *buffer_out; 
static buf_t *buffer_in; 

// USART data bits
#if   (USART_DATA_BITS==5U)
#define USART_DATA_BITS_CTRL_CODE ARM_USART_DATA_BITS_5
#elif (USART_DATA_BITS==6U)
#define USART_DATA_BITS_CTRL_CODE ARM_USART_DATA_BITS_6
#elif (USART_DATA_BITS==7U)
#define USART_DATA_BITS_CTRL_CODE ARM_USART_DATA_BITS_7
#elif (USART_DATA_BITS==8U)
#define USART_DATA_BITS_CTRL_CODE ARM_USART_DATA_BITS_8
#elif (USART_DATA_BITS==9U)
#define USART_DATA_BITS_CTRL_CODE ARM_USART_DATA_BITS_9
#endif

// USART baudrates
static const uint32_t USART_BR[] = {
#if (USART_BR_1>0)
  USART_BR_1,
#endif  
#if (USART_BR_2>0)
  USART_BR_2,
#endif 
#if (USART_BR_3>0)
  USART_BR_3,
#endif 
#if (USART_BR_4>0)
  USART_BR_4,
#endif 
#if (USART_BR_5>0)
  USART_BR_5,
#endif 
#if (USART_BR_6>0)
  USART_BR_6,
#endif 
};
static const uint32_t USART_BR_NUM = ARRAY_SIZE(USART_BR);

// Register Driver_USART#
extern ARM_DRIVER_USART CREATE_SYMBOL(Driver_USART, DRV_USART);
static ARM_DRIVER_USART *drv = &CREATE_SYMBOL(Driver_USART, DRV_USART);
static ARM_USART_CAPABILITIES capab;   

static char str[128];

// Event flags
static uint8_t volatile Event;  

// USART event
static void USART_DrvEvent (uint32_t event) {
  Event |= event;
}

// USART asynchronous transfer
int8_t USART_RunTransfer (void *out, void *in, uint32_t cnt);
int8_t USART_RunTransfer (void *out, void *in, uint32_t cnt) {
  uint32_t tick;
  
  Event &= ~ARM_USART_EVENT_RECEIVE_COMPLETE;
      
  drv->Receive (in, cnt);
  drv->Send (out, cnt);
    
  tick = GET_SYSTICK();
  do {
    if (Event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
      return 0;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(USART_TRANSFER_TIMEOUT));

  drv->Control(ARM_USART_ABORT_TRANSFER, 0);
  return -1;
}

// USART send with callback
int8_t USART_RunSend (void *out, uint32_t cnt);
int8_t USART_RunSend (void *out, uint32_t cnt) {
  uint32_t tick;
  
  Event &= ~ARM_USART_EVENT_SEND_COMPLETE;
      
  drv->Send (out, cnt);
    
  tick = GET_SYSTICK();
  do {
    if (Event & ARM_USART_EVENT_SEND_COMPLETE) {
      return 0;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(USART_TRANSFER_TIMEOUT));

  drv->Control(ARM_USART_ABORT_SEND, 0);
  return -1;
}

// USART send without callback
int8_t USART_RunSend_NoCallback (void *out, uint32_t cnt);
int8_t USART_RunSend_NoCallback (void *out, uint32_t cnt) {
  uint32_t tick;
        
  drv->Send (out, cnt);
    
  tick = GET_SYSTICK();
  do {
    if (drv->GetTxCount() == cnt) {
      return 0;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(USART_TRANSFER_TIMEOUT));

  drv->Control(ARM_USART_ABORT_SEND, 0);
  return -1;
}

// USART receive with callback
int8_t USART_RunReceive (void *in, uint32_t cnt);
int8_t USART_RunReceive (void *in, uint32_t cnt) {
  uint32_t tick;
  
  Event &= ~ARM_USART_EVENT_RECEIVE_COMPLETE;
      
  drv->Receive (in, cnt);
    
  tick = GET_SYSTICK();
  do {
    if (Event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
      return 0;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(USART_TRANSFER_TIMEOUT));

  drv->Control(ARM_USART_ABORT_RECEIVE, 0);
  return -1;
}

// USART receive without callback
int8_t USART_RunReceive_NoCallback (void *in, uint32_t cnt);
int8_t USART_RunReceive_NoCallback (void *in, uint32_t cnt) {
  uint32_t tick;
        
  drv->Receive (in, cnt);
    
  tick = GET_SYSTICK();
  do {
    if (drv->GetRxCount() == cnt) {
      return 0;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(USART_TRANSFER_TIMEOUT));

  drv->Control(ARM_USART_ABORT_RECEIVE, 0);
  return -1;
}


/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

 
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup usart_funcs USART Validation
\brief USART test cases
\details
The USART validation test performs the following checks:
- API interface compliance.
- Data communication with various transfer sizes and communication parameters.
- Transfer speed of the data communication.
- Loopback communication.

\anchor usart_loop_back_setup
Loopback Communication Setup
----------------------------

To perform loopback communication tests, it is required to connect the USART's \b TX signal to the \b RX signal (refer to the
schematics of your target hardware for detailed pinout information).
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: USART_GetCapabilities
\details
The test case \b USART_GetCapabilities verifies the function \b GetCapabilities.
*/
void USART_GetCapabilities (void) {
  /* Get USART capabilities */
  capab = drv->GetCapabilities();
  TEST_ASSERT(&capab != NULL); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_Initialization
\details
The test case \b USART_Initialization verifies the USART functions with the sequence:
  - \b Initialize without callback
  - \b Uninitialize
  - \b Initialize with callback
  - \b Uninitialize
*/
void  USART_Initialization (void) {
  
  /* Initialize without callback */
  TEST_ASSERT(drv->Initialize(NULL) == ARM_DRIVER_OK); 
    
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 

  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_CheckInvalidInit
\details
The test case \b USART_CheckInvalidInit verifies the driver behaviour when receiving an invalid initialization sequence:
  - \b Uninitialize
  - \b PowerControl with Power off
  - \b PowerControl with Power on
  - \b Control 
  - \b PowerControl with Power off
  - \b Uninitialize
*/
void  USART_CheckInvalidInit (void) {
  
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Try to power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) != ARM_DRIVER_OK); 
  
  /* Try to set configuration */
  TEST_ASSERT(drv->Control (ARM_USART_MODE_ASYNCHRONOUS | USART_DATA_BITS_CTRL_CODE | ARM_USART_PARITY_NONE | 
    ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE, USART_BR[0]) != ARM_DRIVER_OK);
  
  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
 
  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_PowerControl
\details
The test case \b USART_PowerControl verifies the \b PowerControl function with the sequence:
 - Initialize with callback
 - Power on
 - Power low
 - Power off
 - Uninitialize
*/
void  USART_PowerControl (void) {
  int32_t val;
  
  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  
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
\brief  Test case: USART_Config_PolarityPhase
\details
The test case \b USART_Config_PolarityPhase verifies the \b Control function with the sequence:
 - Initialize with callback
 - Power on
 - Set basic SPI bus configuration
 - Change polarity
 - Change phase 
 - Change polarity and phase
 - Power off
 - Uninitialize
*/
void USART_Config_PolarityPhase (void) { 
  
  /* Initialize with callback and power on */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);  
  
  /* Set basic SPI bus configuration*/
  TEST_ASSERT(drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_CPOL0 | ARM_USART_CPHA0, USART_BR[0]) == ARM_DRIVER_OK);
  
  /* Change polarity */
  TEST_ASSERT(drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_CPOL1 | ARM_USART_CPHA0, USART_BR[0]) == ARM_DRIVER_OK);
  
  /* Change phase */
  TEST_ASSERT(drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_CPOL0 | ARM_USART_CPHA1, USART_BR[0]) == ARM_DRIVER_OK);
  
  /* Change polarity and phase */
  TEST_ASSERT(drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_CPOL1 | ARM_USART_CPHA1, USART_BR[0]) == ARM_DRIVER_OK);  
  
  /* Power off and uninitialize */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}
  
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_Config_DataBits
\details
The test case \b USART_Config_DataBits verifies the \b Control function with the sequence:
 - Initialize with callback
 - Power on
 - Data bits = \token{5}
 - Data bits = \token{6}
 - Data bits = \token{7}
 - Data bits = \token{8}
 - Data bits = \token{9}
 - Power off
 - Uninitialize
*/
void USART_Config_DataBits (void) { 
  int32_t val;  
  
  /* Initialize with callback and power on */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_5, USART_BR[0]);
  if (val == ARM_USART_ERROR_DATA_BITS) { TEST_MESSAGE("[WARNING] Data Bits = 5 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_6, USART_BR[0]);
  if (val == ARM_USART_ERROR_DATA_BITS) { TEST_MESSAGE("[WARNING] Data Bits = 6 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_7, USART_BR[0]);
  if (val == ARM_USART_ERROR_DATA_BITS) { TEST_MESSAGE("[WARNING] Data Bits = 7 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8, USART_BR[0]);
  if (val == ARM_USART_ERROR_DATA_BITS) { TEST_MESSAGE("[WARNING] Data Bits = 8 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_9, USART_BR[0]);
  if (val == ARM_USART_ERROR_DATA_BITS) { TEST_MESSAGE("[WARNING] Data Bits = 9 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }  
  
  /* Power off and uninitialize */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_Config_StopBits
\details
The test case \b USART_Config_StopBits verifies the \b Control function with the sequence:
 - Initialize with callback
 - Power on
 - Stop bits = \token{1}
 - Stop bits = \token{2}
 - Stop bits = \token{1.5}
 - Stop bits = \token{0.5}
 - Power off
 - Uninitialize
*/
void USART_Config_StopBits (void) { 
  int32_t val;  
  
  /* Initialize with callback and power on */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_STOP_BITS_1, USART_BR[0]);
  if (val == ARM_USART_ERROR_STOP_BITS) { TEST_MESSAGE("[WARNING] Stop Bits = 1 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_STOP_BITS_2, USART_BR[0]);
  if (val == ARM_USART_ERROR_STOP_BITS) { TEST_MESSAGE("[WARNING] Stop Bits = 2 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_STOP_BITS_1_5, USART_BR[0]);
  if (val == ARM_USART_ERROR_STOP_BITS) { TEST_MESSAGE("[WARNING] Stop Bits = 1.5 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_STOP_BITS_0_5, USART_BR[0]);
  if (val == ARM_USART_ERROR_STOP_BITS) { TEST_MESSAGE("[WARNING] Stop Bits = 0.5 are not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }   
  
  /* Power off and uninitialize */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_Config_Parity
\details
The test case \b USART_Config_Parity verifies the \b Control function with the sequence:
 - Initialize with callback
 - Power on
 - Sets parity bits: even parity
 - Sets parity bits: no parity
 - Sets parity bits: odd parity
 - Power off
 - Uninitialize
*/
void USART_Config_Parity (void) { 
  int32_t val;  
  
  /* Initialize with callback and power on */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_PARITY_EVEN, USART_BR[0]);
  if (val == ARM_USART_ERROR_PARITY) { TEST_MESSAGE("[WARNING] Even parity is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_PARITY_NONE, USART_BR[0]);
  if (val == ARM_USART_ERROR_PARITY) { TEST_MESSAGE("[WARNING] No parity is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  val = drv->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_PARITY_ODD, USART_BR[0]);
  if (val == ARM_USART_ERROR_PARITY) { TEST_MESSAGE("[WARNING] Odd parity not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  /* Power off and uninitialize */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_Config_Baudrate
\details
The test case \b USART_Config_Baudrate verifies the \b Control function and configures various baudrates with the sequence:
 - Initialize with callback
 - Power on
 - Change bus speed 
 - Power off
 - Uninitialize
 
\note This test needs to pass to be able to transfer data via the USART correctly. Usually, USART communication is set to a
certain baudrate with a defined tolerance. If the driver is not able to set the baudrate correctly, data exchange will not be
possible.
*/
void USART_Config_Baudrate (void) { 
  uint32_t speed;  

  /* Initialize with callback and power on */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);   
  
  /* Change baud rate*/
  for (speed=0; speed<USART_BR_NUM; speed++) {
    
    TEST_ASSERT (drv->Control(ARM_USART_MODE_ASYNCHRONOUS, USART_BR[speed]) 
       == ARM_DRIVER_OK);   
  }
  
  /* Power off and uninitialize */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_Loopback_CheckBaudrate
\details
The test case \b USART_Loopback_CheckBaudrate verifies the \b Control function, configures various baudrates, and measures
the transfer time with this sequence:

 - Initialize with callback
 - Power on
 - Change baud rate with specific control parameter
 - Measure transfer time
 - Power off
 - Uninitialize

 \note If this test issues errors or warnings, refer to the \ref test_results section for more information.
*/
void USART_Loopback_CheckBaudrate (void) { 
  uint16_t cnt;
  uint32_t speed;          
  uint32_t ticks_measured;
  uint32_t ticks_expected;  
  double rate;
  
  /* Allocate buffer */
  buffer_out = malloc(BUFFER_SIZE_BR*sizeof(buf_t));
  TEST_ASSERT(buffer_out != NULL);
  buffer_in = malloc(BUFFER_SIZE_BR*sizeof(buf_t));
  TEST_ASSERT(buffer_in != NULL);
  
  /* Initialize with callback, power on and configure USART bus */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_MODE_ASYNCHRONOUS | USART_DATA_BITS_CTRL_CODE | ARM_USART_PARITY_NONE | 
    ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE, USART_BR[0]) == ARM_DRIVER_OK); 
  
  /* Test baudrates */
  for (speed=0; speed<USART_BR_NUM; speed++) {
    
    /* Set output buffer with random data*/
    srand(GET_SYSTICK());
    for (cnt = 0; cnt<BUFFER_SIZE_BR; cnt++) {  
      buffer_out[cnt] = (buf_t)rand()&((1U<<USART_DATA_BITS)-1U);
    }  
    
    /* Change baud rate*/
    TEST_ASSERT(drv->Control (ARM_USART_CONTROL_TX,0) == ARM_DRIVER_OK);
    TEST_ASSERT(drv->Control (ARM_USART_CONTROL_RX,0) == ARM_DRIVER_OK);
    TEST_ASSERT(drv->Control(ARM_USART_MODE_ASYNCHRONOUS, USART_BR[speed]) 
       == ARM_DRIVER_OK);       
    TEST_ASSERT(drv->Control (ARM_USART_CONTROL_TX,1) == ARM_DRIVER_OK);
    TEST_ASSERT(drv->Control (ARM_USART_CONTROL_RX,1) == ARM_DRIVER_OK);
        
    /* Set Local Loopback */
    USART_LOCAL_LOOPBACK(); 
        
    /* Measure transfer time */
    ticks_measured = GET_SYSTICK();
    USART_RunTransfer(buffer_out, buffer_in, BUFFER_SIZE_BR);
    ticks_measured = GET_SYSTICK() - ticks_measured;  
    ticks_expected = SYSTICK_MICROSEC(BUFFER_SIZE_BR*10000/(USART_BR[speed]/100));
    ticks_expected *= USART_DATA_BITS+2;
    
    rate = (double)ticks_measured/ticks_expected;
       
    if ((rate>(1.0+(double)TOLERANCE_BR/100))||(rate<(1.0-(double)TOLERANCE_BR/100))) {
      snprintf(str,sizeof(str),"[WARNING] At %dbps: measured time is %f x expected time", USART_BR[speed], rate);
      TEST_MESSAGE(str);
    } else TEST_PASS();     
    
    /* Check received data against sent data */
    if (memcmp(buffer_in, buffer_out, BUFFER_SIZE_BR)!=0) {
      snprintf(str,sizeof(str),"[FAILED] At %dbps: fail to check block of %d bytes", USART_BR[speed], BUFFER_SIZE_BR);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();  
  }
  
  /* Power off and uninitialize */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
  
  /* Free buffer */
  free(buffer_out);   
  free(buffer_in); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_Config_CommonParams
\details
The test case \b USART_Config_CommonParams verifies the \b Control function with the sequence:
 - Initialize with callback
 - Power on
 - Configure USART bus
 - Set transmitter
 - Set receiver
 - Power off
 - Uninitialize
*/
void  USART_Config_CommonParams (void) {
  
  /* Initialize with callback and power on */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Configure USART bus*/
  TEST_ASSERT(drv->Control (ARM_USART_MODE_ASYNCHRONOUS | USART_DATA_BITS_CTRL_CODE | ARM_USART_PARITY_NONE | 
    ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE, USART_BR[0]) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_CONTROL_TX,1) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_CONTROL_RX,1) == ARM_DRIVER_OK); 
  
  /* Power off and uninitialize */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_Send
\details
The test case \b USART_Send verifies the \b Send function with the sequence:
 - Initialize with callback
 - Power on
 - Send data using callback
 - Send data without callback
 - Power off
 - Uninitialize
*/
void USART_Send (void) { 
  uint16_t cnt;
  
  /* Allocate buffer */
  buffer_out = malloc(BUFFER[BUFFER_NUM-1]*sizeof(buf_t));
  TEST_ASSERT(buffer_out != NULL);
  
  /* Initialize with callback, power on and configure USART bus */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_MODE_ASYNCHRONOUS | USART_DATA_BITS_CTRL_CODE | ARM_USART_PARITY_NONE | 
    ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE, USART_BR[0]) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_CONTROL_TX,1) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_CONTROL_RX,1) == ARM_DRIVER_OK); 
  
  /* Send data chunks */
  for (cnt = 0; cnt<BUFFER_NUM; cnt++) { 
    
    /* Send using callback */
    if (USART_RunSend(buffer_out, BUFFER[cnt]) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fail to send %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();   
    
    /* Send without callback */
    if (USART_RunSend_NoCallback(buffer_out, BUFFER[cnt]) != ARM_DRIVER_OK) {   
      snprintf(str,sizeof(str),"[FAILED] Fail to send without callback %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();    
  } 
  
  /* Power off and uninitialize */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
  
  /* Free buffer */
  free(buffer_out);    
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_AsynchronousReceive
\details
The test case \b USART_AsynchronousReceive verifies the \b Receive function with the sequence:
 - Initialize with callback
 - Power on
 - Send data using callback
 - Send data without callback
 - Power off
 - Uninitialize
*/
void USART_AsynchronousReceive (void) { 
  uint16_t cnt;
  
  /* Allocate buffer */
  buffer_in = malloc(BUFFER[BUFFER_NUM-1]*sizeof(buf_t));
  TEST_ASSERT(buffer_out != NULL);
  
  /* Initialize with callback, power on and configure USART bus */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_MODE_ASYNCHRONOUS | USART_DATA_BITS_CTRL_CODE | ARM_USART_PARITY_NONE | 
    ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE, USART_BR[0]) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_CONTROL_TX,1) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_CONTROL_RX,1) == ARM_DRIVER_OK); 
  
  /* Send data chunks */
  for (cnt = 0; cnt<BUFFER_NUM; cnt++) { 
    
    /* Send using callback */
    if (USART_RunReceive(buffer_in, BUFFER[cnt]) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fail to receive %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();   
    
    /* Send without callback */
    if (USART_RunReceive_NoCallback(buffer_out, BUFFER[cnt]) != ARM_DRIVER_OK) {   
      snprintf(str,sizeof(str),"[FAILED] Fail to send without callback %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();    
  } 
  
  /* Power off and uninitialize */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
  
  /* Free buffer */
  free(buffer_in);    
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: USART_Loopback_Transfer
\details
The test case \b USART_Loopback_Transfer verifies the \b Transfer function with the sequence:
 - Initialize with callback
 - Power on
 - Clear input buffer
 - Transfer data
 - Check received data against sent data 
 - Power off
 - Uninitialize
*/
void USART_Loopback_Transfer (void) { 
  uint16_t cnt, i;
  uint8_t pattern[] = BUFFER_PATTERN;
  
  /* Allocate buffer */
  buffer_out = malloc(BUFFER[BUFFER_NUM-1]*sizeof(buf_t));
  TEST_ASSERT(buffer_out != NULL);
  buffer_in = malloc(BUFFER[BUFFER_NUM-1]*sizeof(buf_t));
  TEST_ASSERT(buffer_in != NULL);
  
  /* Initialize with callback, power on and configure USART bus */
  TEST_ASSERT(drv->Initialize(USART_DrvEvent) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_MODE_ASYNCHRONOUS | USART_DATA_BITS_CTRL_CODE | ARM_USART_PARITY_NONE | 
    ARM_USART_STOP_BITS_1 | ARM_USART_FLOW_CONTROL_NONE, USART_BR[0]) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_CONTROL_TX,1) == ARM_DRIVER_OK); 
  TEST_ASSERT(drv->Control (ARM_USART_CONTROL_RX,1) == ARM_DRIVER_OK); 
  
  /* Set Local Loopback */
  USART_LOCAL_LOOPBACK();  
                                                
  /* Set output buffer pattern*/
  for (cnt = 0; cnt<BUFFER[BUFFER_NUM-1];) {  
    for (i = 0; i<ARRAY_SIZE(pattern);) {
      buffer_out[cnt] = pattern[i++];
      if (USART_DATA_BITS>8U) buffer_out[cnt] |= pattern[i++]<<8U;
      buffer_out[cnt++] &= (1U<<USART_DATA_BITS)-1U;  
    }
  } 

  /* Transfer data chunks */
  for (cnt = 0; cnt<BUFFER_NUM; cnt++) {      
    /* Clear input buffer*/
    memset(buffer_in,0,BUFFER[cnt]);    
    if (USART_RunTransfer(buffer_out, buffer_in, BUFFER[cnt]) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fail to transfer block of %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();     
    if (memcmp(buffer_in, buffer_out, BUFFER[cnt])!=0) {
      snprintf(str,sizeof(str),"[FAILED] Fail to check block of %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();     
  } 
  
  /* Set output buffer with random data*/
  srand(GET_SYSTICK());
  for (cnt = 0; cnt<BUFFER[BUFFER_NUM-1]; cnt++) {  
    buffer_out[cnt] = (buf_t)rand()&((1U<<USART_DATA_BITS)-1U);
  }   

  /* Transfer data chunks */
  for (cnt = 0; cnt<BUFFER_NUM; cnt++) {      
    /* Clear input buffer*/
    memset(buffer_in,0,BUFFER[cnt]);    
    if (USART_RunTransfer(buffer_out, buffer_in, BUFFER[cnt]) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fail to transfer block of %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();     
    if (memcmp(buffer_in, buffer_out, BUFFER[cnt])!=0) {
      snprintf(str,sizeof(str),"[FAILED] Fail to check block of %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();     
  } 
  
  /* Power off and uninitialize */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
  
  /* Free buffer */
  free(buffer_out);   
  free(buffer_in);     
}
  
/**
@}
*/ 
// end of group usart_funcs

  

