#ifndef __WCHTOUCH_H__
#define __WCHTOUCH_H__

/*********************************************************************
 * GLOBAL MACROS
 */
/*****************filter*****************/
#define FILTER_NONE    		      0       // --No filter-
#define FILTER_MODE_1    	      1       // --Filter mode 1, multiple keys can be output-
#define FILTER_MODE_3    	      3       // --Filter mode 3, multiple keys can be output-
#define FILTER_MODE_5             5       // --Filter mode 5, multiple keys can be output-
#define FILTER_MODE_7             7       // --Filter mode 7, multiple keys can be output-
#define FILTER_MODE_9             9       // --Dense mode dedicated filter-
#define FILTER_MODE_CS10          2       // --CS10V dedicated filter mode-
/**************single key mode***********/
#define TKY_SINGLE_KEY_MULTI      0       // --Multiple key output, that is, keys exceeding the threshold will trigger
#define TKY_SINGLE_KEY_MAX        1       // --The maximum value single key output, that is, only the key with the largest change is reported among the keys that exceed the threshold
#define TKY_SINGLE_KEY_MUTU       2       // --Mutually exclusive single key output, that is, the next key with the largest change will be reported only after the current key is released.
                                          // Otherwise, no matter how much the other buttons change, they will not be reported.
/***************lib param****************/
#define TKY_BUFLEN  			  24
/*----------------------------------------*/
typedef struct
{
	uint8_t maxQueueNum;				    // --Number of test queues-
	uint8_t singlePressMod;                	// ---Single button mode---
	uint8_t shieldEn;                       // ---Blocking enable---
	uint8_t filterMode;				        // --Filter mode --
	uint8_t filterGrade;				    // --Filter Level-
	uint8_t peakQueueNum;                	// ---Maximum offset queue for keys---
	uint8_t peakQueueOffset;             	// ---The offset value of the maximum offset queue of keys---
	uint8_t baseRefreshOnPress;			    // --Whether the baseline is performed when the key is pressed-
	uint8_t baseUpRefreshDouble;        	// ---Baseline refreshes the double speed parameter upward---
	uint8_t baseDownRefreshSlow;       		// ---Baseline updates downward speed down parameters---
	uint32_t baseRefreshSampleNum;     		// --Number of baseline refresh samples-
	uint8_t *tkyBufP;					 	// --Test channel data buffer pointer-
}TKY_BaseInitTypeDef;

typedef struct
{
	uint8_t queueNum;                 		// --The serial number of the channel in the test queue-
	uint8_t channelNum;               		// --The ADC channel label corresponding to this channel-
	uint8_t chargeTime;		            	// --The charging time of this channel-
	uint8_t disChargeTime;            		// --The discharge time of this channel-
	uint8_t sleepStatus;		          	// --Dormant-
	uint16_t baseLine;  	   	        	// --Baseline-
	uint16_t threshold;		            	// --Threshold-
	uint16_t threshold2;              		// --Threshold 2--
}TKY_ChannelInitTypeDef;


/* *********************************************************************************************
* @fn TKY_BaseInit
*
* @brief TouchKey overall parameter initialization
*
* @param TKY_BaseInitStruct initially identified parameters
*
* @return Initialization result - 0x00: Initialization successful
* - 0x01: Filter mode parameter error
* - 0x02: Filter level parameter error */
extern uint8_t TKY_BaseInit(TKY_BaseInitTypeDef TKY_BaseInitStruct);

/* *********************************************************************************************
* @fn TKY_CHInit
*
* @brief TouchKey channel parameter initialization
*
* @param TKY_CHInitStruct - Initialized parameters
*
* @return Initialization result - 0x00: Initialization successful
* - 0x01: There is an error in the touch channel parameters
* - 0x02: Channel conversion queue position error (exceeding the maximum number of conversion channels)
* - 0x04: Baseline value setting error
* - 0x08: Threshold setting error */
extern uint8_t TKY_CHInit(TKY_ChannelInitTypeDef TKY_CHInitStruct);

/* *********************************************************************************************
* @fn TKY_GetCurChannelMean
* @brief Gets the average value of the current channel, mainly used to set baseline and threshold value.
*
* @param curChNum - Current conversion channel
* @param chargeTime - is the charging time of the current channel
* @param disChargeTime - is the discharge time of the current channel
* @param averageNum - is the average sum total number
*
* @return Average of multiple measurements */
extern uint16_t TKY_GetCurChannelMean(uint8_t curChNum, uint8_t chargeTime, uint8_t disChargeTime, uint16_t averageNum);

/* *********************************************************************************************
* @fn TKY_CaliCrowedModBaseLine
* @brief The baseline value in calibration dense mode is performed after all channels are initially identified, including the driver shield pin
*
* @param curQueueNum - Current conversion channel number
* @param averageNum - Average sum total
*
* @return The baseline value of the current test. */
extern uint16_t TKY_CaliCrowedModBaseLine( uint8_t curQueueNum,uint16_t averageNum);

/* *********************************************************************************************
* @fn TKY_GetCurQueueValue
*
* @brief Get the value processed by the specified channel
*
* @param curQueueNum - The channel that currently needs to get the value
*
* @return Filter mode 1 is used for the filtered measurement value (the difference is given to the baseline value and then the threshold value is compared.
* Comparison), filtering modes 3, 5, 7, 9, CS10 and CS10BLE are filtered for the selected channel
* Change value (can be compared directly with the threshold) */
extern int16_t TKY_GetCurQueueValue(uint8_t curQueueNum);

/* *********************************************************************************************
* @fn TKY_PollForFilterMode_1
*
* @brief TouchKey main loop polling mode, suitable for filter 1 filtering
*
* @return The key value of each channel, and the return value of each bit corresponds to the keys of each queue. For example, the touch channel of queue 0 has
* key, corresponding to the lowest position 1.Note that it is not the ADC channel number */
extern uint16_t TKY_PollForFilterMode_1(void);

/* *********************************************************************************************
* @fn TKY_PollForFilterMode_3
*
* @brief TouchKey main loop polling mode, suitable for filter 3 filtering, blocking of execution process
*
* @return The key value of each channel, the return value of each bit corresponds to the keys of each queue. For example, the touch channel of queue 0 has a key.
* Corresponding to the lowest position 1.Note that it is not the ADC channel number */
extern uint16_t TKY_PollForFilterMode_3(void);

/* *********************************************************************************************
* @fn TKY_PollForFilterMode_5
*
* @brief TouchKey main loop polling mode, consistent with FilerMode3 effect, non-blocking of execution
*
* @return The key value of each channel, and the return value of each bit corresponds to the keys of each queue. For example, the touch channel of queue 0 has
* key, corresponding to the lowest position 1.Note that it is not the ADC channel number */
extern uint16_t TKY_PollForFilterMode_5(void);

/* *********************************************************************************************
* @fn TKY_PollForFilterMode_7
*
* @brief TouchKey main loop polling mode, suitable for filter 7 filtering, blocking of execution process
*
* @return The key value of each channel, and the return value of each bit corresponds to the keys of each queue. For example, the touch channel of queue 0 has
* key, corresponding to the lowest position 1.Note that it is not the ADC channel number */
extern uint16_t TKY_PollForFilterMode_7(void);

/* *********************************************************************************************
* @fn TKY_PollForFilterMode_9
*
* @brief TouchKey main loop polling mode, suitable for dense mode filters, blocking of execution process
*
* @return The key value of each channel, and the return value of each bit corresponds to the keys of each queue. For example, the touch channel of queue 0 has
* key, corresponding to the lowest position 1.Note that it is not the ADC channel number */
extern uint16_t TKY_PollForFilterMode_9(void);

/* *********************************************************************************************
* @fn TKY_PollForFilterMode_CS10
*
* @brief TouchKey main loop polling mode, suitable for CS10V testing
*
* @return The key value of each channel, and the return value of each bit corresponds to the keys of each queue. For example, the touch channel of queue 0 has
* key, corresponding to the lowest position 1.Note that it is not the ADC channel number */
extern uint16_t TKY_PollForFilterMode_CS10(void);


/* *********************************************************************************************
* @fn TKY_ScanForWakeUp
*
* @brief TouchKey sleep detection, main loop timed poll
*
* @param scanBitValue - The bit of the value is set to 1, which is the specified scan bit
* Example 0x0013, scan the sequence 0, 1, and 4 channels
*
* @return The scan channel is suspected to have a key value, and the return value of each bit corresponds to the keys of each queue, such as the touch of queue 0
* The channel has a button, corresponding to the lowest position 1.Note that it is not the ADC channel number */
extern uint16_t TKY_ScanForWakeUp(uint16_t scanBitValue);

/* *********************************************************************************************
* @fn TKY_SetCurQueueSleepStatus
*
* @brief Sets the sleep mode of the specified queue channel, and the function TKY_PollForFilterMode after sleep
* This channel will no longer be scanned
*
* @param curQueueNum - The channel that needs to be set currently
* @param sleepStatus - 0: No sleep
* - 1: Sleep
*
* @return Set status - 0: Setting successfully
* - 1: Exceeded the maximum queue length */
extern uint8_t TKY_SetCurQueueSleepStatus(uint8_t curQueueNum, uint8_t sleepStatus);

/* *********************************************************************************************
* @fn TKY_SetSleepStatusValue
*
* @brief Sets the sleep mode of all queue channels, with the function TKY_SetCurQueueSleepStatus
* Compared to this function, this function directly configures all channels to sleep state
*
* @param setValue - Sets the sleep state bitwise in order of detection queues, 0: not sleep, 1: sleep.
*
* @return None */
extern void TKY_SetSleepStatusValue(uint16_t setValue);

/* *********************************************************************************************
* @fn TKY_ReadSleepStatusValue
*
* @brief Gets the sleep mode of all queue channels
*
* @param None
*
* @return Returns the sleep state of all channels. If the corresponding bit is 0, it will not sleep. If it is 1, it will sleep. */
extern uint16_t TKY_ReadSleepStatusValue(void);

/* *********************************************************************************************
* @fn TKY_SetCurQueueChargeTime
*
* @brief Sets the charge and discharge time of the specified queue channel.For filter mode 1&5, to ensure security update settings,
* Please check the idle status (TKY_GetCurIdleStatus) and update it when it is idle
*
* @param curQueueNum - The measurement channel that needs to be set at present, note that it is not the ADC channel number
* @param chargeTime - For the set charging time parameters, the parameter value is of the meaning, please refer to the chip manual
* @param disChargeTime - The set discharge time parameters, the parameter value and meaning are consulted for the chip manual
*
* @return 0 - Setting successfully
* 1 - Maximum queue length exceeded.
********************************************************************************************* */
extern uint8_t TKY_SetCurQueueChargeTime( uint8_t curQueueNum,
										uint8_t chargeTime,
										uint8_t disChargeTime );

/* *********************************************************************************************
* @fn TKY_SetCurQueueThreshold
* @brief Sets the threshold for the specified queue channel.For filter mode 1&5, to ensure security update settings,
* Please check the idle status (TKY_GetCurIdleStatus) and update it when it is idle
*
* @param curQueueNum - The channel that needs to be set currently.
* @param Threshold - The upper limit threshold of this channel, that is, the decision value of the "press" state when the key is detected
* @param Threshold2 - The lower limit threshold of this channel, that is, the decision value of the "release" state when the key is used to detect
*
* @return 0 - then the setting is successful
* 1 - Exceed the maximum queue length
********************************************************************************************* */
extern uint8_t TKY_SetCurQueueThreshold(uint8_t curQueueNum,
									  uint16_t threshold,
									  uint16_t threshold2);

/* *********************************************************************************************
* @fn TKY_GetCurIdleStatus
*
* @brief Get idle status
*
* @return Return whether it is idle
********************************************************************************************* */
extern uint8_t TKY_GetCurIdleStatus(void);

/* *********************************************************************************************
* @fn TKY_GetCurVersion
*
* @brief Get the current version number
*
* @return Current version number */
extern uint16_t TKY_GetCurVersion(void);

/* *********************************************************************************************
* @fn TKY_GetCurQueueBaseLine
*
* @brief Get the specified channel baseline value
*
* @param curQueueNum - The selected measurement touch channel queue number, note that it is not the ADC channel number
*
* @return Baseline value of selected queue channel */
extern uint16_t TKY_GetCurQueueBaseLine(uint8_t curQueueNum);

/* *********************************************************************************************
* @fn TKY_SetCurQueueBaseLine
*
* @brief Gets the specified channel baseline value.
*
* @param curQueueNum - The channel that currently needs to set the value
* @param baseLineValue - The setting value of the current channel.
*
* @return The processing value of the current channel. */
extern void TKY_SetCurQueueBaseLine(uint8_t curQueueNum, uint16_t baseLineValue);

/* *********************************************************************************************
* @fn TKY_SetBaseRefreshSampleNum
*
* @brief Set the number of baseline refresh samples, and refresh the baseline every how many samples.For filter mode 1&5,
* To ensure security update settings, please check the idle status (TKY_GetCurIdleStatus),
* Update when idle
*
* @param newValue - New samples
*
* @return None */
extern void TKY_SetBaseRefreshSampleNum(uint32_t newValue);

/* *********************************************************************************************
* @fn TKY_SetBaseDownRefreshSlow
*
* @brief Sets the multiple parameter for the baseline up update.For filter mode 1&5, to ensure security update settings,
* Please check the idle status (TKY_GetCurIdleStatus) and update it when it is idle
*
* @param newValue - New parameters
*
* @return None */
extern void TKY_SetBaseUpRefreshDouble(uint8_t newValue);

/* *********************************************************************************************
* @fn TKY_SetBaseDownRefreshSlow
*
* @brief Sets the deceleration parameters that update the baseline downwards.For filter mode 1&5, to ensure security update settings,
* Please check the idle status (TKY_GetCurIdleStatus) and update it when it is idle
*
* @param newValue - New parameters
*
* @return None */
extern void TKY_SetBaseDownRefreshSlow(uint8_t newValue);

/* *********************************************************************************************
* @fn TKY_SetFilterMode
*
* @brief Sets the new filtering mode.It is not recommended to use in general scenarios, only use special scenario switching in hibernation scenarios
*
* @param newValue - Filter mode
*
* @return None */
extern void TKY_SetFilterMode(uint8_t newValue);

/* *********************************************************************************************
* @fn TKY_ClearHistoryData
*
* @brief Clears the historical data of the current filter.The application is interrupted for a long time in touch conversion.
* Scenarios where historical data are meaningless.
*
* @param curFilterMode - Current filtering mode
*
* @return None */
extern void TKY_ClearHistoryData(uint8_t curFilterMode);

/* *********************************************************************************************
* @fn TKY_SaveAndStop
*
* @brief saves the touch-related register value and pauses the touch function when judging that the touch scan is idle.
* to free up the ADC module for ADC conversion
*
* @param None
*
* @return None */
extern void TKY_SaveAndStop(void);

/* *********************************************************************************************
* @fn TKY_LoadAndRun
*
* @brief Load the touch-related register value and restart the paused touch button function
*
* @param None
*
* @return None */
extern void TKY_LoadAndRun(void);

/* *********************************************************************************************
* @fn TKY_GetCurQueueRealVal
*
* @brief Gets the original measured value of the specified channel, unfiltered value.
*
* @param curQueueNum - The channel that currently needs to take the value.
*
* @return The original measurement value of the current channel. */
extern uint16_t TKY_GetCurQueueRealVal(uint8_t curQueueNum);

#endif
