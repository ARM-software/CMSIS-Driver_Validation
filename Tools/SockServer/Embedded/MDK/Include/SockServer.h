/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2019 ARM Germany GmbH. All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    SockServer.h
 * Purpose: Socket tester definitions
 * Rev.:    V1.1
 *----------------------------------------------------------------------------*/

// Definitions
#define ESC                 0x1b        // Ascii code for ESC
#define BUFF_SIZE           2000        // Size of buffers (heap: 6000 bytes)

// Service ports
#define ECHO_PORT           7           // Echo port number
#define DISCARD_PORT        9           // Discard port number
#define CHARGEN_PORT        19          // Chargen port number
#define ASSISTANT_PORT      5000        // Test Assistant port number
#define TCP_REJECTED_PORT   5001        // Rejected connection server TCP port
#define TCP_TIMEOUT_PORT    5002        // Non-responding server TCP port

#define GET_SYSTICK()       osKernelGetSysTimerCount()
#define SYSTICK_MSEC(ms)    ((uint64_t)ms * osKernelGetSysTimerFreq() / 1000)

// Socket Server threads
extern void DgramServer (void *argument);
extern void StreamServer (void *argument);
extern void TestAssistant (void *argument);
