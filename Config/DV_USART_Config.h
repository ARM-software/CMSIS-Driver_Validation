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
 * Title:       Universal Synchronous Asynchronous Receiver/Transmitter (USART) 
 *              driver validation configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_USART_CONFIG_H_
#define DV_USART_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> USART
// <i> Universal Synchronous Asynchronous Receiver/Transmitter (USART) driver validation configuration
// <o> Driver_USART# <0-255>
// <i> Choose the Driver_USART# instance to test.
// <i> For example to test Driver_USART0 select 0.
#define DRV_USART                       0
// <h> USART baudrates
// <i> Set the USART baudrates (bps)
// <i> Value zero is ignored
// <o> USART baudrate 1
#define USART_BR_1                      9600
// <o> USART baudrate 2
#define USART_BR_2                      19200
// <o> USART baudrate 3
#define USART_BR_3                      38400
// <o> USART baudrate 4
#define USART_BR_4                      57600
// <o> USART baudrate 5
#define USART_BR_5                      115200
// <o> USART baudrate 6
#define USART_BR_6                      921600
// </h>
// <o> Percentual tolerance for baudrate test
// <i> Set the tolerance between measured and expected baudrates (%)
#define TOLERANCE_BR                    5
// <o> USART data bits
// <i> Set the USART data bits
#define USART_DATA_BITS                 8
// <o> Transfer timeout
// <i> Set the transfer timeout (us)
#define USART_TRANSFER_TIMEOUT          1000000
// <h> Tests
// <i> Enable / disable tests.
// <q> USART_GetCapabilities
#define USART_GETCAPABILITIES_EN        1
// <q> USART_Initialization
#define USART_INITIALIZATION_EN         1
// <q> USART_PowerControl
#define USART_POWERCONTROL_EN           1
// <q> USART_Config_PolarityPhase
#define USART_CONFIG_POLARITYPHASE_EN   1
// <q> USART_Config_DataBits
#define USART_CONFIG_DATABITS_EN        1
// <q> USART_Config_StopBits
#define USART_CONFIG_STOPBITS_EN        1
// <q> USART_Config_Parity
#define USART_CONFIG_PARITY_EN          1
// <q> USART_Config_Baudrate
#define USART_CONFIG_BAUDRATE_EN        1
// <q> USART_Config_CommonParams
#define USART_CONFIG_COMMONPARAMS_EN    1
// <q> USART_Send
#define USART_SEND_EN                   1
// <q> USART_AsynchronousReceive
#define USART_ASYNCHRONOUSRECEIVE_EN    1
// <q> USART_Loopback_CheckBaudrate
#define USART_LOOPBACK_CHECKBAUDRATE_EN 1
// <q> USART_Loopback_Transfer
#define USART_LOOPBACK_TRANSFER_EN      1
// <q> USART_CheckInvalidInit
#define USART_CHECKINVALIDINIT_EN       1
// </h>
// </h>

// Buffer definitions
#define BUFFER_ELEM_1_32                0               // 0 = disable, 1 = enable
#define BUFFER_ELEM_512                 1               // 0 = disable, 1 = enable
#define BUFFER_ELEM_1024                0               // 0 = disable, 1 = enable
#define BUFFER_ELEM_4096                0               // 0 = disable, 1 = enable
#define BUFFER_ELEM_32768               0               // 0 = disable, 1 = enable
#define BUFFER_SIZE_BR                  512             // Buffer size for baudrate tests
#define BUFFER_PATTERN                 {0x55, 0xAA}     // Buffer pattern

// Device specific local loopback settings
#define USART_LOCAL_LOOPBACK()

#endif /* DV_USART_CONFIG_H_ */
