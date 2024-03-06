/*
 * Copyright (c) 2023 Arm Limited. All rights reserved.
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
 * Title:       General-Purpose Input/Output (GPIO) driver validation 
 *              configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_GPIO_CONFIG_H_
#define DV_GPIO_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> GPIO
//   <i> General-Purpose Input/Output (GPIO) driver validation configuration
//   <o0> Driver_GPIO# <0-255>
//     <i> Choose the Driver_GPIO# instance to test.
//     <i> For example to test Driver_GPIO0 select 0.
//   <h> Configuration
//     <i> Pins and Tests configuration.
//     <o1> Pin Under Test <0-255>
//       <i> Select pin to be tested.
//       <i> This pin should not have any external resistors or any external devices connected to it.
//     <o2> Auxiliary Pin
//       <i> Select Auxiliary Pin with serial low resistance resistor connected to Pin Under Test.
//       <i> Suggested resistance of this serial resistor is around 1 kOhm.
//       <i> This pin should not have any external resistors or any external devices connected to it.
//   </h>
//   <h> Tests
//     <i> Enable / disable tests.
//     <q3> GPIO_Setup
//       <i> Enable / disable Setup function tests.
//     <q4> GPIO_SetDirection
//       <i> Enable / disable SetDirection function tests.
//     <q5> GPIO_SetOutputMode
//       <i> Enable / disable SetOutputMode function tests.
//     <q6> GPIO_SetPullResistor
//       <i> Enable / disable SetPullResistor function tests.
//     <q7> GPIO_SetEventTrigger
//       <i> Enable / disable SetEventTrigger function tests.
//     <q8> GPIO_SetOutput
//       <i> Enable / disable SetOutput function tests.
//     <q9> GPIO_GetInput
//       <i> Enable / disable GetInput function tests.
//   </h>
// </h>

#define DRV_GPIO                        0
#define GPIO_CFG_PIN_UNDER_TEST         0
#define GPIO_CFG_PIN_AUX                0
#define GPIO_TC_SETUP_EN                1
#define GPIO_TC_SET_DIRECTION_EN        1
#define GPIO_TC_SET_OUTPUT_MODE_EN      1
#define GPIO_TC_SET_PULL_RESISTOR_EN    1
#define GPIO_TC_SET_EVENT_TRIGGER_EN    1
#define GPIO_TC_SET_OUTPUT_EN           1
#define GPIO_TC_GET_INPUT_EN            1

#endif /* DV_GPIO_CONFIG_H_ */
