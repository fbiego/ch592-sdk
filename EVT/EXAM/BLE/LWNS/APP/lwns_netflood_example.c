/* ********************************* (C) COPYRIGHT ***************************
* File Name: lwns_netflood_example.c
* Author: WCH
* Version: V1.0
* Date: 2021/07/12
* Description: netflood, network flood transmission example
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */
#include "lwns_netflood_example.h"

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

static uint8_t  TX_DATA[LWNS_DATA_SIZE] = {0}; // Maximum length data transmission and reception test
static uint8_t  RX_DATA[LWNS_DATA_SIZE] = {0}; // Maximum length data transmission and reception test
static uint16_t lwns_netflood_ProcessEvent(uint8_t task_id, uint16_t events);
static int      netflood_recv(lwns_controller_ptr ptr,
                              const lwns_addr_t  *from,
                              const lwns_addr_t *originator, uint8_t hops);
static void     netflood_sent(lwns_controller_ptr ptr);
static void     netflood_dropped(lwns_controller_ptr ptr);

/* *
* lwns network flood callback function structure, register callback function */
static const struct lwns_netflood_callbacks callbacks = {netflood_recv,
                                                         netflood_sent, netflood_dropped};

static uint8_t netflood_taskID;

void lwns_netflood_process_init(void);

static lwns_netflood_controller netflood; // Network flood control structure

/* ***************************************************************************************
* @fn netflood_recv
*
* @brief lwns netflood receives callback function
*
* @param ptr - The netflood control structure pointer to which the data received this time belongs.
* @param from - The address pointer of the previous jump forward of the data received this time.
* @param originator - The sender address pointer of the data received this time.
* @param hops - The number of hops experienced by the data received this time from the sender to this node.
*
* @return 0/1 - 0 means that this node no longer forwards the data packet, and 1 means that this node continues to forward the data packet. */
static int netflood_recv(lwns_controller_ptr ptr,
                         const lwns_addr_t  *from,
                         const lwns_addr_t *originator, uint8_t hops)
{
    uint8_t len;
    len = lwns_buffer_datalen(); // Get the data length received in the current buffer
    PRINTF("netflood %d rec %02x %02x %02x %02x %02x %02x,hops=%d\r\n", get_lwns_object_port(ptr),
           from->v8[0], from->v8[1], from->v8[2], from->v8[3], from->v8[4],
           from->v8[5], hops); // Print forwarder, that is, who forwarded the received forwarded data.
    PRINTF("netflood orec %02x %02x %02x %02x %02x %02x,hops=%d\r\n",
           originator->v8[0], originator->v8[1], originator->v8[2],
           originator->v8[3], originator->v8[4], originator->v8[5],
           hops);                   // Print out the initiator of the information, which is the node address that initiated the network flooding.
    lwns_buffer_save_data(RX_DATA); // Receive data to user data area
    PRINTF("data:");
    for(uint8_t i = 0; i < len; i++)
    {
        PRINTF("%02x ", RX_DATA[i]);
    }
    PRINTF("\n");
    return 1; // Return 1, then this node will continue to send netflood and forward data
    // return 0;//Return 0, then this node will no longer continue netflood and will terminate directly
}

/* ***************************************************************************
* @fn netflood_sent
*
* @brief lwns netflood sends completion callback function
*
* @param ptr - The network flood control structure pointer completed by this sending.
*
* @return None. */
static void netflood_sent(lwns_controller_ptr ptr)
{
    PRINTF("netflood %d sent\n", get_lwns_object_port(ptr));
}

/* ***************************************************************************
* @fn netflood_dropped
*
* @brief lwns netflood packet drop callback function
*
* @param ptr - The network flood control structure pointer completed by this sending.
*
* @return None. */
static void netflood_dropped(lwns_controller_ptr ptr)
{
    PRINTF("netflood %d dropped\n", get_lwns_object_port(ptr));
}

/* ***************************************************************************
* @fn lwns_netflood_process_init
*
* @brief lwns netflood routine initialization.
*
* @param None.
*
* @return None. */
void lwns_netflood_process_init(void)
{
    netflood_taskID = TMOS_ProcessEventRegister(lwns_netflood_ProcessEvent);
    for(uint8_t i = 0; i < LWNS_DATA_SIZE; i++)
    {
        TX_DATA[i] = i;
    }
    lwns_netflood_init(&netflood,
                       137,                   // Open a flood structure with port number 137
                       HTIMER_SECOND_NUM * 1, // Waiting for forwarding time
                       1,                     // During the waiting period, the same packet is cancelled after receiving several times.
                       3,                     // Maximum forwarding level
                       FALSE,                 // While waiting for forwarding, a new data packet needs to be forwarded is received. Should the old data packet be sent out immediately or discarded? FALSE is sent immediately and TRUE is discarded.
                       50,                    // Network recovery parameter, this value defines a gap. If the packet sequence number is smaller than the packet sequence number stored in memory is greater than this value, the network failure will be considered to be restored and the packet will continue to be received.
                       // At the same time, this value also determines the difference value of the new data packet, that is, the sequence number of the new data packet from the same node cannot be much larger than that in memory, that is, it is larger than this value.
                       // For example, the stored data packet number in memory is 10, the new packet number is 60, and the difference is 50, which is greater than or equal to the 50 set at this time, so it will not be considered as a new packet and will be discarded.
                       // Only if the sequence number is 59 and the difference is 49, which is less than this value, will it be received.
                       &callbacks); // Returning 0 means opening failed.Return to 1 Open successfully.
#if 1
    tmos_start_task(netflood_taskID, NETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
#endif
}

/*********************************************************************
 * @fn      lwns_netflood_ProcessEvent
 *
 * @brief   lwns netflood Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
static uint16_t lwns_netflood_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & NETFLOOD_EXAMPLE_TX_PERIOD_EVT)
    {
        PRINTF("send\n");
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); // Load the data to be sent to the buffer
        lwns_netflood_send(&netflood);                   // Send network flood packets
        tmos_start_task(netflood_taskID, NETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(3000)); // Send once in 10 seconds
        return (events ^ NETFLOOD_EXAMPLE_TX_PERIOD_EVT);
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
