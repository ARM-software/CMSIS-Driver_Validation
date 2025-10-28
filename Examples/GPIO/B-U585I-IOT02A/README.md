# GPIO CMSIS-Driver Validation Example

This folder provides a **ready-to-use CMSIS-Driver Validation application** for testing and verifying the **GPIO driver** on the STMicroelectronics [**B-U585I-IOT02A**](https://www.st.com/en/evaluation-tools/B-U585I-IOT02A.html) development board.

---

## Overview

This application validates GPIO driver compliance to the [CMSIS-Driver GPIO Interface Driver Specification](https://arm-software.github.io/CMSIS_6/latest/Driver/group__gpio__interface__gr.html).

It serves as a quick-start configuration for developers who need to ensure GPIO driver compliance and performance using the CMSIS standard.

This example was created using the CMSIS-Driver Validation [**Template**](https://github.com/ARM-software/CMSIS-Driver_Validation/blob/main/Template/README.md) application.

---

## Configuration

Tests and parameters are configured in the [DV_GPIO_Config.h](RTE/CMSIS_Driver_Validation/DV_GPIO_Config.h) configuration file.

You can modify this file to customize test parameters, timing, and execution behavior.

---

## Requirements

### **Hardware**
- STMicroelectronics [**B-U585I-IOT02A**](https://github.com/Open-CMSIS-Pack/ST_B-U585I-IOT02A_BSP/blob/main/Documents/README.md) board
- **approx. 1 kOhm resistor** ([documentation link](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__gpio.html))
- **USB-A to micro USB** cable

### **Software**
- [Visual Studio Code](https://code.visualstudio.com/)
- [Arm Keil Studio Pack (MDK v6) Extension](https://marketplace.visualstudio.com/items?itemName=Arm.keil-studio-pack)
- Terminal application (e.g., PuTTY, Tera Term)

---

## Usage Instructions

To run the driver validation:

1. Open the [**CMSIS_DV solution**](CMSIS_DV.csolution.yml) in **Visual Studio Code**.
2. Connect the **1 kOhm resistor** between **ARDUINO ARD.D7** (PD15, GPIO PIN 63) and **ARDUINO ARD.D8** (PC1, GPIO PIN 33) pins ([board schematic (Arduino on sheet 8)](https://www.st.com/resource/en/schematic_pack/mb1551-u585i-c02_schematic.pdf)).
3. Connect the board’s **ST-Link USB interface** to your PC using a USB-A to micro USB cable.
4. Open a terminal and connect to the **STLink Virtual COM Port (COMx)** using `115200 8-N-1` settings.
5. Build and run the validation application.
6. Observe the test output in the terminal window.

---

## Example Test Results

A successful validation output should resemble the following:

```
CMSIS-Driver_Validation v3.1.0 CMSIS-Driver GPIO Test Report   Oct  9 2025   09:15:16 

TEST 01: GPIO_Setup                       PASSED
TEST 02: GPIO_SetDirection                PASSED
TEST 03: GPIO_SetOutputMode               PASSED
TEST 04: GPIO_SetPullResistor             PASSED
TEST 05: GPIO_SetEventTrigger             PASSED
TEST 06: GPIO_SetOutput                   PASSED
TEST 07: GPIO_GetInput                    PASSED

Test Summary: 7 Tests, 7 Passed, 0 Failed.
Test Result: PASSED
```
