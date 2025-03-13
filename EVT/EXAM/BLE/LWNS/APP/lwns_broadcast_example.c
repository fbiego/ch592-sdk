/* ********************************* (C) COPYRIGHT *******************************
* File Name          : lwns_broadcast_example.c
* Author             : WCH
* Version            : V1.0
* Date               : 2021/06/20
* Description        : broadcast£¬¹ã²¥³ÌÐòÀý×Ó
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#include "lwns_broadcast_example.h"

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

// The data buffer that the user needs to send
static uint8_t TX_DATA[10] =
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

static uint8_t  RX_DATA[10]; // The buffer for users to receive data
static uint16_t lwns_broadcast_ProcessEvent(uint8_t task_id, uint16_t events);
static void     broadcast_recv(lwns_controller_ptr ptr,
                               const lwns_addr_t  *sender); // Receive callback function declaration
static void     broadcast_sent(lwns_controller_ptr ptr);   // Send callback function declaration

static lwns_broadcast_controller broadcast; // Broadcast control structure

static uint8_t broadcast_taskID;

/* ***************************************************************************
* @fn broadcast_recv
*
* @brief lwns broadcast receive callback function
*
* @param ptr - The pointer of the broadcast control structure to which the data received this time belongs.
* @param sender - The sender address pointer of the data received this time.
*
* @return None. */
static void broadcast_recv(lwns_controller_ptr ptr,
                           const lwns_addr_t  *sender)
{
    uint8_t len;
    len = lwns_buffer_datalen(); // Get the data length received in the current buffer
    if(len == 10)
    {
        lwns_buffer_save_data(RX_DATA); // Receive data to user data area
        PRINTF("broadcast %d rec from %02x %02x %02x %02x %02x %02x\n", get_lwns_object_port(ptr),
               sender->v8[0], sender->v8[1], sender->v8[2], sender->v8[3],
               sender->v8[4], sender->v8[5]); // Print out the sender address of this message
        PRINTF("data:");
        for(uint8_t i = 0; i < len; i++)
        {
            PRINTF("%02x ", RX_DATA[i]); // Print data
        }
        PRINTF("\n");
    }
    else
    {
        PRINTF("data len err\n");
    }
}

/* ***************************************************************************
* @fn broadcast_sent
*
* @brief lwns broadcast send complete callback function
*
* @param ptr - The broadcast control structure pointer completed by this sending.
*
* @return None. */
static void broadcast_sent(lwns_controller_ptr ptr)
{
    PRINTF("broadcast %d sent\n", get_lwns_object_port(ptr)); // Print sending completion information
}

/* *
* lwns broadcast callback function structure, register callback function */
static const struct lwns_broadcast_callbacks broadcast_call = {
    broadcast_recv, broadcast_sent};

/* ***************************************************************************
* @fn lwns_broadcast_process_init
*
* @brief lwns broadcast routine initialization.
*
* @param None.
*
* @return None. */
void lwns_broadcast_process_init(void)
{
    broadcast_taskID = TMOS_ProcessEventRegister(lwns_broadcast_ProcessEvent);
    lwns_broadcast_init(&broadcast, 136, &broadcast_call); // Open a broadcast port with port number 136 and register the callback function
    tmos_start_task(broadcast_taskID, BROADCAST_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000)); // Start periodic broadcast tasks
}

/*********************************************************************
 * @fn      lwns_broadcast_ProcessEvent
 *
 * @brief   lwns broadcast Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_broadcast_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & BROADCAST_EXAMPLE_TX_PERIOD_EVT)
    {
        uint8_t temp;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; // Shift the data to observe the effect
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); // Load the data to be sent to the buffer
        lwns_broadcast_send(&broadcast);                 // Broadcast send data
        tmos_start_task(broadcast_taskID, BROADCAST_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000)); // Periodic sending

        return events ^ BROADCAST_EXAMPLE_TX_PERIOD_EVT;
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
