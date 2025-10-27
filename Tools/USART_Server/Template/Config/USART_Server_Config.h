/*
 * Copyright (c) 2020-2025 Arm Limited. All rights reserved.
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
 * Project:     USART Server
 * Title:       USART Server configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef  USART_SERVER_CONFIG_H_
#define  USART_SERVER_CONFIG_H_

#ifdef   CMSIS_target_header
#include CMSIS_target_header
#else
#define  ARDUINO_UNO_UART               0
#endif

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//------ With VS Code: Open Preview for Configuration Wizard -------------------

// <h> USART Server
//   <i> USART Server configuration.
//   <i> Fixed settings used by the USART Server for command exchange are:
//   <i> Baudrate: 115200
//   <i> Data Bits: 8
//   <i> Parity: None
//   <i> Stop Bits: 1
//   <i> Flow Control: None
//   <y> Driver_USART#
//     <i> Choose the Driver_USART# instance.
//     <i> For example to use Driver_USART0 select 0.
//     <h> Communication settings
//       <i> These settings specify configurable settings used by the USART Server for command exchange.
//       <o1> Mode
//         <i> Select mode setting used by the USART Server.
//         <1=> Asynchronous
//         <4=> Single-wire
//         <5=> IrDA
//     </h>
// </h>

#define  USART_SERVER_DRV_NUM           ARDUINO_UNO_UART
#define  USART_SERVER_MODE              1
#define  USART_SERVER_BUF_SIZE          4096
#define  USART_SERVER_CMD_TIMEOUT       100

#endif
