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
 * Title:       SPI Server hardware specific driver implementation
 *              for STMicroelectronics STM32F407 microcontroller
 *
 * -----------------------------------------------------------------------------
 */

#include "SPI_Server_HW.h"

#include "stm32f4xx_hal.h"


#define  GPIO_PORT_CLOCK_ENABLE()       __GPIOI_CLK_ENABLE()
#define  GPIO_PORT                      GPIOI
#define  GPIO_PIN                       GPIO_PIN_0

/**
  \fn            void SPI_Server_Pin_SS_Initialize (void)
  \brief         Initialize, power up and configure Slave Select pin as GPIO.
  \return        none
*/
void SPI_Server_Pin_SS_Initialize (void) {
  GPIO_InitTypeDef gpio_config;

  GPIO_PORT_CLOCK_ENABLE();

  /* Configure pin as GPIO output */
  gpio_config.Pin       = GPIO_PIN;
  gpio_config.Mode      = GPIO_MODE_OUTPUT_PP;
  gpio_config.Pull      = GPIO_NOPULL;
  gpio_config.Speed     = GPIO_SPEED_MEDIUM;
  gpio_config.Alternate = 0U;
  HAL_GPIO_Init(GPIO_PORT, &gpio_config);
}

/**
  \fn            void SPI_Server_Pin_SS_Uninitialize (void)
  \brief         Unconfigure, power down and uninitialize Slave Select pin.
  \return        none
*/
void SPI_Server_Pin_SS_Uninitialize (void) {
  HAL_GPIO_DeInit(GPIO_PORT, GPIO_PIN);
}

/**
  \fn            void SPI_Server_Pin_SS_SetState (uint32_t state)
  \brief         Set state of Slave Select pin.
  \param[in]     state          State to be set
                   - 0:    Drive pin to not active state
                   - != 0: Drive pin to active state
  \return        none
*/
void SPI_Server_Pin_SS_SetState (uint32_t state) {
  GPIO_PinState pin_state;

  if (state == 0U) {
    pin_state = GPIO_PIN_SET;
  } else {
    pin_state = GPIO_PIN_RESET;
  }

  HAL_GPIO_WritePin(GPIO_PORT, GPIO_PIN, pin_state);
}
