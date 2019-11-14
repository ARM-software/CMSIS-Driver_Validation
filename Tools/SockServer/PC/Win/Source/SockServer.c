/*------------------------------------------------------------------------------
 * CMSIS-Driver_Validation - Tools
 * Copyright (c) 2019 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    SockServer.c
 * Purpose: Implements ECHO, DISCARD and CHARGEN standard services
 *          - Echo Protocol service                [RFC 862]
 *          - Discard Protocol service             [RFC 863]
 *          - Character Generator Protocol service [RFC 864]
 *----------------------------------------------------------------------------*/

#define VERSION     "v1.1"

#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <time.h>
#include <windows.h>
#include <winsock2.h>
#include "SockServer.h"

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// Global status variables
SOCKADDR_IN remote_addr;        // Remote IP address and port
uint32_t rx_cnt;                // Receive count
uint32_t tx_cnt;                // Transmit count

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

// Debug print status
static void print_status (void) {
  printf("\rAddr=%s, rx_cnt=%d, tx_cnt=%d",inet_ntoa(remote_addr.sin_addr),rx_cnt,tx_cnt);
}

// Datagram server thread
// (runs ECHO and CHARGEN services)
DWORD WINAPI DgramServer (void *argument) {
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

    nfds = sock_echo;
    if (sock_chargen > nfds) nfds = sock_chargen;

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
        print_status ();
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
        print_status ();
      }
      if (rc < 0) {
        break;
      }
    }
  }
  free (buff);
  return (0);
}

// ECHO stream socket handler
DWORD WINAPI EchoThread (void *argument) {
  int32_t sock = (int32_t)argument;
  int32_t rc;

  char *buff = malloc (BUFF_SIZE);
  for (; buff;) {
    rc = recv (sock, buff, BUFF_SIZE, 0);
    if (rc <= 0) break;
    rx_cnt += rc;
    rc = send (sock, buff, rc, 0);
    if (rc < 0) break;
    tx_cnt += rc;
    // ESC terminates the thread
    if (buff[0] == ESC) break;
    print_status ();
  }
  closesocket (sock);
  free (buff);
  return (0);
}

// CHARGEN stream socket handler
DWORD WINAPI ChargenThread (void *argument) {
  int32_t rc,sock = (int32_t)argument;
  char buff[82],setchar = '@';
  unsigned long enable = 1;  

  // Set non-blocking mode
  ioctlsocket (sock, FIONBIO, &enable);
  for (;;) {
    rc = recv (sock, buff, sizeof(buff), 0);
    if (rc > 0) {
      rx_cnt += rc;
      // ESC terminates the thread
      if (buff[0] == ESC) break;
    }
    else {
      // Non-blocking mode, check error code
      if (WSAGetLastError() != WSAEWOULDBLOCK) break;
    }
    setchar = gen_char (buff, setchar, 81);
    rc = send (sock, buff, 81, 0);
    if (rc < 0) break;
    tx_cnt += rc;
    print_status ();
    Sleep (100);
  }
  closesocket (sock);
  return (0);
}

// DISCARD stream socket handler
DWORD WINAPI DiscardThread (void *argument) {
  int32_t rc,sock = (int32_t)argument;
  char buff[40];
  
  for (;;) {
    rc = recv (sock, buff, sizeof(buff), 0);
    if (rc <= 0) break;
    rx_cnt += rc;
    // ESC terminates the thread
    if (buff[0] == ESC) break;
    print_status ();
  }
  closesocket (sock);
  return (0);
}

// Test assistant thread
DWORD WINAPI AssistantThread (void *argument) {
  int32_t rc,sock = (int32_t)argument;
  SOCKADDR_IN sa;
  int32_t sa_len;
  char buff[1500];
   
  // Set blocking receive timeout
  uint32_t tout = 2000;
  setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tout, sizeof(tout));

  // Get remote peer address
  sa_len = sizeof (sa);
  getpeername (sock, (SOCKADDR *)&sa, &sa_len);

  // Receive the command (tout = 2s)
  rc = recv (sock, buff, sizeof(buff), 0);
  if (rc <= 0) {
    closesocket (sock);
    return (0);
  }

  // Parse the command
  buff[rc] = 0;

  /* Syntax:  CONNECT <proto>,<ip_addr>,<port>,<delay_ms>
     Param:   <proto>    = protocol (TCP, UDP)
              <ip_addr>  = IP address (0.0.0.0 = sender address)
              <port>     = port number
              <delay_ms> = startup delay

     Example: CONNECT TCP,192.168.1.200,80,600
     (wait 600ms then connect to 192.168.1.200, port 80)
  */
  if ((strncmp (buff, "CONNECT TCP", 11) == 0) || 
      (strncmp (buff, "CONNECT UDP", 11) == 0)) {
    uint16_t port;
    uint32_t delay;
    IN_ADDR da;

    Sleep (10);
    closesocket (sock);
    
    // Parse command parameters
    da.s_addr = INADDR_ANY;
    sscanf (buff+11,",%hhu.%hhu.%hhu.%hhu,%hu,%u",
                    &da.s_b1,&da.s_b2,&da.s_b3,&da.s_b4,&port,&delay);
 
    sa.sin_family = AF_INET;
    sa.sin_port   = htons (port);
    if (da.s_addr != INADDR_ANY) {
      // Supplied address not 0.0.0.0 use it
      sa.sin_addr.s_addr = da.s_addr;
    }
 
    // Limit the timeout
    if (delay < 10)   delay = 10;
    if (delay > 5000) delay = 5000;
    Sleep (delay);

    // Create stream or datagram socket
    sock = socket (PF_INET,  (buff[8] == 'T') ? SOCK_STREAM : SOCK_DGRAM, 0);

    // Connect to requested address
    rc = connect (sock, (SOCKADDR *)&sa, sizeof(sa));
    if (rc == 0) {
      // Send some text, wait and close
      send (sock, "SockServer", 10, 0);
      Sleep (500);
    }
    closesocket (sock);
    return (0);
  }

  /* Syntax:  SEND <proto>,<blocksz>,<time_ms>
     Param:   <proto>   = protocol (TCP, UDP)
              <bsize>   = size of data block in bytes 
              <time_ms> = test duration in ms
  */
  if (strncmp (buff, "SEND TCP", 8) == 0) { 
    uint32_t bsize,time;
    clock_t ticks,tout;
    int32_t i,n,cnt,ch = 'a';
    
    // Parse command parameters
    sscanf (buff+8,",%u,%u",&bsize,&time);
    
    // Check limits
    if (bsize < 32)    bsize = 32;
    if (bsize > 1460)  bsize = 1460;
    if (time < 500)    time  = 500;
    if (time > 60000)  time  = 60000;

    // Adjust Winsock2 send buffering
    n = bsize * 2;
    setsockopt (sock, SOL_SOCKET, SO_SNDBUF, (char *)&n, sizeof(n));
    Sleep (10);

    i = cnt = 0;
    ticks = clock ();
    do {
      n = sprintf (buff,"Block[%d] ",++i);
      memset (buff+n, ch, bsize-n);
      buff[bsize] = 0;
      if (++ch > '~') ch = ' ';
      n = send (sock, buff, bsize, 0);
      if (n > 0) cnt += n; 
    } while (clock () - ticks < time);

    // Inform the client of the number of bytes received
    n = sprintf (buff,"STAT %d bytes.",cnt);       
    send (sock, buff, n, 0);

    // let the client close the connection
    while (recv (sock, buff, sizeof(buff), 0) > 0);

    closesocket (sock);
    return (0);
  }

  /* Syntax:  RECV <proto>,<blocksz>
     Param:   <proto> = protocol (TCP, UDP)
              <bsize> = size of data block in bytes 
  */
  if (strncmp (buff, "RECV TCP", 8) == 0) { 
    uint32_t bsize;
    int32_t i,n,cnt;
    
    // Parse command parameters
    sscanf (buff+8,",%u",&bsize);
    
    // Check limits
    if (bsize < 32)    bsize = 32;
    if (bsize > 1460)  bsize = 1460;

    Sleep (10);

    for (cnt = 0;  ; cnt += n) {
      n = recv (sock, buff, bsize, 0);
      if (strncmp(buff, "STOP", 4) == 0) {
        // Client terminated upload
        break;
      }
      if (n <= 0) break; 
    }

    // Inform the client of the number of bytes received
    n = sprintf (buff, "STAT %d bytes.",cnt);
    send (sock, buff, n, 0);
 
    // let the client close the connection
    while (recv (sock, buff, sizeof(buff), 0) > 0);

    closesocket (sock);
    return (0);
  }

  closesocket (sock);
  return (0);
}

// Conditional accept filtering (Winsock2)
int32_t CALLBACK ConditionAcceptFunc(
    LPWSABUF lpCallerId,
    LPWSABUF lpCallerData,
    LPQOS pQos,
    LPQOS lpGQOS,
    LPWSABUF lpCalleeId,
    LPWSABUF lpCalleeData,
    GROUP FAR * g,
    DWORD_PTR dwCallbackData) {
  return (CF_REJECT);
}

// Stream server thread
// (runs ECHO, CHARGEN, DISCARD and ASSISTANT services)
DWORD WINAPI StreamServer (void *argument) {
  int32_t sock_echo,sock_chargen,sock_discard,sock_assistant,sock_rejected;
  SOCKADDR_IN sa;
  int32_t sock,nfds,sa_len;
  struct timeval tv;
  fd_set fds;
  uint32_t en;
  int32_t retv;

  // Allocate sockets
  sock_echo      = socket (PF_INET, SOCK_STREAM, 0);
  sock_chargen   = socket (PF_INET, SOCK_STREAM, 0);
  sock_discard   = socket (PF_INET, SOCK_STREAM, 0);
  sock_assistant = socket (PF_INET, SOCK_STREAM, 0);
  sock_rejected  = socket (PF_INET, SOCK_STREAM, 0);
  
  // Enable conditional accept (Winsock2)
  en = 1;
  retv = setsockopt (sock_rejected, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char *)&en, sizeof(en));
  if (retv != NO_ERROR) {
    printf ("Failed to set SO_CONDITIONAL_ACCEPT\n");
  }
  
  // Bind sockets
  sa.sin_family      = AF_INET;
  sa.sin_addr.s_addr = INADDR_ANY;

  sa.sin_port = htons (ECHO_PORT);
  bind (sock_echo,    (SOCKADDR *)&sa, sizeof(sa));

  sa.sin_port = htons (CHARGEN_PORT);
  bind (sock_chargen, (SOCKADDR *)&sa, sizeof(sa));

  sa.sin_port = htons (DISCARD_PORT);
  bind (sock_discard, (SOCKADDR *)&sa, sizeof(sa));

  sa.sin_port = htons  (ASSISTANT_PORT);
  bind (sock_assistant,(SOCKADDR *)&sa, sizeof(sa));

  sa.sin_port = htons  (TCP_REJECTED_PORT);
  bind (sock_rejected, (SOCKADDR *)&sa, sizeof(sa));
  
  // Start listening
  listen (sock_echo, 2);
  listen (sock_chargen, 2);
  listen (sock_discard, 2);
  listen (sock_assistant, 2);
  listen (sock_rejected, 2);
  
  for (;;) {
    FD_ZERO(&fds);
    FD_SET(sock_echo, &fds);
    FD_SET(sock_chargen, &fds);
    FD_SET(sock_discard, &fds);
    FD_SET(sock_assistant, &fds);
    FD_SET(sock_rejected, &fds);
    
    nfds = sock_echo;
    if (sock_chargen > nfds)   nfds = sock_chargen;
    if (sock_discard > nfds)   nfds = sock_discard;
    if (sock_assistant > nfds) nfds = sock_assistant;
    if (sock_rejected > nfds)  nfds = sock_rejected;
    
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
        // Create spawn thread
        CreateThread(NULL, 0, EchoThread, (void *)sock, 0, NULL);
      }
    }

    if (FD_ISSET(sock_chargen, &fds)) {
      // Connect is pending, accept will not block
      sa_len = sizeof(sa);
      sock   = accept (sock_chargen, (SOCKADDR *)&sa, &sa_len);
      if (sock >= 0) {
        memcpy (&remote_addr, &sa, sa_len);
        // Create spawn thread
        CreateThread(NULL, 0, ChargenThread, (void *)sock, 0, NULL);
      }
    }

    if (FD_ISSET(sock_discard, &fds)) {
      // Connect is pending, accept will not block
      sa_len = sizeof(sa);
      sock   = accept (sock_discard, (SOCKADDR *)&sa, &sa_len);
      if (sock >= 0) {
        memcpy (&remote_addr, &sa, sa_len);
        // Create spawn thread
        CreateThread(NULL, 0, DiscardThread, (void *)sock, 0, NULL);
      }
    }

    if (FD_ISSET(sock_assistant, &fds)) {
      // Connect is pending, accept will not block
      sa_len = sizeof(sa);
      sock   = accept (sock_assistant, (SOCKADDR *)&sa, &sa_len);
      if (sock >= 0) {
        memcpy (&remote_addr, &sa, sa_len);
        // Create spawn thread
        CreateThread(NULL, 0, AssistantThread, (void *)sock, 0, NULL);
      }
    }

    if (FD_ISSET(sock_rejected, &fds)) {
      // Connect is pending, reject it
      sock = WSAAccept (sock_rejected, NULL, NULL, &ConditionAcceptFunc, 0);
      if (sock >= 0) {
        closesocket (sock);
      }
    }
    Sleep (10);
  }
}

// Main program
int main() {
  WSADATA wsaData;
  HANDLE thread;
  int iResult;
  char ac[80];
  struct hostent *phe;
    
  printf("\nSockServer %s\n", VERSION);

  // Initialize Winsock2 
  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (iResult != NO_ERROR) {
    printf("WSAStartup failed with error: %d\n", iResult);
    return (1);
  }

  thread = CreateThread(NULL, 0, DgramServer, NULL, 0, NULL);
  if (thread == NULL) {
    printf("Failed to create thread: DgramServer\n");
    WSACleanup();
    return (1);
  }

  thread = CreateThread(NULL, 0, StreamServer, NULL, 0, NULL);
  if (thread == NULL) {
    printf("Failed to create thread: StreamServer\n");
    WSACleanup();
    return (1);
  }

  // Print info about local host
  if (gethostname (ac, sizeof(ac)) != SOCKET_ERROR) {
    printf ("\nServer name: %s\n",ac);
    phe = gethostbyname (ac);
    if (phe != NULL) {
      for (int i = 0; phe->h_addr_list[i] != 0; i++) {
        struct in_addr addr;
        memcpy (&addr, phe->h_addr_list[i], sizeof (struct in_addr));
        printf ("Address: %s\n", inet_ntoa(addr));
      }
    }
  }

  printf("\nPress any key to stop...\n");
  _getch ();
  
  // Terminate use of Winsock2
  WSACleanup();
  printf ("\nOk\n");
  return 0;
}
