/*
 * Copyright (c) 2015-2023 Arm Limited. All rights reserved.
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
 * Title:       Ethernet (ETH) Driver Validation tests
 *
 * -----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmsis_dv.h"
#include "DV_ETH_Config.h"
#include "DV_Framework.h"

#include "Driver_ETH_MAC.h"
#include "Driver_ETH_PHY.h"

#ifdef BUFFER_PATTERN
#error Please update DV_ETH_Config.h
#endif

#define ETH_MTU          1500

// Ethernet PTP time definitions
#define PTP_S_NS         1000000000U
#define PTP_TIME_REF     ETH_PTP_TIME_REF
#define PTP_TIME_REF_NS  ETH_PTP_TIME_REF*1000000U

static const ARM_ETH_MAC_ADDR mac_addr     = {0x02, 0x30, 0x05, 0x1D, 0x1E, 0x27};
static const ARM_ETH_MAC_ADDR mac_bcast    = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const ARM_ETH_MAC_ADDR mac_mcast[6] ={{0x01, 0x00, 0x5E, 0x00, 0x00, 0x01},
                                             {0x01, 0x00, 0x5E, 0x00, 0x00, 0x02},
                                             {0x01, 0x00, 0x5E, 0x7F, 0xFF, 0xFF},
                                             {0x33, 0x33, 0x00, 0x00, 0x00, 0x01},
                                             {0x33, 0x33, 0x00, 0x00, 0x00, 0x02},
                                             {0x33, 0x33, 0xFF, 0xFF, 0xFF, 0xFF}};

// Register Driver_ETH_MAC# Driver_ETH_PHY#
extern ARM_DRIVER_ETH_MAC CREATE_SYMBOL(Driver_ETH_MAC, DRV_ETH);
extern ARM_DRIVER_ETH_PHY CREATE_SYMBOL(Driver_ETH_PHY, DRV_ETH);

static ARM_DRIVER_ETH_MAC       *eth_mac;
static ARM_DRIVER_ETH_PHY       *eth_phy;
static ARM_ETH_MAC_CAPABILITIES  capab;
static ARM_ETH_MAC_SignalEvent_t cb_event;

// Local variables
static volatile uint8_t phy_power;
static volatile uint8_t mac_lockup;
static char str[128];

// Allocated buffer pointers
static uint8_t *buffer_out;
static uint8_t *buffer_in;

// Event flags
static uint8_t volatile Event;

// Ethernet event
static void ETH_DrvEvent (uint32_t event) {
  Event |= event;
}

// Ethernet transfer
static int32_t ETH_RunTransfer (const uint8_t *out, uint8_t *in, uint32_t len, uint32_t frag) {
  uint32_t tick,size;

  Event &= ~ARM_ETH_MAC_EVENT_RX_FRAME;
  if (frag == 0U) {
    // Send the entire frame at once
    eth_mac->SendFrame(out, len, 0);
  }
  else {
    // Split the frame into two fragments
    eth_mac->SendFrame(out, frag, ARM_ETH_MAC_TX_FRAME_FRAGMENT);
    eth_mac->SendFrame(out+frag, len-frag, 0);
  }

  tick = GET_SYSTICK();
  do {
    // Wait for RX event or run the polling mode
    if ((Event & ARM_ETH_MAC_EVENT_RX_FRAME) || !capab.event_rx_frame) {
      size = eth_mac->GetRxFrameSize();
      if (size > 0) {
        eth_mac->ReadFrame(in, size);
        return ARM_DRIVER_OK;
      }
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(ETH_TRANSFER_TIMEOUT*1000));

  return ARM_DRIVER_ERROR;
}

// Initialize MAC driver wrapper for RMII interface
static int32_t mac_initialize (ARM_ETH_MAC_SignalEvent_t cb_event) {
  ARM_DRIVER_ETH_MAC *drv_mac = &CREATE_SYMBOL(Driver_ETH_MAC, DRV_ETH);

  phy_power = 1U;
  return drv_mac->Initialize(cb_event);
}

// Uninitialize MAC driver wrapper for RMII interface
static int32_t mac_uninitialize (void) {
  ARM_DRIVER_ETH_MAC *drv_mac = &CREATE_SYMBOL(Driver_ETH_MAC, DRV_ETH);

  phy_power = 0U;
  return drv_mac->Uninitialize();
}

// MAC driver power control wrapper for RMII interface
static int32_t mac_power_control (ARM_POWER_STATE state) {
  ARM_DRIVER_ETH_MAC *drv_mac = &CREATE_SYMBOL(Driver_ETH_MAC, DRV_ETH);
  ARM_DRIVER_ETH_PHY *drv_phy = &CREATE_SYMBOL(Driver_ETH_PHY, DRV_ETH);
  int32_t retv;

  retv = drv_mac->PowerControl(state);
  if ((state == ARM_POWER_FULL) && (retv == ARM_DRIVER_ERROR) && (phy_power == 1U)) {
    /* RMII solution when the PHY is a 50 MHz reference clock source   */
    /* MAC never exits soft reset when PHY is powered down (no 50 MHz) */
    /* So turn on the power for the PHY here to prevent deadlock       */
    drv_phy->Initialize(drv_mac->PHY_Read, drv_mac->PHY_Write);
    if (drv_phy->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK) {
      phy_power = 2U;
      osDelay (10);
      retv = drv_mac->PowerControl(state);
      if (retv == ARM_DRIVER_OK) {
        /* RMII lockup detected */
        mac_lockup = 1U;
      }
    }
  }
  return (retv);
}

// PHY driver power control wrapper for RMII interface
static int32_t phy_power_control (ARM_POWER_STATE state) {
  ARM_DRIVER_ETH_PHY *drv_phy = &CREATE_SYMBOL(Driver_ETH_PHY, DRV_ETH);
  int32_t retv;

  retv = drv_phy->PowerControl(state);
  if ((state == ARM_POWER_OFF) && (retv == ARM_DRIVER_OK) && (phy_power == 2U)) {
    phy_power = 1U;
  }
  return (retv);
}

/* Helper function that is called before tests start executing */
void ETH_DV_Initialize (void) {
  static struct _ARM_DRIVER_ETH_MAC s_mac;
  static struct _ARM_DRIVER_ETH_PHY s_phy;

  eth_mac = &CREATE_SYMBOL(Driver_ETH_MAC, DRV_ETH);
  eth_phy = &CREATE_SYMBOL(Driver_ETH_PHY, DRV_ETH);
  capab   = eth_mac->GetCapabilities();
  if (capab.media_interface == ARM_ETH_INTERFACE_RMII) {
    memcpy(&s_mac, eth_mac, sizeof(s_mac));
    memcpy(&s_phy, eth_phy, sizeof(s_phy));
    /* Use wrapper functions in RMII interface mode */
    s_mac.Initialize   = &mac_initialize;
    s_mac.Uninitialize = &mac_uninitialize;
    s_mac.PowerControl = &mac_power_control;
    s_phy.PowerControl = &phy_power_control;
    eth_mac = &s_mac;
    eth_phy = &s_phy;
    phy_power  = 0U;
    mac_lockup = 0U;
  }
  cb_event = (capab.event_rx_frame) ? ETH_DrvEvent : NULL;
}

/* Helper function that is called after tests stop executing */
void ETH_DV_Uninitialize (void) {
  eth_mac  = &CREATE_SYMBOL(Driver_ETH_MAC, DRV_ETH);
  eth_phy  = &CREATE_SYMBOL(Driver_ETH_PHY, DRV_ETH);
  cb_event = NULL;
}

/*-----------------------------------------------------------------------------
 *      Tests
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup dv_eth Ethernet Validation
\brief Ethernet driver validation
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

\defgroup eth_tests Tests
\ingroup dv_eth

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function ETH_MAC_GetVersion
\details
The function \b ETH_MAC_GetVersion verifies the Ethernet MAC \b GetVersion function.
*/
void ETH_MAC_GetVersion (void) {
  ARM_DRIVER_VERSION ver;

  ver = eth_mac->GetVersion();

  TEST_ASSERT((ver.api >= ARM_DRIVER_VERSION_MAJOR_MINOR(1UL,0UL)) && (ver.drv >= ARM_DRIVER_VERSION_MAJOR_MINOR(1UL,0UL)));

  snprintf(str,sizeof(str),"[INFO] API version %d.%d, Driver version %d.%d",(ver.api>>8),(ver.api&0xFFU),(ver.drv>>8),(ver.drv&0xFFU));
  TEST_MESSAGE(str);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_MAC_GetCapabilities
\details
The function \b ETH_MAC_GetCapabilities verifies the Ethernet MAC \b GetCapabilities function.
*/
void ETH_MAC_GetCapabilities (void) {
  ARM_ETH_MAC_CAPABILITIES cap;

  /* Get ETH_MAC capabilities */
  cap = eth_mac->GetCapabilities();

  TEST_ASSERT_MESSAGE((cap.reserved == 0U), "[FAILED] Capabilities reserved field must be 0");
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_MAC_Initialization
\details
The function \b ETH_MAC_Initialization verifies the Ethernet MAC functions in the following order:
  - \b Initialize without callback
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
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_MAC_CheckInvalidInit
\details
The function \b ETH_MAC_CheckInvalidInit verifies the driver behavior when receiving an invalid initialization sequence:
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
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);

  /* Try to power on */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK);

  /* Try to set configuration */
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_100M | ARM_ETH_MAC_DUPLEX_FULL |
    ARM_ETH_MAC_ADDRESS_BROADCAST | ARM_ETH_MAC_ADDRESS_ALL) != ARM_DRIVER_OK);

  /* Power off */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_MAC_PowerControl
\details
The function \b ETH_MAC_PowerControl verifies the Ethernet MAC \b PowerControl function with the sequence:
  - Initialize
  - Power on
  - Power low
  - Power off
  - Uninitialize
*/
void ETH_MAC_PowerControl (void) {
  int32_t retv;

  /* Initialize with callback if supported */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);

  /* Power on */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Power low */
  retv = eth_mac->PowerControl(ARM_POWER_LOW);
  if (retv == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Low power is not supported"); }
  else { TEST_ASSERT(retv == ARM_DRIVER_OK); }

  /* Power off */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_MAC_MacAddress
\details
The function \b ETH_MAC_MacAddress verifies the Ethernet MAC \b GetMacAddress and \b SetMacAddress functions
with the sequence:
  - Initialize
  - Power on
  - Set Ethernet MAC Address
  - Get Ethernet MAC Address
  - Power off
  - Uninitialize
*/
void ETH_MAC_MacAddress (void) {
  ARM_ETH_MAC_ADDR my_addr;

  /* Initialize with callback if supported and power on */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Set MAC Address */
  TEST_ASSERT(eth_mac->SetMacAddress(&mac_addr) == ARM_DRIVER_OK);

  /* Verify MAC Address */
  memset(&my_addr, 0, 6);
  TEST_ASSERT(eth_mac->GetMacAddress(&my_addr) == ARM_DRIVER_OK);
  if (memcmp(&mac_addr, &my_addr, 6) != 0) {
    TEST_FAIL_MESSAGE("[FAILED] Verify MAC address");
  } else TEST_PASS();

  /* Power off */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_MAC_SetBusSpeed
\details
The function \b ETH_MAC_SetBusSpeed verifies the Ethernet MAC \b Control function with the sequence:
  - Initialize
  - Power on
  - Set bus speed \token{10M}
  - Set bus speed \token{100M}
  - Set bus speed \token{1G}
  - Power off
  - Uninitialize
*/
void ETH_MAC_SetBusSpeed (void) {
  int32_t retv;

  /* Initialize with callback if supported and power on */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Set bus speed 10M */
  retv = eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_10M);
  if (retv == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Link speed 10M is not supported"); }
  else { TEST_ASSERT(retv == ARM_DRIVER_OK); }

  /* Set bus speed 100M */
  retv = eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_100M);
  if (retv == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Link speed 100M is not supported"); }
  else { TEST_ASSERT(retv == ARM_DRIVER_OK); }

  /* Set bus speed 1G */
  retv = eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_1G);
  if (retv == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Link speed 1G is not supported"); }
  else { TEST_ASSERT(retv == ARM_DRIVER_OK); }

  /* Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_MAC_Config_Mode
\details
The function \b ETH_MAC_Config_Mode verifies the Ethernet MAC \b Control function with the sequence:
  - Initialize
  - Power on
  - Set full duplex
  - Set half duplex
  - Power off
  - Uninitialize
*/
void ETH_MAC_Config_Mode (void) {

  /* Initialize with callback if supported and power on */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Set full duplex */
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_DUPLEX_FULL) == ARM_DRIVER_OK);

  /* Set half duplex */
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_DUPLEX_HALF) == ARM_DRIVER_OK);

  /* Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_MAC_Config_CommonParams
\details
The function \b ETH_MAC_Config_CommonParams verifies the Ethernet MAC \b Control function with the sequence:
  - Initialize
  - Power on
  - Configure Ethernet MAC bus
  - Configure receiver
  - Configure transmitter
  - Power off
  - Uninitialize
*/
void ETH_MAC_Config_CommonParams (void) {

  /* Initialize with callback if supported and power on */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Configure ETH_MAC bus*/
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_100M | ARM_ETH_MAC_DUPLEX_FULL |
    ARM_ETH_MAC_ADDRESS_BROADCAST | ARM_ETH_MAC_ADDRESS_ALL | ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK);

  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_RX, 1) == ARM_DRIVER_OK);

  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_TX, 1) == ARM_DRIVER_OK);

  /* Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Function: ETH_MAC_Control_Filtering
\details
The function \b ETH_MAC_Control_Filtering verifies the Ethernet MAC \b Control function with the following sequence:
  - Buffer allocation
  - Initialize
  - Power on
  - Broadcast receive
  - Receive with broadcast disabled
  - Multicast receive
  - Receive with multicast disabled
  - Unicast receive
  - Promiscuous mode receive
  - Power off
  - Uninitialize

\note
The internal Ethernet MAC loopback is used for the test.
*/
void ETH_MAC_Control_Filtering (void) {
  uint32_t i,tick;

  /* Allocate buffers */
  buffer_out = (uint8_t *)malloc(64);
  TEST_ASSERT(buffer_out != NULL);
  if (buffer_out == NULL) return;
  buffer_in = (uint8_t *)malloc(64);
  TEST_ASSERT(buffer_in != NULL);
  if (buffer_in == NULL) { free(buffer_out); return; }

  /* Initialize, power on and configure MAC and PHY */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->SetMacAddress(&mac_addr) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_100M |
    ARM_ETH_MAC_DUPLEX_FULL | ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  osDelay (100);
  TEST_ASSERT(eth_phy->SetInterface(capab.media_interface) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->SetMode(ARM_ETH_PHY_AUTO_NEGOTIATE) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_RX, 1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_TX, 1) == ARM_DRIVER_OK);

  /* Set Ethernet header */
  memcpy(&buffer_out[6], &mac_addr, 6);
  buffer_out[12] = 0;
  buffer_out[13] = 50;

  for (i = 14; i < 64; i++) {
    buffer_out[i] = i + 'A' - 14;
  }

  /* Broadcast receive */
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_LOOPBACK |
    ARM_ETH_MAC_ADDRESS_BROADCAST) == ARM_DRIVER_OK);
  memcpy(&buffer_out[0], &mac_bcast, 6);
  if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
    TEST_FAIL_MESSAGE("[FAILED] Receive broadcast");
  } else TEST_PASS();

  /* Receive with broadcast disabled */
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK);
  if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) == ARM_DRIVER_OK) {
    TEST_MESSAGE("[WARNING] Broadcast receive not disabled");
  }

  /* Multicast receive */
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_LOOPBACK |
    ARM_ETH_MAC_ADDRESS_MULTICAST) == ARM_DRIVER_OK);
  for (i = 0; i < 6; i++) {
    memcpy(&buffer_out[0], &mac_mcast[i], 6);
    if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Receive multicast %d address",i);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();
  }

  /* Receive with multicast disabled */
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK);
  memcpy(&buffer_out[0], &mac_mcast[0], 6);
  if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) == ARM_DRIVER_OK) {
    TEST_MESSAGE("[WARNING] Multicast receive not disabled");
  }

  /* Unicast receive */
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK);
  memcpy(&buffer_out[0], &mac_addr, 6);
  if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
    TEST_FAIL_MESSAGE("[FAILED] Receive unicast");
  } else TEST_PASS();

  /* Promiscuous mode receive */
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_LOOPBACK |
    ARM_ETH_MAC_ADDRESS_ALL) == ARM_DRIVER_OK);
  /* Test broadcast receive */
  memcpy(&buffer_out[0], &mac_bcast, 6);
  if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
    TEST_FAIL_MESSAGE("[FAILED] Receive broadcast in promiscuous mode");
  } else TEST_PASS();
  /* Test multicast receive */
  memcpy(&buffer_out[0], &mac_mcast[0], 6);
  if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
    TEST_FAIL_MESSAGE("[FAILED] Receive multicast in promiscuous mode");
  } else TEST_PASS();
  /* Test unicast receive */
  memcpy(&buffer_out[0], &mac_addr, 6);
  if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
    TEST_FAIL_MESSAGE("[FAILED] Receive unicast in promiscuous mode");
  } else TEST_PASS();

  /* Power off and uninitialize */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);

  /* Free buffers */
  free(buffer_out);
  free(buffer_in);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Function: ETH_MAC_SetAddressFilter
\details
The function \b ETH_MAC_SetAddressFilter verifies the Ethernet MAC \b SetAddressFilter function with the following sequence:
  - Buffer allocation
  - Initialize
  - Power on
  - Receive one multicast address
  - Receive two multicast addresses
  - Receive three multicast addresses
  - Receive four multicast addresses
  - Receive six multicast addresses
  - Power off
  - Uninitialize

\note
The internal Ethernet MAC loopback is used for the test.
*/
void ETH_MAC_SetAddressFilter (void) {
  uint32_t i,tick;

  /* Allocate buffers */
  buffer_out = (uint8_t *)malloc(64);
  TEST_ASSERT(buffer_out != NULL);
  if (buffer_out == NULL) return;
  buffer_in = (uint8_t *)malloc(64);
  TEST_ASSERT(buffer_in != NULL);
  if (buffer_in == NULL) { free(buffer_out); return; }

  /* Initialize, power on and configure MAC and PHY */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->SetMacAddress(&mac_addr) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_100M |
    ARM_ETH_MAC_DUPLEX_FULL | ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  osDelay (100);
  TEST_ASSERT(eth_phy->SetInterface(capab.media_interface) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->SetMode(ARM_ETH_PHY_AUTO_NEGOTIATE) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_RX, 1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_TX, 1) == ARM_DRIVER_OK);

  /* Set Ethernet header */
  memcpy(&buffer_out[6], &mac_addr, 6);
  buffer_out[12] = 0;
  buffer_out[13] = 50;

  for (i = 14; i < 64; i++) {
    buffer_out[i] = i + 'A' - 14;
  }

  /* Receive one multicast address */
  TEST_ASSERT(eth_mac->SetAddressFilter(mac_mcast, 1) == ARM_DRIVER_OK);

  memcpy(&buffer_out[0], &mac_mcast[0], 6);
  if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
    /* Error, enabled multicast address not received */
    TEST_FAIL_MESSAGE("[FAILED] Receive multicast 0 address");
  } else TEST_PASS();

  for (i = 1; i < 6; i++) {
    memcpy(&buffer_out[0], &mac_mcast[i], 6);
    if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) == ARM_DRIVER_OK) {
      /* Warning, disabled multicast address received */
      snprintf(str,sizeof(str),"[WARNING] Receive multicast %d address",i);
      TEST_MESSAGE(str);
    }
  }

  /* Receive two multicast addresses */
  TEST_ASSERT(eth_mac->SetAddressFilter(mac_mcast, 2) == ARM_DRIVER_OK);

  for (i = 0; i < 2; i++) {
    memcpy(&buffer_out[0], &mac_mcast[i], 6);
    if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
      /* Error, enabled multicast address not received */
      snprintf(str,sizeof(str),"[FAILED] Receive multicast %d address",i);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();
  }

  for (; i < 6; i++) {
    memcpy(&buffer_out[0], &mac_mcast[i], 6);
    if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) == ARM_DRIVER_OK) {
      /* Warning, disabled multicast address received */
      snprintf(str,sizeof(str),"[WARNING] Receive multicast %d address",i);
      TEST_MESSAGE(str);
    }
  }

  /* Receive three multicast addresses */
  TEST_ASSERT(eth_mac->SetAddressFilter(mac_mcast, 3) == ARM_DRIVER_OK);

  for (i = 0; i < 3; i++) {
    memcpy(&buffer_out[0], &mac_mcast[i], 6);
    if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
      /* Error, enabled multicast address not received */
      snprintf(str,sizeof(str),"[FAILED] Receive multicast %d address",i);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();
  }

  for (; i < 6; i++) {
    memcpy(&buffer_out[0], &mac_mcast[i], 6);
    if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) == ARM_DRIVER_OK) {
      /* Warning, disabled multicast address received */
      snprintf(str,sizeof(str),"[WARNING] Receive multicast %d address",i);
      TEST_MESSAGE(str);
    }
  }

  /* Receive four multicast addresses */
  TEST_ASSERT(eth_mac->SetAddressFilter(mac_mcast, 4) == ARM_DRIVER_OK);

  for (i = 0; i < 4; i++) {
    memcpy(&buffer_out[0], &mac_mcast[i], 6);
    if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
      /* Error, enabled multicast address not received */
      snprintf(str,sizeof(str),"[FAILED] Receive multicast %d address",i);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();
  }

  for (; i < 6; i++) {
    memcpy(&buffer_out[0], &mac_mcast[i], 6);
    if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) == ARM_DRIVER_OK) {
      /* Warning, disabled multicast address received */
      snprintf(str,sizeof(str),"[WARNING] Receive multicast %d address",i);
      TEST_MESSAGE(str);
    }
  }

  /* Receive all multicast addresses */
  TEST_ASSERT(eth_mac->SetAddressFilter(mac_mcast, 6) == ARM_DRIVER_OK);

  for (i = 0; i < 6; i++) {
    memcpy(&buffer_out[0], &mac_mcast[i], 6);
    if (ETH_RunTransfer(buffer_out, buffer_in, 64, 0) != ARM_DRIVER_OK) {
      /* Error, enabled multicast address not received */
      snprintf(str,sizeof(str),"[FAILED] Receive multicast %d address",i);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();
  }

  /* Power off and uninitialize */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);

  /* Free buffers */
  free(buffer_out);
  free(buffer_in);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_MAC_SignalEvent
\details
The function \b ETH_MAC_SignalEvent verifies the Ethernet MAC interrupt operation with the sequence:
  - Initialize
  - Power on
  - Configure receiver
  - Configure transmitter
  - Set output buffer pattern
  - Send data and check receive interrupts
  - Power off
  - Uninitialize

\note
The internal Ethernet MAC loopback is used for the test.
*/
void ETH_MAC_SignalEvent (void) {
  uint32_t i,tick;

  if (!capab.event_rx_frame) {
    TEST_MESSAGE("[WARNING] Interrupt mode is not supported");
    return;
  }

  /* Allocate buffer */
  buffer_out = (uint8_t *)malloc(64);
  TEST_ASSERT(buffer_out != NULL);
  if (buffer_out == NULL) return;

  /* Initialize, power on and configure MAC */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->SetMacAddress(&mac_addr) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_100M | ARM_ETH_MAC_DUPLEX_FULL |
    ARM_ETH_MAC_ADDRESS_BROADCAST | ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  osDelay (100);
  TEST_ASSERT(eth_phy->SetInterface(capab.media_interface) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->SetMode(ARM_ETH_PHY_AUTO_NEGOTIATE) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_RX, 1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_TX, 1) == ARM_DRIVER_OK);

  /* Set Ethernet header */
  memcpy(&buffer_out[0], &mac_bcast, 6);
  memcpy(&buffer_out[6], &mac_addr,  6);
  buffer_out[12] = 0;
  buffer_out[13] = 50;

  for (i = 14; i < 64; i++) {
    buffer_out[i] = i + 'A' - 14;
  }

  Event &= ~ARM_ETH_MAC_EVENT_RX_FRAME;
  TEST_ASSERT(eth_mac->SendFrame(buffer_out, 64, 0) == ARM_DRIVER_OK);

  /* Wait for RX interrupt event */
  tick = GET_SYSTICK();
  do {
    if (Event & ARM_ETH_MAC_EVENT_RX_FRAME) {
      break;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(ETH_TRANSFER_TIMEOUT*1000));

  if (!(Event & ARM_ETH_MAC_EVENT_RX_FRAME)) {
    TEST_FAIL_MESSAGE("[FAILED] Interrupt mode not working");
  } else TEST_PASS();

  /* Power off and uninitialize */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function ETH_PHY_GetVersion
\details
The function \b ETH_PHY_GetVersion verifies the Ethernet PHY \b GetVersion function.
*/
void ETH_PHY_GetVersion (void) {
  ARM_DRIVER_VERSION ver;

  ver = eth_phy->GetVersion();

  TEST_ASSERT((ver.api >= ARM_DRIVER_VERSION_MAJOR_MINOR(1UL,0UL)) && (ver.drv >= ARM_DRIVER_VERSION_MAJOR_MINOR(1UL,0UL)));

  snprintf(str,sizeof(str),"[INFO] API version %d.%d, Driver version %d.%d",(ver.api>>8),(ver.api&0xFFU),(ver.drv>>8),(ver.drv&0xFFU));
  TEST_MESSAGE(str);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_PHY_Initialization
\details
The function \b ETH_PHY_Initialization verifies the Ethernet PHY functions in the following order:
  - \b Initialize with read and write functions
*/
void ETH_PHY_Initialization (void) {

  /* MAC Initialize and power on */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Initialize */
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);

  /* MAC Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_PHY_CheckInvalidInit
\details
The function \b ETH_PHY_CheckInvalidInit verifies the driver behavior when receiving an invalid initialization sequence:
  - \b Uninitialize
  - \b PowerControl with Power off
  - \b PowerControl with Power on
  - \b SetInterface to configure the Ethernet PHY bus
  - \b SetMode to configure the Ethernet PHY bus
  - \b PowerControl with Power off
  - \b Uninitialize
*/
void ETH_PHY_CheckInvalidInit (void) {

  /* Uninitialize */
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);

  /* Power off */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_OFF) != ARM_DRIVER_OK);

  /* Try to power on */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK);

  /* Try to configure ETH_PHY bus*/
  TEST_ASSERT(eth_phy->SetInterface(capab.media_interface) != ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->SetMode(ARM_ETH_PHY_AUTO_NEGOTIATE) != ARM_DRIVER_OK);

  /* Try to initialize without read and write functions */
  TEST_ASSERT(eth_phy->Initialize(NULL, NULL) != ARM_DRIVER_OK);

  /* Power off */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_OFF) != ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_PHY_PowerControl
\details
The function \b ETH_PHY_PowerControl verifies the Ethernet PHY \b PowerControl function with the sequence:
  - Initialize
  - Power on
  - Power low
  - Power off
  - Check RMII lockup
  - Uninitialize

\note
When the Ethernet PHY is the 50 MHz clock source for the MAC in RMII mode, the MAC may lock up if the PHY power is off
and therefore not generating a reference clock.
*/
void ETH_PHY_PowerControl (void) {
  int32_t retv;

  /* MAC Initialize and power on */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Initialize */
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK);

  /* Power on */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Power low */
  retv = eth_phy->PowerControl(ARM_POWER_LOW);
  if (retv == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Low power is not supported"); }
  else { TEST_ASSERT(retv == ARM_DRIVER_OK); }

  /* Power off */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);

  /* Check RMII lockup */
  if ((capab.media_interface == ARM_ETH_INTERFACE_RMII) && (!mac_lockup)) {
    eth_mac->PowerControl(ARM_POWER_OFF);
    eth_mac->PowerControl(ARM_POWER_FULL);
    eth_phy->PowerControl(ARM_POWER_OFF);
    eth_phy->Uninitialize();
  }
  if (mac_lockup) {
    TEST_MESSAGE("[WARNING] MAC is locked when PHY power is off");
  }

  /* MAC Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Function: ETH_PHY_Config
\details
The function \b ETH_PHY_Config verifies the PHY functions
  - Initialize
  - Power on
  - SetInterface
  - SetMode
  - Power off
  - Uninitialize
*/
void ETH_PHY_Config (void) {

  /* MAC Initialize and power on */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Initialize and power on */
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Configure ETH_PHY bus */
  TEST_ASSERT(eth_phy->SetInterface(capab.media_interface) == ARM_DRIVER_OK);

  TEST_ASSERT(eth_phy->SetMode(ARM_ETH_PHY_AUTO_NEGOTIATE) == ARM_DRIVER_OK);

  /* Power off and uninitialize */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);

  /* MAC Power off and uninitialize */
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Function: ETH_Loopback_Transfer
\details
The function \b ETH_Loopback_Transfer verifies data transfer via Ethernet with the following sequence:
  - Buffer allocation
  - Initialize
  - Power on
  - Set output buffer pattern
  - Transfer data chunks
  - Set output buffer with random data
  - Transfer data chunks
  - Transfer data by sending in two fragments
  - Power off
  - Uninitialize

\note
The internal Ethernet MAC loopback is used as a data loopback, so there is no need to use an external loopback cable.
*/
void ETH_Loopback_Transfer (void) {
  const uint16_t test_len[] = {
    28,36,40,44,45,46,47,48,49,50,55,60,65,70,75,80,85,90,95,100,110,120,127,128,
    129,140,150,160,170,180,190,196,200,220,240,250,256,270,300,325,350,375,400,
    425,450,475,500,511,512,513,525,550,575,600,625,650,700,750,800,850,900,950,
    1000,1024,1050,1100,1150,1200,1250,1300,1320,1420,1480,1490,1491,1492,1493,
    1494,1495,1496,1497,1498,1499,1500};
  const uint32_t test_num = ARRAY_SIZE(test_len);
  uint32_t i,cnt,tick;

  /* Allocate buffers, add space for Ethernet header */
  buffer_out = (uint8_t *)malloc(14+ETH_MTU);
  TEST_ASSERT(buffer_out != NULL);
  if (buffer_out == NULL) return;
  buffer_in = (uint8_t *)malloc(14+ETH_MTU);
  TEST_ASSERT(buffer_in != NULL);
  if (buffer_in == NULL) { free(buffer_out); return; }

  /* Initialize, power on and configure MAC */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->SetMacAddress(&mac_addr) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_100M | ARM_ETH_MAC_DUPLEX_FULL |
    ARM_ETH_MAC_ADDRESS_BROADCAST | ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  osDelay (100);
  TEST_ASSERT(eth_phy->SetInterface(capab.media_interface) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->SetMode(ARM_ETH_PHY_AUTO_NEGOTIATE) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_RX, 1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_TX, 1) == ARM_DRIVER_OK);

  /* Set output buffer pattern */
  for (i = 0; i < ETH_MTU; i+=2) {
    buffer_out[14+i] = 0x55;
    buffer_out[15+i] = 0xAA;
  }

  /* Set Ethernet header */
  memcpy(&buffer_out[0], &mac_bcast, 6);
  memcpy(&buffer_out[6], &mac_addr,  6);

  /* Transfer data chunks */
  for (cnt = 0; cnt < test_num; cnt++) {
    /* Clear input buffer */
    memset(buffer_in, 0, test_len[cnt]);
    /* Set Ethernet type/length */
    buffer_out[12] = test_len[cnt] >> 8;
    buffer_out[13] = test_len[cnt] & 0xFF;
    if (ETH_RunTransfer(buffer_out, buffer_in, 14+test_len[cnt], 0) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Transfer block of %d bytes",test_len[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else if (memcmp(buffer_in, buffer_out, 14+test_len[cnt]) != 0) {
      snprintf(str,sizeof(str),"[FAILED] Verify block of %d bytes",test_len[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();
  }

  /* Set output buffer with random data */
  srand(GET_SYSTICK());
  for (i = 0; i < ETH_MTU; i++) {
    buffer_out[14+cnt] = (uint8_t)rand();
  }

  /* Transfer data chunks */
  for (cnt = 0; cnt < test_num; cnt++) {
    /* Clear input buffer */
    memset(buffer_in, 0, test_len[cnt]);
    /* Set Ethernet type/length */
    buffer_out[12] = test_len[cnt] >> 8;
    buffer_out[13] = test_len[cnt] & 0xFF;
    if (ETH_RunTransfer(buffer_out, buffer_in, 14+test_len[cnt], 0) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Transfer block of %d bytes",test_len[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else if (memcmp(buffer_in, buffer_out, 14+test_len[cnt]) != 0) {
      snprintf(str,sizeof(str),"[FAILED] Verify block of %d bytes",test_len[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();
  }

  /* Block transfer in two fragments: header and data */
  for (cnt = 0; cnt < test_num; cnt++) {
    /* Clear input buffer */
    memset(buffer_in, 0, test_len[cnt]);
    /* Set Ethernet type/length */
    buffer_out[12] = test_len[cnt] >> 8;
    buffer_out[13] = test_len[cnt] & 0xFF;
    if (ETH_RunTransfer(buffer_out, buffer_in, 14+test_len[cnt], 0) != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[FAILED] Fragmented block transfer of %d bytes",test_len[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else if (memcmp(buffer_in, buffer_out, 14+test_len[cnt]) != 0) {
      snprintf(str,sizeof(str),"[FAILED] Fragmented block check of %d bytes",test_len[cnt]);
      TEST_FAIL_MESSAGE(str);
    } else TEST_PASS();
  }

  /* Power off and uninitialize */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);

  /* Free buffers */
  free(buffer_out);
  free(buffer_in);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Function: ETH_Loopback_External
\details
The function \b ETH_Loopback_External verifies data transfer via Ethernet with the following sequence:
  - Buffer allocation
  - Initialize
  - Power on
  - Set output buffer pattern
  - Transfer data on PHY internal loopback
  - Ethernet connection
  - Transfer data on external cable loopback
  - Power off
  - Uninitialize

\note
An external loopback cable is required for this test.
*/
void ETH_Loopback_External (void) {
  ARM_ETH_LINK_INFO info;
  uint32_t i,cnt,tick;

  /* Allocate buffers, add space for Ethernet header */
  buffer_out = (uint8_t *)malloc(14+ETH_MTU);
  TEST_ASSERT(buffer_out != NULL);
  if (buffer_out == NULL) return;
  buffer_in = (uint8_t *)malloc(14+ETH_MTU);
  TEST_ASSERT(buffer_in != NULL);
  if (buffer_in == NULL) { free(buffer_out); return; }

  /* Initialize, power on and configure MAC and PHY */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->SetMacAddress(&mac_addr) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_100M | ARM_ETH_MAC_DUPLEX_FULL |
    ARM_ETH_MAC_ADDRESS_BROADCAST) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  osDelay (100);
  TEST_ASSERT(eth_phy->SetInterface(capab.media_interface) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->SetMode(ARM_ETH_PHY_SPEED_100M |
   ARM_ETH_PHY_DUPLEX_FULL | ARM_ETH_PHY_LOOPBACK) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_RX, 1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_TX, 1) == ARM_DRIVER_OK);

  /* Fill output buffer */
  for (cnt = 0; cnt < ETH_MTU; cnt++) {
    buffer_out[14+cnt] = (cnt ^ 0x20) & 0x7F;
  }

  /* Set Ethernet header */
  memcpy(&buffer_out[0], &mac_bcast, 6);
  memcpy(&buffer_out[6], &mac_addr,  6);

  /* Set Ethernet type/length */
  buffer_out[12] = (ETH_MTU >> 8) & 0xFF;
  buffer_out[13] =  ETH_MTU       & 0xFF;

  /* Check PHY internal loopback */
  TEST_ASSERT(eth_phy->SetMode(ARM_ETH_PHY_SPEED_100M |
    ARM_ETH_PHY_DUPLEX_FULL | ARM_ETH_PHY_LOOPBACK) == ARM_DRIVER_OK);
  osDelay(200);

  /* Clear input buffer*/
  memset(buffer_in, 0, 14+ETH_MTU);
  if (ETH_RunTransfer(buffer_out, buffer_in, 14+ETH_MTU, 0) != ARM_DRIVER_OK) {
    TEST_MESSAGE("[WARNING] PHY internal loopback is not active");
  } else if (memcmp(buffer_in, buffer_out, 14+ETH_MTU) != 0) {
    TEST_FAIL_MESSAGE("[FAILED] Verify received data");
  } else TEST_PASS();

  /* Check external cable loopback */
  TEST_ASSERT(eth_phy->SetMode(ARM_ETH_PHY_AUTO_NEGOTIATE) == ARM_DRIVER_OK);

  /* Check Ethernet link */
  tick = GET_SYSTICK();
  while (eth_phy->GetLinkState() != ARM_ETH_LINK_UP) {
    if ((GET_SYSTICK() - tick) >= SYSTICK_MICROSEC(ETH_LINK_TIMEOUT*1000)) {
      TEST_FAIL_MESSAGE("[FAILED] Link down, connect Ethernet cable");
      goto end;
    }
  }

  info = eth_phy->GetLinkInfo();
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE,
    (uint32_t)info.speed  << ARM_ETH_MAC_SPEED_Pos  |
    (uint32_t)info.duplex << ARM_ETH_MAC_DUPLEX_Pos |
    ARM_ETH_MAC_ADDRESS_BROADCAST) == ARM_DRIVER_OK);

  /* Clear input buffer*/
  memset(buffer_in, 0, 14+ETH_MTU);
  if (ETH_RunTransfer(buffer_out, buffer_in, 14+ETH_MTU, 0) != ARM_DRIVER_OK) {
    TEST_FAIL_MESSAGE("[FAILED] Transfer external cable loopback");
  } else if (memcmp(buffer_in, buffer_out, 14+ETH_MTU) != 0) {
    TEST_FAIL_MESSAGE("[FAILED] Verify received data");
  } else TEST_PASS();

  /* Power off and uninitialize */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
end:
  /* Free buffers */
  free(buffer_out);
  free(buffer_in);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_MAC_PTP_ControlTimer
\details
The function \b ETH_MAC_PTP_ControlTimer verifies the PTP \b ControlTimer function with the sequence:
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
  ARM_ETH_MAC_TIME time1,time2;
  int64_t t,t1ns,t2ns,overhead;
  double rate;

  /* Get capabilities */
  if (!capab.precision_timer) {
    TEST_MESSAGE("[WARNING] Precision Time Protocol is not supported");
    return;
  }

  /* Initialize, power on and configure MAC */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Set Time -------------------------------------------------------------------------------- */
  time1.sec = 0U;
  time1.ns  = 0U;
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_SET_TIME, &time1) == ARM_DRIVER_OK);

  /* Check System Time */
  osDelay(1U);
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK);
  osDelay(PTP_TIME_REF);
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK);

  /* Get timestamps in nanoseconds */
  t1ns = (int64_t)time1.sec*PTP_S_NS + time1.ns;
  t2ns = (int64_t)time2.sec*PTP_S_NS + time2.ns;
  t    = t2ns - t1ns - PTP_TIME_REF_NS;

  /* Check timestamps difference */
  if (llabs(t) > ETH_PTP_TOLERANCE) {
    snprintf(str,sizeof(str),"[WARNING] PTP measured time is %lldns from expected", t);
    TEST_MESSAGE(str);
  } else TEST_PASS();

  /* Adjust clock - Set correction factor ---------------------------------------------------- */
  /* Calculate rate and convert it to q31 format */
  rate     = (double)PTP_TIME_REF_NS / (t2ns-t1ns);
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
  t    = t2ns - t1ns - PTP_TIME_REF_NS;

  /* Check timestamps difference */
  if (llabs(t) > ETH_PTP_TOLERANCE) {
    snprintf(str,sizeof(str),"[WARNING] PTP measured time with adj clk is %lldns from expected", t);
    TEST_MESSAGE(str);
  } else TEST_PASS();

  /* Measure time overhead for increment/decrement calls ------------------------------------- */
  time2.sec = 0;
  time2.ns  = 0;
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_INC_TIME, &time2) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK);
  t1ns = (int64_t)time1.sec*PTP_S_NS + time1.ns;
  t2ns = (int64_t)time2.sec*PTP_S_NS + time2.ns;
  overhead = t2ns - t1ns;

  /* Increment time -------------------------------------------------------------------------- */
  time2.sec = PTP_TIME_REF/1000U;
  time2.ns  = (PTP_TIME_REF-time2.sec*1000U)*1000000U;
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_INC_TIME, &time2) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK);

  /* Get timestamps in nanoseconds */
  t1ns = (int64_t)time1.sec*PTP_S_NS + time1.ns;
  t2ns = (int64_t)time2.sec*PTP_S_NS + time2.ns;
  t    = t2ns - t1ns - PTP_TIME_REF_NS - overhead;

  /* Check timestamps difference */
  if (llabs(t) > ETH_PTP_TOLERANCE) {
    snprintf(str,sizeof(str),"[WARNING] PTP incremented time is %lldns from expected", t);
    TEST_MESSAGE(str);
  } else TEST_PASS();

  /* Decrement time -------------------------------------------------------------------------- */
  time2.sec =  PTP_TIME_REF/1000U;
  time2.ns  = (PTP_TIME_REF-time2.sec*1000U)*1000000U;
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_DEC_TIME, &time2) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK);

  /* Get timestamps in nanoseconds */
  t1ns = (int64_t)time1.sec*PTP_S_NS + time1.ns;
  t2ns = (int64_t)time2.sec*PTP_S_NS + time2.ns;
  t    = t2ns - t1ns + PTP_TIME_REF_NS - overhead;

  /* Check timestamps difference */
  if (llabs(t) > ETH_PTP_TOLERANCE) {
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
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: ETH_Loopback_PTP
\details
The function \b ETH_Loopback_PTP verifies the Precision Time Protocol functions \b ControlTimer, \b GetRxFrameTime and
\b GetTxFrameTime with the sequence:
  - Initialize
  - Power on
  - Set Control Timer
  - Transfer a frame
  - Get TX frame time
  - Get RX frame time
  - Power off
  - Uninitialize

\note
The internal Ethernet MAC loopback is used as the data loopback.
*/
void ETH_Loopback_PTP (void) {
  ARM_ETH_MAC_TIME time1,time2;
  uint32_t tick;

  // PTP over Ethernet IPv4 sample frame
  const uint8_t PTP_frame[] = {
    0x01,0x00,0x5e,0x00,0x01,0x81,0x02,0x30,0x05,0x1d,0x1e,0x27,0x08,0x00,0x45,0x00,
    0x00,0x98,0x00,0x5d,0x40,0x00,0x01,0x11,0x29,0x68,0x0a,0x0a,0x64,0x05,0xe0,0x00,
    0x01,0x81,0x01,0x3f,0x01,0x3f,0x00,0x84,0xc0,0x7b,0x00,0x01,0x00,0x01,0x5f,0x44,
    0x46,0x4c,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,
    0x00,0x30,0x05,0x1d,0x1e,0x27,0x00,0x01,0x00,0x5e,0x00,0x00,0x00,0x08,0x00,0x00,
    0x00,0x00,0x45,0x5b,0x0a,0x38,0x0e,0xb9,0x26,0x58,0x00,0x00,0x00,0x00,0x00,0x01,
    0x00,0x30,0x05,0x1d,0x1e,0x27,0x00,0x00,0x00,0x5e,0x00,0x00,0x00,0x04,0x44,0x46,
    0x4c,0x54,0x00,0x00,0xf0,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,
    0xf0,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x44,0x46,0x4c,0x54,0x00,0x01,
    0x00,0x30,0x05,0x1d,0x1e,0x27,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00
  };
  const uint32_t PTP_frame_len = sizeof(PTP_frame);

  /* Get capabilities */
  if (!capab.precision_timer) {
    TEST_MESSAGE("[WARNING] Precision Time Protocol is not supported");
    return;
  }

  /* Allocate buffer */
  buffer_in = (uint8_t *)malloc(PTP_frame_len);
  TEST_ASSERT(buffer_in != NULL);
  if (buffer_in == NULL) return;

  /* Initialize, power on and configure MAC and PHY */
  TEST_ASSERT(eth_mac->Initialize(cb_event) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONFIGURE, ARM_ETH_MAC_SPEED_100M | ARM_ETH_MAC_DUPLEX_FULL |
    ARM_ETH_MAC_ADDRESS_BROADCAST | ARM_ETH_MAC_ADDRESS_ALL | ARM_ETH_MAC_LOOPBACK) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_RX, 1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Control(ARM_ETH_MAC_CONTROL_TX, 1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Initialize(eth_mac->PHY_Read, eth_mac->PHY_Write) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK);
  osDelay (100);
  TEST_ASSERT(eth_phy->SetInterface(capab.media_interface) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->SetMode(ARM_ETH_PHY_AUTO_NEGOTIATE) == ARM_DRIVER_OK);

  /* Set Time */
  time1.sec = 0U;
  time1.ns  = 0U;
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_SET_TIME, &time1) == ARM_DRIVER_OK);

  /* Check timestamps - verify if control timer is running */
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time1) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->ControlTimer(ARM_ETH_MAC_TIMER_GET_TIME, &time2) == ARM_DRIVER_OK);
  TEST_ASSERT((time2.sec==time1.sec)?(time2.ns>time1.ns):(time2.sec>time1.sec));

  /* Transfer frame */
  TEST_ASSERT(eth_mac->SendFrame(PTP_frame, PTP_frame_len, ARM_ETH_MAC_TX_FRAME_TIMESTAMP) == ARM_DRIVER_OK);
  tick = GET_SYSTICK();
  while (eth_mac->GetRxFrameSize() == 0) {
    if ((GET_SYSTICK() - tick) >= SYSTICK_MICROSEC(ETH_TRANSFER_TIMEOUT*1000)) {
      TEST_FAIL_MESSAGE("[FAILED] Transfer timeout");
      break;
    }
  }

  /* Get TX Frame Time */
  TEST_ASSERT(eth_mac->GetTxFrameTime(&time1) == ARM_DRIVER_OK);

  /* Get RX Frame Time */
  TEST_ASSERT(eth_mac->GetRxFrameTime(&time2) == ARM_DRIVER_OK);

  /* Check timestamps */
  TEST_ASSERT((time2.sec==time1.sec) ? (time2.ns>time1.ns) : (time2.sec>time1.sec));

  /* Check frame */
  eth_mac->ReadFrame(buffer_in, PTP_frame_len);
  TEST_ASSERT(memcmp(buffer_in, PTP_frame, PTP_frame_len) == 0);

  /* Power off and uninitialize */
  TEST_ASSERT(eth_phy->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_phy->Uninitialize() == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(eth_mac->Uninitialize() == ARM_DRIVER_OK);

  /* Free buffer */
  free(buffer_in);
}


/**
@}
*/
// end of group dv_eth
