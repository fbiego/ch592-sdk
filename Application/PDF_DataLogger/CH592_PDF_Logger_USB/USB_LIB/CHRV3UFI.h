/* 2014.09.09
*****************************************
**   Copyright  (C)  W.ch  1999-2019   **
**   Web:      http://wch.cn           **
*****************************************
**  USB-flash File Interface for CHRV3 **
**  KEIL423, gcc 8.20          **
*****************************************
*/
/* CHRV3 USB host file system interface, support: FAT12/FAT16/FAT32 */

//#include "CHRV3BAS.H"

#ifndef __CHRV3UFI_H__
#define __CHRV3UFI_H__

#define CHRV3_LIB_VER		0x10

//#define DISK_BASE_BUF_LEN		512	/* 默认的磁盘数据缓冲区大小为512字节(可以选择为2048甚至4096以支持某些大扇区的U盘),为0则禁止在本文件中定义缓冲区并由应用程序在pDISK_BASE_BUF中指定 */
/* If you need to multiplex the disk data buffer to save RAM, then DISK_BASE_BUF_LEN can be defined as 0 to prohibit the definition of buffers in this file. The application will place the buffer start address used with other programs into the pDISK_BASE_BUF variable before calling CHRV3LibInit */

// #define NO_DEFAULT_ACCESS_SECTOR 1 /* The default disk sector reading and writing subroutine is prohibited, and the following is a self-written program instead of it */
// #define NO_DEFAULT_DISK_CONNECT 1 /* The default check of disk connection subroutine is prohibited, and the following is a self-written program instead of it */
// #define NO_DEFAULT_FILE_ENUMER 1 /* The default file name enumeration callback program is prohibited, and the following is a self-written program instead of it */

#ifdef __cplusplus
extern "C" {
#endif

/* ********************************************************************************************************************* */

/* FILE: CHRV3UF.H */

/* Error code */
#ifndef ERR_SUCCESS
#define ERR_SUCCESS				0x00	/* Operation is successful */
#endif
#ifndef ERR_DISK_DISCON
#define ERR_CHRV3_ERROR			0x81	/* CHRV3 hardware error, CHRV3 may need to be reset */
// #define ERR_DISK_DISCON 0x82 /* The disk has not been connected yet, the disk may have been disconnected */
#define ERR_STATUS_ERR			0x83	/* Disk status is wrong, may be connecting or disconnecting the disk */
#define ERR_HUB_PORT_FREE		0x84	/* USB-HUB is already connected but the HUB port is not connected to the disk, and the disk may be disconnected */
#define ERR_MBR_ERROR			0x91	/* The disk's primary boot record is invalid, the disk may not have been partitioned or formatted */
#define ERR_TYPE_ERROR			0x92	/* Disk partition type does not support it, only supports FAT12/FAT16/BigDOS/FAT32, and needs to be repartitioned by disk management tools. */
#define ERR_BPB_ERROR			0xA1	/* The disk has not been formatted yet, or the parameters are incorrect, and it needs to be reformatted by WINDOWS using default parameters. */
#define ERR_TOO_LARGE			0xA2	/* The disk is not formatted normally and has a capacity greater than 4GB, or a capacity greater than 250GB, and needs to be reformatted by WINDOWS using default parameters. */
#define ERR_FAT_ERROR			0xA3	/* The disk file system does not support it, only supports FAT12/FAT16/FAT32, and needs to be reformatted by WINDOWS using default parameters. */
#define ERR_DISK_FULL			0xB1	/* The disk files are too full, there is too little space left or there is no disk consolidation */
#define ERR_FDT_OVER			0xB2	/* There are too many files in the directory and there are no free directory entries. The number of files in the root directory of FAT12/FAT16 should be less than 500, and disk sysing is required */
#define ERR_MISS_DIR			0xB3	/* A subdirectory of the specified path is not found, it may be that the directory name is incorrect */
#define ERR_FILE_CLOSE			0xB4	/* The file has been closed. If you need to use it, you should reopen the file */
#define ERR_OPEN_DIR			0x41	/* The directory with the specified path is opened */
#define ERR_MISS_FILE			0x42	/* The file with the specified path is not found, it may be that the file name is wrong */
#define ERR_FOUND_NAME			0x43	/* Search for a file name that matches the wildcard character, the file name and its full path are in the command buffer. If you need to use it, the file should be opened. */
#endif
/* Code 2XH-3XH is used for communication failure code of USB host mode, and the return of CH375 is imitated by the CHRV3 subroutine. */
/* Code 1XH is used for the operation status code of USB host mode, and the return of CHRV3 subroutine is simulated by CH375. */
#ifndef ERR_USB_CONNECT
#define	ERR_USB_CONNECT_LS		0x13	/* 检测到低速USB设备连接事件 */
#define	ERR_USB_CONNECT			0x15	/* USB device connection event was detected and the disk has been connected */
#define	ERR_USB_DISCON			0x16	/* The USB device disconnect event was detected and the disk was disconnected */
#define	ERR_USB_BUF_OVER		0x17	/* USB传输的数据有误或者数据太多缓冲区溢出 */
#define	ERR_USB_DISK_ERR		0x1F	/* The USB memory operation failed. During initialization, the USB memory may not be supported. During read and write operations, the disk may be damaged or disconnected. */
#define	ERR_USB_TRANSFER		0x20	/* NAK/STALL等更多错误码在0x20~0x2F */
#endif

/* Disk and file status */
#define DISK_UNKNOWN			0x00	/* Not initialized yet, unknown state */
#define DISK_DISCONNECT			0x01	/* 磁盘没有连接或者已经断开 */
#define DISK_CONNECT			0x02	/* 磁盘已经连接,但是尚未初始化或者无法识别该磁盘 */
#define DISK_USB_ADDR			0x04	/* The disk has been assigned a USB device address, but the USB and initialization disk have not been configured yet */
#define DISK_MOUNTED			0x05	/* 磁盘已经初始化成功,但是尚未分析文件系统或者文件系统不支持 */
#define DISK_READY				0x10	/* 已经分析磁盘的文件系统并且能够支持 */
#define DISK_OPEN_ROOT			0x12	/* The root directory has been opened, and the sector mode can only read and write the contents of the directory in units of sectors. It must be turned off after use. Note that the root directory of FAT12/FAT16 is a fixed length */
#define DISK_OPEN_DIR			0x13	/* Subdirectory has been opened, sector mode, and can only read and write the contents of the directory in sectors. */
#define DISK_OPEN_FILE			0x14	/* Files have been opened, sector mode, and data can be read and written in sector units */
#define DISK_OPEN_FILE_B		0x15	/* The file has been opened, byte mode, and data can be read and written in units of bytes */

/* FAT type flag */
#ifndef DISK_FAT16
#define DISK_FS_UNKNOWN			0		/* Unknown file system */
#define DISK_FAT12				1		/* FAT12 file system */
#define DISK_FAT16				2		/* FAT16 file system */
#define DISK_FAT32				3		/* FAT32文件系统 */
#endif

/* FAT数据区中文件目录信息 */
typedef struct _FAT_DIR_INFO {
	UINT8	DIR_Name[11];				/* 00H,文件名,共11字节,不足处填空格 */
	UINT8	DIR_Attr;					/* 0BH, file attributes, refer to the instructions below */
	UINT8	DIR_NTRes;					/* 0CH */
	UINT8	DIR_CrtTimeTenth;			/* 0DH, the time of file creation, counted in 0.1 seconds */
	UINT16	DIR_CrtTime;				/* 0EH, the time of file creation */
	UINT16	DIR_CrtDate;				/* 10H,文件创建的日期 */
	UINT16	DIR_LstAccDate;				/* 12H,最近一次存取操作的日期 */
	UINT16	DIR_FstClusHI;				/* 14H */
	UINT16	DIR_WrtTime;				/* 16H, file modification time, refer to the macro MAKE_FILE_TIME */
	UINT16	DIR_WrtDate;				/* 18H,文件修改日期,参考宏MAKE_FILE_DATA */
	UINT16	DIR_FstClusLO;				/* 1AH */
	UINT32	DIR_FileSize;				/* 1CH,文件长度 */
} FAT_DIR_INFO;							/* 20H */

typedef FAT_DIR_INFO *PX_FAT_DIR_INFO;

/* 文件属性 */
#define ATTR_READ_ONLY			0x01	/* File is read-only attribute */
#define ATTR_HIDDEN				0x02	/* Files are implicit attributes */
#define ATTR_SYSTEM				0x04	/* File is a system attribute */
#define ATTR_VOLUME_ID			0x08	/* Coil label */
#define ATTR_DIRECTORY			0x10	/* 子目录 */
#define ATTR_ARCHIVE			0x20	/* 文件为存档属性 */
#define ATTR_LONG_NAME			( ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID )
/* 文件属性 UINT8 */
/* bit0 bit1 bit2 bit3 bit4 bit5 bit6 bit7 */
/*  只   隐   系   卷   目   存   未定义   */
/* Reading and hiding record files */
/* File time UINT16 */
/* Time = (Hour<<11) + (Minute<<5) + (Second>>1) */
#define MAKE_FILE_TIME( h, m, s )	( (h<<11) + (m<<5) + (s>>1) )	/* Generate file time data for specified time and time */
/* File date UINT16 */
/* Date = ((Year-1980)<<9) + (Month<<5) + Day */
#define MAKE_FILE_DATE( y, m, d )	( ((y-1980)<<9) + (m<<5) + d )	/* Generate file date data for the specified year, month and date */

/* file name */
#define PATH_WILDCARD_CHAR		0x2A	/* 路径名的通配符 '*' */
#define PATH_SEPAR_CHAR1		0x5C	/* The delimiter of the pathname '\' */
#define PATH_SEPAR_CHAR2		0x2F	/* The delimiter of the pathname '/' */
#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN			64		/* Maximum path length, including all slash separators and decimal spacers and path ending character 00H */
#endif

/* External command parameters */
typedef union _CMD_PARAM {
	struct {
		UINT8	mBuffer[ MAX_PATH_LEN ];
	} Other;
	struct {
		UINT32	mTotalSector;			/* Return: The total number of sectors of the current logical disk */
		UINT32	mFreeSector;			/* Return: The number of remaining sectors of the current logical disk */
		UINT32	mSaveValue;
	} Query;							/* CMD_DiskQuery, query disk information */
	struct {
		UINT8	mPathName[ MAX_PATH_LEN ];	/* Input parameters: Path: [Disk letter, colon, slash, directory name or file name and extension..., ending character 00H], where the drive letter and colon can be omitted, such as "C:\DIR1.EXT\DIR2\FILENAME.EXT",00H */
	} Open;								/* CMD_FileOpen, open file */
//	struct {
// UINT8 mPathName[ MAX_PATH_LEN ]; /* Input parameters: Path: [Disk letter, colon, slash, directory name or file name and extension (including wildcard character *)..., ending character 00H], where the drive letter and colon can be omitted, such as "C:\DIR1.EXT\DIR2\FILE*",00H */
// } Open; /* CMD_FileOpen, enumerate files, if the highest bit of CHRV3vFileSize is 1, each call xFileNameEnumer, if it is 0, return the file name with the specified sequence number */
	struct {
		UINT8	mUpdateLen;				/* Input parameters: Whether to allow update length: 0 prohibited, 1 permitted */
	} Close;							/* CMD_FileClose, close the current file */
	struct {
		UINT8	mPathName[ MAX_PATH_LEN ];	/* Input parameters: Path: [Disk letter, colon, slash, directory name or file name and extension..., ending character 00H], where the drive letter and colon can be omitted, such as "C:\DIR1.EXT\DIR2\FILENAME.EXT",00H */
	} Create;							/* CMD_FileCreate, 新建文件并打开,如果文件已经存在则先删除后再新建 */
	struct {
		UINT8	mPathName[ MAX_PATH_LEN ];	/* Input parameters: Path: [Disk letter, colon, slash, directory name or file name and extension..., ending character 00H], where the drive letter and colon can be omitted, such as "C:\DIR1.EXT\DIR2\FILENAME.EXT",00H */
	} Erase;							/* CMD_FileErase, 删除文件并关闭 */
	struct {
		UINT32	mFileSize;				/* 输入参数: 新的文件长度,为0FFFFFFFFH则不修改, 返回: 原长度 */
		UINT16	mFileDate;				/* Input parameters: If the new file date is 0FFFFH, it will not be modified. Return to: Original date */
		UINT16	mFileTime;				/* 输入参数: 新的文件时间,为0FFFFH则不修改, 返回: 原时间 */
		UINT8	mFileAttr;				/* Input parameters: New file attribute, if it is 0FFH, it will not be modified. Return to: Original attribute */
	} Modify;							/* CMD_FileQuery, 查询当前文件的信息; CMD_FileModify, 查询或者修改当前文件的信息 */
	struct {
		UINT32	mSaveCurrClus;
		UINT32	mSaveLastClus;
	} Alloc;							/* CMD_FileAlloc, adjusts the disk space allocated to the file according to the file length */
	struct {
		UINT32	mSectorOffset;			/* Input parameters: sector offset, 0 will move to the file header, 0FFFFFFH will move to the end of the file, Return: The current file refers to the corresponding absolute linear sector number, 0FFFFFFFH will reach the end of the file */
		UINT32	mLastOffset;
	} Locate;							/* CMD_FileLocate, move the current file pointer */
	struct {
		UINT8	mSectorCount;			/* Input parameters: Read the number of sectors, return: Actual read the number of sectors */
		UINT8	mActCnt;
		UINT8	mLbaCount;
		UINT8	mRemainCnt;
		PUINT8	mDataBuffer;			/* 输入参数: 缓冲区起始地址, 返回: 缓冲区当前地址 */
		UINT32	mLbaStart;
	} Read;								/* CMD_FileRead, read data from the current file */
	struct {
		UINT8	mSectorCount;			/* Input parameters: Number of write sectors, return: The actual number of write sectors */
		UINT8	mActCnt;
		UINT8	mLbaCount;
		UINT8	mAllocCnt;
		PUINT8	mDataBuffer;			/* Enter parameters: Buffer start address, return: Buffer current address */
		UINT32	mLbaStart;
		UINT32	mSaveValue;
	} Write;							/* CMD_FileWrite, 向当前文件写入数据 */
	struct {
		UINT32	mDiskSizeSec;			/* Return: The total number of sectors of the entire physical disk, only returned on the first call */
	} DiskReady;						/* CMD_DiskReady, 查询磁盘就绪 */
	struct {
		UINT32	mByteOffset;			/* 输入参数: 以字节为单位的偏移量, 以字节为单位的文件指针, 返回: 当前文件指针对应的绝对线性扇区号, 0FFFFFFFFH则已到文件尾 */
		UINT32	mLastOffset;
	} ByteLocate;						/* CMD_ByteLocate, 以字节为单位移动当前文件指针 */
	struct {
		UINT16	mByteCount;				/* Input parameters: The number of bytes to be read, return: The number of bytes actually read */
		PUINT8	mByteBuffer;			/* 输入参数: 指向存放读出数据块的缓冲区 */
		UINT16	mActCnt;
	} ByteRead;							/* CMD_ByteRead, reads data blocks from the current file in units of bytes */
	struct {
		UINT16	mByteCount;				/* 输入参数: 准备写入的字节数, 返回: 实际写入的字节数 */
		PUINT8	mByteBuffer;			/* Input parameters: Point to the buffer where the read data block is stored */
		UINT16	mActCnt;
	} ByteWrite;						/* CMD_ByteWrite, 以字节为单位向当前文件写入数据块 */
	struct {
		UINT8	mSaveVariable;			/* 输入参数: 为0则恢复单个U盘的变量,为0x80则恢复多个U盘的变量,其它值则备份/保存变量 */
		UINT8	mReserved[3];
		PUINT8	mBuffer;				/* Input parameters: Backup buffer of variables pointing to subroutine library, with a length of no less than 80 bytes */
	} SaveVariable;						/* CMD_SaveVariable, Backup/Save/Restore variables of the subroutine library */
} CMD_PARAM;

typedef CMD_PARAM CMD_PARAM_I;
//typedef CMD_PARAM *P_CMD_PARAM;

/* SCSI command code */
#ifndef SPC_CMD_INQUIRY
#define SPC_CMD_INQUIRY			0x12
#define SPC_CMD_READ_CAPACITY	0x25
#define SPC_CMD_READ10			0x28
#define SPC_CMD_WRITE10			0x2A
#define SPC_CMD_TEST_READY		0x00
#define SPC_CMD_REQUEST_SENSE	0x03
#define SPC_CMD_MODESENSE6		0x1A
#define SPC_CMD_MODESENSE10		0x5A
#define SPC_CMD_START_STOP		0x1B
#endif

/* FILE: CHRV3UFI.C */

#define EN_DISK_WRITE			1

#ifndef DISK_BASE_BUF_LEN
#define DISK_BASE_BUF_LEN		512		/* The default disk data buffer size is 512 bytes. It is recommended to select USB disks with 2048 or even 4096 to support certain large sectors. If 0 is, the buffer is prohibited from being defined in the .H file and specified by the application in pDISK_BASE_BUF. */
#endif

/* 子程序库中提供的变量 */
extern	UINT8V	CHRV3IntStatus;				/* CHRV3操作的中断状态 */
extern	UINT8V	CHRV3DiskStatus;			/* Disk and file status */
extern	UINT8	CHRV3vDiskFat;				/* FAT flag of the logical disk: 1=FAT12, 2=FAT16, 3=FAT32 */
extern	UINT8	CHRV3vSecPerClus;			/* Number of sectors per cluster of logical disks */
extern	UINT8	CHRV3vSectorSizeB;			/* log2(CHRV3vSectorSize) */
extern	UINT32	CHRV3vStartLba;				/* The start absolute sector number of the logical disk LBA */
extern	UINT32	CHRV3vDiskRoot;				/* For FAT16 disks, the number of sectors occupied by the root directory, for FAT32 disks, the number of the root directory starts cluster */
extern	UINT32	CHRV3vDataStart;			/* 逻辑盘的数据区域的起始LBA */
extern	UINT32	CHRV3vStartCluster;			/* The starting cluster number of the current file or directory */
extern	UINT32	CHRV3vFileSize;				/* The length of the current file */
extern	UINT32	CHRV3vCurrentOffset;		/* Current file pointer, byte offset of current read and write position */
extern	UINT32	CHRV3vFdtLba;				/* 当前FDT所在的LBA地址 */
extern	UINT32	CHRV3vLbaCurrent;			/* 当前读写的磁盘起始LBA地址 */
extern	UINT16	CHRV3vFdtOffset;			/* The offset address of the current FDT in the sector */
extern	UINT16	CHRV3vSectorSize;			/* Disk sector size */
extern	UINT8	CHRV3vCurrentLun;			/* The current logical unit number of the disk */
extern	BOOL	CHRV3vSubClassIs6;			/* USB存储类设备的子类为6,0则非6 */
extern	PUINT8	pDISK_BASE_BUF;		/* 指向外部RAM的磁盘数据缓冲区,缓冲区长度不小于CHRV3vSectorSize,由应用程序初始化 */
extern	PUINT8	pDISK_FAT_BUF;		/* The disk FAT data buffer pointing to external RAM, the buffer length is not less than CHRV3vSectorSize, initialized by the application */
extern	UINT16	CHRV3vPacketSize;			/* USB存储类设备的最大包长度:64@FS,512@HS/SS,由应用程序初始化 */
extern	PUINT32	pTX_DMA_A_REG;				/* Point to the send DMA address register, initialized by the application */
extern	PUINT32	pRX_DMA_A_REG;				/* Point to the receiving DMA address register, initialized by the application */
extern	PUINT16	pTX_LEN_REG;				/* Point to the send length register, initialized by the application */
extern	PUINT16	pRX_LEN_REG;				/* Point to the receive length register, initialized by the application */

extern	CMD_PARAM_I	mCmdParam;				/* 命令参数 */

extern	UINT8	RxBuffer[ MAX_PACKET_SIZE ];  // IN, must even address
extern	UINT8	TxBuffer[ MAX_PACKET_SIZE ];  // OUT, must even address

//#define		PXUDISK_BOC_CBW	PUDISK_BOC_CBW
//#define		PXUDISK_BOC_CSW	PUDISK_BOC_CSW

#ifndef	pSetupReq
#define	pSetupReq	((PUSB_SETUP_REQ)TxBuffer)
#endif
#ifndef	pCBW
#define	pCBW		((PXUDISK_BOC_CBW)TxBuffer)
#define	pCSW		((PXUDISK_BOC_CSW)RxBuffer)
#endif
#ifndef	pBOC_buf
#define	pBOC_buf	(TxBuffer+((USB_BO_CBW_SIZE+4)&0xFE))
#endif

#if		DISK_BASE_BUF_LEN > 0
//extern	UINT8	DISK_BASE_BUF[ DISK_BASE_BUF_LEN ];	/* 外部RAM的磁盘数据缓冲区,缓冲区长度为一个扇区的长度 */
#endif
extern	UINT8	CHRV3ReadSector( UINT8 SectCount, PUINT8 DataBuf );	/* Read data from multiple sectors into the buffer from disk */
#ifdef	EN_DISK_WRITE
extern	UINT8	CHRV3WriteSector( UINT8 SectCount, PUINT8 DataBuf );	/* 将缓冲区中的多个扇区的数据块写入磁盘 */
#endif

extern	UINT8	CHRV3DiskConnect( void );	/* 检查磁盘是否连接并更新磁盘状态 */
extern	void	xFileNameEnumer( void );	/* 调用外部定义的子程序,文件名枚举回调子程序 */

extern	UINT8	CHRV3LibInit( void );		/* 初始化CHRV3程序库,操作成功返回0 */

/* Subroutines provided in the subroutine library */
/* In the following subroutines, both the file operation subroutine CHRV3File* and the disk query subroutine CHRV3DiskQuery may use the disk data buffer pDISK_BASE_BUF,
And it is possible that disk information is saved in pDISK_BASE_BUF, so it is necessary to ensure that pDISK_BASE_BUF is not used for other purposes.
If there is less RAM, to use pDISK_BASE_BUF temporarily for other purposes, then after temporary use, CHRV3DirtyBuffer must be called to clear the disk buffer */
extern	UINT8	CHRV3GetVer( void );		/* Get the version number of the current subroutine library */
extern	void	CHRV3DirtyBuffer( void );	/* Clear the disk buffer */
extern	UINT8	CHRV3BulkOnlyCmd( PUINT8 DataBuf );	/* Execute commands based on BulkOnly protocol */
extern	UINT8	CHRV3DiskReady( void );		/* 查询磁盘是否准备好 */
extern	UINT8	CHRV3AnalyzeError( UINT8 iMode );	/* USB operation failed analysis CHRV3IntStatus returns error status */
extern	UINT8	CHRV3FileOpen( void );		/* Open a file or enumerate a file */
extern	UINT8	CHRV3FileClose( void );		/* 关闭当前文件 */
#ifdef	EN_DISK_WRITE
extern	UINT8	CHRV3FileErase( void );		/* 删除文件并关闭 */
extern	UINT8	CHRV3FileCreate( void );	/* Create a new file and open it. If the file already exists, delete it first and then create it. */
extern	UINT8	CHRV3FileAlloc( void );		/* 根据文件长度调整为文件分配的磁盘空间 */
#endif
extern	UINT8	CHRV3FileModify( void );	/* 查询或者修改当前文件的信息 */
extern	UINT8	CHRV3FileQuery( void );		/* Query the information of the current file */
extern	UINT8	CHRV3FileLocate( void );	/* Move the current file pointer */
extern	UINT8	CHRV3FileRead( void );		/* 从当前文件读取数据到指定缓冲区 */
#ifdef	EN_DISK_WRITE
extern	UINT8	CHRV3FileWrite( void );		/* Write data from the specified buffer to the current file */
#endif
extern	UINT8	CHRV3ByteLocate( void );	/* Move the current file pointer in bytes */
extern	UINT8	CHRV3ByteRead( void );		/* Read data blocks from the current location in units of bytes */
#ifdef	EN_DISK_WRITE
extern	UINT8	CHRV3ByteWrite( void );		/* 以字节为单位向当前位置写入数据块 */
#endif
extern	UINT8	CHRV3DiskQuery( void );		/* 查询磁盘信息 */
extern	void	CHRV3SaveVariable( void );	/* Backup/save/restore variables of the subroutine library, used to switch the subroutine library between multiple chips or USB flash drives */

extern	void	mDelayuS( UINT16 n );		// Delay in uS units
extern	void	mDelaymS( UINT16 n );		// Delay in mS
extern	UINT8	USBHostTransact( UINT8 endp_pid, UINT8 tog, UINT32 timeout );	// CHRV3 transmits transactions, input destination endpoint address/PID token, synchronization flag, NAK retry time, return 0 successful, timeout/error retry
extern	UINT8	HostCtrlTransfer( PUINT8 DataBuf, PUINT8 RetLen );	// Execute control transmission, 8-byte request code in pSetupReq, DataBuf is an optional sending and receiving buffer, and the actual sending and receiving length is returned in the variable pointed to by ReqLen.
//extern	void	CopySetupReqPkg( PCCHAR pReqPkt );  // 复制控制传输的请求包
//extern	UINT8	CtrlGetDeviceDescrTB( void );  // 获取设备描述符,返回在TxBuffer中
extern	UINT8	CtrlGetConfigDescrTB( void );  // 获取配置描述符,返回在TxBuffer中
//extern	UINT8	CtrlSetUsbAddress( UINT8 addr );  // 设置USB设备地址
extern	UINT8	CtrlSetUsbConfig( UINT8 cfg );  // Set up USB device configuration
extern	UINT8	CtrlClearEndpStall( UINT8 endp );  // Clear endpoint STALL
#ifndef	FOR_ROOT_UDISK_ONLY
// extern UINT8 CtrlGetHubDescr( void ); // Get the HUB descriptor and return it in TxBuffer
extern	UINT8	HubGetPortStatus( UINT8 HubPortIndex );  // 查询HUB端口状态,返回在TxBuffer中
//extern	UINT8	HubSetPortFeature( UINT8 HubPortIndex, UINT8 FeatureSelt );  // 设置HUB端口特性
extern	UINT8	HubClearPortFeature( UINT8 HubPortIndex, UINT8 FeatureSelt );  // Clear HUB port features
#endif

#ifdef __cplusplus
}
#endif

#endif
