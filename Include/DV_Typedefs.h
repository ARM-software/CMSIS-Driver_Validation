/*-----------------------------------------------------------------------------
 *      Name:         DV_Typedefs.h 
 *      Purpose:      Test framework filetypes and structures header
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/

#ifndef __CMSIS_DV_TYPEDEFS_H__
#define __CMSIS_DV_TYPEDEFS_H__

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

typedef unsigned int    BOOL;

#ifndef __TRUE
 #define __TRUE         1
#endif
#ifndef __FALSE
 #define __FALSE        0
#endif

#ifndef ENABLED
 #define ENABLED        1
#endif
#ifndef DISABLED
 #define DISABLED       0
#endif

#ifndef NULL
 #ifdef __cplusplus              // EC++
  #define NULL          0
 #else
  #define NULL          ((void *) 0)
 #endif
#endif

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

/* Test macros */
#define TEST_FAIL()                             TEST_FAIL_MESSAGE("[FAILED]")
#define TEST_FAIL_MESSAGE(message)              __set_result (__FILE__, __LINE__, message, FAILED)
#define TEST_PASS()                             TEST_PASS_MESSAGE(NULL)
#define TEST_PASS_MESSAGE(message)              __set_result (__FILE__, __LINE__, message, PASSED)

#define TEST_ASSERT(condition)                  TEST_ASSERT_MESSAGE(condition,"[FAILED]")
#define TEST_ASSERT_MESSAGE(condition,message)  if (condition) { __set_result (__FILE__, __LINE__, NULL, PASSED); } else __set_result (__FILE__, __LINE__, message, FAILED)

#define TEST_MESSAGE(message)                   __set_message(__FILE__, __LINE__, message)

#endif /* __CMSIS_DV_TYPEDEFS_H__ */
