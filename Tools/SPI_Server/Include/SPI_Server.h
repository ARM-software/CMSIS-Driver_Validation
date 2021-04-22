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
 * Title:       SPI Server header file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef SPI_SERVER_H_
#define SPI_SERVER_H_

#include <stdint.h>

#define SPI_SERVER_VER                 "1.1.0"

#define SPI_SERVER_STATE_RECEPTION      0
#define SPI_SERVER_STATE_EXECUTION      1
#define SPI_SERVER_STATE_TERMINATE      255


// Global functions
extern int32_t SPI_Server_Start (void);
extern int32_t SPI_Server_Stop  (void);

#endif
