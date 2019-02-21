/* -----------------------------------------------------------------------------
 * Copyright (c) 2017 ARM Ltd.
 *  
 * This software is provided 'as-is', without any express or implied warranty. 
 * In no event will the authors be held liable for any damages arising from 
 * the use of this software. Permission is granted to anyone to use this 
 * software for any purpose, including commercial applications, and to alter 
 * it and redistribute it freely, subject to the following restrictions:
 *  
 * 1. The origin of this software must not be misrepresented; you must not 
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be 
 *    appreciated but is not required. 
 * 
 * 2. Altered source versions must be plainly marked as such, and must not be 
 *    misrepresented as being the original software. 
 * 
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * $Date:        07. September 2017
 * $Revision:    V1.4.0
 *
 * Project:      RTE Device Configuration for Atmel SAM-ESV7
 * -------------------------------------------------------------------------- */

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

#ifndef __RTE_DEVICE_H
#define __RTE_DEVICE_H

#include "chip.h"

// <e> SPI0 (Serial Peripheral Interface 0) [Driver_SPI0]
// <i> Configuration settings for Driver_SPI0 in component ::CMSIS Driver:SPI
#define RTE_SPI0                         1

//   <o> SPI0_MISO Pin <0=>Not Used <1=>PD20
//   <i> Master In Slave Out (MISO) 
//   <i> This data line supplies the output data from a slave to the input of the master.
#define RTE_SPI0_MISO_PORT_ID            1
#if    (RTE_SPI0_MISO_PORT_ID == 0)
#define RTE_SPI0_MISO_PORT               NULL
#elif  (RTE_SPI0_MISO_PORT_ID == 1)           
#define RTE_SPI0_MISO_PORT               {PIO_PD20, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid SPI0_MISO Pin Configuration!"
#endif

//   <o> SPI0_MOSI Pin <0=>Not Used <1=>PD21
//   <i> Master Out Slave In (MOSI)
//   <i> This data line supplies the output data from the master to the input of the slave.
#define RTE_SPI0_MOSI_PORT_ID            1
#if    (RTE_SPI0_MOSI_PORT_ID == 0)
#define RTE_SPI0_MOSI_PORT               NULL 
#elif  (RTE_SPI0_MOSI_PORT_ID == 1)
#define RTE_SPI0_MOSI_PORT               {PIO_PD21, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid SPI0_MOSI Pin Configuration!"
#endif

//   <o> SPI0_SPCK Pin <0=>Not Used <1=>PD22
//   <i> Serial Clock (SPCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_SPI0_SPCK_PORT_ID            1
#if    (RTE_SPI0_SPCK_PORT_ID == 0)
#define RTE_SPI0_SPCK_PORT               NULL
#elif  (RTE_SPI0_SPCK_PORT_ID == 1)
#define RTE_SPI0_SPCK_PORT               {PIO_PD22, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid SPI0_SPCK Pin Configuration!"
#endif

//   <o> SPI0_NPCS Pin <0=>Not Used <1=>PA31 <2=>PB2 <3=>PD12 <4=>PD25 <5=>PD27
//   <i> Peripheral Chip Select (NPCS) 
//   <i> Output in master mode.
//   <i> Input in slave mode (only PB2).
#define RTE_SPI0_NPCS_PORT_ID            2
#if    (RTE_SPI0_NPCS_PORT_ID == 0)      
#define RTE_SPI0_NPCS_PORT               NULL
#define RTE_SPI0_NPCS_NUM                NULL
#elif  (RTE_SPI0_NPCS_PORT_ID == 1)      
#define RTE_SPI0_NPCS_PORT               {PIO_PA31, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT} 
#define RTE_SPI0_NPCS_NUM                1
#elif  (RTE_SPI0_NPCS_PORT_ID == 2)      
#define RTE_SPI0_NPCS_PORT               {PIO_PB2, PIOB, ID_PIOB, PIO_PERIPH_D, PIO_DEFAULT}
#define RTE_SPI0_NPCS_NUM                0
#elif  (RTE_SPI0_NPCS_PORT_ID == 3)      
#define RTE_SPI0_NPCS_PORT               {PIO_PD12, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define RTE_SPI0_NPCS_NUM                2
#elif  (RTE_SPI0_NPCS_PORT_ID == 4)      
#define RTE_SPI0_NPCS_PORT               {PIO_PD25, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#define RTE_SPI0_NPCS_NUM                1
#elif  (RTE_SPI0_NPCS_PORT_ID == 5)      
#define RTE_SPI0_NPCS_PORT               {PIO_PD27, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#define RTE_SPI0_NPCS_NUM                3
#else
#error "Invalid SPI0_NPCS Pin Configuration!"
#endif

//   <q> DMA SPI0 TX Channel
//   <i> Enable DMA TX Channel for data transfers
#define RTE_SPI0_DMA_TX            0
//   <q> DMA SPI0 RX Channel
//   <i> Enable DMA RX Channel for data transfers
#define RTE_SPI0_DMA_RX            0

#if (RTE_SPI0==1)
#if (RTE_SPI0_DMA_RX==1)&(RTE_SPI0_DMA_TX==0)
#error "SPI DMA RX Channel requires SPI DMA TX Channel enabled!"
#endif
#if (RTE_SPI0_DMA_TX==1)&(RTE_SPI0_DMA_RX==0)
#warning "SPI DMA RX Channel is disabled, only SPI Send function can be used!"
#endif
#endif

// </e>


// <e> SPI1 (Serial Peripheral Interface 1) [Driver_SPI1]
// <i> Configuration settings for Driver_SPI1 in component ::CMSIS Driver:SPI
#define RTE_SPI1                         0

//   <o> SPI1_MISO Pin <0=>Not Used <1=>PC26
//   <i> Master In Slave Out (MISO) 
//   <i> This data line supplies the output data from a slave to the input of the master.
#define RTE_SPI1_MISO_PORT_ID            1
#if    (RTE_SPI1_MISO_PORT_ID == 0)
#define RTE_SPI1_MISO_PORT               NULL 
#elif  (RTE_SPI1_MISO_PORT_ID == 1)           
#define RTE_SPI1_MISO_PORT               {PIO_PC26, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
#else
#error "Invalid SPI1_MISO Pin Configuration!"
#endif

//   <o> SPI1_MOSI Pin <0=>Not Used <1=>PC27
//   <i> Master Out Slave In (MOSI) 
//   <i> This data line supplies the output data from the master to the input of the slave.
#define RTE_SPI1_MOSI_PORT_ID            1
#if    (RTE_SPI1_MOSI_PORT_ID == 0)
#define RTE_SPI1_MOSI_PORT               NULL 
#elif  (RTE_SPI1_MOSI_PORT_ID == 1)
#define RTE_SPI1_MOSI_PORT               {PIO_PC27, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
#else
#error "Invalid SPI1_MOSI Pin Configuration!"
#endif

//   <o> SPI1_SPCK Pin <0=>Not Used <1=>PC24
//   <i> Serial Clock (SPCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_SPI1_SPCK_PORT_ID            1
#if    (RTE_SPI1_SPCK_PORT_ID == 0)
#define RTE_SPI1_SPCK_PORT               NULL 
#elif  (RTE_SPI1_SPCK_PORT_ID == 1)
#define RTE_SPI1_SPCK_PORT               {PIO_PC24, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
#else
#error "Invalid SPI1_SPCK Pin Configuration!"
#endif

//   <o> SPI1_NPCS Pin <0=>Not Used <1=>PC25 <2=>PC28 <3=>PC29 <4=>PC30 <5=>PD0 <6=>PD1 <7=>PD2
//   <i> Peripheral Chip Select (NPCS) 
//   <i> Output in master mode.
//   <i> Input in slave mode (only PC25).
#define RTE_SPI1_NPCS_PORT_ID            1
#if    (RTE_SPI1_NPCS_PORT_ID == 0)      
#define RTE_SPI1_NPCS_PORT               NULL 
#define RTE_SPI1_NPCS_NUM                NULL
#elif  (RTE_SPI1_NPCS_PORT_ID == 1)      
#define RTE_SPI1_NPCS_PORT               {PIO_PC25, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
#define RTE_SPI1_NPCS_NUM                0
#elif  (RTE_SPI1_NPCS_PORT_ID == 2)      
#define RTE_SPI1_NPCS_PORT               {PIO_PC28, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
#define RTE_SPI1_NPCS_NUM                1
#elif  (RTE_SPI1_NPCS_PORT_ID == 3)      
#define RTE_SPI1_NPCS_PORT               {PIO_PC29, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
#define RTE_SPI1_NPCS_NUM                2
#elif  (RTE_SPI1_NPCS_PORT_ID == 4)      
#define RTE_SPI1_NPCS_PORT               {PIO_PC30, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
#define RTE_SPI1_NPCS_NUM                3
#elif  (RTE_SPI1_NPCS_PORT_ID == 5)      
#define RTE_SPI1_NPCS_PORT               {PIO_PD0, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define RTE_SPI1_NPCS_NUM                1
#elif  (RTE_SPI1_NPCS_PORT_ID == 6)      
#define RTE_SPI1_NPCS_PORT               {PIO_PD1, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define RTE_SPI1_NPCS_NUM                2
#elif  (RTE_SPI1_NPCS_PORT_ID == 7)      
#define RTE_SPI1_NPCS_PORT               {PIO_PD2, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define RTE_SPI1_NPCS_NUM                3
#else
#error "Invalid SPI1_NPCS Pin Configuration!"
#endif

//   <q> DMA SPI1 TX Channel
//   <i> Enable DMA TX Channel for data transfers
#define RTE_SPI1_DMA_TX                  0
//   <q> DMA SPI1 RX Channel
//   <i> Enable DMA RX Channel for data transfers
#define RTE_SPI1_DMA_RX                  0

#if (RTE_SPI1==1)
#if (RTE_SPI1_DMA_RX==1)&(RTE_SPI1_DMA_TX==0)
#error "SPI DMA RX Channel requires SPI DMA TX Channel enabled!"
#endif
#if (RTE_SPI1_DMA_TX==1)&(RTE_SPI1_DMA_RX==0)
#warning "SPI DMA RX Channel is disabled, only SPI Send function can be used!"
#endif
#endif

// </e>


// <e> ETH (Ethernet Interface) [Driver_ETH_MAC0]
// <i> Configuration settings for Driver_ETH_MAC0 in component ::CMSIS Driver:Ethernet MAC
#define RTE_ETH                         1

//   <o>Interface
//     <0=> MII 
//     <1=> RMII 
//   <i> MII (Media Independent Interface)
//   <i> RMII (Reduced Media Independent Interface) 
#define RTE_ETH_RMII                     1

//   <o> DMA Descriptor/Buffer Memory Address <0x20000000-0xE0000000>
//   <i> Configure location of the Ethernet DMA Descriptor and Buffer memory
//   <i> Ensure it is in a non-cacheable RAM region
#define RTE_ETH_DMA_MEM_ADDR             0x2045F000

// </e>


// <e> SDIO (Secure Digital Input/Output) [Driver_MCI0]
// <i> Configuration settings for Driver_MCI0 in component ::CMSIS Driver:MCI
#define RTE_SDIO                        1

//   <o> Card Detect Pin <0=>Not Used <1=>PD18 <2=>PC16
#define RTE_MCI_CD_PORT_ID               0
#if    (RTE_MCI_CD_PORT_ID == 0)
#define RTE_MCI_CD_PORT                  NULL 
#elif  (RTE_MCI_CD_PORT_ID == 1)         
#define RTE_MCI_CD_PORT                  {PIO_PD18, PIOD, ID_PIOD, PIO_INPUT, PIO_PULLUP}
#elif  (RTE_MCI_CD_PORT_ID == 2)         
#define RTE_MCI_CD_PORT                  {PIO_PC16, PIOC, ID_PIOC, PIO_INPUT, PIO_PULLUP}

#else
#error "Invalid MCI Card Detect Pin Configuration!"
#endif

//   <o> Write Protect Pin <0=>Not Used
#define RTE_MCI_WP_PORT_ID               0
#if    (RTE_MCI_WP_PORT_ID == 0)         
#define RTE_MCI_WP_PORT                  NULL 
#else
#error "Invalid MCI Write Protect Pin Configuration!"
#endif

// </e>

// <e> I2C (Inter Integrated Circuit) [Driver_I2C0]
// <i> Configuration settings for Drvier_I2C0 in  component ::CMSIS Driver:I2C
#define RTE_I2C0                         1

//   <o> I2C0 Mode <0=> Polling mode <1=> With interruption
//   <i> Configuration settings for the I2C0 peripheral
#define RTE_I2C0_MODE                    0
#if     RTE_I2C0_MODE == 0
#define RTE_I2C0_MODE_POLLING
#elif   RTE_I2C0_MODE == 1
#define RTE_I2C0_MODE_IRQ
#endif

//   <o> I2C0_SDA Pin <0=>Not Used <1=>PA3
//   <i> SDA (SDA)
//   <i> This data line send and receive the data.
#define RTE_I2C0_SDA_PORT_ID             1
#if    (RTE_I2C0_SDA_PORT_ID == 0)
#define RTE_I2C0_SDA_PORT                NULL
#elif  (RTE_I2C0_SDA_PORT_ID == 1)
#define RTE_I2C0_SDA_PORT                {PIO_PA3, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid I2C0_SDA Pin Configuration!"
#endif

//   <o> I2C0_SCL Pin <0=>Not Used <1=>PA4
//   <i> SCL (SCL)
//   <i> This data line send and receive the data.
#define RTE_I2C0_SCL_PORT_ID             1
#if    (RTE_I2C0_SCL_PORT_ID == 0)
#define RTE_I2C0_SCL_PORT                NULL
#elif  (RTE_I2C0_SCL_PORT_ID == 1)
#define RTE_I2C0_SCL_PORT                {PIO_PA4, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid I2C0_SCL Pin Configuration!"
#endif

// </e>

// <e> I2C (Inter Integrated Circuit) [Driver_I2C]
// <i> Configuration settings for Drvier_I2C1 in  component ::CMSIS Driver:I2C
#define RTE_I2C1                         0

//   <o> I2C1 Mode <0=> Polling mode <1=> With interruption
//   <i> Configuration settings for the I2C1 peripheral
#define RTE_I2C1_MODE                    0
#if     RTE_I2C1_MODE == 0
#define RTE_I2C1_MODE_POLLING
#elif   RTE_I2C1_MODE == 1
#define RTE_I2C1_MODE_IRQ
#endif

//   <o> I2C1_SDA Pin <0=>Not Used <1=>PB4
//   <i> SDA (SDA)
//   <i> This data line send and receive the data.
#define RTE_I2C1_SDA_PORT_ID             1
#if    (RTE_I2C1_SDA_PORT_ID == 0)
#define RTE_I2C1_SDA_PORT                NULL
#elif  (RTE_I2C1_SDA_PORT_ID == 1)
#define RTE_I2C1_SDA_PORT                {PIO_PB4, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid I2C1_SDA Pin Configuration!"
#endif

//   <o> I2C1_SCL Pin <0=>Not Used <1=>PB5
//   <i> SCL (SCL)
//   <i> This data line send and receive the data.
#define RTE_I2C1_SCL_PORT_ID             1
#if    (RTE_I2C1_SCL_PORT_ID == 0)
#define RTE_I2C1_SCL_PORT                NULL
#elif  (RTE_I2C1_SCL_PORT_ID == 1)
#define RTE_I2C1_SCL_PORT                {PIO_PB5, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid I2C1_SCL Pin Configuration!"
#endif

// </e>

// <e> I2C (Inter Integrated Circuit) [Driver_I2C2]
// <i> Configuration settings for Drvier_I2C2 in  component ::CMSIS Driver:I2C
#define RTE_I2C2                         0

//   <o> I2C2 Mode <0=> Polling mode <1=> With interruption
//   <i> Configuration settings for the I2C2 peripheral
#define RTE_I2C2_MODE                    0
#if     RTE_I2C2_MODE == 0
#define RTE_I2C2_MODE_POLLING
#elif   RTE_I2C2_MODE == 1
#define RTE_I2C2_MODE_IRQ
#endif

//   <o> I2C2_SDA Pin <0=>Not Used <1=>PD27
//   <i> SDA (SDA)
//   <i> This data line send and receive the data.
#define RTE_I2C2_SDA_PORT_ID             1
#if    (RTE_I2C2_SDA_PORT_ID == 0)
#define RTE_I2C2_SDA_PORT                NULL
#elif  (RTE_I2C2_SDA_PORT_ID == 1)
#define RTE_I2C2_SDA_PORT                {PIO_PD27, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#else
#error "Invalid I2C2_SDA Pin Configuration!"
#endif

//   <o> I2C2_SCL Pin <0=>Not Used <1=>PD28
//   <i> SCL (SCL)
//   <i> This data line send and receive the data.
#define RTE_I2C2_SCL_PORT_ID             1
#if    (RTE_I2C2_SCL_PORT_ID == 0)
#define RTE_I2C2_SCL_PORT                NULL
#elif  (RTE_I2C2_SCL_PORT_ID == 1)
#define RTE_I2C2_SCL_PORT                {PIO_PD28, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#else
#error "Invalid I2C2_SCL Pin Configuration!"
#endif

// </e>

// <e> USART0 (Universal Synchronous Asynchonous Receive TRansmitter 0) [Driver_USART0]
// <i> Configuration settings for Driver_USART0 in component ::CMSIS Driver:USART
#define RTE_USART0                       1

//   <o> USART0 TX Operating Mode <0=> Polling <1=> Interrupt <2=> DMA
//   <i> Configuration settings for the USART0 peripheral
#define RTE_USART0_TX_MODE               1
#if   RTE_USART0_TX_MODE == 0
#define RTE_USART0_POLLING_TX
#elif RTE_USART0_TX_MODE == 1
#define RTE_USART0_IRQ_TX
#elif RTE_USART0_TX_MODE == 2
#define RTE_USART0_DMA_TX                1
#endif

//   <o> USART0 RX Operating Mode <0=> Polling <1=> Interrupt <2=> DMA
//   <i> Configuration settings for the USART0 peripheral
#define RTE_USART0_RX_MODE               1
#if   RTE_USART0_RX_MODE == 0
#define RTE_USART0_POLLING_RX
#elif RTE_USART0_RX_MODE == 1
#define RTE_USART0_IRQ_RX
#elif RTE_USART0_RX_MODE == 2
#define RTE_USART0_DMA_RX                1
#endif

//   <o> USART0_RX Pin <0=>Not Used <1=>PB0
//   <i> RX (RX) 
//   <i> This data line recieves the data.
#define RTE_USART0_RX_PORT_ID            1
#if    (RTE_USART0_RX_PORT_ID == 0)
#define RTE_USART0_RX_PORT               NULL
#elif  (RTE_USART0_RX_PORT_ID == 1)           
#define RTE_USART0_RX_PORT               {PIO_PB0, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}
#else
#error "Invalid USART0_RX Pin Configuration!"
#endif

//   <o> USART0_TX Pin <0=>Not Used <1=>PB1
//   <i> TX (TX)
//   <i> This data line supplies the output data.
#define RTE_USART0_TX_PORT_ID            1
#if    (RTE_USART0_TX_PORT_ID == 0)
#define RTE_USART0_TX_PORT               NULL 
#elif  (RTE_USART0_TX_PORT_ID == 1)
#define RTE_USART0_TX_PORT               {PIO_PB1, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}
#else
#error "Invalid USART0_TX Pin Configuration!"
#endif

//   <o> USART0_SCK Pin <0=>Not Used <1=>PB13
//   <i> Serial Clock (SCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_USART0_SCK_PORT_ID            0
#if    (RTE_USART0_SCK_PORT_ID == 0)
#define RTE_USART0_SCK_PORT               NULL
#elif  (RTE_USART0_SCK_PORT_ID == 1)
#define RTE_USART0_SCK_PORT               {PIO_PB13, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}
#else
#error "Invalid USART0_SPCK Pin Configuration!"
#endif

//   <o> USART0_RTS Pin <0=>Not Used <1=>PB3
//   <i> Request to Send (SCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_USART0_RTS_PORT_ID            0
#if    (RTE_USART0_RTS_PORT_ID == 0)
#define RTE_USART0_RTS_PORT               NULL
#elif  (RTE_USART0_RTS_PORT_ID == 1)
#define RTE_USART0_RTS_PORT               {PIO_PB3, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}
#else
#error "Invalid USART0_RTS Pin Configuration!"
#endif

//   <o> USART0_CTS Pin <0=>Not Used <1=>PB2
//   <i> Request to Send (SCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_USART0_CTS_PORT_ID            0
#if    (RTE_USART0_CTS_PORT_ID == 0)
#define RTE_USART0_CTS_PORT               NULL
#elif  (RTE_USART0_CTS_PORT_ID == 1)
#define RTE_USART0_CTS_PORT               {PIO_PB2, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}
#else
#error "Invalid USART0_CTS Pin Configuration!"
#endif

//   <o> USART0_DCD Pin <0=>Not Used <1=>PD0
//   <i> Carrier Detect (DCD)
#define RTE_USART0_DCD_PORT_ID            0
#if    (RTE_USART0_DCD_PORT_ID == 0)
#define RTE_USART0_DCD_PORT               NULL
#elif  (RTE_USART0_DCD_PORT_ID == 1)
#define RTE_USART0_DCD_PORT               {PIO_PD0, PIOD, ID_PIOD, PIO_PERIPH_D, PIO_DEFAULT}
#else
#error "Invalid USART0_DCD Pin Configuration!"
#endif

//   <o> USART0_DSR Pin <0=>Not Used <1=>PD2
//   <i> Data Set Ready (DSR)
#define RTE_USART0_DSR_PORT_ID            0
#if    (RTE_USART0_DSR_PORT_ID == 0)
#define RTE_USART0_DSR_PORT               NULL
#elif  (RTE_USART0_DSR_PORT_ID == 1)
#define RTE_USART0_DSR_PORT               {PIO_PD2, PIOD, ID_PIOD, PIO_PERIPH_D, PIO_DEFAULT}
#else
#error "Invalid USART0_DSR Pin Configuration!"
#endif

//   <o> USART0_DTR Pin <0=>Not Used <1=>PD1
//   <i> Data Terminal Ready (DTR)
#define RTE_USART0_DTR_PORT_ID            0
#if    (RTE_USART0_DTR_PORT_ID == 0)
#define RTE_USART0_DTR_PORT               NULL
#elif  (RTE_USART0_DTR_PORT_ID == 1)
#define RTE_USART0_DTR_PORT               {PIO_PD1, PIOD, ID_PIOD, PIO_PERIPH_D, PIO_DEFAULT}
#else
#error "Invalid USART0_DTR Pin Configuration!"
#endif

//   <o> USART0_RI Pin <0=>Not Used <1=>PD3
//   <i> Ring Indicator (RI) 
#define RTE_USART0_RI_PORT_ID            0
#if    (RTE_USART0_RI_PORT_ID == 0)
#define RTE_USART0_RI_PORT               NULL
#elif  (RTE_USART0_RI_PORT_ID == 1)
#define RTE_USART0_RI_PORT               {PIO_PD3, PIOD, ID_PIOD, PIO_PERIPH_D, PIO_DEFAULT}
#else
#error "Invalid USART0_RI Pin Configuration!"
#endif


#if (RTE_USART0==1)
#if (RTE_USART0_DMA_RX==1)&(RTE_USART0_DMA_TX==0)
#error "USART DMA RX Channel requires USART DMA TX Channel enabled!"
#endif
#if (RTE_USART0_DMA_TX==1)&(RTE_USART0_DMA_RX==0)
#warning "USART DMA RX Channel is disabled, only USART Send function can be used!"
#endif
#endif

// </e>

// <e> USART1 (Universal Synchronous Asynchonous Receive Transmitter 1) [Driver_USART1]
// <i> Configuration settings for Driver_USART1 in component ::CMSIS Driver:USART
#define RTE_USART1                       0

//   <o> USART1 TX Operating Mode <0=> Polling <1=> Interrupt <2=> DMA
//   <i> Configuration settings for the USART1 peripheral
#define RTE_USART1_TX_MODE               0
#if   RTE_USART1_TX_MODE == 0
#define RTE_USART1_POLLING_TX
#elif RTE_USART1_TX_MODE == 1
#define RTE_USART1_IRQ_TX
#elif RTE_USART1_TX_MODE == 2
#define RTE_USART1_DMA_TX                1
#endif

//   <o> USART1 RX Operating Mode <0=> Polling <1=> Interrupt <2=> DMA
//   <i> Configuration settings for the USART1 peripheral
#define RTE_USART1_RX_MODE               0
#if   RTE_USART1_RX_MODE == 0
#define RTE_USART1_POLLING_RX
#elif RTE_USART1_RX_MODE == 1
#define RTE_USART1_IRQ_RX
#elif RTE_USART1_RX_MODE == 2
#define RTE_USART1_DMA_RX                1
#endif

//   <o> USART1_RX Pin <0=>Not Used <1=>PA21
//   <i> RX (RX) 
//   <i> This data line recieves the data.
#define RTE_USART1_RX_PORT_ID            1
#if    (RTE_USART1_RX_PORT_ID == 0)
#define RTE_USART1_RX_PORT               NULL
#elif  (RTE_USART1_RX_PORT_ID == 1)           
#define RTE_USART1_RX_PORT               {PIO_PA21, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid USART1_RX Pin Configuration!"
#endif

//   <o> USART1_TX Pin <0=>Not Used <1=>PB4
//   <i> TX (TX)
//   <i> This data line supplies the output data.
#define RTE_USART1_TX_PORT_ID            1
#if    (RTE_USART1_TX_PORT_ID == 0)
#define RTE_USART1_TX_PORT               NULL 
#elif  (RTE_USART1_TX_PORT_ID == 1)
#define RTE_USART1_TX_PORT               {PIO_PB4, PIOB, ID_PIOB, PIO_PERIPH_D, PIO_DEFAULT}
#else
#error "Invalid USART1_TX Pin Configuration!"
#endif

//   <o> USART1_SCK Pin <0=>Not Used <1=>PA23
//   <i> Serial Clock (SCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_USART1_SCK_PORT_ID           0
#if    (RTE_USART1_SCK_PORT_ID == 0)
#define RTE_USART1_SCK_PORT              NULL
#elif  (RTE_USART1_SCK_PORT_ID == 1)
#define RTE_USART1_SCK_PORT              {PIO_PA23, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid USART1_SPCK Pin Configuration!"
#endif

//   <o> USART1_RTS Pin <0=>Not Used <1=>PA24
//   <i> Request to Send (SCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_USART1_RTS_PORT_ID           0
#if    (RTE_USART1_RTS_PORT_ID == 0)
#define RTE_USART1_RTS_PORT              NULL
#elif  (RTE_USART1_RTS_PORT_ID == 1)
#define RTE_USART1_RTS_PORT              {PIO_PA24, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid USART1_RTS Pin Configuration!"
#endif

//   <o> USART1_CTS Pin <0=>Not Used <1=>PA25
//   <i> Request to Send (SCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_USART1_CTS_PORT_ID           0
#if    (RTE_USART1_CTS_PORT_ID == 0)
#define RTE_USART1_CTS_PORT              NULL
#elif  (RTE_USART1_CTS_PORT_ID == 1)
#define RTE_USART1_CTS_PORT              {PIO_PA25, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid USART1_CTS Pin Configuration!"
#endif

//   <o> USART1_DCD Pin <0=>Not Used <1=>PA26
//   <i> Carrier Detect (DCD)
#define RTE_USART1_DCD_PORT_ID           0
#if    (RTE_USART1_DCD_PORT_ID == 0)
#define RTE_USART1_DCD_PORT              NULL
#elif  (RTE_USART1_DCD_PORT_ID == 1)
#define RTE_USART1_DCD_PORT              {PIO_PA26, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid USART1_DCD Pin Configuration!"
#endif

//   <o> USART1_DSR Pin <0=>Not Used <1=>PA28
//   <i> Data Set Ready (DSR)
#define RTE_USART1_DSR_PORT_ID           0
#if    (RTE_USART1_DSR_PORT_ID == 0)
#define RTE_USART1_DSR_PORT              NULL
#elif  (RTE_USART1_DSR_PORT_ID == 1)
#define RTE_USART1_DSR_PORT              {PIO_PA28, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid USART1_DSR Pin Configuration!"
#endif

//   <o> USART1_DTR Pin <0=>Not Used <1=>PA27
//   <i> Data Terminal Ready (DTR)
#define RTE_USART1_DTR_PORT_ID           0
#if    (RTE_USART1_DTR_PORT_ID == 0)
#define RTE_USART1_DTR_PORT              NULL
#elif  (RTE_USART1_DTR_PORT_ID == 1)
#define RTE_USART1_DTR_PORT              {PIO_PA27, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid USART1_DTR Pin Configuration!"
#endif

//   <o> USART1_RI Pin <0=>Not Used <1=>PA29
//   <i> Ring Indicator (RI) 
#define RTE_USART1_RI_PORT_ID            0
#if    (RTE_USART1_RI_PORT_ID == 0)
#define RTE_USART1_RI_PORT               NULL
#elif  (RTE_USART1_RI_PORT_ID == 1)
#define RTE_USART1_RI_PORT               {PIO_PA29, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid USART1_RI Pin Configuration!"
#endif

//   <o> USART1_LONCOL Pin <0=>Not Used <1=>PA3
//   <i> LONCOL
#define RTE_USART1_RI_PORT_ID            0
#if    (RTE_USART1_RI_PORT_ID == 0)
#define RTE_USART1_RI_PORT               NULL
#elif  (RTE_USART1_RI_PORT_ID == 1)
#define RTE_USART1_RI_PORT               {PIO_PA3, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid USART1_RI Pin Configuration!"
#endif

#if (RTE_USART1==1)
#if (RTE_USART1_DMA_RX==1)&(RTE_USART1_DMA_TX==0)
#error "USART DMA RX Channel requires USART DMA TX Channel enabled!"
#endif
#if (RTE_USART1_DMA_TX==1)&(RTE_USART1_DMA_RX==0)
#warning "USART DMA RX Channel is disabled, only USART Send function can be used!"
#endif
#endif

// </e>

// <e> USART2 (Universal Synchronous Asynchonous Receive Transmitter 1) [Driver_USART2]
// <i> Configuration settings for Driver_USART2 in component ::CMSIS Driver:USART
#define RTE_USART2                       0

//   <o> USART2 TX Operating Mode <0=> Polling <1=> Interrupt <2=> DMA
//   <i> Configuration settings for the USART2 peripheral
#define RTE_USART2_TX_MODE               0
#if   RTE_USART2_TX_MODE == 0
#define RTE_USART2_POLLING_TX
#elif RTE_USART2_TX_MODE == 1
#define RTE_USART2_IRQ_TX
#elif RTE_USART2_TX_MODE == 2
#define RTE_USART2_DMA_TX                1
#endif

//   <o> USART2 RX Operating Mode <0=> Polling <1=> Interrupt <2=> DMA
//   <i> Configuration settings for the USART2 peripheral
#define RTE_USART2_RX_MODE               0
#if   RTE_USART2_RX_MODE == 0
#define RTE_USART2_POLLING_RX
#elif RTE_USART2_RX_MODE == 1
#define RTE_USART2_IRQ_RX
#elif RTE_USART2_RX_MODE == 2
#define RTE_USART2_DMA_RX                1
#endif

//   <o> USART2_RX Pin <0=>Not Used <1=>PD15
//   <i> RX (RX) 
//   <i> This data line recieves the data.
#define RTE_USART2_RX_PORT_ID            1
#if    (RTE_USART2_RX_PORT_ID == 0)
#define RTE_USART2_RX_PORT               NULL
#elif  (RTE_USART2_RX_PORT_ID == 1)           
#define RTE_USART2_RX_PORT               {PIO_PD15, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid USART2_RX Pin Configuration!"
#endif

//   <o> USART2_TX Pin <0=>Not Used <1=>PD16
//   <i> TX (TX)
//   <i> This data line supplies the output data.
#define RTE_USART2_TX_PORT_ID            1
#if    (RTE_USART2_TX_PORT_ID == 0)
#define RTE_USART2_TX_PORT               NULL 
#elif  (RTE_USART2_TX_PORT_ID == 1)
#define RTE_USART2_TX_PORT               {PIO_PD16, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid USART2_TX Pin Configuration!"
#endif

//   <o> USART2_SCK Pin <0=>Not Used <1=>PD17
//   <i> Serial Clock (SCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_USART2_SCK_PORT_ID           0
#if    (RTE_USART2_SCK_PORT_ID == 0)
#define RTE_USART2_SCK_PORT              NULL
#elif  (RTE_USART2_SCK_PORT_ID == 1)
#define RTE_USART2_SCK_PORT              {PIO_PD17, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid USART2_SPCK Pin Configuration!"
#endif

//   <o> USART2_RTS Pin <0=>Not Used <1=>PD18
//   <i> Request to Send (SCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_USART2_RTS_PORT_ID           0
#if    (RTE_USART2_RTS_PORT_ID == 0)
#define RTE_USART2_RTS_PORT              NULL
#elif  (RTE_USART2_RTS_PORT_ID == 1)
#define RTE_USART2_RTS_PORT              {PIO_PD18, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid USART2_RTS Pin Configuration!"
#endif

//   <o> USART2_CTS Pin <0=>Not Used <1=>PD19
//   <i> Request to Send (SCK) 
//   <i> This control line is driven by the master and regulates the flow of the data bits.
#define RTE_USART2_CTS_PORT_ID           0
#if    (RTE_USART2_CTS_PORT_ID == 0)
#define RTE_USART2_CTS_PORT              NULL
#elif  (RTE_USART2_CTS_PORT_ID == 1)
#define RTE_USART2_CTS_PORT              {PIO_PD19, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid USART2_CTS Pin Configuration!"
#endif

//   <o> USART2_DCD Pin <0=>Not Used <1=>PD4
//   <i> Carrier Detect (DCD)
#define RTE_USART2_DCD_PORT_ID           0
#if    (RTE_USART2_DCD_PORT_ID == 0)
#define RTE_USART2_DCD_PORT              NULL
#elif  (RTE_USART2_DCD_PORT_ID == 1)
#define RTE_USART2_DCD_PORT              {PIO_PD4, PIOD, ID_PIOD, PIO_PERIPH_D, PIO_DEFAULT}
#else
#error "Invalid USART2_DCD Pin Configuration!"
#endif

//   <o> USART2_DSR Pin <0=>Not Used <1=>PD6
//   <i> Data Set Ready (DSR)
#define RTE_USART2_DSR_PORT_ID           0
#if    (RTE_USART2_DSR_PORT_ID == 0)
#define RTE_USART2_DSR_PORT              NULL
#elif  (RTE_USART2_DSR_PORT_ID == 1)
#define RTE_USART2_DSR_PORT              {PIO_PD6, PIOD, ID_PIOD, PIO_PERIPH_D, PIO_DEFAULT}
#else
#error "Invalid USART2_DSR Pin Configuration!"
#endif

//   <o> USART2_DTR Pin <0=>Not Used <1=>PD5
//   <i> Data Terminal Ready (DTR)
#define RTE_USART2_DTR_PORT_ID           0
#if    (RTE_USART2_DTR_PORT_ID == 0)
#define RTE_USART2_DTR_PORT              NULL
#elif  (RTE_USART2_DTR_PORT_ID == 1)
#define RTE_USART2_DTR_PORT              {PIO_PD5, PIOD, ID_PIOD, PIO_PERIPH_D, PIO_DEFAULT}
#else
#error "Invalid USART2_DTR Pin Configuration!"
#endif

//   <o> USART2_RI Pin <0=>Not Used <1=>PD7
//   <i> Ring Indicator (RI) 
#define RTE_USART2_RI_PORT_ID            0
#if    (RTE_USART2_RI_PORT_ID == 0)
#define RTE_USART2_RI_PORT               NULL
#elif  (RTE_USART2_RI_PORT_ID == 1)
#define RTE_USART2_RI_PORT               {PIO_PD7, PIOD, ID_PIOD, PIO_PERIPH_D, PIO_DEFAULT}
#else
#error "Invalid USART2_RI Pin Configuration!"
#endif

#if (RTE_USART2==1)
#if (RTE_USART2_DMA_RX==1)&(RTE_USART2_DMA_TX==0)
#error "USART DMA RX Channel requires USART DMA TX Channel enabled!"
#endif
#if (RTE_USART2_DMA_TX==1)&(RTE_USART2_DMA_RX==0)
#warning "USART DMA RX Channel is disabled, only USART Send function can be used!"
#endif
#endif

// </e>

// <e> USBD (USB Device Interface) [Driver_USBD0]
// <i> Configuration settings for Driver_USBD0 in component ::CMSIS Driver:USB Device
#define RTE_USBD                         1

//   <o>USB Speed <0=>Full <1=>High
//   <i> USB Speed (Full/High)
#define RTE_USBD_HIGH_SPEED              1

// </e>

// <e> USBH (USB Host Interface) [Driver_USBH0]
// <i> Configuration settings for Driver_USBH0 in component ::CMSIS Driver:USB Host
#define RTE_USBH                         0
#if ((RTE_USBH == 1)&&(RTE_USBD == 1))
#error "USB Host and USB Device cannot be used at the same time!"
#endif

//   <o>VBUS Enable Pin <0=>Not Used <1=>PC16
//   <i> VBUS Power On/Off Pin
#define RTE_USBH_VBUS_PORT_ID            1
#if    (RTE_USBH_VBUS_PORT_ID == 0)
#define RTE_USBH_VBUS_PORT               NULL
#elif  (RTE_USBH_VBUS_PORT_ID == 1)
#define RTE_USBH_VBUS_PORT               {PIO_PC16, PIOC, ID_PIOC, PIO_OUTPUT_1, PIO_DEFAULT}
#else
#error "Invalid USB Host VBUS Pin Configuration!"
#endif

//   <o>USB Speed <0=>Full <1=>High
//   <i> USB Speed (Full/High)
#define RTE_USBH_HIGH_SPEED              0             

// </e>

// <e> CAN0 (Controller Area Network) [Driver_CAN0]
// <i> Configuration settings for Driver_CAN0 in component ::CMSIS Driver:CAN
#define RTE_CAN0                         1

//   <o> CAN0 RX Pin <0=>Not Used <1=>PB3
//   <i> CAN 0 receive pin configuration
#define RTE_CAN0_RX_PIN_ID               1
#if    (RTE_CAN0_RX_PIN_ID == 0)
#define RTE_CAN0_RX_PIN                  NULL
#elif  (RTE_CAN0_RX_PIN_ID == 1)
#define RTE_CAN0_RX_PIN                  {PIO_PB3, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid CAN 0 Receive Pin Configuration!"
#endif

//   <o> CAN0 TX Pin <0=>Not Used <1=>PB2
//   <i> CAN 0 transmit pin configuration
#define RTE_CAN0_TX_PIN_ID               1
#if    (RTE_CAN0_TX_PIN_ID == 0)
#define RTE_CAN0_TX_PIN                  NULL
#elif  (RTE_CAN0_TX_PIN_ID == 1)
#define RTE_CAN0_TX_PIN                  {PIO_PB2, PIOB, ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#else
#error "Invalid CAN 0 Transmit Pin Configuration!"
#endif

// </e>

// <e> CAN1 (Controller Area Network) [Driver_CAN1]
// <i> Configuration settings for Driver_CAN1 in component ::CMSIS Driver:CAN
#define RTE_CAN1                         0

//   <o> CAN1 RX Pin <0=>Not Used <1=>PC12 <2=>PD28
//   <i> CAN 1 receive pin configuration
#define RTE_CAN1_RX_PIN_ID               1
#if    (RTE_CAN1_RX_PIN_ID == 0)
#define RTE_CAN1_RX_PIN                  NULL
#elif  (RTE_CAN1_RX_PIN_ID == 1)
#define RTE_CAN1_RX_PIN                  {PIO_PC12, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
#elif  (RTE_CAN1_RX_PIN_ID == 2)
#define RTE_CAN1_RX_PIN                  {PIO_PD28, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid CAN 1 Receive Pin Configuration!"
#endif

//   <o> CAN1 TX Pin <0=>Not Used <1=>PC14 <2=>PD12
//   <i> CAN 1 transmit pin configuration
#define RTE_CAN1_TX_PIN_ID               1
#if    (RTE_CAN1_TX_PIN_ID == 0)
#define RTE_CAN1_TX_PIN                  NULL
#elif  (RTE_CAN1_TX_PIN_ID == 1)
#define RTE_CAN1_TX_PIN                  {PIO_PC14, PIOC, ID_PIOC, PIO_PERIPH_C, PIO_DEFAULT}
#elif  (RTE_CAN1_TX_PIN_ID == 2)
#define RTE_CAN1_TX_PIN                  {PIO_PD12, PIOD, ID_PIOD, PIO_PERIPH_B, PIO_DEFAULT}
#else
#error "Invalid CAN 1 Transmit Pin Configuration!"
#endif

// </e>

#endif  /* __RTE_DEVICE_H */
