/* ********************************* (C) COPYRIGHT *******************************
* File Name          : lwns_unicast_example.c
* Author             : WCH
* Version            : V1.0
* Date               : 2021/06/19
* Description        : lwnsµ¥²¥´«ÊäÀý×Ó
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#include "lwns_unicast_example.h"

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
static lwns_addr_t dst_addr = {{0xab, 0xdf, 0x38, 0xe4, 0xc2, 0x84}}; // When testing, please modify the target node address according to the different MAC address of the circuit board chip.Modify it to the recipient's MAC address, please do not use your own MAC address
#else
static lwns_addr_t dst_addr = {{0xd9, 0x37, 0x3c, 0xe4, 0xc2, 0x84}};
#endif

static uint8_t TX_DATA[10] =
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
static uint8_t  RX_DATA[10];
static uint16_t lwns_unicast_ProcessEvent(uint8_t task_id, uint16_t events);
static void     unicast_recv(lwns_controller_ptr c, const lwns_addr_t *from); // Unicast receiving callback function
static void     unicast_sent(lwns_controller_ptr ptr);                        // Unicast send complete callback function

static lwns_unicast_controller unicast; // Declare unicast control structure

static uint8_t unicast_taskID; // Declare unicast control task id

/* ***************************************************************************
* @fn unicast_recv
*
* @brief lwns unicast receive callback function
*
* @param ptr - The unicast control structure pointer to which the data received this time belongs.
* @param sender - The sender address pointer of the data received this time.
*
* @return None. */
static void unicast_recv(lwns_controller_ptr ptr, const lwns_addr_t *sender)
{
    uint8_t len;
    len = lwns_buffer_datalen(); // Get the data length received in the current buffer
    if(len == 10)
    {
        lwns_buffer_save_data(RX_DATA); // Receive data to user data area
        PRINTF("unicast %d rec from %02x %02x %02x %02x %02x %02x\n",
               get_lwns_object_port(ptr),
               sender->v8[0], sender->v8[1], sender->v8[2], sender->v8[3],
               sender->v8[4], sender->v8[5]); // sender is the sender address of the received data
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
* @fn unicast_sent
*
* @brief lwns unicast send complete callback function
*
* @param ptr - The reliable unicast control structure pointer completed by this sending.
*
* @return None. */
static void unicast_sent(lwns_controller_ptr ptr)
{
    PRINTF("unicast %d sent\n", get_lwns_object_port(ptr));
}

/* *
* lwns unicast callback function structure, register callback function */
static const struct lwns_unicast_callbacks unicast_callbacks =
    {unicast_recv, unicast_sent};

/* ***************************************************************************
* @fn lwns_unicast_process_init
*
* @brief lwns unicast routine initialization.
*
* @param None.
*
* @return None. */
void lwns_unicast_process_init(void)
{
    unicast_taskID = TMOS_ProcessEventRegister(lwns_unicast_ProcessEvent);
    lwns_unicast_init(&unicast,
                      136,                 // Open a unicast with port number 136
                      &unicast_callbacks); // Returning 0 means opening failed.Return to 1 Open successfully.
    tmos_start_task(unicast_taskID, UNICAST_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
}

/*********************************************************************
 * @fn      lwns_unicast_ProcessEvent
 *
 * @brief   lwns unicast Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_unicast_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & UNICAST_EXAMPLE_TX_PERIOD_EVT)
    {
        uint8_t temp;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; // Shift the data to observe the effect
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); // Load the data to be sent to the buffer
        lwns_unicast_send(&unicast, &dst_addr);          // Unicast sends data to the specified node
        tmos_start_task(unicast_taskID, UNICAST_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000)); // Periodic sending
        return events ^ UNICAST_EXAMPLE_TX_PERIOD_EVT;
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
