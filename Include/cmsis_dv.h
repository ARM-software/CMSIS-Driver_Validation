/*-----------------------------------------------------------------------------
 *      Name:         cmsis_dv.h 
 *      Purpose:      cmsis_dv header
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/
#ifndef __CMSIS_DV_H
#define __CMSIS_DV_H

#include <stdint.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#if defined(RTE_CMSIS_RTOS)
#include "cmsis_os.h"
#define GET_SYSTICK() osKernelSysTick()
#define SYSTICK_MICROSEC(microsec) (((uint64_t)microsec * (osKernelSysTickFrequency)) / 1000000)

#elif defined(RTE_CMSIS_RTOS2)
#include "cmsis_os2.h"
#define GET_SYSTICK() osKernelGetSysTimerCount()
#define SYSTICK_MICROSEC(microsec) (((uint64_t)microsec *  osKernelGetSysTimerFreq()) / 1000000)
#endif

/* Expansion macro used to create CMSIS Driver references */
#define EXPAND_SYMBOL(name, port) name##port
#define CREATE_SYMBOL(name, port) EXPAND_SYMBOL(name, port)

// Buffer list sizes
extern const uint32_t BUFFER[];
extern const uint32_t BUFFER_NUM;

// Test main function
extern void cmsis_dv (void *argument);

// Test cases
extern void SPI_GetCapabilities (void);
extern void SPI_Initialization (void);
extern void SPI_PowerControl (void);
extern void SPI_Config_PolarityPhase (void);
extern void SPI_Config_DataBits (void);
extern void SPI_Config_BitOrder (void);
extern void SPI_Config_SSMode (void);
extern void SPI_Config_CommonParams (void);
extern void SPI_Config_BusSpeed (void);
extern void SPI_Send (void);
extern void SPI_Receive (void);
extern void SPI_Loopback_CheckBusSpeed (void);
extern void SPI_Loopback_Transfer (void);
extern void SPI_CheckInvalidInit (void);

extern void USART_GetCapabilities (void);
extern void USART_Initialization (void);
extern void USART_PowerControl (void);
extern void USART_Config_PolarityPhase (void);
extern void USART_Config_DataBits (void);
extern void USART_Config_StopBits (void);
extern void USART_Config_Parity (void);
extern void USART_Config_CommonParams (void);
extern void USART_Config_Baudrate (void);
extern void USART_Send (void);
extern void USART_AsynchronousReceive (void);
extern void USART_Loopback_CheckBaudrate (void);
extern void USART_Loopback_Transfer (void);
extern void USART_CheckInvalidInit (void);

extern void ETH_MAC_GetCapabilities (void);
extern void ETH_MAC_Initialization (void);
extern void ETH_MAC_PowerControl (void);
extern void ETH_MAC_SetBusSpeed (void);
extern void ETH_MAC_Config_Mode (void);
extern void ETH_MAC_Config_CommonParams (void);
extern void ETH_MAC_PTP_ControlTimer (void);
extern void ETH_PHY_Initialization (void);
extern void ETH_PHY_PowerControl (void);
extern void ETH_PHY_Config (void);
extern void ETH_Loopback_Transfer (void);
extern void ETH_Loopback_PTP (void);
extern void ETH_PHY_CheckInvalidInit (void);
extern void ETH_MAC_CheckInvalidInit (void);

extern void I2C_GetCapabilities (void);
extern void I2C_Initialization (void);
extern void I2C_PowerControl (void);
extern void I2C_SetBusSpeed (void);
extern void I2C_SetOwnAddress (void);
extern void I2C_BusClear (void);
extern void I2C_AbortTransfer (void);
extern void I2C_CheckInvalidInit (void);

extern void MCI_GetCapabilities (void);
extern void MCI_Initialization (void);
extern void MCI_PowerControl (void);
extern void MCI_SetBusSpeedMode (void);
extern void MCI_Config_DataWidth (void);
extern void MCI_Config_CmdLineMode (void);
extern void MCI_Config_DriverStrength (void);
extern void MCI_CheckInvalidInit (void);

extern void USBD_GetCapabilities (void);
extern void USBD_Initialization (void);
extern void USBD_PowerControl (void);
extern void USBD_CheckInvalidInit (void);

extern void USBH_GetCapabilities (void);
extern void USBH_Initialization (void);
extern void USBH_PowerControl (void);
extern void USBH_CheckInvalidInit (void);

extern void CAN_GetCapabilities (void);
extern void CAN_Initialization (void);
extern void CAN_PowerControl (void);
extern void CAN_CheckInvalidInit (void);
extern void CAN_Loopback_CheckBitrate (void);
extern void CAN_Loopback_CheckBitrateFD (void);
extern void CAN_Loopback_Transfer (void);
extern void CAN_Loopback_TransferFD (void);

extern void WIFI_DV_Initialize (void);
extern void WIFI_DV_Uninitialize (void);

extern void WIFI_GetVersion (void);
extern void WIFI_GetCapabilities (void);
extern void WIFI_Initialize_Uninitialize (void);
extern void WIFI_PowerControl (void);
extern void WIFI_GetModuleInfo (void);

extern void WIFI_SetOption_GetOption (void);
extern void WIFI_Scan (void);
extern void WIFI_Configure (void);
extern void WIFI_Activate_Deactivate (void);
extern void WIFI_IsConnected (void);
extern void WIFI_GetNetInfo (void);

extern void WIFI_Activate_AP (void);
extern void WIFI_Activate_Station_WPS_PBC (void);
extern void WIFI_Activate_Station_WPS_PIN (void);
extern void WIFI_Activate_AP_WPS_PBC (void);
extern void WIFI_Activate_AP_WPS_PIN (void);

extern void WIFI_SocketCreate (void);
extern void WIFI_SocketBind (void);
extern void WIFI_SocketListen (void);
extern void WIFI_SocketAccept (void);
extern void WIFI_SocketConnect (void);
extern void WIFI_SocketRecv (void);
extern void WIFI_SocketRecvFrom (void);
extern void WIFI_SocketSend (void);
extern void WIFI_SocketSendTo (void);
extern void WIFI_SocketGetSockName (void);
extern void WIFI_SocketGetPeerName (void);
extern void WIFI_SocketGetOpt (void);
extern void WIFI_SocketSetOpt (void);
extern void WIFI_SocketClose (void);
extern void WIFI_SocketGetHostByName (void);
extern void WIFI_Ping (void);

extern void WIFI_Transfer_Fixed (void);
extern void WIFI_Transfer_Incremental (void);
extern void WIFI_Send_Fragmented (void);
extern void WIFI_Recv_Fragmented (void);
extern void WIFI_Test_Speed (void);
extern void WIFI_Concurrent_Socket (void);
extern void WIFI_Downstream_Rate (void);
extern void WIFI_Upstream_Rate (void);

#endif /* __CMSIS_DV_H */
