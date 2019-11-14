/*-----------------------------------------------------------------------------
 *      Name:         DV_Framework.c
 *      Purpose:      Test framework
 *----------------------------------------------------------------------------
 *      Copyright(c) KEIL - An ARM Company
 *----------------------------------------------------------------------------*/

#include "cmsis_dv.h" 
#include "DV_Framework.h"

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\defgroup framework_funcs Framework Functions
\brief Functions in the Framework software component
\details

@{
*/

/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief Close the debug session.
\details
Debug session dead end - debug script should close session here. This function is called by \ref cmsis_dv.
*/
void closeDebug(void);
void __attribute__((noinline)) closeDebug(void){
  __NOP();
  // Test completed
}


/*=======0=========1=========2=========3=========4=========5=========6=========7=========8=========9=========0=========1====*/
/**
\brief This is the entry point of the test framework.
\details
Program flow:
  -# Test report is initialized
  -# For each test group following steps are executed:
    -# Test group initialization is called (custom test group initialization)
    -# Test group header is written to standard output 
    -# All tests in a group are executed as follows:
        - Test case statistics are initialized
        - Test case report header is written to the standard output
        - Test case is executed
        - Test case results are written to the standard output
        - Test case report footer is written to the standard output
    -# Test group footer is written to standard output 
    -# Test group uninitialization is called (custom test group uninitialization)
  -# Debug session ends when closeDebug function is reached
*/
void cmsis_dv (void __attribute__((unused)) *argument) {
  const char *fn;
  uint32_t    i, tc, no;

  if (tg_cnt != 0) {                    /* If at least 1 test is enabled      */

    ritf.tr_Init ();                    /* Init test report                   */

    for (i = 0U; i < tg_cnt; i++) {

      if (ts[i].Init) {
        ts[i].Init();                   /* Init test group (group setup)      */
      }
                                        /* Init test group report             */
      ritf.tg_Init(ts[i].ReportTitle,   /* Write test group title             */
                   ts[i].Date,          /* Write test group compilation date  */
                   ts[i].Time,          /* Write test group compilation time  */
                   ts[i].FileName);     /* Write test group module file name  */

      /* Execute all test cases in a group */
      for (tc = 0; tc < ts[i].NumOfTC; tc++) {
        no = ts[i].TCBaseNum+tc;        /* Test case number                   */
        fn = ts[i].TC[tc].TFName;       /* Test function name string          */
        ritf.tc_Init (no, fn);          /* Init test case report #(Base + TC) */
        if (ts[i].TC[tc].en) {
          ts[i].TC[tc].TestFunc();      /* Execute test case if enabled       */
        }
        ritf.tc_Uninit ();              /* Uninit test case report            */
      }

      ritf.tg_Uninit ();                /* Uninit test group report           */

      if (ts[i].Uninit) {
        ts[i].Uninit();                 /* Uninit test group (group teardown) */
      }
    }

    ritf.tr_Uninit();                   /* Uninit test report                 */
  }

  closeDebug();                         /* Close debug session                */
}

/**
@}
*/ 
// end of group framework_funcs
