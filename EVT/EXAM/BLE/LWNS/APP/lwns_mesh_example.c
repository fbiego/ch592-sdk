/* ********************************* (C) COPYRIGHT *******************************
* File Name          : lwns_mesh_example.c
* Author             : WCH
* Version            : V1.0
* Date               : 2021/06/28
* Description        : mesh´«Êä³ÌÐòÀý×Ó
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
****************************************************************************** */
#include "lwns_mesh_example.h"

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

#define MESH_TEST_ROUTE_AUTO       1  // Set whether to automatically manage routing tables
#define MESH_TEST_SELF_ADDR_IDX    2  // The node address in the current test is in the device_addr array
#define MESH_TEST_ADDR_MAX_IDX     2  // How many nodes are there in the current test

static uint8_t mesh_test_send_dst = 0;

static lwns_mesh_controller lwns_mesh_test;

static uint8_t           lwns_mesh_test_taskID;
static uint16_t          lwns_mesh_test_ProcessEvent(uint8_t task_id, uint16_t events);
static uint8_t           TX_DATA[10] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                              0x38, 0x39};
static const lwns_addr_t device_addr[] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, // Root node address
    {0x00, 0x00, 0x00, 0x00, 0x01, 0x01}, // Group 1 Routing Node 1
    {0x00, 0x00, 0x00, 0x00, 0x01, 0x02}, // Group 1 Node 2
    {0x00, 0x00, 0x00, 0x00, 0x01, 0x03}, // Group 1 Node 3
    {0x00, 0x00, 0x00, 0x00, 0x01, 0x04}, // Group 1 Node 4
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x01}, // Group 2 routing node 1
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x02}, // Group 2 Node 2
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x03}, // Group 2 Node 3
    {0x00, 0x00, 0x00, 0x00, 0x02, 0x04}, // Group 2 Node 4
    {0x00, 0x00, 0x00, 0x00, 0x03, 0x01}, // Group 3 routing node 1
    {0x00, 0x00, 0x00, 0x00, 0x03, 0x02}, // Group 3 node 2
    {0x00, 0x00, 0x00, 0x00, 0x03, 0x03}, // Group 3 Node 3
    {0x00, 0x00, 0x00, 0x00, 0x03, 0x04}, // Group 3 node 4
};

static void lwns_mesh_recv(lwns_controller_ptr ptr,
                           const lwns_addr_t *sender, uint8_t hops);
static void lwns_mesh_sent(lwns_controller_ptr ptr);
static void lwns_mesh_timedout(lwns_controller_ptr ptr);

/* ***************************************************************************
* @fn lwns_mesh_recv
*
* @brief lwns mesh receive callback function
*
* @param ptr - The mesh control structure pointer to which the data received this time belongs.
* @param sender - The sender address pointer of the data received this time.
* @param hops - The number of hops experienced by the data received this time from the sender to this node.
*
* @return None. */
static void lwns_mesh_recv(lwns_controller_ptr ptr,
                           const lwns_addr_t *sender, uint8_t hops)
{
    PRINTF("mesh %d received from %02x.%02x.%02x.%02x.%02x.%02x: %s (%d)\n",
           get_lwns_object_port(ptr), sender->v8[0], sender->v8[1],
           sender->v8[2], sender->v8[3], sender->v8[4], sender->v8[5],
           (char *)lwns_buffer_dataptr(), lwns_buffer_datalen());
    if(MESH_TEST_SELF_ADDR_IDX != 0)
    {
        // Not 0, it proves that it is a slave, and reply to the host after receiving it
        lwns_buffer_save_data(TX_DATA);
        tmos_set_event(lwns_mesh_test_taskID, MESH_EXAMPLE_TX_NODE_EVT);
    }
}

/* ***************************************************************************
* @fn lwns_mesh_sent
*
* @brief lwns mesh send complete callback function
*
* @param ptr - The mesh control structure pointer completed by this sending.
*
* @return None. */
static void lwns_mesh_sent(lwns_controller_ptr ptr)
{
    PRINTF("mesh %d packet sent\n", get_lwns_object_port(ptr));
}

/* ***************************************************************************
* @fn lwns_mesh_timedout
*
* @brief lwns mesh send timeout callback function
*
* @param ptr - The mesh control structure pointer completed by this sending.
*
* @return None. */
static void lwns_mesh_timedout(lwns_controller_ptr ptr)
{
    // The callback will be entered only if the route is timed out. If there is already a route, but the next jump node is disconnected, the callback will not be entered.Deactivate time is managed by lwns_route_init(TRUE, 60, HTIMER_SECOND_NUM);
    PRINTF("mesh %d packet timedout\n", get_lwns_object_port(ptr));
}

/* *
* lwns mesh callback function structure, register callback function */
static const struct lwns_mesh_callbacks callbacks = {lwns_mesh_recv,
                                                     lwns_mesh_sent, lwns_mesh_timedout};

/* * Mesh uses the netflood module to find routes, so the initialization parameters include the parameters required for netflood
* mesh is the basic structure for multi-hop forwarding using routing
* The route must be initialized before use, including the memory space provided to the routing table during library initialization. */

/* ***************************************************************************
* @fn lwns_mesh_process_init
*
* @brief lwns mesh routine initialization.
*
* @param None.
*
* @return None. */
void lwns_mesh_process_init(void)
{
    uint8_t route_enable = FALSE;
    lwns_addr_set(
        (lwns_addr_t *)(&device_addr[MESH_TEST_SELF_ADDR_IDX])); // Change lwns internal addr
    if(device_addr[MESH_TEST_SELF_ADDR_IDX].v8[5] == 1)
    {
        // Each group of first nodes opens the routing function, while other nodes do not open the routing function.
        route_enable = TRUE;
    }
    lwns_mesh_test_taskID = TMOS_ProcessEventRegister(lwns_mesh_test_ProcessEvent);
#if MESH_TEST_ROUTE_AUTO
    lwns_route_init(TRUE, 60, HTIMER_SECOND_NUM);
#else
    lwns_route_init(TRUE, 0, HTIMER_SECOND_NUM); //disable auto clean route entry
#endif                                    /*MESH_TEST_ROUTE_AUTO*/
    lwns_mesh_init(&lwns_mesh_test, 132,  // Open a mesh network with port number 132, and then occupy the other two ports as searching for routes, that is, 133 and 134 are also opened at the same time.
                   HTIMER_SECOND_NUM / 2, // Netflood's QUEUETIME function
                   1,                     // netflood dups function
                   2,                     // The maximum number of jumps is 5 levels, that is, the data packet can be forwarded up to 5 times, and the data packet will be discarded if it exceeds the value.
                   FALSE,                 // During the forwarding process of routing requests, a new data packet that needs to be forwarded is received. Should the old data packet be sent out immediately or discarded? FALSE is sent immediately and TRUE is discarded.
                   50,                    // Network recovery parameter, this value defines a gap. If the packet sequence number is smaller than the packet sequence number stored in memory is greater than this value, the network failure will be considered to be restored and the packet will continue to be received.
                   // At the same time, this value also determines the difference value of the new data packet, that is, the sequence number of the new data packet from the same node cannot be much larger than that in memory, that is, it is larger than this value.
                   // For example, the stored data packet number in memory is 10, the new packet number is 60, and the difference is 50, which is greater than or equal to the 50 set at this time, so it will not be considered as a new packet and will be discarded.
                   // Only if the sequence number is 59 and the difference is 49, which is less than this value, will it be received.
                   route_enable,          // Whether to enable the routing function of the local machine, if false, it will not respond to routing requests from other nodes.
                   TRUE,                  // Decide whether to add a routing loop, for example, if a mesh packet from node a is received, whether the machine needs to store the routing table to node a.FALSE does not exist, TRUE is present.
                   HTIMER_SECOND_NUM * 2, // The route timeout time, if the time exceeds the time, stop looking for the route and enter the timeout callback. This value should be greater than route_discovery_hoptime * (hops+1) * 2
                   &callbacks);           // If the routing table is not initialized, 0 will be returned, which means that the opening has failed.Return to 1 Open successfully.
    if(MESH_TEST_SELF_ADDR_IDX == 0)
    { // If it is a host, start periodically training other nodes.
        mesh_test_send_dst = 1;
        tmos_start_task(lwns_mesh_test_taskID, MESH_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(200));
#if MESH_TEST_ROUTE_AUTO
        PRINTF("Auto route\n");
#else
        uint8_t i;
        for(i = 1; i < 5; i++)
        {
            lwns_route_add(&device_addr[i], &device_addr[1], 2); // Manual management, add routing entries
        }
        for(i = 5; i < 9; i++)
        {
            lwns_route_add(&device_addr[i], &device_addr[5], 2); // Manual management, add routing entries
        }
        for(i = 9; i < 13; i++)
        {
            lwns_route_add(&device_addr[i], &device_addr[9], 2); // Manual management, add routing entries
        }
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 1)
    {
        lwns_route_add(&device_addr[4], &device_addr[4], 1); // Manual management, add routing entries
        lwns_route_add(&device_addr[3], &device_addr[3], 1); // Manual management, add routing entries
        lwns_route_add(&device_addr[2], &device_addr[2], 1); // Manual management, add routing entries
        lwns_route_add(&device_addr[0], &device_addr[0], 1); // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 2)
    {
        lwns_route_add(&device_addr[0], &device_addr[1], 2); // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 3)
    {
        lwns_route_add(&device_addr[0], &device_addr[1], 2); // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 4)
    {
        lwns_route_add(&device_addr[0], &device_addr[1], 2); // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 5)
    {
        lwns_route_add(&device_addr[6], &device_addr[6], 1); // Manual management, add routing entries
        lwns_route_add(&device_addr[7], &device_addr[7], 1); // Manual management, add routing entries
        lwns_route_add(&device_addr[8], &device_addr[8], 1); // Manual management, add routing entries
        lwns_route_add(&device_addr[0], &device_addr[0], 1); // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 6)
    {
        lwns_route_add(&device_addr[0], &device_addr[5], 2); // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 7)
    {
        lwns_route_add(&device_addr[0], &device_addr[5], 2); // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 8)
    {
        lwns_route_add(&device_addr[0], &device_addr[5], 2); // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 9)
    {
        lwns_route_add(&device_addr[10], &device_addr[10], 1); // Manual management, add routing entries
        lwns_route_add(&device_addr[11], &device_addr[11], 1); // Manual management, add routing entries
        lwns_route_add(&device_addr[12], &device_addr[12], 1); // Manual management, add routing entries
        lwns_route_add(&device_addr[0], &device_addr[0], 1);   // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 10)
    {
        lwns_route_add(&device_addr[0], &device_addr[9], 2); // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 11)
    {
        lwns_route_add(&device_addr[0], &device_addr[9], 2); // Manual management, add routing entries
    }
    else if(MESH_TEST_SELF_ADDR_IDX == 12)
    {
        lwns_route_add(&device_addr[0], &device_addr[9], 2); // Manual management, add routing entries
#endif /*MESH_TEST_ROUTE_AUTO*/
    }
}

/*********************************************************************
 * @fn      lwns_mesh_test_ProcessEvent
 *
 * @brief   lwns mesh Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed.
 */
static uint16_t lwns_mesh_test_ProcessEvent(uint8_t task_id, uint16_t events)
{
    if(events & MESH_EXAMPLE_TX_PERIOD_EVT)
    { // Host periodically polls slave tasks
        uint8_t                  temp;
        struct lwns_route_entry *rt;
        temp = TX_DATA[0];
        for(uint8_t i = 0; i < 9; i++)
        {
            TX_DATA[i] = TX_DATA[i + 1]; // Shift the data to observe the effect
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA));          // Load the data to be sent to the buffer
        rt = lwns_route_lookup(&device_addr[mesh_test_send_dst]); // Find the next hop route in the routing table
        if(rt != NULL)
        {
            // Print the next hop routing information
            PRINTF("dst:%d,forwarding to %02x.%02x.%02x.%02x.%02x.%02x\n",
                   mesh_test_send_dst, rt->nexthop.v8[0], rt->nexthop.v8[1],
                   rt->nexthop.v8[2], rt->nexthop.v8[3], rt->nexthop.v8[4],
                   rt->nexthop.v8[5]);
        }
        else
        {
            PRINTF("no route to dst:%d\n", mesh_test_send_dst);
        }
        lwns_mesh_send(&lwns_mesh_test, &device_addr[mesh_test_send_dst]); // Send mesh message
        mesh_test_send_dst++;                                              // Poll the target node to next
        if(mesh_test_send_dst > MESH_TEST_ADDR_MAX_IDX)
        { // Repeat polling
            mesh_test_send_dst = 1;
        }
        tmos_start_task(lwns_mesh_test_taskID, MESH_EXAMPLE_TX_PERIOD_EVT,
                        MS1_TO_SYSTEM_TIME(2500)); // Periodic polling
        return (events ^ MESH_EXAMPLE_TX_PERIOD_EVT);
    }
    if(events & MESH_EXAMPLE_TX_NODE_EVT)
    { // The task of a node sending data to the host
        struct lwns_route_entry *rt;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); // Load the data to be sent to the buffer
        rt = lwns_route_lookup(&device_addr[0]);         // Find the next hop route in the routing table
        if(rt != NULL)
        {
            // Print out the next hop routing information
            PRINTF("src:%d,forwarding to %02x.%02x.%02x.%02x.%02x.%02x\n",
                   MESH_TEST_SELF_ADDR_IDX, rt->nexthop.v8[0],
                   rt->nexthop.v8[1], rt->nexthop.v8[2], rt->nexthop.v8[3],
                   rt->nexthop.v8[4], rt->nexthop.v8[5]);
        }
        lwns_mesh_send(&lwns_mesh_test, &device_addr[0]); // Call the mesh send function to send data
        return (events ^ MESH_EXAMPLE_TX_NODE_EVT);
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
