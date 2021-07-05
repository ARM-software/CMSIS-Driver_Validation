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
 * Project:     USART Server
 * Title:       USART Server hardware specific driver implementation template
 *
 * -----------------------------------------------------------------------------
 */

#include "USART_Server_HW.h"

#include <stdint.h>

// Add device specific include files here
// <code USART_Server_include_files>

// </code>



/**
  \fn            void USART_Server_Pins_Initialize (void)
  \brief         Initialize, power up and configure GPIO pins used for 
                 driving DCD and RI lines of the USART Client.
  \return        none
*/
void USART_Server_Pins_Initialize (void) {
  // Add code for initializing pins for driving DCD and RI lines of the USART Client here
  // <code USART_Server_Pins_Initialize>

  // </code>
}

/**
  \fn            void USART_Server_Pins_Uninitialize (void)
  \brief         Unconfigure, power down and uninitialize GPIO pins used for
                 driving DCD and RI lines of the USART Client.
  \return        none
*/
void USART_Server_Pins_Uninitialize (void) {
  // Add code for uninitializing pins driving DCD and RI lines of the USART Client here
  // <code USART_Server_Pins_Uninitialize>

  // </code>
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
  // Add code for driving DCD line of the USART Client here
  // <code USART_Server_Pin_DCD_SetState>

  // </code>
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
  // Add code for driving RI line of the USART Client here
  // <code USART_Server_Pin_RI_SetState>

  // </code>
}
