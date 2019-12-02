/*-----------------------------------------------------------------------------
 *      Name:         DV_WIFI.c
 *      Purpose:      WiFi driver test cases
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/

/*
  Known limitations:
  - Bypass mode and functionality is not tested
  - Set/GetOption API does not test IPv6 options
  - SetOption operation is not tested, only API is tested
    (BSSID, MAC, static IP operation testing would require dedicated hardware
     with manual check of results on dedicated hardware)
  - WPS operation is not tested (not Station nor AP)
    (WPS functional testing would require dedicated hardware
     with manual check of results WPS AP, WPS on Station could
     be checked by comparing parameters with expected result (configured))
  - WiFi sockets tested in blocking mode only
  - WiFi sockets not tested for IPv6
*/

#include "cmsis_dv.h"
#include "DV_Config.h"
#include "DV_Framework.h"
#include "Driver_WiFi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Service ports */
#define ECHO_PORT               7               // Echo port number
#define DISCARD_PORT            9               // Discard port number
#define CHARGEN_PORT            19              // Chargen port number
#define ASSISTANT_PORT          5000            // Test Assistant port number
#define TCP_REJECTED_PORT       5001            // Rejected connection server TCP port
#define TCP_TIMEOUT_PORT        5002            // Non-responding server TCP port

/* Helper function identifiers */
#define F_CREATE                0x00000001
#define F_CREATE_TCP            F_CREATE        // Never used with CREATE
#define F_CREATE_UDP            0x00000002
#define F_CLOSE                 0x00000004
#define F_BIND                  0x00000008
#define F_LISTEN                0x00000010
#define F_ACCEPT                0x00000020
#define F_CONNECT               0x00000040
#define F_RECV                  0x00000080
#define F_RECVFROM              0x00000100
#define F_SEND                  0x00000200
#define F_SENDTO                0x00000400
#define F_GETSOCKNAME           0x00000800
#define F_GETPEERNAME           0x00001000
#define F_GETOPT                0x00002000
#define F_SETOPT                0x00004000
#define F_GETHOSTBYNAME         0x00008000
#define F_PING                  0x00010000
#define F_SEND_CTRL             F_PING          // Never used with PING
#define F_XFER_FIXED            0x00020000
#define F_XFER_INCR             0x00040000
#define F_SEND_FRAG             0x00080000
#define F_UPLOAD                F_SEND_FRAG     // Never used with SEND_FRAG
#define F_RECV_FRAG             0x00100000
#define F_DOWNLOAD              F_RECV_FRAG     // Never used with RECV_FRAG
#define F_ALL                   0x001FFFFF

#define SK_TERMINATE            0x00000001

/* Helper function return values */
#define TH_OK                   0x01
#define TH_TOUT                 0x02
#define TH_ALL                  0x03

/* Register Driver_WiFi# */
extern ARM_DRIVER_WIFI         ARM_Driver_WiFi_(DRV_WIFI);
static ARM_DRIVER_WIFI* drv = &ARM_Driver_WiFi_(DRV_WIFI);

/* Local variables */
static uint8_t                  powered   = 0U;
static uint8_t                  connected = 0U;
static uint8_t                  socket_funcs_exist = 0U;
static uint8_t volatile         event;

static char                     msg_buf [128];
static char                     data_buf[128] __ALIGNED(4);

static ARM_WIFI_SignalEvent_t   event_func;
static ARM_WIFI_CAPABILITIES    cap;
static ARM_WIFI_CONFIG_t        config;
static ARM_WIFI_NET_INFO_t      net_info;
static ARM_WIFI_SCAN_INFO_t     scan_info[WIFI_SCAN_MAX_NUM];

static const uint8_t            ip_unspec[4] = {   0,   0,   0,   0 };
static const uint8_t            ip_bcast[4]  = { 255, 255, 255, 255 };
static       uint8_t            ip_socket_server[4];

/* String representation of Driver return codes */
static const char *str_ret[] = {
  "ARM_DRIVER_OK",
  "ARM_DRIVER_ERROR",
  "ARM_DRIVER_ERROR_BUSY",
  "ARM_DRIVER_ERROR_TIMEOUT",
  "ARM_DRIVER_ERROR_UNSUPPORTED",
  "ARM_DRIVER_ERROR_PARAMETER",
  "ARM_DRIVER_ERROR_SPECIFIC"
};

/* String representation of Driver Socket fucntion's return codes */
static const char *str_sock_ret[] = {
 "OK",
 "ARM_SOCKET_ERROR",
 "ARM_SOCKET_ESOCK",
 "ARM_SOCKET_EINVAL",
 "ARM_SOCKET_ENOTSUP",
 "ARM_SOCKET_ENOMEM",
 "ARM_SOCKET_EAGAIN",
 "ARM_SOCKET_EINPROGRESS",
 "ARM_SOCKET_ETIMEDOUT",
 "ARM_SOCKET_EISCONN",
 "ARM_SOCKET_ENOTCONN",
 "ARM_SOCKET_ECONNREFUSED",
 "ARM_SOCKET_ECONNRESET",
 "ARM_SOCKET_ECONNABORTED",
 "ARM_SOCKET_EALREADY",
 "ARM_SOCKET_EADDRINUSE",
 "ARM_SOCKET_EHOSTNOTFOUND"
};

/* Test message containing all letters of the alphabet */
static const uint8_t test_msg[44] = {
  "The quick brown fox jumps over the lazy dog."
};
/* Dummy text with normal distribution of letters */
static const uint8_t test_buf[2050] = {
  "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer nec odio. Praesent "
  "libero. Sed cursus ante dapibus diam. Sed nisi. Nulla quis sem at nibh elementum "
  "imperdiet. Duis sagittis ipsum. Praesent mauris. Fusce nec tellus sed augue semper "
  "porta. Mauris massa. Vestibulum lacinia arcu eget nulla. Class aptent taciti sociosqu "
  "ad litora torquent per conubia nostra, per inceptos himenaeos."

  "Curabitur sodales ligula in libero. Sed dignissim lacinia nunc. Curabitur tortor. "
  "Pellentesque nibh. Aenean quam. In scelerisque sem at dolor. Maecenas mattis. Sed "
  "convallis tristique sem. Proin ut ligula vel nunc egestas porttitor. Morbi lectus "
  "risus, iaculis vel, suscipit quis, luctus non, massa. Fusce ac turpis quis ligula "
  "lacinia aliquet. Mauris ipsum. Nulla metus metus, ullamcorper vel, tincidunt sed, "
  "euismod in, nibh."

  "Quisque volutpat condimentum velit. Class aptent taciti sociosqu ad litora torquent "
  "per conubia nostra, per inceptos himenaeos. Nam nec ante. Sed lacinia, urna non "
  "tincidunt mattis, tortor neque adipiscing diam, a cursus ipsum ante quis turpis. "
  "Nulla facilisi. Ut fringilla. Suspendisse potenti. Nunc feugiat mi a tellus consequat "
  "imperdiet. Vestibulum sapien. Proin quam. Etiam ultrices. Suspendisse in justo eu "
  "magna luctus suscipit. Sed lectus. Integer euismod lacus luctus magna."

  "Quisque cursus, metus vitae pharetra auctor, sem massa mattis sem, at interdum magna "
  "augue eget diam. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
  "posuere cubilia Curae; Morbi lacinia molestie dui. Praesent blandit dolor. Sed non "
  "quam. In vel mi sit amet augue congue elementum. Morbi in ipsum sit amet pede facilisis "
  "laoreet. Donec lacus nunc, viverra nec, blandit vel, egestas et, augue. Vestibulum "
  "tincidunt malesuada tellus. Ut ultrices ultrices enim. Curabitur sit amet mauris. "
  "Morbi in dui quis est pulvinar ullamcorper. Nulla facilisi. Integer lacinia sollicitudin "
  "massa. Cras metus."

  "Sed aliquet risus a tortor. Integer id quam. Morbi mi. Quisque nisl felis, venenatis "
  "tristique, dignissim in, ultrices sit amet augue."
};
static uint8_t buffer[2048];

/* WiFi event */
static void WIFI_DrvEvent (uint32_t evt, void *arg) {
  (void)arg;

  event |= evt;
}


/*-----------------------------------------------------------------------------
 *      Test cases
 *----------------------------------------------------------------------------*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup wifi_funcs WiFi Validation
\brief WiFi test cases
\details
The WiFi validation test performs the following tests:
- API interface compliance.
- Some of the control and management operations.
- Socket operation with various transfer sizes and communication parameters.
- Socket performance.
*/

/* Helper function that initializes and powers on WiFi Module if not initialized and powered */
static int32_t init_and_power_on (void) {

  if (powered == 0U) {
    if ((drv->Initialize   (event_func)     == ARM_DRIVER_OK) && 
        (drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK)) {
      powered = 1U;
    } else {
      return 0;
    }
  }

  return 1;
}

/* Helper function that is called before tests start executing */
void WIFI_DV_Initialize (void) {

  sscanf(WIFI_SOCKET_SERVER_IP, "%hhu.%hhu.%hhu.%hhu", &ip_socket_server[0], &ip_socket_server[1], &ip_socket_server[2], &ip_socket_server[3]);

  cap = drv->GetCapabilities();

  event_func = NULL;
  if ((cap.event_eth_rx_frame   != 0U) ||
      (cap.event_ap_connect     != 0U) ||
      (cap.event_ap_disconnect  != 0U)) {
    event_func = WIFI_DrvEvent;
  }

  if ((drv->SocketCreate        != NULL) &&
      (drv->SocketBind          != NULL) &&
      (drv->SocketListen        != NULL) &&
      (drv->SocketAccept        != NULL) &&
      (drv->SocketConnect       != NULL) &&
      (drv->SocketRecv          != NULL) &&
      (drv->SocketRecvFrom      != NULL) &&
      (drv->SocketSend          != NULL) &&
      (drv->SocketSendTo        != NULL) &&
      (drv->SocketGetSockName   != NULL) &&
      (drv->SocketGetPeerName   != NULL) &&
      (drv->SocketGetOpt        != NULL) &&
      (drv->SocketSetOpt        != NULL) &&
      (drv->SocketClose         != NULL) &&
      (drv->SocketGetHostByName != NULL)) {
    socket_funcs_exist = 1U;
  }
}

/* Helper function that is called after tests stop executing */
void WIFI_DV_Uninitialize (void) {

  if (connected != 0U) {
    if (drv->Deactivate (0U) == ARM_DRIVER_OK) {
      connected = 0U;
    }
  }
  if (powered != 0U) {
    if ((drv->PowerControl (ARM_POWER_OFF) == ARM_DRIVER_OK) && 
        (drv->Uninitialize ()              == ARM_DRIVER_OK)) {
      powered = 0U;
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* WiFi Control tests */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup wifi_ctrl WiFi Control
\ingroup wifi_funcs
\details
These tests verify API and operation of the WiFi control functions.
@{
*/

/**
\brief Test case: WIFI_GetVersion
\details
The test case \b WIFI_GetVersion verifies the WiFi Driver \b GetVersion function.
\code
ARM_DRIVER_VERSION (*GetVersion) (void);
\endcode
*/
void WIFI_GetVersion (void) {
  ARM_DRIVER_VERSION ver;

  ver = drv->GetVersion();

  TEST_ASSERT((ver.api >= ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)) && (ver.drv >= ARM_DRIVER_VERSION_MAJOR_MINOR(1,0)));

  snprintf(msg_buf, sizeof(msg_buf), "[INFO] Driver API version %d.%d, Driver version %d.%d", (ver.api >> 8), (ver.api & 0xFFU), (ver.drv >> 8), (ver.drv & 0xFFU));
  TEST_MESSAGE(msg_buf);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_GetCapabilities
\details
The test case \b WIFI_GetCapabilities verifies the WiFi Driver \b GetCapabilities function.
\code
ARM_WIFI_CAPABILITIES (*GetCapabilities) (void);
\endcode
*/
void WIFI_GetCapabilities (void) {

  cap = drv->GetCapabilities();

  TEST_ASSERT_MESSAGE((cap.station_ap != 0U) || (cap.station != 0U) || (cap.ap != 0U), "At least 1 mode must be supported");

  if (cap.wps_station != 0U) {
    TEST_ASSERT_MESSAGE((cap.station_ap != 0U) || (cap.station != 0U), "If WPS for station is supported version of station mode of operation must be supported also");
  }

  if (cap.wps_ap != 0U) {
    TEST_ASSERT_MESSAGE((cap.station_ap != 0U) || (cap.ap != 0U), "If WPS for AP is supported version of AP mode of operation must be supported also");
  }

  if ((cap.event_ap_connect != 0U) || (cap.event_ap_disconnect != 0U)) {
    TEST_ASSERT_MESSAGE((cap.station_ap != 0U) || (cap.ap != 0U), "If events for AP are supported version of AP mode of operation must be supported also");
  }

  if (cap.event_eth_rx_frame != 0U) {
    TEST_ASSERT_MESSAGE(cap.bypass_mode != 0U, "If event for Ethernet Rx frame is supported, bypass mode must be supported also");
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_Initialize/Uninitialize
\details
The test case \b WIFI_Initialize_Uninitialize verifies the WiFi Driver \b Initialize and \b Uninitialize functions.
\code
int32_t (*Initialize) (ARM_WIFI_SignalEvent_t cb_event);
\endcode
and
\code
int32_t (*Uninitialize) (void);
\endcode
Testing sequence:
 - Initialize without callback
 - Uninitialize
 - Initialize with callback (if driver supports it)
 - Power on
 - Uninitialize
 - Initialize without callback
 - Power on
 - Power off
 - Uninitialize
*/
void WIFI_Initialize_Uninitialize (void) {
  int32_t ret;

  if ((cap.event_eth_rx_frame  != 0U) ||
      (cap.event_ap_connect    != 0U) ||
      (cap.event_ap_disconnect != 0U)) {
    event_func = WIFI_DrvEvent;
  }

  TEST_ASSERT(drv->Initialize   (NULL)           == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize ()               == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Initialize   (event_func)     == ARM_DRIVER_OK);
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Uninitialize ()               == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Initialize   (NULL)           == ARM_DRIVER_OK);
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  ret =       drv->PowerControl (ARM_POWER_OFF);
  TEST_ASSERT((ret == ARM_DRIVER_OK) || (ret == ARM_DRIVER_ERROR_UNSUPPORTED));
  TEST_ASSERT(drv->Uninitialize ()               == ARM_DRIVER_OK);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_PowerControl
\details
The test case \b WIFI_PowerControl verifies the WiFi Driver \b PowerControl function.
\code
int32_t (*PowerControl) (ARM_POWER_STATE state);
\endcode
Testing sequence:
 - Power off
 - Initialize with callback (if driver supports it)
 - Power off
 - Power on
 - Scan
 - Power low
 - Power off
 - Uninitialize
*/
void WIFI_PowerControl (void) {
  int32_t ret;

  ret =       drv->PowerControl (ARM_POWER_OFF);
  TEST_ASSERT(ret == ARM_DRIVER_ERROR);
  TEST_ASSERT(drv->Initialize   (event_func)     == ARM_DRIVER_OK);
  ret =       drv->PowerControl (ARM_POWER_OFF);
  if (ret == ARM_DRIVER_ERROR_UNSUPPORTED) {
    TEST_MESSAGE("[WARNING] PowerControl (ARM_POWER_OFF) not supported");
  } else {
    TEST_ASSERT(ret == ARM_DRIVER_OK);
  }
  TEST_ASSERT(drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK);
  TEST_ASSERT(drv->Scan         (scan_info, WIFI_SCAN_MAX_NUM) >= 0);

  /* Test low power */
  ret = drv->PowerControl (ARM_POWER_LOW);
  switch (ret) {
    case ARM_DRIVER_OK:
      break;

    case ARM_DRIVER_ERROR_UNSUPPORTED:
      TEST_MESSAGE("[WARNING] PowerControl (ARM_POWER_LOW) is not supported");
      break;

    default:
      snprintf(msg_buf, sizeof(msg_buf), "[WARNING] PowerControl (ARM_POWER_LOW) returned %s", str_ret[-ret]);
      TEST_MESSAGE(msg_buf);
      break;
  }

  drv->PowerControl (ARM_POWER_OFF);
  drv->Uninitialize ();
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_GetModuleInfo
\details
The test case \b WIFI_GetModuleInfo verifies the WiFi Driver \b GetModuleInfo function.
\code
int32_t (*GetModuleInfo) (char *module_info, uint32_t max_len);
\endcode
*/
void WIFI_GetModuleInfo (void) {
  int32_t ret;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

  ret = drv->GetModuleInfo(NULL, sizeof(data_buf));
  if (ret == ARM_DRIVER_ERROR_UNSUPPORTED) {
    TEST_MESSAGE("[WARNING] GetModuleInfo (...) not supported");
  } else {
    TEST_ASSERT(ret == ARM_DRIVER_ERROR_PARAMETER);
  }
  ret = drv->GetModuleInfo(data_buf, 0U);
  if (ret == ARM_DRIVER_ERROR_UNSUPPORTED) {
    TEST_MESSAGE("[WARNING] GetModuleInfo (...) not supported");
  } else {
    TEST_ASSERT(ret == ARM_DRIVER_ERROR_PARAMETER);
  }

  memset((void *)data_buf, 0xCC, sizeof(data_buf));
  ret = drv->GetModuleInfo(data_buf, sizeof(data_buf));
  switch (ret) {
    case ARM_DRIVER_OK:
      TEST_ASSERT(strlen(data_buf) != 0);
      break;
    case ARM_DRIVER_ERROR_UNSUPPORTED:
      TEST_MESSAGE("[WARNING] GetModuleInfo () is not supported");
      break;
    default:
      snprintf(msg_buf, sizeof(msg_buf), "[WARNING] GetModuleInfo () returned %s", str_ret[-ret]);
      TEST_MESSAGE(msg_buf);
      break;
  }
}
/**
@}
*/
// End of wifi_ctrl

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* WiFi Management tests */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup wifi_mgmt WiFi Management
\ingroup wifi_funcs
\details
These tests verify API and operation of the WiFi management functions.
@{
*/

#if (WIFI_SETGETOPTION_BSSID_EN != 0)
static void WIFI_SetOption_GetOption_BSSID (void) {
#if ((WIFI_SETGETOPTION_BSSID_EN & 1) != 0)
  uint8_t  u8_arr[8] __ALIGNED(4);
  uint8_t  bssid[7]  __ALIGNED(4);
#endif
#if ((WIFI_SETGETOPTION_BSSID_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_BSSID_EN & 1) != 0)
  // Set tests
  memset((void *)bssid, 0x11, 7);
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_BSSID, bssid, 6U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_BSSID, bssid, 6U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_BSSID_STA, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]) == 6);
    memset((void *) u8_arr, 0xCC, 8);
    memcpy((void *)&u8_arr[1], (const void *)bssid, 6);
    if (drv->SetOption (0U, ARM_WIFI_BSSID, bssid, 6U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_BSSID, NULL,       0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_BSSID, NULL,       6U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_BSSID, bssid,      0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_BSSID, bssid,      5U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_BSSID, &u8_arr[1], 6U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_BSSID, &u8_arr[1], 7U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_BSSID, bssid,      7U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_BSSID, bssid,      6U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_BSSID for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_BSSID_AP, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]) == 6);
    memset((void *) u8_arr, 0xCC, 8);
    memcpy((void *)&u8_arr[1], (const void *)bssid, 6);
    if (drv->SetOption (1U, ARM_WIFI_BSSID, bssid, 6U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BSSID, NULL,       0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BSSID, NULL,       6U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BSSID, bssid,      0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BSSID, bssid,      5U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BSSID, &u8_arr[1], 6U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BSSID, &u8_arr[1], 7U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BSSID, bssid,      7U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BSSID, bssid,      6U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_BSSID for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_BSSID_EN & 2) != 0)
  // Get tests
  len = 6U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_BSSID, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_BSSID, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 6U;
    if (drv->GetOption (0U, ARM_WIFI_BSSID, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_BSSID, NULL,      &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 6U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_BSSID, NULL,      &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_BSSID, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_BSSID, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 6U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_BSSID, data_buf+1,&len) == ARM_DRIVER_OK);
      len = 7U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_BSSID, data_buf+1,&len) == ARM_DRIVER_OK);
      len = 7U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_BSSID, data_buf,  &len) == ARM_DRIVER_OK);
      len = 6U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_BSSID, data_buf,  &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_BSSID for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 6U;
    if (drv->GetOption (1U, ARM_WIFI_BSSID, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BSSID, NULL,      &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 6U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BSSID, NULL,      &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BSSID, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BSSID, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 6U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BSSID, data_buf+1,&len) == ARM_DRIVER_OK);
      len = 7U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BSSID, data_buf+1,&len) == ARM_DRIVER_OK);
      len = 7U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BSSID, data_buf,  &len) == ARM_DRIVER_OK);
      len = 6U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BSSID, data_buf,  &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_BSSID for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_BSSID_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_BSSID_STA, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]) == 6);
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    len = 6U;
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_BSSID, bssid,    6U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_BSSID, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 6U);
    TEST_ASSERT(memcmp((const void *)bssid, (const void *)data_buf, (size_t)len) == 0);
  }
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_BSSID_AP, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]) == 6);
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    len = 6U;
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BSSID, bssid,    6U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BSSID, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 6U);
    TEST_ASSERT(memcmp((const void *)bssid, (const void *)data_buf, (size_t)len) == 0);
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_TX_POWER_EN != 0)
static void WIFI_SetOption_GetOption_TX_POWER (void) {
#if ((WIFI_SETGETOPTION_TX_POWER_EN & 1) != 0)
  uint32_t power;
#endif
#if ((WIFI_SETGETOPTION_TX_POWER_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_TX_POWER_EN & 1) != 0)
  // Set tests
  power = WIFI_TX_POWER_STA;
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_TX_POWER, &power, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_TX_POWER, &power, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    power = WIFI_TX_POWER_STA;
    if (drv->SetOption (0U, ARM_WIFI_TX_POWER, &power, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_TX_POWER, NULL,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_TX_POWER, NULL,   4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_TX_POWER, &power, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_TX_POWER, &power, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_TX_POWER, &power, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_TX_POWER, &power, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_TX_POWER for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    power = WIFI_TX_POWER_AP;
    if (drv->SetOption (1U, ARM_WIFI_TX_POWER, &power, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_TX_POWER, NULL,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_TX_POWER, NULL,   4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_TX_POWER, &power, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_TX_POWER, &power, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_TX_POWER, &power, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_TX_POWER, &power, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_TX_POWER for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_TX_POWER_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    if (drv->GetOption (0U, ARM_WIFI_TX_POWER, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_TX_POWER, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_TX_POWER, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_TX_POWER for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_TX_POWER, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_TX_POWER, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_TX_POWER, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_TX_POWER for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_TX_POWER_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    power = WIFI_TX_POWER_STA;
    len   = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_TX_POWER, &power,   4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&power, (const void *)data_buf, (size_t)len) == 0);
  }
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    power = WIFI_TX_POWER_AP;
    len   = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_TX_POWER, &power,   4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_TX_POWER, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&power, (const void *)data_buf, (size_t)len) == 0);
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_LP_TIMER_EN != 0)
static void WIFI_SetOption_GetOption_LP_TIMER (void) {
#if ((WIFI_SETGETOPTION_LP_TIMER_EN & 1) != 0)
  uint32_t time;
#endif
#if ((WIFI_SETGETOPTION_LP_TIMER_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_LP_TIMER_EN & 1) != 0)
  // Set tests
  time = WIFI_LP_TIMER_STA;
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_LP_TIMER, &time, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_LP_TIMER, &time, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    time = WIFI_LP_TIMER_STA;
    if (drv->SetOption (0U, ARM_WIFI_LP_TIMER, &time, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_LP_TIMER, NULL,  0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_LP_TIMER, NULL,  4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_LP_TIMER, &time, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_LP_TIMER, &time, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_LP_TIMER, &time, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_LP_TIMER, &time, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_LP_TIMER for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_LP_TIMER, &time, 4U) == ARM_DRIVER_ERROR_UNSUPPORTED);
  }
#endif

#if ((WIFI_SETGETOPTION_LP_TIMER_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_LP_TIMER, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_LP_TIMER, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    if (drv->GetOption (0U, ARM_WIFI_LP_TIMER, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_LP_TIMER, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_LP_TIMER, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_LP_TIMER, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_LP_TIMER, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_LP_TIMER, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_LP_TIMER, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_LP_TIMER for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    TEST_ASSERT  (drv->GetOption (1U, ARM_WIFI_LP_TIMER, data_buf,  &len) == ARM_DRIVER_ERROR_UNSUPPORTED);
  }
#endif

#if ((WIFI_SETGETOPTION_LP_TIMER_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    time = WIFI_LP_TIMER_STA;
    len  = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_LP_TIMER, &time,    4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_LP_TIMER, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&time, (const void *)data_buf, (size_t)len) == 0);
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_DTIM_EN != 0)
static void WIFI_SetOption_GetOption_DTIM (void) {
#if ((WIFI_SETGETOPTION_DTIM_EN & 1) != 0)
  uint32_t dtim;
#endif
#if ((WIFI_SETGETOPTION_DTIM_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_DTIM_EN & 1) != 0)
  // Set tests
  dtim = WIFI_DTIM_STA;
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_DTIM, &dtim, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_DTIM, &dtim, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    dtim = WIFI_DTIM_STA;
    if (drv->SetOption (0U, ARM_WIFI_DTIM, &dtim, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_DTIM, NULL,  0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_DTIM, NULL,  4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_DTIM, &dtim, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_DTIM, &dtim, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_DTIM, &dtim, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_DTIM, &dtim, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_DTIM for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    dtim = WIFI_DTIM_AP;
    if (drv->SetOption (1U, ARM_WIFI_DTIM, &dtim, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_DTIM, NULL,  0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_DTIM, NULL,  4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_DTIM, &dtim, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_DTIM, &dtim, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_DTIM, &dtim, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_DTIM, &dtim, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_DTIM for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_DTIM_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    if (drv->GetOption (0U, ARM_WIFI_DTIM, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_DTIM, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_DTIM, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_DTIM for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_DTIM, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_DTIM, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_DTIM, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_DTIM for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_DTIM_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    dtim = WIFI_DTIM_STA;
    len  = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_DTIM, &dtim,   4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&dtim, (const void *)data_buf, (size_t)len) == 0);
  }
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    dtim = WIFI_DTIM_AP;
    len  = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_DTIM, &dtim,   4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_DTIM, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&dtim, (const void *)data_buf, (size_t)len) == 0);
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_BEACON_EN != 0)
static void WIFI_SetOption_GetOption_BEACON (void) {
#if ((WIFI_SETGETOPTION_BEACON_EN & 1) != 0)
  uint32_t beacon;
#endif
#if ((WIFI_SETGETOPTION_BEACON_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_BEACON_EN & 1) != 0)
  // Set tests
  beacon = WIFI_BEACON_AP;
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_BEACON, &beacon, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_BEACON, &beacon, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_BEACON, &beacon, 4U) == ARM_DRIVER_ERROR_UNSUPPORTED);
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    beacon = WIFI_BEACON_AP;
    if (drv->SetOption (1U, ARM_WIFI_BEACON, &beacon, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BEACON, NULL,    0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BEACON, NULL,    4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BEACON, &beacon, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BEACON, &beacon, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BEACON, &beacon, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BEACON, &beacon, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_BEACON for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_BEACON_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_BEACON, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_BEACON, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_BEACON, data_buf,  &len) == ARM_DRIVER_ERROR_UNSUPPORTED);
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_BEACON, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BEACON, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BEACON, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BEACON, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BEACON, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BEACON, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BEACON, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_BEACON for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_BEACON_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    beacon = WIFI_BEACON_AP;
    len    = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_BEACON, &beacon,   4U)  == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_BEACON, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&beacon, (const void *)data_buf, (size_t)len) == 0);
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_MAC_EN != 0)
static void WIFI_SetOption_GetOption_MAC (void) {
#if ((WIFI_SETGETOPTION_MAC_EN & 1) != 0)
  uint8_t  u8_arr[8]      __ALIGNED(4);
  uint8_t  mac[7]         __ALIGNED(4);
  uint8_t  mac_ap_def[6]  __ALIGNED(4);
  uint8_t  mac_sta_def[6] __ALIGNED(4);
#endif
#if ((WIFI_SETGETOPTION_MAC_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

// Read default MAC so it can be restored at the end of this test case
#if ((WIFI_SETGETOPTION_MAC_EN & 3) == 3)
  if (((cap.station_ap != 0) || (cap.station != 0))) {
    len = 6U;
    drv->GetOption (0U, ARM_WIFI_MAC, mac_sta_def, &len);
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {
    len = 6U;
    drv->GetOption (1U, ARM_WIFI_MAC, mac_ap_def, &len);
  }
#endif

#if ((WIFI_SETGETOPTION_MAC_EN & 1) != 0)
  // Set tests
  memset((void *)mac, 0x11, 7);
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_MAC, mac, 6U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_MAC, mac, 6U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_MAC_STA, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6);
    memset((void *) u8_arr, 0xCC, 8);
    memcpy((void *)&u8_arr[1], (const void *)mac, 6);
    if (drv->SetOption (0U, ARM_WIFI_MAC, mac, 6U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_MAC, NULL,       0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_MAC, NULL,       6U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_MAC, mac,        0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_MAC, mac,        5U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_MAC, &u8_arr[1], 6U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_MAC, &u8_arr[1], 7U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_MAC, mac,        7U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_MAC, mac,        6U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_MAC for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_MAC_AP, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6);
    memset((void *) u8_arr, 0xCC, 8);
    memcpy((void *)&u8_arr[1], (const void *)mac, 6);
    if (drv->SetOption (1U, ARM_WIFI_MAC, mac, 6U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_MAC, NULL,       0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_MAC, NULL,       6U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_MAC, mac,        0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_MAC, mac,        5U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_MAC, &u8_arr[1], 6U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_MAC, &u8_arr[1], 7U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_MAC, mac,        7U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_MAC, mac,        6U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_MAC for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_MAC_EN & 2) != 0)
  // Get tests
  len = 6U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_MAC, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_MAC, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 6U;
    if (drv->GetOption (0U, ARM_WIFI_MAC, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_MAC, NULL,      &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 6U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_MAC, NULL,      &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_MAC, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_MAC, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 6U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_MAC, data_buf+1,&len) == ARM_DRIVER_OK);
      len = 7U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_MAC, data_buf+1,&len) == ARM_DRIVER_OK);
      len = 7U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_MAC, data_buf,  &len) == ARM_DRIVER_OK);
      len = 6U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_MAC, data_buf,  &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_MAC for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 6U;
    if (drv->GetOption (1U, ARM_WIFI_MAC, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_MAC, NULL,      &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 6U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_MAC, NULL,      &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_MAC, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_MAC, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 6U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_MAC, data_buf+1,&len) == ARM_DRIVER_OK);
      len = 7U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_MAC, data_buf+1,&len) == ARM_DRIVER_OK);
      len = 7U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_MAC, data_buf,  &len) == ARM_DRIVER_OK);
      len = 6U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_MAC, data_buf,  &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_MAC for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_MAC_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_MAC_STA, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6);
    len = 6U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_MAC, mac,      6U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_MAC, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 6U);
    TEST_ASSERT(memcmp((const void *)mac, (const void *)data_buf, (size_t)len) == 0);
  }
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_MAC_AP, "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6);
    len = 6U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_MAC, mac,      6U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_MAC, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 6U);
    TEST_ASSERT(memcmp((const void *)mac, (const void *)data_buf, (size_t)len) == 0);
  }

  // Restore default MAC
  if (((cap.station_ap != 0) || (cap.station != 0)) && (memcmp((const void *)mac_sta_def, (const void *)"\0\0\0\0\0\0", 6) != 0)) {
    drv->SetOption (0U, ARM_WIFI_MAC, mac_sta_def, 6U);
  }
  if (((cap.station_ap != 0) || (cap.ap != 0))      && (memcmp((const void *)mac_ap_def,  (const void *)"\0\0\0\0\0\0", 6) != 0)) {
    drv->SetOption (1U, ARM_WIFI_MAC, mac_ap_def,  6U);
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_IP_EN != 0)
static void WIFI_SetOption_GetOption_IP (void) {
#if ((WIFI_SETGETOPTION_IP_EN & 1) != 0)
  uint32_t u32_0, u32_1;
  uint8_t  ip[5] __ALIGNED(4);
#endif
#if ((WIFI_SETGETOPTION_IP_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_IP_EN & 1) != 0)
  // Set tests
  u32_0 = 0U;
  u32_1 = 1U;
  memset((void *)ip, 0, 5);
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_IP, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_IP, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_STA, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (0U, ARM_WIFI_IP, ip, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP, ip,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP, ip,   3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP, ip,   5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP, ip,   4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP for Station is not supported");
    }
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (1U, ARM_WIFI_IP, ip, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP, ip,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP, ip,   3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP, ip,   5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP, ip,   4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP for Access Point is not supported");
    }
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif

#if ((WIFI_SETGETOPTION_IP_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_IP, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_IP, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    if (drv->GetOption (0U, ARM_WIFI_IP, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_IP, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_STA, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP, ip,       4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)ip, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP, ip,       4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)ip, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_IP_SUBNET_MASK_EN != 0)
static void WIFI_SetOption_GetOption_IP_SUBNET_MASK (void) {
#if ((WIFI_SETGETOPTION_IP_SUBNET_MASK_EN & 1) != 0)
  uint32_t u32_0, u32_1;
  uint8_t  mask[5] __ALIGNED(4);
#endif
#if ((WIFI_SETGETOPTION_IP_SUBNET_MASK_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_IP_SUBNET_MASK_EN & 1) != 0)
  // Set tests
  u32_0 = 0U;
  u32_1 = 1U;
  memset((void *)mask, 0, 5);
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_IP_SUBNET_MASK, mask, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_IP_SUBNET_MASK, mask, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_SUBNET_MASK_STA, "%hhu.%hhu.%hhu.%hhu", &mask[0], &mask[1], &mask[2], &mask[3]) == 4);
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (0U, ARM_WIFI_IP_SUBNET_MASK, mask, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_SUBNET_MASK, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_SUBNET_MASK, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_SUBNET_MASK, mask, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_SUBNET_MASK, mask, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_SUBNET_MASK, mask, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_SUBNET_MASK, mask, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_SUBNET_MASK for Station is not supported");
    }
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_SUBNET_MASK_AP, "%hhu.%hhu.%hhu.%hhu", &mask[0], &mask[1], &mask[2], &mask[3]) == 4);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (1U, ARM_WIFI_IP_SUBNET_MASK, mask, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_SUBNET_MASK, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_SUBNET_MASK, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_SUBNET_MASK, mask, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_SUBNET_MASK, mask, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_SUBNET_MASK, mask, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_SUBNET_MASK, mask, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_SUBNET_MASK for Access Point is not supported");
    }
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif

#if ((WIFI_SETGETOPTION_IP_SUBNET_MASK_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    if (drv->GetOption (0U, ARM_WIFI_IP_SUBNET_MASK, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_SUBNET_MASK, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_SUBNET_MASK, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_SUBNET_MASK for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_IP_SUBNET_MASK, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_SUBNET_MASK, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_SUBNET_MASK, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_SUBNET_MASK for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_SUBNET_MASK_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_SUBNET_MASK_STA, "%hhu.%hhu.%hhu.%hhu", &mask[0], &mask[1], &mask[2], &mask[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_SUBNET_MASK, mask,     4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)mask, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_SUBNET_MASK_AP, "%hhu.%hhu.%hhu.%hhu", &mask[0], &mask[1], &mask[2], &mask[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_SUBNET_MASK, mask,     4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_SUBNET_MASK, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)mask, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_IP_GATEWAY_EN != 0)
static void WIFI_SetOption_GetOption_IP_GATEWAY (void) {
#if ((WIFI_SETGETOPTION_IP_GATEWAY_EN & 1) != 0)
  uint32_t u32_0, u32_1;
  uint8_t  ip[5] __ALIGNED(4);
#endif
#if ((WIFI_SETGETOPTION_IP_GATEWAY_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_IP_GATEWAY_EN & 1) != 0)
  // Set tests
  u32_0 = 0U;
  u32_1 = 1U;
  memset((void *)ip, 0, 5);
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_IP_GATEWAY, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_IP_GATEWAY, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_GATEWAY_STA, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (0U, ARM_WIFI_IP_GATEWAY, ip, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_GATEWAY, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_GATEWAY, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_GATEWAY, ip,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_GATEWAY, ip,   3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_GATEWAY, ip,   5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_GATEWAY, ip,   4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_GATEWAY for Station is not supported");
    }
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_GATEWAY_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (1U, ARM_WIFI_IP_GATEWAY, ip, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_GATEWAY, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_GATEWAY, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_GATEWAY, ip,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_GATEWAY, ip,   3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_GATEWAY, ip,   5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_GATEWAY, ip,   4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_GATEWAY for Access Point is not supported");
    }
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif

#if ((WIFI_SETGETOPTION_IP_GATEWAY_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    if (drv->GetOption (0U, ARM_WIFI_IP_GATEWAY, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_GATEWAY, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_GATEWAY, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_GATEWAY for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_IP_GATEWAY, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_GATEWAY, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_GATEWAY, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_GATEWAY for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_GATEWAY_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_GATEWAY_STA, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_GATEWAY, ip,       4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)ip, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_GATEWAY_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_GATEWAY, ip,       4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_GATEWAY, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)ip, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_IP_DNS1_EN != 0)
static void WIFI_SetOption_GetOption_IP_DNS1 (void) {
#if ((WIFI_SETGETOPTION_IP_DNS1_EN & 1) != 0)
  uint32_t u32_0, u32_1;
  uint8_t  ip[5] __ALIGNED(4);
#endif
#if ((WIFI_SETGETOPTION_IP_DNS1_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_IP_DNS1_EN & 1) != 0)
  // Set tests
  u32_0 = 0U;
  u32_1 = 1U;
  memset((void *)ip, 0, 5);
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_IP_DNS1, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_IP_DNS1, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DNS1_STA, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (0U, ARM_WIFI_IP_DNS1, ip, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS1, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS1, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS1, ip,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS1, ip,   3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS1, ip,   5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS1, ip,   4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_DNS1 for Station is not supported");
    }
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DNS1_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (1U, ARM_WIFI_IP_DNS1, ip, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS1, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS1, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS1, ip,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS1, ip,   3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS1, ip,   5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS1, ip,   4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_DNS1 for Access Point is not supported");
    }
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DNS1_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    if (drv->GetOption (0U, ARM_WIFI_IP_DNS1, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS1, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS1, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_DNS1 for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_IP_DNS1, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS1, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS1, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_DNS1 for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DNS1_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DNS1_STA, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS1, ip,       4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)ip, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DNS1_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS1, ip,       4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS1, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)ip, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_IP_DNS2_EN != 0)
static void WIFI_SetOption_GetOption_IP_DNS2 (void) {
#if ((WIFI_SETGETOPTION_IP_DNS2_EN & 1) != 0)
  uint32_t u32_0, u32_1;
  uint8_t  ip[5] __ALIGNED(4);
#endif
#if ((WIFI_SETGETOPTION_IP_DNS2_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_IP_DNS2_EN & 1) != 0)
  // Set tests
  u32_0 = 0U;
  u32_1 = 1U;
  memset((void *)ip, 0, 5);
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_IP_DNS2, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_IP_DNS2, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DNS2_STA, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (0U, ARM_WIFI_IP_DNS2, ip, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS2, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS2, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS2, ip,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS2, ip,   3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS2, ip,   5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS2, ip,   4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_DNS2 for Station is not supported");
    }
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DNS2_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (1U, ARM_WIFI_IP_DNS2, ip, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS2, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS2, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS2, ip,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS2, ip,   3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS2, ip,   5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS2, ip,   4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_DNS2 for Access Point is not supported");
    }
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DNS2_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_IP_DNS2, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_IP_DNS2, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    if (drv->GetOption (0U, ARM_WIFI_IP_DNS2, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS2, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS2, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS2, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS2, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS2, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS2, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_DNS2 for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_IP_DNS2, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS2, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS2, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS2, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS2, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS2, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS2, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_DNS2 for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DNS2_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DNS2_STA, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DNS2, ip,       4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DNS2, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)ip, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DNS2_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DNS2, ip,       4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DNS2, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)ip, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_IP_DHCP_EN != 0)
static void WIFI_SetOption_GetOption_IP_DHCP (void) {
#if ((WIFI_SETGETOPTION_IP_DHCP_EN & 1) != 0)
  uint32_t u32_1, u32_0;
#endif
#if ((WIFI_SETGETOPTION_IP_DHCP_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_IP_DHCP_EN & 1) != 0)
  // Set tests
  u32_0 = 0U;
  u32_1 = 1U;
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_IP_DHCP, &u32_0, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_IP_DHCP, &u32_0, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    if (drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP, NULL,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP, NULL,   4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0, 4U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_DHCP for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    if (drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP, NULL,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP, NULL,   4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_DHCP for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DHCP_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_IP_DHCP, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_IP_DHCP, data_buf,  &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    if (drv->GetOption (0U, ARM_WIFI_IP_DHCP, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DHCP, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DHCP, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 1U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_DHCP for Station is not supported");
    }
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_IP_DHCP, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_DHCP for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DHCP_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.station != 0)) && ((not_suported & 1U) == 0U)) {   // Station test
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_0,   4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&u32_0, (const void *)data_buf, (size_t)len) == 0);
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP, &u32_1,   4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&u32_1, (const void *)data_buf, (size_t)len) == 0);
  }
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0,   4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&u32_0, (const void *)data_buf, (size_t)len) == 0);
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1,   4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&u32_1, (const void *)data_buf, (size_t)len) == 0);
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_IP_DHCP_POOL_BEGIN_EN != 0)
static void WIFI_SetOption_GetOption_IP_DHCP_POOL_BEGIN (void) {
#if ((WIFI_SETGETOPTION_IP_DHCP_POOL_BEGIN_EN & 1) != 0)
  uint32_t u32_1, u32_0;
  uint8_t  ip[5] __ALIGNED(4);
#endif
#if ((WIFI_SETGETOPTION_IP_DHCP_POOL_BEGIN_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_IP_DHCP_POOL_BEGIN_EN & 1) != 0)
  // Set tests
  memset((void *)ip, 0, 5);
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_IP_DHCP_POOL_BEGIN, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_IP_DHCP_POOL_BEGIN, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP_POOL_BEGIN, ip, 4U) == ARM_DRIVER_ERROR_UNSUPPORTED);
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DHCP_POOL_BEGIN_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, ip, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, ip,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, ip,   3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, ip,   5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, ip,   4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_DHCP_POOL_BEGIN for Access Point is not supported");
    }
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DHCP_POOL_BEGIN_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_IP_DHCP_POOL_BEGIN, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_IP_DHCP_POOL_BEGIN, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DHCP_POOL_BEGIN, data_buf, &len) == ARM_DRIVER_ERROR_UNSUPPORTED);
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_DHCP_POOL_BEGIN for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DHCP_POOL_BEGIN_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DHCP_POOL_BEGIN_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, ip,       4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_BEGIN, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)ip, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_IP_DHCP_POOL_END_EN != 0)
static void WIFI_SetOption_GetOption_IP_DHCP_POOL_END (void) {
#if ((WIFI_SETGETOPTION_IP_DHCP_POOL_END_EN & 1) != 0)
  uint32_t u32_1, u32_0;
  uint8_t  ip[5] __ALIGNED(4);
#endif
#if ((WIFI_SETGETOPTION_IP_DHCP_POOL_END_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_IP_DHCP_POOL_END_EN & 1) != 0)
  // Set tests
  memset((void *)ip, 0, 5);
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_IP_DHCP_POOL_END, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_IP_DHCP_POOL_END, ip, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP_POOL_END, ip, 4U) == ARM_DRIVER_ERROR_UNSUPPORTED);
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DHCP_POOL_END_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);  // Turn DHCP off
    if (drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, ip, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, NULL, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, NULL, 4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, ip,   0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, ip,   3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, ip,   5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, ip,   4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_DHCP_POOL_END for Access Point is not supported");
    }
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);  // Turn DHCP on
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DHCP_POOL_END_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_IP_DHCP_POOL_END, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_IP_DHCP_POOL_END, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    TEST_ASSERT(drv->GetOption (0U, ARM_WIFI_IP_DHCP_POOL_END, data_buf, &len) == ARM_DRIVER_ERROR_UNSUPPORTED);
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_DHCP_POOL_END for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DHCP_POOL_END_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    TEST_ASSERT(sscanf((const char *)WIFI_IP_DHCP_POOL_END_AP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]) == 4);
    len = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_0, 4U);          // Turn DHCP off
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, ip,       4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_POOL_END, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)ip, (const void *)data_buf, (size_t)len) == 0);
    drv->SetOption (1U, ARM_WIFI_IP_DHCP, &u32_1, 4U);          // Turn DHCP on
  }
#endif
}
#endif

#if (WIFI_SETGETOPTION_IP_DHCP_LEASE_TIME_EN != 0)
static void WIFI_SetOption_GetOption_IP_DHCP_LEASE_TIME (void) {
#if ((WIFI_SETGETOPTION_IP_DHCP_LEASE_TIME_EN & 1) != 0)
  uint32_t time;
#endif
#if ((WIFI_SETGETOPTION_IP_DHCP_LEASE_TIME_EN & 2) != 0)
  uint32_t len;
#endif
  uint8_t  not_suported;

  not_suported = 0U;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

#if ((WIFI_SETGETOPTION_IP_DHCP_LEASE_TIME_EN & 1) != 0)
  // Set tests
  time = WIFI_IP_DHCP_LEASE_TIME_AP;
  TEST_ASSERT(drv->SetOption (  2U, ARM_WIFI_IP_DHCP_LEASE_TIME, &time, 4U) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->SetOption (255U, ARM_WIFI_IP_DHCP_LEASE_TIME, &time, 4U) == ARM_DRIVER_ERROR_PARAMETER);

  if (((cap.station_ap != 0) || (cap.station != 0))) {  // Station test
    TEST_ASSERT(drv->SetOption (0U, ARM_WIFI_IP_DHCP_LEASE_TIME, &time, 4U) == ARM_DRIVER_ERROR_UNSUPPORTED);
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    time = WIFI_IP_DHCP_LEASE_TIME_AP;
    if (drv->SetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, &time, 4U) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, NULL,  0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, NULL,  4U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, &time, 0U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, &time, 3U) == ARM_DRIVER_ERROR_PARAMETER);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, &time, 5U) == ARM_DRIVER_OK);
      TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, &time, 4U) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] SetOption ARM_WIFI_IP_DHCP_LEASE_TIME for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DHCP_LEASE_TIME_EN & 2) != 0)
  // Get tests
  len = 4U;
  TEST_ASSERT(drv->GetOption (  2U, ARM_WIFI_IP_DHCP_LEASE_TIME, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->GetOption (255U, ARM_WIFI_IP_DHCP_LEASE_TIME, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);

  if ((cap.station_ap != 0) || (cap.station != 0)) {    // Station test
    len = 4U;
    TEST_ASSERT  (drv->GetOption (0U, ARM_WIFI_IP_DHCP_LEASE_TIME, data_buf,  &len) == ARM_DRIVER_ERROR_UNSUPPORTED);
  }
  if ((cap.station_ap != 0) || (cap.ap != 0)) {         // AP test
    len = 4U;
    if (drv->GetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, data_buf,  &len) != ARM_DRIVER_ERROR_UNSUPPORTED) {
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, NULL,     &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 0U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 3U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, data_buf, &len) == ARM_DRIVER_ERROR_PARAMETER);
      len = 5U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, data_buf, &len) == ARM_DRIVER_OK);
      len = 4U;
      TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, data_buf, &len) == ARM_DRIVER_OK);
    } else {
      not_suported |= 2U;
      TEST_MESSAGE("[WARNING] GetOption ARM_WIFI_IP_DHCP_LEASE_TIME for Access Point is not supported");
    }
  }
#endif

#if ((WIFI_SETGETOPTION_IP_DHCP_LEASE_TIME_EN & 3) == 3)
  // Check with Get that Set has written the correct values
  if (((cap.station_ap != 0) || (cap.ap != 0)) && ((not_suported & 2U) == 0U)) {        // AP test
    time = WIFI_IP_DHCP_LEASE_TIME_AP;
    len  = 4U;
    memset((void *)data_buf, 0xCC, sizeof(data_buf));
    TEST_ASSERT(drv->SetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, &time,    4U)   == ARM_DRIVER_OK);
    TEST_ASSERT(drv->GetOption (1U, ARM_WIFI_IP_DHCP_LEASE_TIME, data_buf, &len) == ARM_DRIVER_OK);
    TEST_ASSERT(len == 4U);
    TEST_ASSERT(memcmp((const void *)&time, (const void *)data_buf, (size_t)len) == 0);
  }
#endif
}
#endif

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_SetOption_GetOption
\details
The test case \b WIFI_SetOption_GetOption verifies the WiFi Driver \b SetOption and \b GetOption functions.
(Options: ARM_WIFI_BSSID, ARM_WIFI_MAC, ARM_WIFI_IP, ARM_WIFI_IP_SUBNET_MASK, ARM_WIFI_IP_GATEWAY, ARM_WIFI_IP_DNS1,
ARM_WIFI_IP_DNS2, ARM_WIFI_IP_DHCP_POOL_BEGIN, ARM_WIFI_IP_DHCP_POOL_END are checked with buffer not aligned to 4 bytes).<br>
Tests for each option is conditionally executed depending on WIFI_SETGETOPTION_... settings in DV_Config.h file.
\code
  int32_t (*SetOption) (uint32_t interface, uint32_t option, const void *data, uint32_t len);
\endcode
and
\code
  int32_t (*GetOption) (uint32_t interface, uint32_t option, void *data, uint32_t *len);
\endcode
Function \b WIFI_SetOption_GetOption_BSSID              tests \b ARM_WIFI_BSSID option.<br>
Function \b WIFI_SetOption_GetOption_TX_POWER           tests \b ARM_WIFI_TX_POWER option.<br>
Function \b WIFI_SetOption_GetOption_LP_TIMER           tests \b ARM_WIFI_LP_TIMER option.<br>
Function \b WIFI_SetOption_GetOption_DTIM               tests \b ARM_WIFI_DTIM option.<br>
Function \b WIFI_SetOption_GetOption_BEACON             tests \b ARM_WIFI_BEACON option.<br>
Function \b WIFI_SetOption_GetOption_MAC                tests \b ARM_WIFI_MAC option.<br>
Function \b WIFI_SetOption_GetOption_IP                 tests \b ARM_WIFI_IP option.<br>
Function \b WIFI_SetOption_GetOption_IP_SUBNET_MASK     tests \b ARM_WIFI_IP_SUBNET_MASK option.<br>
Function \b WIFI_SetOption_GetOption_IP_GATEWAY         tests \b ARM_WIFI_IP_GATEWAY option.<br>
Function \b WIFI_SetOption_GetOption_IP_DNS1            tests \b ARM_WIFI_IP_DNS1 option.<br>
Function \b WIFI_SetOption_GetOption_IP_DNS2            tests \b ARM_WIFI_IP_DNS2 option.<br>
Function \b WIFI_SetOption_GetOption_IP_DHCP            tests \b ARM_WIFI_IP_DHCP option.<br>
Function \b WIFI_SetOption_GetOption_IP_DHCP_POOL_BEGIN tests \b ARM_WIFI_IP_DHCP_POOL_BEGIN option.<br>
Function \b WIFI_SetOption_GetOption_IP_DHCP_POOL_END   tests \b ARM_WIFI_IP_DHCP_POOL_END option.<br>
Function \b WIFI_SetOption_GetOption_IP_DHCP_LEASE_TIME tests \b ARM_WIFI_IP_DHCP_LEASE_TIME option.
*/
void WIFI_SetOption_GetOption (void) {

#if (WIFI_SETGETOPTION_BSSID_EN != 0)
  WIFI_SetOption_GetOption_BSSID ();
#endif
#if (WIFI_SETGETOPTION_TX_POWER_EN != 0)
  WIFI_SetOption_GetOption_TX_POWER ();
#endif
#if (WIFI_SETGETOPTION_LP_TIMER_EN != 0)
  WIFI_SetOption_GetOption_LP_TIMER ();
#endif
#if (WIFI_SETGETOPTION_DTIM_EN != 0)
  WIFI_SetOption_GetOption_DTIM ();
#endif
#if (WIFI_SETGETOPTION_BEACON_EN != 0)
  WIFI_SetOption_GetOption_BEACON ();
#endif
#if (WIFI_SETGETOPTION_MAC_EN != 0)
  WIFI_SetOption_GetOption_MAC ();
#endif
#if (WIFI_SETGETOPTION_IP_EN != 0)
  WIFI_SetOption_GetOption_IP ();
#endif
#if (WIFI_SETGETOPTION_IP_SUBNET_MASK_EN != 0)
  WIFI_SetOption_GetOption_IP_SUBNET_MASK ();
#endif
#if (WIFI_SETGETOPTION_IP_GATEWAY_EN != 0)
  WIFI_SetOption_GetOption_IP_GATEWAY ();
#endif
#if (WIFI_SETGETOPTION_IP_DNS1_EN != 0)
  WIFI_SetOption_GetOption_IP_DNS1 ();
#endif
#if (WIFI_SETGETOPTION_IP_DNS2_EN != 0)
  WIFI_SetOption_GetOption_IP_DNS2 ();
#endif
#if (WIFI_SETGETOPTION_IP_DHCP_EN != 0)
  WIFI_SetOption_GetOption_IP_DHCP ();
#endif
#if (WIFI_SETGETOPTION_IP_DHCP_POOL_BEGIN_EN != 0)
  WIFI_SetOption_GetOption_IP_DHCP_POOL_BEGIN ();
#endif
#if (WIFI_SETGETOPTION_IP_DHCP_POOL_END_EN != 0)
  WIFI_SetOption_GetOption_IP_DHCP_POOL_END ();
#endif
#if (WIFI_SETGETOPTION_IP_DHCP_LEASE_TIME_EN != 0)
  WIFI_SetOption_GetOption_IP_DHCP_LEASE_TIME ();
#endif
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_Scan
\details
The test case \b WIFI_Scan verifies the WiFi Driver \b Scan function.
\code
int32_t (*Scan) (ARM_WIFI_SCAN_INFO_t scan_info[], uint32_t max_num);
\endcode
*/
void WIFI_Scan (void) {
  int32_t ret;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

  TEST_ASSERT(drv->Scan(NULL,      WIFI_SCAN_MAX_NUM) == ARM_DRIVER_ERROR_PARAMETER);
  TEST_ASSERT(drv->Scan(scan_info, 0U)                == ARM_DRIVER_ERROR_PARAMETER);

  memset((void *)scan_info, 0xCC, sizeof(scan_info));
  ret = drv->Scan(scan_info, 10U);
  if (ret == 0) {
    TEST_MESSAGE("[WARNING] Scan (..) found no networks");
  } else {
    TEST_ASSERT((ret > 0) && (ret <= WIFI_SCAN_MAX_NUM));
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_Activate_Deactivate
\details
The test case \b WIFI_Activate_Deactivate verifies the WiFi Driver \b Activate and \b Deactivate functions.
\code
int32_t (*Activate) (uint32_t interface, const ARM_WIFI_CONFIG_t *config);
\endcode
and
\code
int32_t (*Deactivate) (uint32_t interface);
\endcode
Testing sequence (for Station and Access Point):
 - if not initialized and powered initialize and power on
 - Deactivate
 - Activate (with invalid parameters)
 - Activate (with valid parameters)
 - Deactivate
 - Activate (with invalid WPS parameters)
*/
void WIFI_Activate_Deactivate (void) {
  int32_t ret;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

  /* Test Station configuration setup */
  if ((cap.station != 0) || (cap.station_ap != 0)) {

    TEST_ASSERT(drv->Deactivate (0U) == ARM_DRIVER_OK);

    /* Test function with invalid parameters */
    TEST_ASSERT(drv->Activate(0U, NULL)    == ARM_DRIVER_ERROR_PARAMETER);

    config.ssid       = NULL;
    config.pass       = NULL;
    config.security   = 0U;
    config.ch         = 0U;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(0U, &config) == ARM_DRIVER_ERROR_PARAMETER);

    config.ssid       = WIFI_STA_SSID;
    config.pass       = NULL;
    config.security   = WIFI_STA_SECURITY;
    config.ch         = 0U;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(0U, &config) == ARM_DRIVER_ERROR_PARAMETER);

    config.ssid       = WIFI_STA_SSID;
    config.pass       = WIFI_STA_PASS;
    config.security   = ARM_WIFI_SECURITY_UNKNOWN;
    config.ch         = WIFI_STA_CH;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(0U, &config) == ARM_DRIVER_ERROR_PARAMETER);

    config.ssid       = WIFI_STA_SSID;
    config.pass       = WIFI_STA_PASS;
    config.security   = WIFI_STA_SECURITY;
    config.ch         = 255U;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(0U, &config) == ARM_DRIVER_ERROR_PARAMETER);

    config.ssid       = WIFI_STA_SSID;
    config.pass       = WIFI_STA_PASS;
    config.security   = WIFI_STA_SECURITY;
    config.ch         = WIFI_STA_CH;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(3U, &config) == ARM_DRIVER_ERROR_PARAMETER);

    /* Test function with autodetect channel, can return unsupported or succeed */
    config.ssid       = WIFI_STA_SSID;
    config.pass       = WIFI_STA_PASS;
    config.security   = WIFI_STA_SECURITY;
    config.ch         = 0U;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    ret = drv->Activate(0U, &config);
    if (ret == ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_MESSAGE("[WARNING] Activate (0, ...) with autodetect channel not supported");
    } else {
      TEST_ASSERT(ret == ARM_DRIVER_OK);
    }

    if (ret == ARM_DRIVER_OK) {
      TEST_ASSERT(drv->Deactivate (0U) == ARM_DRIVER_OK);
    }

    /* Test function with valid parameters -> must succeed */
    config.ssid       = WIFI_STA_SSID;
    config.pass       = WIFI_STA_PASS;
    config.security   = WIFI_STA_SECURITY;
    config.ch         = WIFI_STA_CH;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(0U, &config) == ARM_DRIVER_OK);

    if (ret == ARM_DRIVER_OK) {
      TEST_ASSERT(drv->Deactivate (0U) == ARM_DRIVER_OK);
    }

    if (cap.wps_station != 0U) {
      /* Test function with invalid WPS configurations */
      config.ssid       = WIFI_STA_SSID;
      config.pass       = WIFI_STA_PASS;
      config.security   = WIFI_STA_SECURITY;
      config.ch         = WIFI_STA_CH;
      config.wps_method = 255U;
      config.wps_pin    = NULL;
      TEST_ASSERT(drv->Activate(0U, &config) == ARM_DRIVER_ERROR_PARAMETER);

      config.ssid       = WIFI_STA_SSID;
      config.pass       = WIFI_STA_PASS;
      config.security   = WIFI_STA_SECURITY;
      config.ch         = WIFI_STA_CH;
      config.wps_method = ARM_WIFI_WPS_METHOD_PIN;
      config.wps_pin    = NULL;
      TEST_ASSERT(drv->Activate(0U, &config) == ARM_DRIVER_ERROR_PARAMETER);
    }
  }

  /* Test Access Point configuration setup */
  if ((cap.ap != 0) || (cap.station_ap != 0)) {

    TEST_ASSERT(drv->Deactivate (1U) == ARM_DRIVER_OK);

    /* Test function with invalid parameters */
    TEST_ASSERT(drv->Activate(1U, NULL)    == ARM_DRIVER_ERROR_PARAMETER);

    config.ssid       = NULL;
    config.pass       = NULL;
    config.security   = 0U;
    config.ch         = 0U;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(1U, &config) == ARM_DRIVER_ERROR_PARAMETER);

    config.ssid       = WIFI_STA_SSID;
    config.pass       = NULL;
    config.security   = WIFI_AP_SECURITY;
    config.ch         = 0U;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(1U, &config) == ARM_DRIVER_ERROR_PARAMETER);

    config.ssid       = WIFI_AP_SSID;
    config.pass       = WIFI_AP_PASS;
    config.security   = ARM_WIFI_SECURITY_UNKNOWN;
    config.ch         = WIFI_AP_CH;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(1U, &config) == ARM_DRIVER_ERROR_PARAMETER);

    config.ssid       = WIFI_AP_SSID;
    config.pass       = WIFI_AP_PASS;
    config.security   = WIFI_AP_SECURITY;
    config.ch         = 255U;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(1U, &config) == ARM_DRIVER_ERROR_PARAMETER);

    config.ssid       = WIFI_AP_SSID;
    config.pass       = WIFI_AP_PASS;
    config.security   = WIFI_AP_SECURITY;
    config.ch         = WIFI_AP_CH;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(3U, &config) == ARM_DRIVER_ERROR_PARAMETER);

    /* Test function with autodetect channel, can return unsupported or succeed */
    config.ssid       = WIFI_AP_SSID;
    config.pass       = WIFI_AP_PASS;
    config.security   = WIFI_AP_SECURITY;
    config.ch         = 0U;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    ret = drv->Activate(1U, &config);
    if (ret == ARM_DRIVER_ERROR_UNSUPPORTED) {
      TEST_MESSAGE("[WARNING] Activate (1, ...) with autodetect channel not supported");
    } else {
      TEST_ASSERT(ret == ARM_DRIVER_OK);
    }

    if (ret == ARM_DRIVER_OK) {
      TEST_ASSERT(drv->Deactivate (1U) == ARM_DRIVER_OK);
    }

    /* Test function with valid parameters -> must succeed */
    config.ssid       = WIFI_AP_SSID;
    config.pass       = WIFI_AP_PASS;
    config.security   = WIFI_AP_SECURITY;
    config.ch         = WIFI_AP_CH;
    config.wps_method = 0U;
    config.wps_pin    = NULL;
    TEST_ASSERT(drv->Activate(1U, &config) == ARM_DRIVER_OK);

    if (ret == ARM_DRIVER_OK) {
      TEST_ASSERT(drv->Deactivate (1U) == ARM_DRIVER_OK);
    }

    if (cap.wps_ap != 0U) {
      /* Test function with invalid WPS configurations */
      config.ssid       = WIFI_AP_SSID;
      config.pass       = WIFI_AP_PASS;
      config.security   = WIFI_AP_SECURITY;
      config.ch         = WIFI_AP_CH;
      config.wps_method = 255U;
      config.wps_pin    = NULL;
      TEST_ASSERT(drv->Activate(1U, &config) == ARM_DRIVER_ERROR_PARAMETER);

      config.ssid       = WIFI_AP_SSID;
      config.pass       = WIFI_AP_PASS;
      config.security   = WIFI_AP_SECURITY;
      config.ch         = WIFI_AP_CH;
      config.wps_method = ARM_WIFI_WPS_METHOD_PIN;
      config.wps_pin    = NULL;
      TEST_ASSERT(drv->Activate(1U, &config) == ARM_DRIVER_ERROR_PARAMETER);
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_IsConnected
\details
The test case \b WIFI_IsConnected verifies the WiFi Driver \b IsConnected function.
\code
uint32_t (*IsConnected) (void);
\endcode
*/
void WIFI_IsConnected (void) {

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

  TEST_ASSERT(drv->IsConnected () == 0U);

  /* Test function with valid Station configuration */
  if ((cap.station != 0) || (cap.station_ap != 0)) {
    memset((void *)&config, 0, sizeof(config));
    config.ssid     = WIFI_STA_SSID;
    config.pass     = WIFI_STA_PASS;
    config.security = WIFI_STA_SECURITY;
    config.ch       = WIFI_STA_CH;

    drv->Activate(0U, &config);
    TEST_ASSERT(drv->IsConnected () != 0U);

    drv->Deactivate (0U);
    TEST_ASSERT(drv->IsConnected () == 0U);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_GetNetInfo
\details
The test case \b WIFI_GetNetInfo verifies the WiFi Driver \b GetNetInfo function.
\code
int32_t (*GetNetInfo) (ARM_WIFI_NET_INFO_t *net_info);
\endcode
*/
void WIFI_GetNetInfo (void) {

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

  /* Test function with invalid pointer */
  TEST_ASSERT(drv->GetNetInfo (NULL) == ARM_DRIVER_ERROR_PARAMETER);

  /* Test function with valid Station configuration */
  if ((cap.station != 0) || (cap.station_ap != 0)) {
    memset((void *)&config, 0, sizeof(config));
    config.ssid     = WIFI_STA_SSID;
    config.pass     = WIFI_STA_PASS;
    config.security = WIFI_STA_SECURITY;
    config.ch       = WIFI_STA_CH;

    drv->Activate(0U, &config);

    memset((void *)&net_info, 0xCC, sizeof(net_info));
    TEST_ASSERT(drv->GetNetInfo (&net_info) == ARM_DRIVER_OK);

    /* Check returned info */
    TEST_ASSERT(net_info.security == config.security);
    if (config.ch != 0U) {
      TEST_ASSERT(net_info.ch == config.ch);
    }
    TEST_ASSERT(net_info.rssi != 0U);

    drv->Deactivate (0U);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_Activate_AP
\details
The test case \b WIFI_Activate_AP verifies the WiFi Driver \b Activate function AP operation.
Test result is checked by connecting WiFi client to AP.
*/
void WIFI_Activate_AP (void) {
  int32_t ret, tout;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

  if ((cap.event_ap_connect != 0U) && (event_func != NULL)) {
    event = 0U;
  }

  /* Test Access Point configuration setup */
  if ((cap.ap != 0) || (cap.station_ap != 0)) {
    memset((void *)&config, 0, sizeof(config));
    config.ssid     = WIFI_AP_SSID;
    config.pass     = WIFI_AP_PASS;
    config.security = WIFI_AP_SECURITY;
    config.ch       = WIFI_AP_CH;
    ret = drv->Activate(1U, &config);
    TEST_ASSERT(ret == ARM_DRIVER_OK);

    if (ret == ARM_DRIVER_OK) {
      // Wait for WIFI_AP_CLIENT_CON_TIMEOUT in ms for client to connect
      // If event for connect is supported, loop will end when connect is detected
      // otherwise result of test is result of client connection status.
      for (tout = WIFI_AP_CLIENT_CON_TIMEOUT / 128; tout > 0; tout--) {
        osDelay(128U);
        if ((cap.event_ap_connect != 0U) && (event_func != NULL)) {
          if (event & ARM_WIFI_EVENT_AP_CONNECT) {
            break;
          }
        }
      }
    }

    if ((cap.event_ap_connect != 0U) && (event_func != NULL)) {
      TEST_ASSERT((event & ARM_WIFI_EVENT_AP_CONNECT) != 0U);
      event = 0U;
    }

    // Stop AP
    drv->Deactivate (1U);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_Activate_Station_WPS_PBC
\details
The test case \b WIFI_Activate_Station_WPS_PBC verifies the WiFi Driver \b Activate function Station connection with WPS 
and Push-Button Configuration method.
This test case requires that test Access Point has active Push-button WPS method when test is started.
Usually started on the WiFi AP (router) by pressing the WPS button.
*/
void WIFI_Activate_Station_WPS_PBC (void) {
  int32_t ret;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

  /* Test Station WPS PBC connection */
  if ((cap.wps_station != 0U) && ((cap.station != 0) || (cap.station_ap != 0))) {
    memset((void *)&config, 0, sizeof(config));

    /* Connect with valid WPS configuration - Push-Button Configuration method -> should succeed */
    config.wps_method = ARM_WIFI_WPS_METHOD_PBC;
    ret = drv->Activate(0U, &config);
    TEST_ASSERT(ret == ARM_DRIVER_OK);

    if (ret == ARM_DRIVER_OK) {
      // Check connect information is as expected
      memset((void *)&net_info, 0xCC, sizeof(net_info));
      drv->GetNetInfo (&net_info);

      TEST_ASSERT(memcmp((const void *)net_info.ssid, (const void *)WIFI_STA_SSID, sizeof(WIFI_STA_SSID)) == 0);
      TEST_ASSERT(memcmp((const void *)net_info.pass, (const void *)WIFI_STA_PASS, sizeof(WIFI_STA_PASS)) == 0);
      TEST_ASSERT(net_info.security == WIFI_STA_SECURITY);
      if (WIFI_STA_CH != 0) {
        TEST_ASSERT(net_info.ch == WIFI_STA_CH);
      }

      // Disconnect
      drv->Deactivate (0U);
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_Activate_Station_WPS_PIN
\details
The test case \b WIFI_Activate_Station_WPS_PIN verifies the WiFi Driver \b Activate function Station connection with WPS 
and PIN method.
This test case requires that test Access Point has active PIN WPS method when test is started.
Usually needs to be configured on the WiFi AP (router).
*/
void WIFI_Activate_Station_WPS_PIN (void) {
  int32_t ret;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

  /* Test Station WPS PIN connection */
  if ((cap.wps_station != 0U) && ((cap.station != 0) || (cap.station_ap != 0))) {
    memset((void *)&config, 0, sizeof(config));

    /* Connect with valid WPS configuration - PIN method -> should succeed */
    config.wps_method = ARM_WIFI_WPS_METHOD_PIN;
    config.wps_pin    = WIFI_STA_WPS_PIN;
    ret = drv->Activate(0U, &config);
    TEST_ASSERT(ret == ARM_DRIVER_OK);

    if (ret == ARM_DRIVER_OK) {
      // Check connect information is as expected
      memset((void *)&net_info, 0xCC, sizeof(net_info));
      drv->GetNetInfo (&net_info);

      TEST_ASSERT(memcmp((const void *)net_info.ssid, (const void *)WIFI_STA_SSID, sizeof(WIFI_STA_SSID)) == 0);
      TEST_ASSERT(memcmp((const void *)net_info.pass, (const void *)WIFI_STA_PASS, sizeof(WIFI_STA_PASS)) == 0);
      TEST_ASSERT(net_info.security == WIFI_STA_SECURITY);
      if (WIFI_STA_CH != 0) {
        TEST_ASSERT(net_info.ch == WIFI_STA_CH);
      }

      // Disconnect
      drv->Deactivate (0U);
    }
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_Activate_AP_WPS_PBC
\details
The test case \b WIFI_Activate_AP_WPS_PBC verifies the WiFi Driver \b Activate function AP WPS 
and Push-Button Configuration method functionality.
Test result is checked by connecting the WiFi client to AP with WPS Push-Button Configuration method.
*/
void WIFI_Activate_AP_WPS_PBC (void) {
  int32_t ret;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

  /* Test AP WPS PBC */
  if ((cap.wps_ap != 0U) && ((cap.ap != 0) || (cap.station_ap != 0))) {
    memset((void *)&config, 0, sizeof(config));
    config.ssid     = WIFI_AP_SSID;
    config.pass     = WIFI_AP_PASS;
    config.security = WIFI_AP_SECURITY;
    config.ch       = WIFI_AP_CH;

    /* Start AP with WPS configuration - Push-Button Configuration method -> should succeed */
    config.wps_method = ARM_WIFI_WPS_METHOD_PBC;
    ret = drv->Activate(1U, &config);
    TEST_ASSERT(ret == ARM_DRIVER_OK);

    if (ret == ARM_DRIVER_OK) {
      // Wait predefined time for Client to connect
      osDelay(WIFI_AP_CLIENT_CON_TIMEOUT);
    }

    // Stop AP
    drv->Deactivate (1U);
  }
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Test case: WIFI_Activate_AP_WPS_PIN
\details
The test case \b WIFI_Activate_AP_WPS_PIN verifies the WiFi Driver \b Activate function AP WPS 
PIN method functionality.
Test result is checked by connecting the WiFi client to AP with WPS PIN method.
*/
void WIFI_Activate_AP_WPS_PIN (void) {
  int32_t ret;

  if (init_and_power_on () == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Driver initialization and power on failed");
    return;
  }

  /* Test AP WPS PIN */
  if ((cap.wps_ap != 0U) && ((cap.ap != 0) || (cap.station_ap != 0))) {
    memset((void *)&config, 0, sizeof(config));
    config.ssid     = WIFI_AP_SSID;
    config.pass     = WIFI_AP_PASS;
    config.security = WIFI_AP_SECURITY;
    config.ch       = WIFI_AP_CH;

    /* Start AP with WPS configuration - PIN method -> should succeed */
    config.wps_method = ARM_WIFI_WPS_METHOD_PIN;
    config.wps_pin    = WIFI_AP_WPS_PIN;
    ret = drv->Activate(1U, &config);
    TEST_ASSERT(ret == ARM_DRIVER_OK);

    if (ret == ARM_DRIVER_OK) {
      // Wait predefined time for Client to connect
      osDelay(WIFI_AP_CLIENT_CON_TIMEOUT);
    }

    // Stop AP
    drv->Deactivate (1U);
  }
}
/**
@}
*/
// End of wifi_mgmt

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/* WiFi Socket tests */
/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/**
\defgroup wifi_sock_api WiFi Socket API
\ingroup wifi_funcs
\details 
These tests verify API and operation of the WiFi socket functions.
*/

/* Helper function that initialize and connects to WiFi Access Point */
static int32_t station_init (uint32_t con) {

  if (cap.station == 0U) {
    return 0;
  }

  if (powered == 0U) {
    if ((drv->Initialize   (event_func)     == ARM_DRIVER_OK) && 
        (drv->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK)) {
      powered = 1U;
    } else {
      return 0;
    }
  }

  if (connected != con) {
    if (con != 0) {
      memset((void *)&config, 0, sizeof(config));
      config.ssid     = WIFI_STA_SSID;
      config.pass     = WIFI_STA_PASS;
      config.security = WIFI_STA_SECURITY;
      config.ch       = WIFI_STA_CH;
      if (drv->Activate(0U, &config) != ARM_DRIVER_OK) {
        return 0;
      }
    } else {
      if (drv->Deactivate(0U) != ARM_DRIVER_OK) {
        return 0;
      }
    }

    connected = (uint8_t)con;
  }

  return 1;
}

/* Helper function that disconnects and uninitializes Station */
static void station_uninit (void) {

  drv->Deactivate (0U);
  connected = 0U;

  drv->PowerControl (ARM_POWER_OFF);
  drv->Uninitialize ();
  powered = 0U;
}

/* Helper function for execution of socket test function in the worker thread */
static int32_t th_execute (osThreadId_t *id, uint32_t sig, uint32_t tout) {
  osThreadFlagsSet (id, sig);
  if (osThreadFlagsWait (TH_OK | TH_TOUT, osFlagsWaitAny, tout) == TH_OK) {
    /* Success, completed in time */
    return (1);
  }
  /* If function timeout expired prepare output message */
  snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Execution timeout (%d ms)", tout);
  return (0);
}

/* Helper function for preparing output message for TH_ASSERT2 macro */
static void th_assert2_msg (const char *s1, int32_t r1, int32_t r2) {
  snprintf(msg_buf, sizeof(msg_buf), "[WARNING] Non BSD-strict, %s (result %s, expected %s)", s1, str_sock_ret[-r1], str_sock_ret[-r2]);
}

#define TH_EXECUTE(sig,tout) do {                                               \
                               io.xid++;                                        \
                               rval = th_execute (worker, sig, tout);           \
                               if (rval == 0) {                                 \
                                 /* Msg was prepared in th_execute function */  \
                                 TEST_ASSERT_MESSAGE(0,msg_buf);                \
                               }                                                \
                             } while (0)

#define TH_ASSERT(cond)      do {                                               \
                               if (rval) { TEST_ASSERT(cond); }                 \
                             } while (0)

#define TH_ASSERT2(c1,c2,s1,r1,r2) do {                                         \
                               if (rval) {                                      \
                                 if (!c2) { TEST_ASSERT(c1); }                  \
                                 else {                                         \
                                   th_assert2_msg(s1, r1, r2); /* Prep msg */   \
                                   TEST_MESSAGE(msg_buf);                       \
                                 }                                              \
                               }                                                \
                             } while (0)

#define ARG_INIT()           do {                                               \
                               io.owner = osThreadGetId ();                     \
                               io.xid   = 0;                                    \
                             } while (0)

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* Create IO parameters */
typedef struct {
  int32_t      sock;
  int32_t      af;
  int32_t      type;
  int32_t      protocol;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_CREATE;

/* Assign arguments */
#define ARG_CREATE(_af,_type,_proto) do {                     \
                                       io.af       = _af;     \
                                       io.type     = _type;   \
                                       io.protocol = _proto;  \
                                     } while (0)

/* Create worker thread */
__NO_RETURN static void Th_Create (IO_CREATE *io) {
  uint32_t flags, xid;

  for (;;) {
    flags = osThreadFlagsWait (F_CREATE | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE:
        /* Create socket */
        io->rc = drv->SocketCreate (io->af, io->type, io->protocol);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketCreate
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketCreate verifies the WiFi Driver \b SocketCreate function:
\code
int32_t (*SocketCreate) (int32_t af, int32_t type, int32_t protocol);
\endcode

Create socket test:
 - Check function parameters 
 - Create multiple stream sockets
 - Gradually close stream sockets and create datagram sockets
 - Close datagram sockets
*/
void WIFI_SocketCreate (void) { 
  osThreadId_t worker;
  int32_t      rval;
  IO_CREATE    io;
  int32_t      sock[WIFI_SOCKET_MAX_NUM], i;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Create, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Check parameter (af = -1) */
  ARG_CREATE (-1, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (af = INT32_MIN) */
  ARG_CREATE (INT32_MIN, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (af = INT32_MAX) */
  ARG_CREATE (INT32_MAX, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (type = -1) */
  ARG_CREATE (ARM_SOCKET_AF_INET, -1, ARM_SOCKET_IPPROTO_TCP);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (type = INT32_MIN) */
  ARG_CREATE (ARM_SOCKET_AF_INET, INT32_MIN, ARM_SOCKET_IPPROTO_TCP);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (type = INT32_MAX) */
  ARG_CREATE (ARM_SOCKET_AF_INET, INT32_MAX, ARM_SOCKET_IPPROTO_TCP);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter, stream socket (protocol = -1) */
  ARG_CREATE (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, -1);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter, stream socket (protocol = INT32_MIN) */
  ARG_CREATE (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, INT32_MIN);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter, stream socket (protocol = INT32_MAX) */
  ARG_CREATE (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, INT32_MAX);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter, datagram socket (protocol = -1) */
  ARG_CREATE (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, -1);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter, datagram socket (protocol = INT32_MIN) */
  ARG_CREATE (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, INT32_MIN);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter, datagram socket (protocol = INT32_MAX) */
  ARG_CREATE (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, INT32_MAX);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter, stream socket (protocol = ARM_SOCKET_IPPROTO_UDP) */
  ARG_CREATE (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_UDP);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter, datagram socket (protocol = ARM_SOCKET_IPPROTO_TCP) */
  ARG_CREATE (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_TCP);
  TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Create multiple stream sockets */
  ARG_CREATE (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
  for (i = 0; i < WIFI_SOCKET_MAX_NUM; i++) {
    TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc >= 0);
    sock[i] = io.rc;
  }
  osDelay (10);

  /* Gradually close stream sockets, create datagram sockets */
  ARG_CREATE (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
  for (i = 0; i < WIFI_SOCKET_MAX_NUM; i++) {
    /* Close stream socket */
    io.sock = sock[i];
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Create datagram socket */
    TH_EXECUTE (F_CREATE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc >= 0);
    sock[i] = io.rc;
  }
  osDelay (10);

  /* Close datagram sockets */
  for (i = 0; i < WIFI_SOCKET_MAX_NUM; i++) {
    io.sock = sock[i];
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
  }
  osDelay (10);

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* Bind IO parameters */
typedef struct {
  int32_t        sock;
  const uint8_t *ip;
  uint32_t       ip_len;
  uint16_t       port;
  uint16_t       reserved;
  int32_t        rc;
  /* Control */
  osThreadId_t   owner;
  uint32_t       xid;
} IO_BIND;

/* Assign arguments */
#define ARG_BIND(_sock,_ip,_ip_len,_port) do {                   \
                                            io.sock   = _sock;   \
                                            io.ip     = _ip;     \
                                            io.ip_len = _ip_len; \
                                            io.port   = _port;   \
                                          } while (0)

/* Bind worker thread */
__NO_RETURN static void Th_Bind (IO_BIND *io) {
  uint32_t flags,xid;

  for (;;) {
    flags = osThreadFlagsWait (F_CREATE_TCP | F_CREATE_UDP | F_BIND | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_BIND:
        /* Bind socket */
        io->rc = drv->SocketBind (io->sock, io->ip, io->ip_len, io->port);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketBind
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketBind verifies the WiFi Driver \b SocketBind function:
\code
int32_t (*SocketBind) (int32_t socket, const uint8_t *ip, uint32_t  ip_len, uint16_t  port);
\endcode

Stream socket test:
 - Create stream socket
 - Check function parameters 
 - Bind stream socket
 - Bind socket second time
 - Create 2nd stream socket
 - Bind 2nd socket, used port
 - Bind 2nd socket, unused port
 - Close stream sockets
 - Bind closed socket

Datagram socket test: 
 - Create datagram socket
 - Bind datagram socket
 - Bind socket second time
 - Create 2nd datagram socket
 - Bind 2nd socket, used port
 - Bind 2nd socket, unused port
 - Close datagram socket
 - Bind closed socket
*/
void WIFI_SocketBind (void) { 
  osThreadId_t worker;
  int32_t      rval;
  IO_BIND      io;
  int32_t      sock,sock2;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Bind, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Check parameter (socket = -1) */
    ARG_BIND   (-1, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_BIND (INT32_MIN, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_BIND (INT32_MAX, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (ip = NULL) */
    ARG_BIND (sock, NULL, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (ip_len = 0) */
    ARG_BIND (sock, ip_unspec, 0, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (ip_len = UINT32_MAX) */
    ARG_BIND (sock, ip_unspec, UINT32_MAX, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (port = 0) */
    ARG_BIND (sock, ip_unspec, 4, 0);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Bind socket */
    ARG_BIND (sock, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Bind socket 2nd time */
    ARG_BIND (sock, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket already bound) */
    /* Strict: EINVAL, valid non-strict: OK, ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EINVAL), ((io.rc == 0) || (io.rc == ARM_SOCKET_ERROR)), "bind socket to same address again", io.rc, ARM_SOCKET_EINVAL);

    /* Create 2nd stream socket */
    TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
    if (io.rc < 0) {
      TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
    }
    sock2 = io.rc;

    /* Bind 2nd socket, used port */
    ARG_BIND (sock2, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (address already used) */
    /* Strict: EADDRINUSE, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EADDRINUSE), (io.rc == ARM_SOCKET_ERROR), "bind another socket to used address", io.rc, ARM_SOCKET_EADDRINUSE);

    /* Bind 2nd socket, unused port */
    ARG_BIND (sock2, ip_unspec, 4, ECHO_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Close sockets */
    io.sock = sock2;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Bind again, closed socket */
    ARG_BIND (sock, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    /* Return code strict: ESOCK, non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "bind on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Check parameter (socket = -1) */
    ARG_BIND (-1, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_BIND (INT32_MIN, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_BIND (INT32_MAX, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (ip = NULL) */
    ARG_BIND (sock, NULL, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (ip_len = 0) */
    ARG_BIND (sock, ip_unspec, 0, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (ip_len = UINT32_MAX) */
    ARG_BIND (sock, ip_unspec, UINT32_MAX, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (port = 0) */
    ARG_BIND (sock, ip_unspec, 4, 0);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Bind socket */
    ARG_BIND (sock, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Bind socket 2nd time */
    ARG_BIND (sock, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket already bound) */
    /* Strict: EINVAL, valid non-strict: OK, ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EINVAL), ((io.rc == 0) || (io.rc == ARM_SOCKET_ERROR)), "bind socket to same address again", io.rc, ARM_SOCKET_EINVAL);

    /* Create 2nd datagram socket */
    TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
    if (io.rc < 0) {
      TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
    }
    sock2 = io.rc;

    /* Bind 2nd socket, used port */
    ARG_BIND (sock2, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (address already used) */
    /* Strict: EADDRINUSE, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EADDRINUSE), (io.rc == ARM_SOCKET_ERROR), "bind another socket to used address", io.rc, ARM_SOCKET_EADDRINUSE);

    /* Bind 2nd socket, unused port */
    ARG_BIND (sock2, ip_unspec, 4, ECHO_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Close sockets */
    io.sock = sock2;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Bind again, closed socket */
    ARG_BIND (sock, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "bind on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* Listen IO parameters */
typedef struct {
  int32_t      sock;
  int32_t      backlog;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_LISTEN;

/* Assign arguments */
#define ARG_LISTEN(_sock,_backlog) do {                     \
                                     io.sock    = _sock;    \
                                     io.backlog = _backlog; \
                                   } while (0)

/* Listen worker thread */
__NO_RETURN static void Th_Listen (IO_LISTEN *io) {
  uint32_t flags,xid;

  for (;;) {
    flags = osThreadFlagsWait (F_CREATE_TCP | F_CREATE_UDP |
                               F_BIND       | F_LISTEN     | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_BIND:
        /* Bind socket */
        io->rc = drv->SocketBind (io->sock, ip_unspec, 4, DISCARD_PORT);
        break;

      case F_LISTEN:
        /* Listen on socket */
        io->rc = drv->SocketListen (io->sock, io->backlog);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketListen
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketListen verifies the WiFi Driver \b SocketListen function:
\code
int32_t (*SocketListen) (int32_t socket, int32_t backlog);
\endcode

Stream socket test 1:
 - Create stream socket
 - Bind socket
 - Check function parameters 
 - Start listening
 - Start listening 2nd time
 - Close socket

Stream socket test 2:
 - Create stream socket
 - Start listening, unbound socket
 - Close socket
 - Start listening, closed socket

Datagram socket test:
 - Create datagram socket
 - Bind socket
 - Start listening
 - Close socket
*/
void WIFI_SocketListen (void) { 
  osThreadId_t worker;
  int32_t      rval;
  IO_LISTEN    io;
  int32_t      sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Listen, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ARG_LISTEN (-1, 1);
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_LISTEN (INT32_MIN, 1);
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_LISTEN (INT32_MAX, 1);
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Start listening */
    ARG_LISTEN (sock, 1);
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Start listening 2nd time */
    ARG_LISTEN (sock, 1);
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket already listening) */
    /* Strict: EINVAL, valid non-strict: OK, ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EINVAL), ((io.rc == 0) || (io.rc == ARM_SOCKET_ERROR)), "listen on already listening socket", io.rc, ARM_SOCKET_EINVAL);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Start listening, unbound socket */
    ARG_LISTEN (sock, 1);
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not bound) */
    /* Strict: EINVAL, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EINVAL), (io.rc == ARM_SOCKET_ERROR), "listen on unbound socket", io.rc, ARM_SOCKET_EINVAL);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Start listening, closed socket */
    ARG_LISTEN (sock, 1);
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "listen on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Start listening */
    ARG_LISTEN (sock, 1);
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    /* Should return error (operation not supported) */
    /* Strict: ENOTSUP, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTSUP), (io.rc == ARM_SOCKET_ERROR), "listen on datagram socket", io.rc, ARM_SOCKET_ENOTSUP);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* Accept IO parameters */
typedef struct {
  int32_t      sock;
  uint8_t     *ip;
  uint32_t    *ip_len;
  uint16_t    *port;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
  const char  *cmd;
} IO_ACCEPT;

/* Assign arguments */
#define ARG_ACCEPT(_sock,_ip,_ip_len,_port) do {                   \
                                              io.sock   = _sock;   \
                                              io.ip     = _ip;     \
                                              io.ip_len = _ip_len; \
                                              io.port   = _port;   \
                                            } while (0)

/* TestAssistant control */
#define TEST_PORT           2000

/* CONNECT <proto>,<ip_addr>,<port>,<delay_ms>
           <proto>    = protocol (TCP, UDP)
           <ip_addr>  = IP address (0.0.0.0 = sender address)
           <port>     = port number
           <delay_ms> = startup delay

  Example: CONNECT TCP,192.168.1.200,80,600
  (wait 600ms then connect to 192.168.1.200, port 80)
*/
#define CMD_CONNECT_TCP     "CONNECT TCP,0.0.0.0,2000,500"
#define CMD_CONNECT_UDP     "CONNECT UDP,0.0.0.0,2000,200"

/* Accept worker thread */
__NO_RETURN static void Th_Accept (IO_ACCEPT *io) {
  uint32_t flags,xid;
  int32_t sock;

  for (;;) {
    flags = osThreadFlagsWait (F_CREATE_TCP | F_CREATE_UDP | F_BIND | F_LISTEN |
                               F_ACCEPT     | F_SEND_CTRL  | F_RECV | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_BIND:
        /* Bind socket */
        io->rc = drv->SocketBind (io->sock, ip_unspec, 4, TEST_PORT);
        break;

      case F_LISTEN:
        /* Listen on socket */
        io->rc = drv->SocketListen (io->sock, 1);
        break;

      case F_ACCEPT:
        /* Accept on socket */
        io->rc = drv->SocketAccept (io->sock, io->ip, io->ip_len, io->port);
        break;

      case F_RECV:
        /* Recv on socket (stream, datagram) */
        memset((void *)buffer, 0xCC, 16);
        io->rc = drv->SocketRecv (io->sock, buffer, 16);
        if ((io->rc > 0) && (memcmp ((const void *)buffer, (const void *)"SockServer", 10) != 0)) {
          /* Failed if rc <= 0 */
          io->rc = 0;
        }
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;

      case F_SEND_CTRL:
        /* Send control command to TestAssistant */
        sock = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        drv->SocketConnect (sock, ip_socket_server, 4, ASSISTANT_PORT);
        io->rc = drv->SocketSend (sock, io->cmd, strlen(io->cmd));
        drv->SocketClose (sock);
        osDelay (10);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketAccept
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketAccept verifies the WiFi Driver \b SocketAccept function:
\code
int32_t (*SocketAccept) (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
\endcode

Stream socket test:
 - Create stream socket
 - Bind socket
 - Start listening
 - Check function parameters 
 - Accept connection, NULL parameters
 - Receive ServerId on accepted socket
 - Close accepted socket
 - Accept connection again, return IP address and port
 - Receive ServerId on accepted socket
 - Receive again, server closed connection
 - Close accepted socket
 - Close listening socket
 - Accept again, closed socket

Datagram socket test:
 - Create datagram socket
 - Bind socket
 - Start listening
 - Accept connection, provide return parameters for IP address and port
 - Receive ServerId on socket
 - Close socket
*/
void WIFI_SocketAccept (void) {
  uint8_t      ip[4];
  uint32_t     ip_len;
  uint16_t     port;
  osThreadId_t worker;
  int32_t      rval;
  IO_ACCEPT    io;
  int32_t      sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Accept, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Start listening */
    io.sock = sock;
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ip_len = sizeof(ip);
    ARG_ACCEPT (-1, ip, &ip_len, &port);
    TH_EXECUTE (F_ACCEPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_ACCEPT (INT32_MIN, ip, &ip_len, &port);
    TH_EXECUTE (F_ACCEPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_ACCEPT (INT32_MAX, ip, &ip_len, &port);
    TH_EXECUTE (F_ACCEPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Parameters 'ip', 'ip_len' and 'port' are optional, can be NULL */

    /* Request a remote server to connect to us */
    io.cmd = CMD_CONNECT_TCP;
    TH_EXECUTE (F_SEND_CTRL, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc > 0);

    /* Accept connection with NULL parameters */
    ARG_ACCEPT (sock, NULL, NULL, NULL);
    TH_EXECUTE (F_ACCEPT, WIFI_SOCKET_TIMEOUT_LONG);
    /* Accepted socket should be different */
    TH_ASSERT  ((io.rc != io.sock) && (io.rc >= 0));

    /* Receive SockServer id string */
    io.sock = io.rc;
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT (io.rc > 0);

    /* Close accepted socket */
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (500);

    /* Request from remote server to connect to us */
    io.cmd = CMD_CONNECT_TCP;
    TH_EXECUTE (F_SEND_CTRL, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc > 0);

    /* Initialize buffers for return values */
    port   = 0;
    ip_len = sizeof(ip) + 1;
    memset ((void *)ip, 0, sizeof(ip));

    /* Accept again, return ip address and port */
    ARG_ACCEPT (sock, &ip[0], &ip_len, &port);
    TH_EXECUTE (F_ACCEPT, WIFI_SOCKET_TIMEOUT_LONG);
    /* Accepted socket should be different */
    TH_ASSERT  ((io.rc != io.sock) && (io.rc >= 0));
    /* IP address should be the address of the server */
    TH_ASSERT  ((memcmp ((const void *)ip, (const void *)ip_socket_server, 4) == 0) && (ip_len == 4));
    /* Port number of remote peer should be non-zero */
    TH_ASSERT  (port != 0);

    /* Receive SockServer id string */
    io.sock = io.rc;
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT (io.rc > 0);

    /* SockServer disconnects after 500ms */

    /* Receive again, no data */
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT_LONG);
    /* Should return error (connection reset) */
    /* Strict: ECONNRESET, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ECONNRESET), (io.rc == ARM_SOCKET_ERROR), "receive on disconnected socket", io.rc, ARM_SOCKET_ECONNRESET);

    /* Close accepted socket */
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Close listening socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Accept again, closed socket */
    ip_len = 4;
    ARG_ACCEPT (sock, &ip[0], &ip_len, &port);
    TH_EXECUTE (F_ACCEPT, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "accept on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Start listening */
    io.sock = sock;
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    /* Listen on datagram socket should fail */
    /* Strict: ENOTSUP, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTSUP), (io.rc == ARM_SOCKET_ERROR), "listen on datagram socket", io.rc, ARM_SOCKET_ENOTSUP);

    /* Initialize buffers for return values */
    port   = 0;
    ip_len = sizeof(ip);
    memset ((void *)ip, 0, sizeof(ip));

    /* Accept on datagram socket */
    ARG_ACCEPT (sock, &ip[0], &ip_len, &port);
    TH_EXECUTE (F_ACCEPT, WIFI_SOCKET_TIMEOUT);
    /* Accept on datagram socket should fail */
    /* Strict: ENOTSUP, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTSUP), (io.rc == ARM_SOCKET_ERROR), "accept on datagram socket", io.rc, ARM_SOCKET_ENOTSUP);

    osDelay (500);

    /* Request from remote server to send us a test message */
    io.cmd = CMD_CONNECT_UDP;
    TH_EXECUTE (F_SEND_CTRL, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc > 0);

    /* Receive SockServer id string */
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc > 0);

    /* Close socket */
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* Connect IO parameters */
typedef struct {
  int32_t        sock;
  const uint8_t *ip;
  uint32_t       ip_len;
  uint16_t       port;
  uint16_t       reserved;
  int32_t        rc;
  /* Control */
  osThreadId_t   owner;
  uint32_t       xid;
} IO_CONNECT;

/* Assign arguments */
#define ARG_CONNECT(_sock,_ip,_ip_len,_port) do {                   \
                                               io.sock   = _sock;   \
                                               io.ip     = _ip;     \
                                               io.ip_len = _ip_len; \
                                               io.port   = _port;   \
                                             } while (0)

/* Connect worker thread */
__NO_RETURN static void Th_Connect (IO_CONNECT *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_TCP | F_CREATE_UDP | F_BIND |
                               F_CONNECT    | F_LISTEN     | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_BIND:
        /* Bind socket */
        io->rc = drv->SocketBind (io->sock, ip_unspec, 4, DISCARD_PORT);
        break;

      case F_CONNECT:
        /* Connect on socket */
        io->rc = drv->SocketConnect (io->sock, io->ip, io->ip_len, io->port);
        break;

      case F_LISTEN:
        /* Listen on socket */
        io->rc = drv->SocketListen (io->sock, 1);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketConnect
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketConnect verifies the WiFi Driver \b SocketConnect function:
\code
int32_t (*SocketConnect) (int32_t socket, const uint8_t *ip, uint32_t  ip_len, uint16_t  port);
\endcode

Stream socket test 1:
 - Create stream socket
 - Check function parameters
 - Connect to server, blocking mode
 - Connect again, already connected
 - Bind connected socket
 - Close socket
 - Connect on closed socket

Stream socket test 2:
 - Create stream socket
 - Connect to server, connection rejected
 - Close socket

Stream socket test 3:
 - Create stream socket
 - Connect to server, non-responding or non-existent
 - Close socket

Stream socket test 4:
 - Create stream socket
 - Bind socket
 - Start listening
 - Connect to server, blocking mode
 - Close socket

Datagram socket test:
 - Create datagram socket
 - Bind socket
 - Check function parameters
 - Connect to server, enable address filtering
 - Connect to unspecified address, disable filtering
 - Close socket
 - Connect again, closed socket
*/
void WIFI_SocketConnect (void) {
  osThreadId_t worker;
  int32_t      rval;
  IO_CONNECT   io;
  int32_t      sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Connect, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Check parameter (socket = -1) */
    ARG_CONNECT(-1, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_CONNECT(INT32_MIN, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_CONNECT(INT32_MAX, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (ip = NULL) */
    ARG_CONNECT(sock, NULL, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (ip = 0.0.0.0) */
    ARG_CONNECT(sock, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (ip_len = 0) */
    ARG_CONNECT(sock, ip_socket_server, 0, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (ip_len = 5) */
    ARG_CONNECT(sock, ip_socket_server, 5, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (port = 0) */
    ARG_CONNECT(sock, ip_socket_server, 4, 0);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Connect to stream server */
    ARG_CONNECT(sock, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Connect 2nd time */
    ARG_CONNECT(sock, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket already connected) */
    /* Strict: EISCONN, valid non-strict: OK, ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EISCONN), ((io.rc == 0) || (io.rc == ARM_SOCKET_ERROR)), "connect socket to same address again", io.rc, ARM_SOCKET_EISCONN);

    /* Bind connected socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket already connected) */
    /* Strict: EISCONN, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EISCONN), (io.rc == ARM_SOCKET_ERROR), "bind on connected socket", io.rc, ARM_SOCKET_EISCONN);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Connect again, closed socket */
    ARG_CONNECT(sock, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "connect on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server (connection rejected) */
    ARG_CONNECT(sock, ip_socket_server, 4, TCP_REJECTED_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    /* Should return error (connection rejected by the peer) */
    /* Strict: ECONNREFUSED, valid non-strict: ETIMEDOUT, ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ECONNREFUSED), ((io.rc == ARM_SOCKET_ETIMEDOUT) || (io.rc == ARM_SOCKET_ERROR)), "connect to non-existent port", io.rc, ARM_SOCKET_ECONNREFUSED);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server (non-existent) */
    ARG_CONNECT(sock, ip_socket_server, 4, TCP_TIMEOUT_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    /* Should return error (connection timeout) */
    /* Strict: ETIMEDOUT, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ETIMEDOUT), (io.rc == ARM_SOCKET_ERROR), "connect to non-existent stream server", io.rc, ARM_SOCKET_ETIMEDOUT);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Start listening */
    io.sock = sock;
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Connect to stream server */
    ARG_CONNECT(sock, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    /* Connect on listening socket should fail */
    /* Strict: EINVAL, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EINVAL), (io.rc == ARM_SOCKET_ERROR), "connect on listening socket", io.rc, ARM_SOCKET_EINVAL);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Check parameter (socket = -1) */
    ARG_CONNECT(-1, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_CONNECT(INT32_MIN, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_CONNECT(INT32_MAX, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (ip = NULL) */
    ARG_CONNECT(sock, NULL, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (ip = 0.0.0.0) */
    ARG_CONNECT(sock, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    /* Datagram sockets may dissolve the association */
    /* by connecting to unspecified address.         */
    /* Strict: OK, valid non-strict: EINVAL, ERROR */
    TH_ASSERT2 ((io.rc == 0), ((io.rc == ARM_SOCKET_EINVAL) || (io.rc == ARM_SOCKET_ERROR)), "connect datagram socket to unspecified address (0.0.0.0)", io.rc, 0);

    /* Check parameter (ip_len = 0) */
    ARG_CONNECT(sock, ip_socket_server, 0, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (ip_len = 5) */
    ARG_CONNECT(sock, ip_socket_server, 5, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (port = 0) */
    ARG_CONNECT(sock, ip_socket_server, 4, 0);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Connect to datagram server */
    ARG_CONNECT(sock, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Connect to unspecified address (0.0.0.0) */
    ARG_CONNECT(sock, ip_unspec, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    /* Datagram sockets may dissolve the association */
    /* by connecting to unspecified address.         */
    /* Should return ok (socket address deleted) */
    /* Strict: OK, valid non-strict: EINVAL, ERROR */
    TH_ASSERT2 ((io.rc == 0), ((io.rc == ARM_SOCKET_EINVAL) || (io.rc == ARM_SOCKET_ERROR)), "connect datagram socket to unspecified address (0.0.0.0)", io.rc, 0);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Connect again, closed socket */
    ARG_CONNECT(sock, ip_socket_server, 4, DISCARD_PORT);
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "connect on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* Recv IO parameters */
typedef struct {
  int32_t      sock;
  uint8_t     *buf;
  uint32_t     len;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
  uint32_t     tval;
} IO_RECV;

/* Assign arguments */
#define ARG_RECV(_sock,_buf,_len) do {               \
                                    io.sock = _sock; \
                                    io.buf  = _buf;  \
                                    io.len  = _len;  \
                                  } while (0)

/* Recv worker thread */
__NO_RETURN static void Th_Recv (IO_RECV *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_TCP | F_BIND | F_CONNECT | F_LISTEN |
                               F_SETOPT     | F_RECV | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_BIND:
        /* Bind socket */
        io->rc = drv->SocketBind (io->sock, ip_unspec, 4, DISCARD_PORT);
        break;

      case F_CONNECT:
        /* Connect on socket */
        io->rc = drv->SocketConnect (io->sock, ip_socket_server, 4, (uint16_t)io->tval);
        break;

      case F_LISTEN:
        /* Listen on socket */
        io->rc = drv->SocketListen (io->sock, 1);
        break;

      case F_SETOPT:
        /* Set socket options */
        io->rc = drv->SocketSetOpt (io->sock, ARM_SOCKET_SO_RCVTIMEO, &io->tval, sizeof(io->tval));
        break;

      case F_RECV:
        /* Recv on socket */
        if (io->buf != NULL) {
          memset((void *)io->buf, 0xCC, io->len);
        }
        io->rc = drv->SocketRecv (io->sock, io->buf, io->len);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketRecv
\ingroup wifi_sock_api
\details
Test case \b WIFI_SocketRecv verifies the WiFi Driver \b SocketRecv function:
\code
int32_t (*SocketRecv) (int32_t socket, void *buf, uint32_t len);
\endcode

Stream socket test 1:
 - Create stream socket
 - Connect to Chargen server
 - Check function parameters
 - Receive data in blocking mode
 - Close socket
 - Receive again, closed socket

Stream socket test 2:
 - Create stream socket
 - Receive data, created socket
 - Bind socket
 - Receive data, bound socket
 - Start listening
 - Receive data, listening socket
 - Close socket

Stream socket test 3:
 - Create stream socket
 - Connect to Discard server
 - Set receive timeout to 1 sec
 - Receive data, timeout expires
 - Close socket
*/
void WIFI_SocketRecv (void) {
  uint8_t      buf[4];
  uint32_t     ticks,tout;
  osThreadId_t worker;
  int32_t      rval;
  IO_RECV      io;
  int32_t      sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Recv, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.sock = sock;
    io.tval = CHARGEN_PORT;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ARG_RECV   (-1, buf, sizeof(buf));
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_RECV   (INT32_MIN, buf, sizeof(buf));
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_RECV   (INT32_MAX, buf, sizeof(buf));
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (buf = NULL) */
    ARG_RECV   (sock, NULL, sizeof(buf));
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (len = 0) */
    ARG_RECV   (sock, buf, 0);
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Receive some data */
    ARG_RECV   (sock, buffer, sizeof(buffer));
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc >= 2);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Receive again, closed socket */
    ARG_RECV (sock, buffer, sizeof(buffer));
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "recv on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    /* Test server mode */
    sock = io.rc;

    /* Receive, created socket */
    ARG_RECV   (sock, buffer, sizeof(buffer));
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not connected) */
    /* Strict: ENOTCONN, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTCONN), (io.rc == ARM_SOCKET_ERROR), "recv on created socket", io.rc, ARM_SOCKET_ENOTCONN);

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Receive, bound socket */
    ARG_RECV   (sock, buffer, sizeof(buffer));
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not connected) */
    /* Strict: ENOTCONN, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTCONN), (io.rc == ARM_SOCKET_ERROR), "recv on bound socket", io.rc, ARM_SOCKET_ENOTCONN);

    /* Start listening */
    io.sock = sock;
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Receive, listening socket */
    ARG_RECV   (sock, buffer, sizeof(buffer));
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not connected) */
    /* Strict: ENOTCONN, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTCONN), (io.rc == ARM_SOCKET_ERROR), "recv on listening socket", io.rc, ARM_SOCKET_ENOTCONN);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.sock = sock;
    io.tval = DISCARD_PORT;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Set receive timeout to 1 sec */
    io.sock = sock;
    io.tval = 1000;
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Receive until timeout, no data */
    ARG_RECV   (sock, buffer, sizeof(buffer));
    ticks = GET_SYSTICK();
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT);
    tout = GET_SYSTICK() - ticks;
    /* Should return EAGAIN (operation timed out) */
    TH_ASSERT  (io.rc == ARM_SOCKET_EAGAIN);
    /* Check receive timeout is in the range of 0.9 to 1.1 sec */
    TH_ASSERT  (tout > SYSTICK_MICROSEC(900000) && tout < SYSTICK_MICROSEC(1100000));

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* RecvFrom IO parameters */
typedef struct {
  int32_t      sock;
  uint8_t     *buf;
  uint32_t     len;
  uint8_t     *ip;
  uint32_t    *ip_len;
  uint16_t    *port;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
  uint32_t     tout;
} IO_RECVFROM;

/* Assign arguments */
#define ARG_RECVFROM(_sock,_buf,_len,_ip,_ip_len,_port) do {                   \
                                                          io.sock   = _sock;   \
                                                          io.buf    = _buf;    \
                                                          io.len    = _len;    \
                                                          io.ip     = _ip;     \
                                                          io.ip_len = _ip_len; \
                                                          io.port   = _port;   \
                                                        } while (0)

/* RecvFrom worker thread */
__NO_RETURN static void Th_RecvFrom (IO_RECVFROM *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_UDP | F_CONNECT | F_SETOPT |
                               F_RECVFROM   | F_SEND    | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_CONNECT:
        /* Connect on socket */
        io->rc = drv->SocketConnect (io->sock, ip_socket_server, 4, CHARGEN_PORT);
        break;

      case F_SETOPT:
        /* Set socket options */
        io->rc = drv->SocketSetOpt (io->sock, ARM_SOCKET_SO_RCVTIMEO, &io->tout, sizeof(io->tout));
        break;

      case F_RECVFROM:
        /* RecvFrom on socket */
        if (io->buf != NULL) {
          memset((void *)io->buf, 0xCC, io->len);
        }
        io->rc = drv->SocketRecvFrom (io->sock, io->buf, io->len, io->ip, io->ip_len, io->port);
        break;

      case F_SEND:
        /* Send on socket */
        io->rc = drv->SocketSend (io->sock, "a", 1);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketRecvFrom
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketRecvFrom verifies the WiFi Driver \b SocketRecvFrom function:
\code
int32_t (*SocketRecvFrom) (int32_t socket, void *buf, uint32_t len, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
\endcode

Datagram socket test 1:
 - Create datagram socket
 - Connect to Chargen server
 - Check function parameters
 - Receive data in blocking mode
 - Set receive timeout to 1 sec
 - Receive again, timeout expires
 - Close socket
 - Receive again, closed socket
*/
void WIFI_SocketRecvFrom (void) {
  uint8_t      ip[4];
  uint32_t     ip_len,ticks,tout;
  uint16_t     port;
  uint8_t      buf[4];
  osThreadId_t worker;
  int32_t      rval;
  IO_RECVFROM  io;
  int32_t      sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_RecvFrom, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Connect to datagram server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ip_len = sizeof(ip);
    ARG_RECVFROM (-1, buf, sizeof(buf), ip, &ip_len, &port);
    TH_EXECUTE (F_RECVFROM, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_RECVFROM (INT32_MIN, buf, sizeof(buf), ip, &ip_len, &port);
    TH_EXECUTE (F_RECVFROM, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_RECVFROM (INT32_MAX, buf, sizeof(buf), ip, &ip_len, &port);
    TH_EXECUTE (F_RECVFROM, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (buf == NULL) */
    ARG_RECVFROM (sock, NULL, sizeof(buf), ip, &ip_len, &port);
    TH_EXECUTE (F_RECVFROM, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (len = 0) */
    ARG_RECVFROM (sock, buf, 0, ip, &ip_len, &port);
    TH_EXECUTE (F_RECVFROM, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Send one byte of data to trigger a reply */
    io.sock = sock;
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1);

    /* Initialize buffers for return values */
    port   = 0;
    ip_len = sizeof(ip) + 1;
    memset ((void *)ip, 0, sizeof(ip));
    
    /* Receive some data */
    ARG_RECVFROM (sock, buffer, sizeof(buffer), ip, &ip_len, &port);
    TH_EXECUTE (F_RECVFROM, WIFI_SOCKET_TIMEOUT_LONG);
    /* Should receive at least 2 bytes */
    TH_ASSERT  (io.rc >= 2);
    /* IP address should be the address of the server */
    TH_ASSERT  ((memcmp ((const void *)ip, (const void *)ip_socket_server, 4) == 0) && (ip_len == 4));
    /* Port number should be the port of the CHARGEN server */
    TH_ASSERT  (port == CHARGEN_PORT);

    /* Set receive timeout to 1 sec */
    io.sock = sock;
    io.tout = 1000;
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Receive until timeout, no data */
    ARG_RECVFROM (sock, buffer, sizeof(buffer), ip, &ip_len, &port);
    ticks = GET_SYSTICK();
    TH_EXECUTE (F_RECVFROM, WIFI_SOCKET_TIMEOUT);
    tout = GET_SYSTICK() - ticks;
    /* Should return EAGAIN (operation timed out) */
    TH_ASSERT  (io.rc == ARM_SOCKET_EAGAIN);
    /* Check receive timeout is in the range of 0.9 to 1.1 sec */
    TH_ASSERT  (tout > SYSTICK_MICROSEC(900000) && tout < SYSTICK_MICROSEC(1100000));

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Receive again, closed socket */
    ARG_RECVFROM (sock, buffer, sizeof(buffer), ip, &ip_len, &port);
    TH_EXECUTE (F_RECVFROM, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "recvfrom on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* Send IO parameters */
typedef struct {
  int32_t        sock;
  const uint8_t *buf;
  uint32_t       len;
  int32_t        rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_SEND;

/* Assign arguments */
#define ARG_SEND(_sock,_buf,_len) do {               \
                                    io.sock = _sock; \
                                    io.buf  = _buf;  \
                                    io.len  = _len;  \
                                  } while (0)

/* Send worker thread */
__NO_RETURN static void Th_Send (IO_SEND *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_TCP | F_BIND | F_CONNECT |
                               F_LISTEN     | F_SEND | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_BIND:
        /* Bind socket */
        io->rc = drv->SocketBind (io->sock, ip_unspec, 4, DISCARD_PORT);
        break;

      case F_CONNECT:
        /* Connect on socket */
        io->rc = drv->SocketConnect (io->sock, ip_socket_server, 4, DISCARD_PORT);
        break;

       case F_LISTEN:
        /* Listen on socket */
        io->rc = drv->SocketListen (io->sock, 1);
        break;

     case F_SEND:
        /* Send on socket */
        io->rc = drv->SocketSend (io->sock, io->buf, io->len);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketSend
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketSend verifies the WiFi Driver \b SocketSend function:
\code
int32_t (*SocketSend) (int32_t socket, const void *buf, uint32_t len);
\endcode

Stream socket test 1:
 - Create stream socket
 - Connect to server, blocking mode
 - Check function parameters
 - Send data, blocking mode
 - Close socket
 - Send again, closed socket

Stream socket test 2:
 - Create stream socket
 - Connect to server, blocking mode
 - Send ESC data, server disconnects
 - Send again, disconnected socket
 - Close socket

Stream socket test 3:
 - Create stream socket
 - Send data, created socket
 - Bind socket
 - Send data, bound socket
 - Start listening
 - Send data, listening socket
 - Close socket
 - Send again, closed socket
*/
void WIFI_SocketSend (void) {
  osThreadId_t worker;
  int32_t      rval;
  IO_SEND      io;
  int32_t      sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Send, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ARG_SEND   (-1, test_msg, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_SEND   (INT32_MIN, test_msg, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_SEND   (INT32_MAX, test_msg, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (buf = NULL) */
    ARG_SEND   (sock, NULL, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (len = 0) */
    ARG_SEND   (sock, test_msg, 0);
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Send some data */
    ARG_SEND   (sock, test_msg, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == sizeof(test_msg));

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Send again, closed socket */
    ARG_SEND   (sock, test_msg, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "send on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Send ESC command, server disconnects */
    ARG_SEND   (sock, (uint8_t *)"\x1B", 1);
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1);

    /* Wait for the server to disconnect */
    osDelay (200);

    /* Send again, disconnected socket */
    ARG_SEND   (sock, test_msg, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (connection reset by the peer) */
    /* Strict: ECONNRESET, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ECONNRESET), (io.rc == ARM_SOCKET_ERROR), "send on disconnected socket", io.rc, ARM_SOCKET_ECONNRESET);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Send data, created socket */
    ARG_SEND   (sock, test_msg, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not connected) */
    /* Strict: ENOTCONN, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTCONN), (io.rc == ARM_SOCKET_ERROR), "send on created socket", io.rc, ARM_SOCKET_ENOTCONN);

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Send data, bound socket */
    ARG_SEND (sock, test_msg, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not connected) */
    /* Strict: ENOTCONN, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTCONN), (io.rc == ARM_SOCKET_ERROR), "send on bound socket", io.rc, ARM_SOCKET_ENOTCONN);

    /* Start listening */
    io.sock = sock;
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Send data, listening socket */
    ARG_SEND (sock, test_msg, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not connected) */
    /* Strict: ENOTCONN, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTCONN), (io.rc == ARM_SOCKET_ERROR), "send on listening socket", io.rc, ARM_SOCKET_ENOTCONN);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Send again, closed socket */
    ARG_SEND (sock, test_msg, sizeof(test_msg));
    TH_EXECUTE (F_SEND, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "send on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* SendTo IO parameters */
typedef struct {
  int32_t        sock;
  const uint8_t *buf;
  uint32_t       len;
  const uint8_t *ip;
  uint32_t       ip_len;
  uint16_t       port;
  uint16_t       reserved;
  int32_t        rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_SENDTO;

/* Assign arguments */
#define ARG_SENDTO(_sock,_buf,_len,_ip,_ip_len,_port) do {                   \
                                                        io.sock   = _sock;   \
                                                        io.buf    = _buf;    \
                                                        io.len    = _len;    \
                                                        io.ip     = _ip;     \
                                                        io.ip_len = _ip_len; \
                                                        io.port   = _port;   \
                                                      } while (0)

/* SendTo worker thread */
__NO_RETURN static void Th_SendTo (IO_SENDTO *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_UDP | F_SENDTO | F_RECV | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_SENDTO:
        /* SendTo on socket */
        io->rc = drv->SocketSendTo (io->sock, io->buf, io->len, io->ip, io->ip_len, io->port);
        break;

      case F_RECV:
        /* Recv on socket */
        memset((void *)buffer, 0xCC, sizeof(buffer));
        io->rc = drv->SocketRecv (io->sock, buffer, sizeof(buffer));
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketSendTo
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketSend verifies the WiFi Driver \b SocketSendTo function:
\code
int32_t (*SocketSendTo) (int32_t socket, const void *buf, uint32_t len, const uint8_t *ip, uint32_t ip_len, uint16_t port);
\endcode

Datagram socket test:
 - Create datagram socket
 - Check function parameters
 - Send data, blocking mode
 - Receive echo data, verify if the same
 - Close socket
 - Send again, closed socket
*/
void WIFI_SocketSendTo (void) {
  osThreadId_t worker;
  int32_t      rval;
  IO_SENDTO    io;
  int32_t      sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_SendTo, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Check parameter (socket = -1) */
    ARG_SENDTO (-1, test_msg, sizeof(test_msg), ip_socket_server, 4, ECHO_PORT);
    TH_EXECUTE (F_SENDTO, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_SENDTO (INT32_MIN, test_msg, sizeof(test_msg), ip_socket_server, 4, ECHO_PORT);
    TH_EXECUTE (F_SENDTO, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_SENDTO (INT32_MAX, test_msg, sizeof(test_msg), ip_socket_server, 4, ECHO_PORT);
    TH_EXECUTE (F_SENDTO, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (buf == NULL) */
    ARG_SENDTO (sock, NULL, sizeof(test_msg), ip_socket_server, 4, ECHO_PORT);
    TH_EXECUTE (F_SENDTO, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (len = 0) */
    ARG_SENDTO (sock, test_msg, 0, ip_socket_server, 4, ECHO_PORT);
    TH_EXECUTE (F_SENDTO, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Send some data */
    ARG_SENDTO (sock, test_msg, sizeof(test_msg), ip_socket_server, 4, ECHO_PORT);
    TH_EXECUTE (F_SENDTO, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == sizeof(test_msg));

    /* Receive the echoed data */
    io.sock = sock;
    TH_EXECUTE (F_RECV, WIFI_SOCKET_TIMEOUT_LONG);
    /* Should receive the same data (ECHO protocol) */
    TH_ASSERT  ((io.rc == sizeof(test_msg)) && (memcmp ((const void *)test_msg, (const void *)buffer, sizeof(test_msg)) == 0));

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Send again, closed socket */
    ARG_SENDTO (sock, test_msg, sizeof(test_msg), ip_socket_server, 4, ECHO_PORT);
    TH_EXECUTE (F_SENDTO, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "sendto on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* GetSockName IO parameters */
typedef struct {
  int32_t      sock;
  uint8_t     *ip;
  uint32_t    *ip_len;
  uint16_t    *port;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_GETSOCKNAME;

/* Assign arguments */
#define ARG_GETSOCKNAME(_sock,_ip,_ip_len,_port) do {                   \
                                                   io.sock   = _sock;   \
                                                   io.ip     = _ip;     \
                                                   io.ip_len = _ip_len; \
                                                   io.port   = _port;   \
                                                 } while (0)

/* GetSockName worker thread */
__NO_RETURN static void Th_GetSockName (IO_GETSOCKNAME *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_TCP | F_CREATE_UDP  | F_BIND |
                               F_CONNECT    | F_GETSOCKNAME | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_BIND:
        /* Bind socket */
        io->rc = drv->SocketBind (io->sock, ip_unspec, 4, DISCARD_PORT);
        break;

      case F_CONNECT:
        /* Connect on socket */
        io->rc = drv->SocketConnect (io->sock, ip_socket_server, 4, DISCARD_PORT);
        break;

      case F_GETSOCKNAME:
        /* Get socket name */
        io->rc = drv->SocketGetSockName (io->sock, io->ip, io->ip_len, io->port);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketGetSockName
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketGetSockName verifies the WiFi Driver \b SocketGetSockName function:
\code
int32_t (*SocketGetSockName) (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
\endcode

Stream socket test 1:
 - Create stream socket
 - Connect to server, blocking mode
 - Check function parameters
 - Get socket name
 - Close socket
 - Get socket name again, closed socket

Stream socket test 1:
 - Create stream socket
 - Get socket name, not bound
 - Bind socket
 - Get socket name, bound
 - Close socket

Datagram socket test 1:
 - Create datagram socket
 - Connect to server, enable packet filtering
 - Check function parameters
 - Get socket name
 - Close socket
 - Get socket name again, closed socket

Datagram socket test 1:
 - Create datagram socket
 - Get socket name, not bound
 - Bind socket
 - Get socket name, bound
 - Close socket
*/
void WIFI_SocketGetSockName (void) {
  uint8_t        local_ip[4];
  uint16_t       local_port;
  uint32_t       ip_len;
  osThreadId_t   worker;
  int32_t        rval;
  IO_GETSOCKNAME io;
  int32_t        sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_GetSockName, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    /* Test client mode */
    sock = io.rc;

    /* Connect to stream server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ip_len = sizeof(local_ip);
    ARG_GETSOCKNAME (-1, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_GETSOCKNAME (INT32_MIN, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_GETSOCKNAME (INT32_MAX, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (port = NULL) */
    ip_len = sizeof(local_ip);
    ARG_GETSOCKNAME (sock, local_ip, &ip_len, NULL);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    /* Request IP address only should be accepted */
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (ip = NULL, ip_len = NULL) */
    ARG_GETSOCKNAME (sock, NULL, NULL, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    /* Request port only should be accepted */
    TH_ASSERT  (io.rc == 0);

    /* Initialize buffers for return values */
    local_port = 0;
    ip_len     = sizeof(local_ip) + 1;
    memcpy (local_ip, ip_bcast, sizeof(local_ip));

    /* Retrieve socket name */
    ARG_GETSOCKNAME (sock, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* IP address should be different from broadcast */
    TH_ASSERT  ((memcmp ((const void *)local_ip, (const void *)ip_bcast, 4) != 0) && (ip_len == 4));
    /* Port number should be non-zero */
    TH_ASSERT  (local_port != 0);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Retrieve socket name again */
    ARG_GETSOCKNAME (sock, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "getsockname on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    /* Test server mode */
    sock = io.rc;

    /* Retrieve socket name, not bound */
    ARG_GETSOCKNAME (sock, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not bound) */
    /* Strict: EINVAL, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EINVAL), (io.rc == ARM_SOCKET_ERROR), "getsockname on unbound socket", io.rc, ARM_SOCKET_EINVAL);

    /* Initialize buffers for return values */
    local_port = 0;
    ip_len     = sizeof(local_ip) + 1;
    memcpy (local_ip, ip_bcast, sizeof(local_ip));

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Retrieve socket name, bound */
    ARG_GETSOCKNAME (sock, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* IP address should be unspecified */
    TH_ASSERT  ((memcmp ((const void *)local_ip, (const void *)ip_unspec, 4) == 0) && (ip_len == 4));
    /* Port number should be listening port */
    TH_ASSERT  (local_port == DISCARD_PORT);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    /* Test client mode */
    sock = io.rc;

    /* Connect to datagram server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ARG_GETSOCKNAME (-1, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_GETSOCKNAME (INT32_MIN, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_GETSOCKNAME (INT32_MAX, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (port = NULL) */
    ip_len = sizeof(local_ip);
    ARG_GETSOCKNAME (sock, local_ip, &ip_len, NULL);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    /* Request IP address only should be accepted */
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (ip = NULL, ip_len = NULL) */
    ARG_GETSOCKNAME (sock, NULL, NULL, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    /* Request port only should be accepted */
    TH_ASSERT  (io.rc == 0);

    /* Initialize buffers for return values */
    local_port = 0;
    ip_len     = sizeof(local_ip) + 1;
    memcpy (local_ip, ip_bcast, sizeof(local_ip));

    /* Retrieve socket name */
    ARG_GETSOCKNAME (sock, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* IP address should be different from broadcast */
    TH_ASSERT  ((memcmp ((const void *)local_ip, (const void *)ip_bcast, 4) != 0) && (ip_len == 4));
    /* Port number should be non-zero */
    TH_ASSERT  (local_port != 0);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Retrieve socket name again */
    ARG_GETSOCKNAME (sock, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "getsockname on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    /* Test server mode */
    sock = io.rc;

    /* Retrieve socket name, not bound */
    ARG_GETSOCKNAME (sock, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not bound) */
    /* Strict: EINVAL, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_EINVAL), (io.rc == ARM_SOCKET_ERROR), "getsockname on unbound socket", io.rc, ARM_SOCKET_EINVAL);

    /* Initialize buffers for return values */
    local_port = 0;
    ip_len     = sizeof(local_ip) + 1;
    memcpy (local_ip, ip_bcast, sizeof(local_ip));

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Retrieve socket name, bound */
    ARG_GETSOCKNAME (sock, local_ip, &ip_len, &local_port);
    TH_EXECUTE (F_GETSOCKNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* IP address should be unspecified */
    TH_ASSERT  ((memcmp ((const void *)local_ip, (const void *)ip_unspec, 4) == 0) && (ip_len == 4));
    /* Port number should be listening port */
    TH_ASSERT  (local_port == DISCARD_PORT);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* GetPeerName IO parameters */
typedef struct {
  int32_t      sock;
  uint8_t     *ip;
  uint32_t    *ip_len;
  uint16_t    *port;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_GETPEERNAME;

/* Assign arguments */
#define ARG_GETPEERNAME(_sock,_ip,_ip_len,_port) do {                   \
                                                   io.sock   = _sock;   \
                                                   io.ip     = _ip;     \
                                                   io.ip_len = _ip_len; \
                                                   io.port   = _port;   \
                                                 } while (0)

/* GetPeerName worker thread */
__NO_RETURN static void Th_GetPeerName (IO_GETPEERNAME *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_TCP | F_CREATE_UDP  | F_BIND | F_CONNECT |
                               F_LISTEN     | F_GETPEERNAME | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_BIND:
        /* Bind socket */
        io->rc = drv->SocketBind (io->sock, ip_unspec, 4, DISCARD_PORT);
        break;

      case F_CONNECT:
        /* Connect on socket */
        io->rc = drv->SocketConnect (io->sock, ip_socket_server, 4, DISCARD_PORT);
        break;

       case F_LISTEN:
        /* Listen on socket */
        io->rc = drv->SocketListen (io->sock, 1);
        break;

      case F_GETPEERNAME:
        /* Get peer name  */
        io->rc = drv->SocketGetPeerName (io->sock, io->ip, io->ip_len, io->port);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketGetPeerName
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketGetPeerName verifies the WiFi Driver \b SocketGetPeerName function:
\code
int32_t (*SocketGetPeerName) (int32_t socket, uint8_t *ip, uint32_t *ip_len, uint16_t *port);
\endcode

Stream socket test  1:
 - Create stream socket
 - Connect to server, blocking mode
 - Check function parameters
 - Get peer name
 - Close socket
 - Get peer name, closed socket

Stream socket test  2:
 - Create stream socket
 - Get peer name, created socket
 - Bind socket
 - Get peer name, bound socket
 - Start listening
 - Get peer name, listening socket
 - Close socket

Datagram socket test:
 - Create datagram socket
 - Connect to server, enable packet filtering
 - Check function parameters
 - Get peer name
 - Close socket
 - Get peer name, closed socket
*/
void WIFI_SocketGetPeerName (void) {
  uint8_t        peer_ip[4];
  uint16_t       peer_port;
  uint32_t       ip_len;
  osThreadId_t   worker;
  int32_t        rval;
  IO_GETPEERNAME io;
  int32_t        sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_GetPeerName, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ip_len = sizeof(peer_ip);
    ARG_GETPEERNAME (-1, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_GETPEERNAME (INT32_MIN, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_GETPEERNAME (INT32_MAX, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (port = NULL) */
    ip_len = sizeof(peer_ip);
    ARG_GETPEERNAME (sock, peer_ip, &ip_len, NULL);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    /* Request IP address only should be accepted */
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (ip = NULL, ip_len = NULL) */
    ARG_GETPEERNAME (sock, NULL, NULL, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    /* Request port only should be accepted */
    TH_ASSERT  (io.rc == 0);

    /* Initialize buffers for return values */
    peer_port = 0;
    ip_len    = sizeof(peer_ip) + 1;
    memcpy (peer_ip, ip_bcast, sizeof(peer_ip));

    /* Retrieve peer name */
    ARG_GETPEERNAME (sock, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* IP address should be the address of the server */
    TH_ASSERT  ((memcmp ((const void *)peer_ip, (const void *)ip_socket_server, 4) == 0) && (ip_len == 4));
    /* Port number should be the DISCARD port */
    TH_ASSERT  (peer_port == DISCARD_PORT);

    /* Close stream socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Retrieve peer name again */
    ARG_GETPEERNAME (sock, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "getpeername on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Get peer name, created socket */
    ARG_GETPEERNAME (sock, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not connected) */
    /* Strict: ENOTCONN, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTCONN), (io.rc == ARM_SOCKET_ERROR), "getpeername on created socket", io.rc, ARM_SOCKET_ENOTCONN);

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Get peer name, bound socket */
    ARG_GETPEERNAME (sock, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not connected) */
    /* Strict: ENOTCONN, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTCONN), (io.rc == ARM_SOCKET_ERROR), "getpeername on bound socket", io.rc, ARM_SOCKET_ENOTCONN);

    /* Start listening */
    io.sock = sock;
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Get peer name, listening socket */
    ARG_GETPEERNAME (sock, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not connected) */
    /* Strict: ENOTCONN, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ENOTCONN), (io.rc == ARM_SOCKET_ERROR), "getpeername on listening socket", io.rc, ARM_SOCKET_ENOTCONN);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Connect to datagram server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ip_len =  sizeof(peer_ip);
    ARG_GETPEERNAME (-1, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_GETPEERNAME (INT32_MIN, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_GETPEERNAME (INT32_MAX, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (port = NULL) */
    ip_len = sizeof(peer_ip);
    ARG_GETPEERNAME (sock, peer_ip, &ip_len, NULL);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    /* Request IP address only should be accepted */
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (ip = NULL, ip_len = NULL) */
    ARG_GETPEERNAME (sock, NULL, NULL, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    /* Request port only should be accepted */
    TH_ASSERT  (io.rc == 0);

    /* Initialize buffers for return values */
    peer_port = 0;
    ip_len    = sizeof(peer_ip) + 1;
    memcpy (peer_ip, ip_bcast, sizeof(peer_ip));

    /* Retrieve peer name */
    ARG_GETPEERNAME (sock, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* IP address should be the address of the server */
    TH_ASSERT  ((memcmp ((const void *)peer_ip, (const void *)ip_socket_server, 4) == 0) && (ip_len == 4));
    /* Port number should be the DISCARD port */
    TH_ASSERT  (peer_port == DISCARD_PORT);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Retrieve peer name again */
    ARG_GETPEERNAME (sock, peer_ip, &ip_len, &peer_port);
    TH_EXECUTE (F_GETPEERNAME, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "getpeername on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* GetOpt IO parameters */
typedef struct {
  int32_t      sock;
  int32_t      opt_id;
  void        *opt_val;
  uint32_t    *opt_len;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_GETOPT;

/* Assign arguments */
#define ARG_GETOPT(_sock,_opt_id,_opt_val,_opt_len) do {                     \
                                                      io.sock    = _sock;    \
                                                      io.opt_id  = _opt_id;  \
                                                      io.opt_val = _opt_val; \
                                                      io.opt_len = _opt_len; \
                                                    } while (0)

/* GetOpt worker thread */
__NO_RETURN static void Th_GetOpt (IO_GETOPT *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_TCP | F_CREATE_UDP |
                               F_GETOPT     | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_GETOPT:
        /* Get socket options */
        io->rc = drv->SocketGetOpt (io->sock, io->opt_id, io->opt_val, io->opt_len);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketGetOpt
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketGetOpt verifies the WiFi Driver \b SocketGetOpt function:
\code
int32_t (*SocketGetOpt) (int32_t socket, int32_t opt_id, void *opt_val, uint32_t *opt_len);
\endcode

Stream socket test:
 - Create stream socket
 - Check function parameters
 - Get socket options
 - Close socket
 - Get socket options again, closed socket

Datagram socket test:
 - Create datagram socket
 - Get socket type
 - Close socket
 - Get socket type
*/
void WIFI_SocketGetOpt (void) {
  uint32_t     opt_val;
  uint32_t     opt_len;
  osThreadId_t worker;
  int32_t      rval;
  IO_GETOPT    io;
  int32_t      sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_GetOpt, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;
 
    /* Check parameter (socket = -1) */
    opt_len = sizeof(opt_val);
    ARG_GETOPT (-1, ARM_SOCKET_SO_TYPE, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_GETOPT (INT32_MIN, ARM_SOCKET_SO_TYPE, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_GETOPT (INT32_MAX, ARM_SOCKET_SO_TYPE, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (opt_id = -1) */
    ARG_GETOPT (sock, -1, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (opt_id = INT32_MIN) */
    ARG_GETOPT (sock, INT32_MIN, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (opt_id = INT32_MAX) */
    ARG_GETOPT (sock, INT32_MAX, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (opt_val = NULL) */
    ARG_GETOPT (sock, ARM_SOCKET_SO_TYPE, NULL, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (opt_len = NULL) */
    ARG_GETOPT (sock, ARM_SOCKET_SO_TYPE, &opt_val, NULL);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (*opt_len = 0) */
    opt_len = 0;
    ARG_GETOPT (sock, ARM_SOCKET_SO_TYPE, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (*opt_len = 5) */
    opt_len = 5;
    ARG_GETOPT (sock, ARM_SOCKET_SO_TYPE, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  ((io.rc == 0) && (opt_len == 4));

    /* Get option FIONBIO (set only) */
    opt_len = sizeof(opt_val);
    ARG_GETOPT (sock, ARM_SOCKET_IO_FIONBIO, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Get option RCVTIMEO */
    opt_len = sizeof(opt_val) + 1;
    opt_val = 0xE2A5A241;
    ARG_GETOPT (sock, ARM_SOCKET_SO_RCVTIMEO, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* Should be different from the initial value */
    TH_ASSERT  ((opt_val != 0xE2A5A241) && (opt_len == 4));

    /* Get option SNDTIMEO */
    opt_len = sizeof(opt_val) + 1;
    opt_val = 0xE2A5A241;
    ARG_GETOPT (sock, ARM_SOCKET_SO_SNDTIMEO, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* Should be different from the initial value */
    TH_ASSERT  ((opt_val != 0xE2A5A241) && (opt_len == 4));

    /* Get option KEEPALIVE */
    opt_len = sizeof(opt_val) + 1;
    opt_val = 0xE2A5A241;
    ARG_GETOPT (sock, ARM_SOCKET_SO_KEEPALIVE, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* Should be different from the initial value */
    TH_ASSERT  ((opt_val != 0xE2A5A241) && (opt_len == 4));

    /* Get option socket TYPE */
    opt_len = sizeof(opt_val) + 1;
    opt_val = UINT32_MAX;
    ARG_GETOPT (sock, ARM_SOCKET_SO_TYPE, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* Should be SOCK_STREAM type */
    TH_ASSERT  ((opt_val == ARM_SOCKET_SOCK_STREAM) && (opt_len == 4));

    /* Close stream socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Get option socket TYPE again */
    opt_len = sizeof(opt_val);
    ARG_GETOPT (sock, ARM_SOCKET_SO_TYPE, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "getsockopt on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Get option socket TYPE */
    opt_len = sizeof(opt_val) + 1;
    opt_val = UINT32_MAX;
    ARG_GETOPT (sock, ARM_SOCKET_SO_TYPE, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);
    /* Should be SOCK_DGRAM type */
    TH_ASSERT  ((opt_val == ARM_SOCKET_SOCK_DGRAM) && (opt_len == 4));

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Get option socket TYPE again */
    opt_len = sizeof(opt_val);
    ARG_GETOPT (sock, ARM_SOCKET_SO_TYPE, &opt_val, &opt_len);
    TH_EXECUTE (F_GETOPT, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "getsockopt on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* SetOpt IO parameters */
typedef struct {
  int32_t      sock;
  int32_t      opt_id;
  const void  *opt_val;
  uint32_t     opt_len;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_SETOPT;

/* Assign arguments */
#define ARG_SETOPT(_sock,_opt_id,_opt_val,_opt_len) do {                     \
                                                      io.sock    = _sock;    \
                                                      io.opt_id  = _opt_id;  \
                                                      io.opt_val = _opt_val; \
                                                      io.opt_len = _opt_len; \
                                                    } while (0)

/* SetOpt worker thread */
__NO_RETURN static void Th_SetOpt (IO_SETOPT *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_TCP | F_CREATE_UDP | F_SETOPT | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_SETOPT:
        /* Set socket options */
        io->rc = drv->SocketSetOpt (io->sock, io->opt_id, io->opt_val, io->opt_len);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketSetOpt
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketSetOpt verifies the WiFi Driver \b SocketSetOpt function:
\code
int32_t (*SocketSetOpt) (int32_t socket, int32_t opt_id, const void *opt_val, uint32_t opt_len);
\endcode

Stream socket test:
 - Create stream socket
 - Check function parameters
 - Set socket options
 - Close socket
 - Set socket option again, closed socket

Datagram socket test:
 - Create datagram socket
 - Set socket options
 - Close socket
 - Set socket option again, closed socket
*/

void WIFI_SocketSetOpt (void) {
  uint32_t     opt_val;
  osThreadId_t worker;
  int32_t      rval;
  IO_SETOPT    io;
  int32_t      sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_SetOpt, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;
 
    /* Check parameter (socket = -1) */
    opt_val = 5000;
    ARG_SETOPT (-1, ARM_SOCKET_SO_RCVTIMEO, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_SETOPT (-1, ARM_SOCKET_SO_RCVTIMEO, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_SETOPT (-1, ARM_SOCKET_SO_RCVTIMEO, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (opt_id = -1) */
    ARG_SETOPT (sock, -1, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (opt_id = INT32_MIN) */
    ARG_SETOPT (sock, INT32_MIN, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (opt_id = INT32_MAX) */
    ARG_SETOPT (sock, INT32_MAX, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (opt_val = NULL) */
    ARG_SETOPT (sock, ARM_SOCKET_SO_TYPE, NULL, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (opt_len = 0) */
    ARG_SETOPT (sock, ARM_SOCKET_SO_TYPE, &opt_val, 0);
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Check parameter (opt_len = 3) */
    ARG_SETOPT (sock, ARM_SOCKET_SO_TYPE, &opt_val, 3);
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Set option FIONBIO (set only) */
    opt_val = 0;
    ARG_SETOPT (sock, ARM_SOCKET_IO_FIONBIO, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Set option RCVTIMEO */
    opt_val = 5000;
    ARG_SETOPT (sock, ARM_SOCKET_SO_RCVTIMEO, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Set option SNDTIMEO */
    opt_val = 2000;
    ARG_SETOPT (sock, ARM_SOCKET_SO_SNDTIMEO, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Set option KEEPALIVE */
    opt_val = 1;
    ARG_SETOPT (sock, ARM_SOCKET_SO_KEEPALIVE, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Set option socket TYPE (get only) */
    opt_val = ARM_SOCKET_SOCK_STREAM;
    ARG_SETOPT (sock, ARM_SOCKET_SO_TYPE, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

    /* Close stream socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Set option RCVTIMEO again */
    opt_val = 5000;
    ARG_SETOPT (sock, ARM_SOCKET_SO_RCVTIMEO, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "setsockopt on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Set option RCVTIMEO */
    opt_val = 5000;
    ARG_SETOPT (sock, ARM_SOCKET_SO_RCVTIMEO, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Close socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Set option RCVTIMEO again */
    ARG_SETOPT (sock, ARM_SOCKET_SO_RCVTIMEO, &opt_val, sizeof(opt_val));
    TH_EXECUTE (F_SETOPT, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: ERROR */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == ARM_SOCKET_ERROR), "setsockopt on closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* Close IO parameters */
typedef struct {
  int32_t      sock;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_CLOSE;

/* Assign arguments */
#define ARG_CLOSE(_sock) do {               \
                           io.sock = _sock; \
                         } while (0)

/* Close worker thread */
__NO_RETURN static void Th_Close (IO_CLOSE *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_TCP | F_CREATE_UDP | F_BIND |
                               F_CONNECT    | F_LISTEN     | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_BIND:
        /* Bind socket */
        io->rc = drv->SocketBind (io->sock, ip_unspec, 4, DISCARD_PORT);
        break;

      case F_CONNECT:
        /* Connect on socket */
        io->rc = drv->SocketConnect (io->sock, ip_socket_server, 4, DISCARD_PORT);
        break;

      case F_LISTEN:
        /* Listen on socket */
        io->rc = drv->SocketListen (io->sock, 1);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketClose
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketClose verifies the WiFi Driver \b SocketClose function:
\code
int32_t (*SocketClose) (int32_t socket);
\endcode

Stream socket test 1:
 - Create stream socket
 - Bind socket
 - Connect to server
 - Check function parameters
 - Close socket
 - Close socket again

Stream socket test 2:
 - Create stream socket
 - Bind socket
 - Start listening
 - Close socket
 - Close socket again

Datagram socket test:
 - Create datagram socket
 - Bind socket
 - Check function parameters
 - Close socket
 - Close socket again
*/
void WIFI_SocketClose (void) {
  osThreadId_t worker;
  int32_t      rval;
  IO_CLOSE     io;
  int32_t      sock;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Close, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ARG_CLOSE  (-1);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_CLOSE  (INT32_MIN);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_CLOSE  (INT32_MAX);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Close socket */
    ARG_CLOSE  (sock);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Close again, closed socket */
    ARG_CLOSE  (sock);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: OK */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == 0), "close already closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Start listening */
    io.sock = sock;
    TH_EXECUTE (F_LISTEN, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Close socket */
    ARG_CLOSE (sock);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Close again, closed socket */
    ARG_CLOSE (sock);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: OK */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == 0), "close already closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Bind socket */
    io.sock = sock;
    TH_EXECUTE (F_BIND, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Check parameter (socket = -1) */
    ARG_CLOSE  (-1);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MIN) */
    ARG_CLOSE  (INT32_MIN);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Check parameter (socket = INT32_MAX) */
    ARG_CLOSE  (INT32_MAX);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == ARM_SOCKET_ESOCK);

    /* Close socket */
    ARG_CLOSE  (sock);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Close again, closed socket */
    ARG_CLOSE  (sock);
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    /* Should return error (socket not created) */
    /* Strict: ESOCK, valid non-strict: OK */
    TH_ASSERT2 ((io.rc == ARM_SOCKET_ESOCK), (io.rc == 0), "close already closed socket", io.rc, ARM_SOCKET_ESOCK);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* GetHostByName IO parameters */
typedef struct {
  const char  *name;
  int32_t      af;
  uint8_t     *ip;
  uint32_t    *ip_len;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_GETHOST;

/* Assign arguments */
#define ARG_GETHOST(_name,_af,_ip,_ip_len) do {                   \
                                             io.name   = _name;   \
                                             io.af     = _af;     \
                                             io.ip     = _ip;     \
                                             io.ip_len = _ip_len; \
                                           } while (0)

/* GetHostByName worker thread */
__NO_RETURN static void Th_GetHostByName (IO_GETHOST *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_GETHOSTBYNAME, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_GETHOSTBYNAME:
        /* Resolve host */
        io->rc = drv->SocketGetHostByName (io->name, io->af, io->ip, io->ip_len);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_SocketGetHostByName
\ingroup wifi_sock_api
\details
The test case \b WIFI_SocketGetHostByName the WiFi Driver \b SocketGetHostByName function:
\code
int32_t (*SocketGetHostByName) (const char *name, int32_t af, uint8_t *ip, uint32_t *ip_len);
\endcode

Function test:
 - Check function parameters
 - Resolve host
 - Resolve non-existent host

\note
This test requires internet connectivity to DNS server.
*/
void WIFI_SocketGetHostByName (void) {
  const char  *host_name = "www.arm.com";
  uint8_t      host_ip[4];
  uint32_t     ip_len;
  osThreadId_t worker;
  int32_t      rval;
  IO_GETHOST   io;

  if (socket_funcs_exist == 0U) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Socket functions not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_GetHostByName, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Check parameter (name = NULL) */
  ip_len = sizeof(host_ip);
  ARG_GETHOST(NULL, ARM_SOCKET_AF_INET, host_ip, &ip_len);
  TH_EXECUTE (F_GETHOSTBYNAME, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (af = -1) */
  ARG_GETHOST(host_name, -1, host_ip, &ip_len);
  TH_EXECUTE (F_GETHOSTBYNAME, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (af = INT32_MIN) */
  ARG_GETHOST(host_name, INT32_MIN, host_ip, &ip_len);
  TH_EXECUTE (F_GETHOSTBYNAME, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (af = INT32_MAX) */
  ARG_GETHOST(host_name, INT32_MAX, host_ip, &ip_len);
  TH_EXECUTE (F_GETHOSTBYNAME, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (ip = NULL) */
  ARG_GETHOST(host_name, ARM_SOCKET_AF_INET, NULL, &ip_len);
  TH_EXECUTE (F_GETHOSTBYNAME, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (ip_len = NULL) */
  ARG_GETHOST(host_name, ARM_SOCKET_AF_INET, host_ip, NULL);
  TH_EXECUTE (F_GETHOSTBYNAME, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (*ip_len = 0) */
  ip_len = 0;
  ARG_GETHOST(host_name, ARM_SOCKET_AF_INET, host_ip, &ip_len);
  TH_EXECUTE (F_GETHOSTBYNAME, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Check parameter (*ip_len = 3) */
  ip_len = 3;
  ARG_GETHOST(host_name, ARM_SOCKET_AF_INET, host_ip, &ip_len);
  TH_EXECUTE (F_GETHOSTBYNAME, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_SOCKET_EINVAL);

  /* Resolve valid host */
  ip_len = sizeof(host_ip) + 1;
  memset((void *)host_ip, 0, sizeof(host_ip));
  ARG_GETHOST(host_name, ARM_SOCKET_AF_INET, host_ip, &ip_len);
  TH_EXECUTE (F_GETHOSTBYNAME, WIFI_SOCKET_TIMEOUT_LONG);
  /* IP address should be valid */
  TH_ASSERT  ((memcmp((const void *)host_ip, (const void *)ip_unspec, 4) != 0) && (ip_len == 4));

  /* Resolve non-existent host */
  ip_len = sizeof(host_ip);
  ARG_GETHOST("non.existent.host", ARM_SOCKET_AF_INET, host_ip, &ip_len);
  TH_EXECUTE (F_GETHOSTBYNAME, WIFI_SOCKET_TIMEOUT_LONG);
  /* Should return error (host not found) */
  /* Strict: EHOSTNOTFOUND, valid non-strict: ERROR */
  TH_ASSERT2 ((io.rc == ARM_SOCKET_EHOSTNOTFOUND), (io.rc == ARM_SOCKET_ERROR), "gethostbyname for non-existing host", io.rc, ARM_SOCKET_EHOSTNOTFOUND);

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* Ping IO parameters */
typedef struct {
  const uint8_t *ip;
  uint32_t       ip_len;
  int32_t        rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
} IO_PING;

/* Assign arguments */
#define ARG_PING(_ip,_ip_len) do {                   \
                                io.ip     = _ip;     \
                                io.ip_len = _ip_len; \
                              } while (0)

/* Ping worker thread */
__NO_RETURN static void Th_Ping (IO_PING *io) {
  uint32_t flags,xid;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_PING, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_PING:
        /* Ping host */
        io->rc = drv->Ping (io->ip, io->ip_len);
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_Ping
\ingroup wifi_sock_api
\details
The test case \b WIFI_Ping verifies the WiFi Driver \b Ping function:
\code
int32_t (*Ping) (const uint8_t *ip, uint32_t ip_len);
\endcode

Function test:
 - Check function parameters
 - Ping host
*/
void WIFI_Ping (void) {
  osThreadId_t worker;
  int32_t      rval;
  IO_PING      io;

  if (drv->Ping == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Ping function not available");
    return;
  }

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Ping, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Check parameter (ip = NULL) */
  ARG_PING   (NULL, 4);
  TH_EXECUTE (F_PING, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_DRIVER_ERROR_PARAMETER);

  /* Check parameter (ip_len = 0) */
  ARG_PING   (ip_socket_server, 0);
  TH_EXECUTE (F_PING, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_DRIVER_ERROR_PARAMETER);

  /* Check parameter (ip_len = 5) */
  ARG_PING   (ip_socket_server, 5);
  TH_EXECUTE (F_PING, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_DRIVER_ERROR_PARAMETER);

  /* Ping server */
  ARG_PING   (ip_socket_server, 4);
  TH_EXECUTE (F_PING, WIFI_SOCKET_TIMEOUT);
  TH_ASSERT  (io.rc == ARM_DRIVER_OK);

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/**
\defgroup wifi_sock_op WiFi Socket Operation
\ingroup wifi_funcs
\details 
These tests verify operation of the WiFi socket functions.
*/

/* Worker thread is used for the following tests:
   - WIFI_Transfer_Fixed
   - WIFI_Transfer_Incremental
   - WIFI_Send_Fragmented
   - WIFI_Recv_Fragmented
   - WIFI_Test_Speed
   - WIFI_Concurrent_Socket
*/

/* Transfer IO parameters */
typedef struct {
  int32_t      sock;
  uint32_t     len;
  uint32_t     size;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
  int32_t      tcp;
} IO_TRANSFER;

/* Assign arguments */
#define ARG_TRANSFER(_sock,_len,_size) do {               \
                                         io.sock = _sock; \
                                         io.len  = _len;  \
                                         io.size = _size; \
                                       } while (0)

/* Transfer worker thread */
__NO_RETURN static void Th_Transfer (IO_TRANSFER *io) {
  uint32_t flags,xid,i,j;
  int32_t  rc = 0;

  for (;;) {
    /* Wait for the signal to select and execute the function */
    flags = osThreadFlagsWait (F_CREATE_TCP | F_CREATE_UDP |
                               F_CONNECT    | F_CLOSE      |
                               F_XFER_FIXED | F_XFER_INCR  |
                               F_SEND_FRAG  | F_RECV_FRAG, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_CREATE_UDP:
        /* Create datagram socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_DGRAM, ARM_SOCKET_IPPROTO_UDP);
        break;

      case F_CONNECT:
        /* Connect on socket */
        io->rc = drv->SocketConnect (io->sock, ip_socket_server, 4, ECHO_PORT);
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;

      case F_XFER_FIXED:
        /* Transfer Fixed size blocks */
        memset ((void *)buffer, 0xCC, io->len);
        /* Send and receive in small blocks */
        for (i = 0; i < io->len; i += io->size) {
          rc = drv->SocketSend (io->sock, &test_buf[i], io->size);
          if (rc <= 0) break;
          for (j = 0; j < io->size; j += (uint32_t)rc) {
            /* Returns any data available, up to requested amount */
            rc = drv->SocketRecv (io->sock, &buffer[i+j], io->size-j);
            if ((rc <= 0) || !io->tcp) break;
          }
          if (rc <= 0) break;
        }
        if (memcmp ((const void *)buffer, (const void *)test_buf, io->len) == 0) {
          rc = (int32_t)i;
        }
        io->rc = rc;
        break;

      case F_XFER_INCR:
        /* Transfer Increased size blocks */
        memset ((void *)buffer, 0xCC, io->len);
        /* Send and receive in enlarged block sizes */
        for (i = 0; i < io->len; i += io->size++) {
          rc = drv->SocketSend (io->sock, &test_buf[i], io->size);
          if (rc <= 0) break;
          rc = drv->SocketRecv (io->sock, &buffer[i], io->size);
          if (rc <= 0) break;
        }
        if (memcmp ((const void *)buffer, (const void *)test_buf, io->len) == 0) {
          rc = (int32_t)i;
        }
        io->rc = rc;
        break;

      case F_SEND_FRAG:
        /* Send Fragmented blocks */
        memset ((void *)buffer, 0xCC, io->len);
        /* Send in small blocks */
        for (i = 0; i < io->len; i += io->size) {
          rc = drv->SocketSend (io->sock, &test_buf[i], io->size);
          if (rc <= 0) break;
        }
        /* Receive in single block */
        if (rc > 0) {
          /* Small delay that blocks are received */
          osDelay (100);
          for (i = 0; i < io->len; i += (uint32_t)rc) {
            /* Returns any data available, up to requested amount */
            rc = drv->SocketRecv (io->sock, &buffer[i], io->len-i);
            if (rc <= 0) break;
          }
          if (memcmp ((const void *)buffer, (const void *)test_buf, io->len) == 0) {
            rc = (int32_t)i;
          }
        }
        io->rc = rc;
        break;

      case F_RECV_FRAG:
        /* Receive Fragmented blocks */
        memset ((void *)buffer, 0xCC, io->len);
        /* Send single block */
        rc = drv->SocketSend (io->sock, test_buf, io->len);
        if (rc > 0) {
          osDelay (100);
          /* Receive in small blocks */
          for (i = 0; i < io->len; i += io->size) {
            for (j = 0; j < io->size; j += (uint32_t)rc) {
              /* Returns any data available, up to requested amount */
              rc = drv->SocketRecv (io->sock, &buffer[i+j], io->size-j);
              if (rc <= 0) break;
            }
            if (rc <= 0) break;
          }
          if (memcmp ((const void *)buffer, (const void *)test_buf, io->len) == 0) {
            rc = (int32_t)i;
          }
        }
        io->rc = rc;
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_Transfer_Fixed
\ingroup wifi_sock_op
\details
The test case \b WIFI_Transfer_Fixed verifies data transfer in fixed size blocks.
 
Stream socket test: 
 - Create stream socket
 - Transfer 128 blocks of   16 bytes
 - Transfer  32 blocks of   64 bytes
 - Transfer   8 blocks of  256 bytes
 - Transfer   2 blocks of 1024 bytes
 - Transfer   1 block  of 2048 bytes
 - Close socket

Datagram socket test:
 - Create datagram socket
 - Transfer 128 blocks of   16 bytes
 - Transfer  32 blocks of   64 bytes
 - Transfer   8 blocks of  256 bytes
 - Transfer   2 blocks of 1024 bytes
 - Transfer   1 block  of 1460 bytes
 - Close socket
*/
void WIFI_Transfer_Fixed (void) {
  osThreadId_t worker;
  int32_t      rval;
  IO_TRANSFER  io;
  int32_t      sock;

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Transfer, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.tcp  = 1;
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Transfer 16-byte block(s) */
    ARG_TRANSFER (sock, 2048, 16);
    TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Transfer 64-byte block(s) */
    ARG_TRANSFER (sock, 2048, 64);
    TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Transfer 256-byte block(s) */
    ARG_TRANSFER (sock, 2048, 256);
    TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Transfer 1024-byte block(s) */
    ARG_TRANSFER (sock, 2048, 1024);
    TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Transfer 2048-byte block */
    ARG_TRANSFER (sock, 2048, 2048);
    TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Close stream socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Connect to datagram server */
    io.tcp  = 0;
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Transfer 16-byte block(s) */
    ARG_TRANSFER (sock, 2048, 16);
    TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Transfer 64-byte block(s) */
    ARG_TRANSFER (sock, 2048, 64);
    TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Transfer 256-byte block(s) */
    ARG_TRANSFER (sock, 2048, 256);
    TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Transfer 1024-byte block(s) */
    ARG_TRANSFER (sock, 2048, 1024);
    TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Transfer 1460-byte block */
    ARG_TRANSFER (sock, 1460, 1460);
    TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1460);

    /* Close datagram socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/**
\brief  Test case: WIFI_Transfer_Incremental
\ingroup wifi_sock_op
\details
The test case \b WIFI_Transfer_Incremental verifies data transfer in ascending size blocks.
Each subsequent block that the socket sends is one byte larger than the previous block.

Stream socket test:
 - Create stream socket
 - Transfer 51 blocks of   1 -  50 bytes
 - Transfer 30 blocks of  51 -  80 bytes
 - Transfer 20 blocks of  81 - 100 bytes
 - Transfer 13 blocks of 120 - 132 bytes
 - Transfer  8 blocks of 252 - 259 bytes
 - Transfer  4 blocks of 510 - 513 bytes
 - Close socket

Datagram socket test:
 - Create datagram socket
 - Transfer 51 blocks of   1 -  50 bytes
 - Transfer 30 blocks of  51 -  80 bytes
 - Transfer 20 blocks of  81 - 100 bytes
 - Transfer 13 blocks of 120 - 132 bytes
 - Transfer  8 blocks of 252 - 259 bytes
 - Transfer  4 blocks of 510 - 513 bytes
 - Close socket
*/
void WIFI_Transfer_Incremental (void) {
  osThreadId_t worker;
  int32_t      rval;
  IO_TRANSFER  io;
  int32_t      sock;

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Transfer, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Transfer 1 byte - 50 byte blocks */
    ARG_TRANSFER (sock, 1275, 1);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1275);

    /* Transfer 51 byte - 80-byte blocks */
    ARG_TRANSFER (sock, 1965, 51);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1965);

    /* Transfer 81 byte - 100 byte blocks */
    ARG_TRANSFER (sock, 1810, 81);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1810);

    /* Transfer 120 byte - 132 byte blocks */
    ARG_TRANSFER (sock, 1905, 120);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1905);

    /* Transfer 252 byte - 259 byte blocks */
    ARG_TRANSFER (sock, 2044, 252);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2044);

    /* Transfer 510 byte - 513 byte blocks */
    ARG_TRANSFER (sock, 2046, 510);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2046);

    /* Close stream socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Connect to datagram server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Transfer 1 byte - 50 byte blocks */
    ARG_TRANSFER (sock, 1275, 1);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1275);

    /* Transfer 51 byte - 80-byte blocks */
    ARG_TRANSFER (sock, 1965, 51);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1965);

    /* Transfer 81 byte - 100 byte blocks */
    ARG_TRANSFER (sock, 1810, 81);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1810);

    /* Transfer 120 byte - 132 byte blocks */
    ARG_TRANSFER (sock, 1905, 120);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1905);

    /* Transfer 252 byte - 259 byte blocks */
    ARG_TRANSFER (sock, 2044, 252);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2044);

    /* Transfer 510 byte - 513 byte blocks */
    ARG_TRANSFER (sock, 2046, 510);
    TH_EXECUTE (F_XFER_INCR, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2046);

    /* Close datagram socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/**
\brief  Test case: WIFI_Send_Fragmented
\ingroup wifi_sock_op
\details
The test case \b WIFI_Send_Fragmented verifies data transfer in chunks.

Stream socket test:
 - Create stream socket
 - Send 16 blocks of   16 bytes, receive 1 block of  256 bytes
 - Send 16 blocks of   64 bytes, receive 1 block of 1024 bytes
 - Send  5 blocks of  256 bytes, receive 1 block of 1280 bytes
 - Send  2 blocks of 1024 bytes, receive 1 block of 2048 bytes
 - Close socket
*/
void WIFI_Send_Fragmented (void) {
  osThreadId_t worker;
  int32_t      rval;
  IO_TRANSFER  io;
  int32_t      sock;

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Transfer, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Transfer 16-byte block(s) */
    ARG_TRANSFER (sock, 256, 16);
    TH_EXECUTE (F_SEND_FRAG, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 256);

    /* Transfer 64-byte block(s) */
    ARG_TRANSFER (sock, 1024, 64);
    TH_EXECUTE (F_SEND_FRAG, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1024);

    /* Transfer 256-byte block(s) */
    ARG_TRANSFER (sock, 1280, 256);
    TH_EXECUTE (F_SEND_FRAG, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1280);

    /* Transfer 1024-byte block(s) */
    ARG_TRANSFER (sock, 2048, 1024);
    TH_EXECUTE (F_SEND_FRAG, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Close stream socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/**
\brief  Test case: WIFI_Recv_Fragmented
\ingroup wifi_sock_op
\details
The test case \b WIFI_Recv_Fragmented verifies data transfer in chunks.

Stream socket test:
 - Create stream socket
 - Send block of  256 bytes, receive 16 blocks of   16 bytes
 - Send block of 1024 bytes, receive 16 blocks of   64 bytes
 - Send block of 1280 bytes, receive  5 blocks of  256 bytes
 - Send block of 2048 bytes, receive  2 blocks of 1024 bytes
 - Close socket
*/
void WIFI_Recv_Fragmented (void) {
  osThreadId_t worker;
  int32_t      rval;
  IO_TRANSFER  io;
  int32_t      sock;

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Transfer, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Transfer 16-byte block(s) */
    ARG_TRANSFER (sock, 256, 16);
    TH_EXECUTE (F_RECV_FRAG, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 256);

    /* Transfer 64-byte block(s) */
    ARG_TRANSFER (sock, 1024, 64);
    TH_EXECUTE (F_RECV_FRAG, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1024);

    /* Transfer 256-byte block(s) */
    ARG_TRANSFER (sock, 1280, 256);
    TH_EXECUTE (F_RECV_FRAG, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 1280);

    /* Transfer 1024-byte block(s) */
    ARG_TRANSFER (sock, 2048, 1024);
    TH_EXECUTE (F_RECV_FRAG, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 2048);

    /* Close stream socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/**
\brief  Test case: WIFI_Test_Speed
\ingroup wifi_sock_op
\details
The test case \b WIFI_Test_Speed tests data transfer speed.

Stream socket test:
 - Create stream socket
 - Transfer for 4 seconds, send and receive
 - Calculate transfer rate
 - Close socket

Datagram socket test:
 - Create datagram socket
 - Transfer for 4 seconds, send and receive
 - Calculate transfer rate
 - Close socket
*/
void WIFI_Test_Speed (void) {
  uint32_t     ticks,tout;
  osThreadId_t worker;
  int32_t      rval,n_bytes;
  IO_TRANSFER  io;
  int32_t      sock;

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Transfer, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Connect to stream server */
    io.tcp  = 1;
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Transfer for 4 seconds */
    tout  = SYSTICK_MICROSEC(4000000);
    ticks = GET_SYSTICK();
    n_bytes = 0;
    do {
      ARG_TRANSFER (sock, 1420, 1420);
      TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
      if (io.rc > 0) n_bytes += io.rc;
      else           break;
    } while (GET_SYSTICK() - ticks < tout);
    /* Check transfer rate */
    if (n_bytes < 10000) {
      snprintf(msg_buf, sizeof(msg_buf), "[WARNING] Slow Transfer rate (%d KB/s)", n_bytes / 2048);
      TEST_MESSAGE(msg_buf);
    }

    /* Close stream socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Connect to datagram server */
    io.tcp  = 0;
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Transfer for 4 seconds */
    tout  = SYSTICK_MICROSEC(4000000);
    ticks = GET_SYSTICK();
    n_bytes = 0;
    do {
      ARG_TRANSFER (sock, 1460, 1460);
      TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
      if (io.rc > 0) n_bytes += io.rc;
      else           break;
    } while (GET_SYSTICK() - ticks < tout);
    /* Check transfer rate */
    if (n_bytes < 10000) {
      snprintf(msg_buf, sizeof(msg_buf), "[WARNING] Slow Transfer rate (%d KB/s)", n_bytes / 2048);
      TEST_MESSAGE(msg_buf);
    }

    /* Close datagram socket */
    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/* Sidekick IO parameters */
typedef struct {
  int32_t  sock;
  uint32_t count;
} IO_SIDEKICK;

/* Concurrent coworker thread */
__NO_RETURN static void Th_Sidekick (IO_SIDEKICK *io2) {
  uint8_t buff[48];
  int32_t rc;

  for (;;) {
    if (osThreadFlagsWait (SK_TERMINATE, osFlagsWaitAny, 200) == SK_TERMINATE) {
      break;
    }
    memset ((void *)buff, 0xCC, sizeof(buff));
    rc = drv->SocketSend (io2->sock, test_msg, sizeof(test_msg));
    if (rc <= 0) break;
    rc = drv->SocketRecv (io2->sock, buff, sizeof(test_msg));
    if (rc <= 0) break;
    if (memcmp ((const void *)buff, (const void *)test_msg, sizeof(test_msg)) == 0) {
      io2->count += sizeof(test_msg);
    }
  }
  /* Owner deletes this thread */
  while (1) osDelay (osWaitForever);
}

/**
\brief  Test case: WIFI_Concurrent_Socket
\ingroup wifi_sock_op
\details
The test case \b WIFI_Concurrent_Socket verifies transfer of two concurrent sockets.

Stream socket test:
 - Create two stream sockets
 - Start transfer on 2nd socket with 200ms intervals
 - Transfer on main socket, full speed
 - Calculate transfer rate
 - Close sockets

Datagram socket test:
 - Create datagram and stream sockets
 - Start transfer on stream socket with 200ms intervals
 - Transfer on main socket, full speed
 - Calculate transfer rate
 - Close sockets
\note
The test runs with a coherent thread, that performs an additional stream socket io.
*/
void WIFI_Concurrent_Socket (void) {
  uint32_t     ticks,tout;
  osThreadId_t worker,spawn;
  int32_t      rval,n_bytes;
  IO_TRANSFER  io;
  IO_SIDEKICK  io2;
  int32_t      sock;

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_Transfer, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* The test connects two stream sockets to the ECHO server and then    */
  /* performs simultaneous data transfer. The main socket transmits at   */
  /* full speed, and the other socket sends messages at 200ms intervals. */
  /* Both sockets record the number of bytes of data transferred, and    */
  /* the transfer rate is calculated.                                    */

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    sock = io.rc;

    /* Create 2nd stream socket */
    TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
    if (io.rc < 0) {
      TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
    }
    io2.sock  = io.rc;
    io2.count = 0;

    /* Connect sockets */
    io.tcp  = 1;
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    io.sock = io2.sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Create spawned thread */
    spawn = osThreadNew ((osThreadFunc_t)Th_Sidekick, &io2, NULL);
    TEST_ASSERT(spawn != NULL);

    /* Transfer for 4 seconds */
    tout  = SYSTICK_MICROSEC(4000000);
    ticks = GET_SYSTICK();
    n_bytes = 0;
    do {
      ARG_TRANSFER (sock, 1420, 1420);
      TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
      if (io.rc > 0) n_bytes += io.rc;
      else           break;
    } while (GET_SYSTICK() - ticks < tout);
    /* Check main transfer rate */
    if (n_bytes < 10000) {
      snprintf(msg_buf, sizeof(msg_buf), "[WARNING] Slow Transfer rate (%d KB/s)", n_bytes / 2048);
      TEST_MESSAGE(msg_buf);
    }
    /* Check auxiliary transfer rate */
    if (io2.count == 0) {
      TEST_ASSERT_MESSAGE(0,"[FAILED] Auxiliary transfer failed");
    }
    else if (io2.count < 440) {
      TEST_MESSAGE("[WARNING] Auxiliary Transfer rate low");
    }

    /* Terminate spawned thread */
    osThreadFlagsSet (spawn, SK_TERMINATE);
    osDelay(100);
    osThreadTerminate (spawn);

    /* Close stream sockets */
    io.sock = io2.sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  /* The test connects datagram and stream sockets to the ECHO server     */
  /* and then performs simultaneous data transfer. The datagram socket    */
  /* transmits at full speed, and the stream socket sends messages at     */
  /* 200ms intervals. The number of bytes of transferred data is recorded */
  /* and the rate of transmission is calculated.                          */

  /* Create datagram socket */
  TH_EXECUTE (F_CREATE_UDP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Datagram Socket not created");
  } else {
    sock = io.rc;

    /* Connect datagram socket */
    io.tcp  = 0;
    io.sock = sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    /* Create stream socket */
    TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
    if (io.rc < 0) {
      TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
    }
    io2.sock  = io.rc;
    io2.count = 0;

    /* Connect stream socket */
    io.sock = io2.sock;
    TH_EXECUTE (F_CONNECT, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc == 0);

    /* Create spawned thread */
    spawn = osThreadNew ((osThreadFunc_t)Th_Sidekick, &io2, NULL);
    TEST_ASSERT(spawn != NULL);

    /* Transfer for 4 seconds */
    tout  = SYSTICK_MICROSEC(4000000);
    ticks = GET_SYSTICK();
    n_bytes = 0;
    do {
      ARG_TRANSFER (sock, 1460, 1460);
      TH_EXECUTE (F_XFER_FIXED, WIFI_SOCKET_TIMEOUT_LONG);
      if (io.rc > 0) n_bytes += io.rc;
      else           break;
    } while (GET_SYSTICK() - ticks < tout);
    /* Check main transfer rate */
    if (n_bytes < 10000) {
      snprintf(msg_buf, sizeof(msg_buf), "[WARNING] Slow Transfer rate (%d KB/s)", n_bytes / 2048);
      TEST_MESSAGE(msg_buf);
    }
    /* Check auxiliary transfer rate */
    if (io2.count == 0) {
      TEST_ASSERT_MESSAGE(0,"[FAILED] Auxiliary transfer failed");
    }
    else if (io2.count < 440) {
      TEST_MESSAGE("[WARNING] Auxiliary Transfer rate low");
    }

    /* Terminate spawned thread */
    osThreadFlagsSet (spawn, SK_TERMINATE);
    osDelay(100);
    osThreadTerminate (spawn);

    /* Close sockets */
    io.sock = io2.sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    io.sock = sock;
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/

/* TestAssistant commands */
#define CMD_SEND_TCP        "SEND TCP,1420,4000"
#define CMD_RECV_TCP        "RECV TCP,1420"
#define TEST_BSIZE          1420

/* StreamRate IO parameters */
typedef struct {
  int32_t      sock;
  int32_t      rc;
  /* Control */
  osThreadId_t owner;
  uint32_t     xid;
  int32_t      loss;
  const char  *cmd;
} IO_STREAMRATE;

/* StreamRate coworker thread */
__NO_RETURN static void Th_StreamRate (IO_STREAMRATE *io) {
  uint32_t flags,xid,ticks,tout;
  int32_t  n,rc,i,val;

  for (;;) {
    flags = osThreadFlagsWait (F_CREATE_TCP | F_DOWNLOAD | F_UPLOAD |
                               F_SEND_CTRL  | F_CLOSE, osFlagsWaitAny, osWaitForever);
    xid   = io->xid;
    switch (flags) {
      case F_CREATE_TCP:
        /* Create stream socket */
        io->rc = drv->SocketCreate (ARM_SOCKET_AF_INET, ARM_SOCKET_SOCK_STREAM, ARM_SOCKET_IPPROTO_TCP);
        break;

      case F_DOWNLOAD:
        /* Downstream test, server is sender */
        for (n = 0; ; n += rc) {
          rc = drv->SocketRecv (io->sock, buffer, TEST_BSIZE);
          if (strncmp ((char *)buffer, "STAT", 4) == 0) {
            /* Server completed the test */
            sscanf ((char *)buffer+4, "%d", &val);
            if (val > n) io->loss = val - n;
            break;
          }
          if (rc <= 0) break;
        }
        io->rc = n;
        break;

      case F_UPLOAD:
        /* Upstream test, server is receiver */
        memset ((void *)buffer, 'a', TEST_BSIZE);
        tout  = SYSTICK_MICROSEC(4000000);
        ticks = GET_SYSTICK();
        i = n = 0;
        do {
          snprintf ((char *)buffer, sizeof(buffer), "Block[%d]", ++i);
          rc = drv->SocketSend (io->sock, buffer, TEST_BSIZE);
          if (rc > 0) n += rc;
        } while (GET_SYSTICK() - ticks < tout);
        rc = snprintf ((char *)buffer, sizeof(buffer), "STOP %d bytes.", n);
        drv->SocketSend (io->sock, buffer, (uint32_t)rc);
        /* Receive report from server */
        drv->SocketRecv (io->sock, buffer, TEST_BSIZE);
        if (strncmp ((char *)buffer, "STAT", 4) == 0) {
          sscanf ((char *)buffer+4, "%d", &val);
          if (n > val) io->loss = n - val;
        }
        io->rc = n;
        break;

      case F_CLOSE:
        /* Close socket */
        io->rc = drv->SocketClose (io->sock);
        break;

      case F_SEND_CTRL:
        /* Send control command to TestAssistant */
        drv->SocketConnect (io->sock, ip_socket_server, 4, ASSISTANT_PORT);
        io->rc = drv->SocketSend (io->sock, io->cmd, strlen(io->cmd));
        break;
    }
    /* Done, send signal to owner thread */
    flags = (xid == io->xid) ? TH_OK : TH_TOUT;
    osDelay(1);
    osThreadFlagsSet (io->owner, flags);
    osThreadFlagsClear (F_ALL);
  }
}

/**
\brief  Test case: WIFI_Downstream_Rate
\ingroup wifi_sock_op
\details
The test case \b WIFI_Downstream_Rate tests the maximum rate at which the data
can be received.
*/
void WIFI_Downstream_Rate (void) {
  osThreadId_t  worker;
  int32_t       rval;
  IO_STREAMRATE io;

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_StreamRate, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    io.sock = io.rc;

    /* Send command to start the download */
    io.cmd = CMD_SEND_TCP;
    TH_EXECUTE (F_SEND_CTRL, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc > 0);

    /* Wait for transfer to complete */
    io.loss = 0;
    TH_EXECUTE (F_DOWNLOAD, 5000 + WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc > 0);

    /* Check data loss */
    if (io.loss) {
      snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Data loss %d byte(s)", io.loss);
      TEST_ASSERT_MESSAGE(0,msg_buf);
    }
    else if (rval != 0) {
      snprintf(msg_buf, sizeof(msg_buf), "[INFO] Speed %d KB/s", io.rc/4000);
      TEST_MESSAGE(msg_buf);
    }

    /* Close stream socket */
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}

/**
\brief  Test case: WIFI_Upstream_Rate
\ingroup wifi_sock_op
\details
The test case \b WIFI_Upstream_Rate tests the maximum rate at which the data
can be sent.
*/
void WIFI_Upstream_Rate (void) {
  osThreadId_t  worker;
  int32_t       rval;
  IO_STREAMRATE io;

  if (station_init (1) == 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Station initialization and connect failed");
    return;
  }

  /* Create worker thread */
  worker = osThreadNew ((osThreadFunc_t)Th_StreamRate, &io, NULL);
  if (worker == NULL) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Worker Thread not created");
    return;
  }

  ARG_INIT();

  /* Create stream socket */
  TH_EXECUTE (F_CREATE_TCP, WIFI_SOCKET_TIMEOUT);
  if (io.rc < 0) {
    TEST_ASSERT_MESSAGE(0,"[FAILED] Stream Socket not created");
  } else {
    io.sock = io.rc;

    /* Send command to start the upload */
    io.cmd = CMD_RECV_TCP;
    TH_EXECUTE (F_SEND_CTRL, WIFI_SOCKET_TIMEOUT_LONG);
    TH_ASSERT  (io.rc > 0);

    /* Wait for transfer to complete */
    io.loss = 0;
    TH_EXECUTE (F_UPLOAD, 5000 + WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc > 0);

    /* Check data loss */
    if (io.loss) {
      snprintf(msg_buf, sizeof(msg_buf), "[FAILED] Data loss %d byte(s)", io.loss);
      TEST_ASSERT_MESSAGE(0,msg_buf);
    }
    else if (rval != 0) {
      snprintf(msg_buf, sizeof(msg_buf), "[INFO] Speed %d KB/s", io.rc/4000);
      TEST_MESSAGE(msg_buf);
    }

    /* Close stream socket */
    TH_EXECUTE (F_CLOSE, WIFI_SOCKET_TIMEOUT);
    TH_ASSERT  (io.rc == 0);

    osDelay (10);
  }

  if (rval == 0) {
    station_uninit ();
  }

  /* Terminate worker thread */
  osThreadTerminate (worker);
}
