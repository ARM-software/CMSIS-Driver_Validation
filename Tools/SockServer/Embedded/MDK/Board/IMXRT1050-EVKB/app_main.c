/*---------------------------------------------------------------------------
 * Copyright (c) 2022 Arm Limited (or its affiliates). All rights reserved.
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
 *      Name:    app_main.c
 *      Purpose: Application main template
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include "main.h"

#include "cmsis_os2.h"

#include "rl_net.h"
#include "SockServer.h"

static const osThreadAttr_t app_main_attr = {
  .stack_size = 4096U
};

static osThreadId_t NetStatus_id;

// IP address change notification
void netDHCP_Notify (uint32_t if_num, uint8_t opt, const uint8_t *val, uint32_t len) {
  if (opt == NET_DHCP_OPTION_IP_ADDRESS) {
    osThreadFlagsSet (NetStatus_id, 0x01);
  }
}

// Network status output thread
static void NetStatus (void *argument) {
  uint8_t ip_addr[NET_ADDR_IP4_LEN];
  static char ip_ascii[16];
  static char buf[32];

  printf ("Socket test server is UP!\r\n\r\n");
  printf ("Services\r\n");
  printf (" ECHO:    port 7\r\n");
  printf (" CHARGEN: port 19\r\n");
  printf (" DISCARD: port 9\r\n\r\n");

  osDelay (100);

  while(1) {
    osThreadFlagsWait (0x01, osFlagsWaitAll, osWaitForever);

    netIF_GetOption (NET_IF_CLASS_ETH | 0, netIF_OptionIP4_Address, ip_addr, sizeof(ip_addr));
    netIP_ntoa (NET_ADDR_IP4, ip_addr, ip_ascii, sizeof(ip_ascii));

    /* Printf out server listen IP */
    sprintf (buf, "Server IP=%-15s",ip_ascii);
    printf ("%s\r\n", buf);
  }
}

/*---------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
static void app_main (void *argument) {
  (void)argument;

  netInitialize();
  osDelay(500U);

  osThreadNew(DgramServer, NULL, NULL);
  osThreadNew(StreamServer, NULL, NULL);
  osThreadNew(TestAssistant, NULL, NULL);

  NetStatus_id = osThreadNew (NetStatus, NULL, NULL);
  osThreadFlagsSet (NetStatus_id, 0x01);

  // Add user code here:
  for (;;) {}
}

/*---------------------------------------------------------------------------
 * Application initialization
 *---------------------------------------------------------------------------*/
void app_initialize (void) {
  osThreadNew(app_main, NULL, &app_main_attr);
}
