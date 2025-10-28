/******************************************************************************
 * @file     vstream_accelerometer_config.h
 * @brief    CMSIS Virtual Streaming interface Driver configuration file for
 *           Accelerometer sensor (ISM330DHCX) on the
 *           STMicroelectronics B-U585I-IOT02A board
 * @version  V1.0.0
 * @date     11. April 2025
 ******************************************************************************/
/*
 * Copyright (c) 2025 Arm Limited. All rights reserved.
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
 */

#ifndef VSTREAM_ACCELEROMETER_CONFIG_H_
#define VSTREAM_ACCELEROMETER_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//------ With VS Code: Open Preview for Configuration Wizard -------------------

// <o> Sensor sampling rate
//   <i> Rate at which the sensor will take measurements (sample).
//   <26=>26 Hz
//   <52=>52 Hz
//   <104=>104 Hz
//   <208=>208 Hz
//   <416=>416 Hz
//   <833=>833 Hz
//   <1666=>1666 Hz
//   <3332=>3332 Hz
//   <6667=>6667 Hz
#define SENSOR_SAMPLING_RATE            52

// <o> Sensor data polling interval
//   <i> Interval for polling data from sensor FIFO (in OS ticks).
//   <i> Should be short enough to allow sensor FIFO not to overfill, usually shorter than sensor sampling interval.
#define SENSOR_POLLING_INTERVAL         19

// <o> Initial samples to discard
//   <i> Number of initial samples to be discarded.
//   <i> Due to accelerometer turn-on/off time it is necessary to discard a number of initial samples as they are invalid.
#define SENSOR_STARTUP_DISCARD_SAMPLES  2

#endif
