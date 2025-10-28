/*---------------------------------------------------------------------------
 * Copyright (c) 2024-2025 Arm Limited (or its affiliates).
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

#ifndef B_U585I_IOT02A_H_
#define B_U585I_IOT02A_H_

#include "GPIO_STM32.h"
#include "Driver_I2C.h"
#include "Driver_SPI.h"
#include "Driver_USART.h"
#include "Driver_USBD.h"
#include "Driver_USBH.h"
#include "Driver_WiFi.h"
#include "cmsis_vstream.h"

// Arduino Connector Digital I/O
#define ARDUINO_UNO_D2      GPIO_PIN_ID_PORTD(15U)
#define ARDUINO_UNO_D3      GPIO_PIN_ID_PORTB(2U)
#define ARDUINO_UNO_D4      GPIO_PIN_ID_PORTE(7U)
#define ARDUINO_UNO_D5      GPIO_PIN_ID_PORTE(0U)
#define ARDUINO_UNO_D6      GPIO_PIN_ID_PORTB(6U)
#define ARDUINO_UNO_D7      GPIO_PIN_ID_PORTF(13U)
#define ARDUINO_UNO_D8      GPIO_PIN_ID_PORTC(1U)
#define ARDUINO_UNO_D9      GPIO_PIN_ID_PORTA(8U)
#define ARDUINO_UNO_D10     GPIO_PIN_ID_PORTE(12U)
#define ARDUINO_UNO_D14     GPIO_PIN_ID_PORTC(0U)
#define ARDUINO_UNO_D15     GPIO_PIN_ID_PORTC(2U)
#define ARDUINO_UNO_D16     GPIO_PIN_ID_PORTC(4U)
#define ARDUINO_UNO_D17     GPIO_PIN_ID_PORTC(5U)
#define ARDUINO_UNO_D18     GPIO_PIN_ID_PORTA(7U)
#define ARDUINO_UNO_D19     GPIO_PIN_ID_PORTB(0U)

// CMSIS Driver instances on Arduino Connector
#define ARDUINO_UNO_I2C     1           // I2C1
#define ARDUINO_UNO_SPI     1           // SPI1
#define ARDUINO_UNO_UART    3           // USART3

// CMSIS Driver instances of Board peripherals
#define CMSIS_DRIVER_USART  2           // Driver_USART2
#define CMSIS_DRIVER_USBD   0           // Driver_USBD0
#define CMSIS_DRIVER_WIFI   0           // Driver_WiFi0

// CMSIS Driver instance for STDIO retarget
#define RETARGET_STDIO_UART 1

// CMSIS Drivers
extern ARM_DRIVER_I2C       ARM_Driver_I2C_(ARDUINO_UNO_I2C);           // I2C
extern ARM_DRIVER_SPI       ARM_Driver_SPI_(ARDUINO_UNO_SPI);           // SPI
extern ARM_DRIVER_USART     ARM_Driver_USART_(ARDUINO_UNO_UART);        // UART
extern ARM_DRIVER_USART     ARM_Driver_USART_(CMSIS_DRIVER_USART);      // STMod UART
extern ARM_DRIVER_USART     ARM_Driver_USART_(RETARGET_STDIO_UART);     // ST-Link
extern ARM_DRIVER_USBD      ARM_Driver_USBD_(CMSIS_DRIVER_USBD);        // USB Device
extern ARM_DRIVER_WIFI      ARM_Driver_WiFi_(CMSIS_DRIVER_WIFI);        // WiFi
extern vStreamDriver_t      Driver_vStreamAccelerometer;                // vStream Accelerometer
extern vStreamDriver_t      Driver_vStreamAudioIn;                      // vStream Audio In

#ifdef   CMSIS_shield_header
#include CMSIS_shield_header
#endif

#endif // B_U585I_IOT02A_H_
