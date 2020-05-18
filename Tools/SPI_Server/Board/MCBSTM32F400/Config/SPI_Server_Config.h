/*
 * Copyright (c) 2020 Arm Limited. All rights reserved.
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
 * Project:     SPI Server
 * Title:       SPI Server configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef  SPI_SERVER_CONFIG_H_
#define  SPI_SERVER_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> SPI Server
// <i> SPI Server configuration.
//   <o0> Driver_SPI# <0-255>
//   <i> Choose the Driver_SPI# instance.
//   <i> For example to use Driver_SPI0 select 0.
//   <h> Communication settings
//     <i> These settings specify SPI communication interface configuration of the SPI Server used for command / response exchange with the SPI Client.
//     <o1.0..3> Clock / Frame Format
//     <i> Select clock / frame format setting used by the SPI Server.
//       <0=> Clock Polarity 0, Clock Phase 0
//       <1=> Clock Polarity 0, Clock Phase 1
//       <2=> Clock Polarity 1, Clock Phase 0
//       <3=> Clock Polarity 1, Clock Phase 1
//       <4=> Texas Instruments Frame Format
//       <5=> National Semiconductor Microwire Frame Format
//     <o2> Data Bits <8=> 8 <16=> 16
//     <i> Select data bit setting used by the SPI Server.
//     <o3> Bit Order <0=> MSB to LSB <1=> LSB to MSB
//     <i> Select bit order setting used by the SPI Server.
//   </h>
// </h>

#define  SPI_SERVER_DRV_NUM             2
#define  SPI_SERVER_FORMAT              0
#define  SPI_SERVER_DATA_BITS           8
#define  SPI_SERVER_BIT_ORDER           0
#define  SPI_SERVER_BUF_SIZE            4096
#define  SPI_SERVER_CMD_TIMEOUT         100

#endif
