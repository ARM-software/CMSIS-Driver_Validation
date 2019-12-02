/*------------------------------------------------------------------------------
 * CMSIS-Driver_Validation - Tools
 * Copyright (c) 2019 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    SockServer.h
 * Purpose: Socket tester definitions
 *----------------------------------------------------------------------------*/

// Definitions
#define ESC                     0x1b            // Ascii code for ESC
#define BUFF_SIZE               2000            // Size of buffers

// Service ports
#define ECHO_PORT               7               // Echo port number
#define DISCARD_PORT            9               // Discard port number
#define CHARGEN_PORT            19              // Chargen port number
#define ASSISTANT_PORT          5000            // Test Assistant port number
#define TCP_REJECTED_PORT       5001            // Rejected connection server TCP port
#define TCP_TIMEOUT_PORT        5002            // Non-responding server TCP port

// Extended option (Winsock2)
#ifndef SO_CONDITIONAL_ACCEPT 
#define SO_CONDITIONAL_ACCEPT   0x3002
#endif

// IN_ADDR byte access definitions
#define s_b1    S_un.S_un_b.s_b1
#define s_b2    S_un.S_un_b.s_b2
#define s_b3    S_un.S_un_b.s_b3
#define s_b4    S_un.S_un_b.s_b4

// Socket Server threads
extern DWORD WINAPI EchoThread (void *argument);
extern DWORD WINAPI ChargenThread (void *argument);
extern DWORD WINAPI DiscardThread (void *argument);
extern DWORD WINAPI AssistantThread (void *argument);
extern DWORD WINAPI DgramServer (void *argument);
extern DWORD WINAPI StreamServer (void *argument);
