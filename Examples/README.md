# CMSIS-Driver Validation Examples

This folder provides a preconfigured **CMSIS-Driver Validation examples** originating from a **template**.  

The following examples for driver validation are available:

| Validation for Driver  | Board             | Additional required hardware                      |
|------------------------|-------------------|---------------------------------------------------|
| Ethernet               | ST NUCLEO-H563ZI  | Ethernet loopback plug                            |
| GPIO                   | ST B-U585I-IOT02A | ST STM32F429I-DISC1, 1kOhm resistor               |
| SPI                    | ST B-U585I-IOT02A | ST STM32F429I-DISC1, 10kOhm resistor, some wires  |
| USART                  | ST B-U585I-IOT02A | ST STM32F429I-DISC1, some wires                   |
| WiFi on-board EMW3080  | ST B-U585I-IOT02A | ST B-U585I-IOT02A, PC with MS Windows             |
| WiFi ESP8266           | ST B-U585I-IOT02A | SparkFun ESP8266 Shield, PC with MS Windows       |
| WiFi ISM43662          | ST B-U585I-IOT02A | Inventek ISMART43362-E Shield, PC with MS Windows |
| WiFi WizFi360          | ST B-U585I-IOT02A | WIZnet WizFi360-EVB Shield, PC with MS Windows    |

> **Note:** SPI and USART validation examples need respective **Server** which is available
  for STMicroelectronics **STM32F429I-DISC1** board.  
> **Note:** The **Server** can be used on any hardware with available **Board Layer** with configured **CMSIS-Driver**
  that is **fully CMSIS compliant**.

## Documentation

Detailed example documentation for respective driver validation:

- [Ethernet](./Ethernet/NUCLEO-H563ZI/README.md)  
- [GPIO](./GPIO/B-U585I-IOT02A/README.md)  
- [SPI](./SPI/B-U585I-IOT02A/README.md)  
- [USART](./USART/B-U585I-IOT02A/README.md)  
- [WiFi on-board EMW3080](./WiFi/EMW3080/B-U585I-IOT02A/README.md)  
- [WiFi ESP8266](./WiFi/ESP8266/B-U585I-IOT02A/README.md)  
- [WiFi ISM43362](./WiFi/ISMART43362-E/B-U585I-IOT02A/README.md)  
- [WiFi WizFi360](./WiFi/WizFi360-EVB/B-U585I-IOT02A/README.md)  
