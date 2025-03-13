/* ********************************* (C) COPYRIGHT ***************************
* File Name: Touch.C
* Author: WCH
* Version: V1.6
* Date: 2021/12/1
* Description: Touch key routine
********************************************************************************************* */

/*********************************************************************
 * INCLUDES
 */
#include "Touch.h"

/*********************
 *      DEFINES
 *********************/
#define WAKEUPTIME  50     //Sleep Time = 250 * SLEEP_TRIGGER_TIME(100ms) = 25s

/**********************
 *      VARIABLES
 **********************/
uint8_t TKY_MEMBUF[ TKY_MEMHEAP_SIZE ];
uint8_t wakeUpCount = 0, wakeupflag = 0;
uint16_t keyData = 0, scanData = 0;
volatile TOUCH_S tkyPinAll = {0};

static const TKY_ChannelInitTypeDef my_tky_ch_init[TKY_QUEUE_END] = {TKY_CHS_INIT};

static const uint32_t TKY_Pin[14][2] = {
  {0x00, 0x00000010},//PA4
  {0x00, 0x00000020},//PA5
  {0x00, 0x00001000},//PA12
  {0x00, 0x00002000},//PA13
  {0x00, 0x00004000},//PA14
  {0x00, 0x00008000},//PA15
  {0x00, 0x00000000},// AIN6 does not exist, it occupies a place here
  {0x00, 0x00000000},// AIN7 does not exist, it occupies a place here

  {0x20, 0x00000001},// PB0, AIN8 only has 592X models
  {0x20, 0x00000040},// PB6, AIN9 only has 592X models

  {0x00, 0x00000040},// PA6, AIN10 only has 592X models
  {0x00, 0x00000080},// PA7, AIN11 only has 592X models
  {0x00, 0x00000100},//PA8,AIN12
  {0x00, 0x00000200} //PA9,AIN13
};
/**********************
 *  STATIC PROTOTYPES
 **********************/
static KEY_T s_tBtn[KEY_COUNT];
static KEY_FIFO_T s_tKey;       /* Key FIFO variable, structure */
static void touch_InitKeyHard(void);
static void touch_InitKeyVar(void);
static void touch_DetectKey(uint8_t i);
static void touch_Baseinit(void);
static void touch_Channelinit(void);
static uint8_t IsKeyDown1(void);
static uint8_t IsKeyDown2(void);
static uint8_t IsKeyDown3(void);
static uint8_t IsKeyDown4(void);
static uint8_t IsKeyDown5(void);
static uint8_t IsKeyDown6(void);
static uint8_t IsKeyDown7(void);
static uint8_t IsKeyDown8(void);
static uint8_t IsKeyDown9(void);
static uint8_t IsKeyDown10(void);
static uint8_t IsKeyDown11(void);
static uint8_t IsKeyDown12(void);

pIsKeyDownFunc KeyDownFunc[14] =
{
        IsKeyDown1,
        IsKeyDown2,
        IsKeyDown3,
        IsKeyDown4,
        IsKeyDown5,
        IsKeyDown6,
        IsKeyDown7,
        IsKeyDown8,
        IsKeyDown9,
        IsKeyDown10,
        IsKeyDown11,
        IsKeyDown12
};
/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/* *******************************************************************************************************
* @fn touch_InitKey
*
* @brief Initialize the key. This function is called by tky_Init().
*
* @return none */
void touch_InitKey(void)
{
    touch_InitKeyHard();          /* Initialize key hardware */
    touch_InitKeyVar();           /* Initialize key variables */
}

/* *******************************************************************************************************
* @fn touch_PutKey
* @brief Press 1 key value into the key FIFO buffer.Can be used to simulate a key.
* @param _KeyCode - key code
* @return none */
void touch_PutKey(uint8_t _KeyCode)
{
    s_tKey.Buf[s_tKey.Write] = _KeyCode;

    if (++s_tKey.Write  >= KEY_FIFO_SIZE)
    {
        s_tKey.Write = 0;
    }
}

/* *******************************************************************************************************
* @fn touch_GetKey
* @brief Read a key value from the key FIFO buffer.
* @param None
* @return key code */
uint8_t touch_GetKey(void)
{
    uint8_t ret;

    if (s_tKey.Read == s_tKey.Write)
    {
        return KEY_NONE;
    }
    else
    {
        ret = s_tKey.Buf[s_tKey.Read];

        if (++s_tKey.Read >= KEY_FIFO_SIZE)
        {
            s_tKey.Read = 0;
        }
        return ret;
    }
}

/* *******************************************************************************************************
* @fn touch_GetKeyState
* @brief The state of the key is read
* @param _ucKeyID - key ID, starting from 0
* @return 1 - Press
* 0 - Not pressed
********************************************************************************************************* */
uint8_t touch_GetKeyState(KEY_ID_E _ucKeyID)
{
    return s_tBtn[_ucKeyID].State;
}

/* *******************************************************************************************************
* @fn touch_SetKeyParam
* @brief Set key parameters
* @param _ucKeyID - key ID, starting from 0
* _LongTime - Long press event time
* _RepeatSpeed ​​- continuous sending speed
* @return none */
void touch_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t  _RepeatSpeed)
{
    s_tBtn[_ucKeyID].LongTime = _LongTime;          /* Long press time 0 means that long press event is not detected */
    s_tBtn[_ucKeyID].RepeatSpeed = _RepeatSpeed;            /* The speed of continuous sending by pressing the button, 0 means that continuous sending is not supported. */
    s_tBtn[_ucKeyID].RepeatCount = 0;                       /* Continuous send counter */
}


/* *******************************************************************************************************
* @fn touch_ClearKey
* @brief Clear key FIFO buffer
* @param None
* @return key code */
void touch_ClearKey(void)
{
    s_tKey.Read = s_tKey.Write;
}

/* *******************************************************************************************************
* @fn touch_ScanWakeUp
* @brief Touch Scan Wake-up Function
* @param None
* @return None */
void touch_ScanWakeUp(void)
{
    wakeUpCount = WAKEUPTIME; // ---Wake-up time---
    wakeupflag = 1;           // Set to wake up

    TKY_SetSleepStatusValue( ~tkyPinAll.tkyQueueAll ); // ---Set channels 0~11 to non-sleep state to prepare for continuous scanning in the next few seconds---
    dg_log("wake up for a while\n");
    TKY_SaveAndStop();    // ---Save the relevant registers---
    touch_GPIOSleep();
}

/* *******************************************************************************************************
* @fn touch_ScanEnterSleep
* @brief Touch Scan Hibernation Function
* @param None
* @return None */
void touch_ScanEnterSleep(void)
{
    TKY_SaveAndStop();    // ---Save the relevant registers---
    touch_GPIOSleep();
    wakeupflag = 0;       // Set to sleep state: 0, wake up state: 1
    TKY_SetSleepStatusValue( tkyPinAll.tkyQueueAll );
    dg_log("Ready to sleep\n");
}

/* *******************************************************************************************************
* @fn touch_KeyScan
* @brief Scan all keys.Non-blocking, periodic calls are interrupted by systick
* @param None
* @return None */
void touch_KeyScan(void)
{
    uint8_t i;
    TKY_LoadAndRun( );                     // ---Separate settings saved before loading hibernation---

    keyData = TKY_PollForFilter( );

#if TKY_SLEEP_EN
    if (keyData)
    {
        wakeUpCount = WAKEUPTIME; // ---Wake-up time---
    }
#endif

    for (i = 0; i < KEY_COUNT; i++)
    {
        touch_DetectKey(i);
    }
    TKY_SaveAndStop();    // ---Save the relevant registers---
}

/* *******************************************************************************************************
* @fn touch_GPIOModeCfg
* @brief Touch button mode configuration
* @param None
* @return None */
void touch_GPIOModeCfg(GPIOModeTypeDef mode)
{
    uint32_t pina = tkyPinAll.PaBit;
    uint32_t pinb = tkyPinAll.PbBit;
    switch(mode)
    {
        case GPIO_ModeIN_Floating:
        	R32_PA_PD_DRV &= ~pina;
        	R32_PA_PU &= ~pina;
        	R32_PA_DIR &= ~pina;
        	R32_PB_PD_DRV &= ~pinb;
        	R32_PB_PU &= ~pinb;
        	R32_PB_DIR &= ~pinb;
            break;

        case GPIO_ModeOut_PP_5mA:
            R32_PA_PU &= ~pina;
            R32_PA_PD_DRV &= ~pina;
            R32_PA_DIR |= pina;
            R32_PB_PU &= ~pinb;
            R32_PB_PD_DRV &= ~pinb;
            R32_PB_DIR |= pinb;
            break;
        default:
            break;
    }
}

void touch_IOSetAdcState(uint8_t ch)
{
	(*((PUINT32V)0x400010B4+TKY_Pin[ch][0])) &= ~TKY_Pin[ch][1];
	(*((PUINT32V)0x400010B0+TKY_Pin[ch][0])) &= ~TKY_Pin[ch][1];
	(*((PUINT32V)0x400010A0+TKY_Pin[ch][0])) &= ~TKY_Pin[ch][1];
}

void touch_SingleChDischarge(uint8_t ch){
	(*((PUINT32V)(0x400010A0+TKY_Pin[ch][0]))) |= TKY_Pin[ch][1];
	(*((PUINT32V)(0x400010AC+TKY_Pin[ch][0]))) = TKY_Pin[ch][1];
}

/* *******************************************************************************************************
* @fn touch_GPIOSleep
* @brief Configure the touch button to sleep
* @param None
* @return None */
void touch_GPIOSleep(void)
{
    uint32_t pina = tkyPinAll.PaBit;
    uint32_t pinb = tkyPinAll.PbBit;
    R32_PA_PU &= ~pina;
    R32_PA_PD_DRV &= ~pina;
    R32_PA_DIR |= pina;
    R32_PA_CLR |= pina;

    R32_PB_PU &= ~pinb;
    R32_PB_PD_DRV &= ~pinb;
    R32_PB_DIR |= pinb;
    R32_PB_CLR |= pinb;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/* *******************************************************************************************************
* @fn touch_InitKeyHard
* @brief Initialize the touch button
* @param None
* @return None */
static void touch_InitKeyHard(void)
{
    touch_Baseinit( );
    touch_Channelinit( );
}


/* *******************************************************************************************************
* @fn touch_InitKeyVar
* @brief Initialize the touch button variable
* @param None
* @return None */
static void touch_InitKeyVar(void)
{
    uint8_t i;

    /* Clear the key FIFO read and write pointer */
    s_tKey.Read = 0;
    s_tKey.Write = 0;

    /* Assign a set of default values ​​to each key structure member variable */
    for (i = 0; i < KEY_COUNT; i++)
    {
        s_tBtn[i].LongTime = KEY_LONG_TIME;             /* Long press time 0 means that long press event is not detected */
        s_tBtn[i].Count = KEY_FILTER_TIME / 2;          /* The counter is set to half of the filtering time */
        s_tBtn[i].State = 0;                            /* The default state of the button is 0. */
        s_tBtn[i].RepeatSpeed = 0;                      /* The speed of continuous sending by pressing the button, 0 means that continuous sending is not supported. */
        s_tBtn[i].RepeatCount = 0;                      /* Continuous send counter */
        s_tBtn[i].IsKeyDownFunc = KeyDownFunc[i];       /* Determine the function of the button press */
    }

    /* If you need to change the parameters of a key separately, you can reassign the value separately here */
    /* For example, we hope that the same key value will be automatically resented after pressing key 1 for more than 1 second. */
//    s_tBtn[KID_K1].LongTime = 100;
// s_tBtn[KID_K1].RepeatSpeed ​​= 5; /* Automatically send key values ​​every 50ms */

}

/* *******************************************************************************************************
* @fn IsKeyDownX
* @brief To determine whether the button is pressed, the user can re-implement the function function by himself
* @param None
* @return 1 - Press
* 0 - Not pressed */
static uint8_t IsKeyDown1(void)
{
    if (keyData & 0x0001)   return 1;
    else                    return 0;
}

static uint8_t IsKeyDown2(void)
{
    if (keyData & 0x0002)   return 1;
    else                    return 0;
}

static uint8_t IsKeyDown3(void)
{
    if (keyData & 0x0004)   return 1;
    else                    return 0;
}

static uint8_t IsKeyDown4(void)
{
    if (keyData & 0x0008)   return 1;
    else                    return 0;
}


static uint8_t IsKeyDown5(void)
{
    if (keyData & 0x0010)   return 1;
    else                    return 0;
}

static uint8_t IsKeyDown6(void)
{
    if (keyData & 0x0020)   return 1;
    else                    return 0;
}

static uint8_t IsKeyDown7(void)
{
    if (keyData & 0x0040)   return 1;
    else                    return 0;
}

static uint8_t IsKeyDown8(void)
{
    if (keyData & 0x0080)   return 1;
    else                    return 0;
}

static uint8_t IsKeyDown9(void)
{
    if (keyData & 0x0100)   return 1;
    else                    return 0;
}

static uint8_t IsKeyDown10(void)
{
    if (keyData & 0x0200)   return 1;
    else                    return 0;
}

static uint8_t IsKeyDown11(void)
{
    if (keyData & 0x0400)   return 1;
    else                    return 0;
}

static uint8_t IsKeyDown12(void)
{
    if (keyData & 0x0800)   return 1;
    else                    return 0;
}


/* *******************************************************************************************************
* @fn touch_InfoDebug
* @brief Touch data printing function
* @param None
* @return None */
 
void touch_InfoDebug(void)
{
    uint8_t i;
    int16_t data_dispNum[ TKY_MAX_QUEUE_NUM ]={0};
	int16_t bl,vl;

    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
#if TKY_FILTER_MODE == FILTER_MODE_1
        bl = TKY_GetCurQueueBaseLine( i );
        vl = TKY_GetCurQueueValue( i );
        if(bl>vl)   data_dispNum[ i ] =  bl-vl ;
        else        data_dispNum[ i ] =  vl-bl ;
#else
        data_dispNum[ i ] = TKY_GetCurQueueValue( i );
#endif
    }

    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
        dg_log("%04d,", data_dispNum[i]);
    } dg_log("\n");

    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
        data_dispNum[ i ] = TKY_GetCurQueueBaseLine( i );
    }

    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
        dg_log("%04d,", data_dispNum[i]);
    } dg_log("\n");
#if TKY_FILTER_MODE == FILTER_MODE_1
    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
        dg_log("%04d,", TKY_GetCurQueueValue( i ));
    }dg_log("\n");
#endif
    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
        dg_log("%04d,", TKY_GetCurQueueRealVal( i ));
    }dg_log("\r\n");
#if TKY_FILTER_MODE == FILTER_MODE_7
    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
    	dg_log("%04d,", TKY_GetCurQueueValue2( i ));
    }dg_log("\r\n");
    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
    	dg_log("%04d,", TKY_GetCurQueueBaseLine2( i ));
    }dg_log("\r\n");
    for (i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
    	dg_log("%04d,", TKY_GetCurQueueRealVal2( i ));
    }dg_log("\r\n");
#endif
    dg_log("\r\n");

}

/* *******************************************************************************************************
* @fn touch_DetectKey
* @brief Detect a key.The non-blocking state must be called periodically.
* @param i - key structure variable pointer
* @return None */
static void touch_DetectKey(uint8_t i)
{
    KEY_T *pBtn;

/* Press the button */
    pBtn = &s_tBtn[i];
    if (pBtn->IsKeyDownFunc()==1)
    {
            if (pBtn->State == 0)
            {
                pBtn->State = 1;
#if !KEY_MODE
                /* Send a message pressed by a button */
                touch_PutKey((uint8_t)(3 * i + 1));
#endif
            }

            /* Process long press */
            if (pBtn->LongTime > 0)
            {
                if (pBtn->LongCount < pBtn->LongTime)
                {
                    /* Send a message with long pressing of buttons */
                    if (++pBtn->LongCount == pBtn->LongTime)
                    {
#if !KEY_MODE
                        pBtn->State = 2;

                        /* Put the key value into the FIFO key */
                        touch_PutKey((uint8_t)(3 * i + 3));
#endif
                    }
                }
                else
                {
                    if (pBtn->RepeatSpeed > 0)
                    {
                        if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
                        {
                            pBtn->RepeatCount = 0;
#if !KEY_MODE
                            /* After long pressing the key, send 1 key every pBtn->RepeatSpeed*10ms */
                            touch_PutKey((uint8_t)(3 * i + 1));
#endif
                        }
                    }
                }
            }
    }
    else
    {
            if (pBtn->State)
            {
#if KEY_MODE
                if(pBtn->State == 1)
                /* Send a message pressed by a button */
                touch_PutKey((uint8_t)(3 * i + 1));
#endif
                pBtn->State = 0;

#if !KEY_MODE
                /* After releasing the key KEY_FILTER_TIME, send the message that the button pops up */
                touch_PutKey((uint8_t)(3 * i + 2));
#endif
            }

        pBtn->LongCount = 0;
        pBtn->RepeatCount = 0;
    }
}

/* *******************************************************************************************************
* @fn touch_Baseinit
* @brief Touch basic library initialization
* @param None
* @return None */
static void touch_Baseinit(void)
{
    TKY_BaseInitTypeDef TKY_BaseInitStructure = {0};
    for(uint8_t i = 0; i < TKY_MAX_QUEUE_NUM; i++)  // Initialize tkyPinAll and tkyQueueAll variables
    {
    	if(TKY_Pin[my_tky_ch_init[i].channelNum][0] == 0x00)
    	{
    		tkyPinAll.PaBit |= TKY_Pin[my_tky_ch_init[i].channelNum][1];
    	}
    	else if(TKY_Pin[my_tky_ch_init[i].channelNum][0] == 0x20)
      {
    		tkyPinAll.PbBit |= TKY_Pin[my_tky_ch_init[i].channelNum][1];
    	}
    	tkyPinAll.tkyQueueAll |= 1<<i;
    }
    dg_log("tP : %08x,%08x; tQ : %04x\n",tkyPinAll.PaBit,tkyPinAll.PbBit,tkyPinAll.tkyQueueAll);
    R8_TKEY_CFG|=RB_TKEY_CURRENT;
#if (TKY_SHIELD_EN)&&((TKY_FILTER_MODE != FILTER_MODE_9))
    tkyPinAll.PaBit |= TKY_SHIELD_PIN;
#endif

    touch_GPIOSleep();  // Pull down all touch pin feet

#if (TKY_SHIELD_EN)&&((TKY_FILTER_MODE != FILTER_MODE_9))
    tkyPinAll.PaBit &= ~TKY_SHIELD_PIN;
    GPIOA_ModeCfg(TKY_SHIELD_PIN, GPIO_ModeIN_Floating);//Shield Pin， only for CH58x series
#endif
    // ------------------------------------
    TKY_BaseInitStructure.filterMode = TKY_FILTER_MODE;
#if (TKY_FILTER_MODE != FILTER_MODE_9)
    TKY_BaseInitStructure.shieldEn = TKY_SHIELD_EN;
#else
    TKY_BaseInitStructure.shieldEn = 0;
#endif
    TKY_BaseInitStructure.singlePressMod = TKY_SINGLE_PRESS_MODE;
    TKY_BaseInitStructure.filterGrade = TKY_FILTER_GRADE;
    TKY_BaseInitStructure.maxQueueNum = TKY_MAX_QUEUE_NUM;
    TKY_BaseInitStructure.baseRefreshOnPress = TKY_BASE_REFRESH_ON_PRESS;
    // ---Baseline update speed, baseRefreshSampleNum and filterGrade, are inversely proportional to the baseline update speed. The baseline update speed is also related to the code structure. You can observe it through the function GetCurQueueBaseLine--
    TKY_BaseInitStructure.baseRefreshSampleNum = TKY_BASE_REFRESH_SAMPLE_NUM;
    TKY_BaseInitStructure.baseUpRefreshDouble = TKY_BASE_UP_REFRESH_DOUBLE;
    TKY_BaseInitStructure.baseDownRefreshSlow = TKY_BASE_DOWN_REFRESH_SLOW;
    TKY_BaseInitStructure.tkyBufP = TKY_MEMBUF;
    TKY_BaseInit( TKY_BaseInitStructure );
}

/* *******************************************************************************************************
* @fn touch_Channelinit
* @brief Touch channel initialization
* @param None
* @return None */
static void touch_Channelinit(void)
{

    uint8_t error_flag = 0;
    uint16_t chx_mean = 0;

    for(uint8_t i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
    	TKY_CHInit(my_tky_ch_init[i]);
    }

#if (TKY_FILTER_MODE != FILTER_MODE_9)
    for(uint8_t i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {

    	chx_mean = TKY_GetCurChannelMean(my_tky_ch_init[i].channelNum, my_tky_ch_init[i].chargeTime,
										 my_tky_ch_init[i].disChargeTime, 1000);

    	if(chx_mean < 3400 || chx_mean > 3800)
    	{
    		error_flag = 1;
    	}
    	else
    	{
    		TKY_SetCurQueueBaseLine(i, chx_mean);
    	}
    	dg_log("queue : %d ch : %d , mean : %d\n",i,my_tky_ch_init[i].channelNum,chx_mean);

    }
    // The charge and discharge baseline value is abnormal, recalibrate the baseline value
    if(error_flag != 0)
    {
    	touch_GPIOSleep();  // Pull down all touch pin feet
        dg_log("\n\nCharging parameters error, preparing for recalibration ...\n\n");
        uint8_t charge_time;
        for (uint8_t i = 0; i < TKY_MAX_QUEUE_NUM; i++) {       // ADC channel conversion by maximum number of sequences
          charge_time = 0,chx_mean = 0;
          touch_IOSetAdcState(my_tky_ch_init[i].channelNum);
          while (1)
          {
              chx_mean = TKY_GetCurChannelMean(my_tky_ch_init[i].channelNum, charge_time,3, 1000);

// dg_log("testing... chg : %d, baseline : %d\n",charge_time,chx_mean);//Print baseline value

              if ((charge_time == 0) && ((chx_mean > 3800))) {// Below the minimum charging parameter
                  dg_log("Error, %u KEY%u Too small Cap,Please check the hardware !\r\n",chx_mean,i);
                  break;
              }
              else {
                  if ((chx_mean > 3200) &&(chx_mean < 3800)) {// Charging parameters are normal
                      TKY_SetCurQueueBaseLine(i, chx_mean);
                      TKY_SetCurQueueChargeTime(i,charge_time,3);
                      dg_log("channel:%u, chargetime:%u,BaseLine:%u\r\n",
                            i, charge_time, chx_mean);
                      break;
                  }else if(chx_mean >= 3800)
                  {
                	  TKY_SetCurQueueBaseLine(i, TKY_GetCurChannelMean(my_tky_ch_init[i].channelNum, charge_time-1,3, 20));
                	  TKY_SetCurQueueChargeTime(i,charge_time-1,3);
                	  dg_log("Warning,channel:%u Too large Current, chargetime:%u,BaseLine:%u\r\n",
                	                              i, charge_time, chx_mean);
                	  break;
                  }
                  charge_time++;
                  if (charge_time > 0x1f) {    // Maximum charging parameters exceeded
                      dg_log("Error, Chargetime Max,KEY%u Too large Cap,Please check the hardware !\r\n",i);
                      break;
                  }
              }
          }
          touch_SingleChDischarge(my_tky_ch_init[i].channelNum);
//          GPIOA_ModeCfg(TKY_Pin[my_tky_ch_init[i].channelNum],GPIO_ModeIN_Floating);
        }
    }
#endif
#if (TKY_FILTER_MODE == FILTER_MODE_9) ||(TKY_FILTER_MODE == FILTER_MODE_7)
#if TKY_SHIELD_EN
    TKY_ChannelInitTypeDef TKY_ChannelInitStructure = {0};
	// --------Initialize touch channel 0 and is ranked as the 13th position in the detection queue-----------
	TKY_ChannelInitStructure.queueNum = 12;
	TKY_ChannelInitStructure.channelNum = 0;
	TKY_ChannelInitStructure.threshold = 40; // ---The threshold threshold is related to PCB board, please adjust according to actual situation---
	TKY_ChannelInitStructure.threshold2 = 30;
	TKY_ChannelInitStructure.sleepStatus = 1;
	TKY_ChannelInitStructure.baseLine = 600;
	TKY_CHInit( TKY_ChannelInitStructure );
#endif
    // In Filter 9 mode, you need to use a separate function to calibrate the baseline value
    TKY_CaliCrowedModBaseLine(0, 1000);
    for (uint8_t i = 0; i < TKY_MAX_QUEUE_NUM; i++)
    {
#if(TKY_FILTER_MODE == FILTER_MODE_7)
    	TKY_SetCurQueueThreshold2(i, my_tky_ch_init[i].threshold, my_tky_ch_init[i].threshold2);
#elif(TKY_FILTER_MODE == FILTER_MODE_9)
    	TKY_SetCurQueueThreshold(i, my_tky_ch_init[i].threshold, my_tky_ch_init[i].threshold2);
#endif
		dg_log("%u  key:%u -> thresholdUp:%u;  thresholdDown:%u;\r\n",TKY_GetCurQueueBaseLine(i),i,
			   my_tky_ch_init[i].threshold, my_tky_ch_init[i].threshold2);
    }
#endif
    TKY_SaveAndStop();
}

/* *******************************************************************************************************
* @fn touch_DetectWheelSlider
* @brief Touch pulley data processing
* @param None
* @return None */
uint16_t touch_DetectWheelSlider(void)
{
	uint8_t  loop;
	uint8_t  max_data_num;
	uint16_t d1;
	uint16_t d2;
	uint16_t d3;
	uint16_t wheel_rpos;
	uint16_t dsum;
	int16_t dval;
	uint16_t unit;
	uint16_t wheel_data[TOUCH_WHEEL_ELEMENTS] = {0};
	uint8_t num_elements=TOUCH_WHEEL_ELEMENTS;
	uint16_t p_threshold = 60;

	if (num_elements < 3)
	{
		return 0;
	}

	for (loop = 0; loop < num_elements; loop++)
	{
		dval = TKY_GetCurQueueValue( loop );
		if(dval>0)
		{
			wheel_data[ loop ] = (uint16_t)dval;
		}
		else {
			wheel_data[ loop ] = 0;
		}
	}
	/* Search max data in slider */
	max_data_num = 0;
	for (loop = 0; loop < (num_elements - 1); loop++)
	{
		if (wheel_data[max_data_num] < wheel_data[loop + 1])
		{
			max_data_num = (uint8_t) (loop + 1);
		}
	}
	/* Array making for wheel operation          */
	/*    Maximum change CH_No -----> Array"0"    */
	/*    Maximum change CH_No + 1 -> Array"2"    */
	/*    Maximum change CH_No - 1 -> Array"1"    */
	if (0 == max_data_num)
	{
		d1 = (uint16_t) (wheel_data[0] - wheel_data[num_elements - 1]);
		d2 = (uint16_t) (wheel_data[0] - wheel_data[1]);
		dsum = (uint16_t) (wheel_data[0] + wheel_data[1] + wheel_data[num_elements - 1]);
	}
	else if ((num_elements - 1) == max_data_num)
	{
		d1 = (uint16_t) (wheel_data[num_elements - 1] - wheel_data[num_elements - 2]);
		d2 = (uint16_t) (wheel_data[num_elements - 1] - wheel_data[0]);
		dsum = (uint16_t) (wheel_data[0] + wheel_data[num_elements - 2] + wheel_data[num_elements - 1]);
	}
	else
	{
		d1 = (uint16_t) (wheel_data[max_data_num] - wheel_data[max_data_num - 1]);
		d2 = (uint16_t) (wheel_data[max_data_num] - wheel_data[max_data_num + 1]);
		dsum = (uint16_t) (wheel_data[max_data_num + 1] + wheel_data[max_data_num] + wheel_data[max_data_num - 1]);
	}

	if (0 == d1)
	    {
	        d1 = 1;
	    }
	    /* Constant decision for operation of angle of wheel    */
	    if (dsum > p_threshold)
	    {
	        d3 = (uint16_t) (TOUCH_DECIMAL_POINT_PRECISION + ((d2 * TOUCH_DECIMAL_POINT_PRECISION) / d1));

	        unit       = (uint16_t) (TOUCH_WHEEL_RESOLUTION / num_elements);
	        wheel_rpos = (uint16_t) (((unit * TOUCH_DECIMAL_POINT_PRECISION) / d3) + (unit * max_data_num));

	        /* Angle division output */
	        /* diff_angle_ch = 0 -> 359 ------ diff_angle_ch output 1 to 360 */
	        if (0 == wheel_rpos)
	        {
	            wheel_rpos = TOUCH_WHEEL_RESOLUTION;
	        }
	        else if ((TOUCH_WHEEL_RESOLUTION + 1) < wheel_rpos)
	        {
	            wheel_rpos = 1;
	        }
	        else
	        {
	            /* Do Nothing */
	        }
	    }
	    else
	    {
	        wheel_rpos = TOUCH_OFF_VALUE;
	    }

	return wheel_rpos;
}
