/*
 * Copyright (c) 2019-2020 Arm Limited. All rights reserved.
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
 * -----------------------------------------------------------------------------
 *
 * Project:     SockServer
 * Title:       SockServer definitions
 *
 * -----------------------------------------------------------------------------
 */

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
