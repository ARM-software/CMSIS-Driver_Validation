/******************************************************************************
 * @file     vio.c
 * @brief    Virtual I/O implementation
 * @version  V1.0.0
 * @date     20. May 2022
 ******************************************************************************/
/*
 * Copyright (c) 2019-2022 Arm Limited. All rights reserved.
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
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "cmsis_vio.h"

#include "RTE_Components.h"             // Component selection
#include CMSIS_device_header

#if !defined CMSIS_VOUT || !defined CMSIS_VIN
// Add user includes here:
#include "stm32f429i_discovery.h"

#if !defined VIO_LCD_DISABLE
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_sdram.h"

#include "cmsis_os2.h"
#endif
#endif

// VIO input, output definitions
#define VIO_PRINT_MAX_SIZE      64U     // maximum size of print memory
#define VIO_PRINTMEM_NUM         4U     // number of print memories
#define VIO_VALUE_NUM            3U     // number of values
#define VIO_VALUEXYZ_NUM         3U     // number of XYZ values
#define VIO_IPV4_ADDRESS_NUM     2U     // number of IPv4 addresses
#define VIO_IPV6_ADDRESS_NUM     2U     // number of IPv6 addresses

// VIO input, output variables
__USED uint32_t      vioSignalIn;                                       // Memory for incoming signal
__USED uint32_t      vioSignalOut;                                      // Memory for outgoing signal
__USED char          vioPrintMem[VIO_PRINTMEM_NUM][VIO_PRINT_MAX_SIZE]; // Memory for the last value for each level
__USED int32_t       vioValue   [VIO_VALUE_NUM];                        // Memory for value used in vioGetValue/vioSetValue
__USED vioValueXYZ_t vioValueXYZ[VIO_VALUEXYZ_NUM];                     // Memory for XYZ value for 3-D vector
__USED vioAddrIPv4_t vioAddrIPv4[VIO_IPV4_ADDRESS_NUM];                 // Memory for IPv4 address value used in vioSetIPv4/vioGetIPv4
__USED vioAddrIPv6_t vioAddrIPv6[VIO_IPV6_ADDRESS_NUM];                 // Memory for IPv6 address value used in vioSetIPv6/vioGetIPv6

#if !defined CMSIS_VOUT
// Add global user types, variables, functions here:
#if !defined VIO_LCD_DISABLE
static osMutexId_t mid_mutLCD;

typedef struct displayArea {
  uint16_t   xOrigin;          // x Origin
  uint16_t   xWidth;           // x width
  uint16_t   yOrigin;          // y Origin
  uint16_t   yHeight;          // y height
  uint16_t   fontWidth;        // font width
  uint16_t   fontHeight;       // font height
} displayArea_t;

static displayArea_t display[4];

/**
  write a string to the selected display

  \param[in]   idx   Display index.
  \param[in]   str   String
*/
static void displayString (uint32_t idx, char *str) {
  char ch;
  uint8_t i = 0;
  uint16_t cursor_x, cursor_y;
  
  cursor_x = display[idx].xOrigin;
  cursor_y = display[idx].yOrigin;

  while (str[i] != '\0') {
    ch = str[i];                                            /* Get character and increase index */
    i++;

    switch (ch) {
      case 0x0A:                                            // Line Feed
        cursor_y += display[idx].fontHeight;                /* Move cursor one row down */
        if (cursor_y >= display[idx].yHeight) {             /* If bottom of display was overstepped */
          cursor_y = display[idx].yOrigin;                  /* Stay in last row */
        }
        break;
      case 0x0D:                                            /* Carriage Return */
        cursor_x = display[idx].xOrigin;                    /* Move cursor to first column */
        break;
      default:
        // Display character at current cursor position
        BSP_LCD_DisplayChar(cursor_x, cursor_y, ch);
        cursor_x += display[idx].fontWidth;                 /* Move cursor one column to right */
        if (cursor_x >= display[idx].xWidth) {              /* If last column was overstepped */
          cursor_x = display[idx].xOrigin;                  /* First column */
          cursor_y += display[idx].fontHeight;              /* Move cursor one row down */
          if (cursor_y >= display[idx].yHeight) {           /* If bottom of display was overstepped */
            cursor_y = display[idx].yOrigin;                /* Rollover to vertical origin */
          }
        }
        break;
    }
  }
}
#endif
#endif

#if !defined CMSIS_VIN
// Add global user types, variables, functions here:

#endif

// Initialize test input, output.
void vioInit (void) {
#if !defined CMSIS_VOUT
// Add user variables here:
#if !defined VIO_LCD_DISABLE
  // Create LCD mutex
  mid_mutLCD = osMutexNew(NULL);
  if (mid_mutLCD == NULL) { /* add error handling */ }

  // Initialize the LCD
  BSP_LCD_Init();

  // Initialize the LCD Layers
  BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER, LCD_FRAME_BUFFER);

  // Set LCD Foreground Layer
  BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);

  BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

  // Clear the LCD
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  BSP_LCD_Clear(LCD_COLOR_BLUE);

  // Initialize display areas
  display[vioLevelHeading].fontWidth  =  11;
  display[vioLevelHeading].fontHeight =  16;
  display[vioLevelHeading].xOrigin    =   3;
  display[vioLevelHeading].xWidth     = BSP_LCD_GetXSize() - 4;
  display[vioLevelHeading].yOrigin    =   4;
  display[vioLevelHeading].yHeight    =  2 * display[vioLevelHeading].fontHeight + display[vioLevelHeading].yOrigin;

  display[vioLevelNone].fontWidth     =  11;
  display[vioLevelNone].fontHeight    =  16;
  display[vioLevelNone].xOrigin       =   3;
  display[vioLevelNone].xWidth        = BSP_LCD_GetXSize() - 4;
  display[vioLevelNone].yOrigin       =  40;
  display[vioLevelNone].yHeight       =  2 * display[vioLevelNone].fontHeight + display[vioLevelNone].yOrigin;

  display[vioLevelError].fontWidth    =  11;
  display[vioLevelError].fontHeight   =  16;
  display[vioLevelError].xOrigin      =   3;
  display[vioLevelError].xWidth       = BSP_LCD_GetXSize() - 4;
  display[vioLevelError].yOrigin      =  68;
  display[vioLevelError].yHeight      =  2 * display[vioLevelError].fontHeight + display[vioLevelError].yOrigin;

  display[vioLevelMessage].fontWidth  =  11;
  display[vioLevelMessage].fontHeight =  16;
  display[vioLevelMessage].xOrigin    =   3;
  display[vioLevelMessage].xWidth     = BSP_LCD_GetXSize() - 4;
  display[vioLevelMessage].yOrigin    = 120;
  display[vioLevelMessage].yHeight    =  2 * display[vioLevelMessage].fontHeight + display[vioLevelMessage].yOrigin;

#endif

#endif
#if !defined CMSIS_VIN
// Add user variables here:

#endif

  vioSignalIn  = 0U;
  vioSignalOut = 0U;

  memset (vioPrintMem, 0, sizeof(vioPrintMem));
  memset (vioValue,    0, sizeof(vioValue));
  memset (vioValueXYZ, 0, sizeof(vioValueXYZ));
  memset (vioAddrIPv4, 0, sizeof(vioAddrIPv4));
  memset (vioAddrIPv6, 0, sizeof(vioAddrIPv6));

#if !defined CMSIS_VOUT
// Add user code here:

#endif

#if !defined CMSIS_VIN
// Add user code here:

#endif
}

// Set signal output.
void vioSetSignal (uint32_t mask, uint32_t signal) {
#if !defined CMSIS_VOUT
// Add user variables here:

#endif

  vioSignalOut &= ~mask;
  vioSignalOut |=  mask & signal;

#if !defined CMSIS_VOUT
// Add user code here:

#endif
}

// Get signal input.
uint32_t vioGetSignal (uint32_t mask) {
  uint32_t signal;
#if !defined CMSIS_VIN
// Add user variables here:

#endif

#if !defined CMSIS_VIN
// Add user code here:

//   vioSignalIn = ...;
#endif

  signal = vioSignalIn;

  return (signal & mask);
}

// Set value output.
void vioSetValue (uint32_t id, int32_t value) {
  uint32_t index = id;
#if !defined CMSIS_VOUT
// Add user variables here:

#endif

  if (index >= VIO_VALUE_NUM) {
    return;                             /* return in case of out-of-range index */
  }

  vioValue[index] = value;

#if !defined CMSIS_VOUT
// Add user code here:

#endif
}

// Get value input.
int32_t vioGetValue (uint32_t id) {
  uint32_t index = id;
  int32_t  value = 0;
#if !defined CMSIS_VIN
// Add user variables here:

#endif

  if (index >= VIO_VALUE_NUM) {
    return value;                       /* return default in case of out-of-range index */
  }

#if !defined CMSIS_VIN
// Add user code here:

//   vioValue[index] = ...;
#endif

  value = vioValue[index];

  return value;
}

// Set XYZ value output.
void vioSetXYZ (uint32_t id, vioValueXYZ_t valueXYZ) {
  uint32_t index = id;
#if !defined CMSIS_VOUT
// Add user variables here:

#endif

  if (index >= VIO_VALUEXYZ_NUM) {
    return;                             /* return in case of out-of-range index */
  }

  vioValueXYZ[index] = valueXYZ;

#if !defined CMSIS_VOUT
// Add user code here:

#endif
}

// Get XYZ value input.
vioValueXYZ_t vioGetXYZ (uint32_t id) {
  uint32_t index = id;
  vioValueXYZ_t valueXYZ = {0, 0, 0};
#if !defined CMSIS_VIN
// Add user variables here:

#endif

  if (index >= VIO_VALUEXYZ_NUM) {
    return valueXYZ;                    /* return default in case of out-of-range index */
  }

#if !defined CMSIS_VIN
// Add user code here:

//   vioValueXYZ[index] = ...;
#endif

  valueXYZ = vioValueXYZ[index];

  return valueXYZ;
}

// Set IPv4 address output.
void vioSetIPv4 (uint32_t id, vioAddrIPv4_t addrIPv4) {
  uint32_t index = id;
#if !defined CMSIS_VOUT
// Add user variables here:

#endif

  if (index >= VIO_IPV4_ADDRESS_NUM) {
    return;                             /* return in case of out-of-range index */
  }

  vioAddrIPv4[index] = addrIPv4;

#if !defined CMSIS_VOUT
// Add user code here:

#endif
}

// Get IPv4 address input.
vioAddrIPv4_t vioGetIPv4 (uint32_t id) {
  uint32_t index = id;
  vioAddrIPv4_t addrIPv4 = {0U, 0U, 0U, 0U};
#if !defined CMSIS_VIN
// Add user variables here:

#endif

  if (index >= VIO_IPV4_ADDRESS_NUM) {
    return addrIPv4;                    /* return default in case of out-of-range index */
  }

#if !defined CMSIS_VIN
// Add user code here:

//   vioAddrIPv4[index] = ...;
#endif

  addrIPv4 = vioAddrIPv4[index];

  return addrIPv4;
}

// Set IPv6 address output.
void vioSetIPv6 (uint32_t id, vioAddrIPv6_t addrIPv6) {
  uint32_t index = id;
#if !defined CMSIS_VOUT
// Add user variables here:

#endif

  if (index >= VIO_IPV6_ADDRESS_NUM) {
    return;                             /* return in case of out-of-range index */
  }

  vioAddrIPv6[index] = addrIPv6;

#if !defined CMSIS_VOUT
// Add user code here:

#endif
}

// Get IPv6 address input.
vioAddrIPv6_t vioGetIPv6 (uint32_t id) {
  uint32_t index = id;
  vioAddrIPv6_t addrIPv6 = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
                            0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
#if !defined CMSIS_VIN
// Add user variables here:

#endif

  if (index >= VIO_IPV6_ADDRESS_NUM) {
    return addrIPv6;                    /* return default in case of out-of-range index */
  }

#if !defined CMSIS_VIN
// Add user code here:

//   vioAddrIPv6[index] = ...;
#endif

  addrIPv6 = vioAddrIPv6[index];

  return addrIPv6;
}

// Print formated string to test terminal.
int32_t vioPrint (uint32_t level, const char *format, ...) {
  va_list args;
  int32_t ret;
#if !defined CMSIS_VOUT
// Add user variables here:

#endif

  if (level > vioLevelError) {
    return (-1);
  }

  if (level > VIO_PRINTMEM_NUM) {
    return (-1);
  }

  va_start(args, format);

  ret = vsnprintf((char *)vioPrintMem[level], sizeof(vioPrintMem[level]), format, args);

  va_end(args);

#if !defined CMSIS_VOUT
#if !defined VIO_LCD_DISABLE
// Draw LCD level
  osMutexAcquire(mid_mutLCD, 0xFF);
  switch (level) {
    case vioLevelNone:
      BSP_LCD_SetFont(&Font16);
      BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
      displayString(level, (char *)vioPrintMem[level]);
      break;
    case vioLevelHeading:
      BSP_LCD_SetFont(&Font16);
      BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
      displayString(level, (char *)vioPrintMem[level]);
      break;
    case vioLevelMessage:
      BSP_LCD_SetFont(&Font16);
      BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
      displayString(level, (char *)vioPrintMem[level]);
      break;
    case vioLevelError:
      BSP_LCD_SetFont(&Font16);
      BSP_LCD_SetTextColor(LCD_COLOR_RED);
      displayString(level, (char *)vioPrintMem[level]);
      break;
  }

  osMutexRelease(mid_mutLCD);
#endif
#endif

  return (ret);
}
