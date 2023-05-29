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
 * Title:       Inter-Integrated Circuit (I2C) driver validation 
 *              configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_I2C_CONFIG_H_
#define DV_I2C_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> I2C
// <i> Inter-Integrated Circuit (I2C) driver validation configuration
// <o> Driver_I2C# <0-255>
// <i> Choose the Driver_I2C# instance to test.
// <i> For example to test Driver_I2C0 select 0.
#define DRV_I2C                         1
// <h> Tests
// <i> Enable / disable tests.
// <q> I2C_GetCapabilities
#define I2C_GETCAPABILITIES_EN          1
// <q> I2C_Initialization
#define I2C_INITIALIZATION_EN           1
// <q> I2C_PowerControl
#define I2C_POWERCONTROL_EN             1
// <q> I2C_SetBusSpeed
#define I2C_SETBUSSPEED_EN              1
// <q> I2C_SetOwnAddress
#define I2C_SETOWNADDRESS_EN            1
// <q> I2C_BusClear
#define I2C_BUSCLEAR_EN                 1
// <q> I2C_AbortTransfer
#define I2C_ABORTTRANSFER_EN            1
// <q> I2C_CheckInvalidInit
#define I2C_CHECKINVALIDINIT_EN         1
// </h>
// </h>

#endif /* DV_I2C_CONFIG_H_ */
