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
 * Title:       Universal Serial Bus (USB) Host driver validation 
 *              configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_USBH_CONFIG_H_
#define DV_USBH_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> USB Host
// <i> Universal Serial Bus (USB) Host driver validation configuration
// <o> Driver_USBH# <0-255>
// <i> Choose the Driver_USBH# instance to test.
// <i> For example to test Driver_USBH0 select 0.
#define DRV_USBH                        0
// <h> Tests
// <i> Enable / disable tests.
// <q> USBH_GetCapabilities
#define USBH_GETCAPABILITIES_EN         1
// <q> USBH_Initialization
#define USBH_INITIALIZATION_EN          1
// <q> USBH_PowerControl
#define USBH_POWERCONTROL_EN            1
// <q> USBH_CheckInvalidInit
#define USBH_CHECKINVALIDINIT_EN        1
// </h>
// </h>

#endif /* DV_USBH_CONFIG_H_ */
