# Ethernet CMSIS-Driver Validation Example

This folder provides a **ready-to-use CMSIS-Driver Validation application** for testing and verifying the **Ethernet driver** on the STMicroelectronics [**NUCLEO-H563ZI**](https://www.st.com/en/evaluation-tools/nucleo-h563zi.html) development board.

---

## Overview

This application validates Ethernet driver compliance to the [CMSIS-Driver Ethernet Interface Driver Specification](https://arm-software.github.io/CMSIS_6/latest/Driver/group__eth__interface__gr.html).

It serves as a quick-start configuration for developers who need to ensure Ethernet driver compliance and performance using the CMSIS standard.

This example was created using the CMSIS-Driver Validation [**Template**](https://github.com/ARM-software/CMSIS-Driver_Validation/blob/main/Template/README.md) application.

---

## Configuration

Tests and parameters are configured in the [DV_ETH_Config.h](RTE/CMSIS_Driver_Validation/DV_ETH_Config.h) configuration file.

You can modify this file to customize test parameters, timing, and execution behavior.

---

## Requirements

### **Hardware**
- STMicroelectronics [**NUCLEO-H563ZI**](https://github.com/Open-CMSIS-Pack/ST_NUCLEO-H563ZI_BSP/blob/main/Documents/README.md) board
- **Ethernet loopback plug** ([documentation link](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__eth.html))
- **USB-A to USB-C** cable

### **Software**
- [Visual Studio Code](https://code.visualstudio.com/)
- [Arm Keil Studio Pack (MDK v6) Extension](https://marketplace.visualstudio.com/items?itemName=Arm.keil-studio-pack)
- Terminal application (e.g., PuTTY, Tera Term)

---

## Usage Instructions

To run the driver validation:

1. Open the [**CMSIS_DV solution**](CMSIS_DV.csolution.yml) in **Visual Studio Code**.
2. Connect the [**Ethernet loopback plug**](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__eth.html) to the board’s Ethernet port.
3. Connect the board’s **ST-Link USB interface** to your PC using a USB-A to USB-C cable.
4. Open a terminal and connect to the **STLink Virtual COM Port (COMx)** using `115200 8-N-1` settings.
5. Build and run the validation application.
6. Observe the test output in the terminal window.

---

## Example Test Results

A successful validation output should resemble the following:

```
CMSIS-Driver_Validation v3.1.0 CMSIS-Driver ETH Test Report   Oct  9 2025   07:30:34 

TEST 01: ETH_MAC_GetVersion               
  DV_ETH.c (267): [INFO] API version 2.2, Driver version 3.1
                                          PASSED
TEST 02: ETH_MAC_GetCapabilities          PASSED
TEST 03: ETH_MAC_Initialization           PASSED
TEST 04: ETH_MAC_PowerControl             
  DV_ETH.c (366): [WARNING] Low power is not supported
                                          PASSED
TEST 05: ETH_MAC_MacAddress               PASSED
TEST 06: ETH_MAC_SetBusSpeed              
  DV_ETH.c (445): [WARNING] Link speed 1G is not supported
                                          PASSED
TEST 07: ETH_MAC_Config_Mode              PASSED
TEST 08: ETH_MAC_Config_CommonParams      PASSED
TEST 09: ETH_MAC_Control_Filtering        PASSED
TEST 10: ETH_MAC_SetAddressFilter         PASSED
TEST 11: ETH_MAC_VLAN_Filter              
  DV_ETH.c (910): [WARNING] Received non VLAN tagged frame
                                          PASSED
TEST 12: ETH_MAC_SignalEvent              PASSED
TEST 13: ETH_MAC_PTP_ControlTimer         
  DV_ETH.c (1412): [WARNING] Precision Time Protocol is not supported
                                          NOT EXECUTED
TEST 14: ETH_MAC_CheckInvalidInit         PASSED
TEST 15: ETH_PHY_GetVersion               
  DV_ETH.c (1018): [INFO] API version 2.2, Driver version 1.3
                                          PASSED
TEST 16: ETH_PHY_Initialization           PASSED
TEST 17: ETH_PHY_PowerControl             
  DV_ETH.c (1114): [WARNING] Low power is not supported
  DV_ETH.c (1131): [WARNING] MAC is locked when PHY power is off
                                          PASSED
TEST 18: ETH_PHY_Config                   PASSED
TEST 19: ETH_PHY_CheckInvalidInit         PASSED
TEST 20: ETH_Loopback_Transfer            PASSED
TEST 21: ETH_Loopback_PTP                 
  DV_ETH.c (1574): [WARNING] Precision Time Protocol is not supported
                                          NOT EXECUTED
TEST 22: ETH_Loopback_External            PASSED

Test Summary: 22 Tests, 20 Passed, 0 Failed.
Test Result: PASSED
```

> **Note:** Some warnings may indicate optional features not supported by the hardware (e.g., low-power mode or PTP).
