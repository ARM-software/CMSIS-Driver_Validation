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
 * Title:       SPI Server hardware specific driver implementation template
 *
 * -----------------------------------------------------------------------------
 */

#include "SPI_Server_HW.h"

// Add device specific include files here
// <code SPI_Server_include_files>

// </code>



/**
  \fn            void SPI_Server_Pin_SS_Initialize (void)
  \brief         Initialize, power up and configure Slave Select pin as GPIO.
  \return        none
*/
void SPI_Server_Pin_SS_Initialize (void) {
  // Add code for initializing Slave Select pin here
  // <code SPI_Server_Pin_SS_Initialize>

  // </code>
}

/**
  \fn            void SPI_Server_Pin_SS_Uninitialize (void)
  \brief         Unconfigure, power down and uninitialize Slave Select pin.
  \return        none
*/
void SPI_Server_Pin_SS_Uninitialize (void) {
  // Add code for uninitializing Slave Select pin here
  // <code SPI_Server_Pin_SS_Uninitialize>

  // </code>
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
  // Add code for driving Slave Select pin here
  // <code SPI_Server_Pin_SS_SetState>

  // </code>
}
