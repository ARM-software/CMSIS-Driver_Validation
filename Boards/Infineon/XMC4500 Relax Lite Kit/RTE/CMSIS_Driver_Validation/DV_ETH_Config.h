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
 * $Revision:   V1.0.0
 *
 * Project:     CMSIS-Driver Validation
 * Title:       Ethernet (ETH) driver validation configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_ETH_CONFIG_H_
#define DV_ETH_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> Ethernet
// <i> Ethernet (ETH) driver validation configuration
// <o> Driver_ETH_MAC# <0-255>
// <i> Choose the Driver_ETH_MAC# instance to test.
// <i> For example to test Driver_ETH_MAC0 select 0.
#define DRV_ETH                         0
// <o> Link timeout
// <i> Set the Ethernet link timeout (us)
#define ETH_LINK_TIMEOUT                3000000
// <o> Transfer timeout
// <i> Set the transfer timeout (us)
#define ETH_TRANSFER_TIMEOUT 1000000
// <o> Time duration for PTP Control Time
// <i> Set time duration for Control Time tests (ms)
#define ETH_PTP_TIME_REF                1000
// <o> Tolerance for PTP Control Time
// <i> Set tolerance for Control Time tests (ns)
#define ETH_PTP_TOLERANCE               0
// <h> Tests
// <i> Enable / disable tests.
// <q> ETH_MAC_GetCapabilities
#define ETH_MAC_GETCAPABILITIES_EN      1
// <q> ETH_MAC_Initialization
#define ETH_MAC_INITIALIZATION_EN       1
// <q> ETH_MAC_PowerControl
#define ETH_MAC_POWERCONTROL_EN         1
// <q> ETH_MAC_SetBusSpeed
#define ETH_MAC_SETBUSSPEED_EN          1
// <q> ETH_MAC_Config_Mode
#define ETH_MAC_CONFIG_MODE_EN          1
// <q> ETH_MAC_Config_CommonParams
#define ETH_MAC_CONFIG_COMMONPARAMS_EN  1
// <q> ETH_MAC_PTP_ControlTimer
#define ETH_MAC_PTP_CONTROLTIMER_EN     0
// <q> ETH_PHY_Initialization
#define ETH_PHY_INITIALIZATION_EN       1
// <q> ETH_PHY_PowerControl
#define ETH_PHY_POWERCONTROL_EN         1
// <q> ETH_PHY_Config
#define ETH_PHY_CONFIG_EN               1
// <q> ETH_Loopback_Transfer
#define ETH_LOOPBACK_TRANSFER_EN        1
// <q> ETH_Loopback_PTP
#define ETH_LOOPBACK_PTP_EN             0
// <q> ETH_PHY_CheckInvalidInit
#define ETH_PHY_CHECKINVALIDINIT_EN     0
// <q> ETH_MAC_CheckInvalidInit
#define ETH_MAC_CHECKINVALIDINIT_EN     0
// </h>
// </h>

// Buffer definitions
#define BUFFER_ELEM_1_32                0               // 0 = disable, 1 = enable
#define BUFFER_ELEM_512                 1               // 0 = disable, 1 = enable
#define BUFFER_ELEM_1024                0               // 0 = disable, 1 = enable
#define BUFFER_ELEM_4096                0               // 0 = disable, 1 = enable
#define BUFFER_ELEM_32768               0               // 0 = disable, 1 = enable
#define BUFFER_PATTERN                 {0x55, 0xAA}     // Buffer pattern

#endif /* DV_ETH_CONFIG_H_ */
