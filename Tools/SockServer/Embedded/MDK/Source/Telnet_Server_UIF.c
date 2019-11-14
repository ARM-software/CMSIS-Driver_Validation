/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    Telnet_Server_UIF.c
 * Purpose: Telnet Server User Interface
 * Rev.:    V7.0.0
 *----------------------------------------------------------------------------*/
//! [code_Telnet_Server_UIF]
#include <stdio.h>
#include <string.h>
#include "rl_net.h"

// ANSI ESC Sequences for terminal control
#define CLS     "\033[2J"

// Global variables
extern SOCKADDR_IN remote_addr;
extern uint32_t rx_cnt;
extern uint32_t tx_cnt;

static const char help[] = {
  "\r\n"
  "Commands:\r\n"
  "  stat        - print Server status\r\n"
  "  clear       - clear RX/TX counters\r\n"
  "  cls         - clear screen\r\n"
  "  help, ?     - display this help\r\n"
  "  <BS>        - delete Character left\r\n"
  "  <UP>,<DOWN> - recall Command History\r\n"
  "  bye,<ESC>,^C- disconnect"
};

// \brief Request a message for a Telnet server session.
// \param[in]     msg           code of requested message.
// \param[out]    buf           output buffer to write the message to.
// \param[in]     buf_len       length of the output buffer in bytes.
// \return        number of bytes written to output buffer.
uint32_t netTELNETs_ProcessMessage (netTELNETs_Message msg, char *buf, uint32_t buf_len) {
  uint32_t len = 0;
 
  switch (msg) {
    case netTELNETs_MessageWelcome:
      // Initial welcome message
      len = sprintf (buf, "\r\n"
                          "*** SockServer ***\r\n"
                          "%s",help);
      break;
 
    case netTELNETs_MessagePrompt:
      // Prompt message
      len = sprintf (buf, "\r\n"
                          "Cmd> ");
      break;

    default:
      break;
  }
  return (len);
}
 
// \brief Process and execute a command requested by the Telnet client.
// \param[in]     cmd           pointer to command string from Telnet client.
// \param[out]    buf           output buffer to write the return message to.
// \param[in]     buf_len       length of the output buffer in bytes.
// \param[in,out] pvar          pointer to a session's local buffer of 4 bytes.
//                              - 1st call = cleared to 0,
//                              - 2nd call = not altered by the system,
//                              - 3rd call = not altered by the system, etc.
// \return        number of bytes written to output buffer.
//                - return len | (1u<<31) = repeat flag, the system calls this function
//                                          again for the same command.
//                - return len | (1u<<30) = disconnect flag, the system disconnects
//                                          the Telnet client.
uint32_t netTELNETs_ProcessCommand (const char *cmd, char *buf, uint32_t buf_len, uint32_t *pvar) {
  uint32_t len = 0;

  // Command line parser
  if (netTELNETs_CheckCommand (cmd, "STAT") == true) {
    // Display SockServer status
    if (*pvar == 0) {
      // Confirm the command
      *pvar = 1;
      len = (uint32_t)sprintf (buf, " Ok\r\n\n");
    }
    else {
      // Update status
      len  = sprintf (buf,     "\r");
      len += sprintf (buf+len, "IP=%s, ",   inet_ntoa(remote_addr.sin_addr));
      len += sprintf (buf+len, "port=%u, ", ntohs(remote_addr.sin_port));
      len += sprintf (buf+len, "rx=%u, ",   rx_cnt);
      len += sprintf (buf+len, "tx=%u",     tx_cnt);
    }
    // Update interval 10 ticks (1 second)
    netTELNETs_RepeatCommand (10);
    return (len | (1u << 31));
  }

  if (netTELNETs_CheckCommand (cmd, "CLEAR") == true) {
    // Clear system counters and IP address
    rx_cnt = 0;
    tx_cnt = 0;
    memset (&remote_addr, 0, sizeof(remote_addr));
    len = (uint32_t)sprintf (buf, " Ok");
    return (len);
  }

  if (netTELNETs_CheckCommand (cmd, "CLS") == true) {
    // Clear the client screen
    len = (uint32_t)sprintf (buf, CLS);
    return (len);
  }

  if (netTELNETs_CheckCommand (cmd, "BYE") == true) {
    // Disconnect the client
    len = (uint32_t)sprintf (buf, "\r\nDisconnecting\r\n");
    // Bit-30 of return value is a disconnect flag
    return (len | (1u << 30));
  }

  if (netTELNETs_CheckCommand (cmd, "HELP") == true ||
      netTELNETs_CheckCommand (cmd, "?") == true) {
    // Display help text
    len = (uint32_t)sprintf (buf, help);
    return (len);
  }
  // Unknown command
  len = (uint32_t)sprintf (buf, " <- Unknown Command");
  return (len);
}
//! [code_Telnet_Server_UIF]
