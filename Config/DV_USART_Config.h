/*
 * Copyright (c) 2015-2021 Arm Limited. All rights reserved.
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
 * $Revision:   V2.0.0
 *
 * Project:     CMSIS-Driver Validation
 * Title:       Universal Synchronous Asynchronous Receiver/Transmitter (USART) 
 *              driver validation configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_USART_CONFIG_H_
#define DV_USART_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> USART
//   <i> Universal Synchronous Asynchronous Receiver/Transmitter (USART) driver validation configuration
//   <o0> Driver_USART# <0-255>
//     <i> Choose the Driver_USART# instance to test.
//     <i> For example to test Driver_USART0 select 0.
//   <h> Configuration
//     <i> Test Mode, USART Server and Tests configuration.
//     <o1.0> Test Mode
//       <i> Select test mode: Loopback or USART Server.
//       <i> For Loopback test mode connect the Tx and Rx lines directly.
//       <i> Loopback test mode is used for basic driver functionality testing before using USART Server test mode.
//       <i> For USART Server test mode connect: 
//       <i>  - Tx line to Rx on the USART Server 
//       <i>  - Rx line to Tx on the USART Server
//       <i>  - GND to same line on the USART Server
//       <i> USART Server test mode is used for extensive driver functionality testing.
//       <0=> Loopback
//       <1=> USART Server
//     <h> USART Server
//       <i> USART Server configuration.
//       <i> Specifies the communication settings at which Driver Validation communicates with the USART Server.
//       <i> These settings must be same as settings configured and used by the USART Server.
//       <i> Fixed settings:
//       <i> Baudrate: 115200
//       <i> Data Bits: 8
//       <i> Parity: None
//       <i> Stop Bits: 1
//       <i> Flow Control: None
//       <o2> Mode
//         <i> Select mode setting used by the USART Server.
//         <1=> Asynchronous
//         <4=> Single-wire
//         <5=> IrDA
//     </h>
//     <h> Tests
//       <i> Tests configuration
//       <h> Default settings
//         <i> Default settings used for tests.
//         <o4> Mode
//           <i> Select default mode setting used for tests.
//           <1=> Asynchronous
//           <2=> Synchronous Master
//           <3=> Synchronous Slave
//           <4=> Single-wire
//           <5=> IrDA
//         <o5> Data Bits <5=> 5 <6=> 6 <7=> 7 <8=> 8 <9=> 9
//           <i> Select default data bits setting for tests.
//           <i> This setting is used for all tests except the Data Exchange: Data Bits tests.
//         <o6> Parity <0=> None <1=> Even <2=> Odd
//           <i> Select default parity setting for tests.
//           <i> This setting is used for all tests except the Data Exchange: Parity tests.
//         <o7> Stop Bits <0=> 1 <1=> 2 <2=> 1.5 <3=> 0.5
//           <i> Select default stop bits setting for tests.
//           <i> This setting is used for all tests except the Data Exchange: Stop Bits tests.
//         <o8> Flow control <0=> No <1=> RTS <2=> CTS <3=> RTS/CTS
//           <i> Select default flow control setting for tests.
//           <i> This setting is used for all tests except the Data Exchange: Flow Control tests.
//         <o9> Clock Polarity
//           <i> Select default clock polarity for tests.
//           <i> This setting is used for all tests except the Data Exchange: Polarity Phase tests.
//           <i> This setting is only relevant for synchronous mode.
//           <0=> Clock Polarity 0
//           <1=> Clock Polarity 1
//         <o10> Clock Phase
//           <i> Select default clock phase setting for tests.
//           <i> This setting is used for all tests except the Data Exchange: Polarity Phase tests.
//           <i> This setting is only relevant for synchronous mode.
//           <0=> Clock Phase 0
//           <1=> Clock Phase 1
//         <o11> Baudrate <115200-1000000000>
//           <i> Select default baudrate setting for tests.
//           <i> This setting is used for all tests except the Data Exchange: Baudrate tests.
//         <o12> Number of Items <1-1024>
//           <i> Select default number of data items for tests.
//           <i> This setting is used for all tests except the Data Exchange: Other: USART_Number_Of_Items tests.
//       </h>
//       <h> Baudrate
//         <i> Baudrate tests configuration.
//         <o13> Minimum Baudrate <115200-1000000000>
//           <i> Select minimum baudrate setting.
//           <i> This setting is used only in USART_Baudrate_Min test function.
//         <o14> Maximum Baudrate <115200-1000000000>
//           <i> Select maximum baudrate setting.
//           <i> This setting is used only in USART_Baudrate_Max test function.
//       </h>
//       <h> Number of Items
//         <i> Number of items test configuration.
//         <i> This setting is used only in USART_Number_Of_Items test function.
//         <i> (Value 0 means setting is not used)
//         <o15> Number of Items 1 <0-1024>
//         <o16> Number of Items 2 <0-1024>
//         <o17> Number of Items 3 <0-1024>
//         <o18> Number of Items 4 <0-1024>
//         <o19> Number of Items 5 <0-1024>
//       </h>
//     </h>
//   </h>
//   <h> Tests
//     <i> Enable / disable tests.
//     <e21> Driver Management
//       <i> Enable / disable driver management tests (functions: GetVersion, GetCapabilities, Initialize, Uninitialize, PowerControl).
//       <q22> USART_GetVersion
//         <i> Enable / disable GetVersion function tests.
//       <q23> USART_GetCapabilities
//         <i> Enable / disable GetCapabilities function tests.
//       <q24> USART_Initialize_Uninitialize
//         <i> Enable / disable Initialize and Uninitialize functions tests.
//       <q25> USART_PowerControl
//         <i> Enable / disable PowerControl function tests.
//     </e>
//     <e26> Data Exchange
//       <i> Enable / disable data exchange tests (functions: Send, Receive, Transfer, GetTxCount, GetRxCount, Control, GetStatus, SignalEvent).
//       <e27> Mode
//         <i> Enable / disable USART mode tests.
//         <i> (for Loopback test mode only Asynchronous, Synchronous Master and Single-wire mode tests are supported!)
//         <q28> USART_Mode_Asynchronous
//           <i> Enable / disable data exchange in Asynchronous mode test.
//         <q29> USART_Mode_Synchronous_Master
//           <i> Enable / disable data exchange in Synchronous Master mode test.
//         <q30> USART_Mode_Synchronous_Slave
//           <i> Enable / disable data exchange in Synchronous Slave mode test.
//         <q31> USART_Mode_Single_Wire
//           <i> Enable / disable data exchange in Single-wire mode test.
//           <i> This test is supported only in Loopback test mode!
//         <q32> USART_Mode_IrDA
//           <i> Enable / disable data exchange in Infra-red Data mode test.
//           <i> This test requires IrDA hardware.
//       </e>
//       <e33> Data Bits
//         <i> Enable / disable data bits tests.
//         <i> (for Loopback test mode only: 8 data bit test is supported!)
//         <q34> USART_Data_Bits_5
//           <i> Enable / disable data exchange with 5 data bits per packet test.
//         <q35> USART_Data_Bits_6
//           <i> Enable / disable data exchange with 6 data bits per packet test.
//         <q36> USART_Data_Bits_7
//           <i> Enable / disable data exchange with 7 data bits per packet test.
//         <q37> USART_Data_Bits_8
//           <i> Enable / disable data exchange with 8 data bits per packet test.
//         <q38> USART_Data_Bits_9
//           <i> Enable / disable data exchange with 9 dtat bits per packet test.
//       </e>
//       <e39> Parity
//         <i> Enable / disable parity tests.
//         <i> (all of these tests are supported only in USART Server test mode!)
//         <q40> USART_Parity_None
//           <i> Enable / disable data exchange with no parity test.
//         <q41> USART_Parity_Even
//           <i> Enable / disable data exchange with even parity test.
//         <q42> USART_Parity_Odd
//           <i> Enable / disable data exchange odd parity test.
//       </e>
//       <e43> Stop Bits
//         <i> Enable / disable stop bits tests.
//         <i> (all of these tests are supported only in USART Server test mode!)
//         <q44> USART_Stop_Bits_1
//           <i> Enable / disable data exchange with 1 stop bit test.
//         <q45> USART_Stop_Bits_2
//           <i> Enable / disable data exchange with 2 stop bits test.
//         <q46> USART_Stop_Bits_1_5
//           <i> Enable / disable data exchange with 1.5 stop bits test.
//         <q47> USART_Stop_Bits_0_5
//           <i> Enable / disable data exchange with 0.5 stop bit test.
//       </e>
//       <e48> Flow Control
//         <i> Enable / disable flow control tests.
//         <i> (all of these tests are supported only in USART Server test mode!)
//         <q49> USART_Flow_Control_None
//           <i> Enable / disable data exchange with no flow control test.
//         <q50> USART_Flow_Control_RTS
//           <i> Enable / disable data exchange with no flow control using RTS signal test.
//         <q51> USART_Flow_Control_CTS
//           <i> Enable / disable data exchange with no flow control using CTS signal test.
//         <q52> USART_Flow_Control_RTS_CTS
//           <i> Enable / disable data exchange with no flow control using RTS and CTS signals test.
//       </e>
//       <e53> Clock Format
//         <i> Enable / disable clock format tests.
//         <i> (all of these tests are supported only in USART Server test mode!)
//         <q54> USART_Clock_Pol0_Pha0
//           <i> Enable / disable data exchange with clock format: Polarity 0 / Phase 0 test.
//         <q55> USART_Clock_Pol0_Pha1
//           <i> Enable / disable data exchange with clock format: Polarity 0 / Phase 1 test.
//         <q56> USART_Clock_Pol1_Pha0
//           <i> Enable / disable data exchange with clock format: Polarity 1 / Phase 0 test.
//         <q57> USART_Clock_Pol1_Pha1
//           <i> Enable / disable data exchange with clock format: Polarity 1 / Phase 1 test.
//       </e>
//       <e58> Baudrate
//         <i> Enable / disable baudrate tests.
//         <q59> USART_Baudrate_Min
//           <i> Enable / disable data exchange at minimum supported baudrate test.
//         <q60> USART_Baudrate_Max
//           <i> Enable / disable data exchange at maximum supported baudrate test.
//       </e>
//       <e61> Other
//         <i> Enable / disable other tests.
//         <q62> USART_Number_Of_Items
//           <i> Enable / disable data exchange with different number of data items test.
//         <q63> USART_GetTxCount
//           <i> Enable / disable GetTxCount count changing during data exchange (Send) test.
//         <q64> USART_GetRxCount
//           <i> Enable / disable GetRxCount count changing during data exchange (Receive) test.
//         <q65> USART_GetTxRxCount
//           <> Enable / disable GetTxRxCount count changing during data exchange (Transfer) test (in Synchronous Master mode only).
//         <q66> USART_AbortSend
//           <i> Enable / disable data exchange Abort Send test.
//         <q67> USART_AbortReceive
//           <i> Enable / disable data exchange Abort Receive test.
//         <q68> USART_AbortTransfer
//           <i> Enable / disable data exchange Abort Transfer test.
//         <q69> USART_TxBreak
//           <i> Enable / disable Break signaling (Tx) test.
//       </e>
//     </e>
//     <e70> Modem
//       <i> Enable / disable modem line tests (functions: SetModemControl, GetModemStatus).
//       <i> (all of these tests are supported only in USART Server test mode!)
//       <q71> USART_SetModem_RTS
//         <i> Enable / disable driving of RTS (Request To Send) modem line test.
//       <q72> USART_SetModem_DTR
//         <i> Enable / disable driving of DTR (Data Terminal Ready) modem line test.
//       <q73> USART_GetModem_CTS
//         <i> Enable / disable reading of CTS (Clear To Send) modem line test.
//       <q74> USART_GetModem_DSR
//         <i> Enable / disable reading of DSR (Data Set Ready) modem line test.
//       <q75> USART_GetModem_DCD
//         <i> Enable / disable reading of DCD (Data Carrier Detect) modem line test.
//       <q76> USART_GetModem_RI
//         <i> Enable / disable reading of RI (Ring Indicator) modem line test.
//     </e>
//     <e77> Event
//       <i> Enable / disable event signaling tests (function: SignalEvent).
//       <i> (all of these tests are supported only in USART Server test mode!)
//       <q78> USART_Tx_Underflow
//         <i> Enable / disable ARM_USART_EVENT_TX_UNDERFLOW event generation test.
//       <q79> USART_Rx_Overflow
//         <i> Enable / disable ARM_USART_EVENT_RX_OVERFLOW event generation test.
//       <q80> USART_Rx_Timeout
//         <i> Enable / disable ARM_USART_EVENT_RX_TIMEOUT event generation test.
//       <q81> USART_Rx_Break
//         <i> Enable / disable ARM_USART_EVENT_RX_BREAK event generation test.
//       <q82> USART_Rx_Framing_Error
//         <i> Enable / disable ARM_USART_EVENT_RX_FRAMING_ERROR event generation test.
//       <q83> USART_Rx_Parity_Error
//         <i> Enable / disable ARM_USART_EVENT_RX_PARITY_ERROR event generation test.
//       <q84> USART_Event_CTS
//         <i> Enable / disable ARM_USART_EVENT_CTS  event generation test.
//       <q85> USART_Event_DSR
//         <i> Enable / disable ARM_USART_EVENT_DSR event generation test.
//       <q86> USART_Event_DCD
//         <i> Enable / disable ARM_USART_EVENT_DCD event generation test.
//       <q87> USART_Event_RI
//         <i> Enable / disable ARM_USART_EVENT_RI event generation test.
//     </e>
//   </h>
// </h>

#define DRV_USART                       0
#define USART_CFG_TEST_MODE             1
#define USART_CFG_SRV_MODE              1
#define USART_CFG_SRV_CMD_TOUT          100
#define USART_CFG_DEF_MODE              1
#define USART_CFG_DEF_DATA_BITS         8
#define USART_CFG_DEF_PARITY            0
#define USART_CFG_DEF_STOP_BITS         0
#define USART_CFG_DEF_FLOW_CONTROL      0
#define USART_CFG_DEF_CPOL              0
#define USART_CFG_DEF_CPHA              0
#define USART_CFG_DEF_BAUDRATE          115200
#define USART_CFG_DEF_NUM               512
#define USART_CFG_MIN_BAUDRATE          115200
#define USART_CFG_MAX_BAUDRATE          921600
#define USART_CFG_NUM1                  1
#define USART_CFG_NUM2                  31
#define USART_CFG_NUM3                  65
#define USART_CFG_NUM4                  1023
#define USART_CFG_NUM5                  1024
#define USART_CFG_XFER_TIMEOUT          200
#define USART_TG_DRIVER_MANAGEMENT_EN   1
#define USART_TC_GET_VERSION_EN         1
#define USART_TC_GET_CAPABILITIES_EN    1
#define USART_TC_INIT_UNINIT_EN         1
#define USART_TC_POWER_CONTROL_EN       1
#define USART_TG_DATA_EXCHANGE_EN       1
#define USART_TG_MODE_EN                1
#define USART_TC_ASYNC_EN               1
#define USART_TC_SYNC_MASTER_EN         0
#define USART_TC_SYNC_SLAVE_EN          0
#define USART_TC_SINGLE_WIRE_EN         0
#define USART_TC_IRDA_EN                0
#define USART_TG_DATA_BITS_EN           1
#define USART_TC_DATA_BITS_5_EN         0
#define USART_TC_DATA_BITS_6_EN         0
#define USART_TC_DATA_BITS_7_EN         0
#define USART_TC_DATA_BITS_8_EN         1
#define USART_TC_DATA_BITS_9_EN         0
#define USART_TG_PARITY_EN              1
#define USART_TC_PARITY_NONE_EN         1
#define USART_TC_PARITY_EVEN_EN         1
#define USART_TC_PARITY_ODD_EN          1
#define USART_TG_STOP_BITS_EN           1
#define USART_TC_STOP_BITS_1_EN         1
#define USART_TC_STOP_BITS_2_EN         1
#define USART_TC_STOP_BITS_1_5_EN       0
#define USART_TC_STOP_BITS_0_5_EN       0
#define USART_TG_FLOW_CTRL_EN           0
#define USART_TC_FLOW_CTRL_NONE_EN      1
#define USART_TC_FLOW_CTRL_RTS_EN       1
#define USART_TC_FLOW_CTRL_CTS_EN       1
#define USART_TC_FLOW_CTRL_RTS_CTS_EN   1
#define USART_TG_CLOCK_EN               0
#define USART_TC_CLOCK_POL0_PHA0_EN     1
#define USART_TC_CLOCK_POL0_PHA1_EN     1
#define USART_TC_CLOCK_POL1_PHA0_EN     1
#define USART_TC_CLOCK_POL1_PHA1_EN     1
#define USART_TG_BAUDRATE_EN            1
#define USART_TC_BAUDRATE_MIN_EN        1
#define USART_TC_BAUDRATE_MAX_EN        1
#define USART_TG_OTHER_EN               1
#define USART_TC_NUMBER_OF_ITEMS_EN     1
#define USART_TC_GET_TX_COUNT_EN        1
#define USART_TC_GET_RX_COUNT_EN        1
#define USART_TC_GET_TX_RX_COUNT_EN     0
#define USART_TC_ABORT_SEND_EN          1
#define USART_TC_ABORT_RECEIVE_EN       1
#define USART_TC_ABORT_TRANSFER_EN      0
#define USART_TC_TX_BREAK_EN            1
#define USART_TG_MODEM_EN               0
#define USART_TC_MODEM_RTS_EN           1
#define USART_TC_MODEM_DTR_EN           0
#define USART_TC_MODEM_CTS_EN           1
#define USART_TC_MODEM_DSR_EN           0
#define USART_TC_MODEM_DCD_EN           0
#define USART_TC_MODEM_RI_EN            0
#define USART_TG_EVENT_EN               0
#define USART_TC_TX_UNDERFLOW_EN        0
#define USART_TC_RX_OVERFLOW_EN         1
#define USART_TC_RX_TIMEOUT_EN          1
#define USART_TC_RX_BREAK_EN            1
#define USART_TC_RX_FRAMING_ERROR_EN    1
#define USART_TC_RX_PARITY_ERROR_EN     1
#define USART_TC_EVENT_CTS_EN           0
#define USART_TC_EVENT_DSR_EN           0
#define USART_TC_EVENT_DCD_EN           0
#define USART_TC_EVENT_RI_EN            0

#endif /* DV_USART_CONFIG_H_ */
