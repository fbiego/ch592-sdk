/* ********************************* (C) COPYRIGHT ***************************
* File Name: EXAM10.c
* Author: WCH
* Version: V1.0
* Date: 2020/08/11
* Description:
* C language USB disk file creation, modify file attributes, delete files, etc.
* Support: FAT12/FAT16/FAT32
* Note that CHRV3UFI.LIB/USBHOST.C/DEBUG.C is included
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */

/* * Do not use the U disk file system library, need to modify DISK_LIB_ENABLE=0 in precompilation of engineering properties. */
/* * When the USB drive is mounted under USBhub, you need to modify DISK_WITHOUT_USB_HUB=0 */

#include "CH59x_common.h"
#include "CHRV3UFI.H"

__attribute__((aligned(4))) uint8_t RxBuffer[MAX_PACKET_SIZE]; // IN, must even address
__attribute__((aligned(4))) uint8_t TxBuffer[MAX_PACKET_SIZE]; // OUT, must even address

uint8_t buf[100]; // The length can be specified by the application itself

/* ***************************************************************************
* @fn mStopIfError
*
* @brief Check operation status, if it is wrong, the error code will be displayed and the operation will be shut down
*
* @param iError - Error code
*
* @return none */
void mStopIfError(uint8_t iError)
{
    if(iError == ERR_SUCCESS)
    {
        return; /* Operation is successful */
    }
    PRINT("Error: %02X\n", (uint16_t)iError); /* Display error */
    /* After encountering an error, the error code and CHRV3DiskStatus status should be analyzed. For example, call CHRV3DiskReady to check whether the current USB disk is connected. If the USB disk is disconnected, wait for the USB disk to be plugged in before operation.
Recommended steps to deal with errors:
1. Call CHRV3DiskReady once, and continue to operate successfully, such as Open, Read/Write, etc.
2. If CHRV3DiskReady fails, then the operation will be forced from scratch (waiting for the USB flash drive connection, CH554DiskReady, etc.) */
    while(1)
    {
    }
}

/* ***************************************************************************
* @fn main
*
* @brief main function
*
* @return none */
int main()
{
    uint8_t s, c, i;

    SetSysClock(CLK_SOURCE_PLL_60MHz);

    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
    PRINT("Start @ChipID=%02X \n", R8_CHIP_ID);

    pHOST_RX_RAM_Addr = RxBuffer;
    pHOST_TX_RAM_Addr = TxBuffer;
    USB_HostInit();
    CHRV3LibInit(); // Initialize the USB disk library to support USB disk files

    FoundNewDev = 0;
    while(1)
    {
        s = ERR_SUCCESS;
        if(R8_USB_INT_FG & RB_UIF_DETECT) // If there is a USB host detection interrupt, it will be handled
        {
            R8_USB_INT_FG = RB_UIF_DETECT; // Clear connection interrupt flag
            s = AnalyzeRootHub();          // Analyze ROOT-HUB status
            if(s == ERR_USB_CONNECT)
                FoundNewDev = 1;
        }

        if(FoundNewDev || s == ERR_USB_CONNECT) // There is a new USB device plugged in
        {
            FoundNewDev = 0;
            mDelaymS(200);        // Since the USB device has just been plugged in, it is not stable yet, so wait for the USB device to be hundreds of milliseconds to eliminate plug-in and unplug jitter
            s = InitRootDevice(); // Initialize USB device
            if(s == ERR_SUCCESS)
            {
                // USB drive operation process: USB bus reset, USB drive connection, obtain device descriptors and set USB address, optional obtain configuration descriptors, and then arrive here, and the CHRV3 subroutine library continues to complete the subsequent work.
                CHRV3DiskStatus = DISK_USB_ADDR;
                for(i = 0; i != 10; i++)
                {
                    PRINT("Wait DiskReady\n");
                    s = CHRV3DiskReady(); // Wait for the USB drive to be ready
                    if(s == ERR_SUCCESS)
                    {
                        break;
                    }
                    else
                    {
                        PRINT("%02x\n", (uint16_t)s);
                    }
                    mDelaymS(50);
                }

                if(CHRV3DiskStatus >= DISK_MOUNTED)
                {
                    // Create a file demo
                    PRINT("Create\n");
                    strcpy((uint8_t *)mCmdParam.Create.mPathName, "/NEWFILE.TXT"); /* New file name, in the root directory, Chinese file name */
                    s = CHRV3FileCreate();                                         /* Create a new file and open it. If the file already exists, delete it first and then create it. */
                    mStopIfError(s);
                    PRINT("ByteWrite\n");
                    // In fact, it should be determined whether the length of the write data and the length of the definition buffer match. If it is greater than the length of the buffer, it needs to be written multiple times.
                    i = sprintf((uint8_t *)buf, "Note: \xd\xaU,RV3\xd\xa"); /* Demo */
                    for(c = 0; c < 10; c++)
                    {
                        mCmdParam.ByteWrite.mByteCount = i;    /* Specify the number of bytes written this time */
                        mCmdParam.ByteWrite.mByteBuffer = buf; /* Point to the buffer */
                        s = CHRV3ByteWrite();                  /* Write data to a file in bytes */
                        mStopIfError(s);
                        PRINT(" %02X\n", (uint16_t)c);
                    }
                    // Demonstrate modifying file properties
                    PRINT("Modify\n");
                    mCmdParam.Modify.mFileAttr = 0xff;                        // Input parameters: New file attribute, if it is 0FFH, it will not be modified.
                    mCmdParam.Modify.mFileTime = 0xffff;                      // Input parameters: If the new file time is 0FFFFH, it will not be modified. The default time generated by using the new file
                    mCmdParam.Modify.mFileDate = MAKE_FILE_DATE(2015, 5, 18); // Input parameters: New file date: 2015.05.18
                    mCmdParam.Modify.mFileSize = 0xffffffff;                  // Input parameters: New file length, writing files in bytes should be automatically updated when the library closes the file, so it will not be modified here
                    i = CHRV3FileModify();                                    // Modify the information of the current file, date of modification
                    mStopIfError(i);
                    PRINT("Close\n");
                    mCmdParam.Close.mUpdateLen = 1; /* Automatically calculate file length and write files in bytes. It is recommended that the program library close the file so that the file length can be automatically updated. */
                    i = CHRV3FileClose();
                    mStopIfError(i);

// strcpy((uint8_t *)mCmdParam.Create.mPathName, "/NEWFILE.TXT"); /* New file name, in the root directory, Chinese file name */
// s = CHRV3FileOpen(); /* Create a new file and open it. If the file already exists, delete it first and then create it */
//                    mStopIfError(s);

                    /* Delete a file */
                    PRINT("Erase\n");
                    strcpy((uint8_t *)mCmdParam.Create.mPathName, "/OLD"); // The file name that will be deleted is in the root directory
                    i = CHRV3FileErase();                                  // Delete the file and close it
                    if(i != ERR_SUCCESS)
                        PRINT("Error: %02X\n", (uint16_t)i); // Display error
                }
            }
        }
        mDelaymS(100);  // Simulate a microcontroller to do other things
        SetUsbSpeed(1); // Default is full speed
    }
}
