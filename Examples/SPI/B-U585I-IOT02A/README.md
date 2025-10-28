# SPI CMSIS-Driver Validation Example

This folder provides a **ready-to-use CMSIS-Driver Validation application** for testing and verifying the **SPI driver** on the STMicroelectronics [**B-U585I-IOT02A**](https://www.st.com/en/evaluation-tools/B-U585I-IOT02A.html) development board.

---

## Overview

This application validates SPI driver compliance to the [CMSIS-Driver SPI Interface Driver Specification](https://arm-software.github.io/CMSIS_6/latest/Driver/group__spi__interface__gr.html).

It serves as a quick-start configuration for developers who need to ensure SPI driver compliance and performance using the CMSIS standard.

This example was created using the CMSIS-Driver Validation [**Template**](https://github.com/ARM-software/CMSIS-Driver_Validation/blob/main/Template/README.md) application.

---

## Configuration

Tests and parameters are configured in the [DV_SPI_Config.h](RTE/CMSIS_Driver_Validation/DV_SPI_Config.h) configuration file.

You can modify this file to customize test parameters, timing, and execution behavior.

---

## Requirements

### **Hardware**
- STMicroelectronics [**B-U585I-IOT02A**](https://github.com/Open-CMSIS-Pack/ST_B-U585I-IOT02A_BSP/blob/main/Documents/README.md) board
- STMicroelectronics [**STM32F429I-DISC1**](https://www.st.com/en/evaluation-tools/32f429idiscovery.html) board (for the SPI Server)
- **approx. 10 kOhm resistor and some wires** ([documentation link](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__spi.html))
- **USB-A to micro USB** cable
- **USB-A to mini USB** cable (for the SPI Server)

### **Software**
- [Visual Studio Code](https://code.visualstudio.com/)
- [Arm Keil Studio Pack (MDK v6) Extension](https://marketplace.visualstudio.com/items?itemName=Arm.keil-studio-pack)
- Terminal application (e.g., PuTTY, Tera Term)

---

## Usage Instructions

This validation requires **SPI Server** running on a dedicated **STM32F429I-DISC1** board.  
Build and download the **SPI Server** application, located in **Tools/SPI_Server/STM32F429I-DISC1** of the **CMSIS-Driver_Validation pack installation**, to the STM32F429I-DISC1 board.

To run the driver validation:

1. Open the [**CMSIS_DV solution**](CMSIS_DV.csolution.yml) in **Visual Studio Code**.
2. Connect the **B-U585I-IOT02A** board pins to **SPI Server** pins as described in the following table:
   | B-U585I-IOT02A                                                  | STM32F429I-DISC1 (SPI Server)   |
   |-----------------------------------------------------------------|---------------------------------|
   | ARDUINO ARD.D13 - (PE13, SPI1_SCK)                              | PA5  (SPI1_SCK)                 |
   | ARDUINO ARD.D11 - (PE15, SPI1_MOSI)                             | PA7  (SPI1_MOSI)                |
   | ARDUINO ARD.D12 - (PE14, SPI1_MISO)                             | PB4  (SPI1_MISO)                |
   | ARDUINO ARD.D10 - (PE12, SPI1_NSS) - to Vcc (3.3V) over 10 kOhm | PA15 (SPI1_NSS)                 |
   | GND                                                             | GND                             |

   > **Note:** For B-U585I-IOT02A schematic see: [board schematic (Arduino on sheet 8)](https://www.st.com/resource/en/schematic_pack/mb1551-u585i-c02_schematic.pdf).  
   > **Note:** For connection between DUT and SPI Server see: [**Test Mode: SPI Server** connections documentation](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__spi.html).  
   > **Note:** Connect also **NSS** line to **Vcc (3.3V)** over approx. **10 kOhm resistor**.  
   > **Note:** Wires should be as short as possible and separate from other wires as possible otherwise errors in the communication can happen.
3. Connect the board’s **ST-Link USB interface** to your PC using a USB-A to micro USB cable.
4. Open a terminal and connect to the **STLink Virtual COM Port (COMx)** using `115200 8-N-1` settings.
5. Power-up and start the **SPI Server** on the **STM32F429I-DISC1** board.
6. Build and run the validation application.
7. Observe the test output in the terminal window.

---

## Example Test Results

A successful validation output should resemble the following:

```
CMSIS-Driver_Validation v3.1.0 CMSIS-Driver SPI Test Report   Oct 10 2025   08:55:16 

Test Mode:          SPI Server
Default settings:
 - Slave Select:    Hardware controlled
 - Format:          Clock Polarity 0, Clock Phase 0
 - Data bits:       8
 - Bit order:       MSB to LSB
 - Bus speed:       1000000 bps
 - Number of Items: 512

TEST 01: SPI_GetVersion                   
  DV_SPI.c (1346): [INFO] Driver API version 2.3, Driver version 3.1
                                          PASSED
TEST 02: SPI_GetCapabilities              PASSED
TEST 03: SPI_Initialize_Uninitialize      PASSED
TEST 04: SPI_PowerControl                 
  DV_SPI.c (1641): [WARNING] PowerControl (ARM_POWER_LOW) is not supported
                                          PASSED
TEST 05: SPI_Mode_Master_SS_Unused        PASSED
TEST 06: SPI_Mode_Master_SS_Sw_Ctrl       PASSED
TEST 07: SPI_Mode_Master_SS_Hw_Ctrl_Out   PASSED
TEST 08: SPI_Mode_Master_SS_Hw_Mon_In     PASSED
TEST 09: SPI_Mode_Slave_SS_Hw_Mon         NOT EXECUTED
TEST 10: SPI_Mode_Slave_SS_Sw_Ctrl        PASSED
TEST 11: SPI_Format_Clock_Pol0_Pha0       PASSED
TEST 12: SPI_Format_Clock_Pol0_Pha1       PASSED
TEST 13: SPI_Format_Clock_Pol1_Pha0       PASSED
TEST 14: SPI_Format_Clock_Pol1_Pha1       PASSED
TEST 15: SPI_Format_Frame_TI              PASSED
TEST 16: SPI_Format_Clock_Microwire       NOT EXECUTED
TEST 17: SPI_Data_Bits_1                  NOT EXECUTED
TEST 18: SPI_Data_Bits_2                  NOT EXECUTED
TEST 19: SPI_Data_Bits_3                  NOT EXECUTED
TEST 20: SPI_Data_Bits_4                  NOT EXECUTED
TEST 21: SPI_Data_Bits_5                  NOT EXECUTED
TEST 22: SPI_Data_Bits_6                  NOT EXECUTED
TEST 23: SPI_Data_Bits_7                  NOT EXECUTED
TEST 24: SPI_Data_Bits_8                  PASSED
TEST 25: SPI_Data_Bits_9                  NOT EXECUTED
TEST 26: SPI_Data_Bits_10                 NOT EXECUTED
TEST 27: SPI_Data_Bits_11                 NOT EXECUTED
TEST 28: SPI_Data_Bits_12                 NOT EXECUTED
TEST 29: SPI_Data_Bits_13                 NOT EXECUTED
TEST 30: SPI_Data_Bits_14                 NOT EXECUTED
TEST 31: SPI_Data_Bits_15                 NOT EXECUTED
TEST 32: SPI_Data_Bits_16                 PASSED
TEST 33: SPI_Data_Bits_17                 NOT EXECUTED
TEST 34: SPI_Data_Bits_18                 NOT EXECUTED
TEST 35: SPI_Data_Bits_19                 NOT EXECUTED
TEST 36: SPI_Data_Bits_20                 NOT EXECUTED
TEST 37: SPI_Data_Bits_21                 NOT EXECUTED
TEST 38: SPI_Data_Bits_22                 NOT EXECUTED
TEST 39: SPI_Data_Bits_23                 NOT EXECUTED
TEST 40: SPI_Data_Bits_24                 NOT EXECUTED
TEST 41: SPI_Data_Bits_25                 NOT EXECUTED
TEST 42: SPI_Data_Bits_26                 NOT EXECUTED
TEST 43: SPI_Data_Bits_27                 NOT EXECUTED
TEST 44: SPI_Data_Bits_28                 NOT EXECUTED
TEST 45: SPI_Data_Bits_29                 NOT EXECUTED
TEST 46: SPI_Data_Bits_30                 NOT EXECUTED
TEST 47: SPI_Data_Bits_31                 NOT EXECUTED
TEST 48: SPI_Data_Bits_32                 NOT EXECUTED
TEST 49: SPI_Bit_Order_MSB_LSB            PASSED
TEST 50: SPI_Bit_Order_LSB_MSB            PASSED
TEST 51: SPI_Bus_Speed_Min                PASSED
TEST 52: SPI_Bus_Speed_Max                PASSED
TEST 53: SPI_Number_Of_Items              PASSED
TEST 54: SPI_GetDataCount                 PASSED
TEST 55: SPI_Abort                        PASSED

Test Summary: 55 Tests, 23 Passed, 0 Failed.
Test Result: PASSED
```

> **Note:** Some warnings may indicate optional features not supported by the hardware.
