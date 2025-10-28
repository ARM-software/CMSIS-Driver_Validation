# Board: STMicroelectronics [B-U585I-IOT02A](https://www.st.com/en/evaluation-tools/b-u585i-iot02a.html)

## Board Layer supporting USB Device, Audio In, WiFi, Accelerometer Sensor

Device: **STM32U585AII6QU**

System Core Clock: **160 MHz**

This setup is configured using **STM32CubeMX**, an interactive tool provided by STMicroelectronics for device configuration.
Refer to ["Configure STM32 Devices with CubeMX"](https://open-cmsis-pack.github.io/cmsis-toolbox/CubeMX/) for additional information.

### System Configuration

| System resource       | Setting
|:----------------------|:--------------------------------------
| Heap                  | 64 kB (configured in the STM32CubeMX)
| Stack (MSP)           |  1 kB (configured in the STM32CubeMX)

### STDIO mapping

**STDIO** is routed to Virtual COM port on the ST-LINK (using **USART1** peripheral)

### CMSIS-Driver mapping

| CMSIS-Driver                | Peripheral            | Board connector/component                     | Connection
|:----------------------------|:----------------------|:----------------------------------------------|:------------------------------
| Driver_GPIO0                | GPIO                  | Arduino digital I/O pins D2..D10, D14..D19    | ARDUINO_UNO_D2..D10, D14..D19
| Driver_I2C1                 | I2C1                  | Arduino I2C pins D20..D21                     | ARDUINO_UNO_I2C
| Driver_SPI1                 | SPI1 (DMA)            | Arduino SPI pins D10..D13                     | ARDUINO_UNO_SPI
| Driver_USART3               | USART3 (DMA)          | Arduino UART pins D0..D1                      | ARDUINO_UNO_UART
| Driver_USART2               | USART2                | ST-Mod pins 2S1, 3S1 (CN3)                    | CMSIS_USART
| Driver_USART1               | USART1                | ST-LINK connector (CN8)                       | STDIN, STDOUT, STDERR
| Driver_USBD0                | USB_OTG_FS            | User USB connector (CN1)                      | CMSIS_USB_Device
| Driver_vStreamAccelerometer | I2C2                  | iNEMO inertial module ISM330DHCX              | CMSIS_VSTREAM_ACCELEROMETER
| Driver_vStreamAudioIn       | MDF (DMA)             | On-board digital microphone (MIC1)            | CMSIS_VSTREAM_AUDIO_IN
| Driver_WiFi0                | SPI2 (DMA), GPIO      | MXCHIP EMW3080                                | CMSIS_WiFi
| CMSIS-Driver VIO            | GPIO                  | LEDs (LD6, LD7) and USER button (B3)          | CMSIS_VIO

Reference to [Arduino UNO connector description](https://open-cmsis-pack.github.io/cmsis-toolbox/ReferenceApplications/#arduino-shield).

### CMSIS-Driver Virtual I/O mapping

| CMSIS-Driver VIO      | Board component
|:----------------------|:--------------------------------------
| vioBUTTON0            | USER button (B3)
| vioLED0               | LED red     (LD6)
| vioLED1               | LED green   (LD7)
