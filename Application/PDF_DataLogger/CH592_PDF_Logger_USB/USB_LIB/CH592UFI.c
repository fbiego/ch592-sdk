/* 2014.09.09
*****************************************
**   Copyright  (C)  W.ch  1999-2019   **
**   Web:      http://wch.cn           **
*****************************************
**  USB-flash File Interface for CH592 **
**  KEIL423, gcc 8.20          **
*****************************************
*/
/* CHRV3 U盘主机文件系统接口, 支持: FAT12/FAT16/FAT32 */

//#define DISK_BASE_BUF_LEN		512	/* 默认的磁盘数据缓冲区大小为512字节(可以选择为2048甚至4096以支持某些大扇区的U盘),为0则禁止在本文件中定义缓冲区并由应用程序在pDISK_BASE_BUF中指定 */
/* If you need to multiplex the disk data buffer to save RAM, then DISK_BASE_BUF_LEN can be defined as 0 to prohibit the definition of buffers in this file. The application will place the buffer start address used with other programs into the pDISK_BASE_BUF variable before calling CHRV3LibInit */

#define NO_DEFAULT_ACCESS_SECTOR	1		/* The default disk sector reading and writing subroutine is prohibited, and the following is a self-written program instead of it. */
//#define NO_DEFAULT_DISK_CONNECT		1		/* 禁止默认的检查磁盘连接子程序,下面用自行编写的程序代替它 */
// #define NO_DEFAULT_FILE_ENUMER 1 /* The default file name enumeration callback program is prohibited, and the following is a self-written program instead of it */

#include "CH59x_common.h"
#include "CHRV3UFI.h"
#include "Internal_Flash.h"
#include "SW_UDISK.h"
#include "pdfFile.h"
#include "spi_flash.h"
#include "peripheral.h"

UINT8 CtrlGetConfigDescrTB(void) // 获取配置描述符,返回在TxBuffer中
{
    return (CtrlGetConfigDescr());
}

CMD_PARAM_I mCmdParam; /* 命令参数 */
#if DISK_BASE_BUF_LEN > 0
// UINT8 DISK_BASE_BUF[ DISK_BASE_BUF_LEN ] __attribute__((at(BA_RAM+SZ_RAM/2))); /* The disk data buffer of external RAM, the buffer length is the length of one sector */
// UINT8 DISK_BASE_BUF[DISK_BASE_BUF_LEN] __attribute__((aligned(4))); /* The disk data buffer of external RAM, the buffer length is the length of one sector */
// UINT8 DISK_FAT_BUF[ DISK_BASE_BUF_LEN ] __attribute__((aligned (4))); /* Disk FAT data buffer of external RAM, buffer length is the length of one sector */
#endif

/* 以下程序可以根据需要修改 */

#ifndef NO_DEFAULT_ACCESS_SECTOR /* 在应用程序中定义NO_DEFAULT_ACCESS_SECTOR可以禁止默认的磁盘扇区读写子程序,然后用自行编写的程序代替它 */
// if ( use_external_interface ) { // Replace the underlying read and write subroutine of the USB disk sector
// CHRV3vSectorSize=512; // Set the actual sector size, which must be a multiple of 512, and this value is the sector size of the disk
// CHRV3vSectorSizeB=9; // Set the displacement number of the actual sector size, 512 corresponds to 9, 1024 corresponds to 10, 2048 corresponds to 11
//    CHRV3DiskStatus=DISK_MOUNTED;  // 强制块设备连接成功(只差分析文件系统)
//}

UINT8 CHRV3ReadSector(UINT8 SectCount, PUINT8 DataBuf) /* Read data from multiple sectors into the buffer from disk */
{
    UINT8 retry;
    // if ( use_external_interface ) return( extReadSector( CHRV3vLbaCurrent, SectCount, DataBuf ) ); /* External interface */
    for(retry = 0; retry < 3; retry++)
    {                                                                /* 错误重试 */
        pCBW->mCBW_DataLen = (UINT32)SectCount << CHRV3vSectorSizeB; /* Data transmission length */
        pCBW->mCBW_Flag = 0x80;
        pCBW->mCBW_LUN = CHRV3vCurrentLun;
        pCBW->mCBW_CB_Len = 10;
        pCBW->mCBW_CB_Buf[0] = SPC_CMD_READ10;
        pCBW->mCBW_CB_Buf[1] = 0x00;
        pCBW->mCBW_CB_Buf[2] = (UINT8)(CHRV3vLbaCurrent >> 24);
        pCBW->mCBW_CB_Buf[3] = (UINT8)(CHRV3vLbaCurrent >> 16);
        pCBW->mCBW_CB_Buf[4] = (UINT8)(CHRV3vLbaCurrent >> 8);
        pCBW->mCBW_CB_Buf[5] = (UINT8)(CHRV3vLbaCurrent);
        pCBW->mCBW_CB_Buf[6] = 0x00;
        pCBW->mCBW_CB_Buf[7] = 0x00;
        pCBW->mCBW_CB_Buf[8] = SectCount;
        pCBW->mCBW_CB_Buf[9] = 0x00;
        CHRV3BulkOnlyCmd(DataBuf); /* Execute commands based on BulkOnly protocol */
        if(CHRV3IntStatus == ERR_SUCCESS)
        {
            return (ERR_SUCCESS);
        }
        CHRV3IntStatus = CHRV3AnalyzeError(retry);
        if(CHRV3IntStatus != ERR_SUCCESS)
        {
            return (CHRV3IntStatus);
        }
    }
    return (CHRV3IntStatus = ERR_USB_DISK_ERR); /* Disk operation error */
}

  #ifdef EN_DISK_WRITE
UINT8 CHRV3WriteSector(UINT8 SectCount, PUINT8 DataBuf) /* Write data blocks of multiple sectors in the buffer to disk */
{
    UINT8 retry;
    // if ( use_external_interface ) return( extWriteSector( CHRV3vLbaCurrent, SectCount, DataBuf ) ); /* External interface */
    for(retry = 0; retry < 3; retry++)
    {                                                                /* Try again in error */
        pCBW->mCBW_DataLen = (UINT32)SectCount << CHRV3vSectorSizeB; /* Data transmission length */
        pCBW->mCBW_Flag = 0x00;
        pCBW->mCBW_LUN = CHRV3vCurrentLun;
        pCBW->mCBW_CB_Len = 10;
        pCBW->mCBW_CB_Buf[0] = SPC_CMD_WRITE10;
        pCBW->mCBW_CB_Buf[1] = 0x00;
        pCBW->mCBW_CB_Buf[2] = (UINT8)(CHRV3vLbaCurrent >> 24);
        pCBW->mCBW_CB_Buf[3] = (UINT8)(CHRV3vLbaCurrent >> 16);
        pCBW->mCBW_CB_Buf[4] = (UINT8)(CHRV3vLbaCurrent >> 8);
        pCBW->mCBW_CB_Buf[5] = (UINT8)(CHRV3vLbaCurrent);
        pCBW->mCBW_CB_Buf[6] = 0x00;
        pCBW->mCBW_CB_Buf[7] = 0x00;
        pCBW->mCBW_CB_Buf[8] = SectCount;
        pCBW->mCBW_CB_Buf[9] = 0x00;
        CHRV3BulkOnlyCmd(DataBuf); /* Execute commands based on BulkOnly protocol */
        if(CHRV3IntStatus == ERR_SUCCESS)
        {
            mDelayuS(200); /* Delay after write operation */
            return (ERR_SUCCESS);
        }
        CHRV3IntStatus = CHRV3AnalyzeError(retry);
        if(CHRV3IntStatus != ERR_SUCCESS)
        {
            return (CHRV3IntStatus);
        }
    }
    return (CHRV3IntStatus = ERR_USB_DISK_ERR); /* Disk operation error */
}
  #endif
#else
UINT8 CHRV3ReadSector(UINT8 SectCount, PUINT8 DataBuf) /* Read multiple sectors of data from disk into a buffer */
{
    int len;
    uint32_t addr;

    len =(uint32_t)SectCount << CHRV3vSectorSizeB;
    addr = CHRV3vLbaCurrent * DEF_UDISK_SECTOR_SIZE;

    FLASH_RD_Block_Start( addr );
    FLASH_RD_Block( DataBuf, len );
    FLASH_RD_Block_End();
//    pdf_memcpy(DataBuf, (uint8_t *)addr, len);
    CHRV3IntStatus = ERR_SUCCESS;

    return ERR_SUCCESS;
}

UINT8 CHRV3WriteSector(UINT8 SectCount, PUINT8 DataBuf) /* Write data blocks of multiple sectors in the buffer to disk */
{
    int len;
    uint32_t addr;

    len =(uint32_t)SectCount << CHRV3vSectorSizeB;
    addr = CHRV3vLbaCurrent * DEF_UDISK_SECTOR_SIZE;

    pdf_memcpy(RAM_BUFFER.PDF_BUFFER.UDisk_Down_Buffer, DataBuf, len);
    W25XXX_WR_512B(RAM_BUFFER.PDF_BUFFER.UDisk_Down_Buffer, addr, 512);
//    IFlash_Prog_512( addr,(uint32_t*)UDisk_Down_Buffer);
    CHRV3IntStatus = ERR_SUCCESS;

    return ERR_SUCCESS;
}
#endif // NO_DEFAULT_ACCESS_SECTOR

#ifndef NO_DEFAULT_DISK_CONNECT /* Defining NO_DEFAULT_DISK_CONNECT in the application can prohibit the default check of disk connection subroutines and replace it with a self-written program */

/*
约定: USB设备地址分配规则(参考USB_DEVICE_ADDR)
地址值  设备位置
0x02    内置Root-HUB0下的USB设备或外部HUB
0x03    内置Root-HUB1下的USB设备或外部HUB
0x1x    内置Root-HUB0下的外部HUB的端口x下的USB设备,x为1~n
0x2x    内置Root-HUB1下的外部HUB的端口x下的USB设备,x为1~n
*/

//#define		UHUB_DEV_ADDR	( CHRV3vRootPort ? R8_USB1_DEV_AD : R8_USB0_DEV_AD )
//#define		UHUB_MIS_STAT	( CHRV3vRootPort ? R8_USB1_MIS_ST : R8_USB0_MIS_ST )
//#define		UHUB_HOST_CTRL	( CHRV3vRootPort ? R8_UHOST1_CTRL : R8_UHOST0_CTRL )
//#define		UHUB_INT_FLAG	( CHRV3vRootPort ? R8_USB1_INT_FG : R8_USB0_INT_FG )
#define UHUB_DEV_ADDR     R8_USB_DEV_AD
#define UHUB_MIS_STAT     R8_USB_MIS_ST
#define UHUB_HOST_CTRL    R8_UHOST_CTRL
#define UHUB_INT_FLAG     R8_USB_INT_FG
#define bUMS_ATTACH       RB_UMS_DEV_ATTACH
#define bUMS_SUSPEND      RB_UMS_SUSPEND

/* Check if the disk is connected */
UINT8 CHRV3DiskConnect(void)
{
    UINT8 ums, devaddr;
    UHUB_DEV_ADDR = UHUB_DEV_ADDR & 0x7F;
    ums = UHUB_MIS_STAT;
    devaddr = UHUB_DEV_ADDR;
    if(devaddr == USB_DEVICE_ADDR)
    {   /* 内置Root-HUB下的USB设备 */
        // if (UHUB_HOST_CTRL & RB_UH_PORT_EN ) { /* The USB device under built-in Root-HUB exists and is not plugged in */
        if(ums & bUMS_ATTACH)
        {   /* The USB device under built-in Root-HUB exists */
            // if ( ( UHUB_INT_FLAG & UIF_DETECT ) == 0 ) { /* The USB device under built-in Root-HUB exists and is not plugged in */
            if((ums & bUMS_SUSPEND) == 0)
            {                         /* 内置Root-HUB下的USB设备存在且未插拔 */
                return (ERR_SUCCESS); /* USB设备已经连接且未插拔 */
            }
            else
            { /* 内置Root-HUB下的USB设备存在 */
            mDiskConnect:
                CHRV3DiskStatus = DISK_CONNECT; /* 曾经断开过 */
                return (ERR_SUCCESS);           /* The external HUB or USB device has been connected or disconnected and reconnected */
            }
        }
        else
        { /* USB device disconnected */
        mDiskDisconn:
            CHRV3DiskStatus = DISK_DISCONNECT;
            return (ERR_USB_DISCON);
        }
    }
  #ifndef FOR_ROOT_UDISK_ONLY
    else if(devaddr > 0x10 && devaddr <= 0x14)
    {   /* 外部HUB的端口下的USB设备 */
        // if (UHUB_HOST_CTRL & RB_UH_PORT_EN ) { /* The external HUB under the built-in Root-HUB exists and is not plugged and unplugged */
        if(ums & bUMS_ATTACH)
        {   /* The USB device under built-in Root-HUB exists */
            // if ( ( UHUB_INT_FLAG & UIF_DETECT ) == 0 ) { /* The USB device under built-in Root-HUB exists and is not plugged in */
            if((ums & bUMS_SUSPEND) == 0)
            {                                                                            /* The USB device under the built-in Root-HUB exists and is not plugged in */
                TxBuffer[MAX_PACKET_SIZE - 1] = devaddr;                                 /* Backup */
                UHUB_DEV_ADDR = USB_DEVICE_ADDR - 1 + (UHUB_DEV_ADDR >> 4);              /* Set the USB address of the USB host side to point to HUB */
                CHRV3IntStatus = HubGetPortStatus(TxBuffer[MAX_PACKET_SIZE - 1] & 0x0F); /* Query the HUB port status and return it in TxBuffer */
                if(CHRV3IntStatus == ERR_SUCCESS)
                {
                    if(TxBuffer[2] & (1 << (HUB_C_PORT_CONNECTION - 0x10)))
                    {                                                                                     /* Plug and unplug event detected on HUB port */
                        CHRV3DiskStatus = DISK_DISCONNECT;                                                /* Assume that the USB device on the HUB port is disconnected */
                        HubClearPortFeature(TxBuffer[MAX_PACKET_SIZE - 1] & 0x0F, HUB_C_PORT_CONNECTION); /* Clear the HUB port connection event status */
                    }
                    UHUB_DEV_ADDR = TxBuffer[MAX_PACKET_SIZE - 1]; /* Set the USB address on the USB host side to point to the USB device */
                    if(TxBuffer[0] & (1 << HUB_PORT_CONNECTION))
                    { /* 连接状态 */
                        if(CHRV3DiskStatus < DISK_CONNECT)
                        {
                            CHRV3DiskStatus = DISK_CONNECT; /* Have been disconnected */
                        }
                        return (ERR_SUCCESS); /* The USB device has been connected or disconnected and reconnected */
                    }
                    else
                    {
                        //						CHRV3DiskStatus = DISK_DISCONNECT;
                        //						return( ERR_USB_DISCON );
                        CHRV3DiskStatus = DISK_CONNECT;
                        return (ERR_HUB_PORT_FREE); /* HUB is already connected but the HUB port is not connected to the disk */
                    }
                }
                else
                {
                    UHUB_DEV_ADDR = TxBuffer[MAX_PACKET_SIZE - 1]; /* Set the USB address on the USB host side to point to the USB device */
                    if(CHRV3IntStatus == ERR_USB_DISCON)
                    {
                        //						CHRV3DiskStatus = DISK_DISCONNECT;
                        //						return( ERR_USB_DISCON );
                        goto mDiskDisconn;
                    }
                    else
                    {
                        CHRV3DiskStatus = DISK_CONNECT; /* HUB operation failed */
                        return (CHRV3IntStatus);
                    }
                }
            }
            else
            {   /* 内置Root-HUB下的USB设备存在,外部HUB或USB设备已经连接或者断开后重新连接 */
                // CHRV3DiskStatus = DISK_CONNECT; /* Once disconnected */
                // return( ERR_SUCCESS ); /* The external HUB or USB device has been connected or disconnected and reconnected */
                goto mDiskConnect;
            }
        }
        else
        { /* External HUB disconnected */
            CHRV3DiskStatus = DISK_DISCONNECT;
        }
    }
  #endif
    else
    {
        //		CHRV3DiskStatus = DISK_DISCONNECT;
        //		return( ERR_USB_DISCON );
        goto mDiskDisconn;
    }
}
#endif // NO_DEFAULT_DISK_CONNECT

#ifndef NO_DEFAULT_FILE_ENUMER /* 在应用程序中定义NO_DEFAULT_FILE_ENUMER可以禁止默认的文件名枚举回调程序,然后用自行编写的程序代替它 */
void xFileNameEnumer(void)     /* File name enumeration callback subroutine */
{
    /* If you call FileOpen after specifying the enumeration number CHRV3vFileSize to 0xFFFFFFFF, then this callback will be called every time a file FileOpen is searched.
After the callback xFileNameEnumer returns, FileOpen decrements CHRV3vFileSize and continues to enumerate until no file or directory is searched.The recommended approach is,
Before calling FileOpen, define a global variable as 0. When FileOpen calls back to this program, this program obtains the structure FAT_DIR_INFO from CHRV3vFdtOffset.
Analyze the DIR_Attr and DIR_Name in the structure to determine whether they are the required file name or directory name, record relevant information, and count the global variables into increments.
When FileOpen returns, it is determined that if the return value is ERR_MISS_FILE or ERR_FOUND_NAME, it is considered that the operation is successful, and the global variable is the number of valid files found.
If CHRV3vFileSize is set to 1 in this callback xFileNameEnumer, you can notify FileOpen to end the search in advance.The following is a callback example */
  #if 0
    UINT8           i;
    UINT16          FileCount;
    PX_FAT_DIR_INFO pFileDir;
    PUINT8          NameBuf;
    pFileDir = (PX_FAT_DIR_INFO)(pDISK_BASE_BUF + CHRV3vFdtOffset); /* 当前FDT的起始地址 */
    FileCount = (UINT16)(0xFFFFFFFF - CHRV3vFileSize);              /* The enumeration number of the current file name, the initial value of CHRV3vFileSize is 0xFFFFFFFF. After finding the file name, it is reduced. */
    if(FileCount < sizeof(FILE_DATA_BUF) / 12)
    {                                             /* 检查缓冲区是否足够存放,假定每个文件名需占用12个字节存放 */
        NameBuf = &FILE_DATA_BUF[FileCount * 12]; /* Calculate the buffer address that saves the current file name */
        for(i = 0; i < 11; i++)
            NameBuf[i] = pFileDir->DIR_Name[i]; /* Copy file name, length is 11 characters, no spaces processed */
                                                //		if ( pFileDir -> DIR_Attr & ATTR_DIRECTORY ) NameBuf[ i ] = 1;  /* 判断是目录名 */
        NameBuf[i] = 0;                         /* 文件名结束符 */
    }
  #endif
}
#endif // NO_DEFAULT_FILE_ENUMER

UINT8 CHRV3LibInit(void) /* Initialize the CHRV3 program library, the operation returns 0 successfully */
{
    if(CHRV3GetVer() < CHRV3_LIB_VER)
        return (0xFF); /* 获取当前子程序库的版本号,版本太低则返回错误 */
#if DISK_BASE_BUF_LEN > 0
    pDISK_BASE_BUF = &RAM_BUFFER.PDF_BUFFER.DISK_BASE_BUF[0]; /* Disk data buffer pointing to external RAM */
//    pDISK_FAT_BUF = &DISK_BASE_BUF[0];  /* 指向外部RAM的磁盘FAT数据缓冲区,可以与pDISK_BASE_BUF合用以节约RAM */
	pDISK_FAT_BUF = &RAM_BUFFER.PDF_BUFFER.DISK_FAT_BUF[0];  /* Disk FAT data buffer pointing to external RAM, independent of pDISK_BASE_BUF to increase speed */
/* If you want to improve file access speed, you can repoint pDISK_FAT_BUF to another independently allocated buffer of the same size as pDISK_BASE_BUF after calling CHRV3LibInit in the main program. */
#endif
    CHRV3DiskStatus = DISK_UNKNOWN;          /* Unknown status */
    CHRV3vSectorSizeB = 9;                  /* The default physical disk sector is 512B */
    CHRV3vSectorSize = 512;                 // The default sector of the physical disk is 512B, which is the sector size of the disk
    CHRV3vStartLba = 0;                      /* Default is to automatically analyze FDD and HDD */
    CHRV3vPacketSize = 64;                   /* The maximum package length of USB storage device: 64@FS, 512@HS/SS, initialized by the application, enumerate the USB disk, if it is high-speed or overspeed, it will be updated to 512 in time. */
    pTX_DMA_A_REG = (PUINT32)&R16_UH_TX_DMA; /* Point to the send DMA address register, initialized by the application */
    pRX_DMA_A_REG = (PUINT32)&R16_UH_RX_DMA; /* Point to the receiving DMA address register, initialized by the application */
    pTX_LEN_REG = (PUINT16)&R8_UH_TX_LEN;    /* Point to the send length register, initialized by the application */
    pRX_LEN_REG = (PUINT16)&R8_USB_RX_LEN;   /* Point to the receive length register, initialized by the application */

    // CHRV3vRootPort = 0; /* USB host selection (similar to Root-hub root hub selection port) */
    return (ERR_SUCCESS);
}
