/*
 * Copyright (c) 2015-2021 Arm Limited. All rights reserved.
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
 * Project:     CMSIS-Driver Validation
 * Title:       Report statistics and layout implementation
 *
 * -----------------------------------------------------------------------------
 */


#include "DV_Report.h"


/* Local macros */
#define PRINT(x) MsgPrint x
#define FLUSH()  MsgFlush()

/* Local functions */
static void tr_Init    (void);
static void tr_Uninit  (void);
static void tg_Init    (const char *title, const char *date, const char *time, const char *fn);
static void tg_Info    (const char *info);
static void tg_InfoDone(void);
static void tg_Uninit  (void);
static void tc_Init    (uint32_t num, const char *fn);
static void tc_Detail  (const char *module, uint32_t line, const char *message);
static void tc_Uninit  (void);
static void as_Result  (TC_RES res);

static void MsgPrint (const char *msg, ...);
static void MsgFlush (void);

/* Global variables */
REPORT_ITF ritf = {                     /* Structure for report interface     */
  tr_Init,
  tr_Uninit,
  tg_Init,
  tg_Info,
  tg_InfoDone,
  tg_Uninit,
  tc_Init,
  tc_Detail,
  tc_Uninit,
  as_Result,
};

/* Local variables */
static TEST_GROUP_RESULTS test_group_result;    /* Test group results         */

static uint32_t   as_passed = 0U;       /* Assertions passed                  */
static uint32_t   as_failed = 0U;       /* Assertions failed                  */
static uint32_t   as_detail = 0U;       /* Assertions details available       */
static const char Passed[] = "PASSED";
static const char Failed[] = "FAILED";
static const char NotExe[] = "NOT EXECUTED";


/*-----------------------------------------------------------------------------
 * No path - helper function
 *----------------------------------------------------------------------------*/
static const char *no_path (const char *fn) {
  const char *cp;
#if defined(__CC_ARM)
  cp = strrchr(fn, (int32_t)'\\');
#else
  cp = strrchr(fn, (int32_t)'/');
#endif
  if (cp != NULL) {
    cp = &cp[1];
  } else {
    cp = fn;
  }
  return (cp);
}

/*-----------------------------------------------------------------------------
 * Init test report
 *----------------------------------------------------------------------------*/
static void tr_Init (void) {

  test_group_result.idx = 0;

#if (PRINT_XML_REPORT==1)
  PRINT(("<?xml version=\"1.0\"?>\n"));
  PRINT(("<?xml-stylesheet href=\"TR_Style.xsl\" type=\"text/xsl\" ?>\n"));
  PRINT(("<report>\n"));
#else
  PRINT(("                                \n\n"));
#endif  
}

/*-----------------------------------------------------------------------------
 * Uninit test report
 *----------------------------------------------------------------------------*/
static void tr_Uninit (void) {
#if (PRINT_XML_REPORT==1)
  PRINT(("</report>\n"));
#endif
}

/*-----------------------------------------------------------------------------
 * Init test group
 *----------------------------------------------------------------------------*/
static void tg_Init (const char *title, const char *date, const char *time, const char *fn) {

  test_group_result.idx++;
  test_group_result.tests  = 0U;
  test_group_result.passed = 0U;
  test_group_result.failed = 0U;

#if (PRINT_XML_REPORT==1)
  PRINT(("<test>\n"));
  PRINT(("<title>%s</title>\n", title));
  PRINT(("<date>%s</date>\n",   date));
  PRINT(("<time>%s</time>\n",   time));
  PRINT(("<file>%s</file>\n",   fn));
  PRINT(("<group>%i</group>\n", test_group_result.idx));
  PRINT(("<info>\n"));
#else
  (void) fn;
  PRINT(("%s   %s   %s \n\n", title, date, time));
#endif  
}

/*-----------------------------------------------------------------------------
 * Write test group info
 *----------------------------------------------------------------------------*/
static void tg_Info (const char *info) {

  PRINT(("%s", info));
  PRINT(("\n"));
#if (PRINT_XML_REPORT==0)
  PRINT(("\n"));
#endif
}

/*-----------------------------------------------------------------------------
 * Test group info done
 *----------------------------------------------------------------------------*/
static void tg_InfoDone (void) {

#if (PRINT_XML_REPORT==1)
  PRINT(("</info>\n"));
  PRINT(("<test_cases>\n"));
#endif  
}

/*-----------------------------------------------------------------------------
 * Uninit test group
 *----------------------------------------------------------------------------*/
static void tg_Uninit (void) {
  const char *tres;

  if (test_group_result.failed > 0U) {  /* If any test failed => Failed       */
    tres = Failed;
  } else if (test_group_result.passed > 0U) {   /* If 1 passed => Passed      */
    tres = Passed;
  } else {                              /* If no tests exec => Not-executed   */
    tres = NotExe;
  }

#if (PRINT_XML_REPORT==1) 
  PRINT(("</test_cases>\n"));
  PRINT(("<summary>\n"));
  PRINT(("<tcnt>%d</tcnt>\n", test_group_result.tests));
  PRINT(("<pass>%d</pass>\n", test_group_result.passed));
  PRINT(("<fail>%d</fail>\n", test_group_result.failed));
  PRINT(("<tres>%s</tres>\n", tres));
  PRINT(("</summary>\n"));
  PRINT(("</test>\n"));
#else
  PRINT(("\nTest Summary: %d Tests, %d Passed, %d Failed.\n", 
         test_group_result.tests, 
         test_group_result.passed, 
         test_group_result.failed));
  PRINT(("Test Result: %s\n\n\n", tres));
#endif 
  FLUSH();
}

/*-----------------------------------------------------------------------------
 * Init test
 *----------------------------------------------------------------------------*/
static void tc_Init (uint32_t num, const char *fn) {

  as_passed = 0U;
  as_failed = 0U;
  as_detail = 0U;

#if (PRINT_XML_REPORT==1)   
  PRINT(("<tc>\n"));
  PRINT(("<no>%d</no>\n",     num));
  PRINT(("<func>%s</func>\n", fn));
  PRINT(("<dbgi>\n"));
#else
  PRINT(("TEST %02d: %-32s ", num, fn));
#endif 
}

/*-----------------------------------------------------------------------------
 * Write test detail
 *----------------------------------------------------------------------------*/
static void tc_Detail (const char *module, uint32_t line, const char *message) {
  const char *module_no_path;

  module_no_path = no_path (module);

  as_detail = 1U;

#if (PRINT_XML_REPORT==1) 
  PRINT(("<detail>\n"));
  PRINT(("<module>%s</module>\n", module_no_path));
  PRINT(("<line>%d</line>\n",     line));
  if (message != NULL) {
    PRINT(("<message>%s</message>\n", message));
  }
  PRINT(("</detail>\n"));
#else
  PRINT(("\n  %s (%d)", module_no_path, line));
  if (message != NULL) {
    PRINT((": %s", message));
  }
#endif
}

/*-----------------------------------------------------------------------------
 * Uninit test
 *----------------------------------------------------------------------------*/
static void tc_Uninit (void) {
  const char *res;

  test_group_result.tests++;

  if (as_failed > 0U) {                 /* If any assertion failed => Failed  */
    test_group_result.failed++;
    res = Failed;
  } else if (as_passed > 0U) {          /* If 1 assertion passed => Passed    */
    test_group_result.passed++;
    res = Passed;
  } else {                              /* If no assertions => Not-executed   */
    res = NotExe;
  }

#if (PRINT_XML_REPORT==1) 
  PRINT(("</dbgi>\n"));
  PRINT(("<res>%s</res>\n", res));
  PRINT(("</tc>\n"));
  (void)as_detail;
#else
  if (as_detail != 0U) {
    PRINT(("\n                                          "));
  }
  PRINT(("%s\n", res));
#endif 
}

/*-----------------------------------------------------------------------------
 * Assertion result registering
 *----------------------------------------------------------------------------*/
static void as_Result (TC_RES res) {

  if (res == PASSED) {
    as_passed++;
  } else if (res == FAILED) {
    as_failed++;
  } else {
    // Do nothing
  }
}

/*-----------------------------------------------------------------------------
 * Add info line to group info
 *----------------------------------------------------------------------------*/
void __tg_info (const char *info) {

  if (info != NULL) {
    tg_Info(info);
  }
}

/*-----------------------------------------------------------------------------
 * Set result
 *----------------------------------------------------------------------------*/
void __set_result (const char *module, uint32_t line, const char *message, TC_RES res) {

  // Set debug info
  if (message != NULL) {
    tc_Detail(module, line, message);
  }

  // Set result
  as_Result(res);
}

/*-----------------------------------------------------------------------------
 * Set message
 *----------------------------------------------------------------------------*/
void __set_message (const char *module, uint32_t line, const char *message) {
  if (message != NULL) {
    tc_Detail(module, line, message);
  }
}


#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
/*-----------------------------------------------------------------------------
 *       MsgPrint:  Print a message to the standard output
 *----------------------------------------------------------------------------*/
static void MsgPrint (const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  (void)vprintf(msg, args);
  va_end(args);
}
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic pop
#endif

/*-----------------------------------------------------------------------------
 *       SER_MsgFlush:  Flush the standard output
 *----------------------------------------------------------------------------*/
static void MsgFlush(void) {
  (void)fflush(stdout);
}

/*-----------------------------------------------------------------------------
 * End of file
 *----------------------------------------------------------------------------*/
