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

#include <stdio.h>
#include <string.h>

#include "app_main.h"

#include "cmsis_os2.h"

#include "cmsis_dv.h"

#include "DV_WiFi_Config.h"

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (4096U)
static uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
static const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

/*---------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
void app_main_thread (void *argument) {
  (void)argument;

  // Check that necessary configuration in the DV_WiFi_Config.h file was done
  if (memcmp(WIFI_STA_SSID, "SSID", 5) == 0) {
    printf("Update Wi-Fi configuration in DV_WiFi_Config.h (WIFI_STA_SSID, WIFI_STA_PASS and WIFI_SOCKET_SERVER_IP) before running the tests!\n");
    while(1);
  }

  cmsis_dv(NULL);                       // Execute tests

  osThreadExit();
}

/*---------------------------------------------------------------------------
 * Application main function
 *---------------------------------------------------------------------------*/
int32_t app_main (void) {
  osKernelInitialize();
  osThreadNew(app_main_thread, NULL, &app_main_attr);
  osKernelStart();
  return 0;
}
