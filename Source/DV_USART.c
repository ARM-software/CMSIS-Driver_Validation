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
 * Title:       Universal Synchronous Asynchronous Receiver/Transmitter (USART) 
 *              Driver Validation tests
 *
 * -----------------------------------------------------------------------------
 */

#ifndef __DOXYGEN__                     // Exclude form the documentation

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmsis_dv.h"
#include "DV_USART_Config.h"
#include "DV_Framework.h"

#include "Driver_USART.h"

// Fixed settings for communication with USART Server (not available through DV_USART_Config.h)
#define  USART_CFG_SRV_BAUDRATE         115200  // Baudrate
#define  USART_CFG_SRV_DATA_BITS        8       // 8 data bits
#define  USART_CFG_SRV_PARITY           0       // None
#define  USART_CFG_SRV_STOP_BITS        0       // 1 stop bit
#define  USART_CFG_SRV_FLOW_CONTROL     0       // None

// Check configuration
#if (USART_CFG_TEST_MODE == 1)          // If USART Server is selected

#if (USART_CFG_DEF_MODE == 1)           // If default mode is Asynchronous
#if (USART_CFG_SRV_MODE != 1)           // If USART Server mode is not Asynchronous
#warning When USART Server Test Mode is used and Default settings for Tests Mode (USART_CFG_DEF_MODE) is Asynchronous then Mode for USART Server (USART_CFG_SRV_MODE) must also be Asynchronous!
#endif
#elif (USART_CFG_DEF_MODE == 2)         // If default mode is Synchronous Master
#if (USART_CFG_SRV_MODE != 1)           // If USART Server mode is not Asynchronous
#warning When USART Server Test Mode is used and Default settings for Tests Mode (USART_CFG_DEF_MODE) is Synchronous Master then Mode for USART Server (USART_CFG_SRV_MODE) must be Asynchronous!
#endif
#elif (USART_CFG_DEF_MODE == 3)         // If default mode is Synchronous Slave
#if (USART_CFG_SRV_MODE != 1)           // If USART Server mode is not Asynchronous
#warning When USART Server Test Mode is used and Default settings for Tests Mode (USART_CFG_DEF_MODE) is Synchronous Slave then Mode for USART Server (USART_CFG_SRV_MODE) must be Asynchronous!
#endif
#elif (USART_CFG_DEF_MODE == 4)         // If default mode is Single-wire
#if (USART_CFG_SRV_MODE != 4)           // If USART Server mode is not Single-wire
#warning When USART Server Test Mode is used and Default settings for Tests Mode (USART_CFG_DEF_MODE) is Single-wire then Mode for USART Server (USART_CFG_SRV_MODE) must also be Single-wire!
#endif
#elif (USART_CFG_DEF_MODE == 5)         // If default mode is IrDA
#if (USART_CFG_SRV_MODE != 5)           // If USART Server mode is not IrDA
#warning When USART Server Test Mode is used and Default settings for Tests Mode (USART_CFG_DEF_MODE) is Single-wire then Mode for USART Server (USART_CFG_SRV_MODE) must also be Single-wire!
#endif
#else
#warning Unknown Default settings for Tests Mode (USART_CFG_DEF_MODE)!
#endif
#else                                   // If Loopback is selected
#if (USART_CFG_DEF_MODE != 1)           // If default mode is not Asynchronous
#error For Loopback Test Mode only Default settings for Tests Mode (USART_CFG_DEF_MODE) Asynchronous setting is supported!
#endif
#endif

#define CMD_LEN                   32UL  // Length of command to USART Server
#define RESP_GET_VER_LEN          16UL  // Length of response from USART Server to GET VER command
#define RESP_GET_CAP_LEN          32UL  // Length of response from USART Server to GET CAP command
#define RESP_GET_CNT_LEN          16UL  // Length of response from USART Server to GET CNT command
#define RESP_GET_BRK_LEN          1UL   // Length of response from USART Server to GET BRK command
#define RESP_GET_MDM_LEN          1UL   // Length of response from USART Server to GET MDM command

#define OP_SEND                   0UL   // Send operation
#define OP_RECEIVE                1UL   // Receive operation
#define OP_TRANSFER               2UL   // Transfer operation (in synchronous mode only)
#define OP_RECEIVE_SEND_LB        3UL   // Loopback testing Receive and Send operation (in asynchronous mode only)
#define OP_ABORT_SEND             4UL   // Abort send operation
#define OP_ABORT_RECEIVE          5UL   // Abort receive operation
#define OP_ABORT_TRANSFER         6UL   // Abort transfer operation

#define MODE_ASYNCHRONOUS         1UL   // UART (Asynchronous)
#define MODE_SYNCHRONOUS_MASTER   2UL   // Synchronous Master (generates clock signal)
#define MODE_SYNCHRONOUS_SLAVE    3UL   // Synchronous Slave (external clock signal)
#define MODE_SINGLE_WIRE          4UL   // UART Single-wire (half-duplex)
#define MODE_IRDA                 5UL   // UART IrDA
#define MODE_SMART_CARD           6UL   // UART Smart Card
#define PARITY_NONE               0UL   // No parity
#define PARITY_EVEN               1UL   // Even Parity
#define PARITY_ODD                2UL   // Odd Parity
#define STOP_BITS_1               0UL   // 1 Stop bit
#define STOP_BITS_2               1UL   // 2 Stop bits
#define STOP_BITS_1_5             2UL   // 1.5 Stop bits
#define STOP_BITS_0_5             3UL   // 0.5 Stop bits
#define FLOW_CONTROL_NONE         0UL   // No flow control signal
#define FLOW_CONTROL_RTS          1UL   // RTS flow control signal
#define FLOW_CONTROL_CTS          2UL   // CTS flow control signal
#define FLOW_CONTROL_RTS_CTS      3UL   // RTS and CTS flow control signal
#define CPOL0                     0UL   // Clock Polarity 0
#define CPOL1                     1UL   // Clock Polarity 1
#define CPHA0                     0UL   // Clock Phase 0
#define CPHA1                     1UL   // Clock Phase 1
#define RTS_AVAILABLE             1UL   // Mask of RTS line available
#define CTS_AVAILABLE             2UL   // Mask of CTS line available
#define DTR_AVAILABLE             4UL   // Mask of DTR line available
#define DSR_AVAILABLE             8UL   // Mask of DSR line available
#define DCD_AVAILABLE             16UL  // Mask of DCD line available
#define RI_AVAILABLE              32UL  // Mask of RI  line available
#define RTS_ON                    1UL   // Set RTS to active state
#define DTR_ON                    2UL   // Set DTR to active state
#define TO_DCD_ON                 4UL   // Set line driving DCD on USART Client to active state
#define TO_RI_ON                  8UL   // Set line driving RI  on USART Client to active state


#define DRIVER_DATA_BITS(x)       ((x == 9U) ? ARM_USART_DATA_BITS_9 : ((x == 8U) ? ARM_USART_DATA_BITS_8 : (((x) << ARM_USART_DATA_BITS_Pos) & ARM_USART_DATA_BITS_Msk)))

// Testing Configuration definitions
#if    (USART_CFG_TEST_MODE != 0)
#define USART_SERVER_USED               1
#else
#define USART_SERVER_USED               0
#endif

// Prepare values for default setting
#define USART_CFG_DEF_MODE_VAL        ((USART_CFG_DEF_MODE << ARM_USART_CONTROL_Pos) & ARM_USART_CONTROL_Msk)
#if    (USART_CFG_DEF_DATA_BITS == 5U)
#define USART_CFG_DEF_DATA_BITS_VAL    (ARM_USART_DATA_BITS_5)
#elif  (USART_CFG_DEF_DATA_BITS == 6U)
#define USART_CFG_DEF_DATA_BITS_VAL    (ARM_USART_DATA_BITS_6)
#elif  (USART_CFG_DEF_DATA_BITS == 7U)
#define USART_CFG_DEF_DATA_BITS_VAL    (ARM_USART_DATA_BITS_7)
#elif  (USART_CFG_DEF_DATA_BITS == 8U)
#define USART_CFG_DEF_DATA_BITS_VAL    (ARM_USART_DATA_BITS_8)
#elif  (USART_CFG_DEF_DATA_BITS == 9U)
#define USART_CFG_DEF_DATA_VAL         (ARM_USART_DATA_BITS_9)
#endif
#define USART_CFG_DEF_PARITY_VAL      ((USART_CFG_DEF_PARITY    << ARM_USART_PARITY_Pos)    & ARM_USART_PARITY_Msk)
#define USART_CFG_DEF_STOP_BITS_VAL   ((USART_CFG_DEF_STOP_BITS << ARM_USART_STOP_BITS_Pos) & ARM_USART_STOP_BITS_Msk)
#define USART_CFG_DEF_FLOW_CONTROL_VAL ((USART_CFG_DEF_FLOW_CONTROL << ARM_USART_FLOW_CONTROL_Pos) & ARM_USART_FLOW_CONTROL_Msk)
#define USART_CFG_DEF_CPOL_VAL        ((USART_CFG_DEF_CPOL << ARM_USART_CPOL_Pos) & ARM_USART_CPOL_Msk)
#define USART_CFG_DEF_CPHA_VAL        ((USART_CFG_DEF_CPHA << ARM_USART_CPHA_Pos) & ARM_USART_CPHA_Msk)

// Check if timeout setting is valid
#if    (USART_CFG_XFER_TIMEOUT <= 2U)
#error  Transfer timeout must be longer than 2ms!
#endif

// Check if default number of items setting is valid
#if    (USART_CFG_DEF_NUM == 0)
#error  Default number of items to test must not be 0!
#endif

// Determine maximum number of items used for testing
#define USART_NUM_MAX                   USART_CFG_DEF_NUM
#if    (USART_CFG_NUM1 > USART_NUM_MAX)
#undef  USART_NUM_MAX
#define USART_NUM_MAX                   USART_CFG_NUM1
#endif
#if    (USART_CFG_NUM2 > USART_NUM_MAX)
#undef  USART_NUM_MAX
#define USART_NUM_MAX                   USART_CFG_NUM2
#endif
#if    (USART_CFG_NUM3 > USART_NUM_MAX)
#undef  USART_NUM_MAX
#define USART_NUM_MAX                   USART_CFG_NUM3
#endif
#if    (USART_CFG_NUM4 > USART_NUM_MAX)
#undef  USART_NUM_MAX
#define USART_NUM_MAX                   USART_CFG_NUM4
#endif
#if    (USART_CFG_NUM5 > USART_NUM_MAX)
#undef  USART_NUM_MAX
#define USART_NUM_MAX                   USART_CFG_NUM5
#endif

// Calculate maximum required buffer size
#if    (USART_CFG_DEF_DATA_BITS > 8)
#define USART_BUF_MAX                  (USART_NUM_MAX * 2U)
#else
#define USART_BUF_MAX                  (USART_NUM_MAX)
#endif

typedef struct {                // USART Server version structure
  uint8_t  major;               // Version major number
  uint8_t  minor;               // Version minor number
  uint16_t patch;               // Version patch (revision) number
} USART_SERV_VER_t;

typedef struct {                // USART Server capabilities structure
  uint32_t mode_mask;           // Mode mask
  uint32_t db_mask;             // Data Bits mask
  uint32_t parity_mask;         // Parity mask
  uint32_t sb_mask;             // Stop Bits mask
  uint32_t fc_mask;             // Flow Control mask
  uint32_t ml_mask;             // Modem lines mask
  uint32_t br_min;              // Min baudrate
  uint32_t br_max;              // Max baudrate
} USART_SERV_CAP_t;

// Register Driver_USART#
#define _ARM_Driver_USART_(n)         Driver_USART##n
#define  ARM_Driver_USART_(n)    _ARM_Driver_USART_(n)
extern   ARM_DRIVER_USART         ARM_Driver_USART_(DRV_USART);
static   ARM_DRIVER_USART *drv = &ARM_Driver_USART_(DRV_USART);

// Local variables (used in this module only)
static int8_t                   buffers_ok;
static int8_t                   driver_ok;
static int8_t                   server_ok;

static USART_SERV_VER_t         usart_serv_ver;
static USART_SERV_CAP_t         usart_serv_cap;

static ARM_USART_CAPABILITIES   drv_cap;
static volatile uint32_t        event;
static volatile uint32_t        duration;
static volatile uint32_t        xfer_count;
static volatile uint32_t        tx_count_sample, rx_count_sample;
static volatile uint8_t         modem_status;
static volatile uint8_t         break_status;
static uint32_t                 systick_freq;
static uint32_t                 ticks_per_ms;

static osEventFlagsId_t         event_flags;

static char                     msg_buf[512];

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
  "USART Server"
};

static const char *str_oper[] = {
  "Send    ",
  "Receive ",
  "Transfer",
  "Receive/Send LB",
  "Abort Send    ",
  "Abort Receive ",
  "Abort Transfer"
};

static const char *str_mode[] = {
  "None",
  "Asynchronous",
  "Synchronous Master",
  "Synchronous Slave",
  "Single-wire",
  "IrDA",
  "Smart Card"
};

static const char *str_parity[] = {
  "None",
  "Even",
  "Odd"
};

static const char *str_stop_bits[] = {
  "1",
  "2",
  "1.5",
  "0.5"
};

static const char *str_flow_control[] = {
  "None",
  "CTS",
  "RTS",
  "RTS/CTS",
};

static const char *str_cpol[] = {
  "CPOL0",
  "CPOL1"
};

static const char *str_cpha[] = {
  "CPHA0",
  "CPHA1"
};

static const char *str_modem_line[] = {
  "RTS",
  "CTS",
  "DTR",
  "DSR",
  "DCD",
  "RI"
};

static const char *str_ret[] = {
  "ARM_DRIVER_OK",
  "ARM_DRIVER_ERROR",
  "ARM_DRIVER_ERROR_BUSY",
  "ARM_DRIVER_ERROR_TIMEOUT",
  "ARM_DRIVER_ERROR_UNSUPPORTED",
  "ARM_DRIVER_ERROR_PARAMETER",
  "ARM_DRIVER_ERROR_SPECIFIC",
  "ARM_USART_ERROR_MODE",
  "ARM_USART_ERROR_BAUDRATE",
  "ARM_USART_ERROR_DATA_BITS",
  "ARM_USART_ERROR_PARITY",
  "ARM_USART_ERROR_STOP_BITS",
  "ARM_USART_ERROR_FLOW_CONTROL",
  "ARM_USART_ERROR_CPOL",
  "ARM_USART_ERROR_CPHA"
};

// Local functions
#if (USART_SERVER_USED == 1)            // If Test Mode USART Server is selected
static int32_t  ComConfigDefault       (void);
static int32_t  ComSendCommand         (const void *data_out, uint32_t len);
static int32_t  ComReceiveResponse     (      void *data_in,  uint32_t len);

static int32_t  CmdGetVer              (void);
static int32_t  CmdGetCap              (void);
static int32_t  CmdSetBufTx            (char pattern);
static int32_t  CmdSetBufRx            (char pattern);
static int32_t  CmdGetBufRx            (uint32_t len);
static int32_t  CmdSetCom              (uint32_t mode, uint32_t data_bits, uint32_t parity, uint32_t stop_bits, uint32_t flow_control, uint32_t cpol, uint32_t cpha, uint32_t baudrate);
static int32_t  CmdXfer                (uint32_t dir,  uint32_t num,       uint32_t delay,  uint32_t timeout,   uint32_t num_cts);
static int32_t  CmdGetCnt              (void);
static int32_t  CmdSetBrk              (uint32_t delay, uint32_t duration);
static int32_t  CmdGetBrk              (void);
static int32_t  CmdSetMdm              (uint32_t mdm_ctrl, uint32_t delay, uint32_t duration);
static int32_t  CmdGetMdm              (void);

static int32_t  ServerInit             (void);
static int32_t  ServerCheck            (uint32_t mode, uint32_t data_bits, uint32_t parity, uint32_t stop_bits, uint32_t flow_control, uint32_t modem_line, uint32_t baudrate);
#endif

static int32_t  IsNotLoopback          (void);
static int32_t  IsNotSync              (void);
static int32_t  IsNotAsync             (void);
static int32_t  IsNotSyncMaster        (void);
static int32_t  IsNotSingleWire        (void);

static uint32_t DataBitsToBytes        (uint32_t data_bits);
static int32_t  DriverInit             (void);
static int32_t  BuffersCheck           (void);
static int32_t  DriverCheck            (uint32_t mode, uint32_t flow_control, uint32_t modem_line_mask);

static void USART_DataExchange_Operation (uint32_t operation, uint32_t mode, uint32_t data_bits, uint32_t parity, uint32_t stop_bits, uint32_t flow_control, uint32_t cpol, uint32_t cpha, uint32_t baudrate, uint32_t num);

// Helper functions

/*
  \fn            void USART_DrvEvent (uint32_t evt)
  \brief         Store event(s) into a global variable.
  \detail        This is a callback function called by the driver upon an event(s).
  \param[in]     evt            USART event
  \return        none
*/
static void USART_DrvEvent (uint32_t evt) {
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
  if (data_bits > 8U) {
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

  if (drv->Initialize    (USART_DrvEvent) == ARM_DRIVER_OK) {
    if (drv->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK) {
      return EXIT_SUCCESS;
    }
  }

  TEST_FAIL_MESSAGE("[FAILED] USART driver initialize or power-up. Check driver Initialize and PowerControl functions! Test aborted!");

  return EXIT_FAILURE;
}

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

#if (USART_SERVER_USED == 1)
  return EXIT_SUCCESS;
#else
  TEST_MESSAGE("[WARNING] Test not supported in Loopback Test Mode! Test not executed!");
  return EXIT_FAILURE;
#endif
}

/*
  \fn            static int32_t IsNotSync (void)
  \brief         Check if Synchronous Slave/Master mode is not selected as default mode.
  \detail        This function is used to skip executing a test if it is not supported 
                 in Synchronous mode.
  \return        execution status
                   - EXIT_SUCCESS: Synchronous mode is not selected
                   - EXIT_FAILURE: Synchronous mode is selected
*/
static int32_t IsNotSync (void) {

#if ((USART_CFG_DEF_MODE != MODE_SYNCHRONOUS_MASTER) && (USART_CFG_DEF_MODE != MODE_SYNCHRONOUS_SLAVE))
  return EXIT_SUCCESS;
#else
  TEST_MESSAGE("[WARNING] Test not supported for Synchronous Mode! Test not executed!");
  return EXIT_FAILURE;
#endif
}

/*
  \fn            static int32_t IsNotSyncMaster (void)
  \brief         Check if Synchronous Master mode is not selected as default mode.
  \detail        This function is used to skip executing a test if it is not supported 
                 in Synchronous Master mode.
  \return        execution status
                   - EXIT_SUCCESS: Synchronous Master mode is not selected
                   - EXIT_FAILURE: Synchronous Master mode is selected
*/
static int32_t IsNotSyncMaster (void) {

#if (USART_CFG_DEF_MODE != MODE_SYNCHRONOUS_MASTER)
  return EXIT_SUCCESS;
#else
  TEST_MESSAGE("[WARNING] Test not supported for Synchronous Master Mode! Test not executed!");
  return EXIT_FAILURE;
#endif
}

/*
  \fn            static int32_t IsNotAsync (void)
  \brief         Check if Asynchronous/Single-wire/IrDA modes are not selected as default mode.
  \detail        This function is used to skip executing a test if it is not supported 
                 in Asynchronous/Single-wire/IrDA mode.
  \return        execution status
                   - EXIT_SUCCESS: Asynchronous/Single-wire/IrDA mode is not selected
                   - EXIT_FAILURE: Asynchronous/Single-wire/IrDA mode is selected
*/
static int32_t IsNotAsync (void) {

#if ((USART_CFG_DEF_MODE != MODE_ASYNCHRONOUS) && (USART_CFG_DEF_MODE != MODE_SINGLE_WIRE) && (USART_CFG_DEF_MODE != MODE_IRDA))
  return EXIT_SUCCESS;
#else
  TEST_MESSAGE("[WARNING] Test not supported for Asynchronous/Single-wire/IrDA Mode! Test not executed!");
  return EXIT_FAILURE;
#endif
}

/*
  \fn            static int32_t IsNotSingleWire (void)
  \brief         Check if Single-wire mode is not selected as default mode.
  \detail        This function is used to skip executing a test if it is not supported 
                 in Single-wire mode.
  \return        execution status
                   - EXIT_SUCCESS: Single-wire mode is not selected
                   - EXIT_FAILURE: Single-wire mode is selected
*/
static int32_t IsNotSingleWire (void) {

#if (USART_CFG_DEF_MODE != MODE_SINGLE_WIRE)
  return EXIT_SUCCESS;
#else
  TEST_MESSAGE("[WARNING] Test not supported for Single-wire Mode! Test not executed!");
  return EXIT_FAILURE;
#endif
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

/*
  \fn            static int32_t DriverCheck (uint32_t mode, uint32_t flow_control, uint32_t modem_line_mask, uint32_t baudrate)
  \brief         Check if USART Driver supports desired settings.
  \param[in]     mode           mode:
                                  - value 1 = Asynchronous
                                  - value 2 = Synchronous Master
                                  - value 3 = Synchronous Slave
                                  - value 4 = Single Wire
                                  - value 5 = IrDA
                                  - value 6 = Smart Card
  \param[in]     flow_control   flow control:
                                  - value 0 = None
                                  - value 1 = CTS
                                  - value 2 = RTS
                                  - value 3 = RTS/CTS
  \param[in]     modem_line_mask  modem line mask:
                                  - bit 0. = RTS
                                  - bit 1. = CTS
                                  - bit 2. = DTR
                                  - bit 3. = DSR
                                  - bit 4. = DCD
                                  - bit 5. = RI
  \return        execution status
                   - EXIT_SUCCESS: USART Driver supports desired settings
                   - EXIT_FAILURE: USART Driver does not support desired settings
*/
static int32_t DriverCheck (uint32_t mode, uint32_t flow_control, uint32_t modem_line_mask) {
  int32_t ret;

  ret = EXIT_SUCCESS;

  switch (mode) {
    case 1:                             // Asynchronous
      if (drv_cap.asynchronous == 0U) {
        ret = EXIT_FAILURE;
      }
      break;
    case 2:                             // Synchronous master
      if (drv_cap.synchronous_master == 0U) {
        ret = EXIT_FAILURE;
      }
      break;
    case 3:                             // Synchronous slave
      if (drv_cap.synchronous_slave == 0U) {
        ret = EXIT_FAILURE;
      }
      break;
    case 4:                             // Single-wire
      if (drv_cap.single_wire == 0U) {
        ret = EXIT_FAILURE;
      }
      break;
    case 5:                             // IrDA
      if (drv_cap.irda == 0U) {
        ret = EXIT_FAILURE;
      }
      break;
    case 6:                             // Samrt Card
      if (drv_cap.irda == 0U) {
        ret = EXIT_FAILURE;
      }
      break;
    default:
      ret = EXIT_FAILURE;
      break;
  }

  if (ret != EXIT_SUCCESS) {
    // If USART Driver does not support desired mode
    if (mode <= 6U) {
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] USART Driver does not support %s mode! Test aborted!", str_mode[mode]);
    } else {
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] USART Driver does not support unknown mode! Test aborted!");
    }
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }

  switch (flow_control) {
    case 0:                             // None
      break;
    case 1:                             // CTS
      if (drv_cap.flow_control_cts == 0U) {
        ret = EXIT_FAILURE;
      }
      break;
    case 2:                             // RTS
      if (drv_cap.flow_control_rts == 0U) {
        ret = EXIT_FAILURE;
      }
      break;
    case 3:                             // RTS/CTS
      if ((drv_cap.flow_control_cts == 0U) || (drv_cap.flow_control_rts == 0U)) {
        ret = EXIT_FAILURE;
      }
      break;
    default:
      ret = EXIT_FAILURE;
      break;
  }

  if (ret != EXIT_SUCCESS) {
    // If USART Driver does not support desired flow control
    if (mode <= 3U) {
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] USART Driver does not support %s flow control! Test aborted!", str_flow_control[flow_control]);
    } else {
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] USART Driver does not support unknown flow control! Test aborted!");
    }
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }

  if ((modem_line_mask & 1U) != 0U) {
    if (drv_cap.rts == 0U) {
      TEST_MESSAGE("[FAILED] USART Driver does not support RTS modem line! Test aborted!");
      return EXIT_FAILURE;
    }
  }
  if ((modem_line_mask & (1U << 1)) != 0U) {
    if (drv_cap.cts == 0U) {
      TEST_MESSAGE("[FAILED] USART Driver does not support CTS modem line! Test aborted!");
      return EXIT_FAILURE;
    }
  }
  if ((modem_line_mask & (1U << 2)) != 0U) {
    if (drv_cap.dtr == 0U) {
      TEST_MESSAGE("[FAILED] USART Driver does not support DTR modem line! Test aborted!");
      return EXIT_FAILURE;
    }
  }
  if ((modem_line_mask & (1U << 3)) != 0U) {
    if (drv_cap.dsr == 0U) {
      TEST_MESSAGE("[FAILED] USART Driver does not support DSR modem line! Test aborted!");
      return EXIT_FAILURE;
    }
  }
  if ((modem_line_mask & (1U << 4)) != 0U) {
    if (drv_cap.dcd == 0U) {
      TEST_MESSAGE("[FAILED] USART Driver does not support DCD modem line! Test aborted!");
      return EXIT_FAILURE;
    }
  }
  if ((modem_line_mask & (1U << 5)) != 0U) {
    if (drv_cap.ri == 0U) {
      TEST_MESSAGE("[FAILED] USART Driver does not support RI modem line! Test aborted!");
      return EXIT_FAILURE;
    }
  }

  return ret;
}

#if (USART_SERVER_USED == 1)            // If Test Mode USART Server is selected

/*
  \fn            static int32_t ComConfigDefault (void)
  \brief         Configure USART Communication Interface to USART Server default communication configuration.
  \return        execution status
                   - EXIT_SUCCESS: Default configuration set successfully
                   - EXIT_FAILURE: Default configuration failed
*/
static int32_t ComConfigDefault (void) {
  int32_t ret;

  ret = EXIT_SUCCESS;

  if (drv->Control(((USART_CFG_SRV_MODE         << ARM_USART_CONTROL_Pos)      & ARM_USART_CONTROL_Msk)      |
                     DRIVER_DATA_BITS(USART_CFG_SRV_DATA_BITS)                                               |
                   ((USART_CFG_SRV_PARITY       << ARM_USART_PARITY_Pos)       & ARM_USART_PARITY_Msk)       |
                   ((USART_CFG_SRV_STOP_BITS    << ARM_USART_STOP_BITS_Pos)    & ARM_USART_STOP_BITS_Msk)    |
                   ((USART_CFG_SRV_FLOW_CONTROL << ARM_USART_FLOW_CONTROL_Pos) & ARM_USART_FLOW_CONTROL_Msk) ,
                     USART_CFG_SRV_BAUDRATE) != ARM_DRIVER_OK) {
    ret = EXIT_FAILURE;
  }
    if (drv->Control(ARM_USART_CONTROL_TX, 1U) != ARM_DRIVER_OK) {
      ret = EXIT_FAILURE;
    }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Configure communication interface to USART Server default settings. Check driver Control function! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t ComSendCommand (const void *data_out, uint32_t num)
  \brief         Send command to USART Server.
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
  num = (len + DataBitsToBytes(USART_CFG_SRV_DATA_BITS) - 1U) / DataBitsToBytes(USART_CFG_SRV_DATA_BITS);

  ret = ComConfigDefault();

  if (ret == EXIT_SUCCESS) {
    (void)osEventFlagsClear(event_flags, 0x7FFFFFFFU); 	
    if (drv->Control(ARM_USART_CONTROL_TX, 1U) != ARM_DRIVER_OK) {
      ret = EXIT_FAILURE;
    }
    if (ret == EXIT_SUCCESS) {
      if (drv->Send(data_out, num) != ARM_DRIVER_OK) {
        ret = EXIT_FAILURE;
      }
      if (ret == EXIT_SUCCESS) {
        if (drv_cap.event_tx_complete != 0U) {
          // If ARM_USART_EVENT_TX_COMPLETE is supported, wait for it
          flags = osEventFlagsWait(event_flags, ARM_USART_EVENT_TX_COMPLETE, osFlagsWaitAny, USART_CFG_SRV_CMD_TOUT);
          if (((flags & 0x80000000U) != 0U) ||
              ((flags & ARM_USART_EVENT_TX_COMPLETE) == 0U)) {
            ret = EXIT_FAILURE;
          }
        } else {
          // Otherwise wait for ARM_USART_EVENT_SEND_COMPLETE flag
          flags = osEventFlagsWait(event_flags, ARM_USART_EVENT_SEND_COMPLETE, osFlagsWaitAny, USART_CFG_SRV_CMD_TOUT);
          if (((flags & 0x80000000U) != 0U) ||
              ((flags & ARM_USART_EVENT_SEND_COMPLETE) == 0U)) {
            ret = EXIT_FAILURE;
          }

          if (ret == EXIT_SUCCESS) {
            // If completed event was signaled, wait for all data to be sent
            for (tout = USART_CFG_SRV_CMD_TOUT; tout != 0U; tout--) {
              if ((drv->GetTxCount() == len) && (drv->GetStatus().tx_busy == 0U)) { 
                break;
              }
              (void)osDelay(1U);
            }
          }
        }
      }
      if (ret == EXIT_FAILURE) {
        (void)drv->Control(ARM_USART_ABORT_SEND, 0U);
      }
    }
  }
  (void)drv->Control(ARM_USART_CONTROL_TX, 0U);

  return ret;
}

/**
  \fn            static int32_t ComReceiveResponse (void *data_in, uint32_t num)
  \brief         Receive response from USART Server.
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
  num = (len + DataBitsToBytes(USART_CFG_SRV_DATA_BITS) - 1U) / DataBitsToBytes(USART_CFG_SRV_DATA_BITS);

  ret = ComConfigDefault();

  if (ret == EXIT_SUCCESS) {
    (void)osEventFlagsClear(event_flags, 0x7FFFFFFFU); 	
    if (drv->Control(ARM_USART_CONTROL_RX, 1U) != ARM_DRIVER_OK) {
      ret = EXIT_FAILURE;
    }
    if (ret == EXIT_SUCCESS) {
      if (drv->Receive(data_in, num) != ARM_DRIVER_OK) {
        ret = EXIT_FAILURE;
      }
      if (ret == EXIT_SUCCESS) {
        flags = osEventFlagsWait(event_flags, ARM_USART_EVENT_RECEIVE_COMPLETE, osFlagsWaitAny, USART_CFG_SRV_CMD_TOUT);
        if (((flags & 0x80000000U) != 0U) ||
            ((flags & ARM_USART_EVENT_RECEIVE_COMPLETE) == 0U)) {
          ret = EXIT_FAILURE;
        }
      }
      if (ret == EXIT_FAILURE) {
        drv->Control(ARM_USART_ABORT_RECEIVE, 0U);
      }
    }
  }
  (void)drv->Control(ARM_USART_CONTROL_RX, 0U);

  return ret;
}

/**
  \fn            static int32_t CmdGetVer (void)
  \brief         Get version from USART Server and check that it is valid.
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

  memset(&usart_serv_ver, 0, sizeof(usart_serv_ver));

  // Send "GET VER" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  memcpy(ptr_tx_buf, "GET VER", 7);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret == EXIT_SUCCESS) {
    // Receive response to "GET VER" command from USART Server
    memset(ptr_rx_buf, (int32_t)'?', RESP_GET_VER_LEN);
    ret = ComReceiveResponse(ptr_rx_buf, RESP_GET_VER_LEN);
    (void)osDelay(10U);
  }

  // Parse version
  if (ret == EXIT_SUCCESS) {
    // Parse major
    ptr_str = (const char *)ptr_rx_buf;
    if (sscanf(ptr_str, "%hhx", &val8) == 1) {
      usart_serv_ver.major = val8;
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
        usart_serv_ver.minor = val8;
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
        usart_serv_ver.patch = val16;
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
  \brief         Get capabilities from USART Server.
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

  memset(&usart_serv_cap, 0, sizeof(usart_serv_cap));

  // Send "GET CAP" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  memcpy(ptr_tx_buf, "GET CAP", 7);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret == EXIT_SUCCESS) {
    (void)osDelay(20U);                 // Give USART Server 20 ms to auto-detect capabilities

    // Receive response to "GET CAP" command from USART Server
    memset(ptr_rx_buf, (int32_t)'?', RESP_GET_CAP_LEN);
    ret = ComReceiveResponse(ptr_rx_buf, RESP_GET_CAP_LEN);
    (void)osDelay(10U);
  }

  // Parse capabilities
  if (ret == EXIT_SUCCESS) {
    // Parse mode mask
    ptr_str = (const char *)ptr_rx_buf;
    if (sscanf(ptr_str, "%hhx", &val8) == 1) {
      usart_serv_cap.mode_mask = val8;
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse data bit mask
    ptr_str = strstr(ptr_str, ",");     // Find ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%x", &val32) == 1) {
        usart_serv_cap.db_mask = val32;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse parity mask
    ptr_str = strstr(ptr_str, ",");     // Find next ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%x", &val32) == 1) {
        usart_serv_cap.parity_mask = val32;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse stop bit mask
    ptr_str = strstr(ptr_str, ",");     // Find next ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%x", &val32) == 1) {
        usart_serv_cap.sb_mask = val32;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse flow control mask
    ptr_str = strstr(ptr_str, ",");     // Find next ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%x", &val32) == 1) {
        usart_serv_cap.fc_mask = val32;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse modem lines mask
    ptr_str = strstr(ptr_str, ",");     // Find next ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%x", &val32) == 1) {
        usart_serv_cap.ml_mask = val32;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse minimum baudrate
    ptr_str = strstr(ptr_str, ",");     // Find next ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%u", &val32) == 1) {
        usart_serv_cap.br_min = val32;
      } else {
        ret = EXIT_FAILURE;
      }
    } else {
      ret = EXIT_FAILURE;
    }
  }
  if ((ret == EXIT_SUCCESS) && (ptr_str != NULL)) {
    // Parse maximum baudrate
    ptr_str = strstr(ptr_str, ",");     // Find next ','
    if (ptr_str != NULL) {
      ptr_str++;                        // Skip ','
      if (sscanf(ptr_str, "%u", &val32) == 1) {
        usart_serv_cap.br_max = val32;
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
  \brief         Set Tx buffer of USART Server to pattern.
  \param[in]     pattern        Pattern to fill the buffer with
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t CmdSetBufTx (char pattern) {
  int32_t ret;

  // Send "SET BUF TX" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "SET BUF TX,0,%02X", (int32_t)pattern);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
  (void)osDelay(10U);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set Tx buffer on USART Server. Check USART Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdSetBufRx (char pattern)
  \brief         Set Rx buffer of USART Server to pattern.
  \param[in]     pattern        Pattern to fill the buffer with
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t CmdSetBufRx (char pattern) {
  int32_t ret;

  // Send "SET BUF RX" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "SET BUF RX,0,%02X", (int32_t)pattern);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
  (void)osDelay(10U);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set Rx buffer on USART Server. Check USART Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdGetBufRx (void)
  \brief         Get Rx buffer from USART Server (into global array pointed to by ptr_rx_buf).
  \param[in]     len            Number of bytes to read from Rx buffer
  \return        execution status
                   - EXIT_SUCCESS: Command sent and response received successfully
                   - EXIT_FAILURE: Command send or response reception failed
*/
static int32_t CmdGetBufRx (uint32_t len) {
  int32_t ret;

  // Send "GET BUF RX" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "GET BUF RX,%i", len);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret == EXIT_SUCCESS) {
    // Receive response to "GET BUF RX" command from USART Server
    memset(ptr_rx_buf, (int32_t)'?', len);
    ret = ComReceiveResponse(ptr_rx_buf, len);
    (void)osDelay(10U);
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Get Rx buffer from USART Server. Check USART Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdSetCom (uint32_t mode, uint32_t data_bits, uint32_t parity, uint32_t stop_bits, uint32_t flow_control, uint32_t cpol, uint32_t cpha, uint32_t baudrate)
  \brief         Set communication parameters on USART Server for next XFER command.
  \param[in]     mode           mode:
                                  - value 0 = Asynchronous
                                  - value 1 = Synchronous Master
                                  - value 2 = Synchronous Slave
                                  - value 3 = Single Wire
                                  - value 4 = IrDA
                                  - value 5 = Smart Card
  \param[in]     data_bits      data bits:
                                  - values 5 to 9
  \param[in]     parity         parity:
                                  - value 0 = None
                                  - value 1 = Even
                                  - value 2 = Odd
  \param[in]     stop_bits      stop bits:
                                  - value 0 = 1 Stop Bit
                                  - value 1 = 2 Stop Bits
                                  - value 2 = 1.5 Stop Bits
                                  - value 3 = 0.5 Stop Bits
  \param[in]     flow_control   flow control:
                                  - value 0 = None
                                  - value 1 = CTS
                                  - value 2 = RTS
                                  - value 3 = RTS/CTS
  \param[in]     cpol           clock polarity:
                                  - value 0 = Data are captured on rising edge
                                  - value 1 = Data are captured on falling edge
  \param[in]     cpha           clock phase:
                                  - value 0 = Sample on first (leading) edge
                                  - value 1 = Sample on second (trailing) edge
  \param[in]     baudrate       baudrate in bauds
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t CmdSetCom (uint32_t mode, uint32_t data_bits, uint32_t parity, uint32_t stop_bits, uint32_t flow_control, uint32_t cpol, uint32_t cpha, uint32_t baudrate) {
  int32_t ret, stat;

  // Send "SET COM" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  stat = snprintf((char *)ptr_tx_buf, CMD_LEN, "SET COM %i,%i,%i,%i,%i,%i,%i,%i", mode, data_bits, parity, stop_bits, flow_control, cpol, cpha, baudrate);
  if ((stat > 0) && (stat < CMD_LEN)) {
    ret = ComSendCommand(ptr_tx_buf, CMD_LEN);
    (void)osDelay(10U);
  } else {
    ret = EXIT_FAILURE;
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set communication settings on USART Server. Check USART Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdXfer (uint32_t dir, uint32_t num, uint32_t delay, uint32_t timeout, uint32_t num_cts)
  \brief         Activate transfer on USART Server.
  \param[in]     dir            direction of transfer 
                                  - 0 = Send (Tx)
                                  - 1 = Receive (Rx)
                                  - 2 = Transfer (simultaneous Tx and Rx (in synchronous mode only))
  \param[in]     num            number of items (according CMSIS USART driver specification)
  \param[in]     delay          initial delay, in milliseconds, before starting requested operation 
                                (0xFFFFFFFF = delay not used)
  \param[in]     timeout        timeout in milliseconds, after delay, if delay is specified
  \param[in]     num_cts        number of items after which CTS line should be de-activated
                                  - 0 = no CTS deactivation
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t CmdXfer (uint32_t dir, uint32_t num, uint32_t delay, uint32_t timeout, uint32_t num_cts) {
  int32_t ret;

  // Send "XFER" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  if (num_cts != 0U) {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i,%i,%i,%i,%i", dir, num, delay, timeout, num_cts);
  } else if ((delay != osWaitForever) && (timeout != 0U)) {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i,%i,%i,%i",    dir, num, delay, timeout);
  } else if (delay != osWaitForever) {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i,%i,%i",       dir, num, delay);
  } else {
    (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "XFER %i,%i",          dir, num);
  }
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Activate transfer on USART Server. Check USART Server! Test aborted!");
  }

  return ret;
}

/*
  \fn            static int32_t CmdGetCnt (void)
  \brief         Get XFER command Tx/Rx count from USART Server.
  \return        execution status
                   - EXIT_SUCCESS: Operation successful
                   - EXIT_FAILURE: Operation failed
*/
static int32_t CmdGetCnt (void) {
  int32_t     ret;
  const char *ptr_str;
  uint32_t    val32;

  xfer_count = 0U;

  // Send "GET CNT" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  memcpy(ptr_tx_buf, "GET CNT", 7);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret == EXIT_SUCCESS) {
    // Receive response to "GET CNT" command from USART Server
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
    TEST_FAIL_MESSAGE("[FAILED] Get count from USART Server. Check USART Server! Test aborted!");
  }

  return ret;
}

/*
  \fn            static int32_t CmdSetBrk (uint32_t delay, uint32_t duration)
  \brief         Request USART Server to send break signal.
  \param[in]     delay:         initial delay, in milliseconds, before start of break signaling
  \param[in]     duration:      duration, in milliseconds, of break signaling
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t CmdSetBrk (uint32_t delay, uint32_t duration) {
  int32_t ret;

  // Send "SET BRK" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "SET BRK %i,%i", delay, duration);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set break on USART Server. Check USART Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdGetBrk (void)
  \brief         Get information on Break state from USART Server.
  \return        execution status
                   - EXIT_SUCCESS: Command sent and response received successfully
                   - EXIT_FAILURE: Command send or response reception failed
*/
static int32_t CmdGetBrk (void) {
  int32_t ret;

  // Send "GET BRK" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  memcpy(ptr_tx_buf, "GET BRK", 7);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret == EXIT_SUCCESS) {
    // Receive response to "GET BRK" command from USART Server
    memset(ptr_rx_buf, (int32_t)'?', RESP_GET_BRK_LEN);
    ret = ComReceiveResponse(ptr_rx_buf, RESP_GET_BRK_LEN);
    (void)osDelay(10U);
  }

  // Store modem status to global variable
  if (ret == EXIT_SUCCESS) {
    break_status = ptr_rx_buf[0] - '0';
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Get break state from USART Server. Check USART Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdSetMdm (uint32_t mdm_ctrl, uint32_t delay, uint32_t duration)
  \brief         Set modem lines on USART Server.
  \param[in]     mdm_ctrl:      modem control line states:
                                  - bit 0.: RTS state
                                  - bit 1.: DTS state
                                  - bit 2.: Line to USART Client's DCD state
                                  - bit 3.: Line to USART Client's RI state
  \param[in]     delay:         initial delay, in milliseconds, before start of controlling modem lines
  \param[in]     duration:      duration, in milliseconds, of controlling modem lines
  \return        execution status
                   - EXIT_SUCCESS: Command sent successfully
                   - EXIT_FAILURE: Command send failed
*/
static int32_t CmdSetMdm (uint32_t mdm_ctrl, uint32_t delay, uint32_t duration) {
  int32_t ret;

  // Send "SET MDM" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  (void)snprintf((char *)ptr_tx_buf, CMD_LEN, "SET MDM %x,%i,%i", mdm_ctrl, delay, duration);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Set modem control on USART Server. Check USART Server! Test aborted!");
  }

  return ret;
}

/**
  \fn            static int32_t CmdGetMdm (void)
  \brief         Get information on modem lines current state from USART Server.
  \return        execution status
                   - EXIT_SUCCESS: Command sent and response received successfully
                   - EXIT_FAILURE: Command send or response reception failed
*/
static int32_t CmdGetMdm (void) {
  int32_t ret;

  // Send "GET MDM" command to USART Server
  memset(ptr_tx_buf, 0, CMD_LEN);
  memcpy(ptr_tx_buf, "GET MDM", 7);
  ret = ComSendCommand(ptr_tx_buf, CMD_LEN);

  if (ret == EXIT_SUCCESS) {
    // Receive response to "GET MDM" command from USART Server
    memset(ptr_rx_buf, (int32_t)'?', RESP_GET_MDM_LEN);
    ret = ComReceiveResponse(ptr_rx_buf, RESP_GET_MDM_LEN);
    (void)osDelay(10U);
  }

  // Store modem status to global variable
  if (ret == EXIT_SUCCESS) {
    modem_status = ptr_rx_buf[0] - '0';
  }

  if (ret != EXIT_SUCCESS) {
    TEST_FAIL_MESSAGE("[FAILED] Get modem lines state from USART Server. Check USART Server! Test aborted!");
  }

  return ret;
}

/*
  \fn            static int32_t ServerInit (void)
  \brief         Initialize communication with USART Server, get version and capabilities.
  \return        execution status
                   - EXIT_SUCCESS: USART Server initialized successfully
                   - EXIT_FAILURE: USART Server initialization failed
*/
static int32_t ServerInit (void) {

  if (server_ok == -1) {                // If -1, means it was not yet checked
    server_ok = 1;

    if (drv->Control(((USART_CFG_SRV_MODE         << ARM_USART_CONTROL_Pos)      & ARM_USART_CONTROL_Msk)      |
                       DRIVER_DATA_BITS(USART_CFG_SRV_DATA_BITS)                                               |
                     ((USART_CFG_SRV_PARITY       << ARM_USART_PARITY_Pos)       & ARM_USART_PARITY_Msk)       |
                     ((USART_CFG_SRV_STOP_BITS    << ARM_USART_STOP_BITS_Pos)    & ARM_USART_STOP_BITS_Msk)    |
                     ((USART_CFG_SRV_FLOW_CONTROL << ARM_USART_FLOW_CONTROL_Pos) & ARM_USART_FLOW_CONTROL_Msk) ,
                       USART_CFG_SRV_BAUDRATE) != ARM_DRIVER_OK) {
      server_ok = 0;
    }
    if (server_ok == 0) {
      TEST_GROUP_INFO("Failed to configure communication interface to USART Server default settings.\n"\
                      "Driver must support basic settings used for communication with USART Server!");
    }

    if (server_ok == 1) {
      (void)osDelay(10U);
      if (CmdGetVer() != EXIT_SUCCESS) {
        TEST_GROUP_INFO("Failed to Get version from USART Server.\nCheck USART Server!\n");
        server_ok = 0;
      }
    }

    if (server_ok == 1) {
      if (usart_serv_ver.major == 0U) { 
        TEST_GROUP_INFO("USART Server version must be 1.0.0. or higher.\nUpdate USART Server to newer version!\n");
        server_ok = 0;
      }
    }

    if (server_ok == 1) {
      if (CmdGetCap() != EXIT_SUCCESS) {
        TEST_GROUP_INFO("Failed to Get capabilities from USART Server.\nCheck USART Server!\n");
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
  \fn            static int32_t ServerCheck (uint32_t mode, uint32_t data_bits, uint32_t parity, uint32_t stop_bits, uint32_t flow_control, uint32_t modem_line, uint32_t baudrate)
  \brief         Check if USART Server is functional and if it supports desired settings.
  \detail        Parameters describe settings required to test, so server must support complementary sattings.
                 For example to test RTS line server must support CTS line.
  \param[in]     mode           mode (expected to be tested):
                                  - value 1 = Asynchronous
                                  - value 2 = Synchronous Master
                                  - value 3 = Synchronous Slave
                                  - value 4 = Single Wire
                                  - value 5 = IrDA
                                  - value 6 = Smart Card
  \param[in]     data_bits      data bits (5 .. 9)
  \param[in]     parity         parity:
                                  - value 0 = None
                                  - value 1 = Even
                                  - value 2 = Odd
  \param[in]     stop_bits      stop bits:
                                  - value 0 = 1 Stop Bit
                                  - value 1 = 2 Stop Bits
                                  - value 2 = 1.5 Stop Bits
                                  - value 3 = 0.5 Stop Bits
  \param[in]     flow_control   flow control:
                                  - value 0 = None
                                  - value 1 = CTS
                                  - value 2 = RTS
                                  - value 3 = RTS/CTS
  \param[in]     modem_line_mask  modem line mask:
                                  - bit 0. = RTS
                                  - bit 1. = CTS
                                  - bit 2. = DTR
                                  - bit 3. = DSR
                                  - bit 4. = DCD
                                  - bit 5. = RI
  \param[in]     baudrate       baudrate in bauds
  \return        execution status
                   - EXIT_SUCCESS: USART Server supports desired settings
                   - EXIT_FAILURE: USART Server does not support desired settings
*/
static int32_t ServerCheck (uint32_t mode, uint32_t data_bits, uint32_t parity, uint32_t stop_bits, uint32_t flow_control, uint32_t modem_line_mask, uint32_t baudrate) {
  uint32_t srv_mode, srv_flow_control, srv_modem_line_mask;

  if (server_ok == 0) {
    TEST_FAIL_MESSAGE("[FAILED] USART Server status. Check USART Server! Test aborted!");
    return EXIT_FAILURE;
  }

#if   (USART_CFG_SRV_MODE == MODE_ASYNCHRONOUS)
    if ((mode == MODE_SINGLE_WIRE) || (mode == MODE_IRDA)) {
      TEST_MESSAGE("[FAILED] USART Server mode Asynchronous does not support requested test mode! Test aborted!");
      return EXIT_FAILURE;
    }
#elif (USART_CFG_SRV_MODE == MODE_SINGLE_WIRE)
    if (mode != MODE_SINGLE_WIRE) {
      TEST_MESSAGE("[FAILED] USART Server mode Single-wire does not support requested test mode! Test aborted!");
      return EXIT_FAILURE;
    }
#elif (USART_CFG_SRV_MODE == MODE_SINGLE_IRDA)
    if (mode != MODE_SINGLE_IRDA) {
      TEST_MESSAGE("[FAILED] USART Server mode IrDA does not support requested test mode! Test aborted!");
      return EXIT_FAILURE;
    }
#else
    TEST_MESSAGE("[FAILED] USART Server mode unknown! Test aborted!");
    return EXIT_FAILURE;
#endif

  srv_mode = mode;
  if (mode == MODE_SYNCHRONOUS_MASTER) {        // If mode to be tested is Synchro Master then server must support Slave
    srv_mode = MODE_SYNCHRONOUS_SLAVE;
  } else if (mode == MODE_SYNCHRONOUS_SLAVE) {  // If mode to be tested is Synchro Slave  then server must support Master
    srv_mode = MODE_SYNCHRONOUS_MASTER;
  }

  srv_flow_control = flow_control;
  if (flow_control == FLOW_CONTROL_RTS) {
    srv_flow_control = FLOW_CONTROL_CTS;
  }
  if (flow_control == FLOW_CONTROL_CTS) {
    srv_flow_control = FLOW_CONTROL_RTS;
  }

  srv_modem_line_mask = 0U;
  if ((modem_line_mask & RTS_AVAILABLE   ) != 0U) {
    srv_modem_line_mask |= CTS_AVAILABLE;
  }
  if ((modem_line_mask & CTS_AVAILABLE   ) != 0U) {
    srv_modem_line_mask |= RTS_AVAILABLE;
  }
  if ((modem_line_mask & DTR_AVAILABLE   ) != 0U) {
    srv_modem_line_mask |= DSR_AVAILABLE;
  }
  if ((modem_line_mask & DSR_AVAILABLE   ) != 0U) {
    srv_modem_line_mask |= DTR_AVAILABLE;
  }
  if ((modem_line_mask & DCD_AVAILABLE) != 0U) {
    srv_modem_line_mask |= DCD_AVAILABLE;
  }
  if ((modem_line_mask & RI_AVAILABLE ) != 0U) {
    srv_modem_line_mask |= RI_AVAILABLE;
  }

  if ((usart_serv_cap.mode_mask & (1UL << (srv_mode - 1U))) == 0U) {
    // If USART Server does not support desired mode
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] USART Server does not support %s mode! Test aborted!", str_mode[mode]);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }
  if ((usart_serv_cap.db_mask & (1UL << (data_bits - 5U))) == 0U) {
    // If USART Server does not support desired data bits
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] USART Server does not support %i data bits! Test aborted!", data_bits);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }
  if ((usart_serv_cap.parity_mask & (1UL << parity)) == 0U) {
    // If USART Server does not support desired parity
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] USART Server does not support %s parity! Test aborted!", str_parity[parity]);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }
  if ((usart_serv_cap.sb_mask & (1UL << stop_bits)) == 0U) {
    // If USART Server does not support desired stop bits
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] USART Server does not support %s stop bits! Test aborted!", str_stop_bits[stop_bits]);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }
  if ((usart_serv_cap.fc_mask & (1UL << srv_flow_control)) == 0U) {
    // If USART Server does not support desired flow control
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] USART Server does not support %s flow control! Test aborted!", str_flow_control[flow_control]);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }
  if (srv_modem_line_mask != 0U) {
    if ((usart_serv_cap.ml_mask & srv_modem_line_mask) == 0U) {
      // If USART Server does not support desired modem line
      TEST_MESSAGE("[FAILED] USART Server does not support desired modem line! Test aborted!");
      return EXIT_FAILURE;
    }
  }
  if ((usart_serv_cap.br_min > baudrate) ||
      (usart_serv_cap.br_max < baudrate)) {
    // If USART Server does not support desired baudrate
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] USART Server does not support %i baudrate bus speed! Test aborted!", baudrate);
    TEST_MESSAGE(msg_buf);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#endif                                  // If Test Mode USART Server is selected

/*
  \fn            static int32_t SettingsCheck (uint32_t mode, uint32_t data_bits, uint32_t parity, uint32_t stop_bits, uint32_t flow_control, uint32_t modem_line, uint32_t baudrate)
  \brief         Check if Driver and USART Server (if used) is functional and if it supports desired settings.
  \detail        Parameters describe settings required to test, so server must support complementary sattings.
                 For example to test RTS line server must support CTS line.
  \param[in]     mode           mode (expected to be tested):
                                  - value 1 = Asynchronous
                                  - value 2 = Synchronous Master
                                  - value 3 = Synchronous Slave
                                  - value 4 = Single Wire
                                  - value 5 = IrDA
                                  - value 6 = Smart Card
  \param[in]     data_bits      data bits (5 .. 9)
  \param[in]     parity         parity:
                                  - value 0 = None
                                  - value 1 = Even
                                  - value 2 = Odd
  \param[in]     stop_bits      stop bits:
                                  - value 0 = 1 Stop Bit
                                  - value 1 = 2 Stop Bits
                                  - value 2 = 1.5 Stop Bits
                                  - value 3 = 0.5 Stop Bits
  \param[in]     flow_control   flow control:
                                  - value 0 = None
                                  - value 1 = CTS
                                  - value 2 = RTS
                                  - value 3 = RTS/CTS
  \param[in]     modem_line_mask  modem line mask:
                                  - bit 0. = RTS
                                  - bit 1. = CTS
                                  - bit 2. = DTR
                                  - bit 3. = DSR
                                  - bit 4. = DCD
                                  - bit 5. = RI
  \param[in]     baudrate       baudrate in bauds
  \return        execution status
                   - EXIT_SUCCESS: Driver and USART Server supports desired settings
                   - EXIT_FAILURE: Driver or USART Server does not support desired settings
*/
static int32_t SettingsCheck (uint32_t mode, uint32_t data_bits, uint32_t parity, uint32_t stop_bits, uint32_t flow_control, uint32_t modem_line_mask, uint32_t baudrate) {

  if (BuffersCheck()  != EXIT_SUCCESS) { 
    return EXIT_FAILURE;
  }

  if (DriverCheck (mode, flow_control, modem_line_mask) != EXIT_SUCCESS) { 
    return EXIT_FAILURE;
  }
#if  (USART_SERVER_USED == 1)
  if (ServerCheck (mode, data_bits, parity, stop_bits, flow_control, modem_line_mask, baudrate) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }
#endif

  return EXIT_SUCCESS;
}

/*
  \fn            void USART_DV_Initialize (void)
  \brief         Initialize testing environment for USART testing.
  \detail        This function is called by the driver validation framework before USART testing begins.
                 It initializes global variables and allocates memory buffers (from heap) used for the USART testing.
  \return        none
*/
void USART_DV_Initialize (void) {

  // Initialize global variables
  buffers_ok   = -1;
  server_ok    = -1;
  driver_ok    = -1;
  event        = 0U;
  duration     = 0xFFFFFFFFUL;
  systick_freq = osKernelGetSysTimerFreq();
  if (systick_freq == 0U) {
    // systick_freq must not be 0
    systick_freq = 1U;
  }
  ticks_per_ms = systick_freq / 1000U;

  memset(&usart_serv_cap, 0, sizeof(usart_serv_cap));
  memset(&msg_buf,        0, sizeof(msg_buf));

  // Allocate buffers for transmission, reception and comparison
  // (maximum size is incremented by 32 bytes to ensure that buffer can be aligned to 32 bytes)

  ptr_tx_buf_alloc = malloc(USART_BUF_MAX + 32U);
  if (((uint32_t)ptr_tx_buf_alloc & 31U) != 0U) {
    // If allocated memory is not 32 byte aligned, use next 32 byte aligned address for ptr_tx_buf
    ptr_tx_buf = (uint8_t *)((((uint32_t)ptr_tx_buf_alloc) + 31U) & (~31U));
  } else {
    // If allocated memory is 32 byte aligned, use it directly
    ptr_tx_buf = (uint8_t *)ptr_tx_buf_alloc;
  }
  ptr_rx_buf_alloc = malloc(USART_BUF_MAX + 32U);
  if (((uint32_t)ptr_rx_buf_alloc & 31U) != 0U) {
    ptr_rx_buf = (uint8_t *)((((uint32_t)ptr_rx_buf_alloc) + 31U) & (~31U));
  } else {
    ptr_rx_buf = (uint8_t *)ptr_rx_buf_alloc;
  }
  ptr_cmp_buf_alloc = malloc(USART_BUF_MAX + 32U);
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
                 " - Mode:            %s\n"\
                 " - Data bits:       %i\n"\
                 " - Parity:          %s\n"\
                 " - Stop bits:       %s\n"\
                 " - Flow control:    %s\n"\
                 " - Clock polarity:  %s\n"\
                 " - Clock phase:     %s\n"\
                 " - Bus speed:       %i bauds\n"\
                 " - Number of Items: %i",
                 str_test_mode   [USART_CFG_TEST_MODE],
                 str_mode        [USART_CFG_DEF_MODE],
                 USART_CFG_DEF_DATA_BITS,
                 str_parity      [USART_CFG_DEF_PARITY],
                 str_stop_bits   [USART_CFG_DEF_STOP_BITS],
                 str_flow_control[USART_CFG_DEF_FLOW_CONTROL],
                 str_cpol        [USART_CFG_DEF_CPOL],
                 str_cpha        [USART_CFG_DEF_CPHA],
                 USART_CFG_DEF_BAUDRATE,
                 USART_CFG_DEF_NUM);
  TEST_GROUP_INFO(msg_buf);

  drv_cap = drv->GetCapabilities();

#if (USART_SERVER_USED == 1)            // If Test Mode USART Server is selected
  // Test communication with USART Server
  int32_t  server_status;
  uint32_t str_len;

  // Test communication with USART Server
  if (drv->Initialize    (USART_DrvEvent) == ARM_DRIVER_OK) {
    if (drv->PowerControl(ARM_POWER_FULL) == ARM_DRIVER_OK) {
      server_status = ServerInit();
    }
  }
  (void)drv->PowerControl(ARM_POWER_OFF);
  (void)drv->Uninitialize();

//(void)snprintf(msg_buf, sizeof(msg_buf), "Server status:      %s\n", str_srv_status[server_status]);
//TEST_GROUP_INFO(msg_buf);
#endif
}

/*
  \fn            void USART_DV_Uninitialize (void)
  \brief         De-initialize testing environment after USART testing.
  \detail        This function is called by the driver validation framework after USART testing is finished.
                 It frees memory buffers used for the USART testing.
  \return        none
*/
void USART_DV_Uninitialize (void) {

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
\defgroup dv_usart USART Validation
\brief USART driver validation
\details
The USART validation performs the following tests:
- API interface compliance
- Data exchange with various speeds, transfer sizes and communication settings
- Modem lines operation
- Event signaling

Two Test Modes are available: <b>Loopback</b> and <b>USART Server</b>.

Test Mode : <b>Loopback</b>
---------------------------

This test mode allows only limited validation of the USART Driver.<br>
It is recommended that this test mode is used only as a proof that driver is 
good enough to be tested with the <b>USART Server</b>.

For this purpose following <b>Default settings</b> should be used:
 - Mode: Asynchronous
 - Data Bits: 8
 - Parity: None
 - Stop Bits: 1
 - Flow control: No
 - Clock Polarity: Clock Polarity 0
 - Clock Phase: Clock Phase 0
 - Baudrate: 115200
 - Number of Items: 32

To enable this mode of testing in the <b>DV_USART_Config.h</b> configuration file select the
<b>Configuration: Test Mode: Loopback</b> setting.

Required pin connection for the <b>Loopback</b> test mode:

\image html usart_loopback_pin_connections.png

\note In this mode following operations / settings cannot be tested:
 - synchronous slave or single-wire modes
 - operation of the Receive function
 - data content sent by the Send function
 - parity, stop bits, flow control, clock polarity / phase settings
 - data bit settings other then 8
 - modem lines operation
 - event signaling

\anchor usart_server_con

Test Mode : <b>USART Server</b>
-----------------------------

This test mode allows extensive validation of the USART Driver.<br>
Results of the Driver Validation in this test mode are relevant as a proof of driver compliance 
to the CMSIS-Driver specification.

To perform extensive communication tests, it is required to use an 
\ref usart_server "USART Server" running on a dedicated hardware.

To enable this mode of testing in the <b>DV_USART_Config.h</b> configuration file select the
<b>Configuration: Test Mode: USART Server</b> setting.

Required pin connections for the <b>USART Server</b> test mode with <b>USART Server Mode</b>: <b>Asynchronous</b> or <b>IrDa</b>

\note Only necessary pin connections are Tx, Rx and GND. Flow control (RTS, CTS), modem lines (DSR, DTR, DCD, RI) and 
synchronous clock (CLK) line are optional and depend on pin availability and driver support for flow control, modem lines and 
synchronous mode.
\note For stable idle level on Rx lines, external pull-ups can be added.

\image html usart_server_pin_connections_async.png

Required pin connections for the <b>USART Server</b> test mode with <b>USART Server Mode</b>: <b>Single-wire</b>

\image html usart_server_pin_connections_single_wire.png

\note Asynchronous and IrDa modes use same connection type and Single-wire uses a different connection. 
\note Please ensure that correct connection is used as required by tests otherwise damage to hardware may occur 
      (For example using Asynchronous mode selected with hardware connected for Single-wire tests can result in damaged pins).
\note In this mode Tx and Rx pins are internally connected together.
\note Tx pin should use open-drain setting in this mode to prevent damage in case of simultaneous 
      driving from both USART Server and USART Client.

\note To ensure proper signal quality:
       - keep the connecting wires as short as possible
       - if possible keep Tx and Rx wires wires separate from each other
       - ensure a good Ground (GND) connection between USART Server and DUT

\defgroup usart_tests Tests
\ingroup dv_usart
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* USART Driver Management tests                                                                                              */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup usart_tests_drv_mgmt Driver Management
\ingroup usart_tests
\details
These tests verify API and operation of the USART driver management functions.

The driver management tests verify the following driver functions
(<a href="http://www.keil.com/pack/doc/CMSIS/Driver/html/group__usart__interface__gr.html" target="_blank">USART Driver function documentation</a>):
 - \b GetVersion
\code
  ARM_DRIVER_VERSION     GetVersion      (void);
\endcode
 - \b GetCapabilities
\code
  ARM_USART_CAPABILITIES GetCapabilities (void);
\endcode
 - \b Initialize
\code
  int32_t                Initialize      (ARM_USART_SignalEvent_t cb_event);
\endcode
 - \b Uninitialize
\code
  int32_t                Uninitialize    (void);
\endcode
 - \b PowerControl
\code
  int32_t                PowerControl    (ARM_POWER_STATE state);
\endcode

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_GetVersion
\details
The function \b UDSART_GetVersion verifies the \b GetVersion function.
\code
  ARM_DRIVER_VERSION GetVersion (void);
\endcode

Testing sequence:
  - Driver is uninitialized and peripheral is powered-off:
    - Call GetVersion function
    - Assert that GetVersion function returned version structure with API and implementation versions higher or equal to 1.0
*/
void USART_GetVersion (void) {
  ARM_DRIVER_VERSION ver;

  ver = drv->GetVersion();

  // Assert that GetVersion function returned version structure with API and implementation versions higher or equal to 1.0
  TEST_ASSERT((ver.api >= ARM_DRIVER_VERSION_MAJOR_MINOR(1UL,0UL)) && (ver.drv >= ARM_DRIVER_VERSION_MAJOR_MINOR(1UL,0UL)));

  (void)snprintf(msg_buf, sizeof(msg_buf), "[INFO] Driver API version %d.%d, Driver version %d.%d", (ver.api >> 8), (ver.api & 0xFFU), (ver.drv >> 8), (ver.drv & 0xFFU));
  TEST_MESSAGE(msg_buf);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_GetCapabilities
\details
The function \b USART_GetCapabilities verifies the \b GetCapabilities function.
\code
  ARM_USART_CAPABILITIES GetCapabilities (void);
\endcode

Testing sequence:
  - Driver is uninitialized and peripheral is powered-off:
    - Call GetCapabilities function
    - Assert that GetCapabilities function returned capabilities structure with reserved field 0
*/
void USART_GetCapabilities (void) {
  ARM_USART_CAPABILITIES cap;

  cap = drv->GetCapabilities();

  // Assert that GetCapabilities function returned capabilities structure with reserved field 0
  TEST_ASSERT_MESSAGE((cap.reserved == 0U), "[FAILED] Capabilities reserved field must be 0");
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Initialize_Uninitialize
\details
The function \b USART_Initialize_Uninitialize verifies the \b Initialize and \b Uninitialize functions.
\code
  int32_t Initialize (ARM_USART_SignalEvent_t cb_event);
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
    - Call GetTxCount function and assert that it returned 0
    - Call GetRxCount function and assert that it returned 0
    - Call Control function and assert that it returned ARM_DRIVER_ERROR status
    - Call GetStatus function
    - Assert that GetStatus function returned status structure with tx_busy flag 0
    - Assert that GetStatus function returned status structure with rx_busy flag 0
    - Assert that GetStatus function returned status structure with tx_underflow flag 0
    - Assert that GetStatus function returned status structure with rx_overflow flag 0
    - Assert that GetStatus function returned status structure with rx_break flag 0
    - Assert that GetStatus function returned status structure with rx_framing_error flag 0
    - Assert that GetStatus function returned status structure with rx_parity_error flag 0
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
    - Call Uninitialize function and assert that it returned ARM_DRIVER_OK status<br>
      (this must unconditionally terminate active send/receive/transfer, power-off the peripheral and uninitialize the driver)
  - Driver is uninitialized and peripheral is powered-off:
    - Call GetStatus function
    - Assert that GetStatus function returned status structure with tx_busy flag 0
    - Assert that GetStatus function returned status structure with rx_busy flag 0
*/
void USART_Initialize_Uninitialize (void) {
  ARM_USART_STATUS stat;

  // Driver is uninitialized and peripheral is powered-off:
  // Call PowerControl(ARM_POWER_FULL) function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_ERROR);

  stat = drv->GetStatus();

  // Call PowerControl(ARM_POWER_LOW) function and assert that it returned status different then ARM_DRIVER_OK
  TEST_ASSERT(drv->PowerControl (ARM_POWER_LOW) != ARM_DRIVER_OK);

  stat = drv->GetStatus();

  // Call Send function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Send (ptr_tx_buf, USART_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call Receive function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Receive (ptr_rx_buf, USART_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call Transfer function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Transfer (ptr_tx_buf, ptr_rx_buf, USART_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call Control function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Control (((USART_CFG_DEF_MODE << ARM_USART_CONTROL_Pos) & ARM_USART_CONTROL_Msk) | ARM_USART_DATA_BITS_8, USART_CFG_DEF_BAUDRATE) == ARM_DRIVER_ERROR);

  // Call GetStatus function
  stat = drv->GetStatus();

  // Assert that GetStatus function returned status structure with tx_busy flag 0
  TEST_ASSERT(stat.tx_busy == 0U);

  // Assert that GetStatus function returned status structure with rx_busy flag 0
  TEST_ASSERT(stat.rx_busy == 0U);

  // Assert that GetStatus function returned status structure with tx_underflow flag 0
  TEST_ASSERT(stat.tx_underflow == 0U);

  // Assert that GetStatus function returned status structure with rx_overflow flag 0
  TEST_ASSERT(stat.rx_overflow == 0U);

  // Assert that GetStatus function returned status structure with rx_break flag 0
  TEST_ASSERT(stat.rx_break == 0U);

  // Assert that GetStatus function returned status structure with rx_framing_error flag 0
  TEST_ASSERT(stat.rx_framing_error == 0U);

  // Assert that GetStatus function returned status structure with rx_parity_error flag 0
  TEST_ASSERT(stat.rx_parity_error == 0U);

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
  TEST_ASSERT(drv->Initialize (USART_DrvEvent) == ARM_DRIVER_OK);

  // Driver is initialized and peripheral is powered-off:
  // Call Initialize function (with callback specified) again and assert that it returned ARM_DRIVER_OK status
  TEST_ASSERT(drv->Initialize (USART_DrvEvent) == ARM_DRIVER_OK);

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
  TEST_ASSERT(drv->Control (((USART_CFG_DEF_MODE << ARM_USART_CONTROL_Pos) & ARM_USART_CONTROL_Msk) | 
                            ((USART_CFG_DEF_CPOL << ARM_USART_CPOL_Pos)    & ARM_USART_CPOL_Msk)    | 
                            ((USART_CFG_DEF_CPHA << ARM_USART_CPHA_Pos)    & ARM_USART_CPHA_Msk)    | 
                              ARM_USART_DATA_BITS_8, USART_CFG_DEF_BAUDRATE) == ARM_DRIVER_OK);

  // Call Uninitialize function assert that it returned ARM_DRIVER_OK status
  // (this must unconditionally terminate active send/receive/transfer, power-off the peripheral and uninitialize the driver)
  TEST_ASSERT(drv->Uninitialize () == ARM_DRIVER_OK);

  // Driver is uninitialized and peripheral is powered-off:
  // Call GetStatus function
  stat = drv->GetStatus();

  // Assert that GetStatus function returned status structure with tx_busy flag 0
  TEST_ASSERT(stat.tx_busy == 0U);

  // Assert that GetStatus function returned status structure with rx_busy flag 0
  TEST_ASSERT(stat.rx_busy == 0U);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_PowerControl
\details
The function \b USART_PowerControl verifies the \b PowerControl function.
\code
  int32_t PowerControl (ARM_POWER_STATE state);
\endcode

Testing sequence:
  - Driver is initialized and peripheral is powered-off:
    - Call Send function and assert that it returned ARM_DRIVER_ERROR status
    - Call Receive function and assert that it returned ARM_DRIVER_ERROR status
    - Call Transfer function and assert that it returned ARM_DRIVER_ERROR status
    - Call GetTxCount function and assert that it returned 0
    - Call GetRxCount function and assert that it returned 0
    - Call Control function and assert that it returned ARM_DRIVER_ERROR status
    - Call GetStatus function
    - Assert that GetStatus function returned status structure with tx_busy flag 0
    - Assert that GetStatus function returned status structure with rx_busy flag 0
    - Assert that GetStatus function returned status structure with tx_underflow flag 0
    - Assert that GetStatus function returned status structure with rx_overflow flag 0
    - Assert that GetStatus function returned status structure with rx_break flag 0
    - Assert that GetStatus function returned status structure with rx_framing_error flag 0
    - Assert that GetStatus function returned status structure with rx_parity_error flag 0
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
    - Call PowerControl(ARM_POWER_OFF) function and assert that it returned ARM_DRIVER_OK status<br>
      (this must unconditionally terminate active send/receive/transfer and power-off the peripheral)
  - Driver is initialized and peripheral is powered-off:
    - Call GetStatus function
    - Assert that GetStatus function returned status structure with tx_busy flag 0
    - Assert that GetStatus function returned status structure with rx_busy flag 0
*/
void USART_PowerControl (void) {
  int32_t          ret;
  ARM_USART_STATUS stat;

  (void)drv->Initialize (NULL);

  // Driver is initialized and peripheral is powered-off:
  // Call Send function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Send (ptr_tx_buf, USART_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call Receive function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Receive (ptr_rx_buf, USART_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call Transfer function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Transfer (ptr_tx_buf, ptr_rx_buf, USART_CFG_DEF_NUM) == ARM_DRIVER_ERROR);

  // Call Control function and assert that it returned ARM_DRIVER_ERROR status
  TEST_ASSERT(drv->Control (((USART_CFG_DEF_MODE << ARM_USART_CONTROL_Pos) & ARM_USART_CONTROL_Msk) | 
                            ((USART_CFG_DEF_CPOL << ARM_USART_CPOL_Pos)    & ARM_USART_CPOL_Msk)    | 
                            ((USART_CFG_DEF_CPHA << ARM_USART_CPHA_Pos)    & ARM_USART_CPHA_Msk)    | 
                              ARM_USART_DATA_BITS_8, USART_CFG_DEF_BAUDRATE) == ARM_DRIVER_ERROR);

  // Call GetStatus function
  stat = drv->GetStatus();

  // Assert that GetStatus function returned status structure with tx_busy flag 0
  TEST_ASSERT(stat.tx_busy == 0U);

  // Assert that GetStatus function returned status structure with rx_busy flag 0
  TEST_ASSERT(stat.rx_busy == 0U);

  // Assert that GetStatus function returned status structure with tx_underflow flag 0
  TEST_ASSERT(stat.tx_underflow == 0U);

  // Assert that GetStatus function returned status structure with rx_overflow flag 0
  TEST_ASSERT(stat.rx_overflow == 0U);

  // Assert that GetStatus function returned status structure with rx_break flag 0
  TEST_ASSERT(stat.rx_break == 0U);

  // Assert that GetStatus function returned status structure with rx_framing_error flag 0
  TEST_ASSERT(stat.rx_framing_error == 0U);

  // Assert that GetStatus function returned status structure with rx_parity_error flag 0
  TEST_ASSERT(stat.rx_parity_error == 0U);

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
  TEST_ASSERT(drv->Control (((USART_CFG_DEF_MODE << ARM_USART_CONTROL_Pos) & ARM_USART_CONTROL_Msk) | 
                            ((USART_CFG_DEF_CPOL << ARM_USART_CPOL_Pos)    & ARM_USART_CPOL_Msk)    | 
                            ((USART_CFG_DEF_CPHA << ARM_USART_CPHA_Pos)    & ARM_USART_CPHA_Msk)    | 
                              ARM_USART_DATA_BITS_8, USART_CFG_DEF_BAUDRATE) == ARM_DRIVER_OK);

  // Call PowerControl(ARM_POWER_OFF) function and assert that it returned ARM_DRIVER_OK status
  // (this must unconditionally terminate active send/receive/transfer and power-off the peripheral)
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);

  // Driver is initialized and peripheral is powered-off:
  // Call GetStatus function
  stat = drv->GetStatus();

  // Assert that GetStatus function returned status structure with tx_busy flag 0
  TEST_ASSERT(stat.tx_busy == 0U);

  // Assert that GetStatus function returned status structure with rx_busy flag 0
  TEST_ASSERT(stat.rx_busy == 0U);

  (void)drv->Uninitialize ();
}

/**
@}
*/
// End of usart_tests_drv_mgmt

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* USART Data Exchange tests                                                                                                  */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup usart_tests_data_xchg Data Exchange
\ingroup usart_tests
\details
These tests verify API and operation of the USART data exchange functions.

The data exchange tests verify the following driver functions
(<a href="http://www.keil.com/pack/doc/CMSIS/Driver/html/group__usart__interface__gr.html" target="_blank">USART Driver function documentation</a>):
 - \b Send
\code
  int32_t          Send         (const void *data,                    uint32_t num);
\endcode
 - \b Receive
\code
  int32_t          Receive      (      void *data,                    uint32_t num);
\endcode
 - \b Transfer
\code
  int32_t          Transfer     (const void *data_out, void *data_in, uint32_t num);
\endcode
 - \b GetTxCount
\code
  uint32_t         GetTxCount   (void);
\endcode
 - \b GetRxCount
\code
  uint32_t         GetRxCount   (void);
\endcode
 - \b Control
\code
  int32_t          Control      (uint32_t control, uint32_t arg);
\endcode
 - \b GetStatus
\code
  ARM_USART_STATUS GetStatus    (void);
\endcode
 - \b SignalEvent
\code
  void (*ARM_USART_SignalEvent_t) (uint32_t event);
\endcode

All of these tests execute a data exchange and check the result of this data exchange.

Data exchange test procedure when Test Mode <b>USART Server</b> is selected:
  - send command "SET BUF TX,.." to the USART Server: Set Tx buffer
  - send command "SET BUF RX,.." to the USART Server: Set Rx buffer
  - send command "SET COM .."    to the USART Server: Set communication settings for the next XFER command
  - send command "XFER .."       to the USART Server: Specify transfer
  - driver Control: Configure the USART interface
  - driver Control: Set the default Tx value (in synchronous mode only)
  - driver Send/Receive/Transfer: Start the requested operation
  - driver GetStatus/SignalEvent: Wait for the current operation to finish or time-out<br>
    (operation is finished when appropriate tx_busy or rx_busy flag is 0 and send/receive/transfer completed event was signaled)
  - assert that operation has finished in expected time
  - assert that appropriate ARM_USART_EVENT_SEND_COMPLETE, ARM_USART_EVENT_RECEIVE_COMPLETE
    or ARM_USART_EVENT_TRANSFER_COMPLETE event was signaled
  - driver GetStatus: Assert that appropriate tx_busy or rx_busy flag is 0
  - driver GetTxCount: If it was a send or transfer operation assert that number of transmitted items is same as requested
  - driver GetRxCount: If it was a receive or transfer operation assert that number of received items is same as requested
  - if operation has timed out call driver Control function to Abort operation and wait timeout time<br>
    to make sure that the USART Server is ready for the next command
  - assert that received content is as expected
  - send command "GET BUF RX,.." to the USART Server: Get Rx buffer
  - assert that sent content (read from the USART Server's receive buffer) is as expected

Data exchange <b>Abort</b> test procedure when Test Mode <b>USART Server</b> is selected:
  - send command "SET BUF TX,.." to the USART Server: Set Tx buffer
  - send command "SET BUF RX,.." to the USART Server: Set Rx buffer
  - send command "SET COM .."    to the USART Server: Set communication settings for the next XFER command
  - send command "XFER .."       to the USART Server: Specify transfer
  - driver Control: Configure the USART interface
  - driver Control: Set the default Tx value (in synchronous mode only)
  - driver Send/Receive/Transfer: Start the requested operation
  - wait up to 1 ms
  - driver Control: Abort the current operation
  - driver GetStatus: Assert that appropriate tx_busy or rx_busy flag is 0
  - driver GetTxCount: If it was a send or transfer operation assert that number of transmitted items is less than requested
  - driver GetRxCount: If it was a receive or transfer operation assert that number of received items is less than requested

Data exchange test procedure when Test Mode <b>Loopback</b> is selected:
  - driver Control: Configure the USART interface
  - driver Control: Set the default Tx value (in synchronous mode only)
  - driver Send/Receive/Transfer: Start the requested operation
  - driver GetStatus/SignalEvent: Wait for the current operation to finish or time-out<br>
    (operation is finished when appropriate tx_busy or rx_busy flag is 0 and send/receive/transfer completed event was signaled)
  - assert that operation has finished in expected time
  - assert that appropriate ARM_USART_EVENT_SEND_COMPLETE and ARM_USART_EVENT_RECEIVE_COMPLETE
    or ARM_USART_EVENT_TRANSFER_COMPLETE event was signaled
  - driver GetStatus: Assert that appropriate tx_busy or rx_busy flag is 0
  - driver GetTxCount: If it was a send and receive or transfer operation assert that number of transmitted items is same as requested
  - driver GetRxCount: If it was a send and receive or transfer operation assert that number of received items is same as requested
  - if operation has timed out call driver Control function to Abort operation
  - assert that sent and received content is same

\note Limitations of Data Exchange tests if Test Mode <b>Loopback</b> is selected:
 - in synchronous mode only Master mode transfer can be tested
 - only 8 data bit tests are supported
 - parity tests are not supported
 - stop bits tests are not supported
 - flow control tests are not supported
 - clock polarity and clock phase tests are not supported
@{
*/

#ifndef __DOXYGEN__                     // Exclude form the documentation
/*
  \brief         Execute USART data exchange or abort operation.
  \param[in]     operation      operation (OP_SEND .. OP_ABORT_TRANSFER)
  \param[in]     mode           mode (MODE_ASYNCHRONOUS .. MODE_SMART_CARD)
  \param[in]     data_bits      data bits (5 .. 9)
  \param[in]     parity         parity (PARITY_NONE, PARITY_EVEN or PARITY_ODD)
  \param[in]     stop_bits      stop bits (STOP_BITS_1 .. STOP_BITS_0_5)
  \param[in]     flow_control   flow control (FLOW_CONTROL_NONE .. FLOW_CONTROL_RTS_CTS)
  \param[in]     cpol           clock polarity (CPOL0 or CPOL1)
  \param[in]     cpha           clock phase (CPHA0 or CPHA1)
  \param[in]     baudrate       baudrate in bauds
  \param[in]     num            number of items to send, receive or transfer
  \return        none
*/
static void USART_DataExchange_Operation (uint32_t operation, uint32_t mode, uint32_t data_bits, uint32_t parity, uint32_t stop_bits, uint32_t flow_control, uint32_t cpol, uint32_t cpha, uint32_t baudrate, uint32_t num) {
  // volatile specifier is used to prevent compiler from optimizing variables 
  // in a way that they cannot be seen with debugger
  volatile  int32_t         stat, def_tx_stat;
  volatile uint32_t         drv_mode, drv_data_bits, drv_parity, drv_stop_bits, drv_flow_control, drv_cpol, drv_cpha;
  volatile uint32_t         srv_mode, srv_dir, srv_flow_control;
  volatile ARM_USART_STATUS usart_stat;
  volatile uint32_t         tx_count, rx_count;
           uint32_t         start_cnt;
           uint32_t         val, delay, i;
  volatile uint32_t         srv_delay;
  volatile uint32_t         drv_delay;
           uint8_t          chk_tx_data, chk_rx_data;
           uint32_t         timeout, start_tick, curr_tick;

  // Prepare parameters for USART Server and Driver configuration
  switch (operation & 0x0FU) {
    case OP_SEND:
    case OP_ABORT_SEND:
      srv_dir   = 1U;
      drv_delay = 10U;
      srv_delay = 0U;
      if (mode == MODE_SYNCHRONOUS_MASTER) {
        // In Synchronous Master test wait for USART Server to start the trasnsfer
        drv_delay = 10U;
        srv_delay = 0U;
      } else if (mode == MODE_SYNCHRONOUS_SLAVE) {
        // In Synchronous Slave test make USART Server wait for driver to start the send operation
        drv_delay = 0U;
        srv_delay = 10U;
      }
      break;
    case OP_RECEIVE:
    case OP_ABORT_RECEIVE:
      srv_dir   = 0U;
      drv_delay = 0U;
      srv_delay = 10U;
      if (mode == MODE_SYNCHRONOUS_MASTER) {
        // In Synchronous Master test wait for USART Server to start the trasnsfer
        drv_delay = 10U;
        srv_delay = 0U;
      } else if (mode == MODE_SYNCHRONOUS_SLAVE) {
        // In Synchronous Slave test make USART Server wait for driver to start the receive operation
        drv_delay = 0U;
        srv_delay = 10U;
      }
      break;
    case OP_TRANSFER:
    case OP_ABORT_TRANSFER:
      srv_dir   = 2U;
      if (mode == MODE_SYNCHRONOUS_MASTER) {
        // In Synchronous Master test wait for USART Server to start the trasnsfer
        drv_delay = 10U;
        srv_delay = 0U;
      } else if (mode == MODE_SYNCHRONOUS_SLAVE) {
        // In Synchronous Slave test mode make USART Server wait for driver to start the transfer
        drv_delay = 0U;
        srv_delay = 10U;
      } else {
        TEST_FAIL_MESSAGE("[FAILED] Synchronous mode unknown! Data exchange operation aborted!");
        return;
      }
    case OP_RECEIVE_SEND_LB:
      drv_delay = 10U;
      break;
  }

  switch (mode) {
    case MODE_ASYNCHRONOUS:
      drv_mode = ARM_USART_MODE_ASYNCHRONOUS;
      srv_mode = MODE_ASYNCHRONOUS;
      break;
    case MODE_SYNCHRONOUS_MASTER:
      drv_mode = ARM_USART_MODE_SYNCHRONOUS_MASTER;
      srv_mode = MODE_SYNCHRONOUS_SLAVE;
      break;
    case MODE_SYNCHRONOUS_SLAVE:
      drv_mode = ARM_USART_MODE_SYNCHRONOUS_SLAVE;
      srv_mode = MODE_SYNCHRONOUS_MASTER;
      break;
    case MODE_SINGLE_WIRE:
      drv_mode = ARM_USART_MODE_SINGLE_WIRE;
      srv_mode = MODE_SINGLE_WIRE;
      break;
    case MODE_IRDA:
      drv_mode = ARM_USART_MODE_IRDA;
      srv_mode = MODE_IRDA;
      break;
    case MODE_SMART_CARD:
      TEST_FAIL_MESSAGE("[FAILED] Smart Card mode testing not supported! Data exchange operation aborted!");
      return;
    default:
      TEST_FAIL_MESSAGE("[FAILED] Unknown mode! Data exchange operation aborted!");
      return;
  }

  switch (data_bits) {
    case 5U:
      drv_data_bits = ARM_USART_DATA_BITS_5;
      break;
    case 6U:
      drv_data_bits = ARM_USART_DATA_BITS_6;
      break;
    case 7U:
      drv_data_bits = ARM_USART_DATA_BITS_7;
      break;
    case 8U:
      drv_data_bits = ARM_USART_DATA_BITS_8;
      break;
    case 9U:
      drv_data_bits = ARM_USART_DATA_BITS_9;
      break;
    default:
      TEST_FAIL_MESSAGE("[FAILED] Data bits not in range 5 to 9! Data exchange operation aborted!");
      return;
  }

  switch (parity) {
    case PARITY_NONE:
      drv_parity = ARM_USART_PARITY_NONE;
      break;
    case PARITY_EVEN:
      drv_parity = ARM_USART_PARITY_EVEN;
      break;
    case PARITY_ODD:
      drv_parity = ARM_USART_PARITY_ODD;
      break;
    default:
      TEST_FAIL_MESSAGE("[FAILED] Unknown parity! Data exchange operation aborted!");
      return;
  }

  switch (stop_bits) {
    case STOP_BITS_1:
      drv_stop_bits = ARM_USART_STOP_BITS_1;
      break;
    case STOP_BITS_2:
      drv_stop_bits = ARM_USART_STOP_BITS_2;
      break;
    case STOP_BITS_1_5:
      drv_stop_bits = ARM_USART_STOP_BITS_1_5;
      break;
    case STOP_BITS_0_5:
      drv_stop_bits = ARM_USART_STOP_BITS_0_5;
      break;
    default:
      TEST_FAIL_MESSAGE("[FAILED] Unknown stop bits! Data exchange operation aborted!");
      return;
  }

  switch (flow_control) {
    case FLOW_CONTROL_NONE:
      drv_flow_control = ARM_USART_FLOW_CONTROL_NONE;
      srv_flow_control = FLOW_CONTROL_NONE;
      break;
    case FLOW_CONTROL_CTS:
      drv_flow_control = ARM_USART_FLOW_CONTROL_CTS;
      srv_flow_control = FLOW_CONTROL_RTS;
      break;
    case FLOW_CONTROL_RTS:
      drv_flow_control = ARM_USART_FLOW_CONTROL_RTS;
      srv_flow_control = FLOW_CONTROL_CTS;
      break;
    case FLOW_CONTROL_RTS_CTS:
      drv_flow_control = ARM_USART_FLOW_CONTROL_RTS_CTS;
      srv_flow_control = FLOW_CONTROL_RTS_CTS;
      break;
    default:
      TEST_FAIL_MESSAGE("[FAILED] Unknown flow control! Data exchange operation aborted!");
      return;
  }

  switch (cpol) {
    case CPOL0:
      drv_cpol = ARM_USART_CPOL0;
      break;
    case CPOL1:
      drv_cpol = ARM_USART_CPOL1;
      break;
    default:
      TEST_FAIL_MESSAGE("[FAILED] Unknown clock polarity! Data exchange operation aborted!");
      return;
  }

  switch (cpha) {
    case CPHA0:
      drv_cpha = ARM_USART_CPHA0;
      break;
    case CPHA1:
      drv_cpha = ARM_USART_CPHA1;
      break;
    default:
      TEST_FAIL_MESSAGE("[FAILED] Unknown clock phase! Data exchange operation aborted!");
      return;
  }

  // Total transfer timeout (10 ms is overhead before transfer starts)
  timeout = USART_CFG_XFER_TIMEOUT + 10U;

  // Check that USART status is not busy before starting data exchange test
  usart_stat = drv->GetStatus();        // Get USART status
  if (usart_stat.tx_busy != 0U) {
    // If tx_busy flag is active
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Tx busy active before operation! Data exchange operation aborted!");
  }
  TEST_ASSERT_MESSAGE(usart_stat.tx_busy == 0U, msg_buf);
  if (usart_stat.rx_busy != 0U) {
    // If rx_busy flag is active
    (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Rx busy active before operation! Data exchange operation aborted!");
  }
  TEST_ASSERT_MESSAGE(usart_stat.rx_busy == 0U, msg_buf);
  if (usart_stat.tx_busy != 0U) {
    return;                             // If tx busy is active abort data exchange operation
  }
  if (usart_stat.rx_busy != 0U) {
    return;                             // If rx busy is active abort data exchange operation
  }

  do {
#if (USART_SERVER_USED == 1)            // If Test Mode USART Server is selected
    if (CmdSetBufTx('S')   != EXIT_SUCCESS) { break; }
    if (CmdSetBufRx('?')   != EXIT_SUCCESS) { break; }
    if (CmdSetCom  (srv_mode, data_bits, parity, stop_bits, srv_flow_control, cpol, cpha, baudrate) != EXIT_SUCCESS) { break; }
    if (CmdXfer    (srv_dir, num, srv_delay, USART_CFG_XFER_TIMEOUT, 0U) != EXIT_SUCCESS) { break; }
#else                                   // If Test Mode Loopback is selected
    // Remove warnings for unused variables
    (void)srv_mode;
    (void)srv_dir;
    (void)srv_flow_control;
    (void)srv_delay;
#endif
    start_tick = osKernelGetTickCount();

    // Initialize buffers
    memset(ptr_tx_buf,  (int32_t)'!' , USART_BUF_MAX);
    memset(ptr_tx_buf,  (int32_t)'T' , num * DataBitsToBytes(data_bits));
    memset(ptr_rx_buf,  (int32_t)'?' , USART_BUF_MAX);
    memset(ptr_cmp_buf, (int32_t)'?' , USART_BUF_MAX);

    // Configure required communication settings
    (void)osDelay(drv_delay);           // Wait specified time before calling Control function
    stat = drv->Control (drv_mode | drv_data_bits | drv_parity | drv_stop_bits | drv_flow_control | drv_cpol | drv_cpha, baudrate);

    if (stat != ARM_DRIVER_OK) {
      // If configuration has failed
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Control function returned", str_ret[-stat]);
    }
    // Assert that Control function returned ARM_DRIVER_OK
    TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);

    if (stat != ARM_DRIVER_OK) {
      // If control function has failed means driver does not support requested settings
      // wait for timeout and exit
      (void)osDelay(timeout+20U);       // Wait for USART Server to timout XFER and start reception of next command
      return;                                   // Here Abort test is finished, exit
    }

    // Set default 3/16 bit IrDA pulse period (only for IrDA mode)
    if (mode == MODE_IRDA) {
      stat = drv->Control (ARM_USART_SET_IRDA_PULSE, 0U);
      if ((stat != ARM_DRIVER_OK) && (stat != ARM_DRIVER_ERROR_UNSUPPORTED)) {
        // If set IrDA pulse has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s", str_oper[operation], "Set IrDA pulse value returned", str_ret[-stat]);
      }
      // Assert that Control function returned ARM_DRIVER_OK or ARM_DRIVER_ERROR_UNSUPPORTED
      TEST_ASSERT_MESSAGE((stat == ARM_DRIVER_OK) || (stat == ARM_DRIVER_ERROR_UNSUPPORTED), msg_buf);

      if (stat == ARM_DRIVER_ERROR_UNSUPPORTED) {
        // If set IrDA pulse value is not supported
        (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] %s: %s", str_oper[operation], "Set default IrDA pulse value is not supported");
        TEST_MESSAGE(msg_buf);
      }
    }

    // Set default Tx value to 'D' byte values (only for synchronous mode)
    if ((mode == MODE_SYNCHRONOUS_MASTER) || (mode == MODE_SYNCHRONOUS_SLAVE)) {
      val = ((uint32_t)'D' << 24) | ((uint32_t)'D' << 16) | ((uint32_t)'D' << 8) | (uint32_t)'D';
      stat = drv->Control (ARM_USART_SET_DEFAULT_TX_VALUE, val);
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

    // Prepare local variables
    event             = 0U;
    duration          = 0xFFFFFFFFUL;
    tx_count          = 0U;
    rx_count          = 0U;
    tx_count_sample   = 0U;
    rx_count_sample   = 0U;
    chk_tx_data       = 0U;
    chk_rx_data       = 0U;
    start_cnt         = osKernelGetSysTimerCount();

    // Start the data exchange operation
    switch (operation & 0x0FU) {
      case OP_SEND:
      case OP_ABORT_SEND:
        stat = drv->Control(ARM_USART_CONTROL_TX, 1U);
        if (stat != ARM_DRIVER_OK) {
          // If transmitter enable has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Transmitter enable returned", str_ret[-stat]);
        }
#if (USART_SERVER_USED == 1)                            // If Test Mode USART Server is selected
        chk_tx_data = 1U;                               // Check sent data
#endif
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
        stat = drv->Control(ARM_USART_CONTROL_RX, 1U);
        if (stat != ARM_DRIVER_OK) {
          // If receiver enable has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Receiver enable returned", str_ret[-stat]);
        }
#if (USART_SERVER_USED == 1)                            // If Test Mode USART Server is selected
        chk_rx_data = 1U;                               // Check received data
#endif
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
        stat = drv->Control(ARM_USART_CONTROL_TX, 1U);
        if (stat != ARM_DRIVER_OK) {
          // If transmitter enable has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Transmitter enable returned", str_ret[-stat]);
        }
        stat = drv->Control(ARM_USART_CONTROL_RX, 1U);
        if (stat != ARM_DRIVER_OK) {
          // If receiver enable has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Receiver enable returned", str_ret[-stat]);
        }
        chk_tx_data = 1U;                               // Check sent data
        chk_rx_data = 1U;                               // Check received data
        stat = drv->Transfer(ptr_tx_buf, ptr_rx_buf, num);
        if (stat != ARM_DRIVER_OK) {
          // If Transfer activation has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Transfer function returned", str_ret[-stat]);
        }
        // Assert that Transfer function returned ARM_DRIVER_OK
        TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);
        break;
      case OP_RECEIVE_SEND_LB:
        stat = drv->Control(ARM_USART_CONTROL_TX, 1U);
        if (stat != ARM_DRIVER_OK) {
          // If transmitter enable has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Transmitter enable returned", str_ret[-stat]);
        }
        stat = drv->Control(ARM_USART_CONTROL_RX, 1U);
        if (stat != ARM_DRIVER_OK) {
          // If receiver enable has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Receiver enable returned", str_ret[-stat]);
        }
        chk_rx_data = 1U;                               // Check received data
        stat = drv->Receive(ptr_rx_buf, num);
        if (stat != ARM_DRIVER_OK) {
          // If Receive activation has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Receive function returned", str_ret[-stat]);
        }
        // Assert that Receive function returned ARM_DRIVER_OK
        TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);
        chk_tx_data = 1U;                               // Check sent data
        stat = drv->Send(ptr_tx_buf, num);
        if (stat != ARM_DRIVER_OK) {
          // If Send activation has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Send function returned", str_ret[-stat]);
        }
        // Assert that Send function returned ARM_DRIVER_OK
        TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);
        break;
      default:
        TEST_FAIL_MESSAGE("[FAILED] Unknown operation! Data exchange operation aborted!");
        return;
    }

    if ((operation == OP_ABORT_SEND)     ||     // This IF block tests only abort functionality
        (operation == OP_ABORT_RECEIVE)  ||
        (operation == OP_ABORT_TRANSFER)) {
      (void)osDelay(1U);                        // Wait short time before doing Abort
      switch (operation & 0x0FU) {
        case OP_ABORT_SEND:
          stat = drv->Control (ARM_USART_ABORT_SEND, 0U);
          break;
        case OP_ABORT_RECEIVE:
          stat = drv->Control (ARM_USART_ABORT_RECEIVE, 0U);
          break;
        case OP_ABORT_TRANSFER:
          stat = drv->Control (ARM_USART_ABORT_TRANSFER, 0U);
          break;
      }
      if (stat != ARM_DRIVER_OK) {
        // If Abort has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s", str_oper[operation], "Control function returned", str_ret[-stat]);
      }
      // Assert that Control function returned ARM_DRIVER_OK
      TEST_ASSERT_MESSAGE(stat == ARM_DRIVER_OK, msg_buf);
      usart_stat = drv->GetStatus();            // Get USART status
      if (usart_stat.tx_busy != 0U) {
        // If tx_busy flag is still active
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Tx busy still active after Abort");
      }
      // Assert that tx_busy flag is not active
      TEST_ASSERT_MESSAGE(usart_stat.tx_busy == 0U, msg_buf);
      if (usart_stat.rx_busy != 0U) {
        // If rx_busy flag is still active
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Rx busy still active after Abort");
      }
      // Assert that rx_busy flag is not active
      TEST_ASSERT_MESSAGE(usart_stat.rx_busy == 0U, msg_buf);

      if ((operation == OP_ABORT_SEND) || (operation == OP_ABORT_TRANSFER)) {
        tx_count = drv->GetTxCount();         // Get Tx count
        if (tx_count >= num) {
          // If Tx count is more or equal to number of items then Abort has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetTxCount returned", tx_count, "after Abort of", num, "items");
        }
        // Assert data count is less then number of items requested for send/transfer
        TEST_ASSERT_MESSAGE(tx_count < num, msg_buf);
      }
      if ((operation == OP_ABORT_RECEIVE) || (operation == OP_ABORT_TRANSFER)) {
        rx_count = drv->GetRxCount();         // Get Rx count
        if (rx_count >= num) {
          // If Rx count is more or equal to number of items then Abort has failed
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetRxCount returned", rx_count, "after Abort of", num, "items");
        }
        // Assert data count is less then number of items requested for receive/transfer
        TEST_ASSERT_MESSAGE(rx_count < num, msg_buf);
      }

      stat = drv->Control(ARM_USART_CONTROL_TX, 0U);
      if (stat != ARM_DRIVER_OK) {
        // If transmitter disable has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Transmitter disable returned", str_ret[-stat]);
      }
      stat = drv->Control(ARM_USART_CONTROL_RX, 0U);
      if (stat != ARM_DRIVER_OK) {
        // If receiver disable has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Receiver disable returned", str_ret[-stat]);
      }

#if (USART_SERVER_USED == 1)                            // If Test Mode USART Server is selected
      // Wait until timeout expires
      curr_tick = osKernelGetTickCount();
      if ((curr_tick - start_tick) < timeout) {
        (void)osDelay(timeout - (curr_tick - start_tick));
      }
      (void)osDelay(20U);                       // Wait for USART Server to start reception of next command
#endif

      return;                                   // Here Abort test is finished, exit
    }

    // Wait for operation to finish
    // for send operation wait until status tx_busy is 0 and 
    // event ARM_USART_EVENT_SEND_COMPLETE is signaled, or timeout
    // for receive operation wait until status rx_busy is 0 and 
    // event ARM_USART_EVENT_RECEIVE_COMPLETE is signaled, or timeout
    // for transfer operation wait until status tx_busy and rx_busy is 0 and 
    // event ARM_USART_EVENT_TRANSFER_COMPLETE is signaled, or timeout
    // for receive and send operation wait until status tx_busy and rx_busy is 0 and 
    // both events ARM_USART_EVENT_SEND_COMPLETE and ARM_USART_EVENT_RECEIVE_COMPLETE are signaled, or timeout
    do {
      if (operation == OP_SEND) {
        if (tx_count_sample == 0U) {
          // Store first Tx count different than 0
          tx_count_sample = drv->GetTxCount();  // Get Tx count
        }
        if ((drv->GetStatus().tx_busy == 0U) && ((event & ARM_USART_EVENT_SEND_COMPLETE) != 0U)) {
          duration = osKernelGetSysTimerCount() - start_cnt;
          break;
        }
      }
      if (operation == OP_RECEIVE) {
        if (rx_count_sample == 0U) {
          // Store first Rx count different than 0
          rx_count_sample = drv->GetRxCount();  // Get Rx count
        }
        if ((drv->GetStatus().rx_busy == 0U) && ((event & ARM_USART_EVENT_RECEIVE_COMPLETE) != 0U)) {
          duration = osKernelGetSysTimerCount() - start_cnt;
          break;
        }
      }
      if (operation == OP_TRANSFER) {
        if (tx_count_sample == 0U) {
          // Store first Tx count different than 0
          tx_count_sample = drv->GetTxCount();  // Get Tx count
        }
        if (rx_count_sample == 0U) {
          // Store first Rx count different than 0
          rx_count_sample = drv->GetRxCount();  // Get Rx count
        }
        if ((drv->GetStatus().tx_busy == 0U) && (drv->GetStatus().rx_busy == 0U) && ((event & ARM_USART_EVENT_TRANSFER_COMPLETE) != 0U)) {
          duration = osKernelGetSysTimerCount() - start_cnt;
          break;
        }
      }
      if (operation == OP_RECEIVE_SEND_LB) {
        if (tx_count_sample == 0U) {
          // Store first Tx count different than 0
          tx_count_sample = drv->GetTxCount();  // Get Tx count
        }
        if (rx_count_sample == 0U) {
          // Store first Rx count different than 0
          rx_count_sample = drv->GetRxCount();  // Get Rx count
        }
        if ((drv->GetStatus().tx_busy == 0U) && (drv->GetStatus().rx_busy == 0U) && 
           ((event & (ARM_USART_EVENT_RECEIVE_COMPLETE | ARM_USART_EVENT_SEND_COMPLETE)) == 
                     (ARM_USART_EVENT_RECEIVE_COMPLETE | ARM_USART_EVENT_SEND_COMPLETE))) {
          duration = osKernelGetSysTimerCount() - start_cnt;
          break;
        }
      }
    } while ((osKernelGetTickCount() - start_tick) < timeout);

    if (duration == 0xFFFFFFFFUL) {
      // If operation has timed out
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Operation timed out");
    }
    // Assert that operation has finished in expected time
    TEST_ASSERT_MESSAGE(duration != 0xFFFFFFFFUL, msg_buf);

    if (duration != 0xFFFFFFFFUL) {
      // For Synchronous Slave duration is started by Master srv_delay later so this has to be deducted
      if (mode == MODE_SYNCHRONOUS_SLAVE) {
        if (srv_delay > 1U) {
          srv_delay --;                 // Reduce 1 ms tolerance of delay
          if (duration > (srv_delay * (systick_freq / 1000U))) {
            duration -= srv_delay * (systick_freq / 1000U);
          }
        }
      }
    }

    // Check all expected conditions after Send operation
    if (operation == OP_SEND) {
      if ((event & ARM_USART_EVENT_SEND_COMPLETE) == 0U) {
        // If send complete event was not signaled
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_SEND_COMPLETE was not signaled");
        chk_tx_data = 0U;                       // Do not check sent content
      }
      // Assert that ARM_USART_EVENT_SEND_COMPLETE was signaled
      TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_SEND_COMPLETE) != 0U, msg_buf);

      if (drv_cap.event_tx_complete != 0U) {
        // If driver supports Tx complete signaling
        if ((event & ARM_USART_EVENT_TX_COMPLETE) == 0U) {
          // If Tx complete event was not signaled
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_TX_COMPLETE was not signaled");
          chk_tx_data = 0U;                       // Do not check sent content
        }
        // Assert that ARM_USART_EVENT_TX_COMPLETE was signaled
        TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_TX_COMPLETE) != 0U, msg_buf);
      }

      usart_stat = drv->GetStatus();            // Get USART status
      if (usart_stat.tx_busy != 0U) {
        // If tx_busy flag is still active
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Tx busy still active after operation");
        chk_tx_data = 0U;                       // Do not check sent content
      }
      // Assert that tx_busy flag is not active
      TEST_ASSERT_MESSAGE(usart_stat.tx_busy == 0U, msg_buf);

      tx_count = drv->GetTxCount();             // Get Tx count
      if (tx_count != num) {
        // If Tx count is different then number of items, then operation has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetTxCount returned", tx_count, "expected was", num, "items");
        chk_tx_data = 0U;                       // Do not check sent content
      }
      // Assert that Tx count is equal to number of items requested for send
      TEST_ASSERT_MESSAGE(tx_count == num, msg_buf);

      if ((drv->GetStatus().tx_busy != 0U) || ((event & ARM_USART_EVENT_SEND_COMPLETE) == 0U)) {
        // If send did not finish in time, abort it
        (void)drv->Control(ARM_USART_ABORT_SEND, 0U);
      }
    }

    // Check all expected conditions after Receive operation
    if (operation == OP_RECEIVE) {
      if ((event & ARM_USART_EVENT_RECEIVE_COMPLETE) == 0U) {
        // If receive complete event was not signaled
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_RECEIVE_COMPLETE was not signaled");
        chk_rx_data = 0U;                       // Do not check received content
      }
      // Assert that ARM_USART_EVENT_RECEIVE_COMPLETE was signaled
      TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RECEIVE_COMPLETE) != 0U, msg_buf);

      usart_stat = drv->GetStatus();            // Get USART status
      if (usart_stat.rx_busy != 0U) {
        // If rx_busy flag is still active
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Rx busy still active after operation");
        chk_rx_data = 0U;                       // Do not check received content
      }
      // Assert that rx_busy flag is not active
      TEST_ASSERT_MESSAGE(usart_stat.rx_busy == 0U, msg_buf);

      if ((event & ARM_USART_EVENT_RX_OVERFLOW) != 0U) {
        // If Rx overflow was signaled during the transfer
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_RX_OVERFLOW was signaled");
        chk_rx_data = 0U;                       // Do not check received content
      }
      // Assert that ARM_USART_EVENT_RX_OVERFLOW was not signaled
      TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RX_OVERFLOW) == 0U, msg_buf);

      rx_count = drv->GetRxCount();             // Get Rx count
      if (rx_count != num) {
        // If Rx count is different then number of items, then operation has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetRxCount returned", rx_count, "expected was", num, "items");
        chk_rx_data = 0U;                       // Do not check received content
      }
      // Assert that Rx count is equal to number of items requested for reception
      TEST_ASSERT_MESSAGE(rx_count == num, msg_buf);

      if ((drv->GetStatus().rx_busy != 0U) || ((event & ARM_USART_EVENT_RECEIVE_COMPLETE) == 0U)) {
        // If reception did not finish in time, abort it
        (void)drv->Control(ARM_USART_ABORT_RECEIVE, 0U);
      }
    }

    // Check all expected conditions after Transfer operation
    if (operation == OP_TRANSFER) {
      if ((event & ARM_USART_EVENT_TRANSFER_COMPLETE) == 0U) {
        // If transfer complete event was not signaled
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_TRANSFER_COMPLETE was not signaled");
        chk_tx_data = 0U;                       // Do not check sent content
        chk_rx_data = 0U;                       // Do not check received content
      }
      // Assert that ARM_USART_EVENT_TRANSFER_COMPLETE was signaled
      TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_TRANSFER_COMPLETE) != 0U, msg_buf);

      usart_stat = drv->GetStatus();            // Get USART status
      if (usart_stat.tx_busy != 0U) {
        // If tx_busy flag is still active
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Tx busy still active after operation");
        chk_tx_data = 0U;                       // Do not check sent content
      }
      // Assert that tx_busy flag is not active
      TEST_ASSERT_MESSAGE(usart_stat.tx_busy == 0U, msg_buf);
      if (usart_stat.rx_busy != 0U) {
        // If rx_busy flag is still active
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Rx busy still active after operation");
        chk_rx_data = 0U;                       // Do not check received content
      }
      // Assert that rx_busy flag is not active
      TEST_ASSERT_MESSAGE(usart_stat.rx_busy == 0U, msg_buf);

      if ((event & ARM_USART_EVENT_TX_UNDERFLOW) != 0U) {
        // If Tx underflow was signaled during the transfer
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_TX_UNDERFLOW was signaled");
        chk_tx_data = 0U;                       // Do not check sent content
      }
      // Assert that ARM_USART_EVENT_TX_UNDERFLOW was not signaled
      TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_TX_UNDERFLOW) == 0U, msg_buf);

      if ((event & ARM_USART_EVENT_RX_OVERFLOW) != 0U) {
        // If Rx overflow was signaled during the transfer
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_TX_UNDERFLOW was signaled");
        chk_rx_data = 0U;                       // Do not check received content
      }
      // Assert that ARM_USART_EVENT_RX_OVERFLOW was not signaled
      TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RX_OVERFLOW) == 0U, msg_buf);

      tx_count = drv->GetTxCount();             // Get Tx count
      if (tx_count != num) {
        // If Tx count is different then number of items, then operation has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetTxCount returned", tx_count, "expected was", num, "items");
        chk_tx_data = 0U;                            // Do not check sent content
      }
      // Assert that Tx count is equal to number of items requested for transfer
      TEST_ASSERT_MESSAGE(tx_count == num, msg_buf);

      rx_count = drv->GetRxCount();             // Get Rx count
      if (rx_count != num) {
        // If Rx count is different then number of items, then operation has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetRxCount returned", rx_count, "expected was", num, "items");
        chk_rx_data = 0U;                            // Do not check received content
      }
      // Assert that Rx count is equal to number of items requested for transfer
      TEST_ASSERT_MESSAGE(rx_count == num, msg_buf);

      if ((drv->GetStatus().tx_busy != 0U) || (drv->GetStatus().rx_busy != 0U) || 
         ((event & ARM_USART_EVENT_TRANSFER_COMPLETE) == 0U)) {
        // If transfer did not finish in time, abort it
        (void)drv->Control(ARM_USART_ABORT_TRANSFER, 0U);
      }
    }

    stat = drv->Control(ARM_USART_CONTROL_TX, 0U);
    if (stat != ARM_DRIVER_OK) {
      // If transmitter disable has failed
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Transmitter disable returned", str_ret[-stat]);
    }
    stat = drv->Control(ARM_USART_CONTROL_RX, 0U);
    if (stat != ARM_DRIVER_OK) {
      // If receiver disable has failed
      (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %s! Data exchange operation aborted!", str_oper[operation], "Receiver disable returned", str_ret[-stat]);
    }

#if (USART_SERVER_USED == 1)            // If Test Mode USART Server is selected

    // Wait until timeout expires
    curr_tick = osKernelGetTickCount();
    if ((curr_tick - start_tick) < timeout) {
      (void)osDelay(timeout - (curr_tick - start_tick));
    }
    (void)osDelay(20U);                 // Wait for USART Server to start reception of next command

    if (chk_rx_data != 0U) {            // If received content should be checked
      // Check received content
      memset(ptr_cmp_buf, (int32_t)'S', num * DataBitsToBytes(data_bits));
      if (data_bits == 9U) {
        // If 9-bit mode is used zero out unused bits in high byte
        for (i = 1U; i < num * 2U; i += 2U) {
          ptr_cmp_buf[i] &= 0x01U;
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

    if (chk_tx_data != 0U) {            // If sent content should be checked
      // Check sent content (by checking USART Server's received buffer content)
      if (ComConfigDefault()         != EXIT_SUCCESS) { break; }
      if (CmdGetBufRx(USART_BUF_MAX) != EXIT_SUCCESS) { break; }

      if ((operation == OP_RECEIVE) && (def_tx_stat == ARM_DRIVER_OK)) {
        // Expected data received by USART Server should be default Tx value
        memset(ptr_cmp_buf, (int32_t)'D', num * DataBitsToBytes(data_bits));
      } else {
        // Expected data received by USART Server should be what was sent
        memset(ptr_cmp_buf, (int32_t)'T', num * DataBitsToBytes(data_bits));
      }
      if (data_bits == 9U) {
        // If 9-bit mode is used zero out unused bits in high byte
        for (i = 1U; i < num * 2U; i += 2U) {
          ptr_cmp_buf[i] &= 0x01U;
        }
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
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s byte %i, USART Server received 0x%02X, sent was 0x%02X", str_oper[operation], "Default Tx data mismatches on", i, ptr_rx_buf[i], ptr_cmp_buf[i]);
        } else {
          // If sent was 'T' bytes
          (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s byte %i, USART Server received 0x%02X, sent was 0x%02X", str_oper[operation], "Sent data mismatches on", i, ptr_rx_buf[i], ptr_cmp_buf[i]);
        }
      }
      // Assert data sent is same as expected
      TEST_ASSERT_MESSAGE(stat == 0, msg_buf);
    }

#else                                   // If Test Mode Loopback is selected

    // Check all expected conditions after Receive and Send operation (in Loopback Test Mode)
    if (operation == OP_RECEIVE_SEND_LB) {
      if ((event & ARM_USART_EVENT_SEND_COMPLETE) == 0U) {
        // If send complete event was not signaled
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_SEND_COMPLETE was not signaled");
        chk_tx_data = 0U;                       // Do not check sent content
      }
      // Assert that ARM_USART_EVENT_SEND_COMPLETE was signaled
      TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_SEND_COMPLETE) != 0U, msg_buf);
      if ((event & ARM_USART_EVENT_RECEIVE_COMPLETE) == 0U) {
        // If receive complete event was not signaled
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_RECEIVE_COMPLETE was not signaled");
        chk_rx_data = 0U;                       // Do not check received content
      }
      // Assert that ARM_USART_EVENT_RECEIVE_COMPLETE was signaled
      TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RECEIVE_COMPLETE) != 0U, msg_buf);

      usart_stat = drv->GetStatus();            // Get USART status
      if (usart_stat.tx_busy != 0U) {
        // If tx_busy flag is still active
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Tx busy still active after operation");
        chk_tx_data = 0U;                       // Do not check sent content
      }
      // Assert that tx_busy flag is not active
      TEST_ASSERT_MESSAGE(usart_stat.tx_busy == 0U, msg_buf);
      if (usart_stat.rx_busy != 0U) {
        // If rx_busy flag is still active
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "Rx busy still active after operation");
        chk_rx_data = 0U;                       // Do not check received content
      }
      // Assert that rx_busy flag is not active
      TEST_ASSERT_MESSAGE(usart_stat.rx_busy == 0U, msg_buf);

      if ((event & ARM_USART_EVENT_TX_UNDERFLOW) != 0U) {
        // If Tx underflow was signaled during the transfer
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_TX_UNDERFLOW was signaled");
        chk_tx_data = 0U;                       // Do not check sent content
      }
      // Assert that ARM_USART_EVENT_TX_UNDERFLOW was not signaled
      TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_TX_UNDERFLOW) == 0U, msg_buf);

      if ((event & ARM_USART_EVENT_RX_OVERFLOW) != 0U) {
        // If Rx overflow was signaled during the transfer
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s", str_oper[operation], "ARM_USART_EVENT_TX_UNDERFLOW was signaled");
        chk_rx_data = 0U;                       // Do not check received content
      }
      // Assert that ARM_USART_EVENT_RX_OVERFLOW was not signaled
      TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RX_OVERFLOW) == 0U, msg_buf);

      tx_count = drv->GetTxCount();             // Get Tx count
      if (tx_count != num) {
        // If Tx count is different then number of items, then operation has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetTxCount returned", tx_count, "expected was", num, "items");
        chk_tx_data = 0U;                            // Do not check sent content
      }
      // Assert that Tx count is equal to number of items requested for transfer
      TEST_ASSERT_MESSAGE(tx_count == num, msg_buf);

      rx_count = drv->GetRxCount();             // Get Rx count
      if (rx_count != num) {
        // If Rx count is different then number of items, then operation has failed
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s %i %s %i %s", str_oper[operation], "GetRxCount returned", rx_count, "expected was", num, "items");
        chk_rx_data = 0U;                            // Do not check received content
      }
      // Assert that Rx count is equal to number of items requested for transfer
      TEST_ASSERT_MESSAGE(rx_count == num, msg_buf);

      if ((drv->GetStatus().tx_busy == 0U) && (drv->GetStatus().rx_busy == 0U) && 
         ((event & (ARM_USART_EVENT_RECEIVE_COMPLETE | ARM_USART_EVENT_SEND_COMPLETE)) == 
                   (ARM_USART_EVENT_RECEIVE_COMPLETE | ARM_USART_EVENT_SEND_COMPLETE))) {
        // If transfer did not finish in time, abort it
        (void)drv->Control(ARM_USART_ABORT_TRANSFER, 0U);
      }
    }

    if ((chk_rx_data != 0U) &&          // If received content should be checked and 
        (chk_tx_data != 0U)) {          // if sent content should be checked
      stat = memcmp(ptr_rx_buf, ptr_tx_buf, num * DataBitsToBytes(data_bits));
      if (stat != 0) {
        // If data received mismatches
        // Find on which byte mismatch starts
        for (i = 0U; i < (num * DataBitsToBytes(data_bits)); i++) {
          if (ptr_rx_buf[i] != ptr_cmp_buf[i]) {
            break;
          }
        }
        (void)snprintf(msg_buf, sizeof(msg_buf), "[FAILED] %s: %s byte %i, received was 0x%02X, sent was 0x%02X", str_oper[operation], "Received data mismatches on", i, ptr_rx_buf[i], ptr_tx_buf[i]);
      }
      // Assert that data received is same as expected
      TEST_ASSERT_MESSAGE(stat == 0, msg_buf);
    }
#endif

    return;
  } while (false);

#if (USART_SERVER_USED == 1)            // If Test Mode USART Server is selected
  TEST_FAIL_MESSAGE("[FAILED] Problems in communication with USART Server. Test aborted!");
#endif
}

#endif                                  // End of exclude form the documentation

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Mode_Asynchronous
\details
The function \b USART_Mode_Asynchronous verifies data exchange:
 - in <b>Asynchronous</b> mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - at default baudrate
 - for default number of data items
*/
void USART_Mode_Asynchronous (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (MODE_ASYNCHRONOUS, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            MODE_ASYNCHRONOUS, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         MODE_ASYNCHRONOUS, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, MODE_ASYNCHRONOUS, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Mode_Synchronous_Master
\details
The function \b USART_Mode_Synchronous_Master verifies data exchange:
 - in <b>Synchronous Master</b> mode
 - with default data bits
 - with no parity
 - with 1 stop bit
 - with no flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for default number of data items

\note In Test Mode <b>Loopback</b> Receive function is not tested
*/
void USART_Mode_Synchronous_Master (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (MODE_SYNCHRONOUS_MASTER, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,     MODE_SYNCHRONOUS_MASTER, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_RECEIVE,  MODE_SYNCHRONOUS_MASTER, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
  USART_DataExchange_Operation(OP_TRANSFER, MODE_SYNCHRONOUS_MASTER, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Mode_Synchronous_Slave
\details
The function \b USART_Mode_Synchronous_Slave verifies data exchange:
 - in <b>Synchronous Slave</b> mode
 - with default data bits
 - with no parity
 - with 1 stop bit
 - with no flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is not executed
*/
void USART_Mode_Synchronous_Slave (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (MODE_SYNCHRONOUS_SLAVE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,     MODE_SYNCHRONOUS_SLAVE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,  MODE_SYNCHRONOUS_SLAVE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_TRANSFER, MODE_SYNCHRONOUS_SLAVE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Mode_Single_Wire
\details
The function \b USART_Mode_Single_Wire verifies data exchange:
 - in <b>Single-Wire</b> mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is not executed
*/
void USART_Mode_Single_Wire (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (MODE_SINGLE_WIRE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            MODE_SINGLE_WIRE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         MODE_SINGLE_WIRE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, MODE_SINGLE_WIRE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Mode_IrDA
\details
The function \b USART_Mode_IrDA verifies data exchange:
 - in <b>Infra-red Data</b> mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - at default baudrate
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is not executed
*/
void USART_Mode_IrDA (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (MODE_IRDA, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            MODE_IRDA, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         MODE_IRDA, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, MODE_IRDA, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Data_Bits_5
\details
The function \b USART_Data_Bits_5 verifies data exchange:
 - in default mode
 - with <b>5 data bits</b>
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is not executed
*/
void USART_Data_Bits_5 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, 5U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, 5U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, 5U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#if ((USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_MASTER) || (USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_SLAVE))
  USART_DataExchange_Operation(OP_TRANSFER,        USART_CFG_DEF_MODE, 5U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Data_Bits_6
\details
The function \b USART_Data_Bits_6 verifies data exchange:
 - in default mode
 - with <b>6 data bits</b>
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is not executed
*/
void USART_Data_Bits_6 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, 6U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, 6U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, 6U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#if ((USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_MASTER) || (USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_SLAVE))
  USART_DataExchange_Operation(OP_TRANSFER,        USART_CFG_DEF_MODE, 6U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Data_Bits_7
\details
The function \b USART_Data_Bits_7 verifies data exchange:
 - in default mode
 - with <b>7 data bits</b>
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is not executed
*/
void USART_Data_Bits_7 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, 7U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, 7U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, 7U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#if ((USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_MASTER) || (USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_SLAVE))
  USART_DataExchange_Operation(OP_TRANSFER,        USART_CFG_DEF_MODE, 7U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Data_Bits_8
\details
The function \b USART_Data_Bits_8 verifies data exchange:
 - in default mode
 - with <b>8 data bits</b>
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for default number of data items
*/
void USART_Data_Bits_8 (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, 8U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, 8U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, 8U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#if ((USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_MASTER) || (USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_SLAVE))
  USART_DataExchange_Operation(OP_TRANSFER,        USART_CFG_DEF_MODE, 8U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, 8U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Data_Bits_9
\details
The function \b USART_Data_Bits_9 verifies data exchange:
 - in default mode
 - with <b>9 data bits</b>
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for default number of data items

\note In Test Mode <b>Loopback</b> this test is not executed
*/
void USART_Data_Bits_9 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, 9U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, 9U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, 9U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#if ((USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_MASTER) || (USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_SLAVE))
  USART_DataExchange_Operation(OP_TRANSFER,        USART_CFG_DEF_MODE, 9U, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Parity_None
\details
The function \b USART_Parity_None verifies data exchange:
 - in default mode
 - with default data bits
 - with <b>no parity</b>
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for default number of data items
*/
void USART_Parity_None (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#if ((USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_MASTER) || (USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_SLAVE))
  USART_DataExchange_Operation(OP_TRANSFER,        USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Parity_Even
\details
The function \b USART_Parity_Even verifies data exchange:
 - in default mode
 - with default data bits
 - with <b>even parity</b>
 - with default stop bits
 - with default flow control
 - at default baudrate
 - for default number of data items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Synchronous Master/Slave</b>
*/
void USART_Parity_Even (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSync()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_EVEN, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_EVEN, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_EVEN, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Parity_Odd
\details
The function \b USART_Parity_Odd verifies data exchange:
 - in default mode
 - with default data bits
 - with <b>odd parity</b>
 - with default stop bits
 - with default flow control
 - at default baudrate
 - for default number of data items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Synchronous Master/Slave</b>
*/
void USART_Parity_Odd (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSync()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_ODD, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_ODD, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_ODD, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Stop_Bits_1
\details
The function \b USART_Stop_Bits_1 verifies data exchange:
 - in default mode
 - with default data bits
 - with default parity
 - with <b>1 stop bit</b>
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for default number of data items
*/
void USART_Stop_Bits_1 (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_1, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_1, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_1, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#if ((USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_MASTER) || (USART_CFG_DEF_MODE == MODE_SYNCHRONOUS_SLAVE))
  USART_DataExchange_Operation(OP_TRANSFER,        USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_1, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_1, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Stop_Bits_2
\details
The function \b USART_Stop_Bits_2 verifies data exchange:
 - in default mode
 - with default data bits
 - with default parity
 - with <b>2 stop bits</b>
 - with default flow control
 - at default baudrate
 - for default number of data items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Synchronous Master/Slave</b>
*/
void USART_Stop_Bits_2 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSync()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_2, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_2, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_2, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Stop_Bits_1_5
\details
The function \b USART_Stop_Bits_1_5 verifies data exchange:
 - in default mode
 - with default data bits
 - with default parity
 - with <b>1.5 stop bits</b>
 - with default flow control
 - at default baudrate
 - for default number of data items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Synchronous Master/Slave</b>
*/
void USART_Stop_Bits_1_5 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSync()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_1_5, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_1_5, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_1_5, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Stop_Bits_0_5
\details
The function \b USART_Stop_Bits_0_5 verifies data exchange:
 - in default mode
 - with default data bits
 - with default parity
 - with <b>0.5 stop bits</b>
 - with default flow control
 - at default baudrate
 - for default number of data items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Synchronous Master/Slave</b>
*/
void USART_Stop_Bits_0_5 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSync()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_0_5, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_0_5, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, STOP_BITS_0_5, USART_CFG_DEF_FLOW_CONTROL, 0U, 0U, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Flow_Control_None
\details
The function \b USART_Flow_Control_None verifies data exchange:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with <b>no flow control</b>
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for default number of data items
*/
void USART_Flow_Control_None (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Flow_Control_RTS
\details
The function \b USART_Flow_Control_RTS verifies functionality of the RTS line flow control by 
trying to receive half of default number of data items, after that, RTS line should 
be deactivated by the USART Client hardware and USART Server should stop sending further data.

The RTS line flow control functionality is tested with following settings:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with <b>flow control using RTS signal</b>
 - at default baudrate

Test procedure consists of the following steps:
 - start reception of half of default number of items
 - after half of default number of items was received 
   the RTS line should go to inactive state
 - USART Server after seeing that its CTS line (USART Clients RTS line)
   went to inactive state should stop sending further data
 - after timeout read from USART Server the number of items it has sent 
   and assert that it is less than default number of items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Synchronous Master/Slave</b> or <b>Single-wire</b>
*/
void USART_Flow_Control_RTS (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_RTS, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdSetCom  (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_CTS, 0U, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { break; }
    if (CmdXfer    (0U, USART_CFG_DEF_NUM, 10U, USART_CFG_XFER_TIMEOUT, 0U) != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      |
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_RTS  , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0U;

    (void)drv->Control(ARM_USART_CONTROL_RX, 1U);

    TEST_ASSERT(drv->Receive(ptr_rx_buf, USART_CFG_DEF_NUM / 2U) == ARM_DRIVER_OK);
    (void)osDelay(USART_CFG_XFER_TIMEOUT + 20U);        // Wait for USART Server to timeout the XFER command

    // Abort and disable reception
    (void)drv->Control(ARM_USART_ABORT_RECEIVE, 0U);
    (void)drv->Control(ARM_USART_CONTROL_RX,    0U);

    (void)osDelay(10U);

    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdGetCnt()        != EXIT_SUCCESS) { break; }
    TEST_ASSERT_MESSAGE(xfer_count < USART_CFG_DEF_NUM, "[FAILED] All data was received, RTS line is not working!");

    if (CmdGetVer()        != EXIT_SUCCESS) { break; }

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Flow_Control_CTS
\details
The function \b USART_Flow_Control_CTS verifies functionality of the CTS line flow control by 
trying to send default number of data items, while at half of transmitted number 
of items the USART Server deactivates its RTS line (it is connected to CTS line on the USART Client).

The CTS line flow control functionality is tested with following settings:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with <b>flow control using CTS signal</b>
 - at default baudrate

Test procedure consists of the following steps:
 - start send of default number of items
 - after USART Server receives half of default number of items it 
   will drive its RTS line (USART Clients CTS line) inactive
 - before timeout check that tx_busy is active and that number of transmitted items is less than default number of items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Synchronous Master/Slave</b> or <b>Single-wire</b>
*/
void USART_Flow_Control_CTS (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_CTS, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdSetCom  (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, 0U, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { break; }
    if (CmdXfer    (1U, USART_CFG_DEF_NUM, 0U, USART_CFG_XFER_TIMEOUT, USART_CFG_DEF_NUM / 2U) != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      |
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_CTS  , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0U;
    (void)osDelay(10U);                 // Wait for USART Server to start reception

    (void)drv->Control(ARM_USART_CONTROL_TX, 1U);
    TEST_ASSERT(drv->Send(ptr_tx_buf, USART_CFG_DEF_NUM) == ARM_DRIVER_OK);
    (void)osDelay(USART_CFG_XFER_TIMEOUT / 2U); // Wait for half of timeout, after which sending should have stopped

    // Assert that tx_busy is still active
    TEST_ASSERT_MESSAGE(drv->GetStatus().tx_busy != 0U, "[FAILED] Send has finished, CTS line is not working!");

    // Assert that tx count is not 0
    TEST_ASSERT_MESSAGE(drv->GetTxCount() != 0U, "[FAILED] No data was sent, CTS line is not working!");

    // Assert that tx count is less than default number of items
    TEST_ASSERT_MESSAGE(drv->GetTxCount() < USART_CFG_DEF_NUM, "[FAILED] All data was sent, CTS line is not working!");

    (void)osDelay(USART_CFG_XFER_TIMEOUT / 2U); // Wait for USART Server to timeout the XFER command

    // Abort and disable transmission
    (void)drv->Control(ARM_USART_ABORT_SEND, 0U);
    (void)drv->Control(ARM_USART_CONTROL_TX, 0U);

    (void)osDelay(10U);                         // Wait for USART Server to prepare for reception of new command

    // Do a dummy send command to flush any data left-over from previous send
    // (When flow control CTS is used the send is started for default number of items.
    //  After half of default number of items are sent the CTS line is deasserted, 
    //  but data register was already loaded with next item to be sent.
    //  To get rid of this loaded data we send a dummy command that USART Server will 
    //  ignore, but it will allow us to send next command properly.)
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    (void)ComSendCommand("Dummy", 5U);

    (void)osDelay(USART_CFG_SRV_CMD_TOUT+10U);  // Wait for USART Server to timeout the "Dummy" command

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Flow_Control_RTS_CTS
\details
The function \b USART_Flow_Control_RTS_CTS verifies functionality of RTS And CTS lines.
It calls function USART_Flow_Control_RTS that checks RTS line functionality, and 
USART_Flow_Control_CTS that checks CTS line functionality.

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Synchronous Master/Slave</b> or <b>Single-wire</b>
*/
void USART_Flow_Control_RTS_CTS (void) {

  USART_Flow_Control_RTS();
  USART_Flow_Control_CTS();
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Clock_Pol0_Pha0
\details
The function \b USART_Clock_Pol0_Pha0 verifies data exchange:
 - in default mode
 - with default data bits
 - with no parity
 - with 1 stop bit
 - with no flow control
 - with <b>clock polarity 0</b>
 - with <b>clock phase 0</b>
 - at default baudrate
 - for default number of data items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Asynchronous/Single-wire/IrDA</b>
*/
void USART_Clock_Pol0_Pha0 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotAsync()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,     USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL0, CPHA0, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,  USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL0, CPHA0, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_TRANSFER, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL0, CPHA0, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Clock_Pol0_Pha1
\details
The function \b USART_Clock_Pol0_Pha1 verifies data exchange:
 - in default mode
 - with default data bits
 - with no parity
 - with 1 stop bit
 - with no flow control
 - with <b>clock polarity 0</b>
 - with <b>clock phase 1</b>
 - at default baudrate
 - for default number of data items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Asynchronous/Single-wire/IrDA</b>
*/
void USART_Clock_Pol0_Pha1 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotAsync()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,     USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL0, CPHA1, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,  USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL0, CPHA1, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_TRANSFER, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL0, CPHA1, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Clock_Pol1_Pha0
\details
The function \b USART_Clock_Pol1_Pha0 verifies data exchange:
 - in default mode
 - with default data bits
 - with no parity
 - with 1 stop bit
 - with no flow control
 - with <b>clock polarity 1</b>
 - with <b>clock phase 0</b>
 - at default baudrate
 - for default number of data items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Asynchronous/Single-wire/IrDA</b>
*/
void USART_Clock_Pol1_Pha0 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotAsync()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,     USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL1, CPHA0, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,  USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL1, CPHA0, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_TRANSFER, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL1, CPHA0, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Clock_Pol1_Pha1
\details
The function \b USART_Clock_Pol1_Pha1 verifies data exchange:
 - in default mode
 - with default data bits
 - with no parity
 - with 1 stop bit
 - with no flow control
 - with <b>clock polarity 1</b>
 - with <b>clock phase 1</b>
 - at default baudrate
 - for default number of data items

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Asynchronous/Single-wire/IrDA</b>
*/
void USART_Clock_Pol1_Pha1 (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotAsync()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND,     USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL1, CPHA1, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_RECEIVE,  USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL1, CPHA1, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_TRANSFER, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, CPOL1, CPHA1, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Baudrate_Min
\details
The function \b USART_Baudrate_Min verifies data exchange:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at <b>minimum baudrate</b> (define <c>USART_CFG_MIN_BAUDRATE</c> in DV_USART_Config.h)
 - for default number of data items

This test function checks the following requirement:
 - measured bus speed for Send operation in Test Mode <b>USART Server</b> or Send/Receive operation  in Test Mode <b>Loopback</b> 
   is not 25% lower, or higher than requested
*/
void USART_Baudrate_Min (void) {
  volatile uint64_t br;
  volatile  int32_t got_baudrate;

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if  (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_MIN_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_MIN_BAUDRATE, USART_CFG_DEF_NUM);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_MIN_BAUDRATE, USART_CFG_DEF_NUM);
#endif

  if (duration != 0xFFFFFFFFU) {        // If Transfer finished before timeout
    if (duration != 0U) {               // If duration of transfer was more than 0 SysTick counts
      br = ((uint64_t)systick_freq * (1U + USART_CFG_DEF_DATA_BITS + USART_CFG_DEF_STOP_BITS + (uint32_t)(USART_CFG_DEF_PARITY != PARITY_NONE)) * USART_CFG_DEF_NUM) / duration;
      if ((br < ((USART_CFG_MIN_BAUDRATE * 3) / 4)) ||
          (br >   USART_CFG_MIN_BAUDRATE)) {
        // If measured baudrate is 25% lower, or higher than requested
        (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] At requested baudrate of %i bauds, effective transfer speed is %i bauds", USART_CFG_MIN_BAUDRATE, (uint32_t)br);
        TEST_MESSAGE(msg_buf);
      }
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Baudrate_Max
\details
The function \b USART_Baudrate_Max verifies data exchange:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at <b>maximum baudrate</b> (define <c>USART_CFG_MAX_BAUDRATE</c> in DV_USART_Config.h)
 - for default number of data items

This test function checks the following requirement:
 - measured bus speed for Send operation in Test Mode <b>USART Server</b> or Send/Receive operation  in Test Mode <b>Loopback</b> 
   is not 25% lower, or higher than requested
*/
void USART_Baudrate_Max (void) {
  volatile uint64_t br;
  volatile  int32_t got_baudrate;

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if  (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_MAX_BAUDRATE, USART_CFG_DEF_NUM);
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_MAX_BAUDRATE, USART_CFG_DEF_NUM);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_MAX_BAUDRATE, USART_CFG_DEF_NUM);
#endif

  if (duration != 0xFFFFFFFFU) {        // If Transfer finished before timeout
    if (duration != 0U) {               // If duration of transfer was more than 0 SysTick counts
      br = ((uint64_t)systick_freq * (1U + USART_CFG_DEF_DATA_BITS + USART_CFG_DEF_STOP_BITS + (uint32_t)(USART_CFG_DEF_PARITY != PARITY_NONE)) * USART_CFG_DEF_NUM) / duration;
      if ((br < ((USART_CFG_MAX_BAUDRATE * 3) / 4)) ||
          (br >   USART_CFG_MAX_BAUDRATE)) {
        // If measured baudrate is 25% lower, or higher than requested
        (void)snprintf(msg_buf, sizeof(msg_buf), "[WARNING] At requested baudrate of %i bauds, effective transfer speed is %i bauds", USART_CFG_MAX_BAUDRATE, (uint32_t)br);
        TEST_MESSAGE(msg_buf);
      }
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Number_Of_Items
\details
The function \b USART_Number_Of_Items verifies data exchange:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
 - for <b>different number of items</b> (defines <c>USART_CFG_NUM1 .. USART_CFG_NUM5</c> in DV_USART_Config.h)
*/
void USART_Number_Of_Items (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if (USART_CFG_NUM1 != 0U)
#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM1);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM1);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM1);
#endif
#endif

#if (USART_CFG_NUM2 != 0U)
#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM2);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM2);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM2);
#endif
#endif

#if (USART_CFG_NUM3 != 0U)
#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM3);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM3);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM3);
#endif
#endif

#if (USART_CFG_NUM4 != 0U)
#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM4);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM4);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM4);
#endif
#endif

#if (USART_CFG_NUM5 != 0U)
#if (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_SEND,            USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM5);
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM5);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_NUM5);
#endif
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_GetTxCount
\details
The function \b USART_GetTxCount verifies \b GetTxCount function (count changing) during data exchange (Send):
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
*/
void USART_GetTxCount (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_SEND, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  TEST_ASSERT_MESSAGE((tx_count_sample != 0U) && (tx_count_sample != USART_CFG_DEF_NUM), "[FAILED] GetTxCount was not changing during the Send!");
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_GetRxCount
\details
The function \b USART_GetRxCount verifies \b GetRxCount function (count changing) during data exchange (Receive):
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
*/
void USART_GetRxCount (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

#if  (USART_SERVER_USED == 1)
  USART_DataExchange_Operation(OP_RECEIVE,         USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#else
  USART_DataExchange_Operation(OP_RECEIVE_SEND_LB, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_GetTxRxCount
\details
The function \b USART_GetTxRxCount verifies \b GetTxCount and \b GetRxCount functions (count changing) during data exchange (Transfer):
 - in default mode
 - with default data bits
 - with no parity
 - with 1 stop bits
 - with no flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate

\note If Tests Default Mode <b>Asynchronous/Single-wire/IrDA</b> is selected this test is not executed
*/
void USART_GetTxRxCount (void) {

  if (IsNotAsync()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_TRANSFER, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
  TEST_ASSERT_MESSAGE((tx_count_sample != 0U) && (tx_count_sample != USART_CFG_DEF_NUM), "[FAILED] GetTxCount was not changing during the Transfer!");
  TEST_ASSERT_MESSAGE((rx_count_sample != 0U) && (rx_count_sample != USART_CFG_DEF_NUM), "[FAILED] GetRxCount was not changing during the Transfer!");
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_AbortSend
\details
The function \b USART_AbortSend verifies \b Abort function abort of data exchange (Send):
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - at default baudrate
*/
void USART_AbortSend (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_ABORT_SEND, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_AbortReceive
\details
The function \b USART_AbortReceive verifies \b Abort function abort of data exchange (Receive):
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate
*/
void USART_AbortReceive (void) {

  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_ABORT_RECEIVE, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_AbortTransfer
\details
The function \b USART_AbortTransfer verifies \b Abort function abort of data exchange (Transfer):
 - in default mode
 - with default data bits
 - with no parity
 - with 1 stop bit
 - with no flow control
 - with default clock polarity
 - with default clock phase
 - at default baudrate

\note If Tests Default Mode <b>Asynchronous/Single-wire/IrDA</b> is selected this test is not executed
*/
void USART_AbortTransfer (void) {

  if (IsNotAsync()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()  != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  USART_DataExchange_Operation(OP_ABORT_TRANSFER, USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, STOP_BITS_1, FLOW_CONTROL_NONE, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE, USART_CFG_DEF_NUM);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_TxBreak
\details
The function \b USART_TxBreak verifies Break signaling:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate

\note This test is not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Synchronous Master/Slave</b>
*/
void USART_TxBreak (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    // Dummy read break status to clear it
    if (CmdGetBrk()        != EXIT_SUCCESS) { break; }

    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdSetCom  (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, 0U, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { break; }

    // Instruct USART Server to receive data so it can detect Break
    if (CmdXfer    (1U, USART_CFG_DEF_NUM, 0U, USART_CFG_XFER_TIMEOUT, 0U) != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_NONE , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0U;

    (void)osDelay(10U);                 // Wait for USART Server to start reception

    (void)drv->Control(ARM_USART_CONTROL_TX, 1U);

    (void)osDelay(10U);
    TEST_ASSERT(drv->Control(ARM_USART_CONTROL_BREAK, 1U) == ARM_DRIVER_OK);
    (void)osDelay(10U);
    TEST_ASSERT(drv->Control(ARM_USART_CONTROL_BREAK, 0U) == ARM_DRIVER_OK);
    (void)osDelay(10U);

    (void)drv->Control(ARM_USART_CONTROL_TX, 0U);

    (void)osDelay(USART_CFG_XFER_TIMEOUT + 10U);        // Wait for USART Server to timeout the XFER command

    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdGetBrk()        != EXIT_SUCCESS) { break; }
    TEST_ASSERT_MESSAGE(break_status == 1U, "[FAILED] Break was not detected on USART Server!");

    (void)osDelay(10U);                 // Wait for USART Server to prepare for reception of new command

    return;
  } while (false);
#endif
}

/**
@}
*/
// End of usart_tests_data_xchg

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* USART Modem Lines tests                                                                                                  */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup usart_tests_modem Modem Lines
\ingroup usart_tests
\details
These tests verify API and operation of the USART modem lines handling functions.

The data exchange tests verify the following driver functions
(<a href="http://www.keil.com/pack/doc/CMSIS/Driver/html/group__usart__interface__gr.html" target="_blank">USART Driver function documentation</a>):
 - \b SetModemControl
\code
  int32_t                SetModemControl (ARM_USART_MODEM_CONTROL control);
\endcode
 - \b GetModemStatus
\code
  ARM_USART_MODEM_STATUS GetModemStatus  (void);
\endcode

\note These tests are not executed if any of the following settings are selected:
 - Test Mode <b>Loopback</b>
 - Tests Default Mode <b>Synchronous Master/Slave</b> or <b>Single-wire</b>
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Modem_RTS
\details
The function \b USART_Modem_RTS verifies driving of modem line Request To Send (RTS):
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate
*/
void USART_Modem_RTS (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, RTS_AVAILABLE, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_NONE , 
                       USART_CFG_DEF_BAUDRATE);

    TEST_ASSERT(drv->SetModemControl(ARM_USART_RTS_CLEAR) == ARM_DRIVER_OK);
    (void)osDelay(10U);

    if (CmdGetMdm() != EXIT_SUCCESS) { break; }

    TEST_ASSERT_MESSAGE((modem_status & 0x1U) == 0x0U, "[FAILED] CTS line on USART Server is not in inactive state!");

    TEST_ASSERT(drv->SetModemControl(ARM_USART_RTS_SET) == ARM_DRIVER_OK);
    (void)osDelay(10U);

    if (CmdGetMdm() != EXIT_SUCCESS) { break; }

    TEST_ASSERT_MESSAGE((modem_status & 0x1U) == 0x1U, "[FAILED] CTS line on USART Server is not in active state!");

    (void)drv->SetModemControl(ARM_USART_RTS_CLEAR);

    // Give USART Server 10 ms to prepare for reception of the next command
    (void)osDelay(10U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Modem_DTR
\details
The function \b USART_Modem_DTR verifies driving of modem line Data Terminal Ready (DTR):
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate
*/
void USART_Modem_DTR (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, DTR_AVAILABLE, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_NONE , 
                       USART_CFG_DEF_BAUDRATE);

    TEST_ASSERT(drv->SetModemControl(ARM_USART_DTR_CLEAR) == ARM_DRIVER_OK);
    (void)osDelay(10U);

    if (CmdGetMdm() != EXIT_SUCCESS) { break; }

    TEST_ASSERT_MESSAGE((modem_status & 0x2U) == 0x0U, "[FAILED] DSR line on USART Server is not in inactive state!");

    TEST_ASSERT(drv->SetModemControl(ARM_USART_DTR_SET) == ARM_DRIVER_OK);
    (void)osDelay(10U);

    if (CmdGetMdm() != EXIT_SUCCESS) { break; }

    TEST_ASSERT_MESSAGE((modem_status & 0x2U) == 0x2U, "[FAILED] DSR line on USART Server is not in active state!");

    (void)drv->SetModemControl(ARM_USART_DTR_CLEAR);

    // Give USART Server 10 ms to prepare for reception of the next command
    (void)osDelay(10U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Modem_CTS
\details
The function \b USART_Modem_CTS verifies read of modem line Clear To Send (CTS):
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate
*/
void USART_Modem_CTS (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, CTS_AVAILABLE, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_NONE , 
                       USART_CFG_DEF_BAUDRATE);

    // Instruct USART Server to drive RTS to active state for 20 ms
    // RTS line from USART Server should be connected to CTS line on the USART Client (DUT)
    if (CmdSetMdm(RTS_ON, 10U, 20U) != EXIT_SUCCESS) { break; }

    (void)osDelay(20U);

    TEST_ASSERT_MESSAGE(drv->GetModemStatus().cts == 1U, "[FAILED] CTS line not active!");

    // Give USART Server 20 ms to finish SET MDM command and prepare for reception of the next command
    (void)osDelay(20U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Modem_DSR
\details
The function \b USART_Modem_DSR verifies read of modem line Data Set Ready (DSR):
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate
*/
void USART_Modem_DSR (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, DSR_AVAILABLE, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_NONE , 
                       USART_CFG_DEF_BAUDRATE);

    // Instruct USART Server to drive DTR to active state for 20 ms
    // DTR line from USART Server should be connected to DSR line on the USART Client (DUT)
    if (CmdSetMdm(DTR_ON, 10U, 20U) != EXIT_SUCCESS) { break; }

    (void)osDelay(20U);

    TEST_ASSERT_MESSAGE(drv->GetModemStatus().dsr == 1U, "[FAILED] DSR line not active!");

    // Give USART Server 20 ms to finish SET MDM command and prepare for reception of the next command
    (void)osDelay(20U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Modem_DCD
\details
The function \b USART_Modem_DCD verifies read of modem line Data Carrier Detect (DCD):
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate
*/
void USART_Modem_DCD (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, DCD_AVAILABLE, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_NONE , 
                       USART_CFG_DEF_BAUDRATE);

    // Instruct USART Server to drive DTR to active state for 20 ms
    if (CmdSetMdm(TO_DCD_ON, 10U, 20U) != EXIT_SUCCESS) { break; }
    
    (void)osDelay(20U);

    TEST_ASSERT_MESSAGE(drv->GetModemStatus().dcd == 1U, "[FAILED] DCD line not active!");

    // Give USART Server 20 ms to finish SET MDM command and prepare for reception of the next command
    (void)osDelay(20U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Modem_RI
\details
The function \b USART_Modem_RI verifies read of modem line Ring Indicator (RI):
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate
*/
void USART_Modem_RI (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, RI_AVAILABLE, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_NONE , 
                       USART_CFG_DEF_BAUDRATE);

    // Instruct USART Server to drive RI to active state for 20 ms
    if (CmdSetMdm(TO_RI_ON, 10U, 20U) != EXIT_SUCCESS) { break; }

    (void)osDelay(20U);

    TEST_ASSERT_MESSAGE(drv->GetModemStatus().ri == 1U, "[FAILED] RI line not active!");

    // Give USART Server 20 ms to finish SET MDM command and prepare for reception of the next command
    (void)osDelay(20U);

    return;
  } while (false);
#endif
}

/**
@}
*/
// End of usart_tests_modem

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* USART Event tests                                                                                                        */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup usart_tests_evt Event
\ingroup usart_tests
\details
These tests verify API and operation of the USART event signaling, except ARM_USART_EVENT_SEND_COMPLETE, 
ARM_USART_EVENT_RECEIVE_COMPLETE, ARM_USART_EVENT_TRANSFER_COMPLETE and ARM_USART_EVENT_TX_COMPLETE signals 
which is tested in the Data Exchange tests.

The event tests verify the following driver function
(<a href="http://www.keil.com/pack/doc/CMSIS/Driver/html/group__usart__interface__gr.html" target="_blank">USART Driver function documentation</a>):
 - \b SignalEvent
\code
  void (*ARM_USART_SignalEvent_t) (uint32_t event);
\endcode

\note In Test Mode <b>Loopback</b> these tests are skipped
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Tx_Underflow
\details
The function \b USART_Tx_Underflow verifies signaling of the <b>ARM_USART_EVENT_TX_UNDERFLOW</b> event:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - at default baudrate

it also checks that status tx_underflow flag was activated.

\note If Tests Default Mode <b>Asynchronous/Synchronous Master/Single-wire/IrDA</b> is selected this test is not executed
*/
void USART_Tx_Underflow (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotAsync()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSyncMaster() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdSetCom(USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { break; }
    if (CmdXfer  (0U, 1U, 10U, 20U, 0U) != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL         | 
                       USART_CFG_DEF_DATA_BITS_VAL    | 
                       USART_CFG_DEF_PARITY_VAL       | 
                       USART_CFG_DEF_STOP_BITS_VAL    | 
                       USART_CFG_DEF_FLOW_CONTROL_VAL | 
                       USART_CFG_DEF_CPOL_VAL         | 
                       USART_CFG_DEF_CPHA_VAL         , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0U;

    (void)drv->Control(ARM_USART_CONTROL_RX, 1U);

    (void)osDelay(30U);                 // Wait for USART Server to timeout

    // Assert that event ARM_USART_EVENT_TX_UNDERFLOW was signaled
    TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_TX_UNDERFLOW) != 0U, "[FAILED] Event ARM_USART_EVENT_TX_UNDERFLOW was not signaled!");

    // Assert that status rx_overflow flag is active
    TEST_ASSERT_MESSAGE(drv->GetStatus().tx_underflow != 0U, "[FAILED] Status tx_underflow flag was not activated!");

    (void)drv->Control(ARM_USART_CONTROL_RX,    0U);

    // Give USART Server 10 ms to prepare for reception of the next command
    (void)osDelay(10U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Rx_Overflow
\details
The function \b USART_Rx_Overflow verifies signaling of the <b>ARM_USART_EVENT_RX_OVERFLOW</b> event:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - at default baudrate

it also checks that status rx_overflow flag was activated.

\note If Tests Default Mode <b>Synchronous Master</b> is selected this test is not executed
*/
void USART_Rx_Overflow (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSyncMaster() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdSetCom(USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { break; }
    if (CmdXfer  (0U, 1U, 10U, 20U, 0U) != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL         | 
                       USART_CFG_DEF_DATA_BITS_VAL    | 
                       USART_CFG_DEF_PARITY_VAL       | 
                       USART_CFG_DEF_STOP_BITS_VAL    | 
                       USART_CFG_DEF_FLOW_CONTROL_VAL | 
                       USART_CFG_DEF_CPOL_VAL         | 
                       USART_CFG_DEF_CPHA_VAL         , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0U;

    (void)drv->Control(ARM_USART_CONTROL_RX, 1U);

    (void)osDelay(30U);                 // Wait for USART Server to timeout

    // Assert that event ARM_USART_EVENT_RX_OVERFLOW was signaled
    TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RX_OVERFLOW) != 0U, "[FAILED] Event ARM_USART_EVENT_RX_OVERFLOW was not signaled!");

    // Assert that status rx_overflow flag is active
    TEST_ASSERT_MESSAGE(drv->GetStatus().rx_overflow != 0U, "[FAILED] Status rx_overflow flag was not activated!");

    (void)drv->Control(ARM_USART_CONTROL_RX, 0U);

    // Give USART Server 10 ms to prepare for reception of the next command
    (void)osDelay(10U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Rx_Timeout
\details
The function \b USART_Rx_Timeout verifies signaling of the <b>ARM_USART_EVENT_RX_TIMEOUT</b> event:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - at default baudrate

\note If Tests Default Mode <b>Synchronous Master/Slave</b> is selected this test is not executed
*/
void USART_Rx_Timeout (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }
    if (CmdSetCom(USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { break; }
    if (CmdXfer  (0U, 1U, 10U, 10U, 0U) != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL         | 
                       USART_CFG_DEF_DATA_BITS_VAL    | 
                       USART_CFG_DEF_PARITY_VAL       | 
                       USART_CFG_DEF_STOP_BITS_VAL    | 
                       USART_CFG_DEF_FLOW_CONTROL_VAL | 
                       USART_CFG_DEF_CPOL_VAL         | 
                       USART_CFG_DEF_CPHA_VAL         , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0U;

    (void)drv->Control(ARM_USART_CONTROL_RX, 1U);
    (void)drv->Receive(ptr_rx_buf, 2U);

    (void)osDelay(30U);                 // Wait for USART Server to timeout

    // Assert that event ARM_USART_EVENT_RX_TIMEOUT was signaled
    TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RX_TIMEOUT) != 0U, "[FAILED] Event ARM_USART_EVENT_RX_TIMEOUT was not signaled!");

    (void)drv->Control(ARM_USART_ABORT_RECEIVE, 0U);
    (void)drv->Control(ARM_USART_CONTROL_RX,    0U);

    // Give USART Server 10 ms to prepare for reception of the next command
    (void)osDelay(10U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Rx_Break
\details
The function \b USART_Rx_Break verifies signaling of the <b>ARM_USART_EVENT_RX_BREAK</b> event:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - at default baudrate

it also checks that status rx_break flag was activated.

\note If Tests Default Mode <b>Synchronous Master/Slave</b> is selected this test is not executed
*/
void USART_Rx_Break (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL         | 
                       USART_CFG_DEF_DATA_BITS_VAL    | 
                       USART_CFG_DEF_PARITY_VAL       | 
                       USART_CFG_DEF_STOP_BITS_VAL    | 
                       USART_CFG_DEF_FLOW_CONTROL_VAL | 
                       USART_CFG_DEF_CPOL_VAL         | 
                       USART_CFG_DEF_CPHA_VAL         , 
                       USART_CFG_DEF_BAUDRATE);

    // Instruct USART Server to signal Break for 20 ms
    if (CmdSetBrk(10U, 20U) != EXIT_SUCCESS) { break; }

    (void)drv->Control(ARM_USART_CONTROL_RX, 1U);
    (void)drv->Receive(ptr_rx_buf, 1U);

    // This test allows break detection for continuous mode as well 
    // as LIN variant (return to inactive after 10/11 bits)
    (void)osDelay(40U);

    // Assert that event ARM_USART_EVENT_RX_BREAK was signaled
    TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RX_BREAK) != 0U, "[FAILED] Event ARM_USART_EVENT_RX_BREAK was not signaled!");

    // Assert that status rx_break flag is active
    TEST_ASSERT_MESSAGE(drv->GetStatus().rx_break != 0U, "[FAILED] Status rx_break flag was not activated!");

    (void)drv->Control(ARM_USART_ABORT_RECEIVE, 0U);
    (void)drv->Control(ARM_USART_CONTROL_RX,    0U);

    // Give USART Server 10 ms to prepare for reception of the next command
    (void)osDelay(10U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Rx_Framing_Error
\details
The function \b USART_Rx_Framing_Error verifies signaling of the <b>ARM_USART_EVENT_RX_FRAMING_ERROR</b> event:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - at default baudrate

it also checks that status rx_framing_error flag was activated.

\note If Tests Default Mode <b>Synchronous Master/Slave</b> is selected this test is not executed
*/
void USART_Rx_Framing_Error (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    // We test framing error by adding parity bit if it is not set as default setting, 
    // or removing parity bit if it is set as default setting
    if (USART_CFG_DEF_PARITY == PARITY_NONE) {
      if (CmdSetCom(USART_CFG_SRV_MODE, USART_CFG_DEF_DATA_BITS, PARITY_EVEN, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { break; }
    } else {
      if (CmdSetCom(USART_CFG_SRV_MODE, USART_CFG_DEF_DATA_BITS, PARITY_NONE, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { break; }
    }
    if (CmdXfer  (0U, 1U, 10U, 20U, 0U) != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL         | 
                       USART_CFG_DEF_DATA_BITS_VAL    | 
                       USART_CFG_DEF_PARITY_VAL       | 
                       USART_CFG_DEF_STOP_BITS_VAL    | 
                       USART_CFG_DEF_FLOW_CONTROL_VAL | 
                       USART_CFG_DEF_CPOL_VAL         | 
                       USART_CFG_DEF_CPHA_VAL         , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0U;

    (void)drv->Control(ARM_USART_CONTROL_RX, 1U);
    (void)drv->Receive(ptr_rx_buf, 1U);

    (void)osDelay(30U);                 // Wait for USART Server to timeout

    // Assert that event ARM_USART_EVENT_RX_FRAMING_ERROR was signaled
    TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RX_FRAMING_ERROR) != 0U, "[FAILED] Event ARM_USART_EVENT_RX_FRAMING_ERROR was not signaled!");

    // Assert that status rx_framing_error flag is active
    TEST_ASSERT_MESSAGE(drv->GetStatus().rx_framing_error != 0U, "[FAILED] Status rx_framing_error flag was not activated!");

    (void)drv->Control(ARM_USART_ABORT_RECEIVE, 0U);
    (void)drv->Control(ARM_USART_CONTROL_RX,    0U);

    // Give USART Server 10 ms to prepare for reception of the next command
    (void)osDelay(10U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Rx_Parity_Error
\details
The function \b USART_Rx_Parity_Error verifies signaling of the <b>ARM_USART_EVENT_RX_PARITY_ERROR</b> event:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with default flow control
 - at default baudrate

it also checks that status rx_parity_error flag was activated.

\note If Tests Default Mode <b>Synchronous Master/Slave</b> is selected this test is not executed
*/
void USART_Rx_Parity_Error (void) {

  if (IsNotLoopback() != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()     != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()    != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck   (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, 0U, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    // We test parity error by requesting USART Server to send an item at ODD parity  
    // and configure USART Client to receive with EVEN parity
    if (CmdSetCom(USART_CFG_SRV_MODE, USART_CFG_DEF_DATA_BITS, PARITY_ODD, USART_CFG_DEF_STOP_BITS, USART_CFG_DEF_FLOW_CONTROL, USART_CFG_DEF_CPOL, USART_CFG_DEF_CPHA, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { break; }

    if (CmdXfer  (0U, 1U, 10U, 20U, 0U) != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL                                       | 
                       USART_CFG_DEF_DATA_BITS_VAL                                  | 
                     ((PARITY_EVEN << ARM_USART_PARITY_Pos) & ARM_USART_PARITY_Msk) | 
                       USART_CFG_DEF_STOP_BITS_VAL                                  | 
                       USART_CFG_DEF_FLOW_CONTROL_VAL                               | 
                       USART_CFG_DEF_CPOL_VAL                                       | 
                       USART_CFG_DEF_CPHA_VAL                                       , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0U;

    (void)drv->Control(ARM_USART_CONTROL_RX, 1U);
    (void)drv->Receive(ptr_rx_buf, 1U);

    (void)osDelay(30U);                 // Wait for USART Server to timeout

    // Assert that event ARM_USART_EVENT_RX_PARITY_ERROR was signaled
    TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RX_PARITY_ERROR) != 0U, "[FAILED] Event ARM_USART_EVENT_RX_PARITY_ERROR was not signaled!");

    // Assert that status rx_parity_error flag is active
    TEST_ASSERT_MESSAGE(drv->GetStatus().rx_parity_error != 0U, "[FAILED] Status rx_parity_error flag was not activated!");

    (void)drv->Control(ARM_USART_ABORT_RECEIVE, 0U);
    (void)drv->Control(ARM_USART_CONTROL_RX,    0U);

    // Give USART Server 10 ms to prepare for reception of the next command
    (void)osDelay(10U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Event_CTS
\details
The function \b USART_Event_CTS verifies signaling of the <b>ARM_USART_EVENT_CTS</b> event:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate

\note If Tests Default Mode <b>Synchronous Master/Slave</b> or <b>Single-wire</b> is selected this test is not executed
*/
void USART_Event_CTS (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, CTS_AVAILABLE, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    // Instruct USART Server to drive RTS to active state for 20 ms
    // RTS line from USART Server should be connected to CTS line on the USART Client (DUT)
    if (CmdSetMdm(RTS_ON, 10U, 20U) != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_CTS  , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0;

    (void)osDelay(20U);

    TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_CTS) == ARM_USART_EVENT_CTS, "[FAILED] CTS line did not register change!");

    (void)osDelay(20U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Event_DSR
\details
The function \b USART_Event_DSR verifies signaling of the <b>ARM_USART_EVENT_DSR</b> event:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate

\note If Tests Default Mode <b>Synchronous Master/Slave</b> or <b>Single-wire</b> is selected this test is not executed
*/
void USART_Event_DSR (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, DSR_AVAILABLE, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_NONE , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0;

    // Instruct USART Server to drive DTR to active state for 20 ms
    // DTR line from USART Server should be connected to DSR line on the USART Client (DUT)
    if (CmdSetMdm(DTR_ON, 10U, 20U) != EXIT_SUCCESS) { break; }

    (void)osDelay(20U);

    TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_DSR) == ARM_USART_EVENT_DSR, "[FAILED] DSR line did not register change!");

    (void)osDelay(20U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Event_DCD
\details
The function \b USART_Event_DCD verifies signaling of the <b>ARM_USART_EVENT_DCD</b> event:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate

\note If Tests Default Mode <b>Synchronous Master/Slave</b> or <b>Single-wire</b> is selected this test is not executed
*/
void USART_Event_DCD (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, DCD_AVAILABLE, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_NONE , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0;

    // Instruct USART Server to drive RTS to active state for 20 ms
    if (CmdSetMdm(TO_DCD_ON, 10U, 20U) != EXIT_SUCCESS) { break; }

    (void)osDelay(20U);

    TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_DCD) == ARM_USART_EVENT_DCD, "[FAILED] DCD line did not register change!");

    (void)osDelay(20U);

    return;
  } while (false);
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Function: Function USART_Event_RI
\details
The function \b USART_Event_RI verifies signaling of the <b>ARM_USART_EVENT_RI</b> event:
 - in default mode
 - with default data bits
 - with default parity
 - with default stop bits
 - with no flow control
 - at default baudrate

\note If Tests Default Mode <b>Synchronous Master/Slave</b> or <b>Single-wire</b> is selected this test is not executed
*/
void USART_Event_RI (void) {

  if (IsNotLoopback()   != EXIT_SUCCESS) { TEST_FAIL(); return; }
#if  (USART_SERVER_USED == 1)
  if (IsNotSync()       != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (IsNotSingleWire() != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (DriverInit()      != EXIT_SUCCESS) { TEST_FAIL(); return; }
  if (SettingsCheck     (USART_CFG_DEF_MODE, USART_CFG_DEF_DATA_BITS, USART_CFG_DEF_PARITY, USART_CFG_DEF_STOP_BITS, FLOW_CONTROL_NONE, RI_AVAILABLE, USART_CFG_DEF_BAUDRATE) != EXIT_SUCCESS) { TEST_FAIL(); return; }

  do {
    if (ComConfigDefault() != EXIT_SUCCESS) { break; }

    (void)drv->Control(USART_CFG_DEF_MODE_VAL      | 
                       USART_CFG_DEF_DATA_BITS_VAL | 
                       USART_CFG_DEF_PARITY_VAL    | 
                       USART_CFG_DEF_STOP_BITS_VAL | 
                       ARM_USART_FLOW_CONTROL_NONE , 
                       USART_CFG_DEF_BAUDRATE);

    event = 0;

    // Instruct USART Server to drive RI to active state for 20 ms
    if (CmdSetMdm(TO_RI_ON, 10U, 20U) != EXIT_SUCCESS) { break; }

    (void)osDelay(40U);

    // RI event should be active after RI returns to inactive state (Trailing Edge RI)
    TEST_ASSERT_MESSAGE((event & ARM_USART_EVENT_RI) == ARM_USART_EVENT_RI, "[FAILED] RI line did not register change!");

    return;
  } while (false);
#endif
}

/**
@}
*/
// End of usart_tests_evt
