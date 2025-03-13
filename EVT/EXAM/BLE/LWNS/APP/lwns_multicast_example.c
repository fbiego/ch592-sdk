/* ********************************* (C) COPYRIGHT *******************************
* File Name          : lwns_multicast_example.c
* Author             : WCH
* Version            : V1.0
* Date               : 2021/06/20
* Description        : single-hop multicast£¬×é²¥´«ÊäÀý×Ó
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#include "lwns_multicast_example.h"

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

static uint8_t subaddrs_index = 0;                 // Send subscription address number
#define SUBADDR_NUM    3                           // Number of subscription addresses
static uint16_t subaddrs[SUBADDR_NUM] = {1, 2, 3}; // Subscribe to an array of addresses

static uint8_t TX_DATA[10] =
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
static uint8_t  RX_DATA[10];
static uint16_t lwns_multicast_ProcessEvent(uint8_t task_id, uint16_t events);
static void     multicast_recv(lwns_controller_ptr c, uint16_t subaddr, const lwns_addr_t *sender); // Multicast Receive Callback Function
static void     multicast_sent(lwns_controller_ptr ptr);                                            // Multicast send complete callback function

static lwns_multicast_controller multicast; // Declare multicast control structure

static uint8_t multicast_taskID; // Declare multicast control task id

/* ***************************************************************************
* @fn multicast_recv
*
* @brief lwns multicast receive callback function
*
* @param ptr - The multicast control structure pointer to which the data received this time belongs.
* @param subaddr - Subscription address for the data received this time.
* @param sender - The sender address pointer of the data received this time.
*
* @return None. */
static void multicast_recv(lwns_controller_ptr ptr, uint16_t subaddr, const lwns_addr_t *sender)
{
    uint8_t len;
    len = lwns_buffer_datalen(); // Get the data length received in the current buffer
    if(len == 10)
    {
        lwns_buffer_save_data(RX_DATA); // Receive data to user data area
        PRINTF("multicast %d rec from %02x %02x %02x %02x %02x %02x\n",
               get_lwns_object_port(ptr),
               sender->v8[0], sender->v8[1], sender->v8[2], sender->v8[3],
               sender->v8[4], sender->v8[5]); // from is the sender address of the received data
        PRINTF("subaddr:%d data:", subaddr);
        for(uint8_t i = 0; i < len; i++)
        {
            PRINTF("%02x ", RX_DATA[i]);
        }
        PRINTF("\n");
    }
    else
    {
        PRINTF("data len err\n");
    }
}

/* ***************************************************************************
* @fn multicast_sent
*
* @brief lwns multicast sends the complete callback function
*
* @param ptr - The multicast control structure pointer completed by this sending.
*
* @return None. */
static void multicast_sent(lwns_controller_ptr ptr)
{
    PRINTF("multicast %d sent\n", get_lwns_object_port(ptr));
}

/* *
* lwns multicast callback function structure, register callback function */
static const struct lwns_multicast_callbacks multicast_callbacks =
    {multicast_recv, multicast_sent};

/* ***************************************************************************
* @fn lwns_multicast_process_init
*
* @brief lwns multicast routine initialization.
*
* @param None.
*
* @return None. */
void lwns_multicast_process_init(void)
{
    multicast_taskID = TMOS_ProcessEventRegister(lwns_multicast_ProcessEvent);
    lwns_multicast_init(&multicast,
                        136,                   // Open a multicast with port number 136
                        subaddrs,              // Subscribe to address array pointer
                        SUBADDR_NUM,           // Number of subscription addresses
                        &multicast_callbacks); // Returning 0 means opening failed.Return to 1 Open successfully.
    tmos_start_task(multicast_taskID, MULTICAST_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
}

/*********************************************************************
 * @fn      lwns_multicast_ProcessEvent
 *
 * @brief   lwns multicast Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_multicast_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & MULTICAST_EXAMPLE_TX_PERIOD_EVT)
    { // Periodically send multicast messages on different multicast addresses
        uint8_t temp;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; // Shift the data to observe the effect
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); // Load the data to be sent to the buffer
        if(subaddrs_index >= SUBADDR_NUM)
        {
            subaddrs_index = 0;
        }
        lwns_multicast_send(&multicast, subaddrs[subaddrs_index]); // Multicast sends data to the specified node
        subaddrs_index++;
        tmos_start_task(multicast_taskID, MULTICAST_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000)); // Periodic sending
        return events ^ MULTICAST_EXAMPLE_TX_PERIOD_EVT;
    }
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
    return 0;
}
