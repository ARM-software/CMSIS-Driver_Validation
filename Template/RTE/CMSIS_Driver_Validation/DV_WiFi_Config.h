/*
 * Copyright (c) 2015-2025 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * -----------------------------------------------------------------------------
 *
 * $Revision:   V1.1.0
 *
 * Project:     CMSIS-Driver Validation
 * Title:       WiFi driver validation configuration file
 *
 * -----------------------------------------------------------------------------
 */

#ifndef DV_WIFI_CONFIG_H_
#define DV_WIFI_CONFIG_H_

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
//------ With VS Code: Open Preview for Configuration Wizard -------------------

// <h> WiFi
// <i> WiFi driver validation configuration
// <o> Driver_WiFi# <0-255>
// <i> Choose the Driver_WiFi# instance to test.
// <i> For example to test Driver_WiFi0 select 0.
#define DRV_WIFI                        0
// <h> Configuration
// <i> Configuration of valid settings for driver functionality testing
// <h> Station
// <i> Settings relevant for Station
// <s.32> SSID
// <i> SSID of network that Station will connect to
#define WIFI_STA_SSID                   "SSID"
// <s.32> Password
// <i> Password of network that Station will connect to
#define WIFI_STA_PASS                   "Password"
// <o> Security Type
// <0=> Open <1=> WEP <2=> WPA <3=> WPA2
// <i> Security type of network that Station will connect to
#define WIFI_STA_SECURITY               3
// <o> Channel
// <i> WiFi channel of network that Station will connect to
// <i> Value 0 means Autodetect
#define WIFI_STA_CH                     0
// <s.8> WPS PIN
// <i> WiFi Protected Setup Personal Identification Number
#define WIFI_STA_WPS_PIN                "12345678"
// </h>
// <h> Access Point
// <i> Settings relevant for Access Point
// <s.32> SSID
// <i> SSID of created network
#define WIFI_AP_SSID                    "CMSIS_DV"
// <s.32> Password
// <i> Password of created network
#define WIFI_AP_PASS                    "Password"
// <o> Security Type
// <0=> Open <1=> WEP <2=> WPA <3=> WPA2
// <i> Security type of created network
#define WIFI_AP_SECURITY                3
// <o> Channel
// <i> WiFi channel of created network
// <i> Value 0 means Autoselect
#define WIFI_AP_CH                      0
// <s.8> WPS PIN
// <i> WiFi Protected Setup Personal Identification Number
#define WIFI_AP_WPS_PIN                 "12345678"
// </h>
// <h> Socket
// <i> Settings relevant for Socket testing
// <s.15>SockServer IP
// <i>Static IPv4 Address of SockServer
#define WIFI_SOCKET_SERVER_IP           "192.168.1.10"
// <o> Number of sockets
// <i> Number of sockets that driver supports
// <i> Default: 4
#define WIFI_SOCKET_MAX_NUM             4
// </h>
// </h>
// <h> Tests
// <i> Enable / disable tests.
// <e> Control
// <i> Control functions tests
#define WIFI_CONTROL_EN                 1
// <q> WIFI_GetVersion
#define WIFI_GETVERSION_EN              1
// <q> WIFI_GetCapabilities
#define WIFI_GETCAPABILITIES_EN         1
// <q> WIFI_Initialization/Uninitialization
#define WIFI_INIT_UNINIT_EN             1
// <q> WIFI_PowerControl
#define WIFI_POWERCONTROL_EN            1
// <q> WIFI_GetModuleInfo
#define WIFI_GETMODULEINFO_EN           1
// </e>
// <e> Management
// <i> Management functions tests
#define WIFI_MANAGEMENT_EN              1
// <e> WIFI_SetOption/GetOption
#define WIFI_SETGETOPTION_EN            1
// <o0> ARM_WIFI_BSSID <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_BSSID_STA/WIFI_BSSID_AP (1E-30-6C-A2-45-5E)
#define WIFI_SETGETOPTION_BSSID_EN      3
// <o0> ARM_WIFI_TX_POWER <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_TX_POWER_STA/WIFI_TX_POWER_AP (16)
#define WIFI_SETGETOPTION_TX_POWER_EN   3
// <o0> ARM_WIFI_LP_TIMER <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_LP_TIMER_STA (10)
#define WIFI_SETGETOPTION_LP_TIMER_EN   3
// <o0> ARM_WIFI_DTIM <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_DTIM_STA/WIFI_DTIM_AP (3)
#define WIFI_SETGETOPTION_DTIM_EN       3
// <o0> ARM_WIFI_BEACON <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_BEACON_AP (2000)
#define WIFI_SETGETOPTION_BEACON_EN     3
// <o0> ARM_WIFI_MAC <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_MAC_STA/WIFI_MAC_AP (1E-30-6C-A2-45-5E)
#define WIFI_SETGETOPTION_MAC_EN        3
// <o0> ARM_WIFI_IP <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_IP_STA/WIFI_IP_AP (192.168.0.100)
#define WIFI_SETGETOPTION_IP_EN         3
// <o0> ARM_WIFI_IP_SUBNET_MASK <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_IP_SUBNET_MASK_STA/WIFI_IP_SUBNET_MASK_AP (255.255.255.0)
#define WIFI_SETGETOPTION_IP_SUBNET_MASK_EN 3
// <o0> ARM_WIFI_IP_GATEWAY <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_IP_GATEWAY_STA/WIFI_IP_GATEWAY_AP (192.168.0.254)
#define WIFI_SETGETOPTION_IP_GATEWAY_EN 3
// <o0> ARM_WIFI_IP_DNS1 <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_IP_DNS1_STA/WIFI_IP_DNS1_AP (8.8.8.8)
#define WIFI_SETGETOPTION_IP_DNS1_EN    3
// <o0> ARM_WIFI_IP_DNS2 <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_IP_DNS2_STA/WIFI_IP_DNS2_AP (8.8.4.4)
#define WIFI_SETGETOPTION_IP_DNS2_EN    3
// <o0> ARM_WIFI_IP_DHCP <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
#define WIFI_SETGETOPTION_IP_DHCP_EN    3
// <o0> ARM_WIFI_IP_DHCP_POOL_BEGIN <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_IP_DHCP_POOL_BEGIN_AP (192.168.0.100)
#define WIFI_SETGETOPTION_IP_DHCP_POOL_BEGIN_EN 3
// <o0> ARM_WIFI_IP_DHCP_POOL_END <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_IP_DHCP_POOL_END_AP (192.168.0.200)
#define WIFI_SETGETOPTION_IP_DHCP_POOL_END_EN   3
// <o0> ARM_WIFI_IP_DHCP_LEASE_TIME <0=>Disabled <1=>Set <2=>Get <3=>Set/Get
// <i> Default test value is set in defines WIFI_IP_DHCP_LEASE_TIME_AP (3600)
#define WIFI_SETGETOPTION_IP_DHCP_LEASE_TIME_EN 3
// </e>
// <q> WIFI_Scan
#define WIFI_SCAN_EN                    1
#define WIFI_SCAN_MAX_NUM               10
// <q> WIFI_Activate/Deactivate
#define WIFI_ACT_DEACT_EN               1
// <q> WIFI_IsConnected
#define WIFI_ISCONNECTED_EN             1
// <q> WIFI_GetNetInfo
#define WIFI_GETNETINFO_EN              1
// </e>
// <e> Management (requires user interaction)
// <i> Management functions tests that require user interaction
#define WIFI_MANAGEMENT_USER_EN         0
// <q> WIFI_Activate_AP
// <i> For this test please connect a WiFi client (for example mobile phone) to AP
// <i> and check if connection has succeeded
#define WIFI_ACT_AP                     1
// <e> WPS
#define WIFI_WPS_USER_EN                1
// <q> WIFI_Activate_Station_WPS_PBC
// <i> For this test please start WPS Push-button method on WiFi AP (router) manually
#define WIFI_ACT_STA_WPS_PBC            1
// <q> WIFI_Activate_Station_WPS_PIN
// <i> For this test please configure WPS PIN method on WiFi AP (router) manually
#define WIFI_ACT_STA_WPS_PIN            1
// <q> WIFI_Activate_AP_WPS_PBC
// <i> For this test please connect a WiFi client (for example mobile phone) to AP with WPS Push-button method
// <i> and check if connection has succeeded
#define WIFI_ACT_AP_WPS_PBC             1
// <q> WIFI_Activate_AP_WPS_PIN
// <i> For this test please connect a WiFi client (for example mobile phone) to AP with WPS PIN method
// <i> and check if connection has succeeded
#define WIFI_ACT_AP_WPS_PIN             1
// </e>
// </e>
// <e> Socket API (requires SockServer)
// <i> Socket functions tests
#define WIFI_SOCKET_EN                  1
// <q> WIFI_SocketCreate
#define WIFI_SOCKETCREATE_EN            1
// <q> WIFI_SocketBind
#define WIFI_SOCKETBIND_EN              1
// <q> WIFI_SocketListen
#define WIFI_SOCKETLISTEN_EN            1
// <q> WIFI_SocketAccept
#define WIFI_SOCKETACCEPT_EN            1
// <q> WIFI_SocketAccept_nbio
#define WIFI_SOCKETACCEPT_NBIO_EN       1
// <q> WIFI_SocketConnect
#define WIFI_SOCKETCONNECT_EN           1
// <q> WIFI_SocketConnect_nbio
#define WIFI_SOCKETCONNECT_NBIO_EN      1
// <q> WIFI_SocketRecv
#define WIFI_SOCKETRECV_EN              1
// <q> WIFI_SocketRecv_nbio
#define WIFI_SOCKETRECV_NBIO_EN         1
// <q> WIFI_SocketRecvFrom
#define WIFI_SOCKETRECVFROM_EN          1
// <q> WIFI_SocketRecvFrom_nbio
#define WIFI_SOCKETRECVFROM_NBIO_EN     1
// <q> WIFI_SocketSend
#define WIFI_SOCKETSEND_EN              1
// <q> WIFI_SocketSendTo
#define WIFI_SOCKETSENDTO_EN            1
// <q> WIFI_SocketGetSockName
#define WIFI_SOCKETGETSOCKNAME_EN       1
// <q> WIFI_SocketGetPeerName
#define WIFI_SOCKETGETPEERNAME_EN       1
// <q> WIFI_SocketGetOpt
#define WIFI_SOCKETGETOPT_EN            1
// <q> WIFI_SocketSetOpt
#define WIFI_SOCKETSETOPT_EN            1
// <q> WIFI_SocketClose
#define WIFI_SOCKETCLOSE_EN             1
// <q> WIFI_SocketGetHostByName
#define WIFI_SOCKETGETHOSTBYNAME_EN     1
// <q> WIFI_Ping
#define WIFI_PING_EN                    1
// </e>
// <e> Socket Operation (requires SockServer)
// <i> Socket operation tests
#define WIFI_SOCKET_OP_EN               1
// <q> WIFI_Transfer_Fixed
// <i> Sends and receives in fixed size blocks
#define WIFI_TRANSFER_FIXED_EN          1
// <q> WIFI_Transfer_Incremental
// <i> Sends and receives in ascending size blocks
#define WIFI_TRANSFER_INCREMENTAL_EN    1
// <q> WIFI_Send_Fragmented
// <i> Sends several smaller blocks, receives one large block
#define WIFI_SEND_FRAGMENTED_EN         1
// <q> WIFI_Recv_Fragmented
// <i> Sends one large block, receives several smaller blocks
#define WIFI_RECV_FRAGMENTED_EN         1
// <q> WIFI_Test_Speed
// <i> Transmits data and measures transfer speed
#define WIFI_TEST_SPEED_EN              1
// <q> WIFI_Concurrent_Socket
// <i> Transmits data in two sockets simultaneously
#define WIFI_CONCURRENT_SOCKET_EN       1
// <q> WIFI_Downstream_Rate
// <i> Measures the downstream bandwidth
#define WIFI_DOWNSTREAM_RATE_EN         1
// <q> WIFI_Upstream_Rate
// <i> Measures the upstream bandwidth
#define WIFI_UPSTREAM_RATE_EN           1
// </e>
// </h>
// </h>

// Configuration settings of test values for Set/GetOptions
// BSSID
#define WIFI_BSSID_STA                  "1E-30-6C-A2-45-5E"
#define WIFI_BSSID_AP                   "1E-30-6C-A2-45-5E"
// Radio transmitting power in dBm
#define WIFI_TX_POWER_STA               16
#define WIFI_TX_POWER_AP                16
// Low-power deep-sleep time value in seconds
#define WIFI_LP_TIMER_STA               10
// Delivery Traffic Indication Message interval value in beacons
#define WIFI_DTIM_STA                   3
#define WIFI_DTIM_AP                    3
// Beacon interval value in milliseconds
#define WIFI_BEACON_AP                  2000
// Ethernet MAC Address in text representation
#define WIFI_MAC_STA                    "1E-30-6C-A2-45-5E"
#define WIFI_MAC_AP                     "1E-30-6C-A2-45-5E"
// Static IPv4 Address in text representation
#define WIFI_IP_STA                     "192.168.0.100"
#define WIFI_IP_AP                      "192.168.0.100"
// Local Subnet mask in text representation
#define WIFI_IP_SUBNET_MASK_STA         "255.255.255.0"
#define WIFI_IP_SUBNET_MASK_AP          "255.255.255.0"
// IP Address of Default Gateway in text representation
#define WIFI_IP_GATEWAY_STA             "192.168.0.254"
#define WIFI_IP_GATEWAY_AP              "192.168.0.254"
// IP Address of Primary DNS Server in text representation
#define WIFI_IP_DNS1_STA                "8.8.8.8"
#define WIFI_IP_DNS1_AP                 "8.8.8.8"
// IP Address of Secondary DNS Server in text representation
#define WIFI_IP_DNS2_STA                "8.8.4.4"
#define WIFI_IP_DNS2_AP                 "8.8.4.4"
// IP Address of DHCP server pool beginning in text representation
#define WIFI_IP_DHCP_POOL_BEGIN_AP      "192.168.0.100"
// IP Address of DHCP server pool end in text representation
#define WIFI_IP_DHCP_POOL_END_AP        "192.168.0.200"
// DHCP lease time value in seconds
#define WIFI_IP_DHCP_LEASE_TIME_AP      3600

// Timeout configuration that can be used for adjusting tests to limitations of the WiFi Module (in ms)
#define WIFI_SOCKET_TIMEOUT             2000
#define WIFI_SOCKET_TIMEOUT_LONG        35000

// Time that Access Point will wait for Client to connect to it (in ms)
#define WIFI_AP_CLIENT_CON_TIMEOUT      120000

#endif /* DV_WIFI_CONFIG_H_ */
