/*
 * Copyright (c) 2020-2021 Arm Limited. All rights reserved.
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
//   <i> SPI Server configuration.
//   <i> Fixed settings used by the SPI Server for command exchange are:
//   <i> Mode: Slave with Slave Select Hardware monitored
//   <i> Clock / Frame Format: Clock Polarity 0, Clock Phase 0
//   <i> Data Bits: 8
//   <i> Bit Order: MSB to LSB
//   <o0> Driver_SPI# <0-255>
//     <i> Choose the Driver_SPI# instance.
//     <i> For example to use Driver_SPI0 select 0.
// </h>

#define  SPI_SERVER_DRV_NUM             0
#define  SPI_SERVER_BUF_SIZE            4096
#define  SPI_SERVER_CMD_TIMEOUT         100

#endif
