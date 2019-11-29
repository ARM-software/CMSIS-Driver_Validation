/*------------------------------------------------------------------------------
 * MDK Middleware
 * Copyright (c) 2004-2019 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    retarget_user.c
 * Purpose: User specific retarget of stdin, stdout and stderr
 *----------------------------------------------------------------------------*/

#include <stdint.h>
#include "fsl_debug_console.h"          // NXP::Device:SDK Utilities:debug_console

int DbgConsole_SendDataReliable(uint8_t *ch, size_t size);
int DbgConsole_ReadCharacter(uint8_t *ch);

/**
  Put a character to the stderr
 
  \param[in]   ch  Character to output
  \return          The character written, or -1 on write error.
*/
int stderr_putchar (int ch) {
  int32_t ret;

  ret = DbgConsole_SendDataReliable((uint8_t *)(&ch), 1);

  if (ret!= -1) {
    ret = ch;
  }
  return (ret);
}

/**
  Put a character to the stdout
 
  \param[in]   ch  Character to output
  \return          The character written, or -1 on write error.
*/
int stdout_putchar (int ch) {
  int32_t ret;

  ret = DbgConsole_SendDataReliable((uint8_t *)(&ch), 1);

  if (ret!= -1) {
    ret = ch;
  }
  return (ret);
}

/**
  Get a character from the stdio
 
  \return     The next character from the input, or -1 on read error.
*/
int stdin_getchar (void) {
  int32_t ret;
  uint8_t ch;

  ret = DbgConsole_ReadCharacter(&ch);

  if (ret!= -1) {
    ret = ch;
  }
  return (ret);
}
