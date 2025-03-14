/* ********************************* (C) COPYRIGHT ***************************
* File Name: lwns_ruc_example.c
* Author: WCH
* Version: V1.0
* Date: 2021/06/30
* Description: reliable unicast, reliable unicast transmission example
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */
#include "lwns_ruc_example.h"

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

#if 1
static lwns_addr_t dst_addr = {{0xa3, 0xdf, 0x38, 0xe4, 0xc2, 0x84}}; // When testing, please modify the target node address according to the different MAC address of the circuit board chip.Modify it to the recipient's MAC address, please do not use your own MAC address
#else
static lwns_addr_t dst_addr = {{0xd9, 0x37, 0x3c, 0xe4, 0xc2, 0x84}};
#endif

static lwns_ruc_controller ruc; // Declare reliable unicast control structure

static uint8_t TX_DATA[10] =
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
static uint8_t RX_DATA[10];

static uint8_t ruc_taskID;
uint16_t       lwns_ruc_ProcessEvent(uint8_t task_id, uint16_t events);

static void recv_ruc(lwns_controller_ptr ptr,
                     const lwns_addr_t  *sender);

static void sent_ruc(lwns_controller_ptr ptr,
                     const lwns_addr_t *to, uint8_t retransmissions);
static void timedout_ruc(lwns_controller_ptr ptr,
                         const lwns_addr_t  *to);

/* ***************************************************************************
* @fn recv_ruc
*
* @brief lwns ruc receive callback function
*
* @param ptr - The reliable unicast control structure pointer to which the data received this time belongs.
* @param sender - The sender address pointer of the data received this time.
*
* @return None. */
static void recv_ruc(lwns_controller_ptr ptr,
                     const lwns_addr_t  *sender)
{
    // The callback will be called after receiving the data sent to you in the ruc
    uint8_t len;
    len = lwns_buffer_datalen(); // Get the data length received in the current buffer
    if(len == 10)
    {
        lwns_buffer_save_data(RX_DATA); // Receive data to user data area
        PRINTF("ruc %d rec %02x %02x %02x %02x %02x %02x\r\n", get_lwns_object_port(ptr), sender->v8[0],
               sender->v8[1], sender->v8[2], sender->v8[3], sender->v8[4], sender->v8[5]);
        PRINTF("data:");
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
* @fn sent_ruc
*
* @brief lwns ruc sends the completed callback function
*
* @param ptr - The reliable unicast control structure pointer completed by this sending.
*
* @return None. */
static void sent_ruc(lwns_controller_ptr ptr,
                     const lwns_addr_t *to, uint8_t retransmissions)
{
    // The callback will be called only after the sending in the ruc is successfully received and the ack reply from the target node is received.
    PRINTF("ruc %d sent %d\r\n", get_lwns_object_port(ptr), retransmissions);
    tmos_start_task(ruc_taskID, RUC_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000)); // Update the task time, send and receive a reply, and send it in 1 second
}

/* ***************************************************************************
* @fn timedout_ruc
*
* @brief lwns ruc send timeout callback function
*
* @param ptr - The ruc control structure pointer completed by this sending.
*
* @return None. */
static void timedout_ruc(lwns_controller_ptr ptr,
                         const lwns_addr_t  *to)
{
    // In ruc, the callback will be called after the number of resents exceeds the maximum number of resents.
    PRINTF("ruc %d timedout\n", get_lwns_object_port(ptr));
    tmos_start_task(ruc_taskID, RUC_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
}

/* *
* lwns Reliable unicast callback function structure, register callback function */
static const struct lwns_ruc_callbacks ruc_callbacks = {
    recv_ruc, sent_ruc, timedout_ruc}; // Declare a reliable unicast callback structure

/* ***************************************************************************
* @fn lwns_ruc_process_init
*
* @brief lwns ruc routine initialization.
*
* @param None.
*
* @return None. */
void lwns_ruc_process_init(void)
{
    lwns_ruc_init(&ruc,
                  144,               // Open a reliable unicast with port number 144
                  HTIMER_SECOND_NUM, // Wait for the ack time interval, and if you don't receive it, you will send it again
                  &ruc_callbacks);   // Returning 0 means opening failed.Return to 1 Open successfully.
    ruc_taskID = TMOS_ProcessEventRegister(lwns_ruc_ProcessEvent);
    tmos_start_task(ruc_taskID, RUC_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
}

/*********************************************************************
 * @fn      lwns_ruc_ProcessEvent
 *
 * @brief   lwns ruc Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_ruc_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & RUC_EXAMPLE_TX_PERIOD_EVT)
    {
        uint8_t temp;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; // Shift the data to observe the effect
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); // Load the data to be sent to the buffer
        lwns_ruc_send(&ruc,
                      &dst_addr, // Reliable unicast target address
                      4          // Maximum number of resents
        );                       // Reliable unicast sending function: send parameters, target address, maximum number of resents, default resent once in one second
        return events ^ RUC_EXAMPLE_TX_PERIOD_EVT;
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
