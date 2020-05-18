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
 * $Revision:   V2.0.0
 *
 * Project:     CMSIS-Driver Validation
 * Title:       Driver Validation main configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_CONFIG_H_
#define DV_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h> Driver Validation
// <i> Driver Validation configuration
//   <o> Report Format <0=> Plain Text <1=> XML
//   <i> Select report format to plain text or XML
#ifndef PRINT_XML_REPORT
#define PRINT_XML_REPORT                0
#endif
// </h>

#endif /* DV_CONFIG_H_ */
