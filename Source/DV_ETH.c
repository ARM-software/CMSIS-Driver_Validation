/*-----------------------------------------------------------------------------
 *      Name:         DV_Ethernet.c
 *      Purpose:      Ethernet test cases
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#include "cmsis_dv.h" 
#include "DV_Config.h"
#include "DV_Framework.h"
#include "Driver_ETH_MAC.h"
#include "Driver_ETH_PHY.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

// Ethernet PTP time definitions 
#define PTP_S_NS         1000000000U 
#define PTP_TIME_REF     ETH_PTP_TIME_REF
#define PTP_TIME_REF_NS  ETH_PTP_TIME_REF*1000000U 

// Ethernet buffer pointers
static uint8_t *buffer_out; 
static uint8_t *buffer_in; 

// Register Driver_ETH_MAC# Driver_ETH_PHY#
extern ARM_DRIVER_ETH_MAC CREATE_SYMBOL(Driver_ETH_MAC, DRV_ETH);
extern ARM_DRIVER_ETH_PHY CREATE_SYMBOL(Driver_ETH_PHY, DRV_ETH);
static ARM_DRIVER_ETH_MAC* eth_mac = &CREATE_SYMBOL(Driver_ETH_MAC, DRV_ETH);
static ARM_DRIVER_ETH_PHY* eth_phy = &CREATE_SYMBOL(Driver_ETH_PHY, DRV_ETH);
static ARM_ETH_MAC_CAPABILITIES capab;  

static char str[128];

// Event flags
static uint8_t volatile Event; 

// Ethernet event
static void ETH_DrvEvent (uint32_t event) {
  Event |= event;
}

// Ethernet transfer
int8_t ETH_RunTransfer (uint8_t *out, uint8_t *in, uint32_t cnt);
int8_t ETH_RunTransfer (uint8_t *out, uint8_t *in, uint32_t cnt) {
  uint32_t tick;
  
  Event &= ~ARM_ETH_MAC_EVENT_RX_FRAME;     

  eth_mac->SendFrame (out, cnt, 0);
    
  tick = GET_SYSTICK();
  do {
    if ((Event & ARM_ETH_MAC_EVENT_RX_FRAME) || (eth_mac->GetRxFrameSize() != 0)) {
      eth_mac->ReadFrame (in, BUFFER[BUFFER_NUM-1]);     
      return 0;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(ETH_TRANSFER_TIMEOUT));
  
  return -1;
}


/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup eth_funcs Ethernet Validation
\brief Ethernet test cases
\details
The Ethernet validation test performs the following checks:
- API interface compliance.
- Data communication with various transfer sizes and communication parameters.
- Loopback communication.

\anchor eth_loopback
Loopback Communication Setup
----------------------------

To perform loopback communication tests, it is required to connect the RX and TX lines of the Ethernet cable together:

- TX+ (Pin 1) with RX+ (Pin 3) and
- TX- (Pin 2) with RX- (Pin 6)

\image html ethernet_loopback.png

Various \b Ethernet \b loopback \b plugs are available from different vendors that fulfill this purpose.

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_MAC_GetCapabilities
\details
The test case \b ETH_MAC_GetCapabilities verifies the Ethernet MAC function \b GetCapabilities.
*/
void ETH_MAC_GetCapabilities (void) {                    
  /* Get ETH_MAC capabilities */
  capab = eth_mac->GetCapabilities();
  TEST_ASSERT(&capab != NULL); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_MAC_Initialization
\details
The test case \b ETH_MAC_Initialization verifies the Ethernet MAC functions in the following order:
  - \b Initialize  without callback
  - \b Uninitialize
  - \b Initialize with callback if supported
  - \b Uninitialize
*/
void ETH_MAC_Initialization (void) { 

  /* Initialize without callback */
  TEST_ASSERT(eth_mac->Initialize(NULL) == ARM_DRIVER_OK); 
    
  /* Uninitialize */
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Initialize with callback if supported */
  TEST_ASSERT(eth_mac->Initialize((capab.event_rx_frame) ? ETH_DrvEvent : NULL) == ARM_DRIVER_OK); 
  
  /* Uninitialize */
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_MAC_CheckInvalidInit
\details
The test case \b ETH_MAC_CheckInvalidInit verifies the driver behaviour when receiving an invalid initialization sequence:
  - \b Uninitialize
  - \b PowerControl with Power off
  - \b PowerControl with Power on
  - \b Control 
  - \b PowerControl with Power off
  - \b Uninitialize
*/
void ETH_MAC_CheckInvalidInit (void) { 

  /* Uninitialize */
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 

  /* Power off */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Try to power on */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) != ARM_DRIVER_OK); 
  
  /* Try to set configuration */
  TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONFIGURE, ARM_ETH_SPEED_100M  | ARM_ETH_MAC_DUPLEX_FULL | 
    ARM_ETH_MAC_ADDRESS_BROADCAST | ARM_ETH_MAC_ADDRESS_ALL )!= ARM_DRIVER_OK);

  /* Power off */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Uninitialize */
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_MAC_PowerControl
\details
The test case \b ETH_MAC_PowerControl verifies the Ethernet MAC \b PowerControl function with the sequence:
  - Initialize 
  - Power on
  - Set bus speed \token{10M}
  - Set bus speed \token{100M}
  - Set bus speed \token{1G} 
  - Power off
  - Uninitialize
*/
void ETH_MAC_PowerControl (void) { 
  int32_t val;
  
  /* Initialize with callback if supported */
  TEST_ASSERT(eth_mac->Initialize((eth_mac->GetCapabilities().event_rx_frame) ? ETH_DrvEvent : NULL) == ARM_DRIVER_OK); 
  
  /* Power on */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);  
  
  /* Power low */
  val = eth_mac->PowerControl (ARM_POWER_LOW);
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Low power is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
   
  /* Power off */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Uninitialize */
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_MAC_SetBusSpeed
\details
The test case \b ETH_MAC_SetBusSpeed verifies the Ethernet MAC \b Control function with the sequence:
  - Initialize 
  - Power on
  - Set bus speed \token{10M}
  - Set bus speed \token{100M}
  - Set bus speed \token{1G}
  - Power off
  - Uninitialize
*/
void ETH_MAC_SetBusSpeed (void) { 
  int32_t val;  
  
  /* Initialize with callback if supported and power on*/
  TEST_ASSERT(eth_mac->Initialize((eth_mac->GetCapabilities().event_rx_frame) ? ETH_DrvEvent : NULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);  
  
  /* Set bus speed 10M */
  val = eth_mac->Control (ARM_ETH_MAC_CONFIGURE, ARM_ETH_SPEED_10M);
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Link speed 10M is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  /* Set bus speed 100M */
  val = eth_mac->Control (ARM_ETH_MAC_CONFIGURE, ARM_ETH_SPEED_100M);
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Link speed 100M is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  /* Set bus speed 1G */
  val = eth_mac->Control (ARM_ETH_MAC_CONFIGURE, ARM_ETH_SPEED_1G);
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Link speed 1G is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); } 
  
  /* Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_MAC_Config_Mode
\details
The test case \b ETH_MAC_Config_Mode verifies the Ethernet MAC \b Control function with the sequence:
  - Initialize 
  - Power on
  - Set full duplex
  - Set half duplex 
  - Power off
  - Uninitialize
*/
void ETH_MAC_Config_Mode (void) { 
  
  /* Initialize with callback if supported and power on*/
  TEST_ASSERT(eth_mac->Initialize((eth_mac->GetCapabilities().event_rx_frame) ? ETH_DrvEvent : NULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Set full duplex */
  TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_DUPLEX_FULL)== ARM_DRIVER_OK);
  
  /* Set half duplex */
  TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_DUPLEX_HALF)== ARM_DRIVER_OK);
  
  /* Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_MAC_Config_CommonParams
\details
The test case \b ETH_MAC_Config_CommonParams verifies the Ethernet MAC \b Control function with the sequence:
  - Initialize 
  - Power on
  - Configure Ethernet MAC bus
  - Configure receiver
  - Configure transmitter
  - Power off
  - Uninitialize
*/
void ETH_MAC_Config_CommonParams (void) { 
  
  /* Initialize with callback if supported and power on*/
  TEST_ASSERT(eth_mac->Initialize((eth_mac->GetCapabilities().event_rx_frame) ? ETH_DrvEvent : NULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Configure ETH_MAC bus*/
  TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONFIGURE, ARM_ETH_SPEED_100M  | ARM_ETH_MAC_DUPLEX_FULL | 
    ARM_ETH_MAC_ADDRESS_BROADCAST | ARM_ETH_MAC_ADDRESS_ALL | ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK);
 
  TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONTROL_RX, 1) == ARM_DRIVER_OK);

  TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONTROL_TX, 1) == ARM_DRIVER_OK);
  
  /* Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_PHY_Initialization
\details
The test case \b ETH_PHY_Initialization verifies the Ethernet PHY functions in the following order:
  - \b Initialize with read and write functions
*/
void ETH_PHY_Initialization (void) { 
  
  /* MAC Initialize and power on */
  TEST_ASSERT(eth_mac->Initialize((eth_mac->GetCapabilities().event_rx_frame) ? ETH_DrvEvent : NULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 

  /* Initialize */
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK); 
  
  /* Uninitialize */
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK); 
  
  /* MAC Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_PHY_CheckInvalidInit
\details
The test case \b ETH_PHY_CheckInvalidInit verifies the driver behaviour when receiving an invalid initialization sequence:
  - \b Uninitialize
  - \b PowerControl with Power off
  - \b PowerControl with Power on
  - \b SetInterface to configure the Ehternet PHY bus
  - \b SetMode to configure the Ehternet PHY bus
  - \b PowerControl with Power off
  - \b Uninitialize
*/
void ETH_PHY_CheckInvalidInit (void) { 
    
  /* Uninitialize */
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Power off */
  TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Try to power on*/
  TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_FULL) != ARM_DRIVER_OK); 
  
  /* Try to configure ETH_PHY bus*/
  TEST_ASSERT(eth_phy->SetInterface (capab.media_interface) != ARM_DRIVER_OK);  
  TEST_ASSERT(eth_phy->SetMode (ARM_ETH_PHY_AUTO_NEGOTIATE) != ARM_DRIVER_OK);
    
  /* Try to initialize without read and write functions */
  TEST_ASSERT(eth_phy->Initialize(NULL, NULL) != ARM_DRIVER_OK); 
  
  /* Power off */
  TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Uninitialize */
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_PHY_PowerControl
\details
The test case \b ETH_PHY_PowerControl verifies the Ethernet PHY \b PowerControl function with the sequence:
  - Initialize
  - Power on
  - Power low
  - Power off
  - Uninitialize
*/
void ETH_PHY_PowerControl (void) { 
  int32_t val;
  
  /* MAC Initialize and power on */
  TEST_ASSERT(eth_mac->Initialize((eth_mac->GetCapabilities().event_rx_frame) ? ETH_DrvEvent : NULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Initialize */
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK); 
  
  /* Power on */
  TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);  
  
  /* Power low */
  val = eth_phy->PowerControl (ARM_POWER_LOW);
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Low power is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }
   
  /* Power off */
  TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  
  /* Uninitialize */
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK); 
  
  /* MAC Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: ETH_PHY_Config
\details
The test case \b ETH_PHY_Config verifies the PHY functions 
  - Initialize
  - Power on
  - SetInterface 
  - SetMode 
  - Power off
  - Uninitialize
*/
void ETH_PHY_Config (void) { 
  
  /* MAC Initialize and power on*/
  TEST_ASSERT(eth_mac->Initialize((eth_mac->GetCapabilities().event_rx_frame) ? ETH_DrvEvent : NULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Initialize and power on*/
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  
  /* Configure ETH_PHY bus*/
  TEST_ASSERT(eth_phy->SetInterface (capab.media_interface) == ARM_DRIVER_OK);
  
  TEST_ASSERT(eth_phy->SetMode (ARM_ETH_PHY_AUTO_NEGOTIATE) == ARM_DRIVER_OK);
  
  /* Power off and uninitialize */
  TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK); 
  
  /* MAC Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: ETH_Loopback_Transfer
\details
The test case \b ETH_Loopback_Transfer verifies data transfer via Ehernet with the following sequence:
 - Buffer allocation
 - Initialize
 - Power on
 - Ethernet connection
 - Set output buffer pattern
 - Transfer data chunks
 - Set output buffer with random data
 - Transfer data chunks
 - Power off
 - Uninitialize
*/
void ETH_Loopback_Transfer (void) { 
  uint16_t cnt, i; 
  uint8_t pattern[] = BUFFER_PATTERN;
  uint32_t tick;
  
  /* Allocate buffer */
  buffer_out = (uint8_t*) malloc(BUFFER[BUFFER_NUM-1]*sizeof(uint8_t));
  TEST_ASSERT(buffer_out != NULL);
  buffer_in = (uint8_t*) malloc(BUFFER[BUFFER_NUM-1]*sizeof(uint8_t));
  TEST_ASSERT(buffer_in != NULL);
  
  /* Initialize, power on and configure MAC and PHY */
  TEST_ASSERT(eth_mac->Initialize((eth_mac->GetCapabilities().event_rx_frame) ? ETH_DrvEvent : NULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONFIGURE, ARM_ETH_SPEED_100M  | ARM_ETH_MAC_DUPLEX_FULL | 
    ARM_ETH_MAC_ADDRESS_BROADCAST | ARM_ETH_MAC_ADDRESS_ALL | ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONTROL_RX, 1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONTROL_TX, 1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_phy->SetInterface (capab.media_interface) == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_phy->SetMode (ARM_ETH_PHY_AUTO_NEGOTIATE) == ARM_DRIVER_OK);
  
  /* Check Ethernet link*/
  tick = GET_SYSTICK();
  while (eth_phy->GetLinkState() != ARM_ETH_LINK_UP) {
    if ((GET_SYSTICK() - tick) >= SYSTICK_MICROSEC(ETH_LINK_TIMEOUT)) {
      TEST_FAIL_MESSAGE("[FAILED] Link is broken, connect Ethernet cable");
      break;
    }
  }   
  
  /* Set output buffer pattern*/
  for (cnt = 0; cnt<BUFFER[BUFFER_NUM-1];) {  
    for (i = 0; i<ARRAY_SIZE(pattern); i++) {
      buffer_out[cnt++] = pattern[i];
    }
  } 
 
  /* Transfer data chunks */
  for (cnt = 0; cnt<BUFFER_NUM; cnt++) {      
    /* Clear input buffer*/
    memset(buffer_in,0,BUFFER[cnt]);    
    if (ETH_RunTransfer(buffer_out, buffer_in, BUFFER[cnt]) != ARM_DRIVER_OK) {
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
    buffer_out[cnt] = (uint8_t)rand();
  }   
  
  /* Transfer data chunks */
  for (cnt = 0; cnt<BUFFER_NUM; cnt++) {      
    /* Clear input buffer*/
    memset(buffer_in,0,BUFFER[cnt]);    
    if (ETH_RunTransfer(buffer_out, buffer_in, BUFFER[cnt]) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fail to transfer block of %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();     
    if (memcmp(buffer_in, buffer_out, BUFFER[cnt])!=0) {
      snprintf(str,sizeof(str),"[FAILED] Fail to check block of %d bytes",BUFFER[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();     
  } 
  
  /* Power off and uninitialize */
  TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK); 
  TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);  
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
  
  /* Free buffer */
  free(buffer_out);   
  free(buffer_in); 
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_MAC_PTP_ControlTimer
\details
The test case \b ETH_MAC_PTP_ControlTimer verifies the PTP ControlTimer function with the sequence:
  - Initialize 
  - Power on
  - Set Time
  - Adjust Clock
  - Increment Time
  - Decrement Time
  - Set Alarm
  - Power off
  - Uninitialize
*/
void ETH_MAC_PTP_ControlTimer (void) { 
  ARM_ETH_MAC_TIME time1, time2;
  int64_t t1ns, t2ns, t, overhead;
  double rate;
    
  /* Get capabilities */
  if (!eth_mac->GetCapabilities().precision_timer) { 
    TEST_MESSAGE("[WARNING] Precision Time Protocol is not supported");    
  } else {    
    /* Initialize, power on and configure MAC */
    TEST_ASSERT(eth_mac->Initialize(ETH_DrvEvent) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);  
    
    /* Set Time -------------------------------------------------------------------------------- */
    time1.sec = 0U;
    time1.ns = 0U;
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_SET_TIME, &time1) == ARM_DRIVER_OK);   
        
    /* Check System Time */
    osDelay(1U);
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK);  
    osDelay(PTP_TIME_REF);
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK); 
    
    /* Get timestamps in nanoseconds */
    t1ns = (int64_t)time1.sec*PTP_S_NS + time1.ns;
    t2ns = (int64_t)time2.sec*PTP_S_NS + time2.ns;
    t = t2ns - t1ns - PTP_TIME_REF_NS;

    /* Check timestamps difference */       
    if (llabs(t)>ETH_PTP_TOLERANCE) {
      snprintf(str,sizeof(str),"[WARNING] PTP measured time is %lldns from expected", t);
      TEST_MESSAGE(str);
    } else TEST_PASS();     
    
    /* Adjust clock - Set correction factor ---------------------------------------------------- */
    /* Calculate rate and convert it to q31 format */
    rate = (double)PTP_TIME_REF_NS/(t2ns-t1ns);    
    time1.ns = (uint32_t)(0x80000000U*rate);
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_ADJUST_CLOCK, &time1) == ARM_DRIVER_OK); 
    
    /* Check System Time after adjusting clock */
    osDelay(1U);
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK);  
    osDelay(PTP_TIME_REF);
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK); 
    
    /* Get timestamps in nanoseconds */
    t1ns = (int64_t)time1.sec*PTP_S_NS + time1.ns;
    t2ns = (int64_t)time2.sec*PTP_S_NS + time2.ns; 
    t = t2ns - t1ns - PTP_TIME_REF_NS;
       
    /* Check timestamps difference */       
    if (llabs(t)>ETH_PTP_TOLERANCE) {
      snprintf(str,sizeof(str),"[WARNING] PTP measured time with adj clk is %lldns from expected", t);
      TEST_MESSAGE(str);
    } else TEST_PASS();  
    
    /* Measure time overhead for increment/decrement calls ------------------------------------- */
    time2.sec = 0;
    time2.ns = 0;
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK);  
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_INC_TIME, &time2) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK); 
    t1ns = (int64_t)time1.sec*PTP_S_NS + time1.ns;
    t2ns = (int64_t)time2.sec*PTP_S_NS + time2.ns;
    overhead = t2ns - t1ns;
    
    /* Increment time -------------------------------------------------------------------------- */
    time2.sec = PTP_TIME_REF/1000U;
    time2.ns = (PTP_TIME_REF-time2.sec*1000U)*1000000U;
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK);  
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_INC_TIME, &time2) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK);    

    /* Get timestamps in nanoseconds */
    t1ns = (int64_t)time1.sec*PTP_S_NS + time1.ns;
    t2ns = (int64_t)time2.sec*PTP_S_NS + time2.ns;
    t = t2ns - t1ns - PTP_TIME_REF_NS - overhead;
    
    /* Check timestamps difference */    
    if (llabs(t)>ETH_PTP_TOLERANCE) {
      snprintf(str,sizeof(str),"[WARNING] PTP incremented time is %lldns from expected", t);
      TEST_MESSAGE(str);
    } else TEST_PASS();   
    
    /* Decrement time -------------------------------------------------------------------------- */
    time2.sec = PTP_TIME_REF/1000U;
    time2.ns = (PTP_TIME_REF-time2.sec*1000U)*1000000U;
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK);  
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_DEC_TIME, &time2) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK);    

    /* Get timestamps in nanoseconds */
    t1ns = (int64_t)time1.sec*PTP_S_NS + time1.ns;
    t2ns = (int64_t)time2.sec*PTP_S_NS + time2.ns;
    t = t2ns - t1ns + PTP_TIME_REF_NS - overhead;
        
    /* Check timestamps difference */    
    if (llabs(t)>ETH_PTP_TOLERANCE) {
      snprintf(str,sizeof(str),"[WARNING] PTP decremented time is %lldns from expected", t);
      TEST_MESSAGE(str);
    } else TEST_PASS();     
    
    /* Set Alarm (1s) -------------------------------------------------------------------------- */
    Event &= ~ARM_ETH_MAC_EVENT_TIMER_ALARM;
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK); 
    time1.sec += 1U;
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_SET_ALARM, &time1) == ARM_DRIVER_OK); 
    
    /* Check alarm event after 999ms */
    osDelay(999U);
    if ((Event & ARM_ETH_MAC_EVENT_TIMER_ALARM) != 0)  {
      TEST_FAIL_MESSAGE("[FAILED] PTP Alarm event triggered too early");
    } else TEST_PASS(); 
    
    /* Check alarm event after 1001ms */
    osDelay(2U);
    if ((Event & ARM_ETH_MAC_EVENT_TIMER_ALARM) == 0)  {
      TEST_FAIL_MESSAGE("[FAILED] PTP Alarm event timeout");
    } else TEST_PASS();   

    /* Power off and uninitialize */
    TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);  
    TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: ETH_Loopback_PTP
\details
The test case \b ETH_Loopback_PTP verifies the Precision Time Protocol functions ControlTimer, GetRxFrameTime and 
GetTxFrameTime with the sequence:
  - Initialize 
  - Power on
  - Set Control Timer
  - Transfer a frame
  - Get TX frame time
  - Get RX frame time
  - Power off
  - Uninitialize
*/
void ETH_Loopback_PTP (void) { 
  ARM_ETH_MAC_TIME time1, time2;
  uint32_t tick;
   
  // PTP over Ethernet IPv4 sample frame
  const uint8_t PTP_frame[] = {   
   0x01, 0x00, 0x5e, 0x00, 0x01, 0x81, 0x00, 0x30, 0x05, 0x1d, 0x1e, 0x27, 0x08, 0x00, 0x45, 0x00,
   0x00, 0x98, 0x00, 0x5d, 0x40, 0x00, 0x01, 0x11, 0x29, 0x68, 0x0a, 0x0a, 0x64, 0x05, 0xe0, 0x00,
   0x01, 0x81, 0x01, 0x3f, 0x01, 0x3f, 0x00, 0x84, 0xc0, 0x7b, 0x00, 0x01, 0x00, 0x01, 0x5f, 0x44,
   0x46, 0x4c, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
   0x00, 0x30, 0x05, 0x1d, 0x1e, 0x27, 0x00, 0x01, 0x00, 0x5e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
   0x00, 0x00, 0x45, 0x5b, 0x0a, 0x38, 0x0e, 0xb9, 0x26, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x00, 0x30, 0x05, 0x1d, 0x1e, 0x27, 0x00, 0x00, 0x00, 0x5e, 0x00, 0x00, 0x00, 0x04, 0x44, 0x46,
   0x4c, 0x54, 0x00, 0x00, 0xf0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
   0xf0, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x44, 0x46, 0x4c, 0x54, 0x00, 0x01,
   0x00, 0x30, 0x05, 0x1d, 0x1e, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00                               
  };
  const uint32_t PTP_frame_len = sizeof(PTP_frame);    
    
  /* Get capabilities */
  if (!eth_mac->GetCapabilities().precision_timer) { 
    TEST_MESSAGE("[WARNING] Precision Time Protocol is not supported");
  } else {    
    /* Allocate buffer */
    buffer_in = (uint8_t*) malloc(PTP_frame_len*sizeof(uint8_t));
    TEST_ASSERT(buffer_in != NULL);
    
    /* Initialize, power on and configure MAC and PHY */
    TEST_ASSERT(eth_mac->Initialize((eth_mac->GetCapabilities().event_rx_frame) ? ETH_DrvEvent : NULL) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONFIGURE, ARM_ETH_SPEED_100M  | ARM_ETH_MAC_DUPLEX_FULL | 
      ARM_ETH_MAC_ADDRESS_BROADCAST | ARM_ETH_MAC_ADDRESS_ALL | ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONTROL_RX, 1) == ARM_DRIVER_OK);
    TEST_ASSERT(eth_mac->Control (ARM_ETH_MAC_CONTROL_TX, 1) == ARM_DRIVER_OK);
    TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_phy->SetInterface (capab.media_interface) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_phy->SetMode (ARM_ETH_PHY_AUTO_NEGOTIATE) == ARM_DRIVER_OK); 
    
    /* Set Time */
    time1.sec = 0U;
    time1.ns = 0U;
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_SET_TIME, &time1) == ARM_DRIVER_OK); 
    
    /* Check timestamps - verify if control timer is running */
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK);   
    TEST_ASSERT((time2.sec==time1.sec)?(time2.ns>time1.ns):(time2.sec>time1.sec)); 
    
    /* Check Ethernet link */
    tick = GET_SYSTICK();
    while (eth_phy->GetLinkState() != ARM_ETH_LINK_UP) {
      if ((GET_SYSTICK() - tick) >= SYSTICK_MICROSEC(ETH_LINK_TIMEOUT)) {
        TEST_FAIL_MESSAGE("[FAILED] Link is broken, connect Ethernet cable");
        break;
      }
    }           
    
    /* Transfer frame */ 
    TEST_ASSERT(eth_mac->SendFrame (PTP_frame, PTP_frame_len, ARM_ETH_MAC_TX_FRAME_TIMESTAMP) == ARM_DRIVER_OK);    
    tick = GET_SYSTICK();
    while (eth_mac->GetRxFrameSize() == 0) {
      if ((GET_SYSTICK() - tick) >= SYSTICK_MICROSEC(ETH_TRANSFER_TIMEOUT)) {
        TEST_FAIL_MESSAGE("[FAILED] Transfer timeout");
        break;
      }
    }
    
    /* Get TX Frame Time */
    TEST_ASSERT(eth_mac->GetTxFrameTime(&time1) == ARM_DRIVER_OK); 
    
    /* Get RX Frame Time */
    TEST_ASSERT(eth_mac->GetRxFrameTime(&time2) == ARM_DRIVER_OK); 
    
    /* Check timestamps */
    TEST_ASSERT((time2.sec==time1.sec)?(time2.ns>time1.ns):(time2.sec>time1.sec)); 
    
    /* Check frame */
    eth_mac->ReadFrame(buffer_in, PTP_frame_len);      
    TEST_ASSERT(memcmp(buffer_in, PTP_frame, PTP_frame_len) == 0);
  
    /* Power off and uninitialize */
    TEST_ASSERT(eth_phy->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
    TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK); 
    TEST_ASSERT(eth_mac->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);  
    TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK); 
    
    /* Free buffer */
    free(buffer_in); 
  }
}


/**
@}
*/ 
// end of group eth_funcs
