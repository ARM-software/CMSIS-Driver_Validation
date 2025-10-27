# SPI Server Application for STM32F429I-DISC1

This application serves as an **SPI Server** running on the STMicroelectronics **STM32F429I-DISC1 (32F429IDISCOVERY)** development board.  
It provides a physical test environment for validating SPI driver compliance to the **CMSIS SPI driver specification v2.0.0 and above**.

---

## Overview

The SPI Server operates in **Slave mode**, awaiting commands over the SPI interface from an external SPI Client (Driver Validation).  
Upon receiving a command, it executes the corresponding operation and returns to the waiting state.  
Commands may either execute to completion or terminate upon timeout.

All commands (except `XFER`) utilize a fixed SPI configuration and timeout as defined in the **`SPI_Server_Config.h`** file.  
The `XFER` command temporarily applies custom SPI settings defined by the most recent `SET COM` command.

---

## Default SPI Configuration

| Parameter   | Value
|-------------|------
| Mode        | Slave (hardware-monitored Slave Select)
| Clock/Frame | Polarity 0, Phase 0
| Data Bits   | 8
| Bit Order   | MSB first

---

## Hardware Connections

Peripheral used: **SPI1**

| Function | Pin  | Description
|----------|------|------------
| SCLK     | PA5  | SPI clock
| MOSI     | PA7  | Master Out, Slave In
| MISO     | PB4  | Master In, Slave Out
| SS       | PA15 | Requires external pull-up to Vcc (3.3V)
| GND      | GND  | Any board ground

---

## Command Protocol

Each command is **32 bytes long**, padded with zeros if shorter. Optional parameters are enclosed in `[]`.

### Supported Commands

| Command (32 bytes zero padded)                            | Data Phase Direction | Description
|-----------------------------------------------------------|----------------------|------------
| `GET VER`                                                 | OUT (16 bytes)       | Returns firmware version in `major.minor.patch` format.
| `GET CAP`                                                 | OUT (32 bytes)       | Returns supported mode/format/bit masks and speed limits.
| `SET BUF RX/TX,len[,pattern]`                             | IN (`1en` bytes)     | Sets RX/TX buffer; optionally pre-fills with pattern.
| `GET BUF RX/TX,len`                                       | OUT (`len` bytes)    | Reads `len` bytes from RX/TX buffer.
| `SET COM mode,format,bit_num,bit_order,ss_mode,bus_speed` | —                    | Sets custom SPI communication parameters for next `XFER`.
| `XFER num[,delay_c][,delay_t][,timeout]`                  | IN/OUT (`num` items) | Performs full-duplex SPI data transfer.
| `GET CNT`                                                 | OUT (16 bytes)       | Returns count in decimal notation.

## Command Parameters

| Parameter   | Description
|-------------|------------
| `RX/TX`     | RX = SPI Server receive buffer, TX = SPI Server transmit buffer
| `len`       | Data length for data phase
| `pattern`   | Hex value used to pre-fill buffer
| `mode`      | 0 = Master, 1 = Slave
| `format`    | 0 = Clock Polarity 0, Clock Phase 0
|             | 1 = Clock Polarity 0, Clock Phase 1
|             | 2 = Clock Polarity 1, Clock Phase 0
|             | 3 = Clock Polarity 1, Clock Phase 1
|             | 4 = Texas Instruments Frame Format
|             | 5 = National Semiconductor Microwire Frame Format
| `bit_num`   | Bits per frame (1–32)
| `bit_order` | 0 = MSB first, 1 = LSB first
| `ss_mode`   | 0 = unused, 1 = Master-driven/Slave-monitored
| `bus_speed` | SPI clock rate (bps)
| `num`       | Number of items (according CMSIS SPI driver specification)
| `delay_c`   | Delay before Control function is called (ms)
| `delay_t`   | Delay after Control function is called but before Transfer function is called (ms)
| `timeout`   | Total transfer timeout including delay_c and delay_t delays, including delay_c and delay_t (ms)

## Response Formats

| Command     | Response
|-------------|---------
| `GET VER`   | 16 bytes, "major.minor.patch"
| `GET CAP`   | 32 bytes, "mode_mask,format_mask,data_bit_mask,bit_order_mask,min_bus_speed_in_kbps,max_bus_speed_in_kbps"
|             | mode_mask (2 digits hex): specifies mask of supported modes
|             |   - bit 0.:  Master
|             |   - bit 1.:  Slave
|             | format_mask (2 digits hex): specifies mask of supported clock/frame formats
|             |   - bit 0.:  Clock Polarity 0, Clock Phase 0
|             |   - bit 1.:  Clock Polarity 0, Clock Phase 1
|             |   - bit 2.:  Clock Polarity 1, Clock Phase 0
|             |   - bit 3.:  Clock Polarity 1, Clock Phase 1
|             |   - bit 4.:  Texas Instruments Frame Format
|             |   - bit 5.:  National Semiconductor Microwire Frame Format
|             | data_bit_mask (8 digits hex): specifies mask of supported data bits
|             |   - bit 0.:  Data Bits 1
|             |      ...
|             |   - bit 31.: Data Bits 32
|             | bit_order_mask (2 digits hex): specifies mask of supported bit orders
|             |   - bit 0.:  MSB first
|             |   - bit 1.:  LSB first
|             | min_bus_speed_in_kbps (dec): minimum supported bus speed (in kbps)
|             | max_bus_speed_in_kbps (dec): maximum supported bus speed (in kbps)
| `GET BUF`   | `len` bytes from respective buffer
| `GET CNT`   | 16 bytes (dec)

---

## Communication Example

```text
-> GET VER           <- "1.1.0"
-> GET CAP           <- "03,1F,00008080,03,1000,10000"
-> SET BUF TX,0,53   (Fill TX with 'S')
-> SET BUF RX,0,3F   (Fill RX with '?')
-> GET BUF RX,16     <- "????????????????"
-> SET COM 1,0,8,0,1,2000000  (Slave mode, CPOL=0, CPHA=0, 8-bit, MSB, 2 Mbps)
-> XFER 16,10,0,100  <-> 16-byte transfer
-> GET CNT           <- "16"
```

---

## Build Targets

| Target      | Description
|-------------|------------
| **Debug**   | Low optimization build with GLCD interface (for debugging).
| **Release** | Optimized binary without UI (for production testing).
