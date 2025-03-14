/* ********************************* (C) COPYRIGHT ***************************
* File Name: lwns_rucft_example.c
* Author: WCH
* Version: V1.0
* Date: 2021/06/30
* Description: reliable unicast file transfer, reliable unicast file transfer example
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */
#include "lwns_rucft_example.h"

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
static lwns_addr_t dst_addr = {{0x66, 0xdf, 0x38, 0xe4, 0xc2, 0x84}}; // When testing, please modify the target node address according to the different MAC address of the circuit board chip.Modify it to the recipient's MAC address, please do not use your own MAC address
#else
static lwns_addr_t dst_addr = {{0xd9, 0x37, 0x3c, 0xe4, 0xc2, 0x84}};
#endif

static uint8_t rucft_taskID;

static lwns_rucft_controller rucft; // Declare the rucft control structure

#define FILESIZE    4000
static char  strsend[FILESIZE]; // Send Buffer
static char *strp;
static void  write_file(lwns_controller_ptr ptr, const lwns_addr_t *sender,
                        int offset, int flag, char *data, int datalen);
static int   read_file(lwns_controller_ptr ptr, int offset, char *to);
static void  timedout_rucft(lwns_controller_ptr ptr);

/* *
* lwns Reliable unicast file transfer callback function structure, register callback function */
const static struct lwns_rucft_callbacks rucft_callbacks = {write_file,
                                                            read_file, timedout_rucft};

uint16_t lwns_rucft_ProcessEvent(uint8_t task_id, uint16_t events);

/* ***************************************************************************
* @fn write_file
*
* @brief lwns rucft receives callback function as a write file callback function
*
* @param ptr - The netflood control structure pointer to which the data received this time belongs.
* @param sender - The sender address pointer of the data received this time.
* @param offset - The offset of the data received this time is also the amount of data received by this file transmission.
* @param flag - The flag of the data received this time, LWNS_RUCFT_FLAG_NONE/LWNS_RUCFT_FLAG_NEWFILE/LWNS_RUCFT_FLAG_END.
* @param data - The header pointer of the data received this time.
* @param datalen - The length of the data received this time.
*
* @return None. */
static void write_file(lwns_controller_ptr ptr, const lwns_addr_t *sender,
                       int offset, int flag, char *data, int datalen)
{
    // sender is the address of the sender
    // If you need to receive different files, you need to make an interface in this function
    if(datalen > 0)
    { // Declare a buffer to get data from data and print it
        PRINTF("r:%c\n", *data);
    }
    if(flag == LWNS_RUCFT_FLAG_END)
    {
        PRINTF("re\n");
        // The last package of this file transfer
    }
    else if(flag == LWNS_RUCFT_FLAG_NONE)
    {
        PRINTF("ru\n");
        // This file transfer is normal
    }
    else if(flag == LWNS_RUCFT_FLAG_NEWFILE)
    {
        PRINTF("rn\n");
        // The first package of this file transfer
    }
}

/* ***************************************************************************
* @fn read_file
*
* @brief lwns rucft sends a complete callback function as a read file callback function
*
* @param ptr - The netflood control structure pointer to which the data received this time belongs.
* @param offset - The offset of the data received this time is also the amount of data that has been sent for this file transmission.
* @param to - The header pointer of the data buffer that needs to be sent this time, and the user will copy the data to the memory space pointed to by this pointer..
*
* @return size - The returned size is the length of the data to be sent this time, and cannot be greater than LWNS_RUCFT_DATASIZE. */
static int read_file(lwns_controller_ptr ptr, int offset, char *to)
{
    // to be a pointer that needs to save the data past
    // If you need to send different files, you need to make an interface in this function
    int size = LWNS_RUCFT_DATASIZE;
    if(offset >= FILESIZE)
    {
        // It's been posted last time, this is the last confirmation
        PRINTF("Send done\n");
        tmos_start_task(rucft_taskID, RUCFT_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(5000)); // Continue to send the test in 5 seconds
        return 0;
    }
    else if(offset + LWNS_RUCFT_DATASIZE >= FILESIZE)
    {
        size = FILESIZE - offset;
    }
    // Press the content that needs to be sent this time into the packet buffer
    tmos_memcpy(to, strp + offset, size);
    return size;
}

/* ***************************************************************************
* @fn timedout_rucft
*
* @brief lwns rucft send timeout callback function
*
* @param ptr - The ruc control structure pointer completed by this sending.
*
* @return None. */
static void timedout_rucft(lwns_controller_ptr ptr)
{
    // In rucft, the callback will be called after the sender resends exceeds the maximum resends.
    // The receiver timed out and did not receive the next packet will also be called
    PRINTF("rucft %d timedout\r\n", get_lwns_object_port(ptr));
}

/* ***************************************************************************
* @fn lwns_rucft_process_init
*
* @brief lwns rucft routine initialization.
*
* @param None.
*
* @return None. */
void lwns_rucft_process_init(void)
{
    lwns_addr_t MacAddr;
    rucft_taskID = TMOS_ProcessEventRegister(lwns_rucft_ProcessEvent);
    lwns_rucft_init(&rucft, 137,            // Port number
                    HTIMER_SECOND_NUM / 10, // Waiting for the target node ack time
                    5,                      // The maximum number of resends is the same as the number of resends in ruc.
                    &rucft_callbacks        // Callback function
    );                                      // Returning 0 means opening failed.Return to 1 Open successfully.
    int i;
    for(i = 0; i < FILESIZE; i++)
    { // LWNS_RUCFT_DATASIZE LWNSNK_RUCFT_DATASIZE b, etc. Initialize the data to be sent
        strsend[i] = 'a' + i / LWNS_RUCFT_DATASIZE;
    }
    strp = strsend;
    GetMACAddress(MacAddr.v8);
    if(lwns_addr_cmp(&MacAddr, &dst_addr))
    {
    }
    else
    {
        tmos_start_task(rucft_taskID, RUCFT_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(1000));
    }
}

/*********************************************************************
 * @fn      lwns_rucft_ProcessEvent
 *
 * @brief   lwns rucft Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
uint16_t lwns_rucft_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & RUCFT_EXAMPLE_TX_PERIOD_EVT)
    {
        PRINTF("send\n");
        lwns_rucft_send(&rucft, &dst_addr); // Start sending to the target node. When the user enables sending, configure packet reading in the callback function.
        return events ^ RUCFT_EXAMPLE_TX_PERIOD_EVT;
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
