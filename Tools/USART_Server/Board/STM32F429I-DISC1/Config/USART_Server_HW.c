/*
 * Copyright (c) 2020-2022 Arm Limited. All rights reserved.
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
 * Title:       USART Server hardware specific driver implementation
 *
 * -----------------------------------------------------------------------------
 */

#include "USART_Server_HW.h"

#include "stm32f4xx_hal.h"

#define   DCD_CLK_ENABLE()              __GPIOA_CLK_ENABLE()
#define   DCD_PORT                      GPIOA
#define   DCD_PIN                       GPIO_PIN_13

#define   RI_CLK_ENABLE()               __GPIOA_CLK_ENABLE()
#define   RI_PORT                       GPIOA
#define   RI_PIN                        GPIO_PIN_14

/**
  \fn            void USART_Server_Pins_Initialize (void)
  \brief         Initialize, power up and configure GPIO pins used for 
                 driving DCD and RI lines of the USART Client.
  \return        none
*/
void USART_Server_Pins_Initialize (void) {
  GPIO_InitTypeDef gpio_config;

  DCD_CLK_ENABLE();

  gpio_config.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio_config.Pull  = GPIO_NOPULL;
  gpio_config.Speed = GPIO_SPEED_LOW;

  /* Configure GPIO pin: DCD as output */
  gpio_config.Pin   = DCD_PIN;
  HAL_GPIO_Init(DCD_PORT, &gpio_config);

  /* Configure GPIO pin: RI as output */
  gpio_config.Pin   = RI_PIN;
  HAL_GPIO_Init(DCD_PORT, &gpio_config);
}

/**
  \fn            void USART_Server_Pins_Uninitialize (void)
  \brief         Unconfigure, power down and uninitialize GPIO pins used for
                 driving DCD and RI lines of the USART Client.
  \return        none
*/
void USART_Server_Pins_Uninitialize (void) {
  HAL_GPIO_DeInit(DCD_PORT, DCD_PIN);
  HAL_GPIO_DeInit(DCD_PORT, RI_PIN);
}

/**
  \fn            void USART_Server_Pin_DCD_SetState (uint32_t state)
  \brief         Set state of GPIO pin used for driving DCD line of the USART Client.
  \param[in]     state          State to be set
                   - 0:    Drive pin to not active state
                   - != 0: Drive pin to active state
  \return        none
*/
void USART_Server_Pin_DCD_SetState (uint32_t state) {

  // DCD is active low
  HAL_GPIO_WritePin(DCD_PORT, DCD_PIN, ((state != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET));
}

/**
  \fn            void USART_Server_Pin_RI_SetState (uint32_t state)
  \brief         Set state of GPIO pin used for driving RI line of the USART Client.
  \param[in]     state          State to be set
                   - 0:    Drive pin to not active state
                   - != 0: Drive pin to active state
  \return        none
*/
void USART_Server_Pin_RI_SetState (uint32_t state) {

  // RI is active low
  HAL_GPIO_WritePin(RI_PORT, RI_PIN, ((state != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET));
}
