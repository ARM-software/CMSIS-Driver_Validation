# Espressif ESP8266 WiFi CMSIS-Driver Validation Example

This folder provides a **ready-to-use CMSIS-Driver Validation application** for testing and verifying the **Espressif ESP8266 WiFi driver** with [**SparkFun ESP8266 WiFi shield**](https://www.sparkfun.com/sparkfun-wifi-shield-esp8266.html) connected to the STMicroelectronics [**B-U585I-IOT02A**](https://www.st.com/en/evaluation-tools/B-U585I-IOT02A.html) development board.

---

## Overview

This application validates WiFi driver compliance to the [CMSIS-Driver WiFi Interface Driver Specification](https://arm-software.github.io/CMSIS_6/latest/Driver/group__wifi__interface__gr.html).

It serves as a quick-start configuration for developers who need to ensure WiFi driver compliance and performance using the CMSIS standard.

This example was created using the CMSIS-Driver Validation [**Template**](https://github.com/ARM-software/CMSIS-Driver_Validation/blob/main/Template/README.md) application.

---

## Configuration

Tests and parameters are configured in the [DV_WiFi_Config.h](RTE/CMSIS_Driver_Validation/DV_WiFi_Config.h) configuration file.

You **MUST** modify the following settings:
- **Configuration/Station: SSID (WIFI_STA_SSID)** set this to SSID of your local WiFi network used for validation
- **Configuration/Station: Password (WIFI_STA_PASS)** set this to password of your local WiFi network used for validation
- **Configuration/Socket: SockServer IP (WIFI_SOCKET_SERVER_IP)** set this to **SockServer IP**

> **Note:** Check under [usage instructions](#usage-instructions) on how to get the SockServer IP.

---

## Requirements

### **Hardware**
- STMicroelectronics [**B-U585I-IOT02A**](https://github.com/Open-CMSIS-Pack/ST_B-U585I-IOT02A_BSP/blob/main/Documents/README.md) board
- [SparkFun ESP8266 WiFi Shield](https://www.sparkfun.com/sparkfun-wifi-shield-esp8266.html) with latest firmware
- Personal Computer running Microsoft Windows
- **USB-A to micro USB** cable

### **Software**
- [Visual Studio Code](https://code.visualstudio.com/)
- [Arm Keil Studio Pack (MDK v6) Extension](https://marketplace.visualstudio.com/items?itemName=Arm.keil-studio-pack)
- **SockServer** application distributed with **CMSIS-Driver_Validation pack**
- Terminal application (e.g., PuTTY, Tera Term)

---

## Usage Instructions

This validation requires **SockServer** running on a **Personal Computer**.  
Start the **SockServer.exe** PC application, located in **Tools/SockServer/PC** of the **CMSIS-Driver_Validation pack installation** on your local PC
and note the **Address** reported by the application.  
Use that **Address** and enter it in the `DV_WiFi_Config.h` configuration file under **Configuration/Socket: SockServer IP (WIFI_SOCKET_SERVER_IP)**.

To run the driver validation:

1. Open the [**CMSIS_DV solution**](CMSIS_DV.csolution.yml) in **Visual Studio Code**.
2. Connect **SparkFun ESP8266 WiFi Shield** to the **Arduino** connector on the **B-U585I-IOT02A** board.
3. Connect the board’s **ST-Link USB interface** to your PC using a USB-A to micro USB cable.
4. Open a terminal and connect to the **STLink Virtual COM Port (COMx)** using `115200 8-N-1` settings.
5. Build and run the validation application.
6. Observe the test output in the terminal window.

---

## Example Test Results

A successful validation output should resemble the following:

```
CMSIS-Driver_Validation v3.1.0 CMSIS-Driver WiFi Test Report   Oct 16 2025   14:31:18 

TEST 01: WIFI_GetVersion                  
  DV_WIFI.c (305): [INFO] Driver API version 1.1, Driver version 1.8
                                          PASSED
TEST 02: WIFI_GetCapabilities             PASSED
TEST 03: WIFI_Initialize_Uninitialize     PASSED
TEST 04: WIFI_PowerControl                
  DV_WIFI.c (424): [WARNING] PowerControl (ARM_POWER_LOW) is not supported
                                          PASSED
TEST 05: WIFI_GetModuleInfo               
  DV_WIFI.c (473): [INFO] Module Info is AT version:1.6.2.0(Apr 13 2018 11:10:59)
SDK version:2.2.1(6ab97e9)
compile time:Jun  7 2018 19:34:26

                                          PASSED
TEST 06: WIFI_SetOption_GetOption         PASSED
TEST 07: WIFI_Scan                        NOT EXECUTED
TEST 08: WIFI_Activate_Deactivate         PASSED
TEST 09: WIFI_IsConnected                 PASSED
TEST 10: WIFI_GetNetInfo                  PASSED
TEST 11: WIFI_SocketCreate                PASSED
TEST 12: WIFI_SocketBind                  PASSED
TEST 13: WIFI_SocketListen                PASSED
TEST 14: WIFI_SocketAccept                PASSED
TEST 15: WIFI_SocketAccept_nbio           NOT EXECUTED
TEST 16: WIFI_SocketConnect               
  DV_WIFI.c (4597): [WARNING] Non BSD-strict, connect to non-existent port (result ARM_SOCKET_ETIMEDOUT, expected ARM_SOCKET_ECONNREFUSED)
                                          PASSED
TEST 17: WIFI_SocketConnect_nbio          NOT EXECUTED
TEST 18: WIFI_SocketRecv                  PASSED
TEST 19: WIFI_SocketRecv_nbio             NOT EXECUTED
TEST 20: WIFI_SocketRecvFrom              PASSED
TEST 21: WIFI_SocketRecvFrom_nbio         NOT EXECUTED
TEST 22: WIFI_SocketSend                  PASSED
TEST 23: WIFI_SocketSendTo                PASSED
TEST 24: WIFI_SocketGetSockName           PASSED
TEST 25: WIFI_SocketGetPeerName           PASSED
TEST 26: WIFI_SocketGetOpt                PASSED
TEST 27: WIFI_SocketSetOpt                PASSED
TEST 28: WIFI_SocketClose                 PASSED
TEST 29: WIFI_SocketGetHostByName         PASSED
TEST 30: WIFI_Ping                        PASSED
TEST 31: WIFI_Transfer_Fixed              PASSED
TEST 32: WIFI_Transfer_Incremental        PASSED
TEST 33: WIFI_Send_Fragmented             PASSED
TEST 34: WIFI_Recv_Fragmented             PASSED
TEST 35: WIFI_Test_Speed                  PASSED
TEST 36: WIFI_Concurrent_Socket           PASSED
TEST 37: WIFI_Downstream_Rate             
  DV_WIFI.c (9267): [INFO] Speed 91 KB/s
                                          PASSED
TEST 38: WIFI_Upstream_Rate               
  DV_WIFI.c (9335): [INFO] Speed 14 KB/s
                                          PASSED

Test Summary: 38 Tests, 33 Passed, 0 Failed.
Test Result: PASSED
```

> **Note:** Some warnings may indicate optional features not supported by the hardware.
