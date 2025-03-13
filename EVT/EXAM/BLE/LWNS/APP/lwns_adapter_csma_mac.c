/* ********************************* (C) COPYRIGHT *******************************
* File Name          : lwns_adapter_csma_mac.c
* Author             : WCH
* Version            : V1.0
* Date               : 2021/06/20
* Description        : lwns适配器，模拟csma的mac协议
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#include "lwns_adapter_csma_mac.h"
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

#if LWNS_USE_CSMA_MAC // Whether to enable the mac protocol that mimics CSS, please note that only one mac layer protocol can be enabled.

//for lwns_packet_buffer save
__attribute__((aligned(4))) static lwns_qbuf_list_t qbuf_memp[QBUF_MANUAL_NUM];

    //for lwns_route_entry manage
  #if ROUTE_ENTRY_MANUAL_NUM
__attribute__((aligned(4))) static lwns_route_entry_data_t route_entry_memp[ROUTE_ENTRY_MANUAL_NUM];
  #endif

//for neighbor manage
__attribute__((aligned(4))) static lwns_neighbor_list_t neighbor_memp[LWNS_NEIGHBOR_MAX_NUM];

static void ble_new_neighbor_callback(lwns_addr_t *n);     // Discover new neighbor callback function
static BOOL ble_phy_output(uint8_t *dataptr, uint8_t len); // Send interface function
static void RF_2G4StatusCallBack(uint8_t sta, uint8_t crc, uint8_t *rxBuf);

static uint8_t  lwns_adapter_taskid;
static uint16_t lwns_adapter_ProcessEvent(uint8_t task_id, uint16_t events);
static uint8_t  lwns_phyoutput_taskid;
static uint16_t lwns_phyoutput_ProcessEvent(uint8_t task_id, uint16_t events);

/* *
* A must-use function interface for lwns, passing pointers to the lwns library for use internally */
static lwns_fuc_interface_t ble_lwns_fuc_interface = {
    .lwns_phy_output = ble_phy_output,
    .lwns_rand = tmos_rand,
    .lwns_memcpy = tmos_memcpy,
    .lwns_memcmp = tmos_memcmp,
    .lwns_memset = tmos_memset,
    .new_neighbor_callback = ble_new_neighbor_callback,
};

static uint8_t                            ble_phy_manage_state, ble_phy_send_cnt = 0, ble_phy_wait_cnt = 0; // ble phy status management, send count, wait count
static struct csma_mac_phy_manage_struct *csma_phy_manage_list_head = NULL;                                 // Mac management send list pointer
static struct csma_mac_phy_manage_struct  csma_phy_manage_list[LWNS_MAC_SEND_PACKET_MAX_NUM];               // mac management send list management array

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
                { // After alignment, the data area has a minimum of 16 bytes, plus the real data length by one byte
                    // The length verification passes, so rxBuf[1] - 1 must be a multiple of 16
                    pMsg = tmos_msg_allocate(rxBuf[1]); // Apply for memory space, the real data length does not need to be decrypted
                    if(pMsg != NULL)
                    {
                        lwns_msg_decrypt(rxBuf + 3, pMsg + 1, rxBuf[1] - 1); // Decrypt the data
                        if((rxBuf[2] ^ pMsg[rxBuf[2]]) == pMsg[rxBuf[2] + 1])
                        {
                            pMsg[0] = rxBuf[2];      // Verify and store the real data length
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
                { // The data length will be sent to the protocol stack for processing only if it matches the data length.
                    pMsg = tmos_msg_allocate(rxBuf[1] + 1);
                    if(pMsg != NULL)
                    {
                        PRINTF("send rx msg\n"); // Send the received data to the receiving task
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
                    PRINTF("bad len\n"); // Bag length is wrong
                }
  #endif
                // When a data packet is received, the sending task that has not started in this time period is stopped and the sending task that has been started is not paused. It imitates csma/ca and performs anti-collision-related detection.
                if(ble_phy_manage_state == BLE_PHY_MANAGE_STATE_WAIT_SEND)
                { // Wait for the packet to be received in the sending state
                    PRINTF("send delay\n");
                    ble_phy_manage_state = BLE_PHY_MANAGE_STATE_RECEIVED;         // Wait for the sending cycle to receive the packet status
                    tmos_stop_task(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_EVT);   // The competition sending permission fails and you give up your sending task.
                    tmos_clear_event(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_EVT); // Give up your sending task
                }
            }
            tmos_set_event(lwns_adapter_taskid, LWNS_PHY_RX_OPEN_EVT); // Reopen Receive
            break;
        }
        case TX_MODE_TX_FINISH:
        case TX_MODE_TX_FAIL:
            tx_end_flag = TRUE;
            tmos_stop_task(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_FINISH_EVT); // Stop timeout counting
            tmos_set_event(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_FINISH_EVT); // Enter the sending process
            break;
        default:
            break;
    }
}

/* ***************************************************************************
* @fn RF_Init
*
* @brief RF Initialization.
*
* @param None.
*
* @return None. */
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
    cfg.lwns_lib_name = (uint8_t *)VER_LWNS_FILE; // Verify the library name to prevent version errors
    cfg.qbuf_num = QBUF_MANUAL_NUM;               // It must be allocated at least 1 memory unit, which is defined according to the number of ports used in your program corresponding to the qbuf unit used by the module.
    cfg.qbuf_ptr = qbuf_memp;                     // Mesh uses up to 3 qbuf units, (uni/multi)netflood uses up to 2, and other modules use 1.
    cfg.routetable_num = ROUTE_ENTRY_MANUAL_NUM;  // If you need to use mesh, you must allocate the routing table memory space.Otherwise, the mesh initialization will not be successful.
  #if ROUTE_ENTRY_MANUAL_NUM
    cfg.routetable_ptr = route_entry_memp;
  #else
    cfg.routetable_ptr = NULL;
  #endif
    cfg.neighbor_num = LWNS_NEIGHBOR_MAX_NUM;                      // Neighbor table number, must be allocated
    cfg.neighbor_list_ptr = neighbor_memp;                         // Neighbor table memory space
    cfg.neighbor_mod = LWNS_NEIGHBOR_AUTO_ADD_STATE_RECALL_ADDALL; // The default management mode for the neighbor table is to receive all packets, add all neighbors and filter duplicate packets.
  #if LWNS_ADDR_USE_BLE_MAC
    GetMACAddress(cfg.addr.v8); // Bluetooth hardware mac address
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
    tmos_start_reload_task(lwns_phyoutput_taskid, LWNS_PHY_PERIOD_EVT, MS1_TO_SYSTEM_TIME(LWNS_MAC_PERIOD_MS));
    tmos_memset(csma_phy_manage_list, 0, sizeof(csma_phy_manage_list)); // Clear the send management structure
    ble_phy_manage_state = BLE_PHY_MANAGE_STATE_FREE;                   // Clear the phy status
    RF_Shut();
    RF_Rx(NULL, 0, USER_RF_RX_TX_TYPE, USER_RF_RX_TX_TYPE); // Turn on RF reception, if low power management is required, turn it on elsewhere.
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

/* ***************************************************************************
* @fn ble_phy_output
*
* @brief lwns send function interface
*
* @param dataptr - The data buffer head pointer to be sent.
* @param len - The buffer length of data to be sent.
*
* @return TRUE if success, FLASE is failed. */
static BOOL ble_phy_output(uint8_t *dataptr, uint8_t len)
{
    uint8_t                           *pMsg, i;
    struct csma_mac_phy_manage_struct *p;
    for(i = 0; i < LWNS_MAC_SEND_PACKET_MAX_NUM; i++)
    {
        if(csma_phy_manage_list[i].data == NULL)
        {
            break; // An empty structure was found to be used.
        }
        else
        {
            if(i == (LWNS_MAC_SEND_PACKET_MAX_NUM - 1))
            {
                PRINTF("send failed!\n"); // The list is full, the sending fails, and you will return directly.
                return FALSE;
            }
        }
    }
  #if LWNS_ENCRYPT_ENABLE
    pMsg = tmos_msg_allocate((((len + 1 + 15) & 0xf0) + 1 + 1)); // Add 1 bit of the check bit and then align it 16 bytes, store the send length +1, and the real data length +1
  #else
    pMsg = tmos_msg_allocate(len + 1); // Apply for memory space to store messages, store sending length +1
  #endif
    if(pMsg != NULL)
    { // Successful application
        p = csma_phy_manage_list_head;
        if(p != NULL)
        {
            while(p->next != NULL)
            { // Find the end point for sending a linked list
                p = p->next;
            }
        }
  #if LWNS_ENCRYPT_ENABLE
        // There are two bytes reserved inside the lwns buffer, and users can directly use dataptr[len] to assign two bytes of content.
        dataptr[len] = dataptr[len - 1] ^ len;                      // The check byte only takes the last byte and length for XOR operation. The first byte is the same port, which may have an impact.It is time-consuming to check, so it is not used
        pMsg[1] = len;                                              // The real data length occupies one byte and is not encrypted. It is used to receive and perform the first verification step.
        pMsg[0] = lwns_msg_encrypt(dataptr, pMsg + 2, len + 1) + 1; // Get the encrypted length of data, that is, the number of bytes to be sent out, and the real data length is not encrypted.
  #else
        pMsg[0] = len;
        tmos_memcpy(pMsg + 1, dataptr, len);
  #endif
        if(csma_phy_manage_list_head != NULL)
        {
            p->next = &csma_phy_manage_list[i]; // Add endings on the linked list
        }
        else
        {
            csma_phy_manage_list_head = &csma_phy_manage_list[i]; // The linked list is empty, and the node is used as the header node.
        }
        csma_phy_manage_list[i].data = pMsg; // Binding message
        csma_phy_manage_list[i].next = NULL;
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
    {
        RF_Shut();
        RF_Rx(NULL, 0, USER_RF_RX_TX_TYPE, USER_RF_RX_TX_TYPE); // Reopen Receive
        return (events ^ LWNS_PHY_RX_OPEN_EVT);
    }
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;
        if((pMsg = tmos_msg_receive(lwns_adapter_taskid)) != NULL)
        {
            // Release the TMOS message,tmos_msg_allocate
            lwns_input(pMsg + 1, pMsg[0]); // Save data into protocol stack buffer
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
    if(events & LWNS_PHY_PERIOD_EVT)
    {
        lwns_htimer_update(); // The update of htimer needs to be consistent with mac's phy management.
        if((csma_phy_manage_list_head != NULL))
        { // There is a packet that needs to be sent
            if((ble_phy_manage_state == BLE_PHY_MANAGE_STATE_FREE) || (ble_phy_manage_state == BLE_PHY_MANAGE_STATE_RECEIVED))
            { // Not currently in the process of sending
                if(ble_phy_manage_state == BLE_PHY_MANAGE_STATE_RECEIVED)
                {                       // The current cycle sends collision, delay sending
                    ble_phy_wait_cnt++; // Record the number of send delays
                }
                else
                {                         //BLE_PHY_MANAGE_STATE_FREE
                    ble_phy_send_cnt = 0; // Clear count
                    ble_phy_wait_cnt = 0; // Clear count
                }
                ble_phy_manage_state = BLE_PHY_MANAGE_STATE_WAIT_SEND; // Set to wait for sending status
                if(ble_phy_wait_cnt >= LWNS_MAC_SEND_DELAY_MAX_TIMES)
                { // The number of times the sending has been cancelled, too much delay, no longer waiting randomly, start sending immediately
                    tmos_set_event(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_EVT);
                    PRINTF("too many delay\n");
                }
                else
                {
                    uint8_t rand_delay;
                    rand_delay = tmos_rand() % LWNS_MAC_SEND_DELAY_MAX_625US + BLE_PHY_ONE_PACKET_MAX_625US; // Random delay to prevent conflicts. Random delay packets will be sent next time when they are received during the random delay period.
                    tmos_start_task(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_EVT, rand_delay);
                    PRINTF("rand send:%d\n", rand_delay);
                }
            }
        }
        return (events ^ LWNS_PHY_PERIOD_EVT);
    }
    if(events & LWNS_PHY_OUTPUT_EVT)
    { // Send a task, competition sends successfully
        if(ble_phy_manage_state == BLE_PHY_MANAGE_STATE_WAIT_SEND)
        {
            ble_phy_manage_state = BLE_PHY_MANAGE_STATE_SENDING;         // Change to the sending state, the competition packet has been sent, and you need to wait for the receiver to prepare
            tmos_clear_event(lwns_adapter_taskid, LWNS_PHY_RX_OPEN_EVT); // Stops a received task that may have been set and may have been opened
        }
        RF_Shut();
        if(!RF_Tx((uint8_t *)(csma_phy_manage_list_head->data + 1),
              csma_phy_manage_list_head->data[0], USER_RF_RX_TX_TYPE,
              USER_RF_RX_TX_TYPE))
        {
            RF_Wait_Tx_End();
        }
        tmos_start_task(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_FINISH_EVT, MS1_TO_SYSTEM_TIME(LWNS_PHY_OUTPUT_TIMEOUT_MS)); // Start sending timeout count
        return (events ^ LWNS_PHY_OUTPUT_EVT);
    }
    if(events & LWNS_PHY_OUTPUT_FINISH_EVT)
    {                       // Send to complete the task
        ble_phy_send_cnt++; // Send count
        if(ble_phy_send_cnt < LWNS_MAC_TRANSMIT_TIMES)
        {                                                               // Send not ended
            tmos_set_event(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_EVT); // The number of sending times has not ended, continue sending
        }
        else
        {                                                     // The sending process ends
            ble_phy_manage_state = BLE_PHY_MANAGE_STATE_FREE; // Clear status
            RF_Shut();
            RF_Rx(NULL, 0, USER_RF_RX_TX_TYPE, USER_RF_RX_TX_TYPE);      // Reopen Receive
            tmos_msg_deallocate(csma_phy_manage_list_head->data);        // Free memory
            csma_phy_manage_list_head->data = NULL;                      // Restore default parameters
            csma_phy_manage_list_head = csma_phy_manage_list_head->next; // List pop, remove the first element
        }
        return (events ^ LWNS_PHY_OUTPUT_FINISH_EVT);
    }
    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;
        if((pMsg = tmos_msg_receive(lwns_phyoutput_taskid)) != NULL)
        {
            // Release the TMOS message,tmos_msg_allocate
            tmos_msg_deallocate(pMsg); // Free memory
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
    RF_Shut(); // Turn off RF reception
    while(csma_phy_manage_list_head != NULL)
    {
        /* Clear all messages to be sent in the cache */
        tmos_msg_deallocate(csma_phy_manage_list_head->data);
        csma_phy_manage_list_head->data = NULL;
        csma_phy_manage_list_head = csma_phy_manage_list_head->next;
    }
    tmos_stop_task(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_EVT);
    tmos_clear_event(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_EVT);

    /* Clear all timeout resend */
    lwns_htimer_flush_all();
    tmos_stop_task(lwns_phyoutput_taskid, LWNS_PHY_PERIOD_EVT); // Stop Htimer heartbeat clock and send list detection
    tmos_clear_event(lwns_phyoutput_taskid, LWNS_PHY_PERIOD_EVT);

    tmos_stop_task(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_FINISH_EVT);
    tmos_clear_event(lwns_phyoutput_taskid, LWNS_PHY_OUTPUT_FINISH_EVT);

    while((pMsg = tmos_msg_receive(lwns_adapter_taskid)) != NULL)
    {
        /* Clear all cached messages */
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
    tmos_start_reload_task(lwns_phyoutput_taskid, LWNS_PHY_PERIOD_EVT, MS1_TO_SYSTEM_TIME(LWNS_MAC_PERIOD_MS));
}

#endif /* LWNS_USE_CSMA_MAC */
