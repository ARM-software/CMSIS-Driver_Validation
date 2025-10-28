/*
 * Copyright (c) 2015-2025 Arm Limited. All rights reserved.
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
 * $Revision:   V1.1.0
 *
 * Project:     CMSIS-Driver Validation
 * Title:       Serial Peripheral Interface Bus (SPI) driver validation 
 *              configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_SPI_CONFIG_H_
#define DV_SPI_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//------ With VS Code: Open Preview for Configuration Wizard -------------------

// <h> SPI
//   <i> Serial Peripheral Interface Bus (SPI) driver validation configuration
//   <o0> Driver_SPI# <0-255>
//     <i> Choose the Driver_SPI# instance to test.
//     <i> For example to test Driver_SPI0 select 0.
//   <h> Configuration
//     <i> Test Mode, SPI Server and Tests configuration.
//     <o1.0> Test Mode
//       <i> Select test mode: Loopback or SPI Server.
//       <i> For Loopback test mode connect the MISO and MOSI lines directly.
//       <i> Loopback test mode is used for basic driver functionality testing before using SPI Server test mode.
//       <i> For SPI Server test mode connect: MISO, MOSI, SCK, SS and GND lines to the same lines on the SPI Server.
//       <i> SPI Server test mode is used for extensive driver functionality testing.
//       <0=> Loopback
//       <1=> SPI Server
//     <h> SPI Server
//       <i> SPI Server configuration.
//       <i> Specifies the communication settings at which Driver Validation communicates with the SPI Server.
//       <i> Fixed settings:
//       <i> Mode: Master
//       <i> Clock / Frame Format: Clock Polarity 0, Clock Phase 0
//       <i> Data Bits: 8
//       <i> Bit Order: MSB to LSB
//       <o2> Slave Select
//         <i> Select mode of driving Slave Select line.
//         <1=> Software Controlled
//         <2=> Hardware Controlled
//       <o3> Bus Speed <10000-1000000000>
//         <i> Select bus speed setting (in bps) used by the SPI Server.
//     </h>
//     <h> Tests
//       <i> Tests configuration
//       <h> Default settings
//         <i> Default settings used for tests.
//         <o5> Slave Select
//           <i> Select default mode of driving Slave Select line in tests.
//           <i> This setting is used for all tests except the Data Exchange: Mode tests.
//           <0=> Not used
//           <1=> Software Controlled
//           <2=> Hardware Controlled
//         <o6> Clock / Frame Format
//           <i> Select default clock / frame format setting for tests.
//           <i> This setting is used for all tests except the Data Exchange: Clock / Frame Format tests.
//           <0=> Clock Polarity 0, Clock Phase 0
//           <1=> Clock Polarity 0, Clock Phase 1
//           <2=> Clock Polarity 1, Clock Phase 0
//           <3=> Clock Polarity 1, Clock Phase 1
//           <4=> Texas Instruments Frame Format
//           <5=> National Semiconductor Microwire Frame Format
//         <o7> Data Bits <1-32>
//           <i> Select default data bits setting for tests.
//           <i> This setting is used for all tests except the Data Exchange: Data Bits tests.
//         <o8> Bit Order <0=> MSB to LSB <1=> LSB to MSB
//           <i> Select default bit order setting for tests.
//           <i> This setting is used for all tests except the Data Exchange: Bit Order tests.
//         <o9> Bus Speed <10000-1000000000>
//           <i> Select default bus speed setting for tests.
//           <i> This setting is used for all tests except the Data Exchange: Bus Speed tests.
//         <o10> Number of Items <1-1024>
//           <i> Select default number of data items for tests.
//           <i> This setting is used for all tests except the Data Exchange: Other: SPI_Number_Of_Items tests.
//       </h>
//       <h> Bus Speed
//         <i> Bus speed tests configuration.
//         <o11> Minimum Bus Speed <10000-1000000000>
//           <i> Select minimum bus speed setting (in bps).
//           <i> This setting is used only in SPI_Bus_Speed_Min test function.
//         <o12> Maximum Bus Speed <10000-1000000000>
//           <i> Select maximum bus speed setting (in bps).
//           <i> This setting is used only in SPI_Bus_Speed_Max test function.
//       </h>
//       <h> Number of Items
//         <i> Number of items test configuration.
//         <i> This setting is used only in SPI_Number_Of_Items test function.
//         <i> (Value 0 means setting is not used)
//         <o13> Number of Items 1 <0-1024>
//         <o14> Number of Items 2 <0-1024>
//         <o15> Number of Items 3 <0-1024>
//         <o16> Number of Items 4 <0-1024>
//         <o17> Number of Items 5 <0-1024>
//       </h>
//     </h>
//   </h>
//   <h> Tests
//     <i> Enable / disable tests.
//     <e19> Driver Management
//       <i> Enable / disable driver management tests (functions: GetVersion, GetCapabilities, Initialize, Uninitialize, PowerControl).
//       <q20> SPI_GetVersion
//         <i> Enable / disable GetVersion function tests.
//       <q21> SPI_GetCapabilities
//         <i> Enable / disable GetCapabilities function tests.
//       <q22> SPI_Initialize_Uninitialize
//         <i> Enable / disable Initialize and Uninitialize functions tests.
//       <q23> SPI_PowerControl
//         <i> Enable / disable PowerControl function tests.
//     </e>
//     <e24> Data Exchange
//       <i> Enable / disable data exchange tests (functions: Send, Receive, Transfer, GetDataCount, Control, GetStatus, SignalEvent).
//       <e25> Mode
//         <i> Enable / disable SPI mode and Slave Select mode tests.
//         <i> (for Loopback test mode only Master mode with Slave Select not used test is supported!)
//         <q26> SPI_Mode_Master_SS_Unused
//           <i> Enable / disable data exchange in Master mode with Slave Select not used test.
//         <q27> SPI_Mode_Master_SS_Sw_Ctrl
//           <i> Enable / disable data exchange in Master mode with Slave Select Software Controlled test.
//         <q28> SPI_Mode_Master_SS_Hw_Ctrl_Out
//           <i> Enable / disable data exchange in Master mode with Slave Select Hardware Controlled Output test.
//         <q29> SPI_Mode_Master_SS_Hw_Mon_In
//           <i> Enable / disable data exchange in Master mode with Slave Select Hardware Monitored Input test.
//         <q30> SPI_Mode_Slave_SS_Hw_Mon
//           <i> Enable / disable data exchange in Slave mode with Slave Select Hardware Monitored test.
//         <q31> SPI_Mode_Slave_SS_Sw_Ctrl
//           <i> Enable / disable data exchange in Slave mode with Slave Select Software Controlled test.
//       </e>
//       <e32> Clock / Frame Format
//         <i> Enable / disable clock / frame format tests.
//         <i> (all of these tests are supported only in SPI Server test mode!)
//         <q33> SPI_Format_Clock_Pol0_Pha0
//           <i> Enable / disable data exchange with clock format: Polarity 0 / Phase 0 test.
//         <q34> SPI_Format_Clock_Pol0_Pha1
//           <i> Enable / disable data exchange with clock format: Polarity 0 / Phase 1 test.
//         <q35> SPI_Format_Clock_Pol1_Pha0
//           <i> Enable / disable data exchange with clock format: Polarity 1 / Phase 0 test.
//         <q36> SPI_Format_Clock_Pol1_Pha1
//           <i> Enable / disable data exchange with clock format: Polarity 1 / Phase 1 test.
//         <q37> SPI_Format_Frame_TI
//           <i> Enable / disable data exchange with Texas Instruments frame format test.
//         <q38> SPI_Format_Frame_Microwire
//           <i> Enable / disable data exchange with National Semiconductor Microwire frame format test.
//       </e>
//       <e39> Data Bits
//         <i> Enable / disable data bits tests.
//         <i> (for Loopback test mode only: 8, 16, 24, and 32 data bit tests are supported!)
//         <o40.0> SPI_Data_Bits_1
//           <i> Enable / disable data exchange with 1 bit per frame test.
//         <o40.1> SPI_Data_Bits_2
//           <i> Enable / disable data exchange with 2 bits per frame test.
//         <o40.2> SPI_Data_Bits_3
//           <i> Enable / disable data exchange with 3 bits per frame test.
//         <o40.3> SPI_Data_Bits_4
//           <i> Enable / disable data exchange with 4 bits per frame test.
//         <o40.4> SPI_Data_Bits_5
//           <i> Enable / disable data exchange with 5 bits per frame test.
//         <o40.5> SPI_Data_Bits_6
//           <i> Enable / disable data exchange with 6 bits per frame test.
//         <o40.6> SPI_Data_Bits_7
//           <i> Enable / disable data exchange with 7 bits per frame test.
//         <o40.7> SPI_Data_Bits_8
//           <i> Enable / disable data exchange with 8 bits per frame test.
//         <o40.8> SPI_Data_Bits_9
//           <i> Enable / disable data exchange with 9 bits per frame test.
//         <o40.9> SPI_Data_Bits_10
//           <i> Enable / disable data exchange with 10 bits per frame test.
//         <o40.10> SPI_Data_Bits_11
//           <i> Enable / disable data exchange with 11 bits per frame test.
//         <o40.11> SPI_Data_Bits_12
//           <i> Enable / disable data exchange with 12 bits per frame test.
//         <o40.12> SPI_Data_Bits_13
//           <i> Enable / disable data exchange with 13 bits per frame test.
//         <o40.13> SPI_Data_Bits_14
//           <i> Enable / disable data exchange with 14 bits per frame test.
//         <o40.14> SPI_Data_Bits_15
//           <i> Enable / disable data exchange with 15 bits per frame test.
//         <o40.15> SPI_Data_Bits_16
//           <i> Enable / disable data exchange with 16 bits per frame test.
//         <o40.16> SPI_Data_Bits_17
//           <i> Enable / disable data exchange with 17 bits per frame test.
//         <o40.17> SPI_Data_Bits_18
//           <i> Enable / disable data exchange with 18 bits per frame test.
//         <o40.18> SPI_Data_Bits_19
//           <i> Enable / disable data exchange with 19 bits per frame test.
//         <o40.19> SPI_Data_Bits_20
//           <i> Enable / disable data exchange with 20 bits per frame test.
//         <o40.20> SPI_Data_Bits_21
//           <i> Enable / disable data exchange with 21 bits per frame test.
//         <o40.21> SPI_Data_Bits_22
//           <i> Enable / disable data exchange with 22 bits per frame test.
//         <o40.22> SPI_Data_Bits_23
//           <i> Enable / disable data exchange with 23 bits per frame test.
//         <o40.23> SPI_Data_Bits_24
//           <i> Enable / disable data exchange with 24 bits per frame test.
//         <o40.24> SPI_Data_Bits_25
//           <i> Enable / disable data exchange with 25 bits per frame test.
//         <o40.25> SPI_Data_Bits_26
//           <i> Enable / disable data exchange with 26 bits per frame test.
//         <o40.26> SPI_Data_Bits_27
//           <i> Enable / disable data exchange with 27 bits per frame test.
//         <o40.27> SPI_Data_Bits_28
//           <i> Enable / disable data exchange with 28 bits per frame test.
//         <o40.28> SPI_Data_Bits_29
//           <i> Enable / disable data exchange with 29 bits per frame test.
//         <o40.29> SPI_Data_Bits_30
//           <i> Enable / disable data exchange with 30 bits per frame test.
//         <o40.30> SPI_Data_Bits_31
//           <i> Enable / disable data exchange with 31 bits per frame test.
//         <o40.31> SPI_Data_Bits_32
//           <i> Enable / disable data exchange with 32 bits per frame test.
//       </e>
//       <e41> Bit Order
//         <i> Enable / disable bit order tests.
//         <i> (all of these tests are supported only in SPI Server test mode!)
//         <q42> SPI_Bit_Order_MSB_LSB
//           <i> Enable / disable data exchange with bit order from MSB to LSB test.
//         <q43> SPI_Bit_Order_LSB_MSB
//           <i> Enable / disable data exchange with bit order from LSB to MSB test.
//       </e>
//       <e44> Bus Speed
//         <i> Enable / disable bus speeds tests.
//         <q45> SPI_Bus_Speed_Min
//           <i> Enable / disable data exchange at minimum supported bus speed test.
//         <q46> SPI_Bus_Speed_Max
//           <i> Enable / disable data exchange at maximum supported bus speed test.
//       </e>
//       <e47> Other
//         <i> Enable / disable other tests.
//         <q48> SPI_Number_Of_Items
//           <i> Enable / disable data exchange with different number of data items test.
//         <q49> SPI_GetDataCount
//           <i> Enable / disable GetDataCount count changing during data exchange test.
//         <q50> SPI_Abort
//           <i> Enable / disable data exchange Abort test.
//       </e>
//     </e>
//     <e51> Event
//       <i> Enable / disable event signaling tests (function: SignalEvent).
//       <i> (all of these tests are supported only in SPI Server test mode!)
//       <q52> SPI_DataLost
//         <i> Enable / disable ARM_SPI_EVENT_DATA_LOST event generation test.
//       <q53> SPI_ModeFault
//         <i> Enable / disable ARM_SPI_EVENT_MODE_FAULT event generation test.
//     </e>
//   </h>
// </h>

#define DRV_SPI                         0
#define SPI_CFG_TEST_MODE               1
#define SPI_CFG_SRV_SS_MODE             1
#define SPI_CFG_SRV_BUS_SPEED           1000000
#define SPI_CFG_SRV_CMD_TOUT            100
#define SPI_CFG_DEF_SS_MODE             1
#define SPI_CFG_DEF_FORMAT              0
#define SPI_CFG_DEF_DATA_BITS           8
#define SPI_CFG_DEF_BIT_ORDER           0
#define SPI_CFG_DEF_BUS_SPEED           1000000
#define SPI_CFG_DEF_NUM                 512
#define SPI_CFG_MIN_BUS_SPEED           1000000
#define SPI_CFG_MAX_BUS_SPEED           10000000
#define SPI_CFG_NUM1                    1
#define SPI_CFG_NUM2                    31
#define SPI_CFG_NUM3                    65
#define SPI_CFG_NUM4                    1023
#define SPI_CFG_NUM5                    1024
#define SPI_CFG_XFER_TIMEOUT            100
#define SPI_TG_DRIVER_MANAGEMENT_EN     1
#define SPI_TC_GET_VERSION_EN           1
#define SPI_TC_GET_CAPABILITIES_EN      1
#define SPI_TC_INIT_UNINIT_EN           1
#define SPI_TC_POWER_CONTROL_EN         1
#define SPI_TG_DATA_EXCHANGE_EN         1
#define SPI_TG_MODE_EN                  1
#define SPI_TC_MASTER_UNUSED_EN         1
#define SPI_TC_MASTER_SW_EN             1
#define SPI_TC_MASTER_HW_OUT_EN         1
#define SPI_TC_MASTER_HW_IN_EN          1
#define SPI_TC_SLAVE_HW_EN              1
#define SPI_TC_SLAVE_SW_EN              1
#define SPI_TG_FORMAT_EN                1
#define SPI_TC_FORMAT_POL0_PHA0_EN      1
#define SPI_TC_FORMAT_POL0_PHA1_EN      1
#define SPI_TC_FORMAT_POL1_PHA0_EN      1
#define SPI_TC_FORMAT_POL1_PHA1_EN      1
#define SPI_TC_FORMAT_TI_EN             1
#define SPI_TC_FORMAT_MICROWIRE_EN      0
#define SPI_TG_DATA_BIT_EN              1
#define SPI_TC_DATA_BIT_EN_MASK         0x00008080
#define SPI_TG_BIT_ORDER_EN             1
#define SPI_TC_BIT_ORDER_MSB_LSB_EN     1
#define SPI_TC_BIT_ORDER_LSB_MSB_EN     1
#define SPI_TG_BUS_SPEED_EN             1
#define SPI_TC_BUS_SPEED_MIN_EN         1
#define SPI_TC_BUS_SPEED_MAX_EN         1
#define SPI_TG_OTHER_EN                 1
#define SPI_TC_NUMBER_OF_ITEMS_EN       1
#define SPI_TC_GET_DATA_COUNT_EN        1
#define SPI_TC_ABORT_EN                 1
#define SPI_TG_EVENT_EN                 1
#define SPI_TC_DATA_LOST_EN             1
#define SPI_TC_MODE_FAULT_EN            1

#endif /* DV_SPI_CONFIG_H_ */
