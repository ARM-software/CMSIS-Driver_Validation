/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2019 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    SockServer.c
 * Purpose: Implements ECHO, DISCARD and CHARGEN services
 *          - Echo Protocol service                [RFC 862]
 *          - Discard Protocol service             [RFC 863]
 *          - Character Generator Protocol service [RFC 864]
 * Rev.:    V1.1
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

// ECHO stream socket handler (2 instances)
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

// CHARGEN stream socket handler (2 instances)
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

// DISCARD stream socket handler (1 instance)
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
  int32_t sock_echo,sock_chargen,sock_discard,sock_timeout;
  int32_t sock,nfds,sa_len;
  struct timeval tv;
  fd_set fds;

  // Allocate sockets
  sock_echo    = socket (PF_INET, SOCK_STREAM, 0);
  sock_chargen = socket (PF_INET, SOCK_STREAM, 0);
  sock_discard = socket (PF_INET, SOCK_STREAM, 0);
  sock_timeout = socket (PF_INET, SOCK_STREAM, 0);

  // Bind sockets
  sa.sin_family      = AF_INET;
  sa.sin_addr.s_addr = INADDR_ANY;

  sa.sin_port = htons (ECHO_PORT);
  bind (sock_echo,    (SOCKADDR *)&sa, sizeof(sa));

  sa.sin_port = htons (CHARGEN_PORT);
  bind (sock_chargen, (SOCKADDR *)&sa, sizeof(sa));

  sa.sin_port = htons (DISCARD_PORT);
  bind (sock_discard, (SOCKADDR *)&sa, sizeof(sa));

  sa.sin_port = htons (TCP_TIMEOUT_PORT);
  bind (sock_timeout, (SOCKADDR *)&sa, sizeof(sa));

  // Start listening
  listen (sock_echo, 2);
  listen (sock_chargen, 2);
  listen (sock_discard, 1);
  listen (sock_timeout, 1);

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
void TestAssistant (void *argument) {
  SOCKADDR_IN sa;
  int32_t sock,sd,rc,sa_len;
  static char buff[1500];

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
        uint32_t tout = 2000;
        setsockopt (sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tout, sizeof(tout));
        // Receive the command (tout = 2s)
        rc = recv (sd, buff, sizeof(buff), 0);
        if (rc > 0) {
          buff[rc] = 0;
          if ((strncmp (buff, "CONNECT TCP", 11) == 0) ||
              (strncmp (buff, "CONNECT UDP", 11) == 0) ||
              (strncmp (buff, "SEND TCP", 8) == 0)     ||
              (strncmp (buff, "RECV TCP", 8) == 0)) {
            break;
          }
        }
        closesocket (sd);
        osDelay (10);
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
    if (buff[0] == 'C') { // CONNECT
      uint16_t delay,port;
      IN_ADDR  da;

      closesocket (sd);

      da.s_addr = INADDR_ANY;
      sscanf (buff+11,",%hhu.%hhu.%hhu.%hhu,%hu,%hu",
                       &da.s_b1, &da.s_b2, &da.s_b3, &da.s_b4, &port, &delay);
      if (da.s_addr != INADDR_ANY) {
        // Supplied address not 0.0.0.0 use it
        sa.sin_addr.s_addr = da.s_addr;
      }
      sa.sin_port = htons (port);

      // Limit the timeout
      if (delay < 10)   delay = 10;
      if (delay > 5000) delay = 6000;
      osDelay (delay);

      // Create stream or datagram socket
      sock = socket (PF_INET, (buff[8] == 'T') ? SOCK_STREAM : SOCK_DGRAM, 0);

      // Connect to requested address
      rc = connect (sock, (SOCKADDR *)&sa, sa_len);
      if (rc == 0) {
        // Send some text, wait and close
        send (sock, "SockServer", 10, 0);
        osDelay (500);
      }
      closesocket (sock);
      osDelay (10);
      continue;
    }

    /* Syntax:  SEND <proto>,<blocksz>,<time_ms>
       Param:   <proto>   = protocol (TCP, UDP)
                <bsize>   = size of data block in bytes 
                <time_ms> = test duration in ms
    */
    if (buff[0] == 'S') { // SEND
      uint32_t bsize,time,ticks;
      int32_t  i,n,cnt,ch = 'a';
    
      // Parse command parameters
      sscanf (buff+8,",%u,%u",&bsize,&time);
    
      // Check limits
      if (bsize < 32)   bsize = 32;
      if (bsize > 1460) bsize = 1460;
      if (time < 500)   time  = 500;
      if (time > 60000) time  = 60000;

      osDelay (10);

      time  = SYSTICK_MSEC(time);
      ticks = GET_SYSTICK();
      i = cnt = 0;
      do {
        n = sprintf (buff,"Block[%d] ",++i);
        memset (buff+n, ch, bsize-n);
        buff[bsize] = 0;
        if (++ch > '~') ch = ' ';
        n = send (sd, buff, bsize, 0);
        if (n > 0) cnt += n; 
      } while (GET_SYSTICK() - ticks < time);

      // Inform the client of the number of bytes received
      n = sprintf (buff,"STAT %d bytes.",cnt);
      send (sd, buff, n, 0);

      // Let the client close the connection
      while (recv (sd, buff, sizeof(buff), 0) > 0);

      closesocket (sd);
      continue;
    }

    /* Syntax:  RECV <proto>,<blocksz>
       Param:   <proto> = protocol (TCP, UDP)
                <bsize> = size of data block in bytes 
    */
    if (buff[0] == 'R') { // RECV
      uint32_t bsize;
      int32_t  n,cnt;
    
      // Parse command parameters
      sscanf (buff+8,",%u",&bsize);
    
      // Check limits
      if (bsize < 32)   bsize = 32;
      if (bsize > 1460) bsize = 1460;

      osDelay (10);

      for (cnt = 0;  ; cnt += n) {
        n = recv (sd, buff, bsize, 0);
        if (strncmp(buff, "STOP", 4) == 0) {
          // Client terminated upload
          break;
        }
        if (n <= 0) break; 
      }

      // Inform the client of the number of bytes received
      n = sprintf (buff, "STAT %d bytes.",cnt);
      send (sd, buff, n, 0);
 
      // Let the client close the connection
      while (recv (sd, buff, sizeof(buff), 0) > 0);

      closesocket (sd);
      continue;
    }
  }
}
