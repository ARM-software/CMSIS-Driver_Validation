/*
 * Copyright (c) 2015-2022 Arm Limited. All rights reserved.
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

// Fixed settings for communication with SPI Server (not available through DV_SPI_Config.h)
#define SPI_CFG_SRV_FORMAT        0     // Clock Polarity 0 / Clock Phase 0
#define SPI_CFG_SRV_DATA_BITS     8     // 8 data bits
#define SPI_CFG_SRV_BIT_ORDER     0     // MSB to LSB bit order

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
#define SS_MODE_MASTER_SW         1UL   // Master mode Slave Select Software controlled
#define SS_MODE_MASTER_HW_OUTPUT  2UL   // Master mode Slave Select Hardware controlled Output
#define SS_MODE_MASTER_HW_INPUT   3UL   // Master mode Slave Select Hardware monitored Input
#define SS_MODE_SLAVE_HW          0UL   // Slave mode Slave Select Hardware monitored
#define SS_MODE_SLAVE_SW          1UL   // Slave mode Slave Select Software controlled
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
#error  Default number of items must not be 0!
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
static int8_t                   driver_ok;
static int8_t                   server_ok;

static SPI_SERV_VER_t           spi_serv_ver;
static SPI_SERV_CAP_t           spi_serv_cap;

static volatile uint32_t        event;
static volatile uint32_t        duration;
static volatile uint32_t        xfer_count;
static volatile uint32_t        data_count_sample;
static uint32_t                 systick_freq;

static osEventFlagsId_t         event_flags;

static char                     msg_buf[256];

// Allocated buffer pointers
static void                    *ptr_tx_buf_alloc;
static void                    *ptr_rx_buf_alloc;
static void                    *ptr_cmp_buf_alloc;

// Buffer pointers used for data transfers (must be aligned to 4 byte)
static uint8_t                 *ptr_tx_buf;
static uint8_t                 *ptr_rx_buf;
static uint8_t                 *ptr_cmp_buf;

// String representation of various codes
static const char *str_srv_status[] = {
  "Ok",
  "Failed"
};

static const char *str_test_mode[] = {
  "Loopback",
  "SPI Server"
};

static const char *str_oper[] = {
  "Send    ",
  "Receive ",
  "Transfer",
  "Abort Send    ",
  "Abort Receive ",
  "Abort Transfer"
};

static const char *str_ss_mode[] = {
  "Unused",
  "Software controlled",
  "Hardware controlled"
};

static const char *str_mode[] = {
  "Master",
  "Slave"
};

static const char *str_format[] = {
  "Clock Polarity 0, Clock Phase 0",
  "Clock Polarity 0, Clock Phase 1",
  "Clock Polarity 1, Clock Phase 0",
  "Clock Polarity 1, Clock Phase 1",
  "Texas Instruments",
  "National Semiconductor Microwire"
};

static const char *str_bit_order[] = {
  "MSB to LSB",
  "LSB to MSB"
};

static const char *str_ret[] = {
  "ARM_DRIVER_OK",
  "ARM_DRIVER_ERROR",
  "ARM_DRIVER_ERROR_BUSY",
  "ARM_DRIVER_ERROR_TIMEOUT",
  "ARM_DRIVER_ERROR_UNSUPPORTED",
  "ARM_DRIVER_ERROR_PARAMETER",
  "ARM_DRIVER_ERROR_SPECIFIC",
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
static int32_t  CmdXfer                (uint32_t num,  uint32_t delay_c, uint32_t delay_t,  uint32_t timeout);
static int32_t  CmdGetCnt              (void);

static int32_t  ServerInit             (void);
static int32_t  ServerCheck            (void);
static int32_t  ServerCheckSupport     (uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t bus_speed);
#endif

static int32_t  IsNotLoopback          (void);
static int32_t  IsNotFrameTI           (void);
static int32_t  IsNotFrameMw           (void);
static int32_t  IsFormatValid          (void);
static int32_t  IsBitOrderValid        (void);

static uint32_t DataBitsToBytes        (uint32_t data_bits);
static int32_t  DriverInit             (void);
static int32_t  BuffersCheck           (void);

static void SPI_DataExchange_Operation (uint32_t operation, uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t ss_mode, uint32_t bus_speed, uint32_t num);

// Helper functions

/*
  \fn            void SPI_DrvEvent (uint32_t evt)
  \brief         Store event(s) into a global variable.
  \detail        This is a callback function called by the driver upon an event(s).
  \param[in]     evt            SPI event
  \return        none
*/
static void SPI_DrvEvent (uint32_t evt) {
  event |= evt;

  (void)osEventFlagsSet(event_flags, evt);
}

/*
  \fn            static uint32_t DataBitsToBytes (uint32_t data_bits)
  \brief         Calculate number of bytes used for an item at required data bits.
  \return        number of bytes per item
*/
static uint32_t DataBitsToBytes (uint32_t data_bits) {
  uint32_t ret;

  ret = 1U;
  if        (data_bits > 16U) {
    ret = 4U;
  } else if (data_bits > 8U) {
    ret = 2U;
  }

  return ret;
}

/*
  \fn            static int32_t DriverInit (void)
  \brief         Initialize and power-on the driver.
  \return        execution status
                   - EXIT_SUCCESS: Driver initialized and powered-up successfully
                   - EXIT_FAILURE: Driver initialization or power-up failed
*/
static int32_t DriverInit (void) {

  if (drv->Initialize    (SPI_DrvEvent)   == ARM_DRIVER_OK) {
    if (drv->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK) {
      return EXIT_SUCCESS;
    }
  }

  TEST_FAIL_MESSAGE("[FAILED] SPI driver initialize or power-up. Check driver Initialize and PowerControl functions! Test aborted!");

  return EXIT_FAILURE;
}

/*
  \fn            static int32_t BuffersCheck (void)
  \brief         Check if buffers are valid.
  \return        execution status
                   - EXIT_SUCCESS: Buffers are valid
                   - EXIT_FAILURE: Buffers are not valid
*/
static int32_t BuffersCheck (void) {

  if ((ptr_tx_buf  != NULL) &&
      (ptr_rx_buf  != NULL) && 
      (ptr_cmp_buf != NULL)) {
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
                   - EXIT_SUCCESS: Default configuration set successfully
                   - EXIT_FAILURE: Default configuration failed
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
    TEST_FAIL_MESSAGE("[FAILED] Configure communication interface to SPI Server default settings. Check driver Control function! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t ComSendCommand (const void *data_out, uint32_t num)
  \brief         Send command to SPI Server.
  \param[out]    data_out       Pointer to memory containing data to be sent
  \param[in]     len            Number of bytes to be sent
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t ComSendCommand (const void *data_out, uint32_t len) {
   int32_t ret;
  uint32_t flags, num, tout;

  ret = EXIT_SUCCESS;
  num = (len + DataBitsToBytes(SPI_CFG_SRV_DATA_BITS) - 1U) / DataBitsToBytes(SPI_CFG_SRV_DATA_BITS);

  ret = ComConfigDefault();

  if (ret == EXIT_SUCCESS) {
    if (SPI_CFG_SRV_SS_MODE == SS_MODE_MASTER_SW) {
      if (drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE) != ARM_DRIVER_OK) {
        ret = EXIT_FAILURE;
      }
    }
    if (ret == EXIT_SUCCESS) {
      (void)osEventFlagsClear(event_flags, 0x7FFFFFFFU); 	
      if (drv->Send(data_out, num) == ARM_DRIVER_OK) {
        flags = osEventFlagsWait(event_flags, ARM_SPI_EVENT_TRANSFER_COMPLETE, osFlagsWaitAny, SPI_CFG_SRV_CMD_TOUT);
        if (((flags & 0x80000000U) != 0U) ||
            ((flags & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U)) {
          ret = EXIT_FAILURE;
          (void)drv->Control (ARM_SPI_ABORT_TRANSFER, 0U);
        }
      }
    }
    if (SPI_CFG_SRV_SS_MODE == SS_MODE_MASTER_SW) {
      if (drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE) != ARM_DRIVER_OK) {
        ret = EXIT_FAILURE;
      }
    }
  }
  (void)drv->Control(ARM_SPI_MODE_INACTIVE, 0U);

  return ret;
}

/**
  \fn            static int32_t ComReceiveResponse (void *data_in, uint32_t num)
  \brief         Receive response from SPI Server.
  \param[out]    data_in     Pointer to memory where data will be received
  \param[in]     len         Number of data bytes to be received
  \return        execution status
                   - EXIT_SUCCESS: Command received successfully
                   - EXIT_FAILURE: Command reception failed
*/
static int32_t ComReceiveResponse (void *data_in, uint32_t len) {
   int32_t ret;
  uint32_t flags, num, tout;

  ret = EXIT_SUCCESS;
  num = (len + DataBitsToBytes(SPI_CFG_SRV_DATA_BITS) - 1U) / DataBitsToBytes(SPI_CFG_SRV_DATA_BITS);

  ret = ComConfigDefault();

  if (ret == EXIT_SUCCESS) {
    if (SPI_CFG_SRV_SS_MODE == SS_MODE_MASTER_SW) {
      if (drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE) != ARM_DRIVER_OK) {
        ret = EXIT_FAILURE;
      }
    }
    if (ret == EXIT_SUCCESS) {
      (void)osEventFlagsClear(event_flags, 0x7FFFFFFFU); 	
      if (drv->Receive(data_in, num) == ARM_DRIVER_OK) {
        flags = osEventFlagsWait(event_flags, ARM_SPI_EVENT_TRANSFER_COMPLETE, osFlagsWaitAny, SPI_CFG_SRV_CMD_TOUT);
        if (((flags & 0x80000000U) != 0U) ||
            ((flags & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U)) {
          ret = EXIT_FAILURE;
          (void)drv->Control (ARM_SPI_ABORT_TRANSFER, 0U);
        }
      }
    }
    if (SPI_CFG_SRV_SS_MODE == SS_MODE_MASTER_SW) {
      if (drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE) != ARM_DRIVER_OK) {
        ret = EXIT_FAILURE;
      }
    }
  }
  (void)drv->Control(ARM_SPI_MODE_INACTIVE, 0U);

  return ret;
}

/**
  \fn            static int32_t CmdGetVer (void)
  \brief         Get version from SPI Server and check that it is valid.
  \return        execution status
                   - EXIT_SUCCESS: Version retrieved successfully
                   - EXIT_FAILURE: Version retreival failed
*/
static int32_t CmdGetVer (void) {
  int32_t     ret;
  const char *ptr_str;
  uint16_t    val16;
  uint8_t     val8;

  ptr_str = NULL;

  memset(&spi_serv_ver, 0, sizeof(spi_serv_ver));

  // Send "GET VER" command to SPI Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  memcpy(ptr_tx_buf, "GET VER", 7);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
  (void)osDelay(10U);

  if (ret == EXIT_SUCCESS) {
    // Receive response to "GET VER" command from SPI Server
    memset(ptr_rx_buf, (int32_t)'?', RESP_GET_VER_LEN);
    ret = ComReceiveResponse(ptr_rx_buf, RESP_GET_VER_LEN);
    (void)osDelay(10U);
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

  return ret;
}

/**
  \fn            static int32_t CmdGetCap (void)
  \brief         Get capabilities from SPI Server.
  \return        execution status
                   - EXIT_SUCCESS: Capabilities retrieved successfully
                   - EXIT_FAILURE: Capabilities retreival failed
*/
static int32_t CmdGetCap (void) {
  int32_t     ret;
  const char *ptr_str;
  uint32_t    val32;
  uint8_t     val8;

  ptr_str = NULL;

  memset(&spi_serv_cap, 0, sizeof(spi_serv_cap));

  // Send "GET CAP" command to SPI Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  memcpy(ptr_tx_buf, "GET CAP", 7);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
  (void)osDelay(10U);

  if (ret == EXIT_SUCCESS) {
    (void)osDelay(20U);                 // Give SPI Server 20 ms to auto-detect capabilities

    // Receive response to "GET CAP" command from SPI Server
    memset(ptr_rx_buf, (int32_t)'?', RESP_GET_CAP_LEN);
    ret = ComReceiveResponse(ptr_rx_buf, RESP_GET_CAP_LEN);
    (void)osDelay(10U);
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

  return ret;
}

/**
  \fn            static int32_t CmdSetBufTx (char pattern)
  \brief         Set Tx buffer of SPI Server to pattern.
  \param[in]     pattern        Pattern to fill the buffer with
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t CmdSetBufTx (char pattern) {
  int32_t ret;

  // Send "SET BUF TX" command to SPI Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "SET BUF TX,0,%02X", (int32_t)pattern);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
  (void)osDelay(10U);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set Tx buffer on SPI Server. Check SPI Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdSetBufRx (char pattern)
  \brief         Set Rx buffer of SPI Server to pattern.
  \param[in]     pattern        Pattern to fill the buffer with
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t CmdSetBufRx (char pattern) {
  int32_t ret;

  // Send "SET BUF RX" command to SPI Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "SET BUF RX,0,%02X", (int32_t)pattern);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
  (void)osDelay(10U);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set Rx buffer on SPI Server. Check SPI Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdGetBufRx (uint32_t len)
  \brief         Get Rx buffer from SPI Server (into global array pointed to by ptr_rx_buf).
  \param[in]     len            Number of bytes to read from Rx buffer
  \return        execution status
                   - EXIT_SUCCESS: Command sent and response received successfully
                   - EXIT_FAILURE: Command send or response reception failed
*/
static int32_t CmdGetBufRx (uint32_t len) {
  int32_t ret;

  // Send "GET BUF RX" command to SPI Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "GET BUF RX,%i", len);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
  (void)osDelay(10U);

  if (ret == EXIT_SUCCESS) {
    // Receive response to "GET BUF RX" command from SPI Server
    memset(ptr_rx_buf, (int32_t)'?', len);
    ret = ComReceiveResponse(ptr_rx_buf, len);
    (void)osDelay(10U);
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Get Rx buffer from SPI Server. Check SPI Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdSetCom (uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t ss_mode, uint32_t bus_speed)
  \brief         Set communication parameters on SPI Server for next XFER command.
  \param[in]     mode           mode (0 = Master, 1 = slave)
  \param[in]     format         clock / frame format:
                                  - value 0 = clock polarity 0, phase 0
                                  - value 1 = clock polarity 0, phase 1
                                  - value 2 = clock polarity 1, phase 0
                                  - value 3 = clock polarity 1, phase 1
                                  - value 4 = Texas Instruments frame format
                                  - value 5 = Microwire frame format
  \param[in]     data_bits      data bits
                                  - values 1 to 32
  \param[in]     bit_order      bit order
                                  - value 0 = MSB to LSB
                                  - value 1 = LSB to MSB
  \param[in]     ss_mode        Slave Select mode:
                                  - value 0 = not used
                                  - value 1 = used (in Master mode driven, in Slave mode monitored as hw input)
  \param[in]     bus_speed      bus speed in bits per second (bps)
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t CmdSetCom (uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t ss_mode, uint32_t bus_speed) {
  int32_t ret, stat;

  // Send "SET COM" command to SPI Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  stat = snprintf((char *)ptr_tx_buf, CMD_LEN, "SET COM %i,%i,%i,%i,%i,%i", mode, format, data_bits, bit_order, ss_mode, bus_speed);
  if ((stat > 0) && (stat < CMD_LEN)) {
    ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
    (void)osDelay(10U);
  } else {
    ret = EXIT_FAILURE;
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set communication settings on SPI Server. Check SPI Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdXfer (uint32_t num, uint32_t delay_c, uint32_t delay_t, uint32_t timeout)
  \brief         Activate transfer on SPI Server.
  \param[in]     num            number of items (according CMSIS SPI driver specification)
  \param[in]     delay_c        delay before control function is called, in milliseconds
                                (0xFFFFFFFF = delay not used)
  \param[in]     delay_t        delay after control function is called but before transfer function is called, in milliseconds
                                (0xFFFFFFFF = delay not used)
  \param[in]     timeout        timeout in milliseconds, after delay, if delay is specified
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t CmdXfer (uint32_t num, uint32_t delay_c, uint32_t delay_t, uint32_t timeout) {
  int32_t ret;

  // Send "XFER" command to SPI Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  if        ((delay_c != osWaitForever) && (delay_t != osWaitForever) && (timeout != 0U)) {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i,%i,%i,%i",    num, delay_c, delay_t, timeout);
  } else if ((delay_c != osWaitForever) && (delay_t != osWaitForever)) {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i,%i,%i",       num, delay_c, delay_t);
  } else if  (delay_c != osWaitForever)                                {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i,%i",          num, delay_c);
  } else {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i",             num);
  }
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Activate transfer on SPI Server. Check SPI Server! Test aborted!");
  }

  return ret;
}

/*
  \fn            static int32_t CmdGetCnt (void)
  \brief         Get XFER command Tx/Rx count from SPI Server.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CmdGetCnt (void) {
  int32_t     ret;
  const char *ptr_str;
  uint32_t    val32;

  xfer_count = 0U;

  // Send "GET CNT" command to SPI Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  memcpy(ptr_tx_buf, "GET CNT", 7);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
  (void)osDelay(10U);

  if (ret == EXIT_SUCCESS) {
    // Receive response to "GET CNT" command from SPI Server
    memset(ptr_rx_buf, (int32_t)'?', RESP_GET_CNT_LEN);
    ret = ComReceiveResponse(ptr_rx_buf, RESP_GET_CNT_LEN);
    (void)osDelay(10U);
  }

  if (ret == EXIT_SUCCESS) {
    // Parse count
    ptr_str = (const char *)ptr_rx_buf;
    if (sscanf(ptr_str, "%i", &val32) == 1) {
      xfer_count = val32;
    } else {
      ret = EXIT_FAILURE;
    }
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Get count from SPI Server. Check SPI Server! Test aborted!");
  }

  return ret;
}

/*
  \fn            static int32_t ServerInit (void)
  \brief         Initialize communication with SPI Server, get version and capabilities.
  \return        execution status
                   - EXIT_SUCCESS: SPI Server initialized successfully
                   - EXIT_FAILURE: SPI Server initialization failed
*/
static int32_t ServerInit (void) {

  if (server_ok == -1) {                // If -1, means it was not yet checked
    server_ok = 1;

    if (drv->Control(ARM_SPI_MODE_MASTER                                                                | 
                   ((SPI_CFG_SRV_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos)   & ARM_SPI_FRAME_FORMAT_Msk)   | 
                   ((SPI_CFG_SRV_DATA_BITS << ARM_SPI_DATA_BITS_Pos)      & ARM_SPI_DATA_BITS_Msk)      | 
                   ((SPI_CFG_SRV_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)      & ARM_SPI_BIT_ORDER_Msk)      | 
                   ((SPI_CFG_SRV_SS_MODE   << ARM_SPI_SS_MASTER_MODE_Pos) & ARM_SPI_SS_MASTER_MODE_Msk) , 
                     SPI_CFG_SRV_BUS_SPEED) != ARM_DRIVER_OK) {
      server_ok = 0;
    }
    if ((server_ok == 1) && (SPI_CFG_SRV_SS_MODE == SS_MODE_MASTER_SW)) {
      if (drv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_INACTIVE) != ARM_DRIVER_OK) {
        server_ok = 0;
      }
    }
    if (server_ok == 0) {
      TEST_GROUP_INFO("Failed to configure communication interface to SPI Server default settings.\n"\
                      "Driver must support basic settings used for communication with SPI Server!");
    }

    if (server_ok == 1) {
      (void)osDelay(10U);
      if (CmdGetVer() != EXIT_SUCCESS) {
        TEST_GROUP_INFO("Failed to Get version from SPI Server.\nCheck SPI Server!\n");
        server_ok = 0;
      }
    }

    if (server_ok == 1) {
      if ((spi_serv_ver.major <= 1U) && (spi_serv_ver.minor < 1U)) { 
        TEST_GROUP_INFO("SPI Server version must be 1.1.0. or higher.\nUpdate SPI Server to newer version!\n");
        server_ok = 0;
      }
    }

    if (server_ok == 1) {
      if (CmdGetCap() != EXIT_SUCCESS) {
        TEST_GROUP_INFO("Failed to Get capabilities from SPI Server.\nCheck SPI Server!\n");
        server_ok = 0;
      }
    }
  }

  if (server_ok == 1) {
    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

/*
  \fn            static int32_t ServerCheck (void)
  \brief         Check if communication with SPI Server is working.
  \return        execution status
                   - EXIT_SUCCESS: If SPI Server status is ok
                   - EXIT_FAILURE: If SPI Server status is fail
*/
static int32_t ServerCheck (void) {

  if (server_ok == 1) {
    return EXIT_SUCCESS;
  }

  TEST_FAIL_MESSAGE("[FAILED] SPI Server status. Check SPI Server! Test aborted!");
  return EXIT_FAILURE;
}

/*
  \fn            static int32_t ServerCheckSupport (uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t bus_speed)
  \brief         Check if SPI Server supports desired settings.
  \param[in]     mode           mode (0 = Master, 1 = slave)
  \param[in]     format         clock / frame format:
                                  - value 0 = clock polarity 0, phase 0
                                  - value 1 = clock polarity 0, phase 1
                                  - value 2 = clock polarity 1, phase 0
                                  - value 3 = clock polarity 1, phase 1
                                  - value 4 = Texas Instruments frame format
                                  - value 5 = Microwire frame format
  \param[in]     data_bits      data bits
                                  - values 1 to 32
  \param[in]     bit_order      bit order
                                  - value 0 = MSB to LSB
                                  - value 1 = LSB to MSB
  \param[in]     bus_speed      bus speed in bits per second (bps)
  \return        execution status
                   - EXIT_SUCCESS: SPI Server supports desired settings
                   - EXIT_FAILURE: SPI Server does not support desired settings
*/
static int32_t ServerCheckSupport (uint32_t mode, uint32_t format, uint32_t data_bits, uint32_t bit_order, uint32_t bus_speed) {

  if ((spi_serv_cap.mode_mask & (1UL << (mode - 1U))) == 0U) {
    // If SPI Server does not support desired mode
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] SPI Server does not support %s mode! Test aborted!", str_mode[mode - 1U]);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }
  if ((spi_serv_cap.fmt_mask & (1UL << format)) == 0U) {
    // If SPI Server does not support desired clock / frame format
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] SPI Server does not support %s clock / frame format! Test aborted!", str_format[format]);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }
  if ((spi_serv_cap.db_mask & (1UL << (data_bits - 1U))) == 0U) {
    // If SPI Server does not support desired data bits
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] SPI Server does not support %i data bits! Test aborted!", data_bits);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }
  if ((spi_serv_cap.bo_mask & (1UL << bit_order)) == 0U) {
    // If SPI Server does not support desired bit order
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] SPI Server does not support %s bit order! Test aborted!", str_bit_order[bit_order]);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }
  if ((spi_serv_cap.bs_min > bus_speed) ||
      (spi_serv_cap.bs_max < bus_speed)) {
    // If SPI Server does not support desired bus speed
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] SPI Server does not support %i bps bus speed! Test aborted!", bus_speed);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#endif                                  // If Test Mode SPI Server is selected

/*
  \fn            static int32_t IsNotLoopback (void)
  \brief         Check if loopback is not selected.
  \detail        This function is used to skip executing a test if it is not supported 
                 in Loopback mode.
  \return        execution status
                   - EXIT_SUCCESS: Loopback is not selected
                   - EXIT_FAILURE: Loopback is selected
*/
static int32_t IsNotLoopback (void) {

#if (SPI_SERVER_USED == 1)
  return EXIT_SUCCESS;
#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test not executed!");
  return EXIT_FAILURE;
#endif
}

/*
  \fn            static int32_t IsNotFrameTI (void)
  \brief         Check if Default Clock / Frame Format selected is not Texas Instruments.
  \detail        This function is used to skip executing a test if it is not supported 
                 for Texas Instruments Frame Format.
  \return        execution status
                   - EXIT_SUCCESS: Texas Instruments Frame Format is not selected
                   - EXIT_FAILURE: Texas Instruments Frame Format is selected
*/
static int32_t IsNotFrameTI (void) {

#if (SPI_CFG_DEF_FORMAT != FORMAT_TI)
  return EXIT_SUCCESS;
#else
  TEST_MESSAGE("[WARNING] Test not supported for Texas Instruments Frame Format! Test not executed!");
  return EXIT_FAILURE;
#endif
}

/*
  \fn            static int32_t IsNotFrameMw (void)
  \brief         Check if Default Clock / Frame Format selected is not National Semiconductor Microwire.
  \detail        This function is used to skip executing a test if it is not supported 
                 for National Semiconductor Microwire Frame Format.
  \return        execution status
                   - EXIT_SUCCESS: National Semiconductor Microwire Frame Format is not selected
                   - EXIT_FAILURE: National Semiconductor Microwire Frame Format is selected
*/
static int32_t IsNotFrameMw (void) {

#if (SPI_CFG_DEF_FORMAT != FORMAT_MICROWIRE)
  return EXIT_SUCCESS;
#else
  TEST_MESSAGE("[WARNING] Test not supported for National Semiconductor Microwire Frame Format! Test not executed!");
  return EXIT_FAILURE;
#endif
}

/*
  \fn            static int32_t IsFormatValid (void)
  \brief         Check if default format settings are valid.
  \detail        This function is used to abort executing a test if Default settings 
                 specify TI or Microwire Frame Format and Slave Select handling 
                 other than Hardware controlled. 
  \return        execution status
                   - EXIT_SUCCESS: Format is valid
                   - EXIT_FAILURE: Format is not valid
*/
static int32_t IsFormatValid (void) {

#if   ((SPI_CFG_DEF_FORMAT == FORMAT_TI) && (SPI_CFG_DEF_SS_MODE != SS_MODE_MASTER_HW_OUTPUT))
  TEST_MESSAGE("[WARNING] TI Frame Format works only with Hardware controlled Slave Select! Test not executed!");
  return EXIT_FAILURE;
#elif ((SPI_CFG_DEF_FORMAT == FORMAT_MICROWIRE) && (SPI_CFG_DEF_SS_MODE != SS_MODE_MASTER_HW_OUTPUT))
  TEST_MESSAGE("[WARNING] Microwire Frame Format works only with Hardware controlled Slave Select! Test not executed!");
  return EXIT_FAILURE;
#else
  return EXIT_SUCCESS;
#endif
}

/*
  \fn            static int32_t IsBitOrderValid (void)
  \brief         Check if default bit order settings valid.
  \detail        This function is used to abort executing a test if Default settings 
                 specify TI or Microwire Frame Format and bit order is not MSB to LSB. 
  \return        execution status
                   - EXIT_SUCCESS: bit order
                   - EXIT_FAILURE: bit order
*/
static int32_t IsBitOrderValid (void) {

#if   ((SPI_CFG_DEF_FORMAT == FORMAT_TI) && (SPI_CFG_DEF_BIT_ORDER != BO_MSB_TO_LSB))
  TEST_MESSAGE("[WARNING] TI Frame Format works only with MSB to LSB bit order! Test not executed!");
  return EXIT_FAILURE;
#elif ((SPI_CFG_DEF_FORMAT == FORMAT_MICROWIRE) && (SPI_CFG_DEF_BIT_ORDER != BO_MSB_TO_LSB))
  TEST_MESSAGE("[WARNING] Microwire Frame Format works only with MSB to LSB bit order! Test not executed!");
  return EXIT_FAILURE;
#else
  return EXIT_SUCCESS;
#endif
}

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
  // (maximum size is incremented by 32 bytes to ensure that buffer can be aligned to 32 bytes)

  ptr_tx_buf_alloc = malloc(SPI_BUF_MAX + 32U);
  if (((uint32_t)ptr_tx_buf_alloc & 31U) != 0U) {
    // If allocated memory is not 32 byte aligned, use next 32 byte aligned address for ptr_tx_buf
    ptr_tx_buf = (uint8_t *)((((uint32_t)ptr_tx_buf_alloc) + 31U) & (~31U));
  } else {
    // If allocated memory is 32 byte aligned, use it directly
    ptr_tx_buf = (uint8_t *)ptr_tx_buf_alloc;
  }
  ptr_rx_buf_alloc = malloc(SPI_BUF_MAX + 32U);
  if (((uint32_t)ptr_rx_buf_alloc & 31U) != 0U) {
    ptr_rx_buf = (uint8_t *)((((uint32_t)ptr_rx_buf_alloc) + 31U) & (~31U));
  } else {
    ptr_rx_buf = (uint8_t *)ptr_rx_buf_alloc;
  }
  ptr_cmp_buf_alloc = malloc(SPI_BUF_MAX + 32U);
  if (((uint32_t)ptr_cmp_buf_alloc & 31U) != 0U) {
    ptr_cmp_buf = (uint8_t *)((((uint32_t)ptr_cmp_buf_alloc) + 31U) & (~31U));
  } else {
    ptr_cmp_buf = (uint8_t *)ptr_cmp_buf_alloc;
  }

  event_flags = osEventFlagsNew(NULL);

  // Output configuration settings
  (void)snprintf(msg_buf, 
                 sizeof(msg_buf),
                 "Test Mode:          %s\n"\
                 "Default settings:\n"\
                 " - Slave Select:    %s\n"\
                 " - Format:          %s\n"\
                 " - Data bits:       %i\n"\
                 " - Bit order:       %s\n"\
                 " - Bus speed:       %i bps\n"\
                 " - Number of Items: %i",
                 str_test_mode[SPI_CFG_TEST_MODE],
                 str_ss_mode  [SPI_CFG_DEF_SS_MODE],
                 str_format   [SPI_CFG_DEF_FORMAT],
                 SPI_CFG_DEF_DATA_BITS,
                 str_bit_order[SPI_CFG_DEF_BIT_ORDER],
                 SPI_CFG_DEF_BUS_SPEED,
                 SPI_CFG_DEF_NUM);
  TEST_GROUP_INFO(msg_buf);

#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected
  // Test communication with SPI Server
  int32_t  server_status;
  uint32_t str_len;

  // Test communication with SPI Server
  if (drv->Initialize    (SPI_DrvEvent)   == ARM_DRIVER_OK) {
    if (drv->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK) {
      server_status = ServerInit();
    }
  }
  (void)drv->PowerControl(ARM_POWER_OFF);
  (void)drv->Uninitialize();

//(void)snprintf(msg_buf, sizeof(msg_buf), "Server status:    %s\n", str_srv_status[server_status]);
//TEST_GROUP_INFO(msg_buf);
#endif
}

/*
  \fn            void SPI_DV_Uninitialize (void)
  \brief         De-initialize testing environment after SPI testing.
  \detail        This function is called by the driver validation framework after SPI testing is finished.
                 It frees memory buffers used for the SPI testing.
  \return        none
*/
void SPI_DV_Uninitialize (void) {

  (void)osEventFlagsDelete(event_flags);

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
- API interface compliance
- Data exchange with various speeds, transfer sizes and communication settings
- Event signaling

Two Test Modes are available: <b>Loopback</b> and <b>SPI Server</b>.

Test Mode : <b>Loopback</b>
---------------------------

This test mode allows only limited validation of the SPI Driver.<br>
It is recommended that this test mode is used only as a proof that driver is 
good enough to be tested with the <b>SPI Server</b>.

For this purpose following <b>Default settings</b> should be used:
 - Slave Select: Not used
 - Clock / Frame Format: Clock Polarity 0, Clock Phase 0
 - Data Bits: 8
 - Bit Order: MSB to LSB
 - Bus Speed: same as setting for the SPI Server
 - Number of Items: 32

To enable this mode of testing in the <b>DV_SPI_Config.h</b> configuration file select the 
<b>Configuration: Test Mode: Loopback</b> setting.

Required pin connection for the <b>Loopback</b> test mode:

\image html spi_loopback_pin_connections.png

\note In this mode following operations / settings cannot be tested:
 - SPI slave mode
 - Slave Select line functionality
 - operation of the Receive function
 - data content sent by the Send function
 - clock / frame format and bit order settings
 - data bit settings other then: 8, 16, 24 and 32
 - event signaling

Test Mode : <b>SPI Server</b>
-----------------------------

This test mode allows extensive validation of the SPI Driver.<br>
Results of the Driver Validation in this test mode are relevant as a proof of driver compliance 
to the CMSIS-Driver specification.

To perform extensive communication tests, it is required to use an 
\ref spi_server "SPI Server" running on a dedicated hardware.

To enable this mode of testing in the <b>DV_SPI_Config.h</b> configuration file select the 
<b>Configuration: Test Mode: SPI Server</b> setting.

Required pin connections for the <b>SPI Server</b> test mode:

\image html spi_server_pin_connections.png

\note Slave Select line has to be pulled to Vcc by an external pull-up (for example 10 kOhm).
\note To ensure proper signal quality:
       - keep the connecting wires as short as possible
       - if possible have SCK and GND wires as a twisted pair and MISO, MOSI and Slave Select 
         wires separate from each other
       - ensure a good Ground (GND) connection between SPI Server and DUT
\note If you experience issues with corrupt data content try reducing bus speed.


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
    - Call Uninitialize function with transfer active and assert that it returned ARM_DRIVER_OK status<br>
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
  // Ensure that SPI Server (if used) is ready for command reception
  (void)osDelay(20U);
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
  // Ensure that SPI Server (if used) is ready for command reception
  (void)osDelay(20U);
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
  - send command "XFER .."       to the SPI Server: Specify transfer
  - driver Control: Configure the SPI interface
  - driver Control: Set the default Tx value
  - driver Send/Receive/Transfer: Start the requested operation
  - driver GetStatus/SignalEvent: Wait for the current operation to finish or time-out<br>
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
  - send command "XFER .."       to the SPI Server: Specify transfer
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
  - driver GetStatus/SignalEvent: Wait for the current operation to finish or time-out<br>
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
  \brief         Execute SPI data exchange or abort operation.
  \param[in]     operation      operation (OP_SEND .. OP_ABORT_TRANSFER)
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
  // in a way that they cannot be seen with a debugger
  volatile  int32_t       stat, def_tx_stat;
  volatile uint32_t       drv_mode, drv_format, drv_data_bits, drv_bit_order, drv_ss_mode;
  volatile uint32_t       srv_mode, srv_ss_mode;
  volatile ARM_SPI_STATUS spi_stat;
  volatile uint32_t       data_count;
           uint32_t       start_cnt;
           uint32_t       val, i;
  volatile uint32_t       srv_delay_c, srv_delay_t;
  volatile uint32_t       drv_delay_c, drv_delay_t;
           uint32_t       timeout, start_tick, curr_tick;
           uint8_t        chk_data;

  // Prepare parameters for SPI Server and Driver configuration
  switch (mode) {
    case MODE_INACTIVE:
      TEST_FAIL_MESSAGE("[FAILED] Inactive mode! Data exchange operation aborted!");
      return;
    case MODE_MASTER:
      // When Master mode is tested, time diagram is as follows:
      // XFER                                                ¯|¯
      // ... 4 ms                                             .
      // Slave Control (SPI Server)                           .
      // ... 4 ms                                             .
      // Master Control (SPI Client (DUT))                    .
      // ... 4 ms                                    SPI_CFG_XFER_TIMEOUT
      // Slave Transfer (SPI Server)                          .
      // ... 4 ms                                             .
      // Master Send/Receive/Transfer (SPI Client (DUT))      .
      // ... data exchange                                   _|_
      drv_mode    = ARM_SPI_MODE_MASTER;
      srv_mode    = 1U;
      srv_delay_c = 4U;
      srv_delay_t = 8U;
      drv_delay_c = 8U;
      drv_delay_t = 8U;
      break;
    case MODE_SLAVE:
      // When Slave mode is tested, time diagram is as follows:
      // XFER                                                ¯|¯
      // ... 4 ms                                             .
      // Slave Control (SPI Client (DUT))                     .
      // ... 4 ms                                             .
      // Master Control (SPI Server)                          .
      // ... 4 ms                                    SPI_CFG_XFER_TIMEOUT
      // Slave Transfer (SPI Client (DUT))                    .
      // ... 4 ms                                             .
      // Master Send/Receive/Transfer (SPI Server)            .
      // ... data exchange                                   _|_
      drv_mode    = ARM_SPI_MODE_SLAVE;
      srv_mode    = 0U;
      srv_delay_c = 8U;
      srv_delay_t = 8U;
      drv_delay_c = 4U;
      drv_delay_t = 8U;
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

  // Total transfer timeout (16 ms is overhead before transfer starts)
  timeout = SPI_CFG_XFER_TIMEOUT + 16U;

  // Check that SPI status is not busy before starting data exchange test
  spi_stat = drv->GetStatus();          // Get SPI status
  if (spi_stat.busy != 0U) {
    // If busy flag is active
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Busy active before operation! Data exchange operation aborted!");
  }
  TEST_ASSERT_MESSAGE(spi_stat.busy == 0U, msg_buf);

  do {
#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected
    if (CmdSetBufTx('S')   != EXIT_SUCCESS) { break; }
    if (CmdSetBufRx('?')   != EXIT_SUCCESS) { break; }
    if (CmdSetCom  (srv_mode, format, data_bits, bit_order, srv_ss_mode, bus_speed) != EXIT_SUCCESS) { break; }
    if (CmdXfer    (num, srv_delay_c, srv_delay_t, SPI_CFG_XFER_TIMEOUT) != EXIT_SUCCESS)            { break; }
    (void)drv->Control(ARM_SPI_MODE_INACTIVE, 0U);
#else                                   // If Test Mode Loopback is selected
    // Remove warnings for unused variables
    (void)srv_mode;
    (void)srv_ss_mode;
    (void)srv_delay_c;
    (void)srv_delay_t;
#endif
    start_tick = osKernelGetTickCount();

    // Initialize buffers
    memset(ptr_tx_buf,  (int32_t)'!' , SPI_BUF_MAX);
    memset(ptr_tx_buf,  (int32_t)'T' , num * DataBitsToBytes(data_bits));
    memset(ptr_rx_buf,  (int32_t)'?' , SPI_BUF_MAX);
    memset(ptr_cmp_buf, (int32_t)'?' , SPI_BUF_MAX);

    // Configure required communication settings
    (void)osDelay(drv_delay_c);         // Wait specified time before calling Control function
    if (mode == MODE_MASTER) {
      stat = drv->Control (drv_mode | drv_format | drv_data_bits | drv_bit_order | drv_ss_mode, bus_speed);
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

    // Set default Tx value to 'D' byte values (only for master mode)
    if (mode == MODE_MASTER) {
      val = ((uint32_t)'D' << 24) | ((uint32_t)'D' << 16) | ((uint32_t)'D' << 8) | (uint32_t)'D';
      stat = drv->Control (ARM_SPI_SET_DEFAULT_TX_VALUE, val);
      def_tx_stat = stat;
      if ((stat != ARM_DRIVER_OK) && (stat != ARM_DRIVER_ERROR_UNSUPPORTED)) {
        // If set default Tx value has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s", str_oper[operation], "Set default Tx value returned", str_ret[-stat]);
      }
      // Assert that Control function returned ARM_DRIVER_OK or ARM_DRIVER_ERROR_UNSUPPORTED
      TEST_ASSERT_MESSAGE((stat == ARM_DRIVER_OK) || (stat == ARM_DRIVER_ERROR_UNSUPPORTED), msg_buf);

      if (stat == ARM_DRIVER_ERROR_UNSUPPORTED) {
        // If set default Tx value is not supported
        (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] %s: %s", str_oper[operation], "Set default Tx value is not supported");
        TEST_MESSAGE(msg_buf);
      }
    } else {
      // For slave mode default Tx is not tested
      def_tx_stat = ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    (void)osDelay(drv_delay_t);         // Wait specified time before calling Send/Receive/Transfer function

    // Prepare local variables
    event             = 0U;
    duration          = 0xFFFFFFFFUL;
    data_count        = 0U;
    data_count_sample = 0U;
    chk_data          = 1U;
    start_cnt         = osKernelGetSysTimerCount();

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
      // If Send/Receive/Transfer start has failed
      (void)drv->Control(ARM_SPI_MODE_INACTIVE, 0U);
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
      TEST_ASSERT_MESSAGE(spi_stat.busy == 0U, msg_buf);

      data_count = drv->GetDataCount();         // Get data count
      if (data_count >= num) {
        // If data count is more or equal to number of items then Abort has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetDataCount returned", data_count, "after Abort of", num, "items");
      }
      // Assert data count is less then number of items requested for exchange
      TEST_ASSERT_MESSAGE(data_count < num, msg_buf);

#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected
      // Deactivate SPI
      (void)drv->Control(ARM_SPI_MODE_INACTIVE, 0U);

      // Wait until timeout expires
      curr_tick = osKernelGetTickCount();
      if ((curr_tick - start_tick) < timeout) {
        (void)osDelay(timeout - (curr_tick - start_tick));
      }
      (void)osDelay(20U);                       // Wait for SPI Server to start reception of next command
#endif

      return;                                   // Here Abort test is finished, exit
    }

    // Wait for operation to finish (status busy is 0 and event complete signaled, or timeout)
    do {
      if (data_count_sample == 0U) {
        // Store first data count different than 0
        data_count_sample = drv->GetDataCount();  // Get data count
      }
      if ((drv->GetStatus().busy == 0U) && ((event & ARM_SPI_EVENT_TRANSFER_COMPLETE) != 0U)) {
        duration = osKernelGetSysTimerCount() - start_cnt;
        break;
      }
    } while ((osKernelGetTickCount() - start_tick) < timeout);

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
      chk_data = 0U;                            // Do not check transferred content
    }
    // Assert that ARM_SPI_EVENT_TRANSFER_COMPLETE was signaled
    TEST_ASSERT_MESSAGE((event & ARM_SPI_EVENT_TRANSFER_COMPLETE) != 0U, msg_buf);

    spi_stat = drv->GetStatus();                // Get SPI status
    if (spi_stat.busy != 0U) {
      // If busy flag is still active
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Busy still active after operation");
      chk_data = 0U;                            // Do not check transferred content
    }
    // Assert that busy flag is not active
    TEST_ASSERT_MESSAGE(spi_stat.busy == 0U, msg_buf);

    if ((event & ARM_SPI_EVENT_DATA_LOST) != 0U) {
      // If data lost was signaled during the transfer
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_SPI_EVENT_DATA_LOST was signaled");
      chk_data = 0U;                            // Do not check transferred content
    }
    // Assert that ARM_SPI_EVENT_DATA_LOST was not signaled
    TEST_ASSERT_MESSAGE((event & ARM_SPI_EVENT_DATA_LOST) == 0U, msg_buf);

    data_count = drv->GetDataCount();           // Get data count
    if (data_count != num) {
      // If data count is different then number of items, then operation has failed
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetDataCount returned", data_count, "expected was", num, "items");
      chk_data = 0U;                            // Do not check transferred content
    }
    // Assert that data count is equal to number of items requested for exchange
    TEST_ASSERT_MESSAGE(data_count == num, msg_buf);

    if ((drv->GetStatus().busy != 0U) || ((event & ARM_SPI_EVENT_TRANSFER_COMPLETE) == 0U)) {
      // If transfer did not finish in time, abort it
      (void)drv->Control(ARM_SPI_ABORT_TRANSFER, 0U);
    }

#if (SPI_SERVER_USED == 1)              // If Test Mode SPI Server is selected
    // Deactivate SPI
    (void)drv->Control(ARM_SPI_MODE_INACTIVE, 0U);

    // Wait until timeout expires
    curr_tick = osKernelGetTickCount();
    if ((curr_tick - start_tick) < timeout) {
      (void)osDelay(timeout - (curr_tick - start_tick));
    }
    (void)osDelay(20U);                 // Wait for SPI Server to start reception of next command

    if (chk_data != 0U) {               // If transferred content should be checked
      // Check received content for receive and transfer operations
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
      if ((mode == MODE_MASTER) || (operation != OP_RECEIVE) || (def_tx_stat == ARM_DRIVER_OK)) {
        // Check sent data in all cases except Slave mode Receive operation
        // with Default Tx not working or unsupported
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
      }
    }
#else                                   // If Test Mode Loopback is selected
    if (chk_data != 0U) {               // If transferred content should be checked
      if (operation == OP_TRANSFER) {
        memset(ptr_cmp_buf, (int32_t)('T' & ((1U << data_bits) - 1U)), num * DataBitsToBytes(data_bits));
        if ((data_bits > 8U) && (data_bits < 16U)) {
          for (i = 1U; i < (num * DataBitsToBytes(data_bits)); i+= DataBitsToBytes(data_bits)) {
            ptr_cmp_buf[i] = 'T' & ((1U << (data_bits - 8U)) - 1U);
          }
        } else if ((data_bits > 16U) && (data_bits < 32U)) {
          for (i = 2U; i < (num * DataBitsToBytes(data_bits)); i+= DataBitsToBytes(data_bits)) {
            if (data_bits <= 24U) {
              ptr_cmp_buf[i  ] = 'T' & ((1U << (data_bits - 16U)) - 1U);
              ptr_cmp_buf[i+1] = 0U;
            } else {
              ptr_cmp_buf[i+1] = 'T' & ((1U << (data_bits - 24U)) - 1U);
            }
          }
        }
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

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_UNUSED, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
#if (SPI_SERVER_USED == 1)
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_UNUSED, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
#endif
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_UNUSED, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Master_SS_Sw_Ctrl
\details
The function \b SPI_Mode_Master_SS_Sw_Ctrl verifies data exchange:
 - in <b>Master Mode</b> with <b>Slave Select line Software controlled</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is not executed
*/
void SPI_Mode_Master_SS_Sw_Ctrl (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Master_SS_Hw_Ctrl_Out
\details
The function \b SPI_Mode_Master_SS_Hw_Ctrl_Out verifies data exchange:
 - in <b>Master Mode</b> with <b>Slave Select line Hardware controlled Output</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test not executed
*/
void SPI_Mode_Master_SS_Hw_Ctrl_Out (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Master_SS_Hw_Mon_In
\details
The function \b SPI_Mode_Master_SS_Hw_Mon_In verifies data exchange:
 - in <b>Master Mode</b> with <b>Slave Select line Hardware monitored Input</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test not executed
*/
void SPI_Mode_Master_SS_Hw_Mon_In (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_INPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_INPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_MASTER_HW_INPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Slave_SS_Hw_Mon
\details
The function \b SPI_Mode_Slave_SS_Hw_Mon verifies data exchange:
 - in <b>Slave Mode</b> with <b>Slave Select line Hardware monitored</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test not executed
*/
void SPI_Mode_Slave_SS_Hw_Mon (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_HW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_HW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_HW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Mode_Slave_SS_Sw_Ctrl
\details
The function \b SPI_Mode_Slave_SS_Sw_Ctrl verifies data exchange:
 - in <b>Slave Mode</b> with <b>Slave Select line Software controlled</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test not executed
*/
void SPI_Mode_Slave_SS_Sw_Ctrl (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SS_MODE_SLAVE_SW, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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

\note In Test Mode <b>Loopback</b> this test not executed
*/
void SPI_Format_Clock_Pol0_Pha0 (void) {

  if (IsNotLoopback()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, FORMAT_CPOL0_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_CPOL0_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_CPOL0_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_CPOL0_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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

\note In Test Mode <b>Loopback</b> this test not executed
*/
void SPI_Format_Clock_Pol0_Pha1 (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, FORMAT_CPOL0_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_CPOL0_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_CPOL0_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_CPOL0_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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

\note In Test Mode <b>Loopback</b> this test not executed
*/
void SPI_Format_Clock_Pol1_Pha0 (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, FORMAT_CPOL1_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_CPOL1_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_CPOL1_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_CPOL1_CPHA0, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Format_Clock_Pol1_Pha1
\details
The function \b SPI_Format_Clock_Pol1_Pha1 verifies data exchange:
 - in Master Mode with default Slave Select mode
 - with clock format: <b>polarity 1 / phase 1</b>
 - with default data bits
 - with default bit order
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test not executed
*/
void SPI_Format_Clock_Pol1_Pha1 (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, FORMAT_CPOL1_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_CPOL1_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_CPOL1_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_CPOL1_CPHA1, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Format_Frame_TI
\details
The function \b SPI_Format_Frame_TI verifies data exchange:
 - in <b>Master Mode</b> with <b>Slave Select line Hardware controlled Output</b>
 - with <b>Texas Instruments frame format</b>
 - with default data bits
 - with bit order <b>from MSB to LSB</b>
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test not executed
*/
void SPI_Format_Frame_TI (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, FORMAT_TI, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_TI, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_TI, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_TI, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Format_Clock_Microwire
\details
The function \b SPI_Format_Clock_Microwire verifies data exchange:
 - in <b>Master Mode</b> with <b>Slave Select line Hardware controlled Output</b>
 - with <b>National Semiconductor Microwire frame format</b>
 - with default data bits
 - with bit order <b>from MSB to LSB</b>
 - at default bus speed
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test not executed
*/
void SPI_Format_Clock_Microwire (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, FORMAT_MICROWIRE, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, FORMAT_MICROWIRE, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, FORMAT_MICROWIRE, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, FORMAT_MICROWIRE, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SS_MODE_MASTER_HW_OUTPUT, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_1 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 1U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 1U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 1U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 1U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_2 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 2U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 2U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 2U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 2U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_3 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 3U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 3U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 3U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 3U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_4 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 4U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 4U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 4U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 4U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_5 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 5U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 5U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 5U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 5U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_6 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 6U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 6U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 6U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 6U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_7 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 7U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 7U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 7U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 7U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 8U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
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
*/
void SPI_Data_Bits_9 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 9U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 9U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 9U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 9U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_10 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 10U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 10U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 10U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 10U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_11 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 11U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 11U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 11U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 11U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_12 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 12U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 12U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 12U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 12U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_13 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 13U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 13U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 13U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 13U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_14 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 14U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 14U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 14U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 14U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_15 (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 15U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 15U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 15U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 15U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 16U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
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
*/
void SPI_Data_Bits_17 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 17U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 17U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 17U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 17U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_18 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 18U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 18U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 18U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 18U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_19 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 19U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 19U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 19U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 19U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_20 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 20U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 20U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 20U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 20U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_21 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 21U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 21U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 21U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 21U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_22 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 22U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 22U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 22U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 22U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_23 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 23U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 23U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 23U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 23U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 24U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
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
*/
void SPI_Data_Bits_25 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 25U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 25U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 25U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 25U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_26 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 26U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 26U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 26U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 26U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_27 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 27U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 27U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 27U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 27U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_28 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 28U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 28U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 28U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 28U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_29 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 29U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 29U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 29U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 29U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_30 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 30U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 30U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 30U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 30U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
*/
void SPI_Data_Bits_31 (void) {

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 31U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, 31U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, 31U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, 31U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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

  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, 32U, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
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

\note In Test Mode <b>Loopback</b> this test is not executed
*/
void SPI_Bit_Order_MSB_LSB (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_MSB_TO_LSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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

\note In Test Mode <b>Loopback</b> this test is not executed
*/
void SPI_Bit_Order_LSB_MSB (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_LSB_TO_MSB, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_LSB_TO_MSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_LSB_TO_MSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, BO_LSB_TO_MSB, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
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
  volatile  int32_t ret_bus_speed;

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_MIN_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
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
  ret_bus_speed = drv->Control (ARM_SPI_GET_BUS_SPEED, 0U);
  if (ret_bus_speed < 0) {
    // If bus speed value returned by the driver is negative
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Get bus speed returned negative value %i", ret_bus_speed);
    TEST_FAIL_MESSAGE(msg_buf);
  } else if ((uint32_t)ret_bus_speed > SPI_CFG_MIN_BUS_SPEED) {
    // If bus speed value returned by the driver is higher then requested
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Get bus speed returned %i bps instead of requested %i bps", ret_bus_speed, SPI_CFG_MIN_BUS_SPEED);
    TEST_FAIL_MESSAGE(msg_buf);
  } else if ((uint32_t)ret_bus_speed < ((SPI_CFG_MIN_BUS_SPEED * 3) / 4)) {
    // If bus speed value returned by the driver is lower then 75% of requested
    (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] Get bus speed returned %i bps instead of requested %i bps", ret_bus_speed, SPI_CFG_MIN_BUS_SPEED);
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
  volatile  int32_t ret_bus_speed;

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_MAX_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_MAX_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_MAX_BUS_SPEED, SPI_CFG_DEF_NUM);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_MAX_BUS_SPEED, SPI_CFG_DEF_NUM);

  if (duration != 0xFFFFFFFFU) {        // If Transfer finished before timeout
    if (duration != 0U) {               // If duration of transfer was more than 0 SysTick counts
      bps = ((uint64_t)systick_freq * SPI_CFG_DEF_DATA_BITS * SPI_CFG_DEF_NUM) / duration;
      if ((bps < ((SPI_CFG_MAX_BUS_SPEED * 3) / 4)) ||
          (bps >   SPI_CFG_MAX_BUS_SPEED)) {
        // If measured bus speed is 25% lower, or higher than requested
        (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] At requested bus speed of %i bps, effective bus speed is %i bps", SPI_CFG_MAX_BUS_SPEED, (uint32_t)bps);
        TEST_MESSAGE(msg_buf);
      }
    }
  }

  drv->Control (ARM_SPI_SET_BUS_SPEED, SPI_CFG_MAX_BUS_SPEED);
  ret_bus_speed = drv->Control (ARM_SPI_GET_BUS_SPEED, 0U);
  if (ret_bus_speed < 0) {
    // If bus speed value returned by the driver is negative
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Get bus speed returned negative value %i", ret_bus_speed);
    TEST_FAIL_MESSAGE(msg_buf);
  } else if ((uint32_t)ret_bus_speed > SPI_CFG_MAX_BUS_SPEED) {
    // If bus speed value returned by the driver is higher then requested
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Get bus speed returned %i bps instead of requested %i bps", ret_bus_speed, SPI_CFG_MAX_BUS_SPEED);
    TEST_FAIL_MESSAGE(msg_buf);
  } else if ((uint32_t)ret_bus_speed < ((SPI_CFG_MAX_BUS_SPEED * 3) / 4)) {
    // If bus speed value returned by the driver is lower then 75% of requested
    (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] Get bus speed returned %i bps instead of requested %i bps", ret_bus_speed, SPI_CFG_MAX_BUS_SPEED);
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

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

#if (SPI_CFG_NUM1 != 0U)
  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM1);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM1);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM1);
#endif

#if (SPI_CFG_NUM2 != 0U)
  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM2);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM2);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM2);
#endif

#if (SPI_CFG_NUM3 != 0U)
  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM3);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM3);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM3);
#endif

#if (SPI_CFG_NUM4 != 0U)
  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM4);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM4);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM4);
#endif

#if (SPI_CFG_NUM5 != 0U)
  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM5);
  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM5);
  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_NUM5);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_GetDataCount
\details
The function \b SPI_GetDataCount verifies \b GetDataCount function (count changing) during data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
*/
void SPI_GetDataCount (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
#endif

  SPI_DataExchange_Operation(OP_SEND,     MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  TEST_ASSERT_MESSAGE((data_count_sample != 0U) && (data_count_sample != SPI_CFG_DEF_NUM), "[FAILED] GetDataCount was not changing during the Send!");

  SPI_DataExchange_Operation(OP_RECEIVE,  MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  TEST_ASSERT_MESSAGE((data_count_sample != 0U) && (data_count_sample != SPI_CFG_DEF_NUM), "[FAILED] GetDataCount was not changing during the Receive!");

  SPI_DataExchange_Operation(OP_TRANSFER, MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_SS_MODE, SPI_CFG_DEF_BUS_SPEED, SPI_CFG_DEF_NUM);
  TEST_ASSERT_MESSAGE((data_count_sample != 0U) && (data_count_sample != SPI_CFG_DEF_NUM), "[FAILED] GetDataCount was not changing during the Transfer!");
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_Abort
\details
The function \b SPI_Abort verifies \b Abort function abort of data exchange:
 - in Master Mode with default Slave Select mode
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed
*/
void SPI_Abort (void) {

  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (IsBitOrderValid() != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_SLAVE, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }
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
/* SPI Event tests                                                                                                          */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup spi_tests_evt Event
\ingroup spi_tests
\details
These tests verify API and operation of the SPI event signaling, except ARM_SPI_EVENT_TRANSFER_COMPLETE
signal which is tested in the Data Exchange tests.

The event tests verify the following driver function
(<a href="http://www.keil.com/pack/doc/CMSIS/Driver/html/group__spi__interface__gr.html" target="_blank">SPI Driver function documentation</a>):
 - \b SignalEvent
\code
  void (*ARM_SPI_SignalEvent_t) (uint32_t event);
\endcode

\note In Test Mode <b>Loopback</b> these tests are not executed
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_DataLost
\details
The function \b SPI_DataLost verifies signaling of the <b>ARM_SPI_EVENT_DATA_LOST</b> event:
 - in <b>Slave Mode</b> with <b>Slave Select line Hardware monitored</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed

it also checks that status data_lost flag was activated.
*/
void SPI_DataLost (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (IsFormatValid()   != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (CmdSetCom  (0U, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, 1U, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { break; }
    if (CmdXfer    (1U, 8U, 8U, SPI_CFG_XFER_TIMEOUT) != EXIT_SUCCESS) { break; }
    drv->Control   (ARM_SPI_MODE_INACTIVE, 0U);

    event = 0U;
    (void)osDelay(4U);
    (void)drv->Control (ARM_SPI_MODE_SLAVE                                                                 | 
                      ((SPI_CFG_DEF_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos)   & ARM_SPI_FRAME_FORMAT_Msk)   | 
                      ((SPI_CFG_DEF_DATA_BITS << ARM_SPI_DATA_BITS_Pos)      & ARM_SPI_DATA_BITS_Msk)      | 
                      ((SPI_CFG_DEF_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)      & ARM_SPI_BIT_ORDER_Msk)      | 
                        ARM_SPI_SS_SLAVE_HW                                                                , 
                        0U);

    (void)osDelay(SPI_CFG_XFER_TIMEOUT+20U);    // Wait for SPI Server to timeout

    (void)drv->Control(ARM_SPI_MODE_INACTIVE, 0U);
    (void)osDelay(20U);                 // Wait for SPI Server to start reception of next command

    // Assert that event ARM_SPI_EVENT_DATA_LOST was signaled
    TEST_ASSERT_MESSAGE((event & ARM_SPI_EVENT_DATA_LOST) != 0U, "[FAILED] Event ARM_SPI_EVENT_DATA_LOST was not signaled!");

    // Assert that status data_lost flag is active
    TEST_ASSERT_MESSAGE(drv->GetStatus().data_lost != 0U, "[FAILED] Status data_lost flag was not activated!");

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function SPI_ModeFault
\details
The function \b SPI_ModeFault verifies signaling of the <b>ARM_SPI_EVENT_MODE_FAULT</b> event:
 - in <b>Master Mode</b> with <b>Slave Select line Hardware monitored Input</b>
 - with default clock / frame format
 - with default data bits
 - with default bit order
 - at default bus speed

it also checks that status mode_fault flag was activated.
*/
void SPI_ModeFault (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) {              return; }
  if (IsNotFrameTI()    != EXIT_SUCCESS) {              return; }
  if (IsNotFrameMw()    != EXIT_SUCCESS) {              return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (BuffersCheck()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (SPI_SERVER_USED == 1)
  if (ServerCheck()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (ServerCheckSupport(MODE_MASTER, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (CmdSetCom  (0U, SPI_CFG_DEF_FORMAT, SPI_CFG_DEF_DATA_BITS, SPI_CFG_DEF_BIT_ORDER, 1U, SPI_CFG_DEF_BUS_SPEED) != EXIT_SUCCESS) { break; }
    if (CmdXfer    (1U, 8U, 8U, SPI_CFG_XFER_TIMEOUT) != EXIT_SUCCESS) { break; }
    drv->Control   (ARM_SPI_MODE_INACTIVE, 0U);

    event = 0U;
    (void)osDelay(4U);
    (void)drv->Control (ARM_SPI_MODE_MASTER                                                              | 
                      ((SPI_CFG_DEF_FORMAT    << ARM_SPI_FRAME_FORMAT_Pos)   & ARM_SPI_FRAME_FORMAT_Msk) | 
                      ((SPI_CFG_DEF_DATA_BITS << ARM_SPI_DATA_BITS_Pos)      & ARM_SPI_DATA_BITS_Msk)    | 
                      ((SPI_CFG_DEF_BIT_ORDER << ARM_SPI_BIT_ORDER_Pos)      & ARM_SPI_BIT_ORDER_Msk)    | 
                        ARM_SPI_SS_MASTER_HW_INPUT                                                       , 
                        SPI_CFG_DEF_BUS_SPEED);

    (void)osDelay(SPI_CFG_XFER_TIMEOUT+20U);    // Wait for SPI Server to timeout

    (void)drv->Control(ARM_SPI_MODE_INACTIVE, 0U);
    (void)osDelay(20U);                 // Wait for SPI Server to start reception of next command

    // Assert that event ARM_SPI_EVENT_MODE_FAULT was signaled
    TEST_ASSERT_MESSAGE((event & ARM_SPI_EVENT_MODE_FAULT) != 0U, "[FAILED] Event ARM_SPI_EVENT_MODE_FAULT was not signaled!");

    // Assert that status mode_fault flag is active
    TEST_ASSERT_MESSAGE(drv->GetStatus().mode_fault != 0U, "[FAILED] Status mode_fault flag was not activated!");

    return;
  } while (false);
#endif
}

/**
@}
*/
// End of spi_tests_evt
