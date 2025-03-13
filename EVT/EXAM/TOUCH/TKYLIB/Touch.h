#ifndef TOUCH_KEY_H_
#define TOUCH_KEY_H_

#include "CH59x_common.h"
#include "TouchKey_CFG.h"
#include "wchtouch.h"
// Whether to enable touch data printing
#define PRINT_EN 0

#if (PRINT_EN)
  #define dg_log               printf
#else
  #define dg_log(x,...)
#endif

/************************KEY_FIFO_DEFINE******************************/
#define KEY_COUNT       	TKY_MAX_QUEUE_NUM               // Number of keys

#define TKY_SHIELD_PIN      GPIO_Pin_4                      // Drive shield pin
/* Whether to enable TMOS */
#ifndef TMOS_EN
#define TMOS_EN     0
#endif
/* Whether the touch hibernation function is supported */
#if TMOS_EN
#define TKY_SLEEP_EN 1
#else
#define TKY_SLEEP_EN 0
#endif

#ifndef TKY_FILTER_MODE
#define TKY_FILTER_MODE FILTER_MODE_3
#endif

#if (TKY_FILTER_MODE == FILTER_MODE_1)
#define TKY_PollForFilter() TKY_PollForFilterMode_1()
#elif (TKY_FILTER_MODE == FILTER_MODE_3)
#define TKY_PollForFilter() TKY_PollForFilterMode_3()
#elif (TKY_FILTER_MODE == FILTER_MODE_5)
#define TKY_PollForFilter() TKY_PollForFilterMode_5()
#elif (TKY_FILTER_MODE == FILTER_MODE_7)
#define TKY_PollForFilter() TKY_PollForFilterMode_7()
#elif (TKY_FILTER_MODE == FILTER_MODE_9)
#define TKY_PollForFilter() TKY_PollForFilterMode_9()
#endif


#if (TKY_FILTER_MODE == FILTER_MODE_7)
#define TKY_MEMHEAP_SIZE    	(KEY_COUNT*TKY_BUFLEN*2)     // Externally define the data buffer length
#else
#define TKY_MEMHEAP_SIZE   		(KEY_COUNT*TKY_BUFLEN)     	 // Externally define the data buffer length
#endif


typedef struct
{
	  UINT32 PaBit;     // ---Record the PIN pin corresponding to A_IO, and use the output assignment to
	  UINT32 PbBit;     // ---Record the PIN pin corresponding to B_IO, and use the output assignment
	  UINT16 tkyQueueAll;
	  UINT16 RFU;
}TOUCH_S;

/* Key ID, mainly used for the entry parameters of tky_GetKeyState() function */
typedef enum
{
    KID_K0 = 0,
    KID_K1,
    KID_K2,
    KID_K3,
    KID_K4,
    KID_K5,
    KID_K6,
    KID_K7,
    KID_K8,
    KID_K9,
    KID_K10,
    KID_K11
}KEY_ID_E;

/* The button filtering time is 50ms, unit is 10ms.
Only when the 50ms state remains unchanged can it be considered valid, including two events: bounce and press.
Even if the key circuit does not perform hardware filtering, this filtering mechanism can ensure reliable detection of key events. */
#define NORMAL_KEY_MODE 0                // Independent key trigger mode
#define TOUCH_KEY_MODE  1                // Touch button trigger mode

#define KEY_MODE    NORMAL_KEY_MODE      // Key Mode Settings
#define KEY_FILTER_TIME   2              // Number of key filtering times
#define KEY_LONG_TIME     0              // Unit: The interval time of tky_KeyScan() calls shall prevail. If the number of times exceeds the number, the long press event is considered

typedef uint8_t (*pIsKeyDownFunc)(void);

/* Each key corresponds to 1 global structure variable. */
typedef struct
{
    /* Below is a function pointer pointing to the function that determines whether the key presses */
    /* ,1 */
    pIsKeyDownFunc IsKeyDownFunc;
    uint8_t  Count;         // Filter Counter
    uint16_t LongCount;     // Press and hold the counter
    uint16_t LongTime;      // The duration of pressing the button is 0 means that the long press is not detected
    uint8_t  State;         // The current status of the button (press or pop up)
    uint8_t  RepeatSpeed;   // Continuous key cycle
    uint8_t  RepeatCount;   // Continuous key counter
}KEY_T;

/* Define key-value codes, and press each key in the following order must be timed to press, pop up and long press events.

It is recommended to use enum, not #define, reason:
(1) Easy to add key values, easy to adjust the order, making the code look more comfortable
(2) The compiler can help us avoid key-value duplication. */
typedef enum
{
    KEY_NONE = 0,           // 0 indicates key event */

    KEY_0_DOWN,             // Press 1 button
    KEY_0_UP,               // 1 button to pop up
    KEY_0_LONG,             // 1 minute

    KEY_1_DOWN,             // Press 2 keys
    KEY_1_UP,               // 2 buttons to pop up
    KEY_1_LONG,             // 2-minute

    KEY_2_DOWN,             // Press 3 keys
    KEY_2_UP,               // 3 keys to play
    KEY_2_LONG,             // 3-minute studs

    KEY_3_DOWN,             // Press 4 keys
    KEY_3_UP,               // 4 keys to pop up
    KEY_3_LONG,             // 4-minute corner

    KEY_4_DOWN,             // Press 5 keys
    KEY_4_UP,               // 5 keys to pop up
    KEY_4_LONG,             // 5-minute studs

    KEY_5_DOWN,             // Press 6 keys
    KEY_5_UP,               // 6 keys to pop up
    KEY_5_LONG,             // 6 

    KEY_6_DOWN,             // 7 
    KEY_6_UP,               // 7 
    KEY_6_LONG,             // 7 keys

    KEY_7_DOWN,             // Press 8 keys
    KEY_7_UP,               // 8 keys to play
    KEY_7_LONG,             // 8 

    KEY_8_DOWN,             // Press 9 keys
    KEY_8_UP,               // 9 keys to pop up
    KEY_8_LONG,             // 9 

    KEY_9_DOWN,             // Press 0 key
    KEY_9_UP,               // 0 buttons to pop up
    KEY_9_LONG,             // 0 mins

    KEY_10_DOWN,            // # 
    KEY_10_UP,              // #Keys
    KEY_10_LONG,            // #Long

    KEY_11_DOWN,            // * Press the key
    KEY_11_UP,              // * keys pop up
    KEY_11_LONG,            // *Long-toned
}KEY_ENUM;

/* Use variables for key FIFO */
#define KEY_FIFO_SIZE   64          // Can be modified according to the usage environment and hardware requirements*/

typedef struct
{
    uint8_t Buf[KEY_FIFO_SIZE];     // 
    uint8_t Read;                   // Buffer read pointer
    uint8_t Write;                  // Buffer write pointer
}KEY_FIFO_T;

/************************WHEEL_SLIDER_DEFINE****************************/
#define TOUCH_DECIMAL_POINT_PRECISION       (100)
#define TOUCH_WHEEL_ELEMENTS            	(KEY_COUNT)
#define TOUCH_WHEEL_RESOLUTION              (360)
#define TOUCH_OFF_VALUE    					(0xFFFF)

/************************LINE_SLIDER_DEFINE****************************/

/************************TOUCH_PAD_DEFINE****************************/

extern uint8_t wakeupflag; // 0  sleep mode   1  wakeup sta
extern volatile TOUCH_S tkyPinAll;
extern uint16_t keyData, scanData;
extern uint8_t wakeUpCount, wakeupflag;

/* Function declaration for external calls */
extern void touch_InitKey(void);
extern void touch_ScanWakeUp(void);
extern void touch_ScanEnterSleep(void);
extern void touch_PutKey(uint8_t _KeyCode);
extern uint8_t touch_GetKey(void);
extern uint8_t touch_GetKeyState(KEY_ID_E _ucKeyID);
extern void touch_SetKeyParam(uint8_t _ucKeyID, uint16_t _LongTime, uint8_t  _RepeatSpeed);
extern void touch_ClearKey(void);
extern void touch_KeyScan(void);
extern void touch_InfoDebug(void);
extern uint16_t touch_DetectWheelSlider(void);
extern void touch_GPIOModeCfg(GPIOModeTypeDef mode);
extern void touch_GPIOSleep(void);
#endif
