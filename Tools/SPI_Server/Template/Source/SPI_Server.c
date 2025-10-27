/*
 * Copyright (c) 2020-2022 Arm Limited. All rights reserved.
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
 * Project:     SPI Server
 * Title:       SPI Server application
 *
 * -----------------------------------------------------------------------------
 */


#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "SPI_Server_Config.h"
#include "SPI_Server.h"

#include "cmsis_os2.h"
#include "cmsis_compiler.h"
#include "cmsis_vio.h"

#include "Driver_SPI.h"                 // ::CMSIS Driver:SPI

#ifndef  SPI_SERVER_DEBUG
#define  SPI_SERVER_DEBUG       0
#endif

// Fixed SPI Server settings (not available through SPI_Server_Config.h)
#define  SPI_SERVER_SS_MODE     2       // Slave Select Hardware monitored
#define  SPI_SERVER_FORMAT      0       // Clock Polarity 0 / Clock Phase 0
#define  SPI_SERVER_DATA_BITS   8       // 8 data bits
#define  SPI_SERVER_BIT_ORDER   0       // MSB to LSB bit order

#define  SPI_EVENTS_MASK       (ARM_SPI_EVENT_TRANSFER_COMPLETE | \
                                ARM_SPI_EVENT_DATA_LOST         | \
                                ARM_SPI_EVENT_MODE_FAULT)

#define  DATA_BITS_TO_BYTES(data_bits)      (((data_bits) > 16) ? (4U) : (((data_bits) > 8) ? (2U) : (1U)))
#define  BYTES_TO_ITEMS(bytes,data_bits)    ((bytes + DATA_BITS_TO_BYTES(data_bits) - 1U) / DATA_BITS_TO_BYTES(data_bits))

/* Access to Driver_SPI# */
#define  SPI_Driver_Aux(n)      Driver_SPI##n
#define  SPI_Driver_Name(n)     SPI_Driver_Aux(n)
extern   ARM_DRIVER_SPI         SPI_Driver_Name(SPI_SERVER_DRV_NUM);
#define  drvSPI               (&SPI_Driver_Name(SPI_SERVER_DRV_NUM))

typedef struct {                // SPI Interface settings structure
  uint32_t mode;
  uint32_t format;
  uint32_t bit_num;
  uint32_t bit_order;
  uint32_t ss_mode;
  uint32_t bus_speed;
} SPI_COM_CONFIG_t;

// Structure containing command string and pointer to command handling function
typedef struct {
  const char     *command;
        int32_t (*Command_Func) (const char *command);
} SPI_CMD_DESC_t;

// Local functions

// Main thread (reception and execution of command)
__NO_RETURN \
static void     SPI_Server_Thread    (void *argument);

// SPI Interface communication functions
static void     SPI_Com_Event        (uint32_t event);
static int32_t  SPI_Com_Initialize   (void);
static int32_t  SPI_Com_Uninitialize (void);
static int32_t  SPI_Com_PowerOn      (void);
static int32_t  SPI_Com_PowerOff     (void);
static int32_t  SPI_Com_Configure    (const SPI_COM_CONFIG_t *config);
static uint32_t SPI_Com_SS           (uint32_t active);
static int32_t  SPI_Com_Receive      (                      void *data_in, uint32_t num, uint32_t timeout);
static int32_t  SPI_Com_Send         (const void *data_out,                uint32_t num, uint32_t timeout);
static int32_t  SPI_Com_Transfer     (const void *data_out, void *data_in, uint32_t num, uint32_t timeout);
static int32_t  SPI_Com_Abort        (void);
static uint32_t SPI_Com_GetCnt       (void);

// Command handling functions
static int32_t  SPI_Cmd_GetVer       (const char *cmd);
static int32_t  SPI_Cmd_GetCap       (const char *cmd);
static int32_t  SPI_Cmd_SetBuf       (const char *cmd);
static int32_t  SPI_Cmd_GetBuf       (const char *cmd);
static int32_t  SPI_Cmd_SetCom       (const char *cmd);
static int32_t  SPI_Cmd_Xfer         (const char *cmd);
static int32_t  SPI_Cmd_GetCnt       (const char *cmd);

// Local variables

// Command specification (command string, command handling function)
static const SPI_CMD_DESC_t spi_cmd_desc[] = {
 { "GET VER" , SPI_Cmd_GetVer },
 { "GET CAP" , SPI_Cmd_GetCap },
 { "SET BUF" , SPI_Cmd_SetBuf },
 { "GET BUF" , SPI_Cmd_GetBuf },
 { "SET COM" , SPI_Cmd_SetCom },
 { "XFER"    , SPI_Cmd_Xfer   },
 { "GET CNT" , SPI_Cmd_GetCnt }
};

static       osThreadId_t       spi_server_thread_id   =   NULL;
static       osThreadAttr_t     thread_attr = {
  .name       = "SPI_Server_Thread",
  .stack_size = 512U
};

static       uint8_t            spi_server_state       =   SPI_SERVER_STATE_RECEPTION;
static       uint32_t           spi_cmd_timeout        =   SPI_SERVER_CMD_TIMEOUT;
static       uint32_t           spi_xfer_timeout       =   SPI_SERVER_CMD_TIMEOUT;
static       uint32_t           spi_xfer_cnt           =   0U;
static       uint32_t           spi_xfer_buf_size      =   SPI_SERVER_BUF_SIZE;
static const SPI_COM_CONFIG_t   spi_com_config_default = { ARM_SPI_MODE_SLAVE, 
                                                         ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk), 
                                                         ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk), 
                                                         ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk), 
                                                           ARM_SPI_SS_SLAVE_HW,
                                                           0U   // Bus speed for Slave mode is unused
                                                         };
static const SPI_COM_CONFIG_t   spi_com_config_inactive= { ARM_SPI_MODE_INACTIVE, 0U, 0U, 0U, 0U, 0U };
static       SPI_COM_CONFIG_t   spi_com_config_xfer;
static       uint8_t            spi_bytes_per_item        = 1U;
static       uint8_t            spi_cmd_buf_rx[32]        __ALIGNED(4);
static       uint8_t            spi_cmd_buf_tx[32]        __ALIGNED(4);
static       uint8_t           *ptr_spi_xfer_buf_rx       = NULL;
static       uint8_t           *ptr_spi_xfer_buf_tx       = NULL;
static       void              *ptr_spi_xfer_buf_rx_alloc = NULL;
static       void              *ptr_spi_xfer_buf_tx_alloc = NULL;

// Global functions

/**
  \fn            int32_t SPI_Server_Start (void)
  \brief         Initialize, power up, configure SPI interface and start SPI Server thread.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
int32_t SPI_Server_Start (void) {
  int32_t ret;

#ifdef DEBUG
  printf("SPI Server v%s\r\n", SPI_SERVER_VER);
#endif

  // Initialize local variables
  spi_server_state   = SPI_SERVER_STATE_RECEPTION;
  spi_cmd_timeout    = SPI_SERVER_CMD_TIMEOUT;
  spi_xfer_timeout   = SPI_SERVER_CMD_TIMEOUT;
  spi_xfer_cnt       = 0U;
  spi_xfer_buf_size  = SPI_SERVER_BUF_SIZE;
  spi_bytes_per_item = DATA_BITS_TO_BYTES(SPI_SERVER_DATA_BITS);
  memset(spi_cmd_buf_rx,  0, sizeof(spi_cmd_buf_rx));
  memset(spi_cmd_buf_tx,  0, sizeof(spi_cmd_buf_tx));
  memcpy(&spi_com_config_xfer, &spi_com_config_default, sizeof(SPI_COM_CONFIG_t));

  // Allocate buffers for data transmission and reception
  // (maximum size is incremented by 4 bytes to ensure that buffer can be aligned to 4 bytes)

  ptr_spi_xfer_buf_rx_alloc = malloc(SPI_SERVER_BUF_SIZE + 4U);
  if (((uint32_t)ptr_spi_xfer_buf_rx_alloc & 3U) != 0U) {
    // If allocated memory is not 4 byte aligned, use next 4 byte aligned address for ptr_tx_buf
    ptr_spi_xfer_buf_rx = (uint8_t *)((((uint32_t)ptr_spi_xfer_buf_rx_alloc) + 3U) & (~3U));
  } else {
    // If allocated memory is 4 byte aligned, use it directly
    ptr_spi_xfer_buf_rx = (uint8_t *)ptr_spi_xfer_buf_rx_alloc;
  }
  ptr_spi_xfer_buf_tx_alloc = malloc(SPI_SERVER_BUF_SIZE + 4U);
  if (((uint32_t)ptr_spi_xfer_buf_tx_alloc & 3U) != 0U) {
    // If allocated memory is not 4 byte aligned, use next 4 byte aligned address for ptr_tx_buf
    ptr_spi_xfer_buf_tx = (uint8_t *)((((uint32_t)ptr_spi_xfer_buf_tx_alloc) + 3U) & (~3U));
  } else {
    // If allocated memory is 4 byte aligned, use it directly
    ptr_spi_xfer_buf_tx = (uint8_t *)ptr_spi_xfer_buf_tx_alloc;
  }

  if ((ptr_spi_xfer_buf_rx != NULL) || (ptr_spi_xfer_buf_tx != NULL)) {
    memset(ptr_spi_xfer_buf_rx, 0, SPI_SERVER_BUF_SIZE);
    memset(ptr_spi_xfer_buf_rx, 0, SPI_SERVER_BUF_SIZE);
    ret = EXIT_SUCCESS;
  } else {
    ret = EXIT_FAILURE;
  }

  if (ret == EXIT_SUCCESS) {
    ret = SPI_Com_Initialize();
  }

  if (ret == EXIT_SUCCESS) {
    ret = SPI_Com_PowerOn();
  }

  if (ret == EXIT_SUCCESS) {
    ret = SPI_Com_Configure(&spi_com_config_default);
  }

  if ((ret == EXIT_SUCCESS) && (spi_server_thread_id == NULL)) {
    // Create SPI_Server_Thread thread
    spi_server_thread_id = osThreadNew(SPI_Server_Thread, NULL, &thread_attr);
    if (spi_server_thread_id == NULL) {
      ret = EXIT_FAILURE;
    }
  }

#ifdef DEBUG
  if (ret != EXIT_SUCCESS) {
    printf("Server Start failed!\r\n");
  }
#endif

  return ret;
}

/**
  \fn            int32_t SPI_Server_Stop (void)
  \brief         Terminate SPI Server thread, power down and uninitialize SPI interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
int32_t SPI_Server_Stop (void) {
   int32_t ret;
  uint32_t i;

  ret = EXIT_FAILURE;

  if (spi_server_thread_id != NULL) {
    spi_server_state = SPI_SERVER_STATE_TERMINATE;
    for (i = 0U; i < 10U; i++) {
      if (osThreadGetState(spi_server_thread_id) == osThreadTerminated) {
        spi_server_thread_id = NULL;
        ret = EXIT_SUCCESS;
        break;
      }
      (void)osDelay(100U);
    }
  }

  if (ret == EXIT_SUCCESS) {
    ret = SPI_Com_PowerOff();
  }

  if (ret == EXIT_SUCCESS) {
    ret = SPI_Com_Uninitialize();
  }

  if (ptr_spi_xfer_buf_rx_alloc != NULL) {
    free(ptr_spi_xfer_buf_rx_alloc);
    ptr_spi_xfer_buf_rx       = NULL;
    ptr_spi_xfer_buf_rx_alloc = NULL;
  }
  if (ptr_spi_xfer_buf_tx_alloc != NULL) {
    free(ptr_spi_xfer_buf_tx_alloc);
    ptr_spi_xfer_buf_tx       = NULL;
    ptr_spi_xfer_buf_tx_alloc = NULL;
  }

#ifdef DEBUG
  if (ret != EXIT_SUCCESS) {
    printf("Server Stop failed!\r\n");
  }
#endif

  return ret;
}


// Local functions

/**
  \fn            static void SPI_Server_Thread (void *argument)
  \brief         SPI Server thread function.
  \detail        This is a thread function that waits to receive a command from SPI Client 
                 (Driver Validation suite), and after command is received it is executed 
                 and the process starts again by waiting to receive next command.
  \param[in]     argument       Not used
  \return        none
*/
static void SPI_Server_Thread (void *argument) {
  uint8_t i;

  (void)argument;

  for (;;) {
    switch (spi_server_state) {

      case SPI_SERVER_STATE_RECEPTION:  // Receive a command
        if (SPI_Com_Receive(spi_cmd_buf_rx, BYTES_TO_ITEMS(sizeof(spi_cmd_buf_rx),SPI_SERVER_DATA_BITS), osWaitForever) == EXIT_SUCCESS) {
          spi_server_state = SPI_SERVER_STATE_EXECUTION;
        }
        // If 32 byte command was not received restart the reception of 32 byte command
        break;

      case SPI_SERVER_STATE_EXECUTION:  // Execute a command
        // Find the command and call handling function
        for (i = 0U; i < (sizeof(spi_cmd_desc) / sizeof(SPI_CMD_DESC_t)); i++) {
          if (memcmp(spi_cmd_buf_rx, spi_cmd_desc[i].command, strlen(spi_cmd_desc[i].command)) == 0) {
            (void)spi_cmd_desc[i].Command_Func((const char *)spi_cmd_buf_rx);
            break;
          }
        }
#ifdef DEBUG
        printf("%.20s\r\n", spi_cmd_buf_rx);
#endif
        spi_server_state = SPI_SERVER_STATE_RECEPTION;
        break;

      case SPI_SERVER_STATE_TERMINATE:  // Self-terminate the thread
      default:                          // Should never happen, processed as terminate request
#ifdef DEBUG
        printf("Server stopped!\r\n");
#endif
        (void)SPI_Com_Abort();
        (void)osThreadTerminate(osThreadGetId());
        break;
    }
  }
}

/**
  \fn            static void SPI_Com_Event (uint32_t event)
  \brief         SPI communication event callback (called from SPI driver from IRQ context).
  \detail        This function dispatches event (flag) to SPI Server thread.
  \param[in]     event       SPI event
                   - ARM_SPI_EVENT_TRANSFER_COMPLETE: Data Transfer completed
                   - ARM_SPI_EVENT_DATA_LOST:         Data lost: Receive overflow / Transmit underflow
                   - ARM_SPI_EVENT_MODE_FAULT:        Master Mode Fault (SS deactivated when Master)
  \return        none
*/
static void SPI_Com_Event (uint32_t event) {

  if (spi_server_thread_id != NULL) {
    (void)osThreadFlagsSet(spi_server_thread_id, event);
  }
}

/**
  \fn            static int32_t SPI_Com_Initialize (void)
  \brief         Initialize SPI interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Com_Initialize (void) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvSPI->Initialize(SPI_Com_Event) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Com_Uninitialize (void)
  \brief         Uninitialize SPI interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Com_Uninitialize (void) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvSPI->Uninitialize() == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Com_PowerOn (void)
  \brief         Power-up SPI interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Com_PowerOn (void) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvSPI->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Com_PowerOff (void)
  \brief         Power-down SPI interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Com_PowerOff (void) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvSPI->PowerControl(ARM_POWER_OFF) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Com_Configure (const SPI_COM_CONFIG_t *config)
  \brief         Configure SPI interface.
  \param[in]     config      Pointer to structure containing SPI interface configuration settings
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Com_Configure (const SPI_COM_CONFIG_t *config) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvSPI->Control(config->mode      |
                      config->format    |
                      config->bit_num   |
                      config->bit_order |
                      config->ss_mode   ,
                      config->bus_speed ) == ARM_DRIVER_OK) {
    spi_bytes_per_item = DATA_BITS_TO_BYTES((config->bit_num & ARM_SPI_DATA_BITS_Msk) >> ARM_SPI_DATA_BITS_Pos);
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static uint32_t SPI_Com_SS (void)
  \brief         Drive Slave Select line with Control function.
  \return        number of data items transferred
*/
static uint32_t SPI_Com_SS (uint32_t active) {
   int32_t ret;
  uint32_t arg;

  ret = EXIT_FAILURE;

  if (active != 0U) {
    arg = ARM_SPI_SS_ACTIVE;
  } else {
    arg = ARM_SPI_SS_INACTIVE;
  }

  if (drvSPI->Control(ARM_SPI_CONTROL_SS, arg) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Com_Receive (void *data_in, uint32_t num, uint32_t timeout)
  \brief         Receive data (command) over SPI interface.
  \param[out]    data_in     Pointer to memory where data will be received
  \param[in]     num         Number of data items to be received
  \param[in]     timeout     Timeout for reception (in ms)
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Com_Receive (void *data_in, uint32_t num, uint32_t timeout) {
   int32_t ret;
  uint32_t flags, tmo, time, cnt;

  ret = EXIT_FAILURE;

  if (spi_server_thread_id != NULL) {
    memset(data_in, (int32_t)'?', spi_bytes_per_item * num);
    vioSetSignal (vioLED0, vioLEDon);
    cnt  = 0;
    time = timeout;
    if (drvSPI->Receive(data_in, num) == ARM_DRIVER_OK) {
      while (time != 0U) {
        if (time > 100U) {
          tmo = 100U;
        } else {
          tmo = time;
        }
        flags = osThreadFlagsWait(SPI_EVENTS_MASK, osFlagsWaitAny, tmo);
        if (flags == osFlagsErrorTimeout) {     // If timeout
          if (time != osWaitForever) {
            time -= tmo;
          }
          if (drvSPI->GetDataCount() != 0U) {
            while (cnt != drvSPI->GetDataCount()) {   // While count is changing
              cnt = drvSPI->GetDataCount();
              flags = osThreadFlagsWait(SPI_EVENTS_MASK, osFlagsWaitAny, 10U);
              if (flags == osFlagsErrorTimeout) {     // If timeout
                if (time != osWaitForever) {
                  if (time > tmo) {
                    time -= tmo;
                  } else {
                    time = 0U;
                    break;
                  }
                }
              } else if ((flags & (0x80000000U | ARM_SPI_EVENT_TRANSFER_COMPLETE)) == ARM_SPI_EVENT_TRANSFER_COMPLETE) {
                // If completed event was signaled
                ret = EXIT_SUCCESS;
                break;
              } else {
                // In all other cases exit with failed status
                break;
              }
            }
            if (cnt != 0U) {
              // If something was received but not of expected size then terminate the reception
              break;
            }
          }
        } else if ((flags & (0x80000000U | ARM_SPI_EVENT_TRANSFER_COMPLETE)) == ARM_SPI_EVENT_TRANSFER_COMPLETE) {
          // If completed event was signaled
          ret = EXIT_SUCCESS;
          break;
        } else {
          // In all other cases exit with failed status
          break;
        }
      }
      if (ret != EXIT_SUCCESS) {
        // If receive was activated but failed to receive expected data then abort the transfer
        (void)drvSPI->Control(ARM_SPI_ABORT_TRANSFER, 0U);
      }
      vioSetSignal (vioLED0, vioLEDoff);
    }
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Com_Send (const void *data_out, uint32_t num, uint32_t timeout)
  \brief         Send data (response) over SPI interface.
  \param[out]    data_out       Pointer to memory containing data to be sent
  \param[in]     num            Number of data items to be sent
  \param[in]     timeout        Timeout for send (in ms)
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Com_Send (const void *data_out, uint32_t num, uint32_t timeout) {
  uint32_t flags;
   int32_t ret;

  ret = EXIT_FAILURE;

  if (spi_server_thread_id != NULL) {
    vioSetSignal (vioLED1, vioLEDon);
    if (drvSPI->Send(data_out, num) == ARM_DRIVER_OK) {
      flags = osThreadFlagsWait(SPI_EVENTS_MASK, osFlagsWaitAny, timeout);
      if ((flags & (0x80000000U | ARM_SPI_EVENT_TRANSFER_COMPLETE)) == ARM_SPI_EVENT_TRANSFER_COMPLETE) {
        // If completed event was signaled
        ret = EXIT_SUCCESS;
      }
      if (ret != EXIT_SUCCESS) {
        // If send was activated but failed to send all of the expected data then abort the transfer
        (void)drvSPI->Control(ARM_SPI_ABORT_TRANSFER, 0U);
      }
      vioSetSignal (vioLED1, vioLEDoff);
    }
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Com_Transfer (const void *data_out, void *data_in, uint32_t num, uint32_t timeout)
  \brief         Transfer (send/receive) data over SPI interface.
  \param[in]     data_out       Pointer to memory containing data to be sent
  \param[out]    data_in        Pointer to memory where data will be received
  \param[in]     num            Number of data items to be transferred
  \param[in]     timeout        Timeout for transfer (in ms)
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Com_Transfer (const void *data_out, void *data_in, uint32_t num, uint32_t timeout) {
  uint32_t flags;
   int32_t ret;

  ret = EXIT_FAILURE;

  if (spi_server_thread_id != NULL) {
    vioSetSignal (vioLED2, vioLEDon);
    if (drvSPI->Transfer(data_out, data_in, num) == ARM_DRIVER_OK) {
      flags = osThreadFlagsWait(SPI_EVENTS_MASK, osFlagsWaitAny, timeout);
      spi_xfer_cnt = drvSPI->GetDataCount();
      if ((flags & (0x80000000U | ARM_SPI_EVENT_TRANSFER_COMPLETE)) == ARM_SPI_EVENT_TRANSFER_COMPLETE) {
        // If completed event was signaled
        ret = EXIT_SUCCESS;
      } else {
        // If error or timeout
        (void)drvSPI->Control(ARM_SPI_ABORT_TRANSFER, 0U);
      }
      vioSetSignal (vioLED2, vioLEDoff);
    }
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Com_Abort (void)
  \brief         Abort current transfer on SPI interface.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Com_Abort (void) {
  int32_t ret;

  ret = EXIT_FAILURE;

  if (drvSPI->Control(ARM_SPI_ABORT_TRANSFER, 0U) == ARM_DRIVER_OK) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

/**
  \fn            static uint32_t SPI_Com_GetCnt (void)
  \brief         Get number of data items transferred over SPI interface in last transfer.
  \return        number of data items transferred
*/
static uint32_t SPI_Com_GetCnt (void) {
  return spi_xfer_cnt;
}


// Command handling functions

/**
  \fn            static int32_t SPI_Cmd_GetVer (const char *cmd)
  \brief         Handle command "GET VER".
  \detail        Return SPI Server version over SPI interface (16 bytes).
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Cmd_GetVer (const char *cmd) {

  (void)cmd;

  memset(spi_cmd_buf_tx, 0, 16);
  memcpy(spi_cmd_buf_tx, SPI_SERVER_VER, sizeof(SPI_SERVER_VER));

  return (SPI_Com_Send(spi_cmd_buf_tx, BYTES_TO_ITEMS(16U, SPI_SERVER_DATA_BITS), spi_cmd_timeout));
}

/**
  \fn            static int32_t SPI_Cmd_GetCap (const char *cmd)
  \brief         Handle command "GET CAP".
  \detail        Return SPI Server capabilities over SPI interface (32 bytes).
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Cmd_GetCap (const char *cmd) {
  int32_t  ret;
  uint32_t mode_mask, format_mask, data_bits_mask, bit_order_mask, bus_speed_min, bus_speed_max;
  uint32_t bs;
  uint8_t  i;

  (void)cmd;

  ret = EXIT_FAILURE;

  // Determine supported minimum bus speed
  // Find minimum speed setting at which Control function succeeds
  bs = 1000U;
  do {
    if (drvSPI->Control(ARM_SPI_MODE_MASTER                                                           | 
                      ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_MASTER_HW_OUTPUT                                                   , 
                        bs     ) == ARM_DRIVER_OK) {
      break;
    }
    if (drvSPI->Control(ARM_SPI_MODE_MASTER                                                           | 
                      ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_MASTER_HW_OUTPUT                                                   , 
                        bs * 2U) == ARM_DRIVER_OK) {
      bs *= 2U;
      break;
    }
    if (drvSPI->Control(ARM_SPI_MODE_MASTER                                                           | 
                      ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_MASTER_HW_OUTPUT                                                   , 
                        bs * 5U) == ARM_DRIVER_OK) {
      bs *= 5U;
      break;
    }
    if (drvSPI->Control(ARM_SPI_MODE_MASTER                                                           | 
                      ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_MASTER_HW_OUTPUT                                                   , 
                        bs * 10U) == ARM_DRIVER_OK) {
      bs *= 10U;
      break;
    }
    bs *= 10U;
  } while (bs < 1000000U);
  bus_speed_min = bs;

  // Determine supported maximum bus speed
  // Find maximum speed setting at which Control function succeeds
  bs = 100000000U;
  do {
    if (drvSPI->Control(ARM_SPI_MODE_MASTER                                                           | 
                      ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_MASTER_HW_OUTPUT                                                   , 
                        bs     ) == ARM_DRIVER_OK) {
      break;
    }
    if (drvSPI->Control(ARM_SPI_MODE_MASTER                                                           | 
                      ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_MASTER_HW_OUTPUT                                                   , 
                        bs / 2U) == ARM_DRIVER_OK) {
      bs /= 2U;
      break;
    }
    if (drvSPI->Control(ARM_SPI_MODE_MASTER                                                           | 
                      ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_MASTER_HW_OUTPUT                                                   , 
                        bs / 5U) == ARM_DRIVER_OK) {
      bs /= 5U;
      break;
    }
    if (drvSPI->Control(ARM_SPI_MODE_MASTER                                                           | 
                      ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_MASTER_HW_OUTPUT                                                   , 
                        bs / 10U) == ARM_DRIVER_OK) {
      bs /= 10U;
      break;
    }
    bs /= 10U;
  } while (bs > 1000000U);
  bus_speed_max = bs;

  // Determine supported modes
  mode_mask = 0U;
  if (drvSPI->Control(ARM_SPI_MODE_MASTER                                                           | 
                    ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                    ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                    ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                      ARM_SPI_SS_MASTER_HW_OUTPUT                                                   , 
                      bus_speed_min ) == ARM_DRIVER_OK) {
    mode_mask |= 1U;
  }

  if (drvSPI->Control(ARM_SPI_MODE_SLAVE                                                            | 
                    ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                    ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                    ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                      ARM_SPI_SS_SLAVE_HW                                                           , 
                      0U ) == ARM_DRIVER_OK) {
    mode_mask |= 1U << 1;
  }

  // Determine supported clock / frame format
  format_mask = 0U;
  if (drvSPI->Control(ARM_SPI_MODE_SLAVE                                                            | 
                      ARM_SPI_CPOL0_CPHA0                                                           | 
                    ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                    ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                      ARM_SPI_SS_SLAVE_HW                                                           , 
                      0U ) == ARM_DRIVER_OK) {
    format_mask |= 1U;
  }
  if (drvSPI->Control(ARM_SPI_MODE_SLAVE                                                            | 
                      ARM_SPI_CPOL0_CPHA1                                                           | 
                    ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                    ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                      ARM_SPI_SS_SLAVE_HW                                                           , 
                      0U ) == ARM_DRIVER_OK) {
    format_mask |= 1U << 1;
  }
  if (drvSPI->Control(ARM_SPI_MODE_SLAVE                                                            | 
                      ARM_SPI_CPOL1_CPHA0                                                           | 
                    ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                    ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                      ARM_SPI_SS_SLAVE_HW                                                           , 
                      0U ) == ARM_DRIVER_OK) {
    format_mask |= 1U << 2;
  }
  if (drvSPI->Control(ARM_SPI_MODE_SLAVE                                                            | 
                      ARM_SPI_CPOL1_CPHA1                                                           | 
                    ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                    ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                      ARM_SPI_SS_SLAVE_HW                                                           , 
                      0U ) == ARM_DRIVER_OK) {
    format_mask |= 1U << 3;
  }
  if (drvSPI->Control(ARM_SPI_MODE_SLAVE                                                            | 
                      ARM_SPI_TI_SSI                                                                | 
                    ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                    ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                      ARM_SPI_SS_SLAVE_HW                                                           , 
                      0U ) == ARM_DRIVER_OK) {
    format_mask |= 1U << 4;
  }
  if (drvSPI->Control(ARM_SPI_MODE_SLAVE                                                            | 
                      ARM_SPI_MICROWIRE                                                             | 
                    ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                    ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                      ARM_SPI_SS_SLAVE_HW                                                           , 
                      0U ) == ARM_DRIVER_OK) {
    format_mask |= 1U << 5;
  }

  // Determine supported data bits
  data_bits_mask = 0U;
  for (i = 1U; i <= 32U; i++) {
    if (drvSPI->Control(ARM_SPI_MODE_SLAVE                                                            | 
                      ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((i                    << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_SERVER_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)    & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_SLAVE_HW                                                           , 
                        0U ) == ARM_DRIVER_OK) {
      data_bits_mask |= 1UL << (i - 1U);
    }
  }

  // Determine bit order
  bit_order_mask = 0U;
  if (drvSPI->Control(ARM_SPI_MODE_SLAVE                                                            | 
                    ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                    ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ARM_SPI_MSB_LSB                                                               | 
                      ARM_SPI_SS_SLAVE_HW                                                           , 
                      0U ) == ARM_DRIVER_OK) {
    bit_order_mask |= 1U;
  }
  if (drvSPI->Control(ARM_SPI_MODE_SLAVE                                                            | 
                    ((SPI_SERVER_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos) & ARM_SPI_FRAME_FORMAT_Msk) | 
                    ((SPI_SERVER_DATA_BITS << ARM_SPI_DATA_BITS_Pos)    & ARM_SPI_DATA_BITS_Msk)    | 
                      ARM_SPI_LSB_MSB                                                               | 
                      ARM_SPI_SS_SLAVE_HW                                                           , 
                      0U ) == ARM_DRIVER_OK) {
    bit_order_mask |= 1U << 1;
  }

  // Revert communication settings to default because they were changed during auto-detection of capabilities
  (void)SPI_Com_Configure(&spi_com_config_default);

  memset(spi_cmd_buf_tx, 0, 32);
  if (snprintf((char *)spi_cmd_buf_tx, 32, "%02X,%02X,%08X,%02X,%i,%i", 
                mode_mask, 
                format_mask, 
                data_bits_mask, 
                bit_order_mask, 
               (bus_speed_min / 1000U), 
               (bus_speed_max / 1000U)) <= 32) {
    ret = SPI_Com_Send(spi_cmd_buf_tx, BYTES_TO_ITEMS(32U, SPI_SERVER_DATA_BITS), spi_cmd_timeout);
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Cmd_SetBuf (const char *cmd)
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
static int32_t SPI_Cmd_SetBuf (const char *cmd) {
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
    ptr_buf = ptr_spi_xfer_buf_rx;
  } else if (strstr(cmd, "TX") != NULL) {
    ptr_buf = ptr_spi_xfer_buf_tx;
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
        if (val <= spi_xfer_buf_size) {
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
    memset(ptr_buf, (int32_t)pattern, spi_xfer_buf_size);
  }

  if ((ret == EXIT_SUCCESS) && (ptr_buf != NULL) && (len != 0U)) {
    // Load 'len' bytes from start of buffer with content received in IN data phase
    ret = SPI_Com_Receive(ptr_buf, BYTES_TO_ITEMS(len, SPI_SERVER_DATA_BITS), spi_cmd_timeout);
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Cmd_GetBuf (const char *cmd)
  \brief         Handle command "GET BUF RX/TX,len".
  \detail        Send content of buffer over SPI interface.
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Cmd_GetBuf (const char *cmd) {
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
    ptr_buf = ptr_spi_xfer_buf_rx;
  } else if (strstr(cmd, "TX") != NULL) {
    ptr_buf = ptr_spi_xfer_buf_tx;
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
        if ((val > 0U) && (val <= spi_xfer_buf_size)) {
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

  if ((ret == EXIT_SUCCESS) && (ptr_buf != NULL) && (len != 0U)) {
    ret = SPI_Com_Send(ptr_buf, BYTES_TO_ITEMS(len, SPI_SERVER_DATA_BITS), spi_cmd_timeout);
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Cmd_SetCom (const char *cmd)
  \brief         Handle command "SET COM mode,format,bit_num,bit_order,ss_mode,bus_speed".
  \detail        Set communication configuration settings used for transfers (XFER commands).
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Cmd_SetCom (const char *cmd) {
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
      case 0U:                          // Master mode
        spi_com_config_xfer.mode = ARM_SPI_MODE_MASTER;
        break;
      case 1U:                          // Slave mode
        spi_com_config_xfer.mode = ARM_SPI_MODE_SLAVE;
        break;
      default:
        ret = EXIT_FAILURE;
        break;
    }
  } else {
    ret = EXIT_FAILURE;
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'format' (clock polarity/phase or frame format)
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        switch (val) {
          case 0:                       // Clock polarity 0, clock phase 0
            spi_com_config_xfer.format = ARM_SPI_CPOL0_CPHA0;
            break;
          case 1:                       // Clock polarity 0, clock phase 1
            spi_com_config_xfer.format = ARM_SPI_CPOL0_CPHA1;
            break;
          case 2:                       // Clock polarity 1, clock phase 0
            spi_com_config_xfer.format = ARM_SPI_CPOL1_CPHA0;
            break;
          case 3:                       // Clock polarity 1, clock phase 1
            spi_com_config_xfer.format = ARM_SPI_CPOL1_CPHA1;
            break;
          case 4:                       // Texas Instruments Frame Format
            spi_com_config_xfer.format = ARM_SPI_TI_SSI;
            break;
          case 5:                       // National Microwire Frame Format
            spi_com_config_xfer.format = ARM_SPI_MICROWIRE;
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
    // Parse 'bit_num'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if ((val > 0U) && (val <= 32U)) {
          spi_com_config_xfer.bit_num = ARM_SPI_DATA_BITS(val);
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'bit_order'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (val == 0U) {
          spi_com_config_xfer.bit_order = ARM_SPI_MSB_LSB;
        } else if (val == 1U) {
          spi_com_config_xfer.bit_order = ARM_SPI_LSB_MSB;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'ss_mode'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (spi_com_config_xfer.mode == ARM_SPI_MODE_MASTER) {
          // Slave Select modes for Master mode
          switch (val) {
            case 0:
              spi_com_config_xfer.ss_mode = ARM_SPI_SS_MASTER_UNUSED;
              break;
            case 1:
              spi_com_config_xfer.ss_mode = ARM_SPI_SS_MASTER_SW;
              break;
            default:
              ret = EXIT_FAILURE;
              break;
          }
        } else {
          // Slave Select modes for Slave mode
          switch (val) {
            case 0:
              spi_com_config_xfer.ss_mode = ARM_SPI_SS_SLAVE_SW;
              break;
            case 1:
              spi_com_config_xfer.ss_mode = ARM_SPI_SS_SLAVE_HW;
              break;
            default:
              ret = EXIT_FAILURE;
              break;
          }
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse 'bus_speed'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        spi_com_config_xfer.bus_speed = val;
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  return ret;
}

/**
  \fn            static int32_t SPI_Cmd_Xfer (const char *cmd)
  \brief         Handle command "XFER num[,delay_c][,delay_t][,timeout]".
  \detail        Send data from SPI TX buffer and receive data to SPI RX buffer 
                 (buffers must be set with "SET BUF" command before this command).
                 Control function is delayed by optional parameter 'delay_c' in milliseconds.
                 Transfer function is delayed by optional parameter 'delay_t' in milliseconds, 
                 starting after delay specified with 'delay_c' parameter.
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Cmd_Xfer (const char *cmd) {
  const char    *ptr_str;
        uint32_t val, num, delay_c, delay_t, start_tick, curr_tick;
         int32_t ret;

  ret             = EXIT_SUCCESS;
  val             = 0U;
  num             = 0U;
  delay_c         = 0U;
  delay_t         = 0U;

  ptr_str = &cmd[4];                    // Skip "XFER"
  while (*ptr_str == ' ') {             // Skip whitespaces
    ptr_str++;
  }

  // Parse 'num'
  if (sscanf(ptr_str, "%u", &val) == 1) {
    if ((val > 0U) && (val <= spi_xfer_buf_size)) {
      num = val;
    } else {
      ret = EXIT_FAILURE;
    }
  } else {
    ret = EXIT_FAILURE;
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse optional 'delay_c'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (val != osWaitForever) {
          delay_c = val;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse optional 'delay_t'
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {              // If ',' was found
      ptr_str++;                        // Skip ','
      while (*ptr_str == ' ') {         // Skip whitespaces after ','
        ptr_str++;
      }
      if (sscanf(ptr_str, "%u", &val) == 1) {
        if (val != osWaitForever) {
          delay_t = val;
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
          spi_xfer_timeout = val;
        } else {
          ret = EXIT_FAILURE;
        }
      } else {
        ret = EXIT_FAILURE;
      }
    }
  }

  start_tick = osKernelGetTickCount();

  if (ret == EXIT_SUCCESS) {
    // Deactivate SPI
    ret = SPI_Com_Configure(&spi_com_config_inactive);
  }

  if ((ret == EXIT_SUCCESS) && (delay_c != 0U)) {
    // Delay before Control function is called
    (void)osDelay(delay_c);
  }

  if (ret == EXIT_SUCCESS) {
    // Configure communication settings before transfer
    ret = SPI_Com_Configure(&spi_com_config_xfer);
  }

  if ((ret == EXIT_SUCCESS) && (delay_t != 0U)) {
    // Delay before Transfer function is called
    (void)osDelay(delay_t);
  }

  if ((ret == EXIT_SUCCESS) && 
     ((spi_com_config_xfer.mode    == ARM_SPI_MODE_SLAVE)     && 
      (spi_com_config_xfer.ss_mode == ARM_SPI_SS_SLAVE_SW))   ||
     ((spi_com_config_xfer.mode    == ARM_SPI_MODE_MASTER)    &&
      (spi_com_config_xfer.ss_mode == ARM_SPI_SS_MASTER_SW)))  {
    ret = SPI_Com_SS(1U);
  }

  if (ret == EXIT_SUCCESS) {
    // Transfer data
    ret = SPI_Com_Transfer(ptr_spi_xfer_buf_tx, ptr_spi_xfer_buf_rx, num, spi_xfer_timeout);
  }

  if ((ret == EXIT_SUCCESS) && 
     ((spi_com_config_xfer.mode    == ARM_SPI_MODE_SLAVE)     && 
      (spi_com_config_xfer.ss_mode == ARM_SPI_SS_SLAVE_SW))   ||
     ((spi_com_config_xfer.mode == ARM_SPI_MODE_MASTER)       &&
      (spi_com_config_xfer.ss_mode == ARM_SPI_SS_MASTER_SW)))  {
    ret = SPI_Com_SS(0U);
  }

  // Deactivate SPI
  (void)SPI_Com_Configure(&spi_com_config_inactive);

  // Wait until timeout expires
  curr_tick = osKernelGetTickCount();
  if ((curr_tick - start_tick) < spi_xfer_timeout) {
    (void)osDelay(spi_xfer_timeout - (curr_tick - start_tick));
  }

  // Wait additional 10 ms to insure that Client has deactivated
  (void)osDelay(10U);

  // Revert communication settings to default
  (void)SPI_Com_Configure(&spi_com_config_default);

  return ret;
}

/**
  \fn            static int32_t SPI_Cmd_GetCnt (const char *cmd)
  \brief         Handle command "GET CNT".
  \detail        Return number of items transferred (sent/received) in last transfer 
                 (requested by last XFER command).
  \param[in]     cmd            Pointer to null-terminated command string
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t SPI_Cmd_GetCnt (const char *cmd) {
  int32_t ret;

  (void)cmd;

  ret = EXIT_FAILURE;

  memset(spi_cmd_buf_tx, 0, 16);
  if (snprintf((char *)spi_cmd_buf_tx, 16, "%u", SPI_Com_GetCnt()) < 16) {
    ret = SPI_Com_Send(spi_cmd_buf_tx, BYTES_TO_ITEMS(16U, SPI_SERVER_DATA_BITS), spi_cmd_timeout);
  }

  return ret;
}
