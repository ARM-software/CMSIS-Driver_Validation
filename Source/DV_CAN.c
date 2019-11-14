/*-----------------------------------------------------------------------------
 *      Name:         DV_CAN.c
 *      Purpose:      CAN test cases
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#include "cmsis_dv.h"
#include "DV_Config.h"
#include "DV_Framework.h"
#include "Driver_CAN.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAN_MSG_SIZE          8U    // CAN data size bytes
#define CAN_MSG_SIZE_FD       64U   // CAN FD data size bytes

// CAN frame format according to BOSCH "CAN with Flexible Data-Rate" Specification Version 1.0
// released April 17th 2012

// CAN extended frame format bits (without datafield)
                                      // SOF BASEID SRR IDE IDEXT RTR r1     r0         DLC DATA CRC CRCDEL ACK EOF
#define CAN_EXT_FRAME_BITS            (    1    +11  +1  +1   +18  +1 +1     +1          +4      +21     +1  +2  +7 )

// CAN FD extended frame format bits sent at NOMINAL bitrate
                                      // SOF BASEID SRR IDE IDEXT     r1 EDL r0 BRS ESI DLC DATA CRC CRCDEL ACK EOF
#define CAN_EXT_FRAME_BITS_NOMINAL    (    1    +11  +1  +1   +18     +1  +1 +1  +1                          +2  +7 )

// CAN FD extended frame format bits sent at FD_DATA bitrate (without datafield)
                                      // SOF BASEID SRR IDE IDEXT     r1 EDL r0 BRS ESI DLC DATA CRC CRCDEL ACK EOF
#define CAN_EXT_FRAME_BITS_FD_DATA    (                                              +1  +4      +21     +1         )

// CAN buffer pointers
static uint8_t *buffer_out;
static uint8_t *buffer_in;

// CAN bitrates
const uint32_t CAN_BR[] = {
#if (CAN_BR_1>0)
  CAN_BR_1,
#endif
#if (CAN_BR_2>0)
  CAN_BR_2,
#endif
#if (CAN_BR_3>0)
  CAN_BR_3,
#endif
#if (CAN_BR_4>0)
  CAN_BR_4,
#endif
#if (CAN_BR_5>0)
  CAN_BR_5,
#endif
#if (CAN_BR_6>0)
  CAN_BR_6,
#endif
};
const uint32_t CAN_BR_NUM = ARRAY_SIZE(CAN_BR);

// Register Driver_CAN#
extern ARM_DRIVER_CAN CREATE_SYMBOL(Driver_CAN, DRV_CAN);
static ARM_DRIVER_CAN *drv = &CREATE_SYMBOL(Driver_CAN, DRV_CAN);
static ARM_CAN_CAPABILITIES capab;
static ARM_CAN_OBJ_CAPABILITIES obj_capab;

static char str[128];

// Event flags
static uint32_t volatile Event;

// Object index
uint32_t Obj_idx;

// CAN Signal Unit Event Callback
void CAN_SignalUnitEvent (uint32_t event) {

  switch (event) {
    case ARM_CAN_EVENT_UNIT_ACTIVE:
      break;
    case ARM_CAN_EVENT_UNIT_WARNING:
      break;
    case ARM_CAN_EVENT_UNIT_PASSIVE:
      break;
    case ARM_CAN_EVENT_UNIT_BUS_OFF:
      break;
  }
}

// CAN Signal Object Event Callback
void CAN_SignalObjectEvent (uint32_t obj_idx, uint32_t event) {
  Obj_idx = obj_idx;
  Event = event;
}

// CAN transfer
int8_t CAN_RunTransfer (uint32_t tx_obj_idx, ARM_CAN_MSG_INFO *tx_msg_info, const uint8_t *tx_data,
                        uint32_t rx_obj_idx, ARM_CAN_MSG_INFO *rx_msg_info, uint8_t *rx_data,
                        uint8_t size) {
  uint32_t tick;

  Event &= ~ARM_CAN_EVENT_RECEIVE;
  drv->MessageSend(tx_obj_idx, tx_msg_info, tx_data, size);

  tick = GET_SYSTICK();
  do {
    if ((Event & ARM_CAN_EVENT_RECEIVE)&&(Obj_idx == rx_obj_idx)) {
      drv->MessageRead(rx_obj_idx, rx_msg_info, rx_data, size);
      return 0;
    }
  }
  while ((GET_SYSTICK() - tick) < SYSTICK_MICROSEC(CAN_TRANSFER_TIMEOUT));
  return -1;
}


/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup can_funcs CAN Validation
\brief CAN test cases
\details
The CAN validation test checks the API interface compliance.
@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: CAN_GetCapabilities
\details
The test case \b CAN_GetCapabilities verifies the function \b GetCapabilities.
*/
void CAN_GetCapabilities (void) {
  /* Get CAN capabilities */
  capab = drv->GetCapabilities();
  TEST_ASSERT(&capab != NULL);
  /* Check number of available objects */
  if (capab.num_objects < 2U) {
    TEST_FAIL_MESSAGE("[FAILED] Driver has less than 2 objects available");
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: CAN_Initialization
\details
The test case \b CAN_Initialization verifies the CAN functions with the sequence:
  - Initialize  without callback
  - Uninitialize
  - Initialize with callback
  - Uninitialize
*/
void CAN_Initialization (void) {

  /* Initialize without callback */
  TEST_ASSERT(drv->Initialize(NULL, NULL) == ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);

  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(CAN_SignalUnitEvent, CAN_SignalObjectEvent) == ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: CAN_CheckInvalidInit
\details
The test case \b CAN_CheckInvalidInit verifies the driver behaviour when receiving an invalid initialization sequence:
  - Uninitialize
  - PowerControl with Power off
  - PowerControl with Power on
  - Set Mode
  - PowerControl with Power off
  - Uninitialize
*/
void CAN_CheckInvalidInit (void) {

  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);

  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);

  /* Try to power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) != ARM_DRIVER_OK);

  /* Try to set mode */
  TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_INITIALIZATION) != ARM_DRIVER_OK);

  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: CAN_PowerControl
\details
The test case \b CAN_PowerControl verifies the \b PowerControl function with the sequence:
 - Initialize
 - Power on
 - Power low
 - Power off
 - Uninitialize
*/
void CAN_PowerControl (void) {
  int32_t val;

  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(CAN_SignalUnitEvent, CAN_SignalObjectEvent) == ARM_DRIVER_OK);

  /* Power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Power low */
  val = drv->PowerControl (ARM_POWER_LOW);
  if (val == ARM_DRIVER_ERROR_UNSUPPORTED) { TEST_MESSAGE("[WARNING] Low power is not supported"); }
  else { TEST_ASSERT(val == ARM_DRIVER_OK); }

  /* Power off */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);

  /* Uninitialize */
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: CAN_Loopback_CheckBitrate
\details
The test case \b CAN_Loopback_CheckBitrate verifies different bitrates with the sequence:
 - Initialize
 - Power on
 - Change bitrate
 - Transfer and measure transfer time
 - Check received data against sent data
 - Power off
 - Uninitialize
*/
void CAN_Loopback_CheckBitrate (void) {
  int32_t val, i;
  uint32_t bitrate, clock;

  ARM_CAN_MSG_INFO tx_data_msg_info;
  ARM_CAN_MSG_INFO rx_data_msg_info;
  uint32_t tx_obj_idx = 0xFFFFFFFFU;
  uint32_t rx_obj_idx = 0xFFFFFFFFU;

  uint32_t ticks_measured;
  uint32_t ticks_expected;
  double rate;

  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(CAN_SignalUnitEvent, CAN_SignalObjectEvent) == ARM_DRIVER_OK);

  /* Power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Check if loopback is available */
  capab = drv->GetCapabilities();
  if ((capab.external_loopback == 0U) && (capab.internal_loopback == 0U)) {
    TEST_FAIL_MESSAGE("[FAILED] Driver does not support loopback mode");
  } else {

    /* Allocate buffer */
    buffer_out = (uint8_t*) malloc(CAN_MSG_SIZE*sizeof(uint8_t));
    TEST_ASSERT(buffer_out != NULL);
    buffer_in = (uint8_t*) malloc(CAN_MSG_SIZE*sizeof(uint8_t));
    TEST_ASSERT(buffer_in != NULL);

    /* Find first available object for receive and transmit */
    for (i = 0U; i < capab.num_objects; i++) {
      obj_capab = drv->ObjectGetCapabilities (i);
      if      ((tx_obj_idx == 0xFFFFFFFFU) && (obj_capab.tx == 1U)) { tx_obj_idx = i; }
      else if ((rx_obj_idx == 0xFFFFFFFFU) && (obj_capab.rx == 1U)) { rx_obj_idx = i; }
    }

    /* Set output buffer with all data = 0x55 to avoid CAN bit stuffing */
    memset(buffer_out,0x55U,CAN_MSG_SIZE);

    /* Get clock */
    clock = drv->GetClock();

    for (bitrate=0; bitrate<CAN_BR_NUM; bitrate++) {

      /* Activate initialization mode */
      TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_INITIALIZATION) == ARM_DRIVER_OK);

      val = ARM_DRIVER_ERROR;
      if ((clock % (5U*(CAN_BR[bitrate]*1000U))) == 0U) {               // If CAN base clock is divisible by 5 * nominal bitrate without remainder
        val = drv->SetBitrate   (ARM_CAN_BITRATE_NOMINAL,               // Set nominal bitrate
                                 CAN_BR[bitrate]*1000U,                 // Set nominal bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (2U) |           // Set propagation segment to 2 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |           // Set phase segment 1 to 1 time quantum (sample point at 80% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(1U) |           // Set phase segment 2 to 1 time quantum (total bit is 5 time quanta long)
                                 ARM_CAN_BIT_SJW       (1U));           // Resynchronization jump width is same as phase segment 2
      }
      if (val != ARM_DRIVER_OK) {                                       // If previous SetBitrate failed try different bit settings
        if ((clock % (6U*(CAN_BR[bitrate]*1000U))) == 0U) {             // If CAN base clock is divisible by 6 * nominal bitrate without remainder
          val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,               // Set nominal bitrate
                                 CAN_BR[bitrate]*1000U,                 // Set nominal bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (3U) |           // Set propagation segment to 3 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |           // Set phase segment 1 to 1 time quantum (sample point at 83.3% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(1U) |           // Set phase segment 2 to 1 time quantum (total bit is 6 time quanta long)
                                 ARM_CAN_BIT_SJW       (1U));           // Resynchronization jump width is same as phase segment 2
        }
      }
      if (val != ARM_DRIVER_OK) {                                       // If previous SetBitrate failed try different bit settings
        if ((clock % (8U*(CAN_BR[bitrate]*1000U))) == 0U) {             // If CAN base clock is divisible by 8 * nominal bitrate without remainder
          val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,               // Set nominal bitrate
                                 CAN_BR[bitrate]*1000U,                 // Set nominal bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (5U) |           // Set propagation segment to 5 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |           // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(1U) |           // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                                 ARM_CAN_BIT_SJW       (1U));           // Resynchronization jump width is same as phase segment 2
        }
      }
      if (val != ARM_DRIVER_OK) {                                       // If previous SetBitrate failed try different bit settings
        if ((clock % (10U*(CAN_BR[bitrate]*1000U))) == 0U) {            // If CAN base clock is divisible by 10 * nominal bitrate without remainder
          val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,               // Set nominal bitrate
                                 CAN_BR[bitrate]*1000U,                 // Set nominal bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (6U) |           // Set propagation segment to 6 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |           // Set phase segment 1 to 1 time quantum (sample point at 70% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(2U) |           // Set phase segment 2 to 2 time quantum (total bit is 10 time quanta long)
                                 ARM_CAN_BIT_SJW       (2U));           // Resynchronization jump width is same as phase segment 2
        }
      }
      if (val != ARM_DRIVER_OK) {
        snprintf(str,sizeof(str),"[WARNING] Invalid bitrate: %dkbit/s, clock %dMHz", CAN_BR[bitrate], clock/1000000U);
        TEST_MESSAGE(str);
      } else TEST_PASS();

      if (val == ARM_DRIVER_OK) {

        if (capab.external_loopback == 1U) {
          // Activate loopback external mode
          TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_LOOPBACK_EXTERNAL) == ARM_DRIVER_OK );
        } else if (capab.internal_loopback == 1U) {
          // Activate loopback internal mode
          TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_LOOPBACK_INTERNAL) == ARM_DRIVER_OK );
        }

        /* ObjectSetFilter add extended exact ID 0x15555555 */
        TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_EXTENDED_ID(0x15555555U), 0U) == ARM_DRIVER_OK );

        /* ObjectConfigure for tx and rx objects */
        TEST_ASSERT(drv->ObjectConfigure(tx_obj_idx, ARM_CAN_OBJ_TX) == ARM_DRIVER_OK );
        TEST_ASSERT(drv->ObjectConfigure(rx_obj_idx, ARM_CAN_OBJ_RX) == ARM_DRIVER_OK );

        /* Clear input buffer */
        memset(buffer_in,0,CAN_MSG_SIZE);

        memset(&tx_data_msg_info, 0U, sizeof(ARM_CAN_MSG_INFO));
        tx_data_msg_info.id = ARM_CAN_EXTENDED_ID(0x15555555U);

        /* Measure transfer time */
        ticks_measured = GET_SYSTICK();
        CAN_RunTransfer (tx_obj_idx, &tx_data_msg_info, buffer_out, rx_obj_idx, &rx_data_msg_info, buffer_in, CAN_MSG_SIZE);
        ticks_measured = GET_SYSTICK() - ticks_measured;
        ticks_expected = SYSTICK_MICROSEC((((CAN_MSG_SIZE * 8U) + CAN_EXT_FRAME_BITS) * 1000) / CAN_BR[bitrate]);

        rate = (double)ticks_measured/ticks_expected;

        if ((rate>(1.0+(double)MIN_BITRATE/100))||(rate<(1.0-(double)MIN_BITRATE/100))) {
          snprintf(str,sizeof(str),"[WARNING] At %dkbit/s: measured time is %f x expected time", CAN_BR[bitrate], rate);
          TEST_MESSAGE(str);
        } else TEST_PASS();

        /* Check received data against sent data*/
        if (memcmp(buffer_in, buffer_out, CAN_MSG_SIZE)!=0) {
          snprintf(str,sizeof(str),"[FAILED] At %dkbit/s: fail to check block of %d bytes", CAN_BR[bitrate], CAN_MSG_SIZE);
          TEST_FAIL_MESSAGE(str);
        } else TEST_PASS();

        /* ObjectSetFilter remove extended exact ID 0x15555555 */
        TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_REMOVE, ARM_CAN_EXTENDED_ID(0x15555555U), 0U) == ARM_DRIVER_OK );
      }
    }

    /* Free buffer */
    free(buffer_out);
    free(buffer_in);
  }

  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: CAN_Loopback_CheckBitrateFD
\details
The test case \b CAN_Loopback_CheckBitrateFD verifies different bitrates with the sequence:
 - Initialize
 - Power on
 - Change bitrate
 - Transfer and measure transfer time
 - Check received data against sent data
 - Power off
 - Uninitialize
*/
void CAN_Loopback_CheckBitrateFD (void) {
  int32_t val, i;
  uint32_t bitrate, clock;

  ARM_CAN_MSG_INFO tx_data_msg_info;
  ARM_CAN_MSG_INFO rx_data_msg_info;
  uint32_t tx_obj_idx = 0xFFFFFFFFU;
  uint32_t rx_obj_idx = 0xFFFFFFFFU;

  uint32_t ticks_measured;
  uint32_t ticks_expected;
  double rate;

  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(CAN_SignalUnitEvent, CAN_SignalObjectEvent) == ARM_DRIVER_OK);

  /* Power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Test FD mode */
  capab = drv->GetCapabilities();
  if (capab.fd_mode == 0U) {
    TEST_FAIL_MESSAGE("[FAILED] Driver does not support FD mode");
  } else {

    /* Check if loopback is available */
    if ((capab.external_loopback == 0U) && (capab.internal_loopback == 0U)) {
      TEST_FAIL_MESSAGE("[FAILED] Driver does not support loopback mode");
    } else {

      /* Allocate buffer */
      buffer_out = (uint8_t*) malloc(CAN_MSG_SIZE_FD*sizeof(uint8_t));
      TEST_ASSERT(buffer_out != NULL);
      buffer_in = (uint8_t*) malloc(CAN_MSG_SIZE_FD*sizeof(uint8_t));
      TEST_ASSERT(buffer_in != NULL);

      /* Find first available object for receive and transmit */
      for (i = 0U; i < capab.num_objects; i++) {
        obj_capab = drv->ObjectGetCapabilities (i);
        if      ((tx_obj_idx == 0xFFFFFFFFU) && (obj_capab.tx == 1U)) { tx_obj_idx = i; }
        else if ((rx_obj_idx == 0xFFFFFFFFU) && (obj_capab.rx == 1U)) { rx_obj_idx = i; }
      }

      /* Set output buffer with all data = 0x55 to avoid CAN bit stuffing */
      memset(buffer_out,0x55U,CAN_MSG_SIZE_FD);

      /* Get clock */
      clock = drv->GetClock();

      for (bitrate=0; bitrate<CAN_BR_NUM; bitrate++) {

        /* Activate initialization mode */
        TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_INITIALIZATION) == ARM_DRIVER_OK);

        val = ARM_DRIVER_ERROR;
        if ((clock % (5U*(CAN_BR[bitrate]*1000U*CAN_DATA_ARB_RATIO))) == 0U) {          // If CAN base clock is divisible by 5 * nominal bitrate without remainder
          val = drv->SetBitrate   (ARM_CAN_BITRATE_NOMINAL,                             // Set nominal bitrate
                                   CAN_BR[bitrate]*1000U,                               // Set nominal bitrate to configured constant value
                                   ARM_CAN_BIT_PROP_SEG  (2U) |                         // Set propagation segment to 2 time quanta
                                   ARM_CAN_BIT_PHASE_SEG1(1U) |                         // Set phase segment 1 to 1 time quantum (sample point at 80% of bit time)
                                   ARM_CAN_BIT_PHASE_SEG2(1U) |                         // Set phase segment 2 to 1 time quantum (total bit is 5 time quanta long)
                                   ARM_CAN_BIT_SJW       (1U));                         // Resynchronization jump width is same as phase segment 2
          if (val == ARM_DRIVER_OK) val = drv->SetBitrate (ARM_CAN_BITRATE_FD_DATA,     // Set FD data phase bitrate
                                   CAN_BR[bitrate]*1000U*CAN_DATA_ARB_RATIO,            // Set FD data phase bitrate to configured constant value
                                   ARM_CAN_BIT_PROP_SEG  (2U) |                         // Set propagation segment to 2 time quanta
                                   ARM_CAN_BIT_PHASE_SEG1(1U) |                         // Set phase segment 1 to 1 time quantum (sample point at 80% of bit time)
                                   ARM_CAN_BIT_PHASE_SEG2(1U) |                         // Set phase segment 2 to 1 time quantum (total bit is 5 time quanta long)
                                   ARM_CAN_BIT_SJW       (1U));                         // Resynchronization jump width is same as phase segment 2
        }
        if (val != ARM_DRIVER_OK) {                                                     // If previous SetBitrate failed try different bit settings
          if ((clock % (6U*(CAN_BR[bitrate]*1000U*CAN_DATA_ARB_RATIO))) == 0U) {        // If CAN base clock is divisible by 6 * nominal bitrate without remainder
            val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,                             // Set nominal bitrate
                                   CAN_BR[bitrate]*1000U,                               // Set nominal bitrate to configured constant value
                                   ARM_CAN_BIT_PROP_SEG  (3U) |                         // Set propagation segment to 3 time quanta
                                   ARM_CAN_BIT_PHASE_SEG1(1U) |                         // Set phase segment 1 to 1 time quantum (sample point at 83.3% of bit time)
                                   ARM_CAN_BIT_PHASE_SEG2(1U) |                         // Set phase segment 2 to 1 time quantum (total bit is 6 time quanta long)
                                   ARM_CAN_BIT_SJW       (1U));                         // Resynchronization jump width is same as phase segment 2
            if (val == ARM_DRIVER_OK) val = drv->SetBitrate (ARM_CAN_BITRATE_FD_DATA,   // Set FD data phase bitrate
                                   CAN_BR[bitrate]*1000U*CAN_DATA_ARB_RATIO,            // Set FD data phase bitrate to configured constant value
                                   ARM_CAN_BIT_PROP_SEG  (3U) |                         // Set propagation segment to 3 time quanta
                                   ARM_CAN_BIT_PHASE_SEG1(1U) |                         // Set phase segment 1 to 1 time quantum (sample point at 83.3% of bit time)
                                   ARM_CAN_BIT_PHASE_SEG2(1U) |                         // Set phase segment 2 to 1 time quantum (total bit is 6 time quanta long)
                                   ARM_CAN_BIT_SJW       (1U));                         // Resynchronization jump width is same as phase segment 2
          }
        }
        if (val != ARM_DRIVER_OK) {                                                     // If previous SetBitrate failed try different bit settings
          if ((clock % (8U*(CAN_BR[bitrate]*1000U*CAN_DATA_ARB_RATIO))) == 0U) {        // If CAN base clock is divisible by 8 * nominal bitrate without remainder
            val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,                             // Set nominal bitrate
                                   CAN_BR[bitrate]*1000U,                               // Set nominal bitrate to configured constant value
                                   ARM_CAN_BIT_PROP_SEG  (5U) |                         // Set propagation segment to 5 time quanta
                                   ARM_CAN_BIT_PHASE_SEG1(1U) |                         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                                   ARM_CAN_BIT_PHASE_SEG2(1U) |                         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                                   ARM_CAN_BIT_SJW       (1U));                         // Resynchronization jump width is same as phase segment 2
            if (val == ARM_DRIVER_OK) val = drv->SetBitrate (ARM_CAN_BITRATE_FD_DATA,   // Set FD data phase bitrate
                                   CAN_BR[bitrate]*1000U*CAN_DATA_ARB_RATIO,            // Set FD data phase bitrate to configured constant value
                                   ARM_CAN_BIT_PROP_SEG  (5U) |                         // Set propagation segment to 5 time quanta
                                   ARM_CAN_BIT_PHASE_SEG1(1U) |                         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                                   ARM_CAN_BIT_PHASE_SEG2(1U) |                         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                                   ARM_CAN_BIT_SJW       (1U));                         // Resynchronization jump width is same as phase segment 2
          }
        }
        if (val != ARM_DRIVER_OK) {                                                     // If previous SetBitrate failed try different bit settings
          if ((clock % (10U*(CAN_BR[bitrate]*1000U*CAN_DATA_ARB_RATIO))) == 0U) {// If CAN base clock is divisible by 10 * nominal bitrate without remainder
            val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,                             // Set nominal bitrate
                                   CAN_BR[bitrate]*1000U,                               // Set nominal bitrate to configured constant value
                                   ARM_CAN_BIT_PROP_SEG  (6U) |                         // Set propagation segment to 6 time quanta
                                   ARM_CAN_BIT_PHASE_SEG1(1U) |                         // Set phase segment 1 to 1 time quantum (sample point at 70% of bit time)
                                   ARM_CAN_BIT_PHASE_SEG2(2U) |                         // Set phase segment 2 to 2 time quantum (total bit is 10 time quanta long)
                                   ARM_CAN_BIT_SJW       (2U));                         // Resynchronization jump width is same as phase segment 2
            if (val == ARM_DRIVER_OK) val = drv->SetBitrate (ARM_CAN_BITRATE_FD_DATA,   // Set FD data phase bitrate
                                   CAN_BR[bitrate]*1000U*CAN_DATA_ARB_RATIO,            // Set FD data phase bitrate to configured constant value
                                   ARM_CAN_BIT_PROP_SEG  (6U) |                         // Set propagation segment to 6 time quanta
                                   ARM_CAN_BIT_PHASE_SEG1(1U) |                         // Set phase segment 1 to 1 time quantum (sample point at 70% of bit time)
                                   ARM_CAN_BIT_PHASE_SEG2(2U) |                         // Set phase segment 2 to 2 time quantum (total bit is 10 time quanta long)
                                   ARM_CAN_BIT_SJW       (2U));                         // Resynchronization jump width is same as phase segment 2
          }
        }
        if (val != ARM_DRIVER_OK) {
          snprintf(str,sizeof(str),"[WARNING] Invalid FD bitrate: %dkbit/s, clock %dMHz", CAN_BR[bitrate]*CAN_DATA_ARB_RATIO, clock/1000000U);
          TEST_MESSAGE(str);
        } else TEST_PASS();

        if (val == ARM_DRIVER_OK) {

          if (capab.external_loopback == 1U) {
            // Activate loopback external mode
            TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_LOOPBACK_EXTERNAL) == ARM_DRIVER_OK );
          } else if (capab.internal_loopback == 1U) {
            // Activate loopback internal mode
            TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_LOOPBACK_INTERNAL) == ARM_DRIVER_OK );
          }

          /* Set FD mode */
          TEST_ASSERT(drv->Control (ARM_CAN_SET_FD_MODE, 1) == ARM_DRIVER_OK);

          /* ObjectSetFilter add extended exact ID 0x15555555 */
          TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_EXTENDED_ID(0x15555555U), 0U) == ARM_DRIVER_OK );

          /* ObjectConfigure for tx and rx objects */
          TEST_ASSERT(drv->ObjectConfigure(tx_obj_idx, ARM_CAN_OBJ_TX) == ARM_DRIVER_OK );
          TEST_ASSERT(drv->ObjectConfigure(rx_obj_idx, ARM_CAN_OBJ_RX) == ARM_DRIVER_OK );

          /* Clear input buffer */
          memset(buffer_in,0,CAN_MSG_SIZE_FD);

          memset(&tx_data_msg_info, 0U, sizeof(ARM_CAN_MSG_INFO));
          tx_data_msg_info.id = ARM_CAN_EXTENDED_ID(0x15555555U);

          /* Measure transfer time */
          ticks_measured = GET_SYSTICK();
          CAN_RunTransfer (tx_obj_idx, &tx_data_msg_info, buffer_out, rx_obj_idx, &rx_data_msg_info, buffer_in, CAN_MSG_SIZE_FD);
          ticks_measured = GET_SYSTICK() - ticks_measured;
          ticks_expected = SYSTICK_MICROSEC((((((CAN_MSG_SIZE_FD * 8U) + CAN_EXT_FRAME_BITS_FD_DATA) * 1000) / (CAN_BR[bitrate] * CAN_DATA_ARB_RATIO)) +
                                                     (((CAN_EXT_FRAME_BITS_NOMINAL)                         * 1000) /  CAN_BR[bitrate]                      ) ));

          rate = (double)ticks_measured/ticks_expected;

          if ((rate>(1.0+(double)MIN_BITRATE/100))||(rate<(1.0-(double)MIN_BITRATE/100))) {
            snprintf(str,sizeof(str),"[WARNING] At FD bitrate %dkbit/s: measured time is %f x expected time", CAN_BR[bitrate]*CAN_DATA_ARB_RATIO, rate);
            TEST_MESSAGE(str);
          } else TEST_PASS();

          /* Check received data against sent data*/
          if (memcmp(buffer_in, buffer_out, CAN_MSG_SIZE_FD)!=0) {
            snprintf(str,sizeof(str),"[FAILED] At FD bitrate %dkbit/s: fail to check block of %d bytes", CAN_BR[bitrate]*CAN_DATA_ARB_RATIO, CAN_MSG_SIZE_FD);
            TEST_FAIL_MESSAGE(str);
          } else TEST_PASS();

          /* ObjectSetFilter remove extended exact ID 0x15555555 */
          TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_REMOVE, ARM_CAN_EXTENDED_ID(0x15555555U), 0U) == ARM_DRIVER_OK );
        }
      }

      /* Free buffer */
      free(buffer_out);
      free(buffer_in);
    }
  }

  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: CAN_Loopback_Transfer
\details
The test case \b CAN_Loopback_Transfer verifies the data transfers with the sequence:
 - Initialize
 - Power on
 - Set filter with standard ID
 - Transfer and check received data against sent data
 - Check filter with standard ID and remove it
 - Set filter with extended ID
 - Transfer and check received data against sent data
 - Check filter with extended ID and remove it
 - Power off
 - Uninitialize
*/
void CAN_Loopback_Transfer (void) {
  int32_t val;
  uint32_t i, cnt, clock;

  ARM_CAN_MSG_INFO tx_data_msg_info;
  ARM_CAN_MSG_INFO rx_data_msg_info;
  uint32_t tx_obj_idx = 0xFFFFFFFFU;
  uint32_t rx_obj_idx = 0xFFFFFFFFU;

  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(CAN_SignalUnitEvent, CAN_SignalObjectEvent) == ARM_DRIVER_OK);

  /* Power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Check if loopback is available */
  capab = drv->GetCapabilities();
  if ((capab.external_loopback == 0U) && (capab.internal_loopback == 0U)) {
    TEST_FAIL_MESSAGE("[FAILED] Driver does not support loopback mode");
  } else {

    /* Allocate buffer */
    buffer_out = (uint8_t*) malloc(CAN_MSG_SIZE*sizeof(uint8_t));
    TEST_ASSERT(buffer_out != NULL);
    buffer_in = (uint8_t*) malloc(CAN_MSG_SIZE*sizeof(uint8_t));
    TEST_ASSERT(buffer_in != NULL);

    /* Find first available object for receive and transmit */
    for (i = 0U; i < capab.num_objects; i++) {
      obj_capab = drv->ObjectGetCapabilities (i);
      if      ((tx_obj_idx == 0xFFFFFFFFU) && (obj_capab.tx == 1U)) { tx_obj_idx = i; }
      else if ((rx_obj_idx == 0xFFFFFFFFU) && (obj_capab.rx == 1U)) { rx_obj_idx = i; }
    }

    /* Set output buffer with random data */
    srand(GET_SYSTICK());
    for (cnt = 0; cnt<CAN_MSG_SIZE; cnt++) {
      buffer_out[cnt] = rand()%0x100U;
    }

    /* Activate initialization mode */
    TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_INITIALIZATION) == ARM_DRIVER_OK);

    if (capab.external_loopback != 0U) {
      // Activate loopback external mode
      TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_LOOPBACK_EXTERNAL) == ARM_DRIVER_OK );
    } else if (capab.internal_loopback == 1U) {
      // Activate loopback internal mode
      TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_LOOPBACK_INTERNAL) == ARM_DRIVER_OK );
    }

    /* Get clock */
    clock = drv->GetClock();

    val = ARM_DRIVER_ERROR;
    if ((clock % (5U*(CAN_BR[0]*1000U))) == 0U) {                       // If CAN base clock is divisible by 5 * nominal bitrate without remainder
      val = drv->SetBitrate   (ARM_CAN_BITRATE_NOMINAL,                 // Set nominal bitrate
                               CAN_BR[0]*1000U,                         // Set nominal bitrate to configured constant value
                               ARM_CAN_BIT_PROP_SEG  (2U) |             // Set propagation segment to 2 time quanta
                               ARM_CAN_BIT_PHASE_SEG1(1U) |             // Set phase segment 1 to 1 time quantum (sample point at 80% of bit time)
                               ARM_CAN_BIT_PHASE_SEG2(1U) |             // Set phase segment 2 to 1 time quantum (total bit is 5 time quanta long)
                               ARM_CAN_BIT_SJW       (1U));             // Resynchronization jump width is same as phase segment 2
    }
    if (val != ARM_DRIVER_OK) {                                         // If previous SetBitrate failed try different bit settings
      if ((clock % (6U*(CAN_BR[0]*1000U))) == 0U) {                     // If CAN base clock is divisible by 6 * nominal bitrate without remainder
        val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,                 // Set nominal bitrate
                               CAN_BR[0]*1000U,                         // Set nominal bitrate to configured constant value
                               ARM_CAN_BIT_PROP_SEG  (3U) |             // Set propagation segment to 3 time quanta
                               ARM_CAN_BIT_PHASE_SEG1(1U) |             // Set phase segment 1 to 1 time quantum (sample point at 83.3% of bit time)
                               ARM_CAN_BIT_PHASE_SEG2(1U) |             // Set phase segment 2 to 1 time quantum (total bit is 6 time quanta long)
                               ARM_CAN_BIT_SJW       (1U));             // Resynchronization jump width is same as phase segment 2
      }
    }
    if (val != ARM_DRIVER_OK) {                                         // If previous SetBitrate failed try different bit settings
      if ((clock % (8U*(CAN_BR[0]*1000U))) == 0U) {                     // If CAN base clock is divisible by 8 * nominal bitrate without remainder
        val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,                 // Set nominal bitrate
                               CAN_BR[0]*1000U,                         // Set nominal bitrate to configured constant value
                               ARM_CAN_BIT_PROP_SEG  (5U) |             // Set propagation segment to 5 time quanta
                               ARM_CAN_BIT_PHASE_SEG1(1U) |             // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                               ARM_CAN_BIT_PHASE_SEG2(1U) |             // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                               ARM_CAN_BIT_SJW       (1U));             // Resynchronization jump width is same as phase segment 2
      }
    }
    if (val != ARM_DRIVER_OK) {                                         // If previous SetBitrate failed try different bit settings
      if ((clock % (10U*(CAN_BR[0]*1000U))) == 0U) {                    // If CAN base clock is divisible by 10 * nominal bitrate without remainder
        val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,                 // Set nominal bitrate
                               CAN_BR[0]*1000U,                         // Set nominal bitrate to configured constant value
                               ARM_CAN_BIT_PROP_SEG  (6U) |             // Set propagation segment to 6 time quanta
                               ARM_CAN_BIT_PHASE_SEG1(1U) |             // Set phase segment 1 to 1 time quantum (sample point at 70% of bit time)
                               ARM_CAN_BIT_PHASE_SEG2(2U) |             // Set phase segment 2 to 2 time quantum (total bit is 10 time quanta long)
                               ARM_CAN_BIT_SJW       (2U));             // Resynchronization jump width is same as phase segment 2
      }
    }
    if (val != ARM_DRIVER_OK) {
      snprintf(str,sizeof(str),"[WARNING] Invalid bitrate: %dkbit/s, clock %dMHz", CAN_BR[0], clock/1000000U);
      TEST_MESSAGE(str);
    } else TEST_PASS();

    /* ObjectSetFilter add standard exact ID 0x7FF */
    TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_STANDARD_ID(0x7FFU), 0U) == ARM_DRIVER_OK );

    /* ObjectConfigure for tx and rx objects */
    TEST_ASSERT(drv->ObjectConfigure(tx_obj_idx, ARM_CAN_OBJ_TX) == ARM_DRIVER_OK );
    TEST_ASSERT(drv->ObjectConfigure(rx_obj_idx, ARM_CAN_OBJ_RX) == ARM_DRIVER_OK );

    memset(&tx_data_msg_info, 0U, sizeof(ARM_CAN_MSG_INFO));
    tx_data_msg_info.id = ARM_CAN_STANDARD_ID(0x7FFU);

    /* Transfer data chunks */
    for (cnt = 1; cnt <= CAN_MSG_SIZE; cnt++) {
      /* Clear input buffer */
      memset(buffer_in,0,CAN_MSG_SIZE);
      if (CAN_RunTransfer (tx_obj_idx, &tx_data_msg_info, buffer_out, rx_obj_idx, &rx_data_msg_info, buffer_in, cnt) != ARM_DRIVER_OK) {
        snprintf(str,sizeof(str),"[FAILED] Fail to transfer block of %d bytes",cnt);
        TEST_FAIL_MESSAGE(str);
      } else TEST_PASS();
      if (memcmp(buffer_in, buffer_out, cnt)!=0) {
        snprintf(str,sizeof(str),"[FAILED] Fail to check block of %d bytes",cnt);
        TEST_FAIL_MESSAGE(str);
      } else TEST_PASS();
    }

    /* Check if a different random ID is filtered */
    tx_data_msg_info.id = ARM_CAN_STANDARD_ID(rand()%0x7FFU);
    memset(buffer_in,0,CAN_MSG_SIZE);
    TEST_ASSERT(CAN_RunTransfer (tx_obj_idx, &tx_data_msg_info, buffer_out,
                                 rx_obj_idx, &rx_data_msg_info, buffer_in,
                                 CAN_MSG_SIZE) != ARM_DRIVER_OK);

    /* ObjectSetFilter remove standard exact ID 0x7FF */
    TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_REMOVE, ARM_CAN_STANDARD_ID(0x7FFU), 0U) == ARM_DRIVER_OK );


    /* ObjectSetFilter add extended exact ID 0x1FFFFFFF */
    TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_EXTENDED_ID(0x1FFFFFFFU), 0U) == ARM_DRIVER_OK );

    /* ObjectConfigure for tx and rx objects */
    TEST_ASSERT(drv->ObjectConfigure(tx_obj_idx, ARM_CAN_OBJ_TX) == ARM_DRIVER_OK );
    TEST_ASSERT(drv->ObjectConfigure(rx_obj_idx, ARM_CAN_OBJ_RX) == ARM_DRIVER_OK );

    memset(&tx_data_msg_info, 0U, sizeof(ARM_CAN_MSG_INFO));
    tx_data_msg_info.id = ARM_CAN_EXTENDED_ID(0x1FFFFFFFU);

    /* Transfer data chunks */
    for (cnt = 1; cnt <= CAN_MSG_SIZE; cnt++) {
      /* Clear input buffer */
      memset(buffer_in,0,CAN_MSG_SIZE);
      if (CAN_RunTransfer (tx_obj_idx, &tx_data_msg_info, buffer_out, rx_obj_idx, &rx_data_msg_info, buffer_in, cnt) != ARM_DRIVER_OK) {
        snprintf(str,sizeof(str),"[FAILED] Fail to transfer block of %d bytes",cnt);
        TEST_FAIL_MESSAGE(str);
      } else TEST_PASS();
      if (memcmp(buffer_in, buffer_out, cnt)!=0) {
        snprintf(str,sizeof(str),"[FAILED] Fail to check block of %d bytes",cnt);
        TEST_FAIL_MESSAGE(str);
      } else TEST_PASS();
    }

    /* Check if a different random ID is filtered */
    tx_data_msg_info.id = ARM_CAN_EXTENDED_ID(rand()%0x1FFFFFFFU);
    memset(buffer_in,0,CAN_MSG_SIZE);
    TEST_ASSERT(CAN_RunTransfer (tx_obj_idx, &tx_data_msg_info, buffer_out,
                                 rx_obj_idx, &rx_data_msg_info, buffer_in,
                                 CAN_MSG_SIZE) != ARM_DRIVER_OK);

    /* ObjectSetFilter remove extended exact ID 0x1FFFFFFF */
    TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_REMOVE, ARM_CAN_EXTENDED_ID(0x1FFFFFFFU), 0U) == ARM_DRIVER_OK );

    /* Free buffer */
    free(buffer_out);
    free(buffer_in);
  }

  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief  Test case: CAN_Loopback_TransferFD
\details
The test case \b CAN_Loopback_TransferFD verifies the data transfers with the sequence:
 - Initialize
 - Power on
 - Set filter with standard ID
 - Transfer and check received data against sent data
 - Check filter with standard ID and remove it
 - Set filter with extended ID
 - Transfer and check received data against sent data
 - Check filter with extended ID and remove it
 - Power off
 - Uninitialize
*/
void CAN_Loopback_TransferFD (void) {
  int32_t val;
  uint32_t i, cnt, clock;

  ARM_CAN_MSG_INFO tx_data_msg_info;
  ARM_CAN_MSG_INFO rx_data_msg_info;
  uint32_t tx_obj_idx = 0xFFFFFFFFU;
  uint32_t rx_obj_idx = 0xFFFFFFFFU;

  /* Initialize with callback */
  TEST_ASSERT(drv->Initialize(CAN_SignalUnitEvent, CAN_SignalObjectEvent) == ARM_DRIVER_OK);

  /* Power on */
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);

  /* Test FD mode */
  capab = drv->GetCapabilities();
  if (capab.fd_mode == 0U) {
    TEST_FAIL_MESSAGE("[FAILED] Driver does not support FD mode");
  } else {

    /* Check if loopback is available */
    if ((capab.external_loopback == 0U) && (capab.internal_loopback == 0U)) {
      TEST_FAIL_MESSAGE("[FAILED] Driver does not support loopback mode");
    } else {

      /* Allocate buffer */
      buffer_out = (uint8_t*) malloc(CAN_MSG_SIZE_FD*sizeof(uint8_t));
      TEST_ASSERT(buffer_out != NULL);
      buffer_in = (uint8_t*) malloc(CAN_MSG_SIZE_FD*sizeof(uint8_t));
      TEST_ASSERT(buffer_in != NULL);

      /* Find first available object for receive and transmit */
      for (i = 0U; i < capab.num_objects; i++) {
        obj_capab = drv->ObjectGetCapabilities (i);
        if      ((tx_obj_idx == 0xFFFFFFFFU) && (obj_capab.tx == 1U)) { tx_obj_idx = i; }
        else if ((rx_obj_idx == 0xFFFFFFFFU) && (obj_capab.rx == 1U)) { rx_obj_idx = i; }
      }

      /* Set output buffer with random data */
      srand(GET_SYSTICK());
      for (cnt = 0; cnt<CAN_MSG_SIZE_FD; cnt++) {
        buffer_out[cnt] = rand()%0x100;
      }

      /* Activate initialization mode */
      TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_INITIALIZATION) == ARM_DRIVER_OK);

      if (capab.external_loopback != 0U) {
        // Activate loopback external mode
        TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_LOOPBACK_EXTERNAL) == ARM_DRIVER_OK );
      } else if (capab.internal_loopback == 1U) {
        // Activate loopback internal mode
        TEST_ASSERT(drv->SetMode (ARM_CAN_MODE_LOOPBACK_INTERNAL) == ARM_DRIVER_OK );
      }

      /* Get clock */
      clock = drv->GetClock();

      val = ARM_DRIVER_ERROR;
      if ((clock % (5U*(CAN_BR[0]*1000U*CAN_DATA_ARB_RATIO))) == 0U) {                  // If CAN base clock is divisible by 5 * nominal bitrate without remainder
        val = drv->SetBitrate   (ARM_CAN_BITRATE_NOMINAL,                               // Set nominal bitrate
                                 CAN_BR[0]*1000U,                                       // Set nominal bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (2U) |                           // Set propagation segment to 2 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |                           // Set phase segment 1 to 1 time quantum (sample point at 80% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(1U) |                           // Set phase segment 2 to 1 time quantum (total bit is 5 time quanta long)
                                 ARM_CAN_BIT_SJW       (1U));                           // Resynchronization jump width is same as phase segment 2
        if (val == ARM_DRIVER_OK) val = drv->SetBitrate (ARM_CAN_BITRATE_FD_DATA,       // Set FD data phase bitrate
                                 CAN_BR[0]*1000U*CAN_DATA_ARB_RATIO,                    // Set FD data phase bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (2U) |                           // Set propagation segment to 2 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |                           // Set phase segment 1 to 1 time quantum (sample point at 80% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(1U) |                           // Set phase segment 2 to 1 time quantum (total bit is 5 time quanta long)
                                 ARM_CAN_BIT_SJW       (1U));                           // Resynchronization jump width is same as phase segment 2
      }
      if (val != ARM_DRIVER_OK) {                                                       // If previous SetBitrate failed try different bit settings
        if ((clock % (6U*(CAN_BR[0]*1000U*CAN_DATA_ARB_RATIO))) == 0U) {                // If CAN base clock is divisible by 6 * nominal bitrate without remainder
          val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,                               // Set nominal bitrate
                                 CAN_BR[0]*1000U,                                       // Set nominal bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (3U) |                           // Set propagation segment to 3 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |                           // Set phase segment 1 to 1 time quantum (sample point at 83.3% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(1U) |                           // Set phase segment 2 to 1 time quantum (total bit is 6 time quanta long)
                                 ARM_CAN_BIT_SJW       (1U));                           // Resynchronization jump width is same as phase segment 2
          if (val == ARM_DRIVER_OK) val = drv->SetBitrate (ARM_CAN_BITRATE_FD_DATA,     // Set FD data phase bitrate
                                 CAN_BR[0]*1000U*CAN_DATA_ARB_RATIO,                    // Set FD data phase bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (3U) |                           // Set propagation segment to 3 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |                           // Set phase segment 1 to 1 time quantum (sample point at 83.3% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(1U) |                           // Set phase segment 2 to 1 time quantum (total bit is 6 time quanta long)
                                 ARM_CAN_BIT_SJW       (1U));                           // Resynchronization jump width is same as phase segment 2
        }
      }
      if (val != ARM_DRIVER_OK) {                                                       // If previous SetBitrate failed try different bit settings
        if ((clock % (8U*(CAN_BR[0]*1000U*CAN_DATA_ARB_RATIO))) == 0U) {                // If CAN base clock is divisible by 8 * nominal bitrate without remainder
          val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,                               // Set nominal bitrate
                                 CAN_BR[0]*1000U,                                       // Set nominal bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (5U) |                           // Set propagation segment to 5 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |                           // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(1U) |                           // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                                 ARM_CAN_BIT_SJW       (1U));                           // Resynchronization jump width is same as phase segment 2
          if (val == ARM_DRIVER_OK) val = drv->SetBitrate (ARM_CAN_BITRATE_FD_DATA,     // Set FD data phase bitrate
                                 CAN_BR[0]*1000U*CAN_DATA_ARB_RATIO,                    // Set FD data phase bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (5U) |                           // Set propagation segment to 5 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |                           // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(1U) |                           // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                                 ARM_CAN_BIT_SJW       (1U));                           // Resynchronization jump width is same as phase segment 2
        }
      }
      if (val != ARM_DRIVER_OK) {                                                       // If previous SetBitrate failed try different bit settings
        if ((clock % (10U*(CAN_BR[0]*1000U*CAN_DATA_ARB_RATIO))) == 0U) {               // If CAN base clock is divisible by 10 * nominal bitrate without remainder
          val = drv->SetBitrate (ARM_CAN_BITRATE_NOMINAL,                               // Set nominal bitrate
                                 CAN_BR[0]*1000U,                                       // Set nominal bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (6U) |                           // Set propagation segment to 6 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |                           // Set phase segment 1 to 1 time quantum (sample point at 70% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(2U) |                           // Set phase segment 2 to 2 time quantum (total bit is 10 time quanta long)
                                 ARM_CAN_BIT_SJW       (2U));                           // Resynchronization jump width is same as phase segment 2
          if (val == ARM_DRIVER_OK) val = drv->SetBitrate (ARM_CAN_BITRATE_FD_DATA,     // Set FD data phase bitrate
                                 CAN_BR[0]*1000U*CAN_DATA_ARB_RATIO,                    // Set FD data phase bitrate to configured constant value
                                 ARM_CAN_BIT_PROP_SEG  (6U) |                           // Set propagation segment to 6 time quanta
                                 ARM_CAN_BIT_PHASE_SEG1(1U) |                           // Set phase segment 1 to 1 time quantum (sample point at 70% of bit time)
                                 ARM_CAN_BIT_PHASE_SEG2(2U) |                           // Set phase segment 2 to 2 time quantum (total bit is 10 time quanta long)
                                 ARM_CAN_BIT_SJW       (2U));                           // Resynchronization jump width is same as phase segment 2
        }
      }
      if (val != ARM_DRIVER_OK) {
        snprintf(str,sizeof(str),"[WARNING] Invalid FD bitrate: %dkbit/s, clock %dMHz", CAN_BR[0]*CAN_DATA_ARB_RATIO, clock/1000000U);
        TEST_MESSAGE(str);
      } else TEST_PASS();

      /* Set FD mode */
      TEST_ASSERT(drv->Control (ARM_CAN_SET_FD_MODE, 1) == ARM_DRIVER_OK);

      /* ObjectSetFilter add standard exact ID 0x7FF */
      TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_STANDARD_ID(0x7FFU), 0U) == ARM_DRIVER_OK );

      /* ObjectConfigure for tx and rx objects */
      TEST_ASSERT(drv->ObjectConfigure(tx_obj_idx, ARM_CAN_OBJ_TX) == ARM_DRIVER_OK );
      TEST_ASSERT(drv->ObjectConfigure(rx_obj_idx, ARM_CAN_OBJ_RX) == ARM_DRIVER_OK );

      memset(&tx_data_msg_info, 0U, sizeof(ARM_CAN_MSG_INFO));
      tx_data_msg_info.id = ARM_CAN_STANDARD_ID(0x7FFU);

      /* Transfer data chunks */
      for (cnt = 1; cnt <= CAN_MSG_SIZE_FD; cnt++) {
        /* Clear input buffer */
        memset(buffer_in,0,CAN_MSG_SIZE_FD);
        if (CAN_RunTransfer (tx_obj_idx, &tx_data_msg_info, buffer_out, rx_obj_idx, &rx_data_msg_info, buffer_in, cnt) != ARM_DRIVER_OK) {
          snprintf(str,sizeof(str),"[FAILED] Fail to transfer block of %d bytes",cnt);
          TEST_FAIL_MESSAGE(str);
        } else TEST_PASS();
        if (memcmp(buffer_in, buffer_out, cnt)!=0) {
          snprintf(str,sizeof(str),"[FAILED] Fail to check block of %d bytes",cnt);
          TEST_FAIL_MESSAGE(str);
        } else TEST_PASS();
      }

      /* Check if a different random ID is filtered */
      tx_data_msg_info.id = ARM_CAN_STANDARD_ID(rand()%0x7FFU);
      memset(buffer_in,0,CAN_MSG_SIZE_FD);
      TEST_ASSERT(CAN_RunTransfer (tx_obj_idx, &tx_data_msg_info, buffer_out,
                                   rx_obj_idx, &rx_data_msg_info, buffer_in,
                                   CAN_MSG_SIZE_FD) != ARM_DRIVER_OK);

      /* ObjectSetFilter remove standard exact ID 0x7FF */
      TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_REMOVE, ARM_CAN_STANDARD_ID(0x7FFU), 0U) == ARM_DRIVER_OK );


      /* ObjectSetFilter add extended exact ID 0x1FFFFFFF */
      TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_EXTENDED_ID(0x1FFFFFFFU), 0U) == ARM_DRIVER_OK );

      /* ObjectConfigure for tx and rx objects */
      TEST_ASSERT(drv->ObjectConfigure(tx_obj_idx, ARM_CAN_OBJ_TX) == ARM_DRIVER_OK );
      TEST_ASSERT(drv->ObjectConfigure(rx_obj_idx, ARM_CAN_OBJ_RX) == ARM_DRIVER_OK );

      memset(&tx_data_msg_info, 0U, sizeof(ARM_CAN_MSG_INFO));
      tx_data_msg_info.id = ARM_CAN_EXTENDED_ID(0x1FFFFFFFU);

      /* Transfer data chunks */
      for (cnt = 1; cnt <= CAN_MSG_SIZE_FD; cnt++) {
        /* Clear input buffer */
        memset(buffer_in,0,CAN_MSG_SIZE_FD);
        if (CAN_RunTransfer (tx_obj_idx, &tx_data_msg_info, buffer_out, rx_obj_idx, &rx_data_msg_info, buffer_in, cnt) != ARM_DRIVER_OK) {
          snprintf(str,sizeof(str),"[FAILED] Fail to transfer block of %d bytes",cnt);
          TEST_FAIL_MESSAGE(str);
        } else TEST_PASS();
        if (memcmp(buffer_in, buffer_out, cnt)!=0) {
          snprintf(str,sizeof(str),"[FAILED] Fail to check block of %d bytes",cnt);
          TEST_FAIL_MESSAGE(str);
        } else TEST_PASS();
      }

      /* Check if a different random ID is filtered */
      tx_data_msg_info.id = ARM_CAN_EXTENDED_ID(rand()%0x1FFFFFFFU);
      memset(buffer_in,0,CAN_MSG_SIZE_FD);
      TEST_ASSERT(CAN_RunTransfer (tx_obj_idx, &tx_data_msg_info, buffer_out,
                                   rx_obj_idx, &rx_data_msg_info, buffer_in,
                                   CAN_MSG_SIZE_FD) != ARM_DRIVER_OK);

      /* ObjectSetFilter remove extended exact ID 0x1FFFFFFF */
      TEST_ASSERT(drv->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_REMOVE, ARM_CAN_EXTENDED_ID(0x1FFFFFFFU), 0U) == ARM_DRIVER_OK );

      /* Free buffer */
      free(buffer_out);
      free(buffer_in);
    }
  }

  /* Power off and uninitialize*/
  TEST_ASSERT(drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize() == ARM_DRIVER_OK);
}

/**
@}
*/
// end of group can_funcs

