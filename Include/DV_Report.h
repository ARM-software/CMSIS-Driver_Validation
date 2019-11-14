/*-----------------------------------------------------------------------------
 *      Name:         DV_Report.h 
 *      Purpose:      Report statistics and layout header
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/

#ifndef __CMSIS_DV_REPORT_H__
#define __CMSIS_DV_REPORT_H__

#include "DV_Config.h"
#include "DV_Typedefs.h"

/*-----------------------------------------------------------------------------
 * Test report global definitions
 *----------------------------------------------------------------------------*/
 
/* Test case result definition */
typedef enum {
  PASSED = 0,
  FAILED,
  NOT_EXECUTED
} TC_RES;

/* Test group statistics */
typedef struct {
  uint32_t idx;                         /* Group index                        */
  uint32_t tests;                       /* Total test cases count             */
  uint32_t passed;                      /* Total test cases passed            */
  uint32_t failed;                      /* Total test cases failed            */
} TEST_GROUP_RESULTS;

/* Test report interface */
typedef struct {
  void (* tr_Init)  (void);
  void (* tr_Uninit)(void);
  void (* tg_Init)  (const char *title, const char *date, const char *time, const char *file);
  void (* tg_Uninit)(void);
  void (* tc_Init)  (uint32_t num, const char *fn);
  void (* tc_Uninit)(void);
  void (* tc_Detail)(const char *module, uint32_t line, const char *message);
  void (* as_Result)(TC_RES res);
} REPORT_ITF;

/* Global structure for interfacing test report */
extern REPORT_ITF ritf;

/* Assertions and test results */
extern void __set_result (const char *module, uint32_t line, const char *message, TC_RES res);
extern void __set_message(const char *module, uint32_t line, const char *message);

#endif /* __CMSIS_DV_REPORT_H__ */
