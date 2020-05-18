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
 * Title:       Controller Area Network (CAN) driver validation 
 *              configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_CAN_CONFIG_H_
#define DV_CAN_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> CAN
// <i> Controller Area Network (CAN) driver validation configuration
// <o> Driver_CAN# <0-255>
// <i> Choose the Driver_CAN# instance to test.
// <i> For example to test Driver_CAN0 select 0.
#define DRV_CAN                         0
// <h> CAN bitrates
// <i> Set the CAN bitrates (kbit/s)
// <i> Value zero is ignored
// <o> CAN bitrate 1
#define CAN_BR_1                        125
// <o> CAN bitrate 2
#define CAN_BR_2                        250
// <o> CAN bitrate 3
#define CAN_BR_3                        500
// <o> CAN bitrate 4
#define CAN_BR_4                        1000
// <o> CAN bitrate 5
#define CAN_BR_5                        0
// <o> CAN bitrate 6
#define CAN_BR_6                        0
// </h>
// <o> Ratio data/arbitration bitrate
// <i> Set the ratio between data and arbitration bitrate for CAN FD
#define CAN_DATA_ARB_RATIO              8
// <o> Percentual trigger for bitrate test
// <i> Set the minimum margin between measured and expected bitrates (%)
#define MIN_BITRATE                     10
// <o> Transfer timeout
// <i> Set the transfer timeout (us)
#define CAN_TRANSFER_TIMEOUT            1000000
// <h> Tests
// <i> Enable / disable tests.
// <q> CAN_GetCapabilities
#define CAN_GETCAPABILITIES_EN          1
// <q> CAN_Initialization
#define CAN_INITIALIZATION_EN           1
// <q> CAN_PowerControl
#define CAN_POWERCONTROL_EN             1
// <q> CAN_Loopback_CheckBitrate
#define CAN_LOOPBACK_CHECK_BR_EN        1
// <q> CAN_Loopback_CheckBitrateFD
#define CAN_LOOPBACK_CHECK_BR_FD_EN     1
// <q> CAN_Loopback_Transfer
#define CAN_LOOPBACK_TRANSFER_EN        1
// <q> CAN_Loopback_TransferFD
#define CAN_LOOPBACK_TRANSFER_FD_EN     1
// <q> CAN_CheckInvalidInit
#define CAN_CHECKINVALIDINIT_EN         1
// </h>
// </h>

#endif /* DV_CAN_CONFIG_H_ */
