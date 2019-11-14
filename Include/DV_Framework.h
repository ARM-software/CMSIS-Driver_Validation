/*-----------------------------------------------------------------------------
 *      Name:         DV_Framework.h 
 *      Purpose:      Framework header
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/

#ifndef __CMSIS_DV_FRAMEWORK_H__
#define __CMSIS_DV_FRAMEWORK_H__

#include "DV_Typedefs.h"
#include "DV_Report.h"

/*-----------------------------------------------------------------------------
 * Test framework global definitions
 *----------------------------------------------------------------------------*/

/* Test case definition macro                                                 */
#define TCD(x, y) {x, #x, y}

/* Test case description structure                                            */
typedef struct {
  void (*TestFunc)(void);             /* Test function                        */
  const char *TFName;                 /* Test function name string            */
  BOOL en;                            /* Test function enabled                */
} const TEST_CASE;

/* Test group description structure                                           */
typedef struct {
  const char *FileName;               /* Test module file name                */
  const char *Date;                   /* Compilation date                     */
  const char *Time;                   /* Compilation time                     */
  const char *ReportTitle;            /* Title or name of module under test   */
  void (*Init)(void);                 /* Init function callback               */
  void (*Uninit)(void);               /* Uninit function callback             */
  
  uint32_t TCBaseNum;                 /* Base number for test case numbering  */
  TEST_CASE *TC;                      /* Array of test cases                  */
  uint32_t NumOfTC;                   /* Number of test cases (sz of TC array)*/

} const TEST_GROUP;

/* Defined in user test module                                                */
extern TEST_GROUP ts[];
extern uint32_t   tg_cnt;

#endif /* __CMSIS_DV_FRAMEWORK_H__ */
