/*
 * Copyright (c) 2015-2025 Arm Limited. All rights reserved.
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
 * Title:       Memory Card Interface (MCI) driver validation 
 *              configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_MCI_CONFIG_H_
#define DV_MCI_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//------ With VS Code: Open Preview for Configuration Wizard -------------------

// <h> MCI
// <i> Memory Card Interface (MCI) driver validation configuration
// <o> Driver_MCI# <0-255>
// <i> Choose the Driver_MCI# instance to test.
// <i> For example to test Driver_MCI0 select 0.
#define DRV_MCI                         0
// <h> Tests
// <i> Enable / disable tests.
// <q> MCI_GetCapabilities
#define MCI_GETCAPABILITIES_EN          1
// <q> MCI_Initialization
#define MCI_INITIALIZATION_EN           1
// <q> MCI_PowerControl
#define MCI_POWERCONTROL_EN             1
// <q> MCI_SetBusSpeedMode
#define MCI_SETBUSSPEEDMODE_EN          1
// <q> MCI_Config_DataWidth
#define MCI_CONFIG_DATAWIDTH_EN         1
// <q> MCI_Config_CmdLineMode
#define MCI_CONFIG_CMDLINEMODE_EN       1
// <q> MCI_Config_DriverStrength
#define MCI_CONFIG_DRIVERSTRENGTH_EN    1
// <q> MCI_CheckInvalidInit
#define MCI_CHECKINVALIDINIT_EN         1
// </h>
// </h>

#endif /* DV_MCI_CONFIG_H_ */
