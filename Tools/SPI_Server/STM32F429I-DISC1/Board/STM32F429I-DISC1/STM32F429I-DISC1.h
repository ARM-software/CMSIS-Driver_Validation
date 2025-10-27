/*---------------------------------------------------------------------------
 * Copyright (c) 2025 Arm Limited (or its affiliates).
 * All rights reserved.
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
 *---------------------------------------------------------------------------*/

#ifndef STM32F429I_DISC1_H_
#define STM32F429I_DISC1_H_

#include "Driver_SPI.h"
#include "Driver_USART.h"

// CMSIS Driver instances of Board peripherals
#define CMSIS_DRIVER_SPI    1           // Driver_SPI1
#define CMSIS_DRIVER_USART  1           // Driver_USART1

// CMSIS Driver instance for STDIO retarget
#define RETARGET_STDIO_UART 2

// CMSIS Drivers
extern ARM_DRIVER_SPI       ARM_Driver_SPI_(CMSIS_DRIVER_SPI);          // SPI1
extern ARM_DRIVER_USART     ARM_Driver_USART_(CMSIS_DRIVER_USART);      // USART1
extern ARM_DRIVER_USART     ARM_Driver_USART_(RETARGET_STDIO_UART);     // STDIO retargeted USART2

#ifdef   CMSIS_shield_header
#include CMSIS_shield_header
#endif

#endif // STM32F429I_DISC1_H_
