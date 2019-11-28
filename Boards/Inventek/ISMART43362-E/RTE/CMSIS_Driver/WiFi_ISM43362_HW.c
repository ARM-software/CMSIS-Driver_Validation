/* -----------------------------------------------------------------------------
 * Copyright (c) 2019 Arm Limited (or its affiliates). All rights reserved.
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
 *
 * $Date:        26. June 2019
 * $Revision:    V1.0
 *
 * Project:      WiFi Driver Hardware specific implementation for 
 *               Inventek ISM43362-M3G-L44 WiFi Module (SPI variant)
 * -------------------------------------------------------------------------- */

#include "WiFi_ISM43362_HW.h"

#include "board.h"
#include "pin_mux.h"

#include "fsl_gpio.h"


/**
  \fn          void WiFi_ISM43362_Pin_Initialize (void)
  \brief       Initialize pin(s).
  \return      none
*/
void WiFi_ISM43362_Pin_Initialize (void) {
}

/**
  \fn          void WiFi_ISM43362_Pin_Uninitialize (void)
  \brief       De-initialize pin(s).
  \return      none
*/
void WiFi_ISM43362_Pin_Uninitialize (void) {
}

/**
  \fn          void WiFi_ISM43362_Pin_RSTN (uint8_t rstn)
  \brief       Drive Reset line.
  \param[in]   rstn
                 - value = 0: Drive Reset line not active state
                 - value = 1: Drive Reset line active state
  \return      none
*/
void WiFi_ISM43362_Pin_RSTN (uint8_t rstn) {
  // NXP LPC55S69-EVK does not provide pin for driving reset of WiFi module 
  // reset is connected to push-button on the board and is not driveable by software
}

/**
  \fn          void WiFi_ISM43362_Pin_SSN (uint8_t ssn)
  \brief       Drive Slave Select line.
  \param[in]   ssn
                 - value = 0: Drive Slave Select line not active state
                 - value = 1: Drive Slave Select line active state
  \return      none
*/
void WiFi_ISM43362_Pin_SSN (uint8_t ssn) {
  GPIO_PinWrite(GPIO, 1U, 1U, ssn ? 0U : 1U);
}

/**
  \fn          uint8_t WiFi_ISM43362_Pin_DATARDY (void)
  \brief       Get Data Ready line state.
  \return      Data Ready line state
                 - 0: Data Ready line is not active state
                 - 1: Data Ready line is active state
*/
uint8_t WiFi_ISM43362_Pin_DATARDY (void) {
  return (GPIO_PinRead(GPIO, 1U, 5U));
}
