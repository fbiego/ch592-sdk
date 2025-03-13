/* ********************************* (C) COPYRIGHT ***************************
* File Name: app_tmos.C
* Author: WCH
* Version: V1.0
* Date: 2023/8/5
* Description: Touch key routine
********************************************************************************************* */

/*********************************************************************
 * INCLUDES
 */
#include "Touch.h"
#include "app.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      VARIABLES
 **********************/
UINT8V timerFlag = 0;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void TKY_PeripheralInit(void);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/* ***************************************************************************
* @fn touch_dataProcess
*
* @brief Touch the data processing function (naked running), print the retrieved key triggering situation
*
* @return none */
void touch_dataProcess(void)
{
    uint8_t key_val = 0;
    static uint16_t print_time = 0;

    if(timerFlag)
    {
        timerFlag = 0;
        touch_KeyScan();
#if PRINT_EN
        print_time++;
        if(print_time == 500)
        {
            print_time = 0;
            touch_InfoDebug();
        }
#endif
    }
    key_val = touch_GetKey();
    switch(key_val)
    {
       case KEY_NONE   :   break;
       case KEY_0_DOWN :   PRINT("KEY_1_DOWN !\n");break;
       case KEY_0_UP   :   PRINT("KEY_1_UP   !\n");break;
       case KEY_0_LONG :   PRINT("KEY_1_LONG !\n");break;
       case KEY_1_DOWN :   PRINT("KEY_2_DOWN !\n");break;
       case KEY_1_UP   :   PRINT("KEY_2_UP   !\n");break;
       case KEY_1_LONG :   PRINT("KEY_2_LONG !\n");break;
       case KEY_2_DOWN :   PRINT("KEY_3_DOWN !\n");break;
       case KEY_2_UP   :   PRINT("KEY_3_UP   !\n");break;
       case KEY_2_LONG :   PRINT("KEY_3_LONG !\n");break;
       case KEY_3_DOWN :   PRINT("KEY_4_DOWN !\n");break;
       case KEY_3_UP   :   PRINT("KEY_4_UP   !\n");break;
       case KEY_3_LONG :   PRINT("KEY_4_LONG !\n");break;
       case KEY_4_DOWN :   PRINT("KEY_5_DOWN !\n");break;
       case KEY_4_UP   :   PRINT("KEY_5_UP   !\n");break;
       case KEY_4_LONG :   PRINT("KEY_5_LONG !\n");break;
       case KEY_5_DOWN :   PRINT("KEY_6_DOWN !\n");break;
       case KEY_5_UP   :   PRINT("KEY_6_UP   !\n");break;
       case KEY_5_LONG :   PRINT("KEY_6_LONG !\n");break;
       case KEY_6_DOWN :   PRINT("KEY_7_DOWN !\n");break;
       case KEY_6_UP   :   PRINT("KEY_7_UP   !\n");break;
       case KEY_6_LONG :   PRINT("KEY_7_LONG !\n");break;
       case KEY_7_DOWN :   PRINT("KEY_8_DOWN !\n");break;
       case KEY_7_UP   :   PRINT("KEY_8_UP   !\n");break;
       case KEY_7_LONG :   PRINT("KEY_8_LONG !\n");break;
       case KEY_8_DOWN :   PRINT("KEY_9_DOWN !\n");break;
       case KEY_8_UP   :   PRINT("KEY_9_UP   !\n");break;
       case KEY_8_LONG :   PRINT("KEY_9_LONG !\n");break;
       case KEY_9_DOWN :   PRINT("KEY_*_DOWN !\n");break;
       case KEY_9_UP   :   PRINT("KEY_*_UP   !\n");break;
       case KEY_9_LONG :   PRINT("KEY_*_LONG !\n");break;
       case KEY_10_DOWN :  PRINT("KEY_0_DOWN!\n");break;
       case KEY_10_UP  :   PRINT("KEY_0_UP  !\n");break;
       case KEY_10_LONG :  PRINT("KEY_0_LONG!\n");break;
       case KEY_11_DOWN :  PRINT("KEY_#_DOWN!\n");break;
       case KEY_11_UP  :   PRINT("KEY_#_UP  !\n");break;
       case KEY_11_LONG :  PRINT("KEY_#_LONG!\n");break;
       default : break;
    }
}


/*********************************************************************
 * @fn      touch_init
 *
 * @brief   tmos
 *
 * @return  none
 */
void touch_init(void)
{
	TKY_PeripheralInit();       /* Initialize peripherals such as backlights and buzzers */

	touch_InitKey();				/* Initialize the touch library */

    TKY_SetSleepStatusValue( ~tkyPinAll.tkyQueueAll );

    TMR0_TimerInit(FREQ_SYS/1000);               // The timing period is 1ms
    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END);
    PFIC_EnableIRQ( TMR0_IRQn );

    dg_log("Touch Key init Finish!\n");
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/*********************************************************************
 * @fn      TKY_PeripheralInit
 *
 * @brief   
 *
 * @return  none
 */
static void TKY_PeripheralInit(void)
{
    /*You code here*/
}

/* ***************************************************************************
* @fn TMR0_IRQHandler
*
* @brief timer 0 interrupt service function
*
* @return none */
__INTERRUPT
__HIGH_CODE
void TMR0_IRQHandler( void )
{
    if( TMR0_GetITFlag( TMR0_3_IT_CYC_END ) )
    {
        TMR0_ClearITFlag( TMR0_3_IT_CYC_END );
        timerFlag=1;
    }
}
