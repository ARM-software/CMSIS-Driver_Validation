# CMSIS-Driver Validation Template

This folder provides a **CMSIS-Driver Validation template**, designed to validate CMSIS-Drivers in a **hardware-agnostic** way.  
It leverages the [CMSIS-Toolbox Reference Applications](https://open-cmsis-pack.github.io/cmsis-toolbox/ReferenceApplications/) and requires only a board support layer with drivers for the target hardware.

> **Note:** WiFi drivers for WiFi Shields require also [**WiFi shield layer**](https://github.com/ARM-software/CMSIS-Driver/tree/main/Shield/WiFi) (see example for WiFi driver validation).  
> **Note:** Some board support layers do not have NSS pin for SPI configured for SPI mode so it might be necessary to configure NSS pin to respective SPI in the board layer (see example for SPI driver validation).

---

## Driver Selection and Configuration

To select a driver for testing, use the **Build Type** option in the **CMSIS_DV solution**.  
> **Note:** Only one driver can be selected at a time.

### Supported Drivers

- CAN
- **Ethernet**
- **GPIO**
- I2C
- MCI
- **SPI**
- **USART**
- USB_Device
- USB_Host
- **WiFi**

> **Note:** Drivers shown in **bold** in the above list provide extensive driver validation.

### Configuration Files

Each driver includes a dedicated configuration file:

| Driver      | Config File            |
|-------------|------------------------|
| CAN         | `DV_CAN_Config.h`      |
| Ethernet    | `DV_ETH_Config.h`      |
| GPIO        | `DV_GPIO_Config.h`     |
| I2C         | `DV_I2C_Config.h`      |
| MCI         | `DV_MCI_Config.h`      |
| SPI         | `DV_SPI_Config.h`      |
| USART       | `DV_USART_Config.h`    |
| USB_Device  | `DV_USBD_Config.h`     |
| USB_Host    | `DV_USBH_Config.h`     |
| WiFi        | `DV_WiFi_Config.h`     |

---

## Documentation

Detailed validation documentation for each driver:

- [CAN](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__can.html)  
- [Ethernet](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__eth.html)  
- [GPIO](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__gpio.html)  
- [I2C](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__i2c.html)  
- [MCI](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__mci.html)  
- [SPI](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__spi.html)  
- [USART](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__usart.html)  
- [USB_Device](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__usbd.html)  
- [USB_Host](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__usbh.html)  
- [WiFi](https://arm-software.github.io/CMSIS-Driver_Validation/latest/group__dv__wifi.html)  

---

## Usage Instructions

Follow these steps to run the validation tests:

1. In the **CMSIS** extension, click **Create a New Solution**.
2. Select the **Target Board**.
3. Under **Templates, Reference Applications, and Examples**, select **Driver Validation (CMSIS-Driver Validation application)**.
4. Choose **Solution Base Folder**, click **Create**.
5. Select the **Board Layer** that provides **required CMSIS Driver**, click **OK**.
6. In the **Manage Solution**, under **Build Type** select desired Driver for validation, click **Save**.
7. Configure the Driver Tests in appropriate **DV_..._Config.h** file.
8. Setup the required hardware
9. Build and Run the application

For more details, see [**Setup**](https://arm-software.github.io/CMSIS-Driver_Validation/latest/setup.html) documentation section.

> **Note:** validation of **SPI** and **USART** drivers requires **SPI_Server** or **USART_Server**.  
> **Note:** validation of **GPIO** driver requires **resistor between test pins**.  
> **Note:** validation of **WiFi** driver requires **SockServer running on a Windows PC** connected to the same network as WiFi will connect to.

