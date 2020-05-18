/*
 * Copyright (c) 2015-2020 Arm Limited. All rights reserved.
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
 * Project:     CMSIS-Driver Validation
 * Title:       Serial Peripheral Interface Bus (SPI) Driver Validation tests
 *
 * -----------------------------------------------------------------------------
 */

#ifndef __DOXYGEN__                     // Exclude form the documentation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmsis_dv.h"
#include "DV_SPI_Config.h"
#include "DV_Framework.h"

#include "Driver_SPI.h"

#define CMD_LEN                   32UL  // Length of command to SPI Server
#define RESP_GET_VER_LEN          16UL  // Length of response from SPI Server to GET VER command
#define RESP_GET_CAP_LEN          32UL  // Length of response from SPI Server to GET CAP command
#define RESP_GET_CNT_LEN          16UL  // Length of response from SPI Server to GET CNT command

#define OP_SEND                   0UL   // Send operation
#define OP_RECEIVE                1UL   // Receive operation
#define OP_TRANSFER               2UL   // Transfer operation
#define OP_ABORT_SEND             3UL   // Abort send operation
#define OP_ABORT_RECEIVE          4UL   // Abort receive operation
#define OP_ABORT_TRANSFER         5UL   // Abort transfer operation

#define MODE_INACTIVE             0UL   // Inactive mode
#define MODE_MASTER               1UL   // Master mode
#define MODE_SLAVE                2UL   // Slave  mode
#define SS_MODE_MASTER_UNUSED     0UL   // Master mode Slave Select unused
#define SS_MODE_MASTER_SW         1UL   // Master mode Slave Select software controlled
#define SS_MODE_MASTER_HW_OUTPUT  2UL   // Master mode Slave Select hardware controlled output
#define SS_MODE_MASTER_HW_INPUT   3UL   // Master mode Slave Select hardware monitored input
#define SS_MODE_SLAVE_HW          0UL   // Slave mode Slave Select hardware monitored
#define SS_MODE_SLAVE_SW          1UL   // Slave mode Slave Select software controlled
#define FORMAT_CPOL0_CPHA0        0UL   // Clock Format: Polarity 0, Phase 0
#define FORMAT_CPOL0_CPHA1        1UL   // Clock Format: Polarity 0, Phase 1
#define FORMAT_CPOL1_CPHA0        2UL   // Clock Format: Polarity 1, Phase 0
#define FORMAT_CPOL1_CPHA1        3UL   // Clock Format: Polarity 1, Phase 1
#define FORMAT_TI                 4UL   // Frame Format: Texas Instruments
#define FORMAT_MICROWIRE          5UL   // Frame Format: National Semiconductor Microwire
#define BO_MSB_TO_LSB             0UL   // Bit Order MSB to LSB
#define BO_LSB_TO_MSB             1UL   // Bit Order LSB to MSB

// Testing Configuration definitions
#if    (SPI_CFG_TEST_MODE != 0)
#define SPI_SERVER_USED                 1
#else
#define SPI_SERVER_USED                 0
#endif

// Determine maximum number of items used for testing
#if    (SPI_CFG_DEF_NUM == 0)
#error  Default number of items to test must not be 0!
#endif

#define SPI_NUM_MAX                     SPI_CFG_DEF_NUM
#if    (SPI_CFG_NUM1 > SPI_NUM_MAX)
#undef  SPI_NUM_MAX
#define SPI_NUM_MAX                     SPI_CFG_NUM1
#endif
#if    (SPI_CFG_NUM2 > SPI_NUM_MAX)
#undef  SPI_NUM_MAX
#define SPI_NUM_MAX                     SPI_CFG_NUM2
#endif
#if    (SPI_CFG_NUM3 > SPI_NUM_MAX)
#undef  SPI_NUM_MAX
#define SPI_NUM_MAX                     SPI_CFG_NUM3
#endif
#if    (SPI_CFG_NUM4 > SPI_NUM_MAX)
#undef  SPI_NUM_MAX
#define SPI_NUM_MAX                     SPI_CFG_NUM4
#endif
#if    (SPI_CFG_NUM5 > SPI_NUM_MAX)
#undef  SPI_NUM_MAX
#define SPI_NUM_MAX                     SPI_CFG_NUM5
#endif

// Calculate maximum required buffer size
#if   ((SPI_CFG_DEF_DATA_BITS > 16) || ((SPI_TC_DATA_BIT_EN_MASK & 0xFFFF0000UL) != 0U))
#define SPI_BUF_MAX                    (SPI_NUM_MAX * 4U)
#elif ((SPI_CFG_DEF_DATA_BITS > 8)  || ((SPI_TC_DATA_BIT_EN_MASK & 0x0000FF00UL) != 0U))
#define SPI_BUF_MAX                    (SPI_NUM_MAX * 2U)
#else
#define SPI_BUF_MAX                    (SPI_NUM_MAX)
#endif
#if    (SPI_SERVER_USED == 1)
// If selected Test Mode is SPI Server take into account SPI Server data bit settings
#if   ((SPI_CFG_SRV_DATA_BITS > 16) && (SPI_BUF_MAX < (SPI_NUM_MAX * 4U)))
#undef  SPI_BUF_MAX
#define SPI_BUF_MAX                    (SPI_NUM_MAX * 4U)
#elif ((SPI_CFG_SRV_DATA_BITS > 8)  && (SPI_BUF_MAX < (SPI_NUM_MAX * 2U)))
#undef  SPI_BUF_MAX
#define SPI_BUF_MAX                    (SPI_NUM_MAX * 2U)
#endif
#endif


typedef struct {                // SPI Server version structure
  uint8_t  major;               // Version major number
  uint8_t  minor;               // Version minor number
  uint16_t patch;               // Version patch (revision) number
} SPI_SERV_VER_t;

typedef struct {                // SPI Server capabilities structure
  uint32_t mode_mask;           // Mode and Slave Select mask
  uint32_t fmt_mask;            // Clock Format or Frame Format mask
  uint32_t db_mask;             // Data Bits mask
  uint32_t bo_mask;             // Bit Order mask
  uint32_t bs_min;              // Min bus speed
  uint32_t bs_max;              // Max bus speed
} SPI_SERV_CAP_t;

// Register Driver_SPI#
#define _ARM_Driver_SPI_(n)         Driver_SPI##n
#define  ARM_Driver_SPI_(n)    _ARM_Driver_SPI_(n)
extern   ARM_DRIVER_SPI         ARM_Driver_SPI_(DRV_SPI);
static   ARM_DRIVER_SPI *drv = &ARM_Driver_SPI_(DRV_SPI);

// Global variables (used in this module only)
static int8_t                   buffers_ok;
static int8_t                   server_ok;
static int8_t                   driver_ok;

static SPI_SERV_CAP_t           spi_serv_cap;

static volatile uint32_t        event;
static volatile uint32_t        duration;
static uint32_t                 systick_freq;

static char                     msg_buf     [256];

// Allocated buffer pointers
static void                    *ptr_tx_buf_alloc;
static void                    *ptr_rx_buf_alloc;
static void                    *ptr_cmp_buf_alloc;

// Buffer pointers used for data transfers (must be aligned to 4 byte)
static uint8_t                 *ptr_tx_buf;
static uint8_t                 *ptr_rx_buf;
static uint8_t                 *ptr_cmp_buf;

// String representation of Driver codes
static const char *str_oper[] = {
  "Send    ",
  "Receive ",
  "Transfer",
  "Abort Send    ",
  "Abort Receive ",
  "Abort Transfer"
};

static const char *str_ret[] = {
  "ARM_DRIVER_OK",
  "ARM_DRIVER_ERROR",
  "ARM_DRIVER_ERROR_BUSY",
  "ARM_DRIVER_ERROR_TIMEOUT",
  "ARM_DRIVER_ERROR_UNSUPPORTED",
  "ARM_DRIVER_ERROR_PARAMETER",
  "ARM_DRIVER_ERROR_SPECIFIC"
  "ARM_SPI_ERROR_MODE",
  "ARM_SPI_ERROR_FRAME_FORMAT",
  "ARM_SPI_ERROR_DATA_BITS",
  "ARM_SPI_ERROR_BIT_ORDER",
  "ARM_SPI_ERROR_SS_MODE"
};

// Local functions
#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected
static int32_t  ComConfigDefault       (void);
static int32_t  ComSendCommand         (const void *data_out, uint32_t len);
static int32_t  ComReceiveResponse     (      void *data_in,  uint32_t len);

static int32_t  CmdGetVer              (void);
static int32_t  CmdGetCap              (void);
static int32_t  CmdSetBufTx            (char pattern);
static int32_t  CmdSetBufRx            (char pattern);
static int32_t  CmdGetBufRx            (uint32_t len);
static int32_t  CmdSetCom              (uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t ss_mode, uint32_t bus_speed);
static int32_t  CmdXfer                (uint32_t num,  uint32_t delay,  uint32_t timeout,   uint32_t num_ss);
static int32_t  CheckServer            (void);
#endif

static uint32_t DataBitsToBytes        (uint32_t data_bits);
static int32_t  InitDriver             (void);
static int32_t  CheckBuffers           (void);

static void SPI_DataExchange_Operation (uint32_t operation, uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t ss_mode, uint32_t bus_speed, uint32_t num);

// Helper functions

/*
  \fn            void SPI_DrvEvent (uint32_t evt)
  \brief         Store event into global variable.
  \detail        This is a callback function called by the driver upon an event.
  \param[in]     evt            SPI event
  \return        none
*/
static void SPI_DrvEvent (uint32_t evt) {
  event |= evt;
}

/*
  \fn            static uint32_t DataBitsToBytes (uint32_t data_bits)
  \brief         Calculate number of bytes used for an item at required data bits.
  \return        number of bytes per item
*/
static uint32_t DataBitsToBytes (uint32_t data_bits) {
  uint32_t ret;

  if        (data_bits > 16U) {
    ret = 4U;
  } else if (data_bits > 8U) {
    ret = 2U;
  } else {
    ret = 1U;
  }

  return ret;
}

/*
  \fn            static int32_t InitDriver (void)
  \brief         Initialize and power-on the driver.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t InitDriver (void) {

  if (driver_ok == -1) {                // If -1, means it was not yet checked
    driver_ok = 0;
    if (drv->Initialize  (SPI_DrvEvent)     == ARM_DRIVER_OK) {
      if (drv->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK) {
        driver_ok = 1;
      }
    }
  }

  if (driver_ok == 1) {
    return EXIT_SUCCESS;
  }

  TEST_FAIL_MESSAGE("[FAILED] SPI driver initialization or power-up failed. Test aborted!");
  return EXIT_FAILURE;
}

/*
  \fn            static int32_t CheckBuffers (void)
  \brief         Check if buffers are valid.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CheckBuffers (void) {

  if (buffers_ok == -1) {               // If -1, means it was not yet checked
    if ((ptr_tx_buf  != NULL) &&
        (ptr_rx_buf  != NULL) && 
        (ptr_cmp_buf != NULL)) {
      buffers_ok = 1;
    } else {
      buffers_ok = 0;
    }
  }

  if (buffers_ok == 1) {
    return EXIT_SUCCESS;
  }

  TEST_FAIL_MESSAGE("[FAILED] Invalid data buffers! Increase heap memory! Test aborted!");
  return EXIT_FAILURE;
}

#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected

/*
  \fn            static int32_t ComConfigDefault (void)
  \brief         Configure SPI Communication Interface to SPI Server default communication configuration.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t ComConfigDefault (void) {
  int32_t ret;

  ret = EXIT_SUCCESS;

  if (drv->Control(ARM_SPI_MODE_MASTER                                                                | 
                 ((SPI_CFG_SRV_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos)   & ARM_SPI_FRAME_FORMAT_Msk)   | 
                 ((SPI_CFG_SRV_DATA_BITS << ARM_SPI_DATA_BITS_Pos)      & ARM_SPI_DATA_BITS_Msk)      | 
                 ((SPI_CFG_SRV_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)      & ARM_SPI_BIT_ORDER_Msk)      | 
                 ((SPI_CFG_SRV_SS_MODE   << ARM_SPI_SS_MASTER_MODE_Pos) & ARM_SPI_SS_MASTER_MODE_Msk) , 
                   SPI_CFG_SRV_BUS_SPEED) != ARM_DRIVER_OK) {
    ret = EXIT_FAILURE;
  }

  if ((ret == EXIT_SUCCESS) && (SPI_CFG_SRV_SS_MODE == SS_MODE_MASTER_SW)) {
    if (drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE) != ARM_DRIVER_OK) {
      ret = EXIT_FAILURE;
    }
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("Failed to configure communication interface to SPI Server default settings. Testing aborted!");
  }

  // Give SPI Server 10 ms to prepare for reception of the command
  (void)osDelay(10U);

  return ret;
}

/**
  \fn            static int32_t ComSendCommand (const void *data_out, uint32_t num)
  \brief         Send command to SPI Server.
  \param[out]    data_out       Pointer to memory containing data to be sent
  \param[in]     len            Number of bytes to be sent
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t ComSendCommand (const void *data_out, uint32_t len) {
   int32_t ret;
  uint32_t num, tout;

  ret = EXIT_SUCCESS;
  num = (len + DataBitsToBytes(SPI_CFG_SRV_DATA_BITS) - 1U) / DataBitsToBytes(SPI_CFG_SRV_DATA_BITS);

  if (SPI_CFG_SRV_SS_MODE == SS_MODE_MASTER_SW) {
    if (drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE) != ARM_DRIVER_OK) {
      ret = EXIT_FAILURE;
    }
  }
  if (ret == EXIT_SUCCESS) {
    if (drv->Send(data_out, num) == ARM_DRIVER_OK) {
      for (tout = SPI_CFG_SRV_CMD_TOUT; tout != 0U; tout--) {
        if ((drv->GetDataCount() == num) && (drv->GetStatus().busy == 0U)) { 
          break;
        }
        (void)osDelay(1U);
      }
      if (tout == 0U) {                 // If send has timed out
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if (SPI_CFG_SRV_SS_MODE == SS_MODE_MASTER_SW) {
    if (drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE) != ARM_DRIVER_OK) {
      ret = EXIT_FAILURE;
    }
  }

  // Give SPI Server 10 ms to prepare for reception of the next command or to 
  // prepare the answer to this command, if it requires response
  (void)osDelay(10U);

  return ret;
}

/**
  \fn            static int32_t ComReceiveResponse (void *data_in, uint32_t num)
  \brief         Receive response from SPI Server.
  \param[out]    data_in     Pointer to memory where data will be received
  \param[in]     len         Number of data bytes to be received
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t ComReceiveResponse (void *data_in, uint32_t len) {
   int32_t ret;
  uint32_t num, tout;

  ret = EXIT_SUCCESS;
  num = (len + DataBitsToBytes(SPI_CFG_SRV_DATA_BITS) - 1U) / DataBitsToBytes(SPI_CFG_SRV_DATA_BITS);

  if (SPI_CFG_SRV_SS_MODE == SS_MODE_MASTER_SW) {
    if (drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE) != ARM_DRIVER_OK) {
      ret = EXIT_FAILURE;
    }
  }
  if (ret == EXIT_SUCCESS) {
    if (drv->Receive(data_in, num) == ARM_DRIVER_OK) {
      for (tout = SPI_CFG_SRV_CMD_TOUT; tout != 0U; tout--) {
        if ((drv->GetDataCount() == num) && (drv->GetStatus().busy == 0U)) { 
          break;
        }
        (void)osDelay(1U);
      }
      if (tout == 0U) {                 // If send has timed out
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if (SPI_CFG_SRV_SS_MODE == SS_MODE_MASTER_SW) {
    if (drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE) != ARM_DRIVER_OK) {
      ret = EXIT_FAILURE;
    }
  }

  // Give SPI Server 10 ms to prepare for reception of the next command
  (void)osDelay(10U);

  return ret;
}

/**
  \fn            static int32_t CmdGetVer (void)
  \brief         Get version from SPI Server and check that it is valid.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CmdGetVer (void) {
  int32_t        ret;
  SPI_SERV_VER_t spi_serv_ver;
  const char    *ptr_str;
  uint16_t       val16;
  uint8_t        val8;

  ptr_str = NULL;

  memset(&spi_serv_ver, 0, sizeof(spi_serv_ver));

  // Send "GET VER" command to SPI Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  memcpy(ptr_tx_buf, "GET VER", 7);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret == EXIT_SUCCESS) {
    // Receive response to "GET VER" command from SPI Server
    memset(ptr_rx_buf, (int32_t)'?', RESP_GET_VER_LEN);
    ret = ComReceiveResponse(ptr_rx_buf, RESP_GET_VER_LEN);
  }

  // Parse version
  if (ret == EXIT_SUCCESS) {
    // Parse major
    ptr_str = (const char *)ptr_rx_buf;
    if (sscanf(ptr_str, "%hhx", &val8) == 1) {
      spi_serv_ver.major = val8;
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse minor
    ptr_str = strstr(ptr_str, ".");     // Find '.'
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip '.'
      if (sscanf(ptr_str, "%hhx", &val8) == 1) {
        spi_serv_ver.minor = val8;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse patch (revision)
    ptr_str = strstr(ptr_str, ".");     // Find next '.'
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip '.'
      if (sscanf(ptr_str, "%hx", &val16) == 1) {
        spi_serv_ver.patch = val16;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }

  if (ret == EXIT_SUCCESS) {
    // Only supported version is v1.0.0
    if ((spi_serv_ver.major != 1U) || 
        (spi_serv_ver.minor != 0U) || 
        (spi_serv_ver.patch != 0U)) { 
      ret = EXIT_FAILURE;
    }
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Get version from SPI Server.");
  }

  return ret;
}

/**
  \fn            static int32_t CmdGetCap (void)
  \brief         Get capabilities from SPI Server.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CmdGetCap (void) {
  int32_t     ret;
  const char *ptr_str;
  uint32_t    val32;
  uint8_t     val8;

  ptr_str = NULL;

  memset(&spi_serv_cap, 0, sizeof(spi_serv_cap));

  // Send "GET CAP" command to SPI Server
  memset(ptr_tx_buf, 0, RESP_GET_CAP_LEN);
  memcpy(ptr_tx_buf, "GET CAP", 7);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret == EXIT_SUCCESS) {
    (void)osDelay(10U);                 // Give SPI Server 10 ms to auto-detect capabilities

    // Receive response to "GET CAP" command from SPI Server
    memset(ptr_rx_buf, (int32_t)'?', RESP_GET_CAP_LEN);
    ret = ComReceiveResponse(ptr_rx_buf, RESP_GET_CAP_LEN);
  }

  // Parse capabilities
  if (ret == EXIT_SUCCESS) {
    // Parse mode mask
    ptr_str = (const char *)ptr_rx_buf;
    if (sscanf(ptr_str, "%hhx", &val8) == 1) {
      spi_serv_cap.mode_mask = val8;
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse format mask
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%hhx", &val8) == 1) {
        spi_serv_cap.fmt_mask = (uint32_t)val8;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse data bit mask
    ptr_str = strstr(ptr_str, ",");     // Find next ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%x", &val32) == 1) {
        spi_serv_cap.db_mask = val32;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse bit order mask
    ptr_str = strstr(ptr_str, ",");     // Find next ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%hhx", &val8) == 1) {
        spi_serv_cap.bo_mask = (uint32_t)val8;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse minimum bus speed
    ptr_str = strstr(ptr_str, ",");     // Find next ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%u", &val32) == 1) {
        spi_serv_cap.bs_min = val32 * 1000U;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse maximum bus speed
    ptr_str = strstr(ptr_str, ",");     // Find next ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%u", &val32) == 1) {
        spi_serv_cap.bs_max = val32 * 1000U;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Get capabilities from SPI Server.");
  }

  return ret;
}

/**
  \fn            static int32_t CmdSetBufTx (char pattern)
  \brief         Set Tx buffer of SPI Server to pattern.
  \param[in]     pattern        Pattern to fill the buffer with
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CmdSetBufTx (char pattern) {
  int32_t ret;

  // Send "SET BUF TX" command to SPI Server
  memset(ptr_tx_buf, 0, 32);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "SET BUF TX,0,%02X", (int32_t)pattern);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set Tx buffer on SPI Server.");
  }

  return ret;
}

/**
  \fn            static int32_t CmdSetBufRx (char pattern)
  \brief         Set Rx buffer of SPI Server to pattern.
  \param[in]     pattern        Pattern to fill the buffer with
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CmdSetBufRx (char pattern) {
  int32_t ret;

  // Send "SET BUF RX" command to SPI Server
  memset(ptr_tx_buf, 0, 32);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "SET BUF RX,0,%02X", (int32_t)pattern);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set Rx buffer on SPI Server.");
  }

  return ret;
}

/**
  \fn            static int32_t CmdGetBufRx (void)
  \brief         Get Rx buffer from SPI Server (into global array pointed to by ptr_rx_buf).
  \param[in]     len            Number of bytes to read from Rx buffer
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CmdGetBufRx (uint32_t len) {
  int32_t ret;

  // Send "GET BUF RX" command to SPI Server
  memset(ptr_tx_buf, 0, 32);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "GET BUF RX,%i", len);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret == EXIT_SUCCESS) {
    // Receive response to "GET BUF RX" command from SPI Server
    memset(ptr_rx_buf, (int32_t)'U', len);
    ret = ComReceiveResponse(ptr_rx_buf, len);
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Get Rx buffer from SPI Server.");
  }

  return ret;
}

/**
  \fn            static int32_t CmdSetCom (uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t ss_mode, uint32_t bus_speed)
  \brief         Set communication parameters on SPI Server for next XFER command.
  \param[in]     mode           mode (0 = Master, 1 = slave)
  \param[in]     format         clock / frame format (0 = clock polarity 0, phase 0 .. 5 = Microwire)
  \param[in]     data_bits      data bits (1..32)
  \param[in]     bit_order      bit order (0 = MSB to LSB, 1 = LSB to MSB)
  \param[in]     ss_mode        Slave Select mode 
                                (0 = not used, 1 = used (in Master mode driven, in Slave mode monitored as hw input))
  \param[in]     bus_speed      bus speed in bits per second (bps)
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CmdSetCom (uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t ss_mode, uint32_t bus_speed) {
  int32_t ret, stat;

  // Send "SET COM" command to SPI Server
  memset(ptr_tx_buf, 0, 32);
  stat = snprintf((char *)ptr_tx_buf, CMD_LEN, "SET COM %i,%i,%i,%i,%i,%i", mode, format, data_bits, bit_order, ss_mode, bus_speed);
  if ((stat > 0) && (stat < CMD_LEN)) {
    ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
  } else {
    ret = EXIT_FAILURE;
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set communication settings on SPI Server.");
  }

  return ret;
}

/**
  \fn            static int32_t CmdXfer (uint32_t num, uint32_t delay, uint32_t timeout, uint32_t num_ss)
  \brief         Activate transfer on SPI Server.
  \param[in]     num            number of items (according CMSIS SPI driver specification)
  \param[in]     delay          initial delay, in milliseconds, before starting requested operation 
                                (0xFFFFFFFF = delay not used)
  \param[in]     timeout        timeout in milliseconds, after delay, if delay is specified
  \param[in]     num_ss         number of items after which Slave Select line should be activated
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CmdXfer (uint32_t num, uint32_t delay, uint32_t timeout, uint32_t num_ss) {
  int32_t ret;

  // Send "XFER" command to SPI Server
  memset(ptr_tx_buf, 0, 32);
  if (num_ss != 0U) {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i,%i,%i,%i", num, delay, timeout, num_ss);
  } else if ((delay != osWaitForever) && (timeout != 0U)) {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i,%i,%i",    num, delay, timeout);
  } else if  (delay != osWaitForever) {                          
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i,%i",       num, delay);
  } else {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i",          num);
  }
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Activate transfer on SPI Server.");
  }

  return ret;
}

/*
  \fn            static int32_t CheckServer (void)
  \brief         Check if communication with SPI Server is working and get capabilities.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CheckServer (void) {

  if (server_ok == -1) {                // If -1, means it was not yet checked
    server_ok = 1;
    if (ComConfigDefault() != EXIT_SUCCESS) {
      server_ok = 0;
    }
    if (server_ok == 1) {
      if (CmdGetVer() != EXIT_SUCCESS) {
        server_ok = 0;
      }
    }
    if (server_ok == 1) {
      if (CmdGetCap() != EXIT_SUCCESS) {
        server_ok = 0;
      }
    }
    if (server_ok == 1) {
      // Check if all default settings are supported by SPI Server
      if ((spi_serv_cap.fmt_mask & (1UL << SPI_CFG_SRV_FORMAT)) == 0U) {
        // If SPI Server does not support default clock / frame format
        TEST_MESSAGE("[FAILED] Default clock / frame format setting in not supported by SPI Server!");
        server_ok = 0;
      }
      if ((spi_serv_cap.db_mask & (1UL << (SPI_CFG_SRV_DATA_BITS - 1U))) == 0U) {
        // If SPI Server does not support default data bits
        TEST_MESSAGE("[FAILED] Default data bits setting in not supported by SPI Server!");
        server_ok = 0;
      }
      if ((spi_serv_cap.bo_mask & (1UL << SPI_CFG_SRV_BIT_ORDER)) == 0U) {
        // If SPI Server does not support default bit order
        TEST_MESSAGE("[FAILED] Default bit order setting in not supported by SPI Server!");
        server_ok = 0;
      }
      if ((spi_serv_cap.bs_min > SPI_CFG_SRV_BUS_SPEED) ||
          (spi_serv_cap.bs_max < SPI_CFG_SRV_BUS_SPEED)) {
        // If SPI Server does not support default bus speed
        TEST_MESSAGE("[FAILED] Default bus speed setting in not supported by SPI Server!");
        server_ok = 0;
      }
    }
  }

  if (server_ok == 1) {
    return EXIT_SUCCESS;
  }

  TEST_FAIL_MESSAGE("[FAILED] Communication with SPI Server is not working. Test aborted!");
  return EXIT_FAILURE;
}

#endif                                  // If Test Mode SPI Server is selected

/*
  \fn            void SPI_DV_Initialize (void)
  \brief         Initialize testing environment for SPI testing.
  \detail        This function is called by the driver validation framework before SPI testing begins.
                 It initializes global variables and allocates memory buffers (from heap) used for the SPI testing.
  \return        none
*/
void SPI_DV_Initialize (void) {

  // Initialize global variables
  buffers_ok   = -1;
  server_ok    = -1;
  driver_ok    = -1;
  event        = 0U;
  duration     = 0xFFFFFFFFUL;
  systick_freq = osKernelGetSysTimerFreq();

  memset(&spi_serv_cap, 0, sizeof(spi_serv_cap));
  memset(&msg_buf,      0, sizeof(msg_buf));

  // Allocate buffers for transmission, reception and comparison
  // (maximum size is incremented by 4 bytes to ensure that buffer can be aligned to 4 bytes)

  ptr_tx_buf_alloc = malloc(SPI_BUF_MAX + 4U);
  if (((uint32_t)ptr_tx_buf_alloc & 3U) != 0U) {
    // If allocated memory is not 4 byte aligned, use next 4 byte aligned address for ptr_tx_buf
    ptr_tx_buf = (uint8_t *)((((uint32_t)ptr_tx_buf_alloc) + 3U) & (~3U));
  } else {
    // If allocated memory is 4 byte aligned, use it directly
    ptr_tx_buf = (uint8_t *)ptr_tx_buf_alloc;
  }
  ptr_rx_buf_alloc = malloc(SPI_BUF_MAX + 4U);
  if (((uint32_t)ptr_rx_buf_alloc & 3U) != 0U) {
    ptr_rx_buf = (uint8_t *)((((uint32_t)ptr_rx_buf_alloc) + 3U) & (~3U));
  } else {
    ptr_rx_buf = (uint8_t *)ptr_rx_buf_alloc;
  }
  ptr_cmp_buf_alloc = malloc(SPI_BUF_MAX + 4U);
  if (((uint32_t)ptr_cmp_buf_alloc & 3U) != 0U) {
    ptr_cmp_buf = (uint8_t *)((((uint32_t)ptr_cmp_buf_alloc) + 3U) & (~3U));
  } else {
    ptr_cmp_buf = (uint8_t *)ptr_cmp_buf_alloc;
  }
}

/*
  \fn            void SPI_DV_Uninitialize (void)
  \brief         De-initialize testing environment after SPI testing.
  \detail        This function is called by the driver validation framework after SPI testing is finished.
                 It frees memory buffers used for the SPI testing.
  \return        none
*/
void SPI_DV_Uninitialize (void) {

  if (ptr_tx_buf_alloc != NULL) {
    free(ptr_tx_buf_alloc);
    ptr_tx_buf        = NULL;
    ptr_tx_buf_alloc  = NULL;
  }
  if (ptr_rx_buf_alloc != NULL) {
    free(ptr_rx_buf_alloc);
    ptr_rx_buf        = NULL;
    ptr_rx_buf_alloc  = NULL;
  }
  if (ptr_cmp_buf_alloc != NULL) {
    free(ptr_cmp_buf_alloc);
    ptr_cmp_buf       = NULL;
    ptr_cmp_buf_alloc = NULL;
  }
}

#endif                                  // End of exclude form the documentation

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup dv_spi SPI Validation
\brief SPI driver validation
\details
The SPI validation performs the following tests:
- API interface compliance.
- Data exchange with various speeds, transfer sizes and communication settings.
- Error event signaling.

Two Test Modes are available: <b>Loopback</b> and <b>SPI Server</b>.

Test Mode : <b>Loopback</b>
---------------------------

This test mode allows only limited validation of the SPI Driver.<br>
It is recommended that this test mode is used only as a proof that driver is 
good enough to be tested with the <b>SPI Server</b>.

To enable this mode of testing in the <b>DV_SPI_Config.h</b> configuration file select 
the <b>Configuration: Test Mode: Loopback</b> setting.

Required pin connection for the <b>Loopback</b> test mode:

\image html spi_loopback_pin_connections.png

\note In this mode following operations / settings cannot be tested:
 - SPI slave mode
 - Slave Select line functionality
 - operation of the Receive function
 - data content sent by the Send function
 - clock / frame format and bit order settings
 - data bit settings other then: 8, 16, 24 and 32
 - error event generation

Test Mode : <b>SPI Server</b>
-----------------------------

This test mode allows extensive validation of the SPI Driver.<br>
Results of the Driver Validation in this test mode are relevant as a proof of driver compliance to the CMSIS-Driver specification.

To perform extensive communication tests, it is required to use an 
\ref spi_server "SPI Server" running on a dedicated hardware.

To enable this mode of testing in the <b>DV_SPI_Config.h</b> configuration file select 
the <b>Configuration: Test Mode: SPI Server</b> setting.

Required pin connections for the <b>SPI Server</b> test mode:

\image html spi_server_pin_connections.png

\defgroup spi_tests Tests
\ingroup dv_spi
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* SPI Driver Management tests                                                                                              */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup spi_tests_drv_mgmt Driver Management
\ingroup spi_tests
\details
These tests verify API and operation of the SPI driver management functions.

The driver management tests verify the following driver functions
(<a href="http://www.keil.com/pack/doc/CMSIS/Driver/html/group__spi__interface__gr.html" target="_blank">SPI Driver function documentation</a>):
 - \b GetVersion
\code
  ARM_DRIVER_VERSION   GetVersion      (void);
\endcode
 - \b GetCapabilities
\code
  ARM_SPI_CAPABILITIES GetCapabilities (void);
\endcode
 - \b Initialize
\code
  int32_t              Initialize      (ARM_SPI_SignalEvent_t cb_event);
\endcode
 - \b Uninitialize
\code
  int32_t              Uninitialize    (void);
\endcode
 - \b PowerControl
\code
  int32_t              PowerControl    (ARM_POWER_STATE state);
\endcode

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_GetVersion
\details
The function \b SPI_GetVersion verifies the \b GetVersion function.
\code
  ARM_DRIVER_VERSION GetVersion (void);
\endcode

Testing sequence:
  - Driver is uninitialized and peripheral is powered-off:
    - Call GetVersion function
    - Assert that GetVersion function returned version structure with API and implementation versions higher or equal to 1.0
*/
void SPI_GetVersion (void) {
  ARM_DRIVER_VERSION ver;

  ver = drv->GetVersion();

  // Assert that GetVersion function returned version structure with API and implementation versions higher or equal to 1.0
  TEST_ASSERT((ver.api >= ARM_DRIVER_VERSION_MAJOR_MINOR(1UL,0UL)) && (ver.drv >= ARM_DRIVER_VERSION_MAJOR_MINOR(1UL,0UL)));

  (void)snprintf(msg_buf, sizeof(msg_buf), "[INFO] Driver API version %d.%d, Driver version %d.%d", (ver.api >> 8), (ver.api & 0xFFU), (ver.drv >> 8), (ver.drv & 0xFFU));
  TEST_MESSAGE(msg_buf);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_GetCapabilities
\details
The function \b SPI_GetCapabilities verifies the \b GetCapabilities function.
\code
  ARM_SPI_CAPABILITIES GetCapabilities (void);
\endcode

Testing sequence:
  - Driver is uninitialized and peripheral is powered-off:
    - Call GetCapabilities function
    - Assert that GetCapabilities function returned capabilities structure with reserved field 0
*/
void SPI_GetCapabilities (void) {
  ARM_SPI_CAPABILITIES cap;

  cap = drv->GetCapabilities();

  // Assert that GetCapabilities function returned capabilities structure with reserved field 0
  TEST_ASSERT_MESSAGE((cap.reserved == 0U), "[FAILED] Capabilities reserved field must be 0");
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Initialize_Uninitialize
\details
The function \b SPI_Initialize_Uninitialize verifies the \b Initialize and \b Uninitialize functions.
\code
  int32_t Initialize (ARM_SPI_SignalEvent_t cb_event);
\endcode
\code
  int32_t Uninitialize (void);
\endcode

Testing sequence:
  - Driver is uninitialized and peripheral is powered-off:
    - Call PowerControl(ARM_POWER_FULL) function and assert that it returned ARM_DRIVER_ERROR status
    - Call PowerControl(ARM_POWER_LOW) function and assert that it returned ARM_DRIVER_ERROR status
    - Call PowerControl(ARM_POWER_OFF) function and assert that it returned ARM_DRIVER_ERROR status
    - Call Send function and assert that it returned ARM_DRIVER_ERROR status
    - Call Receive function and assert that it returned ARM_DRIVER_ERROR status
    - Call Transfer function and assert that it returned ARM_DRIVER_ERROR status
    - Call GetDataCount function and assert that it returned 0
    - Call Control function and assert that it returned ARM_DRIVER_ERROR status
    - Call GetStatus function
    - Assert that GetStatus function returned status structure with busy flag 0
    - Assert that GetStatus function returned status structure with data_lost flag 0
    - Assert that GetStatus function returned status structure with mode_fault flag 0
    - Assert that GetStatus function returned status structure with reserved field 0
    - Call Initialize function (without callback specified) and assert that it returned ARM_DRIVER_OK status
  - Driver is initialized and peripheral is powered-off:
    - Call Initialize function (without callback specified) again and assert that it returned ARM_DRIVER_OK status
    - Call Uninitialize function and assert that it returned ARM_DRIVER_OK status
  - Driver is uninitialized and peripheral is powered-off:
    - Call Initialize function (with callback specified) and assert that it returned ARM_DRIVER_OK status
  - Driver is initialized and peripheral is powered-off:
    - Call Initialize function (with callback specified) again and assert that it returned ARM_DRIVER_OK status
    - Call Uninitialize function and assert that it returned ARM_DRIVER_OK status
  - Driver is uninitialized and peripheral is powered-off:
    - Call Uninitialize function again and assert that it returned ARM_DRIVER_OK status
    - Call Initialize function (without callback specified) and assert that it returned ARM_DRIVER_OK status
  - Driver is initialized and peripheral is powered-off:
    - Call PowerControl(ARM_POWER_FULL) function and assert that it returned ARM_DRIVER_OK status
  - Driver is initialized and peripheral is powered-on:
    - Call Control function and assert that it returned ARM_DRIVER_OK status
    - Call Transfer function and assert that it returned ARM_DRIVER_OK status
    - Call GetStatus function
    - Assert that GetStatus function returned status structure with busy flag 1
    - Call Uninitialize function and assert that it returned ARM_DRIVER_OK status<br>
      (this must unconditionally terminate active transfer, power-off the peripheral and uninitialize the driver)
  - Driver is uninitialized and peripheral is powered-off:
    - Call GetStatus function
    - Assert that GetStatus function returned status structure with busy flag 0
*/
void SPI_Initialize_Uninitialize (void) {
  ARM_SPI_STATUS stat;

  // Driver is uninitialized and peripheral is powered-off:
  // Call PowerControl(ARM_POWER_FULL) function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_ERROR);

  // Call PowerControl(ARM_POWER_LOW) function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_LOW) == ARM_DRIVER_ERROR);

  // Call PowerControl(ARM_POWER_OFF) function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_ERROR);

  // Call Send function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Send (ptr_tx_buf, SPI_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call Receive function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Receive (ptr_rx_buf, SPI_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call Transfer function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Transfer (ptr_tx_buf, ptr_rx_buf, SPI_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call GetDataCount function and assert that it returned 0
  TEST_ASSERT(drv->GetDataCount () == 0U);

  // Call Control function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Control (ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(SPI_CFG_DEF_DATA_BITS), SPI_CFG_DEF_BUS_SPEED) == ARM_DRIVER_ERROR);

  // Call GetStatus function
  stat = drv->GetStatus();

  // Assert that GetStatus function returned status structure with busy flag 0
  TEST_ASSERT(stat.busy == 0U);

  // Assert that GetStatus function returned status structure with data_lost flag 0
  TEST_ASSERT(stat.data_lost == 0U);

  // Assert that GetStatus function returned status structure with mode_fault flag 0
  TEST_ASSERT(stat.mode_fault == 0U);

  // Assert that GetStatus function returned status structure with reserved field 0
  TEST_ASSERT(stat.reserved == 0U);

  // Call Initialize function (without callback specified) and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Initialize (NULL) == ARM_DRIVER_OK);

  // Driver is initialized and peripheral is powered-off:
  // Call Initialize function (without callback specified) again and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Initialize (NULL) == ARM_DRIVER_OK);

  // Call Uninitialize function and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Uninitialize () == ARM_DRIVER_OK);

  // Driver is uninitialized and peripheral is powered-off:
  // Call Initialize function (with callback specified) and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Initialize (SPI_DrvEvent) == ARM_DRIVER_OK);

  // Driver is initialized and peripheral is powered-off:
  // Call Initialize function (with callback specified) again and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Initialize (SPI_DrvEvent) == ARM_DRIVER_OK);

  // Call Uninitialize function and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Uninitialize () == ARM_DRIVER_OK);

  // Driver is uninitialized and peripheral is powered-off:
  // Call Uninitialize function again and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Uninitialize () == ARM_DRIVER_OK);

  // Call Initialize function (without callback specified) and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Initialize (NULL) == ARM_DRIVER_OK);

  // Driver is initialized and peripheral is powered-off:
  // Call PowerControl(ARM_POWER_FULL) function and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);

  // Driver is initialized and peripheral is powered-on:
  // Call Control function and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Control (ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(SPI_CFG_DEF_DATA_BITS), SPI_CFG_DEF_BUS_SPEED) == ARM_DRIVER_OK);

  // Call Transfer function and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Transfer (ptr_tx_buf, ptr_rx_buf, SPI_CFG_DEF_NUM) == ARM_DRIVER_OK);

  // Call GetStatus function
  stat = drv->GetStatus();

  // Assert that GetStatus function returned status structure with busy flag 1
  TEST_ASSERT(stat.busy == 1U);

  // Call Uninitialize function and assert that it returned ARM_DRIVER_OK status
  // (this must unconditionally terminate active transfer, power-off the peripheral and uninitialize the driver)
  TEST_ASSERT(drv->Uninitialize () == ARM_DRIVER_OK);

  // Driver is uninitialized and peripheral is powered-off:
  // Call GetStatus function
  stat = drv->GetStatus();

  // Assert that GetStatus function returned status structure with busy flag 0
  TEST_ASSERT(stat.busy == 0U);

#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected
  // Insure that SPI Server (if used) is ready for command reception
  (void)osDelay(SPI_CFG_XFER_TIMEOUT + 10U);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_PowerControl
\details
The function \b SPI_PowerControl verifies the \b PowerControl function.
\code
  int32_t PowerControl (ARM_POWER_STATE state);
\endcode

Testing sequence:
  - Driver is initialized and peripheral is powered-off:
    - Call Send function and assert that it returned ARM_DRIVER_ERROR status
    - Call Receive function and assert that it returned ARM_DRIVER_ERROR status
    - Call Transfer function and assert that it returned ARM_DRIVER_ERROR status
    - Call GetDataCount function and assert that it returned 0
    - Call Control function and assert that it returned ARM_DRIVER_ERROR status
    - Call GetStatus function
    - Assert that GetStatus function returned status structure with busy flag 0
    - Assert that GetStatus function returned status structure with data_lost flag 0
    - Assert that GetStatus function returned status structure with mode_fault flag 0
    - Assert that GetStatus function returned status structure with reserved field 0
    - Call PowerControl(ARM_POWER_OFF) and assert that it returned ARM_DRIVER_OK status
    - Call PowerControl(ARM_POWER_OFF) again and assert that it returned ARM_DRIVER_OK status
    - Call PowerControl(ARM_POWER_FULL) function and assert that it returned ARM_DRIVER_OK status
  - Driver is initialized and peripheral is powered-on:
    - Call PowerControl(ARM_POWER_FULL) function again and assert that it returned ARM_DRIVER_OK status
    - Call PowerControl(ARM_POWER_OFF) and assert that it returned ARM_DRIVER_OK status
  - Driver is initialized and peripheral is powered-off:
    - Call PowerControl(ARM_POWER_OFF) again and assert that it returned ARM_DRIVER_OK status
    - Call PowerControl(ARM_POWER_LOW) function
  - Driver is initialized and peripheral is powered-on or in low-power mode:
    - Assert that PowerControl(ARM_POWER_LOW) function returned ARM_DRIVER_OK or ARM_DRIVER_ERROR_UNSUPPORTED status
    - Call PowerControl(ARM_POWER_FULL) function and assert that it returned ARM_DRIVER_OK status
  - Driver is initialized and peripheral is powered-on:
    - Call Control function and assert that it returned ARM_DRIVER_OK status
    - Call Transfer function and assert that it returned ARM_DRIVER_OK status
    - Call GetStatus function
    - Assert that GetStatus function returned status structure with busy flag 1
    - Call PowerControl(ARM_POWER_OFF) function with transfer active and assert that it returned ARM_DRIVER_OK status<br>
      (this must unconditionally terminate active transfer and power-off the peripheral)
  - Driver is initialized and peripheral is powered-off:
    - Call GetStatus function
    - Assert that GetStatus function returned status structure with busy flag 0
*/
void SPI_PowerControl (void) {
  int32_t        ret;
  ARM_SPI_STATUS stat;

  (void)drv->Initialize (NULL);

  // Driver is initialized and peripheral is powered-off:
  // Call Send function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Send (ptr_tx_buf, SPI_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call Receive function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Receive (ptr_rx_buf, SPI_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call Transfer function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Transfer (ptr_tx_buf, ptr_rx_buf, SPI_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call GetDataCount function and assert that it returned 0
  TEST_ASSERT(drv->GetDataCount () == 0U);

  // Call Control function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Control (ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(SPI_CFG_DEF_DATA_BITS), SPI_CFG_DEF_BUS_SPEED) == ARM_DRIVER_ERROR);

  // Call GetStatus function
  stat = drv->GetStatus();

  // Assert that GetStatus function returned status structure with busy flag 0
  TEST_ASSERT(stat.busy == 0U);

  // Assert that GetStatus function returned status structure with data_lost flag 0
  TEST_ASSERT(stat.data_lost == 0U);

  // Assert that GetStatus function returned status structure with mode_fault flag 0
  TEST_ASSERT(stat.mode_fault == 0U);

  // Assert that GetStatus function returned status structure with reserved field 0
  TEST_ASSERT(stat.reserved == 0U);

  // Call PowerControl(ARM_POWER_OFF) and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);

  // Call PowerControl(ARM_POWER_OFF) again and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);

  // Call PowerControl(ARM_POWER_FULL) function and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);

  // Driver is initialized and peripheral is powered-on:
  // Call PowerControl(ARM_POWER_FULL) function again and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);

  // Call PowerControl(ARM_POWER_OFF) and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);

  // Driver is initialized and peripheral is powered-off:
  // Call PowerControl(ARM_POWER_OFF) again and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);

  // Call PowerControl(ARM_POWER_LOW) function
  ret = drv->PowerControl (ARM_POWER_LOW);

  // Driver is initialized and peripheral is powered-on or in low-power mode:
  // Assert that PowerControl(ARM_POWER_LOW) function returned ARM_DRIVER_OK or ARM_DRIVER_ERROR_UNSUPPORTED status
  TEST_ASSERT((ret == ARM_DRIVER_OK) || (ret == ARM_DRIVER_ERROR_UNSUPPORTED));
  if (ret == ARM_DRIVER_ERROR_UNSUPPORTED) {
    TEST_MESSAGE("[WARNING] PowerControl (ARM_POWER_LOW) is not supported");
  }

  // Call PowerControl(ARM_POWER_FULL) function and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);

  // Driver is initialized and peripheral is powered-on:
  // Call Control function and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Control (ARM_SPI_MODE_MASTER | ARM_SPI_DATA_BITS(SPI_CFG_DEF_DATA_BITS), SPI_CFG_DEF_BUS_SPEED) == ARM_DRIVER_OK);

  // Call Transfer function and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Transfer (ptr_tx_buf, ptr_rx_buf, SPI_CFG_DEF_NUM) == ARM_DRIVER_OK);

  // Call GetStatus function
  stat = drv->GetStatus();

  // Assert that GetStatus function returned status structure with busy flag 1
  TEST_ASSERT(stat.busy == 1U);

  // Call PowerControl(ARM_POWER_OFF) function with transfer active and assert that it returned ARM_DRIVER_OK status
  // (this must unconditionally terminate active transfer and power-off the peripheral)
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);

  // Driver is initialized and peripheral is powered-off:
  // Call GetStatus function
  stat = drv->GetStatus();

  // Assert that GetStatus function returned status structure with busy flag 0
  TEST_ASSERT(stat.busy == 0U);

  (void)drv->Uninitialize ();

#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected
  // Insure that SPI Server (if used) is ready for command reception
  (void)osDelay(SPI_CFG_XFER_TIMEOUT + 10U);
#endif
}

/**
@}
*/
// End of spi_tests_drv_mgmt

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* SPI Data Exchange tests                                                                                                  */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup spi_tests_data_xchg Data Exchange
\ingroup spi_tests
\details
These tests verify API and operation of the SPI data exchange functions.

The data exchange tests verify the following driver functions
(<a href="http://www.keil.com/pack/doc/CMSIS/Driver/html/group__spi__interface__gr.html" target="_blank">SPI Driver function documentation</a>):
 - \b Send
\code
  int32_t        Send         (const void *data,                    uint32_t num);
\endcode
 - \b Receive
\code
  int32_t        Receive      (      void *data,                    uint32_t num);
\endcode
 - \b Transfer
\code
  int32_t        Transfer     (const void *data_out, void *data_in, uint32_t num);
\endcode
 - \b GetDataCount
\code
  uint32_t       GetDataCount (void);
\endcode
 - \b Control
\code
  int32_t        Control      (uint32_t control, uint32_t arg);
\endcode
 - \b GetStatus
\code
  ARM_SPI_STATUS GetStatus    (void);
\endcode
 - \b SignalEvent
\code
  void (*ARM_SPI_SignalEvent_t) (uint32_t event);
\endcode

All of these tests execute a data exchange and check the result of this data exchange.

Data exchange test procedure when Test Mode <b>SPI Server</b> is selected:
  - send command "SET BUF TX,.." to the SPI Server: Set Tx buffer
  - send command "SET BUF RX,.." to the SPI Server: Set Rx buffer
  - send command "SET COM .."    to the SPI Server: Set communication settings for the next XFER command
  - send command "XFER .."       to the SPI Server: Activate transfer
  - driver Control: Configure the SPI interface
  - driver Control: Set the default Tx value
  - driver Send/Receive/Transfer: Start the requested operation
  - driver GetStatus/SignalEvent: Wait for the current operation to finish or time out<br>
    (operation is finished when busy flag is 0 and completed event was signaled)
  - assert that operation has finished in expected time
  - assert that ARM_SPI_EVENT_TRANSFER_COMPLETE event was signaled
  - driver GetStatus: Assert that busy flag is 0
  - driver GetDataCount: Assert that number of transferred items is same as requested
  - if operation has timed out call driver Control function to Abort operation and wait timeout time<br>
    to make sure that the SPI Server is ready for the next command
  - assert that received content is as expected
  - send command "GET BUF RX,.." to the SPI Server: Get Rx buffer
  - assert that sent content (read from the SPI Server's receive buffer) is as expected

Data exchange <b>Abort</b> test procedure when Test Mode <b>SPI Server</b> is selected:
  - send command "SET BUF TX,.." to the SPI Server: Set Tx buffer
  - send command "SET BUF RX,.." to the SPI Server: Set Rx buffer
  - send command "SET COM .."    to the SPI Server: Set communication settings for the next XFER command
  - send command "XFER .."       to the SPI Server: Activate transfer
  - driver Control: Configure the SPI interface
  - driver Control: Set the default Tx value
  - driver Send/Receive/Transfer: Start the requested operation
  - wait up to 1 ms
  - driver Control: Abort the current operation
  - driver GetStatus: Assert that busy flag is 0
  - driver GetDataCount: Assert that number of transferred items is less than requested

Data exchange test procedure when Test Mode <b>Loopback</b> is selected:
  - driver Control: Configure the SPI interface
  - driver Control: Set the default Tx value
  - driver Send/Transfer: Start the requested operation
  - driver GetStatus/SignalEvent: Wait for the current operation to finish or time out<br>
    (operation is finished when busy flag is 0 and completed event was signaled)
  - assert that operation has finished in expected time
  - assert that ARM_SPI_EVENT_TRANSFER_COMPLETE event was signaled
  - driver GetStatus: Assert that busy flag is 0
  - driver GetDataCount: Assert that number of transferred items is same as requested
  - if operation has timed out call driver Control function to Abort operation
  - assert that received content is as expected (for Transfer operation only)

\note Limitations of Data Exchange tests if Test Mode <b>Loopback</b> is selected:
 - only Master mode with Slave Select not used can be tested
 - Receive function cannot be tested
 - format tests are not supported
 - only 8, 16, 24 and 32 data bit tests are supported
 - bit order tests are not supported
@{
*/

#ifndef __DOXYGEN__                     // Exclude form the documentation
/*
  \brief         Execute SPI data exchange or data exchange abort operation.
  \param[in]     operation      operation (OP_SEND.. OP_ABORT_TRANSFER)
  \param[in]     mode           mode (MODE_MASTER or MODE_SLAVE)
  \param[in]     format         clock/frame format (0 = polarity0/phase0 .. 5 = Microwire)
  \param[in]     data_bits      data bits (1 .. 32)
  \param[in]     bit_order      bit order (BO_MSB_TO_LSB or BO_LSB_TO_MSB)
  \param[in]     ss_mode        Slave Select mode (SS_MODE_xxx)
  \param[in]     bus_speed      bus speed in bits per second (bps)
  \param[in]     num            number of items to send, receive or transfer
  \return        none
*/
static void SPI_DataExchange_Operation (uint32_t operation, uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t ss_mode, uint32_t bus_speed, uint32_t num) {
  // volatile specifier is used to prevent compiler from optimizing variables 
  // in a way that they cannot be seen with debugger
  volatile  int32_t       stat, def_tx_stat;
  volatile uint32_t       drv_mode, drv_format, drv_data_bits, drv_bit_order, drv_ss_mode;
  volatile uint32_t       srv_mode, srv_ss_mode;
  volatile ARM_SPI_STATUS spi_stat;
  volatile uint32_t       data_count;
           uint32_t       start_cnt, max_cnt;
           uint32_t       val, delay, i;

  // Prepare parameters for SPI Server and Driver configuration
  switch (mode) {
    case MODE_INACTIVE:
      TEST_FAIL_MESSAGE("[FAILED] Inactive mode! Data exchange operation aborted!");
      return;
    case MODE_MASTER:
      drv_mode = ARM_SPI_MODE_MASTER;
      srv_mode = 1U;
      delay    = 0U;
      break;
    case MODE_SLAVE:
      drv_mode = ARM_SPI_MODE_SLAVE;
      srv_mode = 0U;
      delay    = 20U;
      break;
    default:
      TEST_FAIL_MESSAGE("[FAILED] Unknown mode! Data exchange operation aborted!");
      return;
  }

  switch (format) {
    case FORMAT_CPOL0_CPHA0:
      drv_format = ARM_SPI_CPOL0_CPHA0;
      break;
    case FORMAT_CPOL0_CPHA1:
      drv_format = ARM_SPI_CPOL0_CPHA1;
      break;
    case FORMAT_CPOL1_CPHA0:
      drv_format = ARM_SPI_CPOL1_CPHA0;
      break;
    case FORMAT_CPOL1_CPHA1:
      drv_format = ARM_SPI_CPOL1_CPHA1;
      break;
    case FORMAT_TI:
      drv_format = ARM_SPI_TI_SSI;
      break;
    case FORMAT_MICROWIRE:
      drv_format = ARM_SPI_MICROWIRE;
      break;
    default:
      TEST_FAIL_MESSAGE("[FAILED] Unknown clock / frame format! Data exchange operation aborted!");
      return;
  }

  if ((data_bits > 0U) && (data_bits <= 32U)) {
    drv_data_bits = ARM_SPI_DATA_BITS(data_bits);
  } else {
    TEST_FAIL_MESSAGE("[FAILED] Data bits not in range 1 to 32! Data exchange operation aborted!");
    return;
  }

  switch (bit_order) {
    case BO_MSB_TO_LSB:
      drv_bit_order = ARM_SPI_MSB_LSB;
      break;
    case BO_LSB_TO_MSB:
      drv_bit_order = ARM_SPI_LSB_MSB;
      break;
    default:
      TEST_FAIL_MESSAGE("[FAILED] Unknown bit order! Data exchange operation aborted!");
      return;
  }

  if (mode == MODE_MASTER) {
    switch (ss_mode) {
      case SS_MODE_MASTER_UNUSED:
        drv_ss_mode = ARM_SPI_SS_MASTER_UNUSED;
        srv_ss_mode = 0U;
        break;
      case SS_MODE_MASTER_SW:
        drv_ss_mode = ARM_SPI_SS_MASTER_SW;
        srv_ss_mode = 1U;
        break;
      case SS_MODE_MASTER_HW_OUTPUT:
        drv_ss_mode = ARM_SPI_SS_MASTER_HW_OUTPUT;
        srv_ss_mode = 1U;
        break;
      case SS_MODE_MASTER_HW_INPUT:
        drv_ss_mode = ARM_SPI_SS_MASTER_UNUSED;
        srv_ss_mode = 0U;
        break;
      default:
        TEST_FAIL_MESSAGE("[FAILED] Unknown Slave Select mode! Data exchange operation aborted!");
        return;
    }
  } else {
    switch (ss_mode) {
      case SS_MODE_SLAVE_HW:
        drv_ss_mode = ARM_SPI_SS_SLAVE_HW;
        srv_ss_mode = 1U;
        break;
      case SS_MODE_SLAVE_SW:
        drv_ss_mode = ARM_SPI_SS_SLAVE_SW;
        srv_ss_mode = 0U;
        break;
      default:
        TEST_FAIL_MESSAGE("[FAILED] Unknown Slave Select mode! Data exchange operation aborted!");
        return;
    }
  }

  // Check that SPI status is not busy before starting data exchange test
  spi_stat = drv->GetStatus();          // Get SPI status
  if (spi_stat.busy != 0U) {
    // If busy flag is still active
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Busy active before operation! Data exchange operation aborted!");
    return;
  }
  TEST_ASSERT_MESSAGE(spi_stat.busy == 0U, msg_buf);

  do {
#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdSetBufTx('S')   != EXIT_SUCCESS) { break; }
    if (CmdSetBufRx('?')   != EXIT_SUCCESS) { break; }
    if (CmdSetCom  (srv_mode, format, data_bits, bit_order, srv_ss_mode, bus_speed) != EXIT_SUCCESS) { break; }
    if (CmdXfer    (num, delay, SPI_CFG_XFER_TIMEOUT, 0U) != EXIT_SUCCESS) { break; }
#else                                   // If Test Mode Loopback is selected
    // Remove warnings for unused variables
    (void)srv_mode;
    (void)srv_ss_mode;
    (void)delay;
#endif

    // Initialize buffers
    memset(ptr_tx_buf,  (int32_t)'!' , SPI_BUF_MAX);
    memset(ptr_tx_buf,  (int32_t)'T' , num * DataBitsToBytes(data_bits));
    memset(ptr_rx_buf,  (int32_t)'?' , SPI_BUF_MAX);
    memset(ptr_cmp_buf, (int32_t)'?' , SPI_BUF_MAX);

    // Configure required communication settings
    if (mode == MODE_MASTER) {
      stat = drv->Control (drv_mode | drv_format | drv_data_bits | drv_bit_order | drv_ss_mode, bus_speed);
      (void)osDelay(10U);               // Give time to SPI Server to prepare Slave transfer
    } else {
      // For Slave mode bus speed argument is not used
      stat = drv->Control (drv_mode | drv_format | drv_data_bits | drv_bit_order | drv_ss_mode, 0U);
    }
    if (stat != ARM_DRIVER_OK) {
      // If configuration has failed
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Control function returned", str_ret[-stat]);
    }
    // Assert that Control function returned ARM_DRIVER_OK
    TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);
    if (stat != ARM_DRIVER_OK) {
      // If Control function has failed there is no sense to try to execute the data exchange
      (void)osDelay(SPI_CFG_XFER_TIMEOUT+10U);  // Wait for SPI Server to timeout the XFER command
      return;
    }

    // Set default Tx value to 'D' byte values
    val = ((uint32_t)'D' << 24) | ((uint32_t)'D' << 16) | ((uint32_t)'D' << 8) | (uint32_t)'D';
    stat = drv->Control (ARM_SPI_SET_DEFAULT_TX_VALUE, val);
    def_tx_stat = stat;
    if ((stat != ARM_DRIVER_OK) && (stat != ARM_DRIVER_ERROR_UNSUPPORTED)) {
      // If set default Tx value has failed or is not supported
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s", str_oper[operation], "Set default Tx value returned", str_ret[-stat]);
    }
    // Assert that Control function returned ARM_DRIVER_OK or ARM_DRIVER_ERROR_UNSUPPORTED
    TEST_ASSERT_MESSAGE((stat == ARM_DRIVER_OK) || (stat == ARM_DRIVER_ERROR_UNSUPPORTED), msg_buf);

    event       = 0U;
    duration    = 0xFFFFFFFFUL;
    data_count  = 0U;
    max_cnt     = (systick_freq /1024U) * (SPI_CFG_XFER_TIMEOUT + 10U);
    start_cnt   = osKernelGetSysTimerCount();

    if (((mode == MODE_MASTER) && (ss_mode == SS_MODE_MASTER_SW)) || 
        ((mode == MODE_SLAVE)  && (ss_mode == SS_MODE_SLAVE_SW)))  {
      // If operation requires software Slave Select driving, activate Slave Select
      stat = drv->Control (ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);
      if (stat != ARM_DRIVER_OK) {
        // If driving of Slave Select to active state has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s", str_oper[operation], "Control function returned", str_ret[-stat]);
      }
      // Assert that Control function returned ARM_DRIVER_OK
      TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);
    }

    // Start the data exchange operation
    switch (operation & 0x0FU) {
      case OP_SEND:
      case OP_ABORT_SEND:
        stat = drv->Send(ptr_tx_buf, num);
        if (stat != ARM_DRIVER_OK) {
          // If Send activation has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Send function returned", str_ret[-stat]);
        }
        // Assert that Send function returned ARM_DRIVER_OK
        TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);
        break;
      case OP_RECEIVE:
      case OP_ABORT_RECEIVE:
        stat = drv->Receive(ptr_rx_buf, num);
        if (stat != ARM_DRIVER_OK) {
          // If Receive activation has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Receive function returned", str_ret[-stat]);
        }
        // Assert that Receive function returned ARM_DRIVER_OK
        TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);
        break;
      case OP_TRANSFER:
      case OP_ABORT_TRANSFER:
        stat = drv->Transfer(ptr_tx_buf, ptr_rx_buf, num);
        if (stat != ARM_DRIVER_OK) {
          // If Transfer activation has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Transfer function returned", str_ret[-stat]);
        }
        // Assert that Transfer function returned ARM_DRIVER_OK
        TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);
        break;
      default:
        TEST_FAIL_MESSAGE("[FAILED] Unknown operation! Data exchange operation aborted!");
        return;
    }
    if (stat != ARM_DRIVER_OK) {
      // If Send/Receive/Transfer start has failed there is no sense to try to continue with the data exchange
      (void)osDelay(SPI_CFG_XFER_TIMEOUT+10U);  // Wait for SPI Server to timeout the XFER command
      return;
    }

    if ((operation == OP_ABORT_SEND)     ||     // This IF block tests only abort functionality
        (operation == OP_ABORT_RECEIVE)  ||
        (operation == OP_ABORT_TRANSFER)) {
      (void)osDelay(1U);                        // Wait short time before doing Abort
      stat = drv->Control (ARM_SPI_ABORT_TRANSFER, 0U);
      if (stat != ARM_DRIVER_OK) {
        // If Abort has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s", str_oper[operation], "Control function returned", str_ret[-stat]);
      }
      // Assert that Control function returned ARM_DRIVER_OK
      TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);

      if (((mode == MODE_MASTER) && (ss_mode == SS_MODE_MASTER_SW)) || 
          ((mode == MODE_SLAVE)  && (ss_mode == SS_MODE_SLAVE_SW)))  {
        // If operation requires software Slave Select driving, deactivate Slave Select
        drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
      }
      spi_stat = drv->GetStatus();              // Get SPI status
      if (spi_stat.busy != 0U) {
        // If busy flag is still active
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Busy still active after Abort");
      }
      // Assert that busy flag is not active
      TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);

      data_count = drv->GetDataCount();         // Get data count
      if (data_count >= num) {
        // If data count is more or equal to number of items then Abort has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetDataCount returned", data_count, "after Abort of", num, "items");
      }
      // Assert data count is less then number of items requested for exchange
      TEST_ASSERT_MESSAGE(data_count < num, msg_buf);

      (void)osDelay(SPI_CFG_XFER_TIMEOUT+10U);  // Wait for SPI Server to timeout aborted operation

      return;                                   // Here Abort test is finished, exit
    }

    // Wait for operation to finish (status busy is 0 and event complete signaled, or timeout)
    do {
      if ((drv->GetStatus().busy == 0U) && ((event & ARM_SPI_EVENT_TRANSFER_COMPLETE) != 0U)) {
        duration = osKernelGetSysTimerCount() - start_cnt;
        break;
      }
    } while ((osKernelGetSysTimerCount() - start_cnt) < max_cnt);

    if (duration == 0xFFFFFFFFUL) {
      // If operation has timed out
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Operation timed out");
    }
    // Assert that operation has finished in expected time
    TEST_ASSERT_MESSAGE(duration != 0xFFFFFFFFUL, msg_buf);

    if (((mode == MODE_MASTER) && (ss_mode == SS_MODE_MASTER_SW)) || 
        ((mode == MODE_SLAVE)  && (ss_mode == SS_MODE_SLAVE_SW)))  {
      // If operation requires software Slave Select driving, deactivate Slave Select
      stat = drv->Control (ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE);
      if (stat != ARM_DRIVER_OK) {
        // If driving of Slave Select to inactive state has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s", str_oper[operation], "Control function returned", str_ret[-stat]);
      }
      // Assert that Control function returned ARM_DRIVER_OK
      TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);
    }

    if ((event & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U) {
      // If transfer complete event was not signaled
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_SPI_EVENT_TRANSFER_COMPLETE was not signaled");
    }
    // Assert that ARM_SPI_EVENT_TRANSFER_COMPLETE was signaled
    TEST_ASSERT_MESSAGE((event & ARM_SPI_EVENT_TRANSFER_COMPLETE) != 0U, msg_buf);

    spi_stat = drv->GetStatus();                // Get SPI status
    if (spi_stat.busy != 0U) {
      // If busy flag is still active
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Busy still active after operation");
    }
    // Assert that busy flag is not active
    TEST_ASSERT_MESSAGE(spi_stat.busy == 0U, msg_buf);

    data_count = drv->GetDataCount();           // Get data count
    if (data_count != num) {
      // If data count is different then number of items, then operation has failed
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetDataCount returned", data_count, "expected was", num, "items");
    }
    // Assert that data count is equal to number of items requested for exchange
    TEST_ASSERT_MESSAGE(data_count == num, msg_buf);

    if ((drv->GetStatus().busy != 0U) || ((event & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U)) {
      // If transfer did not finish in time, abort it
      (void)drv->Control(ARM_SPI_ABORT_TRANSFER, 0U);
      (void)osDelay(SPI_CFG_XFER_TIMEOUT+10U);  // Wait for SPI Server to timeout aborted operation
    }

    // Check received content for receive and transfer operations
#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected
    if ((operation == OP_RECEIVE) || (operation == OP_TRANSFER)) {
      memset(ptr_cmp_buf, (int32_t)'S', num * DataBitsToBytes(data_bits));
      stat = memcmp(ptr_rx_buf, ptr_cmp_buf, num * DataBitsToBytes(data_bits));
      if (stat != 0) {
        // If data received mismatches
        // Find on which byte mismatch starts
        for (i = 0U; i < (num * DataBitsToBytes(data_bits)); i++) {
          if (ptr_rx_buf[i] != ptr_cmp_buf[i]) {
            break;
          }
        }
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s byte %i, received was 0x%02X, expected was 0x%02X", str_oper[operation], "Received data mismatches on", i, ptr_rx_buf[i], ptr_cmp_buf[i]);
      }
      // Assert that data received is same as expected
      TEST_ASSERT_MESSAGE(stat == 0, msg_buf);
    }

    // Check sent content (by checking SPI Server's received buffer content)
    if (ComConfigDefault()       != EXIT_SUCCESS) { break; }
    if (CmdGetBufRx(SPI_BUF_MAX) != EXIT_SUCCESS) { break; }

    if ((operation == OP_RECEIVE) && (def_tx_stat == ARM_DRIVER_OK)) {
      // Expected data received by SPI Server should be default Tx value
      memset(ptr_cmp_buf, (int32_t)'D', num * DataBitsToBytes(data_bits));
    }
    if ((operation == OP_SEND) || (operation == OP_TRANSFER)) {
      // Expected data received by SPI Server should be what was sent
      memset(ptr_cmp_buf, (int32_t)'T', num * DataBitsToBytes(data_bits));
    }

    stat = memcmp(ptr_rx_buf, ptr_cmp_buf, num * DataBitsToBytes(data_bits));
    if (stat != 0) {
      // If data sent mismatches
      // Find on which byte mismatch starts
      for (i = 0U; i < (num * DataBitsToBytes(data_bits)); i++) {
        if (ptr_rx_buf[i] != ptr_cmp_buf[i]) {
          break;
        }
      }
      if (operation == OP_RECEIVE) {
        // If sent was default Tx value, 'D' bytes
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s byte %i, SPI Server received 0x%02X, sent was 0x%02X", str_oper[operation], "Default Tx data mismatches on", i, ptr_rx_buf[i], ptr_cmp_buf[i]);
      } else {
        // If sent was 'T' bytes
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s byte %i, SPI Server received 0x%02X, sent was 0x%02X", str_oper[operation], "Sent data mismatches on", i, ptr_rx_buf[i], ptr_cmp_buf[i]);
      }
    }
    // Assert data sent is same as expected
    TEST_ASSERT_MESSAGE(stat == 0, msg_buf);

#else                                   // If Test Mode Loopback is selected
    if (operation == OP_TRANSFER) {
      memset(ptr_cmp_buf, (int32_t)'T', num * DataBitsToBytes(data_bits));
      stat = memcmp(ptr_rx_buf, ptr_cmp_buf, num * DataBitsToBytes(data_bits));
      if (stat != 0) {
        // If data received mismatches
        // Find on which byte mismatch starts
        for (i = 0U; i < (num * DataBitsToBytes(data_bits)); i++) {
          if (ptr_rx_buf[i] != ptr_cmp_buf[i]) {
            break;
          }
        }
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s byte %i, received was 0x%02X, expected was 0x%02X", str_oper[operation], "Received data mismatches on", i, ptr_rx_buf[i], ptr_cmp_buf[i]);
      }
      // Assert that data received is same as expected
      TEST_ASSERT_MESSAGE(stat == 0, msg_buf);
    }
#endif

    return;
  } while (false);

#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected
  TEST_FAIL_MESSAGE("[FAILED] Problems in communication with SPI Server. Test aborted!");
#endif
}

#endif                                  // End of exclude form the documentation

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Master_SS_Unused
\details
The function \b SPI_Mode_Master_SS_Unused verifies data exchange:
 - in <b>Master Mode</b> with <b>Slave Select line not used</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> Receive function is not checked
*/
void SPI_Mode_Master_SS_Unused (void) {

  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
#if (SPI_SERVER_USED != 0)
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.mode_mask & 2U) == 0U) {    // If SPI Server does not support Slave mode
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_UNUSED, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
#if (SPI_SERVER_USED != 0)
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_UNUSED, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
#endif
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_UNUSED, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Master_SS_Sw_Ctrl
\details
The function \b SPI_Mode_Master_SS_Sw_Ctrl verifies data exchange:
 - in <b>Master Mode</b> with <b>Slave Select line Software Controlled</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Mode_Master_SS_Sw_Ctrl (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.mode_mask & 2U) == 0U) {    // If SPI Server does not support Slave mode
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Master_SS_Hw_Ctrl_Out
\details
The function \b SPI_Mode_Master_SS_Hw_Ctrl_Out verifies data exchange:
 - in <b>Master Mode</b> with <b>Slave Select line Hardware Controlled Output</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Mode_Master_SS_Hw_Ctrl_Out (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.mode_mask & 2U) == 0U) {    // If SPI Server does not support Slave mode
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Master_SS_Hw_Mon_In
\details
The function \b SPI_Mode_Master_SS_Hw_Mon_In verifies data exchange:
 - in <b>Master Mode</b> with <b>Slave Select line Hardware Monitored Input</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Mode_Master_SS_Hw_Mon_In (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.mode_mask & 2U) == 0U) {    // If SPI Server does not support Slave mode
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_INPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_INPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_INPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Slave_SS_Hw_Mon
\details
The function \b SPI_Mode_Slave_SS_Hw_Mon verifies data exchange:
 - in <b>Slave Mode</b> with <b>Slave Select line Hardware Monitored</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Mode_Slave_SS_Hw_Mon (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.mode_mask & 1U) == 0U) {    // If SPI Server does not support Master mode
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_HW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_HW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_HW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Slave_SS_Sw_Ctrl
\details
The function \b SPI_Mode_Slave_SS_Sw_Ctrl verifies data exchange:
 - in <b>Slave Mode</b> with <b>Slave Select line Software Controlled</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Mode_Slave_SS_Sw_Ctrl (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.mode_mask & 1U) == 0U) {    // If SPI Server does not support Master mode
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Format_Clock_Pol0_Pha0
\details
The function \b SPI_Format_Clock_Pol0_Pha0 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with clock format: <b>polarity 0 / phase 0</b>
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Format_Clock_Pol0_Pha0 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.fmt_mask & 1U) == 0U) {     // If SPI Server does not support format
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_CPOL0_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_CPOL0_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_CPOL0_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Format_Clock_Pol0_Pha1
\details
The function \b SPI_Format_Clock_Pol0_Pha1 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with clock format: <b>polarity 0 / phase 1</b>
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Format_Clock_Pol0_Pha1 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.fmt_mask & 2U) == 0U) {     // If SPI Server does not support format
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_CPOL0_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_CPOL0_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_CPOL0_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Format_Clock_Pol1_Pha0
\details
The function \b SPI_Format_Clock_Pol1_Pha0 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with clock format: <b>polarity 1 / phase 0</b>
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Format_Clock_Pol1_Pha0 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.fmt_mask & 4U) == 0U) {     // If SPI Server does not support format
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_CPOL1_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_CPOL1_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_CPOL1_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Format_Clock_Pol0_Pha0
\details
The function \b SPI_Format_Clock_Pol1_Pha1 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with clock format: <b>polarity 1 / phase 1</b>
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Format_Clock_Pol1_Pha1 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.fmt_mask & 8U) == 0U) {     // If SPI Server does not support format
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_CPOL1_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_CPOL1_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_CPOL1_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Format_Frame_TI
\details
The function \b SPI_Format_Frame_TI verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with <b>Texas Instruments frame format</b>
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Format_Frame_TI (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.fmt_mask & 16U) == 0U) {    // If SPI Server does not support format
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_TI, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_TI, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_TI, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Format_Clock_Microwire
\details
The function \b SPI_Format_Clock_Microwire verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with <b>National Semiconductor Microwire frame format</b>
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Format_Clock_Microwire (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.fmt_mask & 32U) == 0U) {    // If SPI Server does not support format
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_MICROWIRE, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_MICROWIRE, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_MICROWIRE, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_1
\details
The function \b SPI_Data_Bits_1 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>1 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_1 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & 1U) == 0U) {      // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 1U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 1U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 1U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_2
\details
The function \b SPI_Data_Bits_2 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>2 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_2 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 1)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 2U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 2U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 2U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_3
\details
The function \b SPI_Data_Bits_3 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>3 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_3 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 2)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 3U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 3U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 3U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_4
\details
The function \b SPI_Data_Bits_4 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>4 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_4 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 3)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 4U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 4U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 4U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_5
\details
The function \b SPI_Data_Bits_5 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>5 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_5 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 4)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 5U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 5U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 5U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_6
\details
The function \b SPI_Data_Bits_6 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>6 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_6 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 5)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 6U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 6U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 6U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_7
\details
The function \b SPI_Data_Bits_7 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>7 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_7 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 6)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 7U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 7U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 7U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_8
\details
The function \b SPI_Data_Bits_8 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>8 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items
*/
void SPI_Data_Bits_8 (void) {

  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
#if (SPI_SERVER_USED != 0)
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 7)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 8U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 8U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 8U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_9
\details
The function \b SPI_Data_Bits_9 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>9 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_9 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 8)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 9U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 9U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 9U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_10
\details
The function \b SPI_Data_Bits_10 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>10 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_10 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 9)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 10U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 10U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 10U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_11
\details
The function \b SPI_Data_Bits_11 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>11 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_11 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 10)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 11U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 11U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 11U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_12
\details
The function \b SPI_Data_Bits_12 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>12 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_12 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 11)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 12U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 12U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 12U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_13
\details
The function \b SPI_Data_Bits_13 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>13 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_13 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 12)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 13U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 13U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 13U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_14
\details
The function \b SPI_Data_Bits_14 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>14 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_14 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 13)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 14U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 14U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 14U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_15
\details
The function \b SPI_Data_Bits_15 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>15 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_15 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 14)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 15U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 15U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 15U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_16
\details
The function \b SPI_Data_Bits_16 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>16 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items
*/
void SPI_Data_Bits_16 (void) {

  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
#if (SPI_SERVER_USED != 0)
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 15)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 16U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 16U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 16U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_17
\details
The function \b SPI_Data_Bits_17 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>17 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_17 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 16)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 17U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 17U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 17U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_18
\details
The function \b SPI_Data_Bits_18 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>18 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_18 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 17)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 18U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 18U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 18U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_19
\details
The function \b SPI_Data_Bits_19 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>19 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_19 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 18)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 19U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 19U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 19U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_20
\details
The function \b SPI_Data_Bits_20 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>20 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_20 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 19)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 20U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 20U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 20U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_21
\details
The function \b SPI_Data_Bits_21 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>21 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_21 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 20)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 21U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 21U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 21U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_22
\details
The function \b SPI_Data_Bits_22 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>22 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_22 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 21)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 22U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 22U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 22U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_23
\details
The function \b SPI_Data_Bits_23 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>23 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_23 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 22)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 23U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 23U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 23U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_24
\details
The function \b SPI_Data_Bits_24 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>24 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items
*/
void SPI_Data_Bits_24 (void) {

  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
#if (SPI_SERVER_USED != 0)
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 23)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 24U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 24U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 24U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_25
\details
The function \b SPI_Data_Bits_25 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>25 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_25 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 24)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 25U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 25U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 25U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_26
\details
The function \b SPI_Data_Bits_26 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>26 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_26 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 25)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 26U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 26U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 26U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_27
\details
The function \b SPI_Data_Bits_27 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>27 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_27 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 26)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 27U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 27U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 27U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_28
\details
The function \b SPI_Data_Bits_28 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>28 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_28 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 27)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 28U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 28U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 28U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_29
\details
The function \b SPI_Data_Bits_29 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>29 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_29 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 28)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 29U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 29U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 29U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_30
\details
The function \b SPI_Data_Bits_30 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>30 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_30 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 29)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 30U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 30U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 30U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_31
\details
The function \b SPI_Data_Bits_31 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>31 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Data_Bits_31 (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1U << 30)) == 0U) {       // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 31U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 31U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 31U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Data_Bits_32
\details
The function \b SPI_Data_Bits_32 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with <b>32 data bits</b> per frame
 - with default bit order
 - at default bus speed
 - for default number of data items
*/
void SPI_Data_Bits_32 (void) {

  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
#if (SPI_SERVER_USED != 0)
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.db_mask & (1UL << 31)) == 0U) {      // If SPI Server does not support data bits setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 32U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 32U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 32U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Bit_Order_MSB_LSB
\details
The function \b SPI_Bit_Order_MSB_LSB verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with default data bits
 - with bit order <b>from MSB to LSB</b>
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Bit_Order_MSB_LSB (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.bo_mask & 1U) == 0U) {      // If SPI Server does not support bit order setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Bit_Order_LSB_MSB
\details
The function \b SPI_Bit_Order_LSB_MSB verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with default data bits
 - with bit order <b>from LSB to MSB</b>
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is skipped
*/
void SPI_Bit_Order_LSB_MSB (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.bo_mask & 2U) == 0U) {      // If SPI Server does not support bit order setting
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_LSB_TO_MSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_LSB_TO_MSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_LSB_TO_MSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Bus_Speed_Min
\details
The function \b SPI_Bus_Speed_Min verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at <b>minimum bus speed</b> (define <c>SPI_CFG_MIN_BUS_SPEED</c> in DV_SPI_Config.h)
 - for default number of data items

This test function checks the following requirements:
 - measured bus speed is not 25% lower, or higher than requested
 - bus speed value returned by the driver is not negative
 - bus speed value returned by the driver is not higher then requested
 - bus speed value returned by the driver is not lower then 75% of requested
*/
void SPI_Bus_Speed_Min (void) {
  volatile uint64_t bps;
  volatile  int32_t got_bus_speed;

  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
#if (SPI_SERVER_USED != 0)
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if (spi_serv_cap.bs_min > SPI_CFG_MIN_BUS_SPEED) {    // If SPI Server does not support minimum bus speed
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_MIN_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_MIN_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_MIN_BUS_SPEED, SPI_CFG_DEF_NUM);

  if (duration != 0xFFFFFFFFU) {        // If Transfer finished before timeout
    if (duration != 0U) {               // If duration of transfer was more than 0 SysTick counts
      bps = ((uint64_t)systick_freq * SPI_CFG_DEF_DATA_BITS * SPI_CFG_DEF_NUM) / duration;
      if ((bps < ((SPI_CFG_MIN_BUS_SPEED * 3) / 4)) ||
          (bps >   SPI_CFG_MIN_BUS_SPEED)) {
        // If measured bus speed is 25% lower, or higher than requested
        (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] At requested bus speed of %i bps, effective bus speed is %i bps", SPI_CFG_MIN_BUS_SPEED, (uint32_t)bps);
        TEST_MESSAGE(msg_buf);
      }
    }
  }

  drv->Control (ARM_SPI_SET_BUS_SPEED, SPI_CFG_MIN_BUS_SPEED);
  got_bus_speed = drv->Control (ARM_SPI_GET_BUS_SPEED, 0U);
  if (got_bus_speed < 0) {
    // If bus speed value returned by the driver is negative
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Get bus speed returned negative value %i", got_bus_speed);
    TEST_FAIL_MESSAGE(msg_buf);
  } else if ((uint32_t)got_bus_speed > SPI_CFG_MIN_BUS_SPEED) {
    // If bus speed value returned by the driver is higher then requested
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Get bus speed returned %i bps instead of requested %i bps", got_bus_speed, SPI_CFG_MIN_BUS_SPEED);
    TEST_FAIL_MESSAGE(msg_buf);
  } else if ((uint32_t)got_bus_speed < ((SPI_CFG_MIN_BUS_SPEED * 3) / 4)) {
    // If bus speed value returned by the driver is lower then 75% of requested
    (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] Get bus speed returned %i bps instead of requested %i bps", got_bus_speed, SPI_CFG_MIN_BUS_SPEED);
    TEST_MESSAGE(msg_buf);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Bus_Speed_Max
\details
The function \b SPI_Bus_Speed_Max verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at <b>maximum bus speed</b> (define <c>SPI_CFG_MAX_BUS_SPEED</c> in DV_SPI_Config.h)
 - for default number of data items

This test function checks the following requirements:
 - measured bus speed is not 25% lower, or higher than requested
 - bus speed value returned by the driver is not negative
 - bus speed value returned by the driver is not higher then requested
 - bus speed value returned by the driver is not lower then 75% of requested
*/
void SPI_Bus_Speed_Max (void) {
  volatile uint64_t bps;
  volatile  int32_t got_bus_speed;

  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
#if (SPI_SERVER_USED != 0)
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if (spi_serv_cap.bs_max < SPI_CFG_MAX_BUS_SPEED) {    // If SPI Server does not support maximum bus speed
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_MAX_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_MAX_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_MAX_BUS_SPEED, SPI_CFG_DEF_NUM);

  if (duration != 0xFFFFFFFFU) {        // If Transfer finished before timeout
    if (duration != 0U) {               // If duration of transfer was more than 0 SysTick counts
      bps = ((uint64_t)systick_freq * SPI_CFG_DEF_DATA_BITS * SPI_CFG_DEF_NUM) / duration;
      if ((bps < ((SPI_CFG_MAX_BUS_SPEED * 3) / 4)) ||
          (bps >   SPI_CFG_MIN_BUS_SPEED)) {
        // If measured bus speed is 25% lower, or higher than requested
        (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] At requested bus speed of %i bps, effective bus speed is %i bps", SPI_CFG_MAX_BUS_SPEED, (uint32_t)bps);
        TEST_MESSAGE(msg_buf);
      }
    }
  }

  drv->Control (ARM_SPI_SET_BUS_SPEED, SPI_CFG_MAX_BUS_SPEED);
  got_bus_speed = drv->Control (ARM_SPI_GET_BUS_SPEED, 0U);
  if (got_bus_speed < 0) {
    // If bus speed value returned by the driver is negative
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Get bus speed returned negative value %i", got_bus_speed);
    TEST_FAIL_MESSAGE(msg_buf);
  } else if ((uint32_t)got_bus_speed > SPI_CFG_MAX_BUS_SPEED) {
    // If bus speed value returned by the driver is higher then requested
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Get bus speed returned %i bps instead of requested %i bps", got_bus_speed, SPI_CFG_MAX_BUS_SPEED);
    TEST_FAIL_MESSAGE(msg_buf);
  } else if ((uint32_t)got_bus_speed < ((SPI_CFG_MAX_BUS_SPEED * 3) / 4)) {
    // If bus speed value returned by the driver is lower then 75% of requested
    (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] Get bus speed returned %i bps instead of requested %i bps", got_bus_speed, SPI_CFG_MAX_BUS_SPEED);
    TEST_MESSAGE(msg_buf);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Number_Of_Items
\details
The function \b SPI_Number_Of_Items verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for <b>different number of items</b> (defines <c>SPI_CFG_NUM1 .. SPI_CFG_NUM5</c> in DV_SPI_Config.h)
*/
void SPI_Number_Of_Items (void) {

  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
#if (SPI_SERVER_USED != 0)
  if (CheckServer()  != EXIT_SUCCESS) { return; }
  if ((spi_serv_cap.mode_mask & 2U) == 0U) {    // If SPI Server does not support Slave mode
    TEST_MESSAGE("[FAILED] Test not supported by SPI Server! Test aborted!");
    return;
  }
#endif

#if (SPI_CFG_NUM1 != 0U)
  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM1);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM1);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM1);
#endif

#if (SPI_CFG_NUM2 != 0U)
  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM2);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM2);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM2);
#endif

#if (SPI_CFG_NUM3 != 0U)
  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM3);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM3);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM3);
#endif

#if (SPI_CFG_NUM4 != 0U)
  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM4);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM4);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM4);
#endif

#if (SPI_CFG_NUM5 != 0U)
  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM5);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM5);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM5);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Abort
\details
The function \b SPI_Abort verifies data exchange Abort:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
*/
void SPI_Abort (void) {

  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
#if (SPI_SERVER_USED != 0)
  if (CheckServer()  != EXIT_SUCCESS) { return; }
#endif

  SPI_DataExchange_Operation(OP_ABORT_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_ABORT_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_ABORT_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/**
@}
*/
// End of spi_tests_data_xchg

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* SPI Error Event tests                                                                                                    */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup spi_tests_err_evt Error Event
\ingroup spi_tests
\details
These tests verify API and operation of the SPI error event signaling.

The error event tests verify the following driver function
(<a href="http://www.keil.com/pack/doc/CMSIS/Driver/html/group__spi__interface__gr.html" target="_blank">SPI Driver function documentation</a>):
 - \b SignalEvent
\code
  void (*ARM_SPI_SignalEvent_t) (uint32_t event);
\endcode

\note In Test Mode <b>Loopback</b> these tests are skipped
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_DataLost
\details
The function \b SPI_DataLost verifies signaling of the <b>ARM_SPI_EVENT_DATA_LOST</b> event:
 - in <b>Slave Mode</b> with <b>Slave Select line Hardware Monitored</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
*/
void SPI_DataLost (void) {

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdSetCom  (0U, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, 1U, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { break; }
    if (CmdXfer    (SPI_CFG_DEF_NUM, 20U, SPI_CFG_XFER_TIMEOUT, 0U) != EXIT_SUCCESS) { break; }

    (void)drv->Control (ARM_SPI_MODE_SLAVE                                                                 | 
                      ((SPI_CFG_DEF_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos)   & ARM_SPI_FRAME_FORMAT_Msk)   | 
                      ((SPI_CFG_DEF_DATA_BITS << ARM_SPI_DATA_BITS_Pos)      & ARM_SPI_DATA_BITS_Msk)      | 
                      ((SPI_CFG_DEF_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)      & ARM_SPI_BIT_ORDER_Msk)      | 
                        ARM_SPI_SS_SLAVE_HW                                                                , 
                        SPI_CFG_DEF_BUS_SPEED);

    event = 0U;
    (void)osDelay(SPI_CFG_XFER_TIMEOUT+20U);    // Wait for SPI Server to timeout

    // Assert that event ARM_SPI_EVENT_DATA_LOST was signaled
    TEST_ASSERT_MESSAGE((event & ARM_SPI_EVENT_DATA_LOST) != 0U, "[FAILED] Event ARM_SPI_EVENT_DATA_LOST was not signaled!");

    return;
  } while (false);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_ModeFault
\details
The function \b SPI_ModeFault verifies signaling of the <b>ARM_SPI_EVENT_MODE_FAULT</b> event:
 - in <b>Master Mode</b> with <b>Slave Select line Hardware Monitored Input</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
*/
void SPI_ModeFault (void) {

  if (drv->GetCapabilities().event_mode_fault == 0U) {
    TEST_MESSAGE("[WARNING] This driver does not support ARM_SPI_EVENT_MODE_FAULT event signaling! Test skipped!");
    return;
  }

#if (SPI_SERVER_USED != 0)
  if (InitDriver()   != EXIT_SUCCESS) { return; }
  if (CheckBuffers() != EXIT_SUCCESS) { return; }
  if (CheckServer()  != EXIT_SUCCESS) { return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdSetCom  (1U, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, 0U, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { break; }
    if (CmdXfer    (SPI_CFG_DEF_NUM, 0U, SPI_CFG_XFER_TIMEOUT, SPI_CFG_DEF_NUM / 2U) != EXIT_SUCCESS) { break; }

    (void)drv->Control (ARM_SPI_MODE_MASTER                                                              | 
                      ((SPI_CFG_DEF_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos)   & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((SPI_CFG_DEF_DATA_BITS << ARM_SPI_DATA_BITS_Pos)      & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_CFG_DEF_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)      & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_MASTER_HW_INPUT                                                       , 
                        SPI_CFG_DEF_BUS_SPEED);

    event = 0U;
    (void)osDelay(10U);
    TEST_ASSERT(drv->Transfer(ptr_tx_buf, ptr_rx_buf, SPI_CFG_DEF_NUM) == ARM_DRIVER_OK);

    (void)osDelay(SPI_CFG_XFER_TIMEOUT+10U);    // Wait for SPI Server to timeout

    // Assert that event ARM_SPI_EVENT_MODE_FAULT was signaled
    TEST_ASSERT_MESSAGE((event & ARM_SPI_EVENT_MODE_FAULT) != 0U, "[FAILED] Event ARM_SPI_EVENT_MODE_FAULT was not signaled!");

    return;
  } while (false);

#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test skipped!");
#endif
}

/**
@}
*/
// End of spi_tests_err_evt
