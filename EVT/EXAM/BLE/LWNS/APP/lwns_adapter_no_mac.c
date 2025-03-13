/* ********************************* (C) COPYRIGHT ***************************
* File Name: lwns_adapter_no_mac.c
* Author: WCH
* Version: V1.0
* Date: 2021/06/20
* Description: lwns adapter, does not use the mac protocol, pure transparent transmission
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */
#include "lwns_adapter_no_mac.h"
#include "lwns_sec.h"

// Each file has a separate debug print switch, setting 0 can prohibit internal printing of this file.
#define DEBUG_PRINT_IN_THIS_FILE    1
#if DEBUG_PRINT_IN_THIS_FILE
  #define PRINTF(...)    PRINT(__VA_ARGS__)
#else
  #define PRINTF(...) \
    do                \
    {                 \
    } while(0)
#endif

#if LWNS_USE_NO_MAC // Whether to enable pure transmissive mac protocol, that is, the mac layer does not participate in any action, please note that only one mac layer protocol can be enabled.

//for lwns_packet_buffer save
__attribute__((aligned(4))) static lwns_qbuf_list_t qbuf_memp[QBUF_MANUAL_NUM];

    //for lwns_route_entry manage
  #if ROUTE_ENTRY_MANUAL_NUM
__attribute__((aligned(4))) static lwns_route_entry_data_t route_entry_memp[ROUTE_ENTRY_MANUAL_NUM];
  #endif

//for neighbor manage
__attribute__((aligned(4))) static lwns_neighbor_list_t neighbor_memp[LWNS_NEIGHBOR_MAX_NUM];

static void ble_new_neighbor_callback(lwns_addr_t *n);     // Discover new neighbor callback function
static BOOL ble_phy_output(uint8_t *dataptr, uint8_t len); //
static void RF_2G4StatusCallBack(uint8_t sta, uint8_t crc, uint8_t *rxBuf);

static uint8_t  lwns_adapter_taskid;
static uint16_t lwns_adapter_ProcessEvent(uint8_t task_id, uint16_t events);
static uint8_t  lwns_phyoutput_taskid;
static uint16_t lwns_phyoutput_ProcessEvent(uint8_t task_id, uint16_t events);

/**
 * lwnslwns
 */
static lwns_fuc_interface_t ble_lwns_fuc_interface = {
    .lwns_phy_output = ble_phy_output,
    .lwns_rand = tmos_rand,
    .lwns_memcpy = tmos_memcpy,
    .lwns_memcmp = tmos_memcmp,
    .lwns_memset = tmos_memset,
    .new_neighbor_callback = ble_new_neighbor_callback,
};

static uint8_t ble_phy_manage_state; // Phy status management

volatile uint8_t tx_end_flag=0;

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
        // About 5ms timeout
        if(i>(FREQ_SYS/1000))
        {
            tx_end_flag = TRUE;
        }
    }
}

/* ***************************************************************************
* @fn RF_2G4StatusCallBack
*
* @brief RF status callback, note: The RF reception or sending API cannot be called directly in this function, and it needs to be called using events.
*
* @param sta - Status type
* @param crc - crc verification result
* @param rxBuf - Data buf pointer
*
* @return None. */
static void RF_2G4StatusCallBack(uint8_t sta, uint8_t crc, uint8_t *rxBuf)
{
    switch(sta)
    {
        case RX_MODE_RX_DATA:
        {
            if(crc == 1)
            {
                PRINTF("crc error\n");
            }
            else if(crc == 2)
            {
                PRINTF("match type error\n");
            }
            else
            {
                uint8_t *pMsg;
  #if LWNS_ENCRYPT_ENABLE // Whether to enable message encryption
                if(((rxBuf[1] % 16) == 1) && (rxBuf[1] >= 17) && (rxBuf[1] > rxBuf[2]))
                { //16
                    //rxBuf[1] - 116
                    pMsg = tmos_msg_allocate(rxBuf[1]); // Apply for memory space, the real data length does not need to be decrypted
                    if(pMsg != NULL)
                    {
                        lwns_msg_decrypt(rxBuf + 3, pMsg + 1, rxBuf[1] - 1); //
                        if((rxBuf[2] ^ pMsg[rxBuf[2]]) == pMsg[rxBuf[2] + 1])
                        {
                            pMsg[0] = rxBuf[2];      //
                            PRINTF("send rx msg\n"); // Send the received data to the receiving process
                            tmos_msg_send(lwns_adapter_taskid, pMsg);
                        }
                        else
                        {
                            PRINTF("verify rx msg err\n"); // Verification failed
                            tmos_msg_deallocate(pMsg);
                        }
                    }
                    else
                    {
                        PRINTF("send rx msg failed\n"); // The memory application failed and the received data could not be sent.
                    }
                }
                else
                {
                    PRINTF("bad len\n"); // Bag length is wrong
                }
  #else
                if(rxBuf[1] >= LWNS_PHY_OUTPUT_MIN_SIZE)
                { //
                    pMsg = tmos_msg_allocate(rxBuf[1] + 1);
                    if(pMsg != NULL)
                    {
                        PRINTF("send rx msg\n"); // Send the received data to the receiving process
                        tmos_memcpy(pMsg, rxBuf + 1, rxBuf[1] + 1);
                        tmos_msg_send(lwns_adapter_taskid, pMsg);
                    }
                    else
                    {
                        PRINTF("rx msg failed\n"); // The memory application failed and the received data could not be sent.
                    }
                }
                else
                {
                    PRINTF("bad len\n"); //
                }
  #endif
            }
            tmos_set_event(lwns_adapter_taskid, LWNS_PHY_RX_OPEN_EVT); // Reopen Receive
            break;
        }
        case TX_MODE_TX_FINISH:
        case TX_MODE_TX_FAIL:
            tmos_stop_task(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_FINISH_EVT); // Stop timeout counting
            tmos_set_event(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_FINISH_EVT); // Enter the sending process
            break;
        default:
            break;
    }
}

/*********************************************************************
 * @fn      RF_Init
 *
 * @brief   RF .
 *
 * @param   None.
 *
 * @return  None.
 */
void RF_Init(void)
{
    uint8_t    state;
    rfConfig_t rfConfig;
    tmos_memset(&rfConfig, 0, sizeof(rfConfig_t));
    rfConfig.accessAddress = 0x17267162; // The use of 0x55555555 and 0xAAAAAAAA (it is recommended not to invert more than 24 bits, and not to exceed 6 consecutive 0s or 1s). There are about 2.3 billion access addresses that correctly comply with the corresponding rules.
    rfConfig.CRCInit = 0x555555;
    rfConfig.Channel = 8;
    rfConfig.LLEMode = LLE_MODE_BASIC; // |LLE_MODE_EX_CHANNEL; // Enable LLE_MODE_EX_CHANNEL means select rfConfig.Frequency as the communication frequency point
    rfConfig.rfStatusCB = RF_2G4StatusCallBack;
    state = RF_Config(&rfConfig);
    PRINTF("rf 2.4g init: %x\n", state);
}

/* ***************************************************************************
* @fn lwns_init
*
* @brief lwns initialization.
*
* @param None.
*
* @return None. */
void lwns_init(void)
{
    uint8_t       s;
    lwns_config_t cfg;
    tmos_memset(&cfg, 0, sizeof(lwns_config_t));
    cfg.lwns_lib_name = (uint8_t *)VER_LWNS_FILE; //
    cfg.qbuf_num = QBUF_MANUAL_NUM;               // It must be allocated at least 1 memory unit, which is defined according to the number of ports used in your program corresponding to the qbuf unit used by the module.
    cfg.qbuf_ptr = qbuf_memp;                     //mesh3qbuf(uni/multi)netflood21
    cfg.routetable_num = ROUTE_ENTRY_MANUAL_NUM;  // If you need to use mesh, you must allocate the routing table memory space.Otherwise, the mesh initialization will not be successful.
  #if ROUTE_ENTRY_MANUAL_NUM
    cfg.routetable_ptr = route_entry_memp;
  #else
    cfg.routetable_ptr = NULL;
  #endif
    cfg.neighbor_num = LWNS_NEIGHBOR_MAX_NUM;                      // Neighbor table number, must be allocated
    cfg.neighbor_list_ptr = neighbor_memp;                         // Neighbor table memory space
    cfg.neighbor_mod = LWNS_NEIGHBOR_AUTO_ADD_STATE_RECALL_NOTADD; // The default management mode for the neighbor table is to receive all packets and not add neighbors.
  #if LWNS_ADDR_USE_BLE_MAC
    GetMACAddress(cfg.addr.v8); //mac
  #else
    // Self-defined address
    uint8_t MacAddr[6] = {0, 0, 0, 0, 0, 1};
    tmos_memcpy(cfg.addr.v8, MacAddr, LWNS_ADDR_SIZE);
  #endif
    s = lwns_lib_init(&ble_lwns_fuc_interface, &cfg); // The underlying initialization of the lwns library
    if(s)
    {
        PRINTF("%s init err:%d\n", VER_LWNS_FILE, s);
    }
    else
    {
        PRINTF("%s init ok\n", VER_LWNS_FILE);
    }
    lwns_adapter_taskid = TMOS_ProcessEventRegister(lwns_adapter_ProcessEvent);
    lwns_phyoutput_taskid = TMOS_ProcessEventRegister(lwns_phyoutput_ProcessEvent);
    tmos_start_reload_task(lwns_phyoutput_taskid, LWNS_HTIMER_PERIOD_EVT, MS1_TO_SYSTEM_TIME(LWNS_HTIMER_PERIOD_MS));
    ble_phy_manage_state = BLE_PHY_MANAGE_STATE_FREE;
    RF_Shut();
    RF_Rx(NULL, 0, USER_RF_RX_TX_TYPE, USER_RF_RX_TX_TYPE); //RF
}

/* ***************************************************************************
* @fn ble_new_neighbor_callback
*
* @brief callback function when a new neighbor is found.
*
* @param n - The address of the new neighbor.
*
* @return None. */
static void ble_new_neighbor_callback(lwns_addr_t *n)
{
    PRINTF("new neighbor: %02x %02x %02x %02x %02x %02x\n", n->v8[0], n->v8[1],
           n->v8[2], n->v8[3], n->v8[4], n->v8[5]);
}

/*********************************************************************
 * @fn      ble_phy_output
 *
 * @brief   lwns
 *
 * @param   dataptr     - .
 * @param   len         - .
 *
 * @return  TRUE if success, FLASE is failed.
 */
static BOOL ble_phy_output(uint8_t *dataptr, uint8_t len)
{
    uint8_t *pMsg;
  #if LWNS_ENCRYPT_ENABLE
    pMsg = tmos_msg_allocate((((len + 1 + 15) & 0xf0) + 1 + 1)); //116+1+1
  #else
    pMsg = tmos_msg_allocate(len + 1); // Apply for memory space to store messages, store sending length +1
  #endif
    if(pMsg != NULL)
    { // Successful application
  #if LWNS_ENCRYPT_ENABLE
        // There are two bytes reserved inside the lwns buffer, and users can directly use dataptr[len] to assign two bytes of content.
        dataptr[len] = dataptr[len - 1] ^ len;                      //port
        pMsg[1] = len;                                              // The real data length occupies one byte and is not encrypted. It is used to receive and perform the first verification step.
        pMsg[0] = lwns_msg_encrypt(dataptr, pMsg + 2, len + 1) + 1; //
  #else
        pMsg[0] = len;
        tmos_memcpy(pMsg + 1, dataptr, len);
  #endif
        tmos_msg_send(lwns_phyoutput_taskid, pMsg);
        return TRUE;
    }
    else
    {
        PRINTF("send failed!\n"); // Cannot apply for memory, it cannot be sent
    }
    return FALSE;
}

/*********************************************************************
 * @fn      lwns_adapter_ProcessEvent
 *
 * @brief   lwns adapter Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
static uint16_t lwns_adapter_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & LWNS_PHY_RX_OPEN_EVT)
    { //
        RF_Shut();
        RF_Rx(NULL, 0, USER_RF_RX_TX_TYPE, USER_RF_RX_TX_TYPE); // Reopen Receive
        return (events ^ LWNS_PHY_RX_OPEN_EVT);
    }
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;
        if((pMsg = tmos_msg_receive(task_id)) != NULL)
        {
            // Release the TMOS message,tmos_msg_allocate
            lwns_input(pMsg + 1, pMsg[0]); //
            tmos_msg_deallocate(pMsg);     // Free the memory first and release it before data processing to prevent data from being sent during data processing and insufficient memory.
            lwns_dataHandler();            // Calling protocol stack to process data functions
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    // Discard unknown events
    return 0;
}

/*********************************************************************
 * @fn      lwns_phyoutput_ProcessEvent
 *
 * @brief   lwns phyoutput Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
static uint16_t lwns_phyoutput_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & LWNS_HTIMER_PERIOD_EVT)
    {
        lwns_htimer_update();                                                                                      // htimer update
        return (events ^ LWNS_HTIMER_PERIOD_EVT);
    }
    if(events & LWNS_PHY_OUTPUT_FINISH_EVT)
    { // Send to complete the task
        RF_Shut();
        RF_Rx(NULL, 0, USER_RF_RX_TX_TYPE, USER_RF_RX_TX_TYPE); //
        ble_phy_manage_state = BLE_PHY_MANAGE_STATE_FREE;       // Send complete, release phy
        return (events ^ LWNS_PHY_OUTPUT_FINISH_EVT);
    }
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;
        if(ble_phy_manage_state == BLE_PHY_MANAGE_STATE_SENDING)
        {
            PRINTF("phy busy\n");
            return 0; // During the sending process, no new data packets are sent, and they are returned directly
        }
        if((pMsg = tmos_msg_receive(lwns_phyoutput_taskid)) != NULL)
        {
            // Release the TMOS message,tmos_msg_allocate
            tmos_clear_event(lwns_adapter_taskid, LWNS_PHY_RX_OPEN_EVT); //
            RF_Shut();
            if(!RF_Tx((uint8_t *)(pMsg + 1), pMsg[0], USER_RF_RX_TX_TYPE, USER_RF_RX_TX_TYPE))
            {
                RF_Wait_Tx_End();
            }
            tmos_start_task(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_FINISH_EVT, MS1_TO_SYSTEM_TIME(LWNS_PHY_OUTPUT_TIMEOUT_MS)); // Start sending timeout count to prevent accidental interruption of sending, resulting in the inability to continue sending
            tmos_msg_deallocate(pMsg);                                                                                          // Free memory
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
}

/* ***************************************************************************
* @fn lwns_shut
*
* @brief Stop lwns, you cannot call it in the processEvent of lwns_phyoutput_taskid and lwns_adapter_taskid.
*
* @param None.
*
* @return None. */
void lwns_shut()
{
    uint8_t *pMsg;
    RF_Shut(); //RF
    /* Clear all timeout resend */
    lwns_htimer_flush_all();
    tmos_stop_task(lwns_phyoutput_taskid, LWNS_HTIMER_PERIOD_EVT);
    tmos_clear_event(lwns_phyoutput_taskid, LWNS_HTIMER_PERIOD_EVT);

    tmos_stop_task(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_FINISH_EVT); // Stop Htimer heartbeat clock and send list detection
    tmos_clear_event(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_FINISH_EVT);

    while((pMsg = tmos_msg_receive(lwns_phyoutput_taskid)) != NULL)
    {
        /* Clear all cached messages */
        tmos_msg_deallocate(pMsg);
    }
    tmos_clear_event(lwns_phyoutput_taskid, SYS_EVENT_MSG);

    while((pMsg = tmos_msg_receive(lwns_adapter_taskid)) != NULL)
    {
        /*  */
        tmos_msg_deallocate(pMsg);
    }
    tmos_stop_task(lwns_adapter_taskid, LWNS_PHY_RX_OPEN_EVT);
    tmos_clear_event(lwns_adapter_taskid, LWNS_PHY_RX_OPEN_EVT);
    tmos_clear_event(lwns_adapter_taskid, SYS_EVENT_MSG);
}

/* ***************************************************************************
* @fn lwns_start
*
* @brief lwns starts running, after using lwns_shut, use it again when you start.
*
* @param None.
*
* @return None. */
void lwns_start()
{
    RF_Shut();
    RF_Rx(NULL, 0, USER_RF_RX_TX_TYPE, USER_RF_RX_TX_TYPE); // Turn on RF reception, if low power management is required, turn it on elsewhere.
    tmos_start_reload_task(lwns_phyoutput_taskid, LWNS_HTIMER_PERIOD_EVT, MS1_TO_SYSTEM_TIME(LWNS_HTIMER_PERIOD_MS));
}

#endif /* LWNS_USE_NO_MAC */
