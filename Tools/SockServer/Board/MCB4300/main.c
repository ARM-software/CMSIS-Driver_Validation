/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2019 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    main.c
 * Purpose: Socket tester using BSD sockets
 *----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "rl_net.h"
#include "SockServer.h"

#include "Board_LED.h"
#include "Board_GLCD.h"
#include "GLCD_Config.h"

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

static osThreadId_t display_id;

// Functions
static void app_main (void *argument);
static void DisplayServer (void *argument);

// IP address change notification
void netDHCP_Notify (uint32_t if_num, uint8_t opt, const uint8_t *val, uint32_t len) {
  if (opt == NET_DHCP_OPTION_IP_ADDRESS) {
    osThreadFlagsSet (display_id, 0x01);
  }
}


// LCD display handler thread
static void DisplayServer (void *argument) {
  uint8_t ip_addr[NET_ADDR_IP4_LEN];
  static char ip_ascii[16];
  static char buf[24];

  GLCD_Initialize         ();
  GLCD_SetBackgroundColor (GLCD_COLOR_BLUE);
  GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
  GLCD_ClearScreen        ();
  GLCD_SetFont            (&GLCD_Font_16x24);
  GLCD_DrawString         (0, 1*24, "    MW-Network      ");
  GLCD_DrawString         (0, 2*24, " Socket test server ");
  GLCD_DrawString         (0, 4*24, " ECHO:    port 7    ");
  GLCD_DrawString         (0, 5*24, " CHARGEN: port 19   ");
  GLCD_DrawString         (0, 6*24, " DISCARD: port 9    ");

  osDelay (100);

  while(1) {
    osThreadFlagsWait (0x01, osFlagsWaitAll, osWaitForever);
    netIF_GetOption (NET_IF_CLASS_ETH | 0,
                     netIF_OptionIP4_Address, ip_addr, sizeof(ip_addr));
    netIP_ntoa (NET_ADDR_IP4, ip_addr, ip_ascii, sizeof(ip_ascii));
    sprintf (buf, " IP=%-15s",ip_ascii);
    GLCD_DrawString (0, 8*24, buf);
  }
}

// Application main thread
static void app_main (void *argument) {

  osDelay (500);

  LED_Initialize ();
  netInitialize ();
  LED_On (0);

  osThreadNew(DgramServer, NULL, NULL);
  osThreadNew(StreamServer, NULL, NULL);
  osThreadNew(TestAssistant, NULL, NULL);
  display_id = osThreadNew (DisplayServer, NULL, NULL);
  osThreadFlagsSet (display_id, 0x01);
}

int main (void) {

  // System Initialization
  SystemCoreClockUpdate();

  osKernelInitialize();
  osThreadNew(app_main, NULL, NULL);
  osKernelStart();
  for (;;) {}
}
