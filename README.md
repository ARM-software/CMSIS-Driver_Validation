# CMSIS-Driver Validation

Test suite for verifying that a peripheral driver implementation is compliant with the corresponding [CMSIS-Driver Specification](https://arm-software.github.io/CMSIS_5/Driver/html/index.html).

## Overview
The branch **main** of this repository contains the code of CMSIS-Driver Validation Suite. [User documentation](http://arm-software.github.io/CMSIS-Driver_Validation/main/index.html) explains the scope and the usage of the framework.

[See verified releases](https://github.com/ARM-software/CMSIS-Driver_Validation/releases) of CMSIS-Driver Validation suite in source code archives and in [CMSIS-Pack (.pack)](https://open-cmsis-pack.github.io/Open-CMSIS-Pack-Spec/main/html/index.html) format as download assets.

Feel free to raise an [issue on GitHub](https://github.com/ARM-software/CMSIS-Driver_Validation/issues) to report a misbehavior (i.e. bugs), start discussions about enhancements or create a pull request with suggested modifications.

### Supported Driver Interfaces

The CMSIS-Driver Validation provides tests for the following CMSIS-Driver interfaces:

Extensive tests with available test servers:
  - **SPI** - Serial Peripheral Interface driver.
  - **USART** - Universal Synchronous and Asynchronous Receiver/Transmitter interface driver.
  - **WiFi** - Wireless Fidelity Interface module/shield driver.

Basic tests:
  - **CAN** - Controller Area Network interface driver.
  - **Ethernet** - Ethernet MAC and PHY peripheral interface driver.
  - **I2C** - Inter-Integrated Circuit multi-master serial single-ended bus interface - driver.
  - **MCI** - Memory Card Interface driver for SD/MMC memory.
  - **USB Device** - Universal Serial Bus Device interface driver.
  - **USB Host** - Universal Serial Bus Host interface driver.


## Repository Structure

| Directory/File        | Content                                                   |
| --------------------- | --------------------------------------------------------- |
| [`Boards`](./Boards/) | Driver Validation examples for various boards             |
| [`Config`](./Config/) | Configuration files for the Driver Validation framework   |
| [`Documentation`](./Documentation)    | Placeholder for the offline documentation in the pack     |
| [`DoxyGen`](./Doxygen)          | Source of the documentation                               |
| [`Include`](./Include)          | Header files for Driver Validation components             |
| [`Scripts`](./Scripts)          | Script files for XML reports                              |
| [`Source`](./Source)           | Source files for Driver Validation components             |
| [`Tools`](./Tools)            | Various Server implementations for extensive testing      |
| [`ARM.CMSIS-Driver_Validation.pdsc`](./ARM.CMSIS-Driver_Validation.pdsc) | Open-CMSIS-Pack description file           |
| [`gen_pack.sh`](./gen_pack.sh)       | Open-CMSIS-Pack generation script                         |
| [`LICENSE.txt`](./LICENSE.txt)       | License text for the repository content                   |



## Examples

Folder [`Boards`](./Boards/) contains example projects that show how to use the CMSIS-Driver Validation on a real hardware with available CMSIS-Driver implementations.The examples are also included in CMSIS-Driver Validation Software Pack. Details are explained in [Examples documentation](https://arm-software.github.io/CMSIS-Driver_Validation/main/examples.html).

## Build CMSIS-Driver Validation as Open-CMSIS-Pack

 A generator script [`gen_pack.sh`](./gen_pack.sh) is provided for building the CMSIS-Driver Validation as [Open-CMSIS-Pack](https://open-cmsis-pack.github.io/Open-CMSIS-Pack-Spec/main/html/index.html) using the [gen-pack library](https://github.com/Open-CMSIS-Pack/gen-pack). Simply follow the steps below:
  
 - Verify that following tools are installed on the PC:
   - git bash (e.g. for Windows: https://gitforwindows.org/)
   - ZIP archive creation utility (e.g. [7-Zip](http://www.7-zip.org/download.html))
   - Doxygen version 1.9.2 (https://sourceforge.net/projects/doxygen/files/rel-1.9.2/)
 - Checkout this repository. For example in git bash with:
    ```git clone https://github.com/ARM-Software/CMSIS-Driver_Validation```
 - In the local repository folder execute `./gen_pack.sh` in the bash shell.
   - this creates a pack file in the local `output` folder. For example `./output/ARM.CMSIS-Driver_Validation.3.0.1-dev12+g3725082.pack`.

## License

Arm CMSIS-Driver Validation is licensed under [Apache 2.0 license](https://opensource.org/licenses/Apache-2.0).

## Useful resources

| Link                        | Description                                                 |
|:--------------------------- |:----------------------------------------------------------- |
| [CMSIS](https://github.com/ARM-software/cmsis_5)               | CMSIS GitHub repository  |
| [CMSIS-Driver Specification](https://arm-software.github.io/CMSIS_5/latest/Driver/html/index.html) | Specification of CMSIS-Driver API    |
| [CMSIS-Driver](https://github.com/ARM-software/CMSIS-Driver)   |  Reference CMSIS-Driver implementations for external peripheral devices |
| [Open-CMSIS-Pack](https://www.open-cmsis-pack.org)  |  Overview of the Open-CMSIS-Pack project|