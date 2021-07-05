/*
 * Copyright (c) 2020-2021 Arm Limited. All rights reserved.
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
 * Project:     USART Server
 * Title:       USART Server application
 *
 * -----------------------------------------------------------------------------
 */


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "USART_Server_Config.h"
#include "USART_Server_HW.h"
#include "USART_Server.h"

#include "cmsis_os2.h"
#include "cmsis_compiler.h"
#include "cmsis_vio.h"

#include "Driver_USART.h"               // ::CMSIS Driver:USART

#ifndef  USART_SERVER_DEBUG
#define  USART_SERVER_DEBUG             0
#endif

// Fixed USART Server settings (not available through USART_Server_Config.h)
#define  USART_SERVER_BAUDRATE          115200  // Baudrate
#define  USART_SERVER_DATA_BITS         8       // 8 data bits
#define  USART_SERVER_PARITY            0       // None
#define  USART_SERVER_STOP_BITS         0       // 1 stop bit
#define  USART_SERVER_FLOW_CONTROL      0       // None

#define  USART_RECEIVE_EVENTS_MASK     (ARM_USART_EVENT_RECEIVE_COMPLETE  | \
                                        ARM_USART_EVENT_RX_OVERFLOW       | \
                                        ARM_USART_EVENT_RX_BREAK          | \
                                        ARM_USART_EVENT_RX_FRAMING_ERROR  | \
                                        ARM_USART_EVENT_RX_PARITY_ERROR)
#define  USART_TRANSFER_EVENTS_MASK    (ARM_USART_EVENT_TRANSFER_COMPLETE | \
                                        ARM_USART_EVENT_TX_UNDERFLOW      | \
                                        ARM_USART_EVENT_RX_OVERFLOW)

#define  DATA_BITS_TO_BYTES(data_bits)      ((((data_bits) > 8) ? (2U) : (1U)))
#define  BYTES_TO_ITEMS(bytes,data_bits)    ((bytes + DATA_BITS_TO_BYTES(data_bits) - 1U) / DATA_BITS_TO_BYTES(data_bits))

/* Access to Driver_USART# */
#define  USART_Driver_Aux(n)            Driver_USART##n
#define  USART_Driver_Name(n)           USART_Driver_Aux(n)
extern   ARM_DRIVER_USART               USART_Driver_Name(USART_SERVER_DRV_NUM);
#define  drvUSART                     (&USART_Driver_Name(USART_SERVER_DRV_NUM))

typedef struct {                        // USART Interface settings structure
  uint32_t mode;
  uint32_t data_bits;
  uint32_t parity;
  uint32_t stop_bits;
  uint32_t flow_control;
  uint32_t cpol;
  uint32_t cpha;
  uint32_t irda_pulse;
  uint32_t smartcard_clock;
  uint32_t smartcard_guard_time;
  uint32_t baudrate;
} USART_COM_CONFIG_t;

// Structure containing command string and pointer to command handling function
typedef struct {
  const char     *command;
        int32_t (*Command_Func) (const char *command);
} USART_CMD_DESC_t;

// Local functions

// Main thread (reception and execution of command)
__NO_RETURN \
static void     USART_Server_Thread      (void *argument);

// USART Interface communication functions
static void     USART_Com_Event          (uint32_t event);
static int32_t  USART_Com_Initialize     (void);
static int32_t  USART_Com_Uninitialize   (void);
static int32_t  USART_Com_PowerOn        (void);
static int32_t  USART_Com_PowerOff       (void);
static int32_t  USART_Com_Configure      (const USART_COM_CONFIG_t *config);
static int32_t  USART_Com_Receive        (                      void *data_in, uint32_t num, uint32_t timeout);
static int32_t  USART_Com_Send           (const void *data_out,                uint32_t num, uint32_t timeout);
static int32_t  USART_Com_Transfer       (const void *data_out, void *data_in, uint32_t num, uint32_t timeout);
static int32_t  USART_Com_Break          (uint32_t val);
static int32_t  USART_Com_SetModemControl(ARM_USART_MODEM_CONTROL control);
static int32_t  USART_Com_Abort          (void);
static uint32_t USART_Com_GetCnt         (void);
static uint32_t USART_Com_GetMdm         (void);

// Command handling functions
static int32_t  USART_Cmd_GetVer         (const char *cmd);
static int32_t  USART_Cmd_GetCap         (const char *cmd);
static int32_t  USART_Cmd_SetBuf         (const char *cmd);
static int32_t  USART_Cmd_GetBuf         (const char *cmd);
static int32_t  USART_Cmd_SetCom         (const char *cmd);
static int32_t  USART_Cmd_Xfer           (const char *cmd);
static int32_t  USART_Cmd_GetCnt         (const char *cmd);
static int32_t  USART_Cmd_SetBrk         (const char *cmd);
static int32_t  USART_Cmd_GetBrk         (const char *cmd);
static int32_t  USART_Cmd_SetMdm         (const char *cmd);
static int32_t  USART_Cmd_GetMdm         (const char *cmd);

// Local variables
static const uint32_t usart_baudrates[] = {
  9600U, 19200U, 38400U, 57600U, 115200U, 230400U, 460800U, 921600U
};

static const char *str_mode[] = {
  "Unknown",
  "Async",
  "Singl-wire",
  "IrDA"
};

// Command specification (command string, command handling function)
static const USART_CMD_DESC_t usart_cmd_desc[] = {
 { "GET VER" , USART_Cmd_GetVer },
 { "GET CAP" , USART_Cmd_GetCap },
 { "SET BUF" , USART_Cmd_SetBuf },
 { "GET BUF" , USART_Cmd_GetBuf },
 { "SET COM" , USART_Cmd_SetCom },
 { "XFER"    , USART_Cmd_Xfer   },
 { "GET CNT" , USART_Cmd_GetCnt },
 { "SET BRK" , USART_Cmd_SetBrk },
 { "GET BRK" , USART_Cmd_GetBrk },
 { "SET MDM" , USART_Cmd_SetMdm },
 { "GET MDM" , USART_Cmd_GetMdm }
};

static       osThreadId_t       usart_server_thread_id    =   NULL;
static       osThreadAttr_t     thread_attr = {
  .name       = "USART_Server_Thread",
  .stack_size = 512U
};

static ARM_USART_CAPABILITIES   drv_cap;

static       uint8_t            usart_server_state        =   USART_SERVER_STATE_RECEPTION;
static       uint32_t           usart_cmd_timeout         =   USART_SERVER_CMD_TIMEOUT;
static       uint32_t           usart_xfer_timeout        =   USART_SERVER_CMD_TIMEOUT;
static       uint32_t           usart_xfer_cnt            =   0U;
static       uint32_t           usart_xfer_buf_size       =   USART_SERVER_BUF_SIZE;
static const USART_COM_CONFIG_t usart_com_config_default  = {(USART_SERVER_MODE         << ARM_USART_CONTROL_Pos)      & ARM_USART_CONTROL_Msk, 
#if (USART_SERVER_DATA_BITS == 8U)
                                                              ARM_USART_DATA_BITS_8,
#else
                                                             (USART_SERVER_DATA_BITS    << ARM_USART_DATA_BITS_Pos)    & ARM_USART_DATA_BITS_Msk, 
#endif
                                                             (USART_SERVER_PARITY       << ARM_USART_PARITY_Pos)       & ARM_USART_PARITY_Msk, 
                                                             (USART_SERVER_STOP_BITS    << ARM_USART_STOP_BITS_Pos)    & ARM_USART_STOP_BITS_Msk, 
                                                             (USART_SERVER_FLOW_CONTROL << ARM_USART_FLOW_CONTROL_Pos) & ARM_USART_FLOW_CONTROL_Msk, 
                                                              ARM_USART_CPOL0, 
                                                              ARM_USART_CPHA0, 
                                                              0U, 
                                                              0U, 
                                                              0U, 
                                                              USART_SERVER_BAUDRATE
                                                            };
static       USART_COM_CONFIG_t usart_com_config_xfer;
static       uint8_t            usart_bytes_per_item        = 1U;
static       uint8_t            usart_cmd_buf_rx[32]        __ALIGNED(4);
static       uint8_t            usart_cmd_buf_tx[32]        __ALIGNED(4);
static       uint8_t           *ptr_usart_xfer_buf_rx       = NULL;
static       uint8_t           *ptr_usart_xfer_buf_tx       = NULL;
static       void              *ptr_usart_xfer_buf_rx_alloc = NULL;
static       void              *ptr_usart_xfer_buf_tx_alloc = NULL;

static       uint32_t           break_status                = 0U;
static       uint32_t           dcd_ri_mask                 = 3U;

// Global functions

// Default empty implementation if external functions are not provided, 
// otherwise overridden by externally provided functions
__WEAK void USART_Server_Pins_Initialize   (void) { dcd_ri_mask = 0U; }
__WEAK void USART_Server_Pins_Uninitialize (void) {}
__WEAK void USART_Server_Pin_DCD_SetState  (uint32_t state) {}
__WEAK void USART_Server_Pin_RI_SetState   (uint32_t state) {}

/**
  \fn            int32_t USART_Server_Start (void)
  \brief         Initialize, power up, configure USART interface and start USART Server thread.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
int32_t USART_Server_Start (void) {
  int32_t  ret;
  uint32_t mode, mode_disp;

  mode = USART_SERVER_MODE;
  switch (mode) {
    case 1:
      mode_disp = 1;
      break;
    case 4:
      mode_disp = 2;
      break;
    case 5:
      mode_disp = 3;
      break;
    default:
      mode_disp = 0U;
      break;
  }

  vioInit();
  (void)vioPrint(vioLevelHeading, "USART Server v%s\r\nMode: %s", USART_SERVER_VER, str_mode[mode_disp]);

  // Initialize local variables
  usart_server_state   = USART_SERVER_STATE_RECEPTION;
  usart_cmd_timeout    = USART_SERVER_CMD_TIMEOUT;
  usart_xfer_timeout   = USART_SERVER_CMD_TIMEOUT;
  usart_xfer_cnt       = 0U;
  usart_xfer_buf_size  = USART_SERVER_BUF_SIZE;
  usart_bytes_per_item = DATA_BITS_TO_BYTES(USART_SERVER_DATA_BITS);
  memset(usart_cmd_buf_rx,  0, sizeof(usart_cmd_buf_rx));
  memset(usart_cmd_buf_tx,  0, sizeof(usart_cmd_buf_tx));
  memcpy(&usart_com_config_xfer, &usart_com_config_default, sizeof(USART_COM_CONFIG_t));

  // Allocate buffers for data transmission and reception
  // (maximum size is incremented by 4 bytes to ensure that buffer can be aligned to 4 bytes)

  ptr_usart_xfer_buf_rx_alloc = malloc(USART_SERVER_BUF_SIZE + 4U);
  if (((uint32_t)ptr_usart_xfer_buf_rx_alloc & 3U) != 0U) {
    // If allocated memory is not 4 byte aligned, use next 4 byte aligned address for ptr_tx_buf
    ptr_usart_xfer_buf_rx = (uint8_t *)((((uint32_t)ptr_usart_xfer_buf_rx_alloc) + 3U) & (~3U));
  } else {
    // If allocated memory is 4 byte aligned, use it directly
    ptr_usart_xfer_buf_rx = (uint8_t *)ptr_usart_xfer_buf_rx_alloc;
  }
  ptr_usart_xfer_buf_tx_alloc = malloc(USART_SERVER_BUF_SIZE + 4U);
  if (((uint32_t)ptr_usart_xfer_buf_tx_alloc & 3U) != 0U) {
    // If allocated memory is not 4 byte aligned, use next 4 byte aligned address for ptr_tx_buf
    ptr_usart_xfer_buf_tx = (uint8_t *)((((uint32_t)ptr_usart_xfer_buf_tx_alloc) + 3U) & (~3U));
  } else {
    // If allocated memory is 4 byte aligned, use it directly
    ptr_usart_xfer_buf_tx = (uint8_t *)ptr_usart_xfer_buf_tx_alloc;
  }

  if ((ptr_usart_xfer_buf_rx != NULL) || (ptr_usart_xfer_buf_tx != NULL)) {
    memset(ptr_usart_xfer_buf_rx, 0, USART_SERVER_BUF_SIZE);
    memset(ptr_usart_xfer_buf_rx, 0, USART_SERVER_BUF_SIZE);
    ret = EXIT_SUCCESS;
  } else {
    ret = EXIT_FAILURE;
  }

  // Get driver capabilities
  drv_cap = drvUSART->GetCapabilities();

  if (ret == EXIT_SUCCESS) {
    ret = USART_Com_Initialize();
  }

  if (ret == EXIT_SUCCESS) {
    ret = USART_Com_PowerOn();
  }

  if (ret == EXIT_SUCCESS) {
    ret = USART_Com_Configure(&usart_com_config_default);
  }

  USART_Server_Pins_Initialize();

  if ((ret == EXIT_SUCCESS) && (usart_server_thread_id == NULL)) {
    // Create USART_Server_Thread thread
    usart_server_thread_id = osThreadNew(USART_Server_Thread, NULL, &thread_attr);
    if (usart_server_thread_id == NULL) {
      ret = EXIT_FAILURE;
    }
  }

  if (ret != EXIT_SUCCESS) {
    vioPrint(vioLevelError, "Server Start failed!");
  }

  return ret;
}

/**
  \fn            int32_t USART_Server_Stop (void)
  \brief         Terminate USART Server thread, power down and uninitialize USART interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
int32_t USART_Server_Stop (void) {
   int32_t ret;
  uint32_t i;

  ret = EXIT_FAILURE;

  if (usart_server_thread_id != NULL) {
    usart_server_state = USART_SERVER_STATE_TERMINATE;
    for (i = 0U; i < 10U; i++) {
      if (osThreadGetState(usart_server_thread_id) == osThreadTerminated) {
        usart_server_thread_id = NULL;
        ret = EXIT_SUCCESS;
        break;
      }
      (void)osDelay(100U);
    }
  }

  USART_Server_Pins_Uninitialize();

  if (ret == EXIT_SUCCESS) {
    ret = USART_Com_PowerOff();
  }

  if (ret == EXIT_SUCCESS) {
    ret = USART_Com_Uninitialize();
  }

  if (ptr_usart_xfer_buf_rx_alloc != NULL) {
    free(ptr_usart_xfer_buf_rx_alloc);
    ptr_usart_xfer_buf_rx       = NULL;
    ptr_usart_xfer_buf_rx_alloc = NULL;
  }
  if (ptr_usart_xfer_buf_tx_alloc != NULL) {
    free(ptr_usart_xfer_buf_tx_alloc);
    ptr_usart_xfer_buf_tx       = NULL;
    ptr_usart_xfer_buf_tx_alloc = NULL;
  }

  if (ret != EXIT_SUCCESS) {
    vioPrint(vioLevelError, "Server Stop failed! ");
  }

  return ret;
}


// Local functions

/**
  \fn            static void USART_Server_Thread (void *argument)
  \brief         USART Server thread function.
  \detail        This is a thread function that waits to receive a command from USART Client 
                 (Driver Validation suite), and after command is received it is executed 
                 and the process starts again by waiting to receive next command.
  \param[in]     argument       Not used
  \return        none
*/
static void USART_Server_Thread (void *argument) {
  uint8_t i;

  (void)argument;

  for (;;) {
    switch (usart_server_state) {

      case USART_SERVER_STATE_RECEPTION:  // Receive a command
        if (USART_Com_Receive(usart_cmd_buf_rx, BYTES_TO_ITEMS(sizeof(usart_cmd_buf_rx),USART_SERVER_DATA_BITS), osWaitForever) == EXIT_SUCCESS) {
          usart_server_state = USART_SERVER_STATE_EXECUTION;
        }
        // If 32 byte command was not received restart the reception of 32 byte command
        break;

      case USART_SERVER_STATE_EXECUTION:  // Execute a command
        // Find the command and call handling function
        for (i = 0U; i < (sizeof(usart_cmd_desc) / sizeof(USART_CMD_DESC_t)); i++) {
          if (memcmp(usart_cmd_buf_rx, usart_cmd_desc[i].command, strlen(usart_cmd_desc[i].command)) == 0) {
            (void)usart_cmd_desc[i].Command_Func((const char *)usart_cmd_buf_rx);
            break;
          }
        }
        vioPrint(vioLevelMessage, "%.20s                    ", usart_cmd_buf_rx);
        usart_server_state = USART_SERVER_STATE_RECEPTION;
        break;

      case USART_SERVER_STATE_TERMINATE:  // Self-terminate the thread
      default:                            // Should never happen, processed as terminate request
        vioPrint(vioLevelError, "Server stopped!     ");
        (void)USART_Com_Abort();
        (void)osThreadTerminate(osThreadGetId());
        break;
    }
  }
}

/**
  \fn            static void USART_Com_Event (uint32_t event)
  \brief         USART communication event callback (called from USART driver from IRQ context).
  \detail        This function dispatches event (flag) to USART Server thread.
  \param[in]     event       USART event
                   - ARM_USART_EVENT_SEND_COMPLETE
                   - ARM_USART_EVENT_RECEIVE_COMPLETE
                   - ARM_USART_EVENT_TRANSFER_COMPLETE
                   - ARM_USART_EVENT_TX_COMPLETE
                   - ARM_USART_EVENT_TX_UNDERFLOW
                   - ARM_USART_EVENT_RX_OVERFLOW
                   - ARM_USART_EVENT_RX_TIMEOUT
                   - ARM_USART_EVENT_RX_BREAK
                   - ARM_USART_EVENT_RX_FRAMING_ERROR
                   - ARM_USART_EVENT_RX_PARITY_ERROR
                   - ARM_USART_EVENT_CTS
                   - ARM_USART_EVENT_DSR
                   - ARM_USART_EVENT_DCD
                   - ARM_USART_EVENT_RI
  \return        none
*/
static void USART_Com_Event (uint32_t event) {

  if ((event & ARM_USART_EVENT_RX_BREAK) != 0U) {
    break_status |= 1U;
  }

  if (usart_server_thread_id != NULL) {
    (void)osThreadFlagsSet(usart_server_thread_id, event);
  }
}

/**
  \fn            static int32_t USART_Com_Initialize (void)
  \brief         Initialize USART interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_Initialize (void) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvUSART->Initialize(USART_Com_Event) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t USART_Com_Uninitialize (void)
  \brief         Uninitialize USART interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_Uninitialize (void) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvUSART->Uninitialize() == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t USART_Com_PowerOn (void)
  \brief         Power-up USART interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_PowerOn (void) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvUSART->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t USART_Com_PowerOff (void)
  \brief         Power-down USART interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_PowerOff (void) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvUSART->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t USART_Com_Configure (const USART_COM_CONFIG_t *config)
  \brief         Configure USART interface.
  \param[in]     config      Pointer to structure containing USART interface configuration settings
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_Configure (const USART_COM_CONFIG_t *config) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvUSART->Control(config->mode                |
                        config->data_bits           |
                        config->parity              |
                        config->stop_bits           |
                        config->flow_control        |
                        config->cpol                |
                        config->cpha                |
                        config->irda_pulse          |
                        config->smartcard_clock     |
                        config->smartcard_guard_time,
                        config->baudrate            ) == ARM_DRIVER_OK) {
    usart_bytes_per_item = 1U;
    if (config->data_bits == ARM_USART_DATA_BITS_9) {
      usart_bytes_per_item = 2U;
    }
    ret = EXIT_SUCCESS;
  }
  if (ret == EXIT_SUCCESS) {
    if (drvUSART->Control(ARM_USART_CONTROL_RX, 1U) == ARM_DRIVER_OK) {
      ret = EXIT_SUCCESS;
    }
  }
  if (ret == EXIT_SUCCESS) {
    if (drvUSART->Control(ARM_USART_CONTROL_TX, 1U) == ARM_DRIVER_OK) {
      ret = EXIT_SUCCESS;
    }
  }

  return ret;
}

/**
  \fn            static int32_t USART_Com_Receive (void *data_in, uint32_t num, uint32_t timeout)
  \brief         Receive data (command) over USART interface.
  \param[out]    data_in     Pointer to memory where data will be received
  \param[in]     num         Number of data items to be received
  \param[in]     timeout     Timeout for reception (in ms)
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_Receive (void *data_in, uint32_t num, uint32_t timeout) {
   int32_t ret;
  uint32_t flags;

  ret = EXIT_FAILURE;

  if (usart_server_thread_id != NULL) {
    memset(data_in, (int32_t)'?', usart_bytes_per_item * num);
    vioSetSignal (vioLED0, vioLEDon);
    osThreadFlagsClear(0x7FFFFFFFU);
    if (drvUSART->Control(ARM_USART_CONTROL_RX, 1U) == ARM_DRIVER_OK) {
      if (drvUSART->Receive(data_in, num) == ARM_DRIVER_OK) {
        if (timeout == osWaitForever) {   // Reception of next command
          for (;;) {
            flags = osThreadFlagsWait(USART_RECEIVE_EVENTS_MASK, osFlagsWaitAny, 1U);
            if ((flags & 0x80000000U) != 0U) {    // If timeout
              if (drvUSART->GetRxCount() != 0U) {
                // If something was received, wait and try to receive complete command
                flags = osThreadFlagsWait(USART_RECEIVE_EVENTS_MASK, osFlagsWaitAny, USART_SERVER_CMD_TIMEOUT);
                if ((flags & (0x80000000U | ARM_USART_EVENT_RECEIVE_COMPLETE)) == ARM_USART_EVENT_RECEIVE_COMPLETE) {
                  // If completed event was signaled
                  ret = EXIT_SUCCESS;
                  break;
                } else {
                  // In all other cases exit with failed status
                  break;
                }
              }
            }
          }
        } else {                          // Reception during XFER
          flags = osThreadFlagsWait(ARM_USART_EVENT_RECEIVE_COMPLETE, osFlagsWaitAny, timeout);
          if ((flags & (0x80000000U | ARM_USART_EVENT_RECEIVE_COMPLETE)) == ARM_USART_EVENT_RECEIVE_COMPLETE) {
            // If completed event was signaled
            ret = EXIT_SUCCESS;
          }
        }
        if (ret != EXIT_SUCCESS) {
          // If receive was activated but failed to receive expected data then abort the reception
          (void)drvUSART->Control(ARM_USART_ABORT_RECEIVE, 0U);
        }
      }
      (void)drvUSART->Control(ARM_USART_CONTROL_RX, 0U);
    }
    vioSetSignal (vioLED0, vioLEDoff);
  }

  return ret;
}

/**
  \fn            static int32_t USART_Com_Send (const void *data_out, uint32_t num, uint32_t timeout)
  \brief         Send data (response) over USART interface.
  \param[out]    data_out       Pointer to memory containing data to be sent
  \param[in]     num            Number of data items to be sent
  \param[in]     timeout        Timeout for send (in ms)
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_Send (const void *data_out, uint32_t num, uint32_t timeout) {
  uint32_t flags;
   int32_t ret;
  uint8_t  tcnt;

  ret = EXIT_FAILURE;

  if (usart_server_thread_id != NULL) {
    vioSetSignal (vioLED1, vioLEDon);
    osThreadFlagsClear(0x7FFFFFFFU);
    if (drvUSART->Control(ARM_USART_CONTROL_TX, 1U) == ARM_DRIVER_OK) {
      if (drvUSART->Send(data_out, num) == ARM_DRIVER_OK) {
        if (drv_cap.event_tx_complete != 0U) {
          // If ARM_USART_EVENT_TX_COMPLETE is supported, wait for it
          flags = osThreadFlagsWait(ARM_USART_EVENT_TX_COMPLETE, osFlagsWaitAny, timeout);
          if ((flags & (0x80000000U | ARM_USART_EVENT_TX_COMPLETE)) == ARM_USART_EVENT_TX_COMPLETE) {
            // If completed event was signaled
            ret = EXIT_SUCCESS;
          }
        } else {
          // Otherwise wait for ARM_USART_EVENT_SEND_COMPLETE flag
          flags = osThreadFlagsWait(ARM_USART_EVENT_SEND_COMPLETE, osFlagsWaitAny, timeout);
          if ((flags & (0x80000000U | ARM_USART_EVENT_SEND_COMPLETE)) == ARM_USART_EVENT_SEND_COMPLETE) {
            // Wait for data to be transmitted on the Tx wire, up to 10 ms.
            // Send complete signals when data was consumed by FIFO not when it was actually 
            // sent on the wire.
            for (tcnt = 0U; tcnt < 10U; tcnt ++) {
              if (drvUSART->GetStatus().tx_busy == 0U) {
                ret = EXIT_SUCCESS;
                break;
              }
              (void)osDelay(1U);
            }
          }
        }
        if (ret != EXIT_SUCCESS) {
          // If send was activated but failed to send all of the expected data then abort the send
          (void)drvUSART->Control(ARM_USART_ABORT_SEND, 0U);
        }
      }
      (void)drvUSART->Control(ARM_USART_CONTROL_TX, 0U);
    }
    vioSetSignal (vioLED1, vioLEDoff);
  }

  return ret;
}

/**
  \fn            static int32_t USART_Com_Transfer (const void *data_out, void *data_in, uint32_t num, uint32_t timeout)
  \brief         Transfer (send/receive) data over USART interface.
  \param[in]     data_out       Pointer to memory containing data to be sent
  \param[out]    data_in        Pointer to memory where data will be received
  \param[in]     num            Number of data items to be transferred
  \param[in]     timeout        Timeout for transfer (in ms)
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_Transfer (const void *data_out, void *data_in, uint32_t num, uint32_t timeout) {
  uint32_t flags;
   int32_t ret;

  ret = EXIT_FAILURE;

  if (usart_server_thread_id != NULL) {
    vioSetSignal (vioLED2, vioLEDon);
    osThreadFlagsClear(0x7FFFFFFFU);
    if (drvUSART->Transfer(data_out, data_in, num) == ARM_DRIVER_OK) {
      flags = osThreadFlagsWait(USART_TRANSFER_EVENTS_MASK, osFlagsWaitAny, timeout);
      usart_xfer_cnt = drvUSART->GetTxCount();
      vioSetSignal (vioLED2, vioLEDoff);
      if ((flags & (0x80000000U | ARM_USART_EVENT_TRANSFER_COMPLETE)) == ARM_USART_EVENT_TRANSFER_COMPLETE) {
        // If completed event was signaled
        ret = EXIT_SUCCESS;
      } else {
        // If error or timeout
        (void)drvUSART->Control(ARM_USART_ABORT_TRANSFER, 0U);
      }
    }
  }

  return ret;
}

/**
  \fn            static int32_t USART_Com_Break (uint32_t val)
  \brief         Control USART Break signaling.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_Break (uint32_t val) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvUSART->Control(ARM_USART_CONTROL_BREAK, val) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t USART_Com_SetModemControl (ARM_USART_MODEM_CONTROL control)
  \brief         Set USART Modem Control line state.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_SetModemControl (ARM_USART_MODEM_CONTROL control) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvUSART->SetModemControl(control) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t USART_Com_Abort (void)
  \brief         Abort current transfer on USART interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Com_Abort (void) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvUSART->Control(ARM_USART_ABORT_TRANSFER, 0U) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static uint32_t USART_Com_GetCnt (void)
  \brief         Get number of data items transferred over USART interface in last transfer.
  \return        number of data items transferred
*/
static uint32_t USART_Com_GetCnt (void) {
  return usart_xfer_cnt;
}


// Command handling functions

/**
  \fn            static int32_t USART_Cmd_GetVer (const char *cmd)
  \brief         Handle command "GET VER".
  \detail        Return USART Server version over USART interface (16 bytes).
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_GetVer (const char *cmd) {

  (void)cmd;

  memset(usart_cmd_buf_tx, 0, 16);
  memcpy(usart_cmd_buf_tx, USART_SERVER_VER, sizeof(USART_SERVER_VER));

  (void)osDelay(10U);                   // Give client time to start the reception

  return (USART_Com_Send(usart_cmd_buf_tx, BYTES_TO_ITEMS(16U, USART_SERVER_DATA_BITS), usart_cmd_timeout));
}

/**
  \fn            static int32_t USART_Cmd_GetCap (const char *cmd)
  \brief         Handle command "GET CAP".
  \detail        Return USART Server capabilities over USART interface (32 bytes).
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_GetCap (const char *cmd) {
  int32_t  ret;
  uint32_t mode_mask, data_bits_mask, parity_mask, stop_bits_mask;
  uint32_t flow_control_mask, modem_lines_mask, min_baudrate, max_baudrate;
  uint32_t br, def_br;
   int8_t  i;

  // Disable Rx and Tx during capabilities detection
  drvUSART->Control(ARM_USART_CONTROL_RX, 0U);
  drvUSART->Control(ARM_USART_CONTROL_TX, 0U);

  (void)cmd;

  ret = EXIT_FAILURE;

  def_br = usart_com_config_default.baudrate;

  // Determine supported minimum baudrate
  // Find minimum baudrate setting at which Control function succeeds
  for (i = 0; i < (sizeof(usart_baudrates) / sizeof(uint32_t)); i++) {
    if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS, usart_baudrates[i]) == ARM_DRIVER_OK) {
      break;
    }
  }
  if (i == (sizeof(usart_baudrates) / sizeof(uint32_t))) {
    i -= 1;
  }
  min_baudrate = usart_baudrates[i];

  // Determine supported maximum baudrate
  // Find maximum baudrate setting at which Control function succeeds
  for (i = ((sizeof(usart_baudrates) / sizeof(uint32_t))) - 1; i >= 0U; i--) {
    if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS, usart_baudrates[i]) == ARM_DRIVER_OK) {
      break;
    }
  }
  if (i < 0) {
    i = 0;
  }
  max_baudrate = usart_baudrates[i];

  // Check baudrate values which are power of 10 for maximum supported baudrate
  br = 100000000U;
  do {
    if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS, br) == ARM_DRIVER_OK) {
      break;
    }
    if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS, br / 2U) == ARM_DRIVER_OK) {
      br /= 2U;
      break;
    }
    if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS, br / 5U) == ARM_DRIVER_OK) {
      br /= 5U;
      break;
    }
    br /= 10U;
    if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS, br) == ARM_DRIVER_OK) {
      break;
    }
  } while (br > 10000U);
  if (br > max_baudrate) {
    max_baudrate = br;
  }

  // Determine supported modes
  mode_mask = 0U;
  if (drv_cap.asynchronous != 0U) {
    mode_mask |= 1U;
  }
  if (drv_cap.synchronous_master != 0U) {
    mode_mask |= 1U << 1;
  }
  if (drv_cap.synchronous_slave != 0U) {
    mode_mask |= 1U << 2;
  }
  if (drv_cap.single_wire != 0U) {
    mode_mask |= 1U << 3;
  }
  if (drv_cap.irda != 0U) {
    mode_mask |= 1U << 4;
  }
  if (drv_cap.smart_card != 0U) {
    mode_mask |= 1U << 5;
  }

  // Determine supported data bits
  data_bits_mask = 0U;
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_5, def_br) == ARM_DRIVER_OK) {
    data_bits_mask |= 1U;
  }
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_6, def_br) == ARM_DRIVER_OK) {
    data_bits_mask |= 1U << 1;
  }
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_7, def_br) == ARM_DRIVER_OK) {
    data_bits_mask |= 1U << 2;
  }
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8, def_br) == ARM_DRIVER_OK) {
    data_bits_mask |= 1U << 3;
  }
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_9, def_br) == ARM_DRIVER_OK) {
    data_bits_mask |= 1U << 4;
  }

  // Determine supported parity options
  parity_mask = 0U;
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_PARITY_NONE, def_br) == ARM_DRIVER_OK) {
    parity_mask |= 1U;
  }
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_PARITY_EVEN, def_br) == ARM_DRIVER_OK) {
    parity_mask |= 1U << 1;
  }
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_PARITY_ODD,  def_br) == ARM_DRIVER_OK) {
    parity_mask |= 1U << 2;
  }

  // Determine supported stop bits
  stop_bits_mask = 0U;
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_STOP_BITS_1,   def_br) == ARM_DRIVER_OK) {
    stop_bits_mask |= 1U;
  }
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_STOP_BITS_2,   def_br) == ARM_DRIVER_OK) {
    stop_bits_mask |= 1U << 1;
  }
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_STOP_BITS_1_5, def_br) == ARM_DRIVER_OK) {
    stop_bits_mask |= 1U << 2;
  }
  if (drvUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_STOP_BITS_0_5, def_br) == ARM_DRIVER_OK) {
    stop_bits_mask |= 1U << 3;
  }

  // Determine supported flow control options
  flow_control_mask = 1U;       // No flow control is always supported
  if ((drv_cap.cts != 0U) && (drv_cap.flow_control_cts != 0U)) {
    flow_control_mask |= 1U << 1;
  }
  if (drv_cap.rts != 0U) {
    flow_control_mask |= 1U << 2;
  }
  if ((drv_cap.rts != 0U) && (drv_cap.cts != 0U) && (drv_cap.flow_control_cts != 0U)) {
    flow_control_mask |= 1U << 3;
  }

  // Determine available modem lines
  modem_lines_mask = 0U;
  if (drv_cap.rts != 0U) {
    modem_lines_mask |= 1U;
  }
  if (drv_cap.cts != 0U) {
    modem_lines_mask |= 1U << 1;
  }
  if (drv_cap.dtr != 0U) {
    modem_lines_mask |= 1U << 2;
  }
  if (drv_cap.dsr != 0U) {
    modem_lines_mask |= 1U << 3;
  }
  if ((dcd_ri_mask & 1U) != 0U) {
    modem_lines_mask |= 1U << 4;
  }
  if ((dcd_ri_mask & 2U) != 0U) {
    modem_lines_mask |= 1U << 5;
  }

  // Revert communication settings to default because they were changed during auto-detection of capabilities
  (void)USART_Com_Configure(&usart_com_config_default);

  (void)osDelay(25U);                   // Give client time to start the reception

  memset(usart_cmd_buf_tx, 0, 32);
  if (snprintf((char *)usart_cmd_buf_tx, 32, "%02X,%02X,%01X,%01X,%01X,%01X,%i,%i", 
                mode_mask, 
                data_bits_mask, 
                parity_mask, 
                stop_bits_mask, 
                flow_control_mask, 
                modem_lines_mask, 
                min_baudrate, 
                max_baudrate) <= 32) {
    ret = USART_Com_Send(usart_cmd_buf_tx, BYTES_TO_ITEMS(32U, USART_SERVER_DATA_BITS), usart_cmd_timeout);
  }

  return ret;
}

/**
  \fn            static int32_t USART_Cmd_SetBuf (const char *cmd)
  \brief         Handle command "SET BUF RX/TX,len[,pattern]".
  \detail        Initialize content of the buffer in the following way:
                  - fill whole buffer with 'pattern' value if it is specified, and 'len' is 0
                  - fill whole buffer with 0 if 'pattern' is not provided and 'len' is 0
                  - load 'len' bytes from start of the buffer with content 
                    received in IN data phase
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_SetBuf (const char *cmd) {
  const char     *ptr_str;
        uint8_t  *ptr_buf;
        uint32_t  val, len;
        uint8_t   pattern;
         int32_t  ret;

  ret     = EXIT_SUCCESS;
  ptr_str = NULL;
  ptr_buf = NULL;
  val     = 0U;
  len     = 0U;
  pattern = 0U;

  // Parse 'RX' or 'TX' selection
  if        (strstr(cmd, "RX") != NULL) {
    ptr_buf = ptr_usart_xfer_buf_rx;
  } else if (strstr(cmd, "TX") != NULL) {
    ptr_buf = ptr_usart_xfer_buf_tx;
  } else {
    ret = EXIT_FAILURE;
  }

  if (ret == EXIT_SUCCESS) {
    // Parse 'len'
    ptr_str = strstr(cmd, ",");         // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (val <= usart_xfer_buf_size) {
          len = val;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse optional 'pattern'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%x", &val) == 1) {
        pattern = (uint8_t)val;
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_buf != NULL)) {
    // Fill the whole buffer with 'pattern', if 'pattern' was not specified 
    // in the command then the whole buffer will be filled with 0
    memset(ptr_buf, (int32_t)pattern, usart_xfer_buf_size);
  }

  if ((ret == EXIT_SUCCESS) && (ptr_buf != NULL) && (len != 0U)) {
    // Load 'len' bytes from start of buffer with content received in IN data phase
    ret = USART_Com_Receive(ptr_buf, BYTES_TO_ITEMS(len, USART_SERVER_DATA_BITS), usart_cmd_timeout);
  }

  return ret;
}

/**
  \fn            static int32_t USART_Cmd_GetBuf (const char *cmd)
  \brief         Handle command "GET BUF RX/TX,len".
  \detail        Send content of buffer over USART interface.
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_GetBuf (const char *cmd) {
  const char     *ptr_str;
  const uint8_t  *ptr_buf;
        uint32_t  val, len;
         int32_t  ret;

  ret      = EXIT_SUCCESS;
  ptr_str  = NULL;
  ptr_buf  = NULL;
  val      = 0U;
  len      = 0U;

  // Parse 'RX' or 'TX' selection
  if        (strstr(cmd, "RX") != NULL) {
    ptr_buf = ptr_usart_xfer_buf_rx;
  } else if (strstr(cmd, "TX") != NULL) {
    ptr_buf = ptr_usart_xfer_buf_tx;
  } else {
    ret = EXIT_FAILURE;
  }

  if (ret == EXIT_SUCCESS) {
    // Parse 'len'
    ptr_str = strstr(cmd, ",");         // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if ((val > 0U) && (val <= usart_xfer_buf_size)) {
          len = val;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }

  (void)osDelay(10U);                   // Give client time to start the reception

  if ((ret == EXIT_SUCCESS) && (ptr_buf != NULL) && (len != 0U)) {
    ret = USART_Com_Send(ptr_buf, BYTES_TO_ITEMS(len, USART_SERVER_DATA_BITS), usart_cmd_timeout);
  }

  return ret;
}

/**
  \fn            static int32_t USART_Cmd_SetCom (const char *cmd)
  \brief         Handle command "SET COM mode,data_bits,parity,stop_bits,flow_ctrl,cpol,cpha,baudrate".
  \detail        Set communication configuration settings used for transfers (XFER commands).
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_SetCom (const char *cmd) {
  const char    *ptr_str;
        uint32_t val;
         int32_t ret;

  ret = EXIT_SUCCESS;
  val = 0U;

  ptr_str = &cmd[7];                    // Skip "SET COM"
  while (*ptr_str == ' ') {             // Skip whitespaces
    ptr_str++;
  }

  // Parse 'mode'
  if (sscanf(ptr_str, "%u", &val) == 1) {
    switch (val) {
      case 1U:                          // Asynchronous mode
        usart_com_config_xfer.mode = ARM_USART_MODE_ASYNCHRONOUS;
        break;
      case 2U:                          // Synchronous Master mode
        usart_com_config_xfer.mode = ARM_USART_MODE_SYNCHRONOUS_MASTER;
        break;
      case 3U:                          // Synchronous Slave mode
        usart_com_config_xfer.mode = ARM_USART_MODE_SYNCHRONOUS_SLAVE;
        break;
      case 4U:                          // Single Wire mode
        usart_com_config_xfer.mode = ARM_USART_MODE_SINGLE_WIRE;
        break;
      case 5U:                          // IrDA mode
        usart_com_config_xfer.mode = ARM_USART_MODE_IRDA;
        break;
      case 6U:                          // Smart Card mode
        usart_com_config_xfer.mode = ARM_USART_MODE_SMART_CARD;
        break;
      default:
        ret = EXIT_FAILURE;
        break;
    }
  } else {
    ret = EXIT_FAILURE;
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'data_bits'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        switch (val) {
          case 5U:                      // 5 data bits
            usart_com_config_xfer.data_bits = ARM_USART_DATA_BITS_5;
            break;
          case 6U:                      // 6 data bits
            usart_com_config_xfer.data_bits = ARM_USART_DATA_BITS_6;
            break;
          case 7U:                      // 7 data bits
            usart_com_config_xfer.data_bits = ARM_USART_DATA_BITS_7;
            break;
          case 8U:                      // 8 data bits
            usart_com_config_xfer.data_bits = ARM_USART_DATA_BITS_8;
            break;
          case 9U:                      // 9 data bits
            usart_com_config_xfer.data_bits = ARM_USART_DATA_BITS_9;
            break;
          default:
            ret = EXIT_FAILURE;
            break;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'parity'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        switch (val) {
          case 0U:                      // No parity
            usart_com_config_xfer.parity = ARM_USART_PARITY_NONE;
            break;
          case 1U:                      // Even parity
            usart_com_config_xfer.parity = ARM_USART_PARITY_EVEN;
            break;
          case 2U:                      // Odd parity
            usart_com_config_xfer.parity = ARM_USART_PARITY_ODD;
            break;
          default:
            ret = EXIT_FAILURE;
            break;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'stop_bits'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        switch (val) {
          case 0U:                      // 1 stop bit
            usart_com_config_xfer.stop_bits = ARM_USART_STOP_BITS_1;
            break;
          case 1U:                      // 2 stop bits
            usart_com_config_xfer.stop_bits = ARM_USART_STOP_BITS_2;
            break;
          case 2U:                      // 1.5 stop bits
            usart_com_config_xfer.stop_bits = ARM_USART_STOP_BITS_1_5;
            break;
          case 3U:                      // 0.5 stop bits
            usart_com_config_xfer.stop_bits = ARM_USART_STOP_BITS_0_5;
            break;
          default:
            ret = EXIT_FAILURE;
            break;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'flow_ctrl'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        switch (val) {
          case 0U:                      // No flow control
            usart_com_config_xfer.flow_control = ARM_USART_FLOW_CONTROL_NONE;
            break;
          case 1U:                      // RTS
            usart_com_config_xfer.flow_control = ARM_USART_FLOW_CONTROL_RTS;
            break;
          case 2U:                      // CTS
            usart_com_config_xfer.flow_control = ARM_USART_FLOW_CONTROL_CTS;
            break;
          case 3U:                      // RTS/CTS
            usart_com_config_xfer.flow_control = ARM_USART_FLOW_CONTROL_RTS_CTS;
            break;
          default:
            ret = EXIT_FAILURE;
            break;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'cpol'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        switch (val) {
          case 0U:                      // Data are captured on rising edge
            usart_com_config_xfer.cpol = ARM_USART_CPOL0;
            break;
          case 1U:                      // Data are captured on falling edge
            usart_com_config_xfer.cpol = ARM_USART_CPOL1;
            break;
          default:
            ret = EXIT_FAILURE;
            break;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'cpha'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        switch (val) {
          case 0U:                      // Sample on first (leading) edge
            usart_com_config_xfer.cpha = ARM_USART_CPHA0;
            break;
          case 1U:                      // Sample on second (trailing) edge
            usart_com_config_xfer.cpha = ARM_USART_CPHA1;
            break;
          default:
            ret = EXIT_FAILURE;
            break;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'baudrate'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        usart_com_config_xfer.baudrate = val;
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  return ret;
}

/**
  \fn            static int32_t USART_Cmd_Xfer (const char *cmd)
  \brief         Handle command "XFER dir,num[,delay][,timeout][,num_rts]".
  \detail        Send data from USART TX buffer if dir = 0, receive data to USART RX buffer 
                 if dir = 1, or do both if dir = 2.
                 (buffers must be set with "SET BUF" command before this command).
                 Transfer start is delayed by optional parameter 'delay' in milliseconds.
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_Xfer (const char *cmd) {
  const char    *ptr_str;
        uint32_t val, dir, num, delay, num_rts;
         int32_t ret;
        uint8_t  num_rts_provided;

  ret              = EXIT_SUCCESS;
  val              = 0U;
  dir              = 0U;
  num              = 0U;
  delay            = 0U;
  num_rts          = 0U;
  num_rts_provided = 0U;

  ptr_str = &cmd[4];                    // Skip "XFER"
  while (*ptr_str == ' ') {             // Skip whitespaces
    ptr_str++;
  }

  // Parse 'dir'
  if (sscanf(ptr_str, "%u", &val) == 1) {
    if ((val >= 0U) && (val <= 2U)) {
      dir = val;
    } else {
      ret = EXIT_FAILURE;
    }
  } else {
    ret = EXIT_FAILURE;
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'num'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if ((val > 0U) && (val <= usart_xfer_buf_size)) {
          num = val;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse optional 'delay'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (val != osWaitForever) {
          delay = val;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse optional 'timeout'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (val != osWaitForever) {
          usart_xfer_timeout = val;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse optional 'num_rts'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (val <= num) {
          num_rts = val;
          num_rts_provided = 1U;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (delay != 0U)) {
    (void)osDelay(delay);
  }

  if (ret == EXIT_SUCCESS) {
    // Configure communication settings before transfer
    ret = USART_Com_Configure(&usart_com_config_xfer);

    if (ret == EXIT_SUCCESS) {
      switch (dir) {
        case 0U:                        // Send
          ret = USART_Com_Send(ptr_usart_xfer_buf_tx, num, usart_xfer_timeout);
          usart_xfer_cnt = drvUSART->GetTxCount();
          break;
        case 1U:                        // Receive
          if (num_rts_provided == 0U) { // Normal Receive
            ret = USART_Com_Receive(ptr_usart_xfer_buf_rx, num, usart_xfer_timeout);
            usart_xfer_cnt = drvUSART->GetRxCount();
          } else {                      // Special handling for activation of Server's RTS line => Client's CTS line
            USART_Com_SetModemControl(ARM_USART_RTS_SET);
            ret = USART_Com_Receive(ptr_usart_xfer_buf_rx, num_rts, (usart_xfer_timeout * num_rts) / num);
            usart_xfer_cnt  = drvUSART->GetRxCount();
            USART_Com_SetModemControl(ARM_USART_RTS_CLEAR);

            USART_Com_Receive(ptr_usart_xfer_buf_rx + num_rts, num - num_rts, (usart_xfer_timeout * (num - num_rts)) / num);
            usart_xfer_cnt += drvUSART->GetRxCount();
          }
          break;
        case 2U:                        // Transfer
          ret = USART_Com_Transfer(ptr_usart_xfer_buf_tx, ptr_usart_xfer_buf_rx, num, usart_xfer_timeout);
          usart_xfer_cnt = drvUSART->GetRxCount();
          break;
        default:
          ret = EXIT_FAILURE;
          break;
      }
    }
  }

  // Revert communication settings to default
  (void)USART_Com_Configure(&usart_com_config_default);

  return ret;
}

/**
  \fn            static int32_t USART_Cmd_GetCnt (const char *cmd)
  \brief         Handle command "GET CNT".
  \detail        Return number of items transferred (sent/received/transferred) in last transfer 
                 (requested by last XFER command).
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_GetCnt (const char *cmd) {
  int32_t ret;

  (void)cmd;

  ret = EXIT_FAILURE;

  (void)osDelay(10U);                   // Give client time to start the reception

  memset(usart_cmd_buf_tx, 0, 16);
  if (snprintf((char *)usart_cmd_buf_tx, 16, "%u", USART_Com_GetCnt()) < 16) {
    ret = USART_Com_Send(usart_cmd_buf_tx, BYTES_TO_ITEMS(16U, USART_SERVER_DATA_BITS), usart_cmd_timeout);
  }

  return ret;
}

/**
  \fn            static int32_t USART_Cmd_SetBrk (const char *cmd)
  \brief         Handle command "SET BRK delay,duration".
  \detail        Control break signal.
                 After specified delay time activate break signal 
                 and hold it for specified duration.
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_SetBrk (const char *cmd) {
  const char    *ptr_str;
        uint32_t val, delay, duration;
         int32_t ret;

  ret      = EXIT_SUCCESS;
  val      = 0U;
  delay    = 0U;
  duration = 0U;

  ptr_str = &cmd[7];                    // Skip "SET BRK"
  while (*ptr_str == ' ') {             // Skip whitespaces
    ptr_str++;
  }

  // Parse 'delay'
  if (sscanf(ptr_str, "%u", &val) == 1) {
    if (val != osWaitForever) {
      delay = val;
    } else {
      ret = EXIT_FAILURE;
    }
  } else {
    ret = EXIT_FAILURE;
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'duration'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (val != osWaitForever) {
          duration = val;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (delay != 0U)) {
    (void)osDelay(delay);
  }

  if (ret == EXIT_SUCCESS) {
    ret = USART_Com_Break(1U);
  }

  if ((ret == EXIT_SUCCESS) && (duration != 0U)) {
    (void)osDelay(duration);
  }

  USART_Com_Break(0U);

  return ret;
}

/**
  \fn            static int32_t USART_Cmd_GetBrk (const char *cmd)
  \brief         Handle command "GET BRK".
  \detail        Return break status. Status is updated upon break event and cleared 
                 after this command is executed.
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_GetBrk (const char *cmd) {
  int32_t ret;

  (void)cmd;

  ret = EXIT_FAILURE;

  usart_cmd_buf_tx[0] = '0' + break_status;
  break_status = 0U;

  (void)osDelay(10U);                   // Give client time to start the reception

  ret = USART_Com_Send(usart_cmd_buf_tx, 1U, usart_cmd_timeout);

  return ret;
}

/**
  \fn            static int32_t USART_Cmd_SetMdm (const char *cmd)
  \brief         Handle command "SET MDM mdm_ctrl,delay,duration".
  \detail        Control modem lines.
                 After specified delay time activate requested line states 
                 and hold it for specified duration.
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_SetMdm (const char *cmd) {
  const char    *ptr_str;
        uint32_t val, mdm_ctrl, delay, duration, control;
         int32_t ret;

  ret      = EXIT_SUCCESS;
  val      = 0U;
  mdm_ctrl = 0U;
  delay    = 0U;
  duration = 0U;
  control  = 0U;

  ptr_str = &cmd[7];                    // Skip "SET MDM"
  while (*ptr_str == ' ') {             // Skip whitespaces
    ptr_str++;
  }

  // Parse 'mdm_ctrl'
  if (sscanf(ptr_str, "%x", &val) == 1) {
    mdm_ctrl = val;
  } else {
    ret = EXIT_FAILURE;
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'delay'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (val != osWaitForever) {
          delay = val;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'duration'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (val != osWaitForever) {
          duration = val;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if (ret == EXIT_SUCCESS) {
    // Deactivate all lines initially
    if (drv_cap.rts == 1U) {
      (void)USART_Com_SetModemControl(ARM_USART_RTS_CLEAR);
    }
    if (drv_cap.dtr == 1U) {
      (void)USART_Com_SetModemControl(ARM_USART_DTR_CLEAR);
    }
    USART_Server_Pin_DCD_SetState(0U);
    USART_Server_Pin_RI_SetState (0U);

    if (delay != 0U) {
      (void)osDelay(delay);
    }
  }

  // Lower 4 bits are same as in USART ARM_USART_MODEM_CONTROL
  // bit 0. = RTS state
  // bit 1. = DTR state
  // bit 2. = DCD state
  // bit 3. = RI  state
  if (ret == EXIT_SUCCESS) {
    if (drv_cap.rts == 1U) {
      if ((mdm_ctrl & (1U     )) != 0U) {
        ret = USART_Com_SetModemControl(ARM_USART_RTS_SET);
      } else {
        ret = USART_Com_SetModemControl(ARM_USART_RTS_CLEAR);
      }
    }
  }
  if (ret == EXIT_SUCCESS) {
    if (drv_cap.dtr == 1U) {
      if ((mdm_ctrl & (1U << 1)) != 0U) {
        ret = USART_Com_SetModemControl(ARM_USART_DTR_SET);
      } else {
        ret = USART_Com_SetModemControl(ARM_USART_DTR_CLEAR);
      }
    }
  }

  if (ret == EXIT_SUCCESS) {
    if ((mdm_ctrl >> 2) & 1) {
      // If pin to DCD should be set to active state
      USART_Server_Pin_DCD_SetState(1U);
    }
    if ((mdm_ctrl >> 3) & 1) {
      // If pin to RI should be set to active state
      USART_Server_Pin_RI_SetState(1U);
    }

    if (duration != 0U) {
      (void)osDelay(duration);
    }
  }

  if (drv_cap.rts == 1U) {
    (void)USART_Com_SetModemControl(ARM_USART_RTS_CLEAR);
  }
  if (drv_cap.dtr == 1U) {
    (void)USART_Com_SetModemControl(ARM_USART_DTR_CLEAR);
  }
  USART_Server_Pin_DCD_SetState(0U);
  USART_Server_Pin_RI_SetState (0U);

  return ret;
}

/**
  \fn            static int32_t USART_Cmd_GetMdm (const char *cmd)
  \brief         Handle command "GET MDM".
  \detail        Return modem lines current status.
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t USART_Cmd_GetMdm (const char *cmd) {
  int32_t                ret;
  ARM_USART_MODEM_STATUS val;

  (void)cmd;

  ret = EXIT_FAILURE;

  val = drvUSART->GetModemStatus();

  usart_cmd_buf_tx[0] = '0' + (val.cts) + (2U * val.dsr);

  (void)osDelay(10U);                   // Give client time to start the reception

  ret = USART_Com_Send(usart_cmd_buf_tx, 1U, usart_cmd_timeout);

  return ret;
}
