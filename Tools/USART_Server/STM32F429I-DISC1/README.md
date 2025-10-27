# USART Server Application for STM32F429I-DISC1

This application serves as a **USART Server** running on the STMicroelectronics **STM32F429I-DISC1 (32F429IDISCOVERY)** development board.  
It provides a physical test environment for validating USART driver compliance to the **CMSIS USART driver specification v2.0.0 and above**.

---

## Overview

The USART Server waits to receive commands over the USART interface from an external USART Client (Driver Validation).  
Upon receiving a command, it executes the corresponding operation and returns to the waiting state.  
Commands may either execute to completion or terminate upon timeout.

All commands (except `XFER`) utilize a (mostly fixed) communication configuration and timeout as defined in the **`USART_Server_Config.h`** file.  
The `XFER` command temporarily applies custom USART settings defined by the most recent `SET COM` command.

---

## Default USART Configuration

| Parameter     | Value
|---------------|------
| Mode          | Asynchronous, Single-wire or IrDA (configurable in the `USART_Server_Config.h` file)
| Baudrate      | 115200
| Data Bits     | 8
| Parity        | None
| Stop Bits     | 1
| Flow Control  | None

---

## Hardware Connections

Peripheral used: **USART1**

| Function     | Pin  | Description
|--------------|------|------------
| TX           | PA9  | Transmit
| RX           | PA10 | Receive
| CK           | PA8  | Synchronous mode Clock
| CTS          | PA11 | Clear To Send
| RTS          | PA12 | Request To Send
| DCD test pin | PG2  | Data Carrier Detect
| RI test pin  | PG3  | Ring Indicator
| GND          | GND  | Any board ground

> **Note:** In Single-wire mode TX pin is used for both transmit and receive working in half-duplex mode.

> **Note:** USART pins are MCU pins before RS232 level shifter.

---

## Command Protocol

Each command is **32 bytes long**, padded with zeros if shorter. Optional parameters are enclosed in `[]`.

### Supported Commands

| Command (32 bytes zero padded)                                         | Data Phase Direction | Description
|------------------------------------------------------------------------|----------------------|------------
| `GET VER`                                                              | OUT (16 bytes)       | Returns firmware version in `major.minor.patch` format.
| `GET CAP`                                                              | OUT (32 bytes)       | Returns supported mode/format/bit masks and speed limits.
| `SET BUF RX/TX,len[,pattern]`                                          | IN (`len` bytes)     | Sets RX/TX buffer; optionally pre-fills with pattern.
| `GET BUF RX/TX,len`                                                    | OUT (`len` bytes)    | Reads `len` bytes from RX/TX buffer.
| `SET COM mode,data_bits,parity,stop_bits,flow_ctrl,cpol,cpha,baudrate` | —                    | Sets custom USART communication parameters for next `XFER`.
| `XFER dir,num[,delay][,timeout][,num_rts]`                             | IN/OUT (`num` items) | Performs USART data transfer based on direction.
| `GET CNT`                                                              | OUT (16 bytes)       | Returns count in decimal notation.
| `SET BRK delay,duration`                                               | —                    | Sets break signal timing parameters.
| `GET BRK`                                                              | OUT (1 byte)         | Returns break signal status.
| `SET MDM mdm_ctrl,delay,duration`                                      | —                    | Controls modem lines with timing.
| `GET MDM`                                                              | OUT (1 byte)         | Returns modem line states.

## Command Parameters

| Parameter      | Description
|----------------|------------
| `RX/TX`        | RX = USART Server receive buffer, TX = USART Server transmit buffer
| `len`          | Data length for data phase
| `pattern`      | Hex value used to pre-fill buffer
| `mode`         | 1 = Asynchronous
|                | 2 = Synchronous Master
|                | 3 = Synchronous Slave
|                | 4 = Single Wire
|                | 5 = IrDA
|                | 6 = Smart Card
| `data_bits`    | Data bits (5–9)
| `parity`       | 0 = None, 1 = Even, 2 = Odd
| `stop_bits`    | 0 = 1 Stop Bit, 1 = 2 Stop Bits, 2 = 1.5 Stop Bits, 3 = 0.5 Stop Bits
| `flow_ctrl`    | 0 = None, 1 = CTS, 2 = RTS, 3 = RTS/CTS
| `cpol`         | Clock polarity (Synchronous mode only): 0 = Rising edge, 1 = Falling edge
| `cpha`         | Clock phase (Synchronous mode only): 0 = First edge, 1 = Second edge
| `baudrate`     | Baudrate in bauds
| `dir`          | 0 = Send (Tx), 1 = Receive (Rx), 2 = Transfer (simultaneous Tx/Rx)
| `num`          | Number of items (according CMSIS USART driver specification)
| `delay`        | Initial delay before operation (ms)
| `timeout`      | Transfer timeout after delay (ms)
| `num_rts`      | Items after which RTS line should be deactivated
| `mdm_ctrl`     | Modem lines state (2 digits hex): bit 0=RTS, bit 1=DTS, bit 2=DCD, bit 3=RI
| `duration`     | Duration for controlling modem lines (ms)

## Response Formats

| Command     | Response
|-------------|---------
| `GET VER`   | 16 bytes, "major.minor.patch"
| `GET CAP`   | 32 bytes, "mode_mask,data_bits_mask,parity_mask,stop_bits_mask,flow_control_mask,modem_line_mask,min_baudrate,max_baudrate"
|             | mode_mask (2 digits hex): supported modes
|             |   - bit 0: Asynchronous
|             |   - bit 1: Synchronous Master
|             |   - bit 2: Synchronous Slave
|             |   - bit 3: Single Wire
|             |   - bit 4: IrDA
|             |   - bit 5: Smart Card
|             | data_bits_mask (2 digits hex): supported data bits
|             |   - bit 0: Data Bits 5
|             |   - bit 1: Data Bits 6
|             |   - bit 2: Data Bits 7
|             |   - bit 3: Data Bits 8
|             |   - bit 4: Data Bits 9
|             | parity_mask (1 digit hex): supported parity options
|             |   - bit 0: No Parity
|             |   - bit 1: Even Parity
|             |   - bit 2: Odd Parity
|             | stop_bits_mask (1 digit hex): supported stop bits
|             |   - bit 0: 1 Stop Bit
|             |   - bit 1: 2 Stop Bits
|             |   - bit 2: 1.5 Stop Bits
|             |   - bit 3: 0.5 Stop Bits
|             | flow_control_mask (1 digit hex): supported flow control
|             |   - bit 0: No Flow Control
|             |   - bit 1: CTS
|             |   - bit 2: RTS
|             |   - bit 3: RTS/CTS
|             | modem_line_mask (1 digit hex): supported modem lines
|             |   - bit 0: RTS line available
|             |   - bit 1: CTS line available
|             |   - bit 2: DTR line available
|             |   - bit 3: DSR line available
|             |   - bit 4: GPIO for DCD line available
|             |   - bit 5: GPIO for RI line available
|             | min_baudrate (dec): minimum supported baudrate (bauds)
|             | max_baudrate (dec): maximum supported baudrate (bauds)
| `GET BUF`   | `len` bytes from respective buffer
| `GET CNT`   | 16 bytes (dec)
| `GET BRK`   | 1 byte (hex) containing break signal status
| `GET MDM`   | 1 byte (hex) containing modem lines state:
|             |   - bit 0: CTS line current state
|             |   - bit 1: DSR line current state

---

## Communication Example

```text
-> GET VER           <- "1.0.0"
-> GET CAP           <- "3B,18,7,F,F,03,9600,5000000"
-> SET BUF TX,0,53   (Fill TX with 'S')
-> SET BUF RX,0,3F   (Fill RX with '?')
-> GET BUF RX,16     <- "????????????????"
-> SET COM 1,8,0,0,0,0,0,115200  (Async, 8-bit, no parity, 1 stop, no flow, 115200 baud)
-> XFER 1,16,0,100   <- 16-byte receive
-> XFER 0,16,0,100   -> 16-byte send
-> GET CNT           <- "16"
-> GET BRK           <- "1"
-> SET MDM 01,10,50  (Activate RTS after 10 ms for 50 ms)
-> GET MDM           <- "3"
```

---

## Build Targets

| Target      | Description
|-------------|------------
| **Debug**   | Low optimization build with GLCD interface (for debugging).
| **Release** | Optimized binary without UI (for production testing).
