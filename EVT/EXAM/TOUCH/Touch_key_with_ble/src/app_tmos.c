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
#include "CONFIG.h"
#include "app_tmos.h"
#include "peripheral.h"
/*********************
 *      DEFINES
 *********************/
#define SLEEP_TRIGGER_TIME MS1_TO_SYSTEM_TIME(500) // 500ms
#define TRIGGER_TIME MS1_TO_SYSTEM_TIME(100)       // 100ms
#define WAKEUP_TIME MS1_TO_SYSTEM_TIME(5)          // 50ms

/**********************
 *      VARIABLES
 **********************/
tmosTaskID TouchKey_TaskID = 0x00;
uint16_t triggerTime = SLEEP_TRIGGER_TIME;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void TKY_PeripheralInit(void);
static void peripherals_EnterSleep(void);
static void peripherals_WakeUp(void);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/* ***************************************************************************
* @fn tky_on_TMOS_dataProcess
*
* @brief Touch data processing function (based on TMOS). After the Bluetooth connection is successful, the key value will be obtained and reported to the Bluetooth computer in the form of a notification.
*
* @return none */
void tky_on_TMOS_dataProcess(void)
{
    uint8_t key_val = 0;
    key_val = touch_GetKey();
    if (key_val != 0x00)
    {
        if (bleConnectState )
        {
            peripheralChar2Notify( &key_val, 1 );// Report key value to the upper computer Bluetooth
        }
    }
}


/* ***************************************************************************
* @fn PeriodicDealData
*
* @brief Touch sleep state processing
*
* @return none */
void PeriodicDealData(void)
{
    TKY_LoadAndRun(); // --- Part of the settings saved before loading hibernation---
//    GPIOTK_PinSleep(  );

    // ---Wakeup state, you can switch the display content when you wake up - baseline or measurement value. Each time you touch it, wake up 10 wakeup times. Press this timer to set the time to 5s---
    if (wakeUpCount)
    {
        wakeUpCount--;
//        dg_log("wakeUpCount: :%d\n", wakeUpCount);
        // ---wakeUpCount count is 0, and the wake-up state is about to turn to sleep---
        if (wakeUpCount == 0)
        {
        	touch_ScanEnterSleep();

            tmos_stop_task(TouchKey_TaskID, WAKEUP_DATA_DEAL_EVT);
            triggerTime = SLEEP_TRIGGER_TIME;
            /*-------------------------
             * Call your peripherals sleep function
             * -----------------------*/
            peripherals_EnterSleep();
        }
    }
    else // ---When sleeping, scan the time between wake-up---
    {
        dg_log("wake up...\n");

        scanData = TKY_ScanForWakeUp(tkyPinAll.tkyQueueAll); // ---Scan the selected queue channel---

        if (scanData) // ---If there is an exception in scanning, call the official scan function mode 3~4--
        {
            TKY_SetSleepStatusValue(~scanData); // ---Set the sleep state, set the channel with abnormal state to a non-sleep state---
            for (uint8_t i = 0; i < 40; i++) // ---It is not necessary to scan the code 64 times, and more than 20 times are fine. In the following code, if the scanning button is pressed, the loop will be exited and the wake-up scan will be started---
            {
                keyData = TKY_PollForFilter();
                if (keyData) // ---Once a key press is detected, the loop scanning is exited---
                {
                	touch_ScanWakeUp();

                    triggerTime = TRIGGER_TIME;
                    tky_DealData_start();
                    tmos_start_task(TouchKey_TaskID, WAKEUP_DATA_DEAL_EVT, 0);
                    /*-------------------------
                     * Call your peripherals WakeUp function
                     * -----------------------*/
                    peripherals_WakeUp();
                    break;
                }
            }
            if (keyData == 0)
            {
                TKY_SaveAndStop(); // ---Save the relevant registers---
            }
        }
        else
        {
            TKY_SaveAndStop(); // ---Save the relevant registers---
        }
    }
    TKY_SaveAndStop(); // ---Save the relevant registers---
}


/* ***************************************************************************
* @fn tky_DealData_start
*
* @brief Touch scan to enable function
*
* @return none */
void tky_DealData_start(void)
{
    tmos_set_event(TouchKey_TaskID, DEALDATA_EVT);
}

/* ***************************************************************************
* @fn tky_DealData_stop
*
* @brief Touch Scan Stop Function
*
* @return none */
void tky_DealData_stop(void)
{
    tmos_stop_task(TouchKey_TaskID, DEALDATA_EVT);
}


/* ***************************************************************************
* @fn Touch_Key_ProcessEvent
*
* @brief Touch key processing function
*
* @return none */
tmosEvents Touch_Key_ProcessEvent(tmosTaskID task_id, tmosEvents events)
{
    uint16_t res;

    if (events & WAKEUP_DATA_DEAL_EVT)
    {
    	touch_KeyScan();
        tky_on_TMOS_dataProcess();
#if TKY_SLEEP_EN
        if (wakeupflag)
#endif
            tmos_start_task(TouchKey_TaskID, WAKEUP_DATA_DEAL_EVT, WAKEUP_TIME);
        return (events ^ WAKEUP_DATA_DEAL_EVT);
    }

    if (events & DEALDATA_EVT)
    {
        PeriodicDealData();
#if TKY_SLEEP_EN
        if (!advState || wakeupflag)
#endif
            tmos_start_task(TouchKey_TaskID, DEALDATA_EVT, triggerTime);
        return (events ^ DEALDATA_EVT);
    }

#if PRINT_EN
    if (events & DEBUG_PRINT_EVENT)
    {
        touch_InfoDebug();

        tmos_start_task(TouchKey_TaskID, DEBUG_PRINT_EVENT, SLEEP_TRIGGER_TIME);
        return (events ^ DEBUG_PRINT_EVENT);
    }
#endif

    return 0;
}


/* ***************************************************************************
* @fn touch_on_TMOS_init
*
* @brief Touch initialization function (based on TMOS)
*
* @return none */
void touch_on_TMOS_init(void)
{
    TouchKey_TaskID = TMOS_ProcessEventRegister(Touch_Key_ProcessEvent);
    TKY_PeripheralInit();       /* Initial peripherals such as backlights and buzzers, etc. */
    touch_InitKey();

    wakeUpCount = 50; // ---Wake-up time---
    wakeupflag = 1;   // Set to wake up
    triggerTime = TRIGGER_TIME;
    TKY_SetSleepStatusValue(~tkyPinAll.tkyQueueAll);
#if TKY_SLEEP_EN
    tky_DealData_start();
#else
    tky_DealData_stop();
#endif

#if PRINT_EN
    tmos_set_event(TouchKey_TaskID, DEBUG_PRINT_EVENT);
#endif

    tmos_set_event(TouchKey_TaskID, WAKEUP_DATA_DEAL_EVT);
    dg_log("Touch Key init Finish!\n");
}


/**********************
 *   STATIC FUNCTIONS
 **********************/


/* ***************************************************************************
* @fn TKY_PeripheralInit
*
* @brief Touch peripheral initialization function, used to initialize peripheral functions related to touch functions
*
* @return none */
static void TKY_PeripheralInit(void)
{
    /*You code here*/
}

/* ***************************************************************************
* @fn peripherals_EnterSleep
*
* @brief Peripheral sleep function, called when touching to prepare for sleep
*
* @return none */
static void peripherals_EnterSleep(void)
{
    /*You code here*/
}


/* ***************************************************************************
* @fn peripherals_WakeUp
*
* @brief The peripheral wake-up function, called when the touch is awakened
*
* @return none */
static void peripherals_WakeUp(void)
{
    /*You code here*/
}
