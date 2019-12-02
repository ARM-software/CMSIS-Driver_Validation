# CMSIS Driver Validation

The branch *master* of this GitHub repository contains the CMSIS-Driver Validation Version 1.2.0.  The [documentation](http://arm-software.github.io/CMSIS-Driver_Validation/html/index.html) is available under 
 http://arm-software.github.io/CMSIS-Driver_Validation/html/index.html

Use [Issues](https://github.com/ARM-software/CMSIS-Driver_Validation#issues-and-labels) to provide feedback and report problems for CMSIS-Driver Validation Version 1.1.0. 

**Note:** The branch *develop* of this GitHub repository reflects our current state of development and is constantly updated. It gives our users and partners contiguous access to the CMSIS development. It allows you to review the work and provide feedback or create pull requests for contributions.

## Directory Structure

| Directory        | Content                                                   |                
| ---------------- | --------------------------------------------------------- |
| Boards           | Driver Validation Projects for a set of boards            |
| DoxyGen          | Source of the documentation                               |
| Include          | header files for driver validation components             |
| Source           | source files for driver validation components             |
| Tools            | SockServer source and executable files                    |
| Utilities        | Utility programs                                          |

## Generate CMSIS Pack for Release

To build a complete CMSIS pack for installation the following additional tools are required:
 - **doxygen.exe**    Version: 1.8.6 (Documentation Generator)
 - **mscgen.exe**     Version: 0.20  (Message Sequence Chart Converter)
 - **7z.exe (7-Zip)** Version: 16.02 (File Archiver)
  
Using these tools, you can generate on a Windows PC:
 - **CMSIS Software Pack** using the batch file **gen_pack.bat** (located in ./CMSIS/Utilities). This batch file also generates the documentation.
 - **CMSIS Documentation** using the batch file **genDoc.bat** (located in ./CMSIS/Doxygen). 

## License

Arm CMSIS is licensed under Apache-2.0.

## Contributions and Pull Requests
Contributions are accepted under Apache-2.0. Only submit contributions where you have authored all of the code.

### Issues and Labels

Please feel free to raise an [issue on GitHub](https://github.com/ARM-software/CMSIS-Driver_Validation/issues)
to report misbehavior (i.e. bugs) or start discussions about enhancements. This
is your best way to interact directly with the maintenance team and the community.
We encourage you to append implementation suggestions as this helps to decrease the
workload of the very limited maintenance team. 

We will be monitoring and responding to issues as best we can.
Please attempt to avoid filing duplicates of open or closed items when possible.
In the spirit of openness we will be tagging issues with the following:

- **bug** – We consider this issue to be a bug that will be investigated.

- **wontfix** - We appreciate this issue but decided not to change the current behavior.
	
- **enhancement** – Denotes something that will be implemented soon. 

- **future** - Denotes something not yet schedule for implementation.

- **out-of-scope** - We consider this issue loosely related to CMSIS. It might by implemented outside of CMSIS. Let us know about your work.
	
- **question** – We have further questions to this issue. Please review and provide feedback.

- **documentation** - This issue is a documentation flaw that will be improved in future.

- **review** - This issue is under review. Please be patient.
	
- **DONE** - We consider this issue as resolved - please review and close it. In case of no further activity this issues will be closed after a week.

- **duplicate** - This issue is already addressed elsewhere, see comment with provided references.

- **Important Information** - We provide essential informations regarding planned or resolved major enhancements.

