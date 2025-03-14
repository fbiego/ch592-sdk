/* ********************************* (C) COPYRIGHT ***************************
* File Name : rf_test.c
* Author: WCH
* Version: V1.0
* Date: 2022/03/15
* Description: rf send and receive test routine, one-way send
* PB15 low level is the sending mode, default is the receiving mode
*
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* SPDX-License-Identifier: Apache-2.0
********************************************************************************************* */

/******************************************************************************/
/* The header file contains */
#include "ch9160.h"
#include <rf.h>
#include "CH59x_common.h"
#include "hal.h"
#include "nvs_flash.h"
#include "app.h"

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
#define  ALIGNED4(x)       ((x+3)/4*4)

#define  RF_RXBUFNB        16
#define  RF_TXBUFNB        16  // ????8

#define  RF_RX_BUF_SZE     64
#define  RF_TX_BUF_SZE     64

RF_DMADESCTypeDef DMARxDscrTab[RF_RXBUFNB];
RF_DMADESCTypeDef DMATxDscrTab[RF_TXBUFNB];

__attribute__((__aligned__(4)))  uint8_t RFRxBuf[RF_RXBUFNB][ALIGNED4( RF_RX_BUF_SZE+PKT_HEAD_LEN+4 )];
__attribute__((__aligned__(4)))  uint8_t RFTxBuf[RF_TXBUFNB][ALIGNED4( RF_TX_BUF_SZE+PKT_HEAD_LEN+8 )];

#define  ROLE_SPEED_LIST_NUM                  1
rfRoleList_t SpeedList[ROLE_SPEED_LIST_NUM];

rfRoleSpeed_t  gSpeedList_t;
uint32_t gTxCount = 0;

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
rfRoleParam_t gParm;

uint32_t RFMode;
#define  RF_MODE_RX  0
#define  RF_MODE_TX  1

RF_DMADESCTypeDef *pDMARxGet;
RF_DMADESCTypeDef *pDMATxGet;

int8_t rssi = 0;
tmosTaskID rfTaskID;
uint8_t gDeviceId;

uint8_t self_mac[6] = {0};
/* *********************************************************************************************
* @fn rfRoleBoundProcess
*
* @brief RF Send and receive DMA initialization
*
* @param None.
*
* @return None. */
void rfDMADescInit( void )
{
    int num;

    for( num = 0; num < RF_RXBUFNB; num++ )
    {
        DMARxDscrTab[num].Status = STA_DMA_ENABLE;
        DMARxDscrTab[num].BufferSize = RF_RX_BUF_SZE + PKT_HEAD_LEN;
        DMARxDscrTab[num].BufferAddr = (uint32_t) RFRxBuf[num];
        DMARxDscrTab[num].NextDescAddr = (uint32_t) &DMARxDscrTab[num + 1];
    }
    DMARxDscrTab[RF_RXBUFNB - 1].NextDescAddr = (uint32_t) &DMARxDscrTab[0];
    pDMARxGet = DMARxDscrTab;

    for( num = 0; num < RF_TXBUFNB; num++ )
    {
        DMATxDscrTab[num].Status = 0;
        DMATxDscrTab[num].BufferSize = RF_TX_BUF_SZE + PKT_HEAD_LEN;
        DMATxDscrTab[num].BufferAddr = (uint32_t) RFTxBuf[num];
        DMATxDscrTab[num].NextDescAddr = (uint32_t) &DMATxDscrTab[num + 1];
    }
    DMATxDscrTab[RF_TXBUFNB - 1].NextDescAddr = (uint32_t) &DMATxDscrTab[0];
    pDMATxGet = DMATxDscrTab;
}

/* *********************************************************************************************
* @fn rf_get_data
*
* @brief Get a packet of RF receiving data in DMA
*
* @param None.
*
* @return None. */
__HIGH_CODE
uint8_t *rf_get_data( uint8_t *pLen)
{
    if(!(pDMARxGet->Status & STA_DMA_ENABLE))
    {
        *pLen = (pDMARxGet->Status & STA_LEN_MASK) - PKT_HEAD_LEN;
        return (uint8_t *)(pDMARxGet->BufferAddr + PKT_HEAD_LEN);
    }
    else {
        return 0;
    }
}

/* *********************************************************************************************
* @fn rf_delete_data
*
* @brief Delete a packet of RF receiving data in DMA
*
* @param None.
*
* @return None. */
__HIGH_CODE
void rf_delete_data()
{
    pDMARxGet->Status = STA_DMA_ENABLE;
    pDMARxGet = (RF_DMADESCTypeDef *) pDMARxGet->NextDescAddr;
}

/* *********************************************************************************************
* @fn rf_send_data
*
* @brief Add a packet of data to send DMA to RF
*
* @param None.
*
* @return None. */
__HIGH_CODE
uint8_t rf_send_data( uint8_t *pData, uint8_t len)
{
    if( !(pDMATxGet->Status & STA_DMA_ENABLE) )
    {
        rfPackage_t *p = (rfPackage_t *) pDMATxGet->BufferAddr;
        uint8_t *pPdu;

        p->type = gDeviceId;
        p->length = PKT_ACK_LEN + len;
        pPdu = (uint8_t *) (pDMATxGet->BufferAddr + PKT_HEAD_LEN);
        tmos_memcpy(pPdu, pData, len);
        pDMATxGet->BufferSize = len + PKT_HEAD_LEN;
        pDMATxGet->Status = STA_DMA_ENABLE;
        pDMATxGet = (RF_DMADESCTypeDef *) pDMATxGet->NextDescAddr;
        return 0;
    }
    return 0xFF;
}

/* *********************************************************************************************
* @fn RF_ProcessCallBack
*
* @brief RF state callback, note that this function is called in interrupt
*
* @param None.
*
* @return None. */
__HIGH_CODE
void RF_ProcessCallBack( rfRole_States_t sta,uint8_t id  )
{
    // Received a data callback
    if( sta & RF_STATE_RX )
    {
//        rssi = RFIP_ReadRssi();
    }
    // Receive DMA full
    if( sta & RF_STATE_RBU )
    {
//        PRINT( "!\n" );
    }
    if( sta & RF_STATE_TX_FINISH )
    {
    }
    if( sta & RF_STATE_TX_IDLE )
    {
    }
}

/* *********************************************************************************************
* @fn rfRoleBoundProcess
*
* @brief Connection binding status callback
*
* @param None.
*
* @return None. */
void rfRoleBoundProcess( staBound_t *pSta )
{
    PRINT( "bound %x\n",pSta->status );
//    PRINT( "status=%x\n", pSta->status );
//    PRINT( "role=%x\n", pSta->role );
//    PRINT( "id=%x\n", pSta->devId );
//    PRINT( "type=%x\n", pSta->devType );
//    PRINT( "hop=%x\n", pSta->hop );
    PRINT( "mac=%x %x %x %x %x %x\n", pSta->PeerInfo[0], pSta->PeerInfo[1], pSta->PeerInfo[2], pSta->PeerInfo[3], pSta->PeerInfo[4], pSta->PeerInfo[5] );
    if( !pSta->status )
    {
        gDeviceId = pSta->devId;
        // Determine the current RF role
        if( !(pSta->role&1) )
        {
            // Currently the receiver (dongle)
            // Configure the current connection parameters. Since there is only one device, all intervals are set to the same device.
            SpeedList[0].deviceId = pSta->devId;
            SpeedList[0].rssi = 0; // 0- means that all rssi devices can be connected, and other values ​​can only connect devices with RSSI greater than this value.
            tmos_memcpy(SpeedList[0].peerInfo, pSta->PeerInfo, 6);
            gSpeedList_t.number = ROLE_SPEED_LIST_NUM;
            gSpeedList_t.pList = SpeedList;
            RFBound_SetSpeedType( &gSpeedList_t );
            if(!tmos_memcmp(nvs_flash_info.peer_mac, pSta->PeerInfo, 6))
            {
                tmos_memcpy(nvs_flash_info.peer_mac, pSta->PeerInfo, 6);
                nvs_flash_store();
            }
            tmos_stop_task(rfTaskID, RF_START_PEER_BOUND_EVENT);
        }
        else
        {
            // Currently the sending end (mouse)
        }
    }
    else
    {
        if( !(pSta->role&1) )
        {
            // Currently the receiver (dongle)
        }
        else
        {
            // Currently the sending end (mouse)
        }
        // RF automatic reconnection timeout failed
        if( pSta->status == FAILURE )
        {
            RFRole_ClearRxData(gDeviceId);
            tmos_set_event( rfTaskID, RF_START_PEER_BOUND_EVENT );
        }
        // RF is currently disconnected, and the connection is automatically enabled
        else if( pSta->status == bleTimeout )
        {
            PRINT( "connect timeout.\n" );
        }
    }
}

__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void LLE_IRQHandler( void )
{
    LLE_LibIRQHandler();
}

__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void BB_IRQHandler( void )
{
    BB_LibIRQHandler();
}

/* *********************************************************************************************
* @fn RF_ProcessEvent
*
* @brief RF layer system task processing
*
* @param None.
*
* @return None. */
tmosEvents RF_ProcessEvent( tmosTaskID task_id, tmosEvents events )
{
    if( events & SYS_EVENT_MSG )
    {
        uint8_t * msgPtr;

        msgPtr = tmos_msg_receive( task_id );
        if( msgPtr )
        {
            /* De-allocate */
            tmos_msg_deallocate( msgPtr );
        }
        return events ^ SYS_EVENT_MSG;
    }

    if( events & RF_START_ALL_BOUND_EVENT )
    {
        rfBoundHost_t bound;
        tmos_memset( &bound, 0, sizeof(rfBoundHost_t) );

        // The first time you start, you can connect to any device
        SpeedList[0].deviceId = RF_ROLE_BOUND_ID;
        SpeedList[0].rssi = -70; // 0- means that all rssi devices can be connected, and other values ​​can only connect devices with RSSI greater than this value.
        SpeedList[0].devType = 0; // 0- means that all device types can be connected, or the type of device that can be connected
        tmos_memset(SpeedList[0].peerInfo, 0, 6);
        gSpeedList_t.number = ROLE_SPEED_LIST_NUM;
        gSpeedList_t.pList = SpeedList;
        RFBound_SetSpeedType( &gSpeedList_t );

        PRINT("start rx mode\n");
        bound.periTime = 8;
        bound.hop = RF_HOP_MANUF_MODE;
        bound.timeout = 100;
        bound.devType = 0;  // 0- means that all device types can be connected, or the type of device that can be connected
        tmos_memcpy(bound.OwnInfo, self_mac, 6);
        bound.rfBoundCB = rfRoleBoundProcess;
        RFBound_StartHost( &bound );
        return events ^ RF_START_ALL_BOUND_EVENT;
    }

    if( events & RF_START_PEER_BOUND_EVENT )
    {
        rfBoundHost_t bound;
        tmos_memset( &bound, 0, sizeof(rfBoundHost_t) );

        // The second time you start, you can only connect to paired devices
        SpeedList[0].deviceId = RF_ROLE_BOUND_ID;
        SpeedList[0].rssi = 0; // 0- means that all rssi devices can be connected, and other values ​​can only connect devices with RSSI greater than this value.
        SpeedList[0].devType = 0; // 0- means that all device types can be connected, or the type of device that can be connected
        tmos_memset(SpeedList[0].peerInfo, 0, 6);
        if(!tmos_isbufset(nvs_flash_info.peer_mac, 0, 6))
        {
            tmos_memcpy(SpeedList[0].peerInfo, nvs_flash_info.peer_mac, 6);
        }
        gSpeedList_t.number = ROLE_SPEED_LIST_NUM;
        gSpeedList_t.pList = SpeedList;
        RFBound_SetSpeedType( &gSpeedList_t );

        PRINT("start rx mode\n");
        bound.periTime = 8;
        bound.hop = RF_HOP_MANUF_MODE;
        bound.timeout = 100;
        bound.devType = 0;  // 0- means that all device types can be connected, or the type of device that can be connected
        tmos_memcpy(bound.OwnInfo, self_mac, 6);

        bound.rfBoundCB = rfRoleBoundProcess;
        RFBound_StartHost( &bound );
        return events ^ RF_START_PEER_BOUND_EVENT;
    }
    if( events & RF_TEST_TTX_EVENT )
    {
        SoftwareUART_Printf("%d\n", gTxCount );
        gTxCount = 0;
        return events ^ RF_TEST_TTX_EVENT;
    }

    return 0;
}

/* *********************************************************************************************
* @fn RF_Init
*
* @brief RF application layer initialization
*
* @param None.
*
* @return None. */
void RF_Init( void )
{
    rfTaskID = TMOS_ProcessEventRegister( RF_ProcessEvent );

    PRINT( "rf mode----------- Rx -----------\n" );
    PRINT( "%s\n", VER_LIB );
    GetMACAddress(self_mac);
    rfDMADescInit();
    {
        rfRoleConfig_t conf;

        conf.TxPower = LL_TX_POWEER_4_DBM;
        conf.pTx = DMATxDscrTab;
        conf.pRx = DMARxDscrTab;
        conf.rfProcessCB = RF_ProcessCallBack;
        conf.processMask = RF_STATE_RX | RF_STATE_RBU | RF_STATE_TX_FINISH | RF_STATE_TX_IDLE;
        RFRole_LibInit( &conf );

        gParm.accessAddress = DEFAULT_PAIR_ACCESS_ADDE;
        gParm.crcInit = 0x555555;
        gParm.frequency = 3;
        gParm.properties = PHY_MODE;
        gParm.rxMaxLen = RF_RX_BUF_SZE+PKT_ACK_LEN;
        PRINT("properties=%x\n",gParm.properties);
        RFRole_SetParam( &gParm );
    }
    PFIC_EnableIRQ( BLEB_IRQn );
    PFIC_EnableIRQ( BLEL_IRQn );
    PRINT( "******* f=%d\n", gParm.frequency );
    tmos_set_event( rfTaskID, RF_START_ALL_BOUND_EVENT );
//    tmos_set_event( rfTaskID, RF_START_PEER_BOUND_EVENT );
    tmos_start_task( rfTaskID, RF_START_PEER_BOUND_EVENT, START_PEER_BOUND_DELAY );
//    tmos_start_reload_task(rfTaskID, RF_TEST_TTX_EVENT, 1600);
}

/******************************** endfile @rf ******************************/
