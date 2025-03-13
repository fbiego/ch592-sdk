/* ********************************* (C) COPYRIGHT *******************************
* File Name          : lwns_multinetflood_example.c
* Author             : WCH
* Version            : V1.0
* Date               : 2021/11/10
* Description        : multinetflood£¬×é²¥ÍøÂç·ººé´«ÊäÀý×Ó
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#include "lwns_multinetflood_example.h"

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

/* * The address of the subscription is of 2 bytes u16 type. */
static uint8_t subaddrs_index = 0;                 // Send subscription address number
#define SUBADDR_NUM    3                           // Number of subscription addresses
static uint16_t subaddrs[SUBADDR_NUM] = {1, 2, 3}; // Subscribe to an array of addresses

static uint8_t  TX_DATA[LWNS_DATA_SIZE] = {0}; // Maximum length data transmission and reception test
static uint8_t  RX_DATA[LWNS_DATA_SIZE] = {0}; // Maximum length data transmission and reception test
static uint16_t lwns_multinetflood_ProcessEvent(uint8_t task_id, uint16_t events);
static void     multinetflood_recv(lwns_controller_ptr ptr, uint16_t subaddr, const lwns_addr_t *sender, uint8_t hops); // Multicast network flooding reception callback function
static void     multinetflood_sent(lwns_controller_ptr ptr);                                                            // Multicast network flooding send complete callback function

static lwns_multinetflood_controller multinetflood; // Declare the multicast network flood control structure

static uint8_t multinetflood_taskID; // Multicast network flood control task id

/* ***************************************************************************
* @fn multinetflood_recv
*
* @brief lwns multinetflood receives callback function
*
* @param ptr - The multinetflood control structure pointer to which the data received this time belongs.
* @param subaddr - Subscription address for the data received this time.
* @param sender - The sender address pointer of the data received this time.
* @param hops - The number of hops experienced by the data received this time from the sender to this node.
*
* @return None. */
static void multinetflood_recv(lwns_controller_ptr ptr, uint16_t subaddr, const lwns_addr_t *sender, uint8_t hops)
{
    uint8_t len;
    len = lwns_buffer_datalen();    // Get the data length received in the current buffer
    lwns_buffer_save_data(RX_DATA); // Receive data to user data area
    PRINTF("multinetflood %d rec from %02x %02x %02x %02x %02x %02x\n",
           get_lwns_object_port(ptr),
           sender->v8[0], sender->v8[1], sender->v8[2], sender->v8[3],
           sender->v8[4], sender->v8[5]); // from is the sender address of the received data
    PRINTF("subaddr:%d,data:", subaddr);
    for(uint8_t i = 0; i < len; i++)
    {
        PRINTF("%02x ", RX_DATA[i]); // Print out data
    }
    PRINTF("\n");
}

/* ***************************************************************************
* @fn multinetflood_sent
*
* @brief lwns multinetflood sends the complete callback function
*
* @param ptr - The multicast network flood control structure pointer completed by this sending.
*
* @return None. */
static void multinetflood_sent(lwns_controller_ptr ptr)
{
    PRINTF("multinetflood %d sent\n", get_lwns_object_port(ptr));
}

/* *
* lwns multicast network flood callback function structure, register callback function */
static const struct lwns_multinetflood_callbacks multinetflood_callbacks =
    {multinetflood_recv, multinetflood_sent};

/* ***************************************************************************
* @fn lwns_multinetflood_process_init
*
* @brief lwns multinetflood routine initialization.
*
* @param None.
*
* @return None. */
void lwns_multinetflood_process_init(void)
{
    multinetflood_taskID = TMOS_ProcessEventRegister(lwns_multinetflood_ProcessEvent);
    for(uint8_t i = 0; i < LWNS_DATA_SIZE; i++)
    {
        TX_DATA[i] = i;
    }
    lwns_multinetflood_init(&multinetflood,
                            137,               // Open a multicast network flood structure with port number 137
                            HTIMER_SECOND_NUM, // Maximum waiting time for forwarding
                            1,                 // During the waiting period, the same packet is cancelled after receiving several times.
                            3,                 // Maximum forwarding level
                            FALSE,             // While waiting for forwarding, a new data packet needs to be forwarded is received. Should the old data packet be sent out immediately or discarded? FALSE is sent immediately and TRUE is discarded.
                            50,                // Network recovery parameter, this value defines a gap. If the packet sequence number is smaller than the packet sequence number stored in memory is greater than this value, the network failure will be considered to be restored and the packet will continue to be received.
                            // At the same time, this value also determines the difference value of the new data packet, that is, the sequence number of the new data packet from the same node cannot be much larger than that in memory, that is, it is larger than this value.
                            // For example, the stored data packet number in memory is 10, the new packet number is 60, and the difference is 50, which is greater than or equal to the 50 set at this time, so it will not be considered as a new packet and will be discarded.
                            // Only if the sequence number is 59 and the difference is 49, which is less than this value, will it be received.
                            TRUE,                      // Whether the target packets that are not native are forwarded, similar to whether Bluetooth mesh enables relay function.
                            subaddrs,                  // Subscribed address array pointer
                            SUBADDR_NUM,               // Number of subscription addresses
                            &multinetflood_callbacks); // Returning 0 means opening failed.Return to 1 Open successfully.
#if 1
    tmos_start_task(multinetflood_taskID, MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
#endif
}

/*********************************************************************
 * @fn      lwns_multinetflood_ProcessEvent
 *
 * @brief   lwns multinetflood Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_multinetflood_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT)
    {
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
        lwns_multinetflood_send(&multinetflood, subaddrs[subaddrs_index]); // Multicast network flooding sends data to subscription address
        subaddrs_index++;

        tmos_start_task(multinetflood_taskID, MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000)); // Periodic sending
        return events ^ MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT;
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
