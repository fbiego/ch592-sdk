/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* The header file contains */
#include "CONFIG.h"
#include "RF_PHY.h"
#include "HAL.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
uint8_t taskID;
uint8_t TX_DATA[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};

volatile uint8_t tx_end_flag=0;
volatile uint8_t rx_end_flag=0;

/* ***************************************************************************
* @fn RF_Wait_Tx_End
*
* @brief The manual mode waits for the sending to complete, the automatic mode waits for the sending-received completion, and must wait in RAM. User code can be executed while waiting, but it should be noted that the executed code must be run in RAM, otherwise it will affect the sending.
*
* @return none */
__HIGH_CODE
__attribute__((noinline))
void RF_Wait_Tx_End()
{
    uint32_t i=0;
    while(!tx_end_flag)
    {
        i++;
        __nop();
        __nop();
        // 5ms
        if(i>(FREQ_SYS/1000))
        {
            tx_end_flag = TRUE;
        }
    }
}

/* ***************************************************************************
* @fn RF_Wait_Rx_End
*
* @brief The automatic mode waits for the reply to be sent to complete. It must wait in RAM. User code can be executed while waiting, but it should be noted that the executed code must be run in RAM, otherwise it will affect the sending.
*
* @return none */
__HIGH_CODE
__attribute__((noinline))
void RF_Wait_Rx_End()
{
    uint32_t i=0;
    while(!rx_end_flag)
    {
        i++;
        __nop();
        __nop();
        // About 5ms timeout
        if(i>(FREQ_SYS/1000))
        {
            rx_end_flag = TRUE;
        }
    }
}

/* ***************************************************************************
* @fn RF_2G4StatusCallBack
*
* @brief RF state callback, this function is called in interrupt.Note: The RF reception or sending API cannot be called directly in this function, and it is necessary to use event methods to call it.
* Note that the variables involved in the function are directly used or called in this callback should be called, and this function is called in the interrupt.
*
* @param sta - Status type
* @param crc - crc verification result
* @param rxBuf - Data buf pointer
*
* @return none */
void RF_2G4StatusCallBack(uint8_t sta, uint8_t crc, uint8_t *rxBuf)
{
    switch(sta)
    {
        case TX_MODE_TX_FINISH:
        {
            break;
        }
        case TX_MODE_TX_FAIL:
        {
            tx_end_flag = TRUE;
            break;
        }
        case TX_MODE_RX_DATA:
        {
            tx_end_flag = TRUE;
            if (crc == 0) {
                uint8_t i;

                PRINT("tx recv,rssi:%d\n", (int8_t)rxBuf[0]);
                PRINT("len:%d-", rxBuf[1]);

                for (i = 0; i < rxBuf[1]; i++) {
                    PRINT("%x ", rxBuf[i + 2]);
                }
                PRINT("\n");
            } else {
                if (crc & (1<<0)) {
                    PRINT("crc error\n");
                }

                if (crc & (1<<1)) {
                    PRINT("match type error\n");
                }
            }
            break;
        }
        case TX_MODE_RX_TIMEOUT: // Timeout is about 200us
        {
            tx_end_flag = TRUE;
            break;
        }
        case TX_MODE_HOP_SHUT:
        {
            tx_end_flag = TRUE;
            PRINT("TX_MODE_HOP_SHUT...\n");
            tmos_set_event(taskID, SBP_RF_CHANNEL_HOP_TX_EVT);
            break;
        }

        case RX_MODE_RX_DATA:
        {
            if (crc == 0) {
                uint8_t i;

                RF_Wait_Rx_End();
                PRINT("rx recv, rssi: %d\n", (int8_t)rxBuf[0]);
                PRINT("len:%d-", rxBuf[1]);
                
                for (i = 0; i < rxBuf[1]; i++) {
                    PRINT("%x ", rxBuf[i + 2]);
                }
                PRINT("\n");
            } else {
                if (crc & (1<<0)) {
                    PRINT("crc error\n");
                }

                if (crc & (1<<1)) {
                    PRINT("match type error\n");
                }
            }
            tmos_set_event(taskID, SBP_RF_RF_RX_EVT);
            break;
        }
        case RX_MODE_TX_FINISH:
        {
            rx_end_flag = TRUE;
            break;
        }
        case RX_MODE_TX_FAIL:
        {
            rx_end_flag = TRUE;
            break;
        }
        case RX_MODE_HOP_SHUT:
        {
            PRINT("RX_MODE_HOP_SHUT...\n");
            rx_end_flag = TRUE;
            tmos_set_event(taskID, SBP_RF_CHANNEL_HOP_RX_EVT);
            break;
        }
    }
}

/* ***************************************************************************
* @fn RF_ProcessEvent
*
* @brief RF event handling
*
* @param task_id - Task ID
* @param events - Event flags
*
* @return Unfinished Event */
uint16_t RF_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(task_id)) != NULL)
        {
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    if(events & SBP_RF_START_DEVICE_EVT)
    {
        tmos_start_task(taskID, SBP_RF_PERIODIC_EVT, 1000);
        return events ^ SBP_RF_START_DEVICE_EVT;
    }
    if(events & SBP_RF_PERIODIC_EVT)
    {
        RF_Shut();
        TX_DATA[0]--;
        tx_end_flag = FALSE;
        if(!RF_Tx(TX_DATA, 10, 0xFF, 0xFF))
        {
            RF_Wait_Tx_End();
        }
        tmos_start_task(taskID, SBP_RF_PERIODIC_EVT, 1000);
        return events ^ SBP_RF_PERIODIC_EVT;
    }
    if(events & SBP_RF_RF_RX_EVT)
    {
        uint8_t state;
        RF_Shut();
        TX_DATA[0]++;
        rx_end_flag = FALSE;
        state = RF_Rx(TX_DATA, 10, 0xFF, 0xFF);
        return events ^ SBP_RF_RF_RX_EVT;
    }
    // Turn on frequency hopping sending
    if(events & SBP_RF_CHANNEL_HOP_TX_EVT)
    {
        PRINT("\n------------- hop tx...\n");
        if(RF_FrequencyHoppingTx(16))
        {
            tmos_start_task(taskID, SBP_RF_CHANNEL_HOP_TX_EVT, 100);
        }
        else
        {
            tmos_start_task(taskID, SBP_RF_PERIODIC_EVT, 1000);
        }
        return events ^ SBP_RF_CHANNEL_HOP_TX_EVT;
    }
    // Turn on frequency hopping reception
    if(events & SBP_RF_CHANNEL_HOP_RX_EVT)
    {
        PRINT("hop rx...\n");
        if(RF_FrequencyHoppingRx(200))
        {
            tmos_start_task(taskID, SBP_RF_CHANNEL_HOP_RX_EVT, 400);
        }
        else
        {

            rx_end_flag = FALSE;
            RF_Rx(TX_DATA, 10, 0xFF, 0xFF);
        }
        return events ^ SBP_RF_CHANNEL_HOP_RX_EVT;
    }
    return 0;
}

/* ***************************************************************************
* @fn RF_Init
*
* @brief RF Initialization
*
* @return none */
void RF_Init(void)
{
    uint8_t    state;
    rfConfig_t rfConfig;

    tmos_memset(&rfConfig, 0, sizeof(rfConfig_t));
    taskID = TMOS_ProcessEventRegister(RF_ProcessEvent);
    rfConfig.accessAddress = 0x71764129; // 0x555555555 and 0xAAAAAAAA (recommendation is recommended not to exceed 24 bits, and not to exceed 6 consecutive 0s or 1s)
    rfConfig.CRCInit = 0x555555;
    rfConfig.ChannelMap = 0xFFFFFFFF;
    rfConfig.LLEMode = LLE_MODE_AUTO;
    rfConfig.rfStatusCB = RF_2G4StatusCallBack;
    rfConfig.RxMaxlen = 251;
#if (CLK_OSC32K != 0)
    //It is better to choose a shorter heartbeat interval for the internal clock.
    rfConfig.HeartPeriod = 4;
#endif
    state = RF_Config(&rfConfig);
    PRINT("rf 2.4g init: %x\n", state);
//    { // RX mode
//        PRINT("RX mode...\n");
//        tmos_set_event(taskID, SBP_RF_CHANNEL_HOP_RX_EVT);
//    }
    RF_Wait_Rx_End();
    { // TX mode
        PRINT("TX mode...\n");
        tmos_set_event(taskID, SBP_RF_CHANNEL_HOP_TX_EVT);
    }
}

/******************************** endfile @ main ******************************/
