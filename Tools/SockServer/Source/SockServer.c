/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2019 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    SockServer.c
 * Purpose: Implements ECHO, DISCARD and CHARGEN services
 *          - Echo Protocol service                [RFC 862]
 *          - Discard Protocol service             [RFC 863]
 *          - Character Generator Protocol service [RFC 864]
 *----------------------------------------------------------------------------*/
 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rl_net.h"
#include "cmsis_os2.h"
#include "SockServer.h"

// Global status variables
SOCKADDR_IN remote_addr;        // Remote IP address and port
uint32_t rx_cnt;                // Receive count
uint32_t tx_cnt;                // Transmit count

// Local functions
static void EchoThread (void *argument);
static void ChargenThread (void *argument);
static void DiscardThread (void *argument);
static char gen_char (char *buf, char setchar, uint32_t len);

// Generate character array for transmit
static char gen_char (char *buf, char setchar, uint32_t len) {
  uint32_t i;
  char ch;

  if ((++setchar < 0x21) || (setchar == 0x7f)) {
    setchar = 0x21;
  }
  for (i = 0, ch = setchar; i < (len-2); i++) {
    buf[i] = ch;
    if (++ch == 0x7f) ch = 0x21;
  }
  buf[i]   = '\n';
  buf[i+1] = '\r';
  return (setchar);
}

// Datagram server thread
// (runs ECHO and CHARGEN services)
void DgramServer (void *argument) {
  SOCKADDR_IN sa;
  int32_t sock_echo,sock_chargen;
  int32_t nfds,rc,sa_len;
  char *buff,setchar;
  struct timeval tv;
  fd_set fds;

  // Allocate sockets
  sock_echo    = socket (PF_INET, SOCK_DGRAM, 0);
  sock_chargen = socket (PF_INET, SOCK_DGRAM, 0);

  // Bind sockets
  sa.sin_family      = AF_INET;
  sa.sin_addr.s_addr = INADDR_ANY;

  sa.sin_port        = htons (ECHO_PORT);
  bind (sock_echo, (SOCKADDR *)&sa, sizeof(sa));

  sa.sin_port        = htons (CHARGEN_PORT);
  bind (sock_chargen, (SOCKADDR *)&sa, sizeof(sa));

  setchar = '@';
  buff = malloc (BUFF_SIZE);

  // Receive data
  for (;;) {
    FD_ZERO(&fds);
    FD_SET(sock_echo, &fds);
    FD_SET(sock_chargen, &fds);
    nfds = (sock_echo > sock_chargen) ? sock_echo : sock_chargen;
    tv.tv_sec  = 120;
    tv.tv_usec = 0;

    // Wait for the packet
    select (nfds+1, &fds, 0, 0, &tv);

    if (FD_ISSET(sock_echo, &fds)) {
      // Data ready, recvfrom will not block
      sa_len = sizeof (sa);
      rc = recvfrom (sock_echo, buff, BUFF_SIZE, 0, (SOCKADDR *)&sa, &sa_len);
      if (rc > 0) {
        rx_cnt += rc;
        memcpy (&remote_addr, &sa, sizeof(sa));
        rc = sendto (sock_echo, buff, rc, 0, (SOCKADDR *)&sa, sa_len);
        if (rc > 0) tx_cnt += rc;
      }
      if (rc < 0) {
        break;
      }
    }
    if (FD_ISSET(sock_chargen, &fds)) {
      // Data ready, recvfrom will not block
      sa_len = sizeof (sa);
      rc = recvfrom (sock_chargen, buff, BUFF_SIZE, 0, (SOCKADDR *)&sa, &sa_len);
      if (rc > 0) {
        rx_cnt += rc;
        memcpy (&remote_addr, &sa, sizeof(sa));
        int32_t len = rand() >> 22;
        if (len < 2)         len = 2;
        if (len > BUFF_SIZE) len = BUFF_SIZE;
        setchar = gen_char (buff, setchar, len);
        rc = sendto (sock_chargen, buff, len, 0, (SOCKADDR *)&sa, sa_len);
        if (rc > 0) tx_cnt += rc;
      }
      if (rc < 0) {
        break;
      }
    }
  }
  free (buff);
}

// Stream socket handler (2 instances)
static void EchoThread (void *argument) {
  int32_t sock = (int32_t)argument;
  int32_t rc;

  char *buff = malloc (BUFF_SIZE);
  for (; buff;) {
    rc = recv (sock, buff, BUFF_SIZE, 0);
    if (rc > 0) {
      rx_cnt += rc;
      rc = send (sock, buff, rc, 0);
      if (rc > 0) tx_cnt += rc;
      // ESC terminates the thread
      if (buff[0] == ESC) break;
    }
    if (rc < 0) break;
  }
  closesocket (sock);
  free (buff);
}

// Stream socket handler (2 instances)
static void ChargenThread (void *argument) {
  int32_t rc,sock = (int32_t)argument;
  char buff[82],setchar = '@';

  for (;;) {
    rc = recv (sock, buff, sizeof(buff), MSG_DONTWAIT);
    if (rc > 0) rx_cnt += rc;
    // ESC terminates the thread
    if ((rc > 0) && (buff[0] == ESC)) break;
    setchar = gen_char (buff, setchar, 81);
    rc = send (sock, buff, 81, 0);
    if (rc < 0) break;
    else tx_cnt += rc;
    osDelay (100);
  }
  closesocket (sock);
}

// Stream socket handler (1 instance)
static void DiscardThread (void *argument) {
  int32_t rc,sock = (int32_t)argument;
  char buff[40];

  for (;;) {
    rc = recv (sock, buff, sizeof(buff), 0);
    if (rc > 0) rx_cnt += rc;
    // ESC terminates the thread
    if ((rc > 0) && (buff[0] == ESC)) break;
    if (rc < 0) break;
  }
  closesocket (sock);
}

// Stream server thread
// (runs ECHO, CHARGEN and DISCARD services)
void StreamServer (void *argument) {
  SOCKADDR_IN sa;
  int32_t sock_echo,sock_chargen,sock_discard;
  int32_t sock,nfds,sa_len;
  struct timeval tv;
  fd_set fds;

  // Allocate sockets
  sock_echo    = socket (PF_INET, SOCK_STREAM, 0);
  sock_chargen = socket (PF_INET, SOCK_STREAM, 0);
  sock_discard = socket (PF_INET, SOCK_STREAM, 0);

  // Bind sockets
  sa.sin_family      = AF_INET;
  sa.sin_addr.s_addr = INADDR_ANY;

  sa.sin_port = htons (ECHO_PORT);
  bind (sock_echo,    (SOCKADDR *)&sa, sizeof(sa));

  sa.sin_port = htons (CHARGEN_PORT);
  bind (sock_chargen, (SOCKADDR *)&sa, sizeof(sa));

  sa.sin_port = htons (DISCARD_PORT);
  bind (sock_discard, (SOCKADDR *)&sa, sizeof(sa));

  // Start listening
  listen (sock_echo, 2);
  listen (sock_chargen, 2);
  listen (sock_discard, 1);

  for (;;) {
    FD_ZERO(&fds);
    FD_SET(sock_echo, &fds);
    FD_SET(sock_chargen, &fds);
    FD_SET(sock_discard, &fds);
    nfds = sock_echo;
    if (sock_chargen > nfds) nfds = sock_chargen;
    if (sock_discard > nfds) nfds = sock_discard;
    tv.tv_sec  = 120;
    tv.tv_usec = 0;

    // Wait for the client to connect
    select (nfds+1, &fds, 0, 0, &tv);
    if (FD_ISSET(sock_echo, &fds)) {
      // Connect is pending, accept will not block
      sa_len = sizeof(sa);
      sock   = accept (sock_echo, (SOCKADDR *)&sa, &sa_len);
      if (sock >= 0) {
        memcpy (&remote_addr, &sa, sa_len);
        // Create spawn thread (max.2)
        osThreadNew(EchoThread, (void *)sock, NULL);
      }
    }
    if (FD_ISSET(sock_chargen, &fds)) {
      // Connect is pending, accept will not block
      sa_len = sizeof(sa);
      sock   = accept (sock_chargen, (SOCKADDR *)&sa, &sa_len);
      if (sock >= 0) {
        memcpy (&remote_addr, &sa, sa_len);
        // Create spawn thread (max.2)
        osThreadNew(ChargenThread, (void *)sock, NULL);
      }
    }
    if (FD_ISSET(sock_discard, &fds)) {
      // Connect is pending, accept will not block
      sa_len = sizeof(sa);
      sock   = accept (sock_discard, (SOCKADDR *)&sa, &sa_len);
      if (sock >= 0) {
        memcpy (&remote_addr, &sa, sa_len);
        // Create spawn thread (max.1)
        osThreadNew(DiscardThread, (void *)sock, NULL);
      }
    }
    osDelay (10);
  }
}

// Test assistant thread
// - Creates a stream server socket
// - Waits for the WiFi driver to connect and send the command
// - Scans the command for <proto>, <ip_addr>, <port> and <delay_ms>
// - Closes connection and waits for <delay_ms>
// - Connects to <ip_addr:port> (stream or datagram type)
// - Sends small text and disconnects
void TestAssistant (void *argument) {
  SOCKADDR_IN sa;
  IN_ADDR da;
  int32_t sock,sd,rc,sa_len,type;
  uint16_t tout,port;
  char buff[80];

  while (1) {
    // Create socket
    sock = socket (PF_INET, SOCK_STREAM, 0);

    // Server mode first
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port        = htons (ASSISTANT_PORT);
    bind (sock, (SOCKADDR *)&sa, sizeof(sa));
    listen (sock, 1);

    while (1) {
      // Wait for the client to connect
      sa_len = sizeof (sa);
      sd = accept (sock, (SOCKADDR *)&sa, &sa_len);
      if (sd >= 0) {
        // Set blocking receive timeout
        tout = 2000;
        setsockopt (sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tout, sizeof(tout));
        // Receive the command (tout = 2s)
        rc = recv (sd, buff, sizeof(buff), 0);
        closesocket (sd);
        if (rc > 0) {
          buff[rc] = 0;
          if (strncmp (buff, "CONNECT TCP", 11) == 0) {
            type = SOCK_STREAM;
            break;
          }
          if (strncmp (buff, "CONNECT UDP", 11) == 0) {
            type = SOCK_DGRAM;
            break;
          }
        }
      }
    }
    closesocket (sock);

    /* Syntax:  CONNECT <proto>,<ip_addr>,<port>,<delay_ms>
       Param:   <proto>    = protocol (TCP, UDP)
                <ip_addr>  = IP address (0.0.0.0 = sender address)
                <port>     = port number
                <delay_ms> = startup delay

       Example: CONNECT TCP,192.168.1.200,80,600
       (wait 600ms then connect to 192.168.1.200, port 80)
    */
    da.s_addr = INADDR_ANY;
    sscanf (buff+11,",%hhu.%hhu.%hhu.%hhu,%hu,%hu",
                     &da.s_b1, &da.s_b2, &da.s_b3, &da.s_b4, &port, &tout);
    if (da.s_addr != INADDR_ANY) {
      // Supplied address not 0.0.0.0 use it
      sa.sin_addr.s_addr = da.s_addr;
    }
    sa.sin_port = htons (port);

    // Limit the timeout
    if (tout > 5000-10) tout = 5000-10;
    osDelay (10 + tout);

    // Create stream or datagram socket
    sock = socket (PF_INET, type, 0);

    // Connect to requested address
    rc = connect (sock, (SOCKADDR *)&sa, sa_len);
    if (rc == 0) {
      // Send some text, wait and close
      send (sock, "SockServer", 10, 0);
      osDelay (500);
    }
    closesocket (sock);
    osDelay (10);
  }
}
