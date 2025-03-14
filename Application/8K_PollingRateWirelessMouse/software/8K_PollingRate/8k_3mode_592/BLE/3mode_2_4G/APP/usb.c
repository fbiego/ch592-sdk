/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* The header file contains */
#include <rf.h>
#include "rf_device.h"
#include "CH59x_common.h"
#include "usb.h"
#include "ota.h"
#include "trans.h"
#include "mouse.h"
#include "peripheral.h"
/*********************************************************************
 * GLOBAL TYPEDEFS
 */
// The maximum number of interfaces supported
#define USB_INTERFACE_MAX_NUM       4
// Maximum value of interface number
#define USB_INTERFACE_MAX_INDEX     3

const uint8_t LangID_StrDescr[ 4 ] =
{
    0x04,
    0x03,                                                                       /* bDescriptorType */
    0x09,
    0x04
};

// Manufacturer information
uint8_t MyManuInfo[64] = {0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0};

// Product Information
uint8_t MyProdInfo[64] = {0x0D+11, 0x03,
    '2', 0,
    '.', 0,
    '4', 0,
    'G', 0,
    ' ', 0,
    'D', 0,
    'o', 0,
    'n', 0,
    'g', 0,
    'l', 0,
    'e', 0
};

// Serial number
uint8_t MySerialNum[64] = {0x0D+11, 0x03,
    '2', 0,
    '.', 0,
    '4', 0,
    'G', 0,
    ' ', 0,
    'D', 0,
    'o', 0,
    'n', 0,
    'g', 0,
    'l', 0,
    'e', 0
};


// ----Report descriptor Mouse---------------------------------------------------------------------------------------------------------------------
const uint8_t HID_ReportDescriptorMouse[]=
{
        0x05,0x01,                  //81    GLOBAL_USAGE_PAGE(Generic Desktop Controls)
        0x09,0x02,                  //83    LOCAL_USAGE(Mouse)
        0xA1,0x01,                  //85    MAIN_COLLECTION(Applicatior)
            0x09,0x01,                  //89    LOCAL_USAGE(Pointer)
            0xA1,0x00,                  //91    MAIN_COLLECTION(Physical)
                0x05,0x09,                  //93    GLOBAL_USAGE_PAGE(Button)
                0x19,0x01,                  //95    LOCAL_USAGE_MINIMUM(1)
                0x29,0x05,                  //97    LOCAL_USAGE_MAXIMUM(5)
                0x15,0x00,                  //99    GLOBAL_LOGICAL_MINIMUM(0)
                0x25,0x01,                  //101   GLOBAL_LOCAL_MAXIMUM(1)
                0x95,0x05,                  //103   GLOBAL_REPORT_COUNT(5)
                0x75,0x01,                  //105   GLOBAL_REPORT_SIZE(1)
                0x81,0x02,                  //107   MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 18.5
                0x95,0x01,                  //109   GLOBAL_REPORT_COUNT(1)
                0x75,0x03,                  //111   GLOBAL_REPORT_SIZE(3)
                0x81,0x01,                  //113   MAIN_INPUT(const array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )   Input 19.0
                0x05,0x01,                  //115   GLOBAL_USAGE_PAGE(Generic Desktop Controls)
                0x09,0x30,                  //117   LOCAL_USAGE(X)
                0x09,0x31,                  //119   LOCAL_USAGE(Y)
    //            0x16, 0x00, 0x80,  //     Logical Minimum (-32768)
    //            0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    //            0x75,0x10,                  //127   GLOBAL_REPORT_SIZE(16)
                0x15, 0x80,  //     Logical Minimum (-128)
                0x25, 0x7F,  //     Logical Maximum (128)
                0x75,0x08,                  //127   GLOBAL_REPORT_SIZE(16)
                0x95,0x02,                  //129   GLOBAL_REPORT_COUNT(2)
                0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
                0x09,0x38,                  //121   LOCAL_USAGE(Wheel)
                0x15,0x81,                  //123   GLOBAL_LOGICAL_MINIMUM(-127)
                0x25,0x7F,                  //125   GLOBAL_LOCAL_MAXIMUM(127)
                0x75,0x08,                  //127   GLOBAL_REPORT_SIZE(8)
                0x95,0x01,                  //129   GLOBAL_REPORT_COUNT(1)
                0x81,0x06,                  //131   MAIN_INPUT(data var relative NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 22.0
            0xC0,                       //133   MAIN_COLLECTION_END
        0xC0,

};

// ----Report descriptor Other----------------------------------------------------------------------------------------------------------------------
const uint8_t HID_ReportDescriptorOther[]=
{
    // Keyboard report:
//      0x05, 0x01,     // Usage Pg (Generic Desktop)
//      0x09, 0x06,     // Usage (Keyboard)
//      0xA1, 0x01,     // Collection: (Application)
//      0x85, 0x01,     //   ReportID(1)
//                      //
//      0x05, 0x07,     // Usage Pg (Key Codes)
//      0x19, 0xE0,     // Usage Min (224)
//      0x29, 0xE7,     // Usage Max (231)
//      0x15, 0x00,     // Log Min (0)
//      0x25, 0x01,     // Log Max (1)
//                      //
//                      // Modifier byte
//      0x75, 0x01,     // Report Size (1)
//      0x95, 0x08,     // Report Count (8)
//      0x81, 0x02,     // Input: (Data, Variable, Absolute)
//                      //
//                      // Reserved byte
//      0x95, 0x01,     // Report Count (1)
//      0x75, 0x08,     // Report Size (8)
//      0x81, 0x01,     // Input: (Constant)
//                      //
//                      // LED report
//      0x95, 0x05,     // Report Count (5)
//      0x75, 0x01,     // Report Size (1)
//      0x05, 0x08,     // Usage Pg (LEDs)
//      0x19, 0x01,     // Usage Min (1)
//      0x29, 0x05,     // Usage Max (5)
//      0x91, 0x02,     // Output: (Data, Variable, Absolute)
//                      //
//                      // LED report padding
//      0x95, 0x01,     // Report Count (1)
//      0x75, 0x03,     // Report Size (3)
//      0x91, 0x01,     // Output: (Constant)
//                      //
//                      // Key arrays (6 bytes)
//      0x95, 0x06,     // Report Count (6)
//      0x75, 0x08,     // Report Size (8)
//      0x15, 0x00,     // Log Min (0)
//      0x25, 0x65,     // Log Max (101)
//      0x05, 0x07,     // Usage Pg (Key Codes)
//      0x19, 0x00,     // Usage Min (0)
//      0x29, 0x65,     // Usage Max (101)
//      0x81, 0x00,     // Input: (Data, Array)
//                      //
//      0xC0 ,           // End Collection

        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x06,        // Usage (Keyboard)
        0xA1, 0x01,        // Collection (Application)
        0x85, 0x01,        //   Report ID (1)
        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
        0x19, 0xE0,        //   Usage Minimum (0xE0)
        0x29, 0xE7,        //   Usage Maximum (0xE7)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x08,        //   Report Count (8)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x08,        //   Report Size (8)
        0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x05,        //   Report Count (5)
        0x75, 0x01,        //   Report Size (1)
        0x05, 0x08,        //   Usage Page (LEDs)
        0x19, 0x01,        //   Usage Minimum (Num Lock)
        0x29, 0x05,        //   Usage Maximum (Kana)
        0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x03,        //   Report Size (3)
        0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x06,        //   Report Count (6)
        0x75, 0x08,        //   Report Size (8)
        0x15, 0x00,        //   Logical Minimum (0)
        0x26, 0xFF, 0x00,  //   Logical Maximum (255)
        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
        0x19, 0x00,        //   Usage Minimum (0x00)
        0x29, 0xFF,        //   Usage Maximum (0xFF)
        0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x0C,        //   Usage Page (Consumer)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x01,        //   Report Count (1)
        0x09, 0xB8,        //   Usage (Eject)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0xFF,        //   Usage Page (Reserved 0xFF)
        0x09, 0x03,        //   Usage (0x03)
        0x75, 0x07,        //   Report Size (7)
        0x95, 0x01,        //   Report Count (1)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              // End Collection

        // 90 bytes


    // Full keyboard:
    0x05,0x01,                  //0     GLOBAL_USAGE_PAGE(Generic Desktop Controls)
    0x09,0x06,                  //2     LOCAL_USAGE(Keyboard)
    0xA1,0x01,                  //4     MAIN_COLLECTION(Applicatior)
    0x85,0x02,                  //6     GLOBAL_REPORT_ID(2)
    0x05,0x07,                  //8     GLOBAL_USAGE_PAGE(Keyboard/Keypad)
    0x19,0x04,                  //10    LOCAL_USAGE_MINIMUM(4)
    0x29,0x70,                  //12    LOCAL_USAGE_MAXIMUM(112)
    0x15,0x00,                  //14    GLOBAL_LOGICAL_MINIMUM(0)
    0x25,0x01,                  //16    GLOBAL_LOCAL_MAXIMUM(1)
    0x75,0x01,                  //18    GLOBAL_REPORT_SIZE(1)
    0x95,0x78,                  //20    GLOBAL_REPORT_COUNT(120)
    0x81,0x02,                  //22    MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 15.0
    0xC0,                       //24    MAIN_COLLECTION_END


    // Multimedia control:
    0x05,0x0C,                  //0     GLOBAL_USAGE_PAGE(Consumer)
    0x09,0x01,                  //2     LOCAL_USAGE(    Consumer Control    )
    0xA1,0x01,                  //4     MAIN_COLLECTION(Applicatior)
    0x85,0x03,                  //6     GLOBAL_REPORT_ID(3)
    0x15,0x00,                  //8     GLOBAL_LOGICAL_MINIMUM(0)
    0x26,0xFF,0x1F,             //10    GLOBAL_LOCAL_MAXIMUM(8191/8191)
    0x19,0x00,                  //13    LOCAL_USAGE_MINIMUM(0)
    0x2A,0xFF,0x1F,             //15    LOCAL_USAGE_MAXIMUM(8191)
    0x75,0x10,                  //18    GLOBAL_REPORT_SIZE(16)
    0x95,0x01,                  //20    GLOBAL_REPORT_COUNT(1)
    0x81,0x00,                  //22    MAIN_INPUT(data array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )    Input 2.0
    0xC0,                       //24    MAIN_COLLECTION_END


    // System control:
    0x05,0x01,                  //0     GLOBAL_USAGE_PAGE(Generic Desktop Controls)
    0x09,0x80,                  //2     LOCAL_USAGE()
    0xA1,0x01,                  //4     MAIN_COLLECTION(Applicatior)
    0x85,0x04,                  //6     GLOBAL_REPORT_ID(4)
    0x05,0x01,                  //8     GLOBAL_USAGE_PAGE(Generic Desktop Controls)
    0x19,0x81,                  //10    LOCAL_USAGE_MINIMUM(129)
    0x29,0x83,                  //12    LOCAL_USAGE_MAXIMUM(131)
    0x15,0x00,                  //14    GLOBAL_LOGICAL_MINIMUM(0)
    0x25,0x01,                  //16    GLOBAL_LOCAL_MAXIMUM(1)
    0x95,0x03,                  //18    GLOBAL_REPORT_COUNT(3)
    0x75,0x01,                  //20    GLOBAL_REPORT_SIZE(1)
    0x81,0x02,                  //22    MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 0.3
    0x95,0x01,                  //24    GLOBAL_REPORT_COUNT(1)
    0x75,0x05,                  //26    GLOBAL_REPORT_SIZE(5)
    0x81,0x01,                  //28    MAIN_INPUT(const array absolute NoWrap linear PreferredState NoNullPosition NonVolatile )   Input 1.0
    0xC0,                       //30    MAIN_COLLECTION_END
};

// ----Report descriptor Manufacturer------------------------------------------------------------------------------------------------------------------
const uint8_t HID_ReportDescriptorManufacturer[]=
{
    0x06,0x13,0xFF,             //0     GLOBAL_USAGE_PAGE(Reserved or Other)
    0x09,0x01,                  //3     LOCAL_USAGE()
    0xA1,0x01,                  //5     MAIN_COLLECTION(Applicatior)
    0x15,0x00,                  //7     GLOBAL_LOGICAL_MINIMUM(0)
    0x26,0xFF,0x00,             //9     GLOBAL_LOCAL_MAXIMUM(255/255)
    0x75,0x08,                  //12    GLOBAL_REPORT_SIZE(8)
    0x95,0x40,                  //14    GLOBAL_REPORT_COUNT(64)
    0x09,0x02,                  //16    LOCAL_USAGE()
    0x81,0x02,                  //18    MAIN_INPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )  Input 64.0
    0x09,0x03,                  //20    LOCAL_USAGE()
    0x91,0x02,                  //22    MAIN_OUTPUT(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile ) Output 64.0
    0x0A,0x00,0xFF,             //24    LOCAL_USAGE()
    0xB1,0x02,                  //27    MAIN_FEATURE(data var absolute NoWrap linear PreferredState NoNullPosition NonVolatile )    Feature 64.0
    0xC0,                       //29    MAIN_COLLECTION_END
};

// ----Report descriptor IAP---------------------------------------------------------------------------------------------------------------------
const uint8_t HID_ReportDescriptorIAP[]=
{

    0x06, 0x60, 0xFF,  // Usage Page (Vendor Defined 0xFF60)
    0x09, 0x61,        // Usage (0x61)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x62,        //   Usage (0x62)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x95, 0x40,        //   Report Count (63)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x63,        //   Usage (0x63)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x95, 0x40,        //   Report Count (63)
    0x75, 0x08,        //   Report Size (8)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
};

const uint8_t HID_ReportDescSizeMouse = sizeof(HID_ReportDescriptorMouse);
const uint8_t HID_ReportDescSizeOther = sizeof(HID_ReportDescriptorOther);
const uint8_t HID_ReportDescSizeManufacturer = sizeof(HID_ReportDescriptorManufacturer);
const uint8_t HID_ReportDescSizeIAP = sizeof(HID_ReportDescriptorIAP);

// Device descriptor
uint8_t MyDevDescr[] = {
    0x12,   /* bLength */
    0x01,   /* bDescriptorType USB_DEVICE_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0110),  /* bcdUSB */
    0x00,   /* bDeviceClass */
    0x00,   /* bDeviceSubClass */
    0x00,   /* bDeviceProtocol */
    DevEP0SIZE, /* bMaxPacketSize0 */
    USB_WBVAL(USB_VID), /* idVendor */
    USB_WBVAL(USB_PID), /* idProduct */
    USB_WBVAL(USB_REV), /* bcdDevice */
    0x01,   /* iManufacturer USB_DEVICE_STRING_RESERVED*/
    0x02,   /* iProduct USB_DEVICE_STRING_RESERVED*/
    0x03,   /* iSerialNumber USB_DEVICE_STRING_RESERVED*/
    0x01    /* bNumConfigurations: one possible configuration*/
};
// Configuration descriptor
const uint8_t MyCfgDescr[] = {
    0x09,   /* bLength */
    0x02,   /* bDescriptorType USB_CONFIGURATION_DESCRIPTOR_TYPE*/
    USB_WBVAL(9+9+9+7+9+9+7+9+7+9+9+9+9+7+7),   /* wTotalLength */
//    USB_WBVAL(9+9+9+7+9+9+7+9+9+7+7+9+9+9+9+7+7),   /* wTotalLength */
    USB_INTERFACE_MAX_NUM,   /* bNumInterfaces */
    0x01,   /* bConfigurationValue */
    0x00,   /* iConfiguration */
    0xA0,
    0x23,    /* bMaxPower */
    // Configuration descriptor

// -----Interface Descriptor Mouse ---------------------------------------------------------------------------------------------------------------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x00,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x01,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x01,   /* bInterfaceSubClass */
    0x02,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    // Interface descriptor

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0100),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeMouse),/* wDescriptorLength */
    // HID class descriptor

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x81,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP1 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP2SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */
    // Endpoint 2 descriptor

// -----Interface Descriptor Other ----------------------------------------------------------------------------------------------------------------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x01,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x01,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x01,   /* bInterfaceSubClass */
    0x01,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    // Interface descriptor

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0100),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeOther),/* wDescriptorLength */
    // HID class descriptor

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x82,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP2 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP2SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */

// -----Interface Descriptor Other ----------------------------------------------------------------------------------------------------------------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x02,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x02,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x00,   /* bInterfaceSubClass */
    0x00,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    // Interface descriptor

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0100),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeManufacturer),/* wDescriptorLength */
    // HID class descriptor

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x83,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP3 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP3SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x03,   /* bEndpointAddress *//* USB_ENDPOINT_OUT USB_EP3 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP3SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */
    // Endpoint 5 descriptor

// // -----Interface descriptor Other ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//    0x09,   /* bLength */
//    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
//    0x03,   /* bInterfaceNumber */
//    0x00,   /* bAlternateSetting */
//    0x00,   /* bNumEndpoints */
//    0x03,   /* bInterfaceClass */
//    0x00,   /* bInterfaceSubClass */
//    0x00,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
//    0x00,   /* iInterface */
//
//    //Interface Descriptor
//    0x09,   /* bLength */
//    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
//    USB_WBVAL(0x0100),/* bcdHID */
//    0x00,   /* bCountryCode */
//    0x00,   /* bNumDescriptors */
//    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
//    USB_WBVAL(0),/* wDescriptorLength */
    // HID class descriptor

// -----Interface Descriptor Other ----------------------------------------------------------------------------------------------------------------------
    0x09,   /* bLength */
    0x04,   /* bDescriptorType USB_INTERFACE_DESCRIPTOR_TYPE*/
    0x03,   /* bInterfaceNumber */
    0x00,   /* bAlternateSetting */
    0x02,   /* bNumEndpoints */
    0x03,   /* bInterfaceClass */
    0x00,   /* bInterfaceSubClass */
    0x00,   /* HID Protocol Codes HID_PROTOCOL_NONE*/
    0x00,   /* iInterface */
    // Interface descriptor

    0x09,   /* bLength */
    0x21,   /* bDescriptorType HID_HID_DESCRIPTOR_TYPE*/
    USB_WBVAL(0x0100),/* bcdHID */
    0x00,   /* bCountryCode */
    0x01,   /* bNumDescriptors */
    0x22,   /* bDescriptorType HID_REPORT_DESCRIPTOR_TYPE*/
    USB_WBVAL(HID_ReportDescSizeIAP),/* wDescriptorLength */
    // HID class descriptor

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x86,   /* bEndpointAddress *//* USB_ENDPOINT_IN USB_EP6 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP6SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */

    0x07,   /* bLength */
    0x05,   /* bDescriptorType USB_ENDPOINT_DESCRIPTOR_TYPE*/
    0x06,   /* bEndpointAddress *//* USB_ENDPOINT_OUT USB_EP6 */
    0x03,   /* bmAttributes */
    USB_WBVAL(DevEP6SIZE),/* wMaxPacketSize */
    0x01,   /* bInterval *//* 1ms */
    // Endpoint 6 descriptor

};

__attribute__((aligned(4)))  uint8_t EP0_Databuf[8]; //ep0(64)+ep4_out(64)+ep4_in(64)
__attribute__((aligned(4)))  uint8_t EP1_Databuf[64 + 64];    //ep1_out(64)+ep1_in(64)
__attribute__((aligned(4)))  uint8_t EP2_Databuf[64 + 64];    //ep2_out(64)+ep2_in(64)
__attribute__((aligned(4)))  uint8_t EP3_Databuf[64 + 64];    //ep3_out(64)+ep3_in(64)
__attribute__((aligned(4)))  uint8_t EP6_Databuf[64 + 64];    //ep5_out(64)+ep5_in(64)

uint8_t        DevConfig= 0;
uint8_t        SetupReqCode;
uint16_t       SetupReqLen;
const uint8_t *pDescr;
uint8_t        Report_Value[USB_INTERFACE_MAX_INDEX+1] = {0x00};
uint8_t        Idle_Value[USB_INTERFACE_MAX_INDEX+1] = {0x00};
volatile uint8_t        USB_SleepStatus = HOST_WAKEUP_ENABLE; /* USB sleep state */
volatile uint8_t    USB_READY_FLAG = 1;
USB_receive_cb_t    USB_receive_cb;
uint8_t         led_val = 0;
uint8_t         USB_receive_buf[64+2];
volatile uint8_t intflag = 0;   // intflag is used to store flag register values
uint8_t         usb_enum_flag = 0;
uint8_t         usb_enum_success_flag=0;

void USB_receive_process(uint8_t *pData,uint8_t len)
{
    if(pData[0] == USB_DATA_IAP)
    {
        OTA_USB_IAPWriteData(&pData[2], len-2);
    }
    else if(pData[0] == USB_DATA_LED)
    {
        peripheral_pilot_led_receive(pData[2]);
    }
    else if(pData[0] == USB_DATA_MANUFACTURER)
    {
        PRINT("MANUFACTURER %x\n",pData[2]);
    }
}

/*********************************************************************
 * @fn      USB_Init
 *
 * @brief   USB_Init
 *
 * @return  none
 */
void USB_Init( void )
{
    PFIC_DisableIRQ(USB_IRQn);
    pEP0_RAM_Addr = EP0_Databuf;
    pEP1_RAM_Addr = EP1_Databuf;
    pEP2_RAM_Addr = EP2_Databuf;
    pEP3_RAM_Addr = EP3_Databuf;
    pEP6_RAM_Addr = EP6_Databuf;
    USB_DeviceInit();
    USB_receive_cb_register(USB_receive_process);
    PFIC_EnableIRQ( USB_IRQn );
}

/*********************************************************************
 * @fn      USB_Uinit
 *
 * @brief   USB_Uinit
 *
 * @return  none
 */
void USB_Uinit( void )
{
    R16_PIN_ANALOG_IE &= ~(RB_PIN_USB_IE | RB_PIN_USB_DP_PU);
    R8_UDEV_CTRL = 0;
    R8_USB_CTRL = 0;
    R8_USB_INT_EN = 0;
    GPIOB_ModeCfg( GPIO_Pin_10 | GPIO_Pin_11, GPIO_ModeIN_PD);
    PFIC_DisableIRQ(USB_IRQn);
}

/*********************************************************************
 * @fn      USB_cfg_vid_pid
 *
 * @brief   USB_cfg_vid_pid
 *
 * @return  none
 */
void USB_cfg_vid_pid( uint16_t VID, uint16_t PID )
{
    MyDevDescr[8] = VID& 0xFF;
    MyDevDescr[9] = (VID >> 8) & 0xFF;
    MyDevDescr[10] = PID& 0xFF;
    MyDevDescr[11] = (PID >> 8) & 0xFF;
}

/*********************************************************************
 * @fn      USB_cfg_manu_info
 *
 * @brief   USB_cfg_manu_info
 *
 * @return  none
 */
void USB_cfg_manu_info( uint8_t *pData, uint8_t len )
{
    uint8_t i;
    MyManuInfo[0] = len*2+2;
    MyManuInfo[1] = 0x03;
    for(i=0; i<len; i++)
    {
        MyManuInfo[2+i*2] = pData[i];
        MyManuInfo[3+i*2] = 0x00;
    }
}

/*********************************************************************
 * @fn      USB_cfg_prod_info
 *
 * @brief   USB_cfg_prod_info
 *
 * @return  none
 */
void USB_cfg_prod_info( uint8_t *pData, uint8_t len )
{
    uint8_t i;
    MyProdInfo[0] = len*2+2;
    MyProdInfo[1] = 0x03;
    for(i=0; i<len; i++)
    {
        MyProdInfo[2+i*2] = pData[i];
        MyProdInfo[3+i*2] = 0x00;
    }
}

/*********************************************************************
 * @fn      USB_cfg_serial_num
 *
 * @brief   USB_cfg_serial_num
 *
 * @return  none
 */
void USB_cfg_serial_num( uint8_t *pData, uint8_t len )
{
    uint8_t i;
    MySerialNum[0] = len*2+2;
    MySerialNum[1] = 0x03;
    for(i=0; i<len; i++)
    {
        MySerialNum[2+i*2] = pData[i];
        MySerialNum[3+i*2] = 0x00;
    }
}
/*********************************************************************
 * @fn      USB_Wake_up
 *
 * @brief   USB_Wake_up
 *
 * @return  none
 */
void USB_Wake_up( void )
{
    R16_PIN_ANALOG_IE &= ~RB_PIN_USB_DP_PU;         // USB pull-up resistor
    R8_UDEV_CTRL |= RB_UD_LOW_SPEED;
    mDelaymS(8);
    R8_UDEV_CTRL &= ~RB_UD_LOW_SPEED;
    R16_PIN_ANALOG_IE |= RB_PIN_USB_DP_PU;         // USB pull-up resistor
}

/*********************************************************************
 * @fn      USB_receive_cb_register
 *
 * @brief   USB_receive_cb_register
 *
 * @return  none
 */
void USB_receive_cb_register(USB_receive_cb_t cback)
{
    USB_receive_cb = cback;
}

/* ***************************************************************************
* @fn USB_mouse_report
*
* @brief Report mouse data
*
* @return 0: Success
* 1: An error occurred */
uint8_t USB_mouse_report(uint8_t *pData,uint8_t len)
{
    if(!USB_READY_FLAG)
    {
        return 0xFF;
    }
    USB_READY_FLAG = 0;
    tmos_memcpy(pEP1_IN_DataBuf, pData, len);
    DevEP1_IN_Deal(len);
    return 0;
}

/* ***************************************************************************
* @fn USB_class_keyboard_report
*
* @brief Report class_keyboard data
*
* @return 0: Success
* 1: An error occurred */
uint8_t USB_class_keyboard_report(uint8_t *pData,uint8_t len)
{
    if(!USB_READY_FLAG)
    {
        return 0xFF;
    }
    USB_READY_FLAG = 0;
    pEP2_IN_DataBuf[0] = REPORT_ID_KEYBOARD;
    tmos_memcpy(&pEP2_IN_DataBuf[1], pData, len);
    DevEP2_IN_Deal(len+1);
    return 0;
}

/* ***************************************************************************
* @fn USB_all_keyboard_report
*
* @brief Report all_keyboard data
*
* @return 0: Success
* 1: An error occurred */
uint8_t USB_all_keyboard_report(uint8_t *pData,uint8_t len)
{
    if(!USB_READY_FLAG)
    {
        return 0xFF;
    }
    USB_READY_FLAG = 0;
    pEP2_IN_DataBuf[0] = REPORT_ID_ALL_KEYBOARD;
    tmos_memcpy(&pEP2_IN_DataBuf[1], pData, len);
    DevEP2_IN_Deal(len+1);
    return 0;
}
/* ***************************************************************************
* @fn USB_manufacturer_report
*
* @brief Report manufacturer data, users should pay attention to making upper-level protocols
*
* @return 0: Success
* 1: An error occurred */
uint8_t USB_manufacturer_report(uint8_t *pData,uint8_t len)
{
    USB_READY_FLAG = 0;
    tmos_memcpy(pEP3_IN_DataBuf, pData, len);
    DevEP3_IN_Deal(len);
    return 0;
}

/* ***************************************************************************
* @fn USB_consumer_report
*
* @brief Report consumer data
*
* @return 0: Success
* 1: An error occurred */
uint8_t USB_consumer_report(uint8_t *pData,uint8_t len)
{
    if(!USB_READY_FLAG)
    {
        return 0xFF;
    }
    USB_READY_FLAG = 0;
    pEP2_IN_DataBuf[0] = REPORT_ID_CONSUMER;
    tmos_memcpy(&pEP2_IN_DataBuf[1], pData, len);
    DevEP2_IN_Deal(len+1);
    return 0;
}

/* ***************************************************************************
* @fn USB_sys_ctl_report
*
* @brief Report sys_ctl data
*
* @return 0: Success
* 1: An error occurred */
uint8_t USB_sys_ctl_report(uint8_t data)
{
    if(!USB_READY_FLAG)
    {
        return 0xFF;
    }
    USB_READY_FLAG = 0;
    pEP2_IN_DataBuf[0] = REPORT_ID_SYS_CTL;
    pEP2_IN_DataBuf[1] = data;
    DevEP2_IN_Deal(2);
    return 0;
}

/* ***************************************************************************
* @fn USB_IAP_report
*
* @brief Report IAP data
*
* @return 0: Success
* 1: An error occurred */
uint8_t USB_IAP_report(uint8_t *pData,uint8_t len)
{
    USB_READY_FLAG = 0;
    tmos_memcpy(pEP6_IN_DataBuf, pData, len);
    DevEP6_IN_Deal(0x40);
    return 0;
}

/* ***************************************************************************
* @fn USB_IRQ_trans_process
*
* @brief USB interrupt function
*
* @return none */
void USB_IRQ_trans_process( void )
{
    uint8_t len, chtype;        // len is used to copy functions, and chtype is used to store information such as data transmission direction, command type, and received objects.
    uint8_t  errflag = 0;   // errflag directive used to mark whether the host is supported

    if( intflag & RB_UIF_TRANSFER )   // Determines the USB transmission completion interrupt flag in _INT_FG.If the transmission is interrupted, enter the if statement
    {
        if( (R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN ) // Non-idle //Judge 5:4 bits in the interrupt status register and view the PID ID of the token.If these two digits are not 11 (meaning idle), enter the if statement
        {
            switch( R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP) )
            // Get the PID ID of the token and the endpoint number of the 3:0 bit in device mode.In host mode, 3:0 bit is the answer PID identification bit
            // Analyze operation tokens and endpoint numbers
            {// Endpoint 0 is used to control transmission.The following IN and OUT tokens of endpoint 0 correspond to the corresponding program, corresponding to the data stage and status stage of the control transmission.
                case UIS_TOKEN_IN:      // The PID of the token package is IN, and the 5:4 bit is 10. The endpoint number of the 3:0 bit is 0.IN token: The device sends data to the host._UIS_: USB interrupt status
                {                       // Endpoint 0 is a bidirectional endpoint, used as control transmission."|0" operation is omitted
                    switch( SetupReqCode )
                    // This value will be assigned when the SETUP packet is received.There will be a SETUP packet handler later, corresponding to the setting stage of control transmission.
                    {
                        case USB_GET_DESCRIPTOR:    // USB standard command, the host obtains description from the USB device
                            len = SetupReqLen >= DevEP0SIZE ? DevEP0SIZE : SetupReqLen; // The packet transmission length is.The maximum length is 64 bytes, and more than 64 bytes are processed in multiple times, and the first few times will be full.
                            tmos_memcpy( pEP0_DataBuf, pDescr, len ); // tmos_memcpy: Memory copy function, copy (number 2) string length from (number 2) address to (number 1) address
                            // DMA is directly connected to memory and will detect memory rewriting, and then the data in memory can be sent out without the control of the microcontroller.If only two arrays are assigned to each other and do not involve physical memory matching DMA, DMA cannot be triggered.
                            SetupReqLen -= len;     // Record the remaining length of data to be sent
                            pDescr += len;          // Update the starting address of the data to be sent next, and use the copy function to
                            R8_UEP0_T_LEN = len;    // Endpoint 0 send length register writes the packet transmission length
                            R8_UEP0_CTRL ^= RB_UEP_T_TOG;   // Synchronous switch.Switch DATA0 and DATA1 in PID of IN direction (for microcontrollers, the T direction)
                            break;                  // The handshake packet response (ACK, NAK, STALL) of the endpoint control register is packaged into a package that complies with the specifications by the hardware, and DMA will automatically send it.
                        case USB_SET_ADDRESS:       // USB standard command, the host sets a unique address for the device, the range 0 to 127, and 0 is the default address
                            R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | SetupReqLen;
                            // 7-bit address + the user-defined address of the highest bit (default is 1), or the "packet transmission length" on the top (the "packet transmission length" here is assigned as the address bit later)
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                            // R responds to OUT transaction ACK, T responds to IN transaction NAK.This CASE branch is in the IN direction. When the DMA corresponding memory is in the microcontroller, it returns to the NAK handshake package.
                            break;                                                  // Generally, the device will return the OUT transaction in the program to the host and will not respond to NAK.

                        case USB_SET_FEATURE:       // USB standard command, the host requires a feature on the device, interface, or endpoint
                            break;

                        default:
                            R8_UEP0_T_LEN = 0;      // The status stage is interrupted or forced upload of 0-length packets to end control transmission (data packets with data field length 0, all SYNC, PID, and EOP fields are included in the packet)
                            R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                            // R responds to OUT transaction ACK, T responds to IN transaction NAK.This CASE branch is OUT direction. When the data is updated in the corresponding DMA memory and the microcontroller acceptance is normal, return to the ACK handshake bag.
                            USB_READY_FLAG = 1;
                            if(usb_enum_flag)
                            {
                                usb_enum_success_flag = 1;
                            }
#if CONFIG_USB_DEBUG
                            PRINT( "Ready_STATUS = %d\n", USB_READY_FLAG );
#endif
                            break;
                    }
                }
                    break;

                case UIS_TOKEN_OUT:     // The PID of the token package is OUT, and the 5:4 bit is 00. The endpoint number of the 3:0 bit is 0.OUT token: The host sends data to the device.
                {                       // Endpoint 0 is a bidirectional endpoint, used as control transmission."|0" operation is omitted
                    len = R8_USB_RX_LEN;    // Read the number of received data bytes stored in the current USB receiving length register //The receiving length register is shared by each endpoint, and the sending length register has its own
                    if((pEP0_DataBuf[0]==REPORT_ID_KEYBOARD)&&USB_receive_cb&&(len==2))
                    {
                        USB_receive_buf[0] = USB_DATA_LED;
                        USB_receive_buf[1] = 1;
                        USB_receive_buf[2] = pEP0_DataBuf[1];
                        led_val = pEP0_DataBuf[1];
                        USB_receive_cb(USB_receive_buf, USB_receive_buf[1]+2);
#if CONFIG_USB_DEBUG
                        PRINT( "OUT 0  %x len %d\n", pEP0_DataBuf[1],len );
#endif
                    }
                }
                    break;

                case UIS_TOKEN_OUT | 3:
                {
                    R8_UEP3_CTRL ^= RB_UEP_R_TOG;       // Switch the DATA of the IN transaction.Sets the PID of the packet to be sent.
                    len = R8_USB_RX_LEN;    // Read the number of received data bytes stored in the current USB receiving length register //The receiving length register is shared by each endpoint, and the sending length register has its own
                    if(USB_receive_cb)
                    {
                        USB_receive_buf[0] = USB_DATA_MANUFACTURER;
                        USB_receive_buf[1] = len;
                        tmos_memcpy(&USB_receive_buf[2], &pEP3_OUT_DataBuf[0], len);
                        USB_receive_cb(USB_receive_buf, len+2);
#if CONFIG_USB_DEBUG
                        PRINT( "OUT 3 %x len %d\n", pEP3_OUT_DataBuf[0],len );
#endif
                    }
                }
                    break;

                case UIS_TOKEN_OUT | 6:
                {
                    R8_UEP6_CTRL ^= RB_UEP_R_TOG;       // Switch the DATA of the IN transaction.Sets the PID of the packet to be sent.
                    len = R8_USB_RX_LEN;    // Read the number of received data bytes stored in the current USB receiving length register //The receiving length register is shared by each endpoint, and the sending length register has its own
                    if(USB_receive_cb)
                    {
                        USB_receive_buf[0] = USB_DATA_IAP;
                        USB_receive_buf[1] = len;
                        tmos_memcpy(&USB_receive_buf[2], &pEP6_OUT_DataBuf[0], len);
                        USB_receive_cb(USB_receive_buf, len+2);
#if CONFIG_USB_DEBUG
                        PRINT( "OUT 6 %x len %d\n", pEP6_OUT_DataBuf[0],len );
#endif
                    }
                }
                    break;

                case UIS_TOKEN_IN | 1: // The PID of the token package is IN and the endpoint number is 1
                    R8_UEP1_CTRL ^= RB_UEP_T_TOG;       // Switch the DATA of the IN transaction.Sets the PID of the packet to be sent.
                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK; // When the data is not updated by the microcontroller in the DMA, the T response IN transaction is set to NAK.Publish data after update.
                    USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                    PRINT( "Ready_IN_EP1 = %d\n", USB_READY_FLAG );
#endif
                    break;

                case UIS_TOKEN_IN | 2: // The PID of the token package is IN and the endpoint number is 2
                    R8_UEP2_CTRL ^= RB_UEP_T_TOG;       // Switch the DATA of the IN transaction.Sets the PID of the packet to be sent.
                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK; // When the data is not updated by the microcontroller in the DMA, the T response IN transaction is set to NAK.Publish data after update.
                    USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                    PRINT( "Ready_IN_EP2 = %d\n", USB_READY_FLAG );
#endif
                    break;

                case UIS_TOKEN_IN | 3: // The PID of the token package is IN and the endpoint number is 3
                    R8_UEP3_CTRL ^= RB_UEP_T_TOG;       // Switch the DATA of the IN transaction.Sets the PID of the packet to be sent.
                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK; // When the data is not updated by the microcontroller in the DMA, the T response IN transaction is set to NAK.Publish data after update.
                    USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                    PRINT( "Ready_IN_EP3 = %d\n", USB_READY_FLAG );
#endif
                    break;

                case UIS_TOKEN_IN | 5: // The PID of the token package is IN and the endpoint number is 5
                    R8_UEP5_CTRL ^= RB_UEP_T_TOG;       // Switch the DATA of the IN transaction.Sets the PID of the packet to be sent.
                    R8_UEP5_CTRL = (R8_UEP5_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK; // When the data is not updated by the microcontroller in the DMA, the T response IN transaction is set to NAK.Publish data after update.
                    USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                    PRINT( "Ready_IN_EP5 = %d\n", USB_READY_FLAG );
#endif
                    break;

                case UIS_TOKEN_IN | 6: // The PID of the token package is IN and the endpoint number is 3
                    R8_UEP6_CTRL ^= RB_UEP_T_TOG;       // Switch the DATA of the IN transaction.Sets the PID of the packet to be sent.
                    R8_UEP6_CTRL = (R8_UEP6_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK; // When the data is not updated by the microcontroller in the DMA, the T response IN transaction is set to NAK.Publish data after update.
                    USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                    PRINT( "Ready_IN_EP6 = %d\n", USB_READY_FLAG );
#endif
                    break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;    // Write 1 Clear interrupt flag
        }

        if( R8_USB_INT_ST & RB_UIS_SETUP_ACT ) // Setup package processing
        {
//            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
            // R responds to OUT transaction expectation to be DATA1 (the PID of the data packet received by DMA must be DATA1, otherwise the calculation data error will be retransmitted) and ACK (the data received in the corresponding memory of DMA, and the acceptance of the microcontroller is normal)
            // The T response IN transaction is set to DATA1 (the microcontroller has data sent to the corresponding DMA memory and is sent out with DATA1) and NAK (the microcontroller has not prepared data).
            SetupReqLen = pSetupReqPak->wLength;    // Number of bytes in the data stage //pSetupReqPak: casts the RAM address of endpoint 0 into an address that stores the structure, and the structure members are arranged in order.
            SetupReqCode = pSetupReqPak->bRequest;  // The sequence number of the command
            chtype = pSetupReqPak->bRequestType;    // Contains information such as data transmission direction, command type, received object, etc.

            len = 0;
            errflag = 0;
            if( (pSetupReqPak->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD ) // Determine the type of the command, if it is not a standard request, enter the if statement
            {
                /* Non-standard request */
                /* Other requests, such as class requests, manufacturer requests, etc. */
                if( pSetupReqPak->bRequestType & 0x40 )   // Get a certain one of the commands and determine whether it is 0, and enter the if statement without zero
                {
                    /* Manufacturer request */
                }
                else if( pSetupReqPak->bRequestType & 0x20 )  // Get a certain one of the commands and determine whether it is 0, and enter the if statement without zero
                {   // Determined as HID class request
                    switch( SetupReqCode )
                    // Determine the sequence number of the command
                    {
                        case DEF_USB_SET_IDLE: /* 0x0A: SET_IDLE */         // The host wants to set the idle time interval for the specific input report of the HID device
                            Idle_Value[pSetupReqPak->wIndex] = (uint8_t)(pSetupReqPak->wValue>>8);
                            break; // This must have

                        case DEF_USB_SET_REPORT: /* 0x09: SET_REPORT */     // The host wants to set the report descriptor for the HID device
                            break;

                        case DEF_USB_SET_PROTOCOL: /* 0x0B: SET_PROTOCOL */ // The host wants to set the protocol currently used by the HID device
                            Report_Value[pSetupReqPak->wIndex] = (uint8_t)(pSetupReqPak->wValue);
                            break;

                        case DEF_USB_GET_IDLE: /* 0x02: GET_IDLE */         // The host wants to read the current idle ratio of the HID device-specific input report
                            EP0_Databuf[0] = Idle_Value[pSetupReqPak->wIndex];
                            len = 1;
                            break;

                        case DEF_USB_GET_PROTOCOL: /* 0x03: GET_PROTOCOL */     // The host wants to obtain the protocol currently used by the HID device
                            EP0_Databuf[0] = Report_Value[pSetupReqPak->wIndex];
                            len = 1;
                            break;

                        default:
                            errflag = 0xFF;
                    }
                }
            }
            else    // Determined as a standard request
            {
                switch( SetupReqCode )
                // Determine the sequence number of the command
                {
                    case USB_GET_DESCRIPTOR:    // The host wants to obtain the standard descriptor
                    {
                        switch( ((pSetupReqPak->wValue) >> 8) )
                        // Move the right 8 bits to see if the original high 8 bits are 0. If it is 1, it means that the direction is IN. Then enter the s-case statement
                        {
                            case USB_DESCR_TYP_DEVICE:  // Different values ​​represent different commands.The host wants to obtain the device descriptor
                            {
                                tmos_stop_task(tran_taskID, SBP_ENTER_SLEEP_EVT);
                                pDescr = MyDevDescr;    // Put the device descriptor string in the pDescr address, and the end of the case "Get standard descriptor" will be sent with a copy function.
                                len = MyDevDescr[0];    // The protocol specifies the length of the first byte of the device descriptor.The copy function will use the len parameter
                            }
                                break;

                            case USB_DESCR_TYP_CONFIG:  // The host wants to obtain the configuration descriptor
                            {
                                pDescr = MyCfgDescr;    // Place the configuration descriptor string in the pDescr address and will be sent later
                                len = MyCfgDescr[2];    // The protocol specifies the total length of the configuration information stored in the third byte of the configuration descriptor.
                            }
                                break;

                            case USB_DESCR_TYP_HID:     // The host wants to obtain the human-computer interface class descriptor.The wIndex in the structure here is different from the configuration descriptor, meaning the interface number.
                                switch( (pSetupReqPak->wIndex) & 0xff )
                                // Take the lower eight digits and wipe off the higher eight digits
                                {
                                    /* Select an interface */
                                    case 0:
                                        pDescr = (uint8_t *) (&MyCfgDescr[18]);  // The class descriptor storage location of interface 1, to be sent
                                        len = 9;
                                        break;

                                    /* Select an interface */
                                    case 1:
                                        pDescr = (uint8_t *) (&MyCfgDescr[43]);  // The class descriptor storage location of interface 1, to be sent
                                        len = 9;
                                        break;

                                    /* Select an interface */
                                    case 2:
                                        pDescr = (uint8_t *) (&MyCfgDescr[68]);  // The class descriptor storage location of interface 1, to be sent
                                        len = 9;
                                        break;

                                    /* Select an interface */
                                    case 3:
                                        pDescr = (uint8_t *) (&MyCfgDescr[68+25+7]);  // The class descriptor storage location of interface 1, to be sent
                                        len = 9;
                                        break;

// /* Select interface */
//                                    case 4:
// pDescr = (uint8_t *) (&MyCfgDescr[68+25+9+9]); //The class descriptor storage location of interface 1, to be sent
//                                        len = 9;
//                                        break;

                                    default:
                                        /* Unsupported string descriptors */
                                        errflag = 0xff;
                                        break;
                                }
                                break;

                            case USB_DESCR_TYP_REPORT:  // The host wants to obtain the device report descriptor
                            {
                                if( ((pSetupReqPak->wIndex) & 0xff) == 0 ) // Interface 0 report descriptor
                                {
                                    pDescr = HID_ReportDescriptorMouse; // Data ready to be uploaded
                                    len = HID_ReportDescSizeMouse;
                                }
                                else if( ((pSetupReqPak->wIndex) & 0xff) == 1 ) // Interface 1 report descriptor
                                {
                                    pDescr = HID_ReportDescriptorOther; // Data ready to be uploaded
                                    len = HID_ReportDescSizeOther;
                                }
                                else if( ((pSetupReqPak->wIndex) & 0xff) == 2 ) // Interface 2 report descriptor
                                {
                                    pDescr = HID_ReportDescriptorManufacturer; // Data ready to be uploaded
                                    len = HID_ReportDescSizeManufacturer;
                                }
                                else if( ((pSetupReqPak->wIndex) & 0xff) == 3 ) // Interface 3 report descriptor
                                {
                                    usb_enum_flag = 1;
                                    pDescr = HID_ReportDescriptorIAP; // Data ready to be uploaded
                                    len = HID_ReportDescSizeIAP;
                                }
                                else
                                    len = 0xff; // This program has only 4 interfaces, so this sentence is not possible to be executed normally
                            }
                                break;

                            case USB_DESCR_TYP_STRING:  // The host wants to obtain the device string descriptor
                            {
                                switch( (pSetupReqPak->wValue) & 0xff )
                                // Pass string information according to the value of wValue
                                {
                                    case 0:
                                        pDescr = LangID_StrDescr;
                                        len = LangID_StrDescr[0];
                                        break;
                                    case 1:
                                        pDescr = MyManuInfo;
                                        len = MyManuInfo[0];
                                        break;
                                    case 2:
                                        pDescr = MyProdInfo;
                                        len = MyProdInfo[0];
                                        break;
                                    case 3:
                                        pDescr = MySerialNum;
                                        len = MySerialNum[0];
                                        break;
                                    default:
                                        errflag = 0xFF; // Unsupported string descriptors
                                        break;
                                }
                            }
                                break;

                            default:
                                errflag = 0xff;
                                break;
                        }
                        if( SetupReqLen > len )
                            SetupReqLen = len;      // The total length needs to be uploaded
                        len = (SetupReqLen >= DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;   // Maximum length is 64 bytes
                        tmos_memcpy( pEP0_DataBuf, pDescr, len );  // Copy functions
                        pDescr += len;
                    }
                        break;

                    case USB_SET_ADDRESS:       // The host wants to set the device address
                        SetupReqLen = (pSetupReqPak->wValue) & 0xff;    // The bit device address distributed by the host is temporarily stored in SetupReqLen
                        break;                                          // The control phase will be assigned to the device address parameters

                    case USB_GET_CONFIGURATION: // The host wants to obtain the current configuration of the device
                        pEP0_DataBuf[0] = DevConfig;    // Put device configuration into RAM
                        if( SetupReqLen > 1 )
                            SetupReqLen = 1;    // Set the number of bytes in the data stage by 1.Because DevConfig has only one byte
                        break;

                    case USB_SET_CONFIGURATION: // The host wants to set the current configuration of the device
                        DevConfig = (pSetupReqPak->wValue) & 0xff;  // Take the lower eight digits and wipe off the higher eight digits
                        break;

                    case USB_CLEAR_FEATURE:     // Turn off the features/functions of the USB device.It can be at the device or endpoint level.
                    {
                        if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP ) // Determine whether it is an endpoint feature (clear the state where the endpoint stops working)
                        {
                            switch( (pSetupReqPak->wIndex) & 0xff )
                            // Take the lower eight digits and erase the higher eight digits.Judge index
                            {// The highest bit of 16 bits determines the data transmission direction, 0 is OUT and 1 is IN.The low position is the endpoint number.
                                case 0x81:      // Clear the three bits of _TOG and _T_RES, and write the latter as _NAK, and respond to IN transaction NAK that means no data is returned.
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x01:      // Clear the three bits of _TOG and _R_RES, and write the latter as _ACK, and respond to OUT transaction ACK that indicates normal reception.
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x82:      // Clear the three bits of _TOG and _T_RES, and write the latter as _NAK, and respond to IN transaction NAK that means no data is returned.
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x02:      // Clear the three bits of _TOG and _R_RES, and write the latter as _ACK, and respond to OUT transaction ACK that indicates normal reception.
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x83:      // Clear the three bits of _TOG and _T_RES, and write the latter as _NAK, and respond to IN transaction NAK that means no data is returned.
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x03:      // Clear the three bits of _TOG and _R_RES, and write the latter as _ACK, and respond to OUT transaction ACK that indicates normal reception.
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x85:      // Clear the three bits of _TOG and _T_RES, and write the latter as _NAK, and respond to IN transaction NAK that means no data is returned.
                                    R8_UEP5_CTRL = (R8_UEP5_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x05:      // Clear the three bits of _TOG and _R_RES, and write the latter as _ACK, and respond to OUT transaction ACK that indicates normal reception.
                                    R8_UEP5_CTRL = (R8_UEP5_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                case 0x86:      // Clear the three bits of _TOG and _T_RES, and write the latter as _NAK, and respond to IN transaction NAK that means no data is returned.
                                    R8_UEP6_CTRL = (R8_UEP6_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                                    break;
                                case 0x06:      // Clear the three bits of _TOG and _R_RES, and write the latter as _ACK, and respond to OUT transaction ACK that indicates normal reception.
                                    R8_UEP6_CTRL = (R8_UEP6_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                                    break;
                                default:
                                    errflag = 0xFF; // Unsupported endpoints
                                    break;
                            }
                        }
                        else if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE ) // Determine whether it is a device feature (used to wake up the device)
                        {
#if CONFIG_USB_DEBUG
                            PRINT( "清睡眠\n" );
#endif
                            if( pSetupReqPak->wValue == 1 )   // The wake-up flag is 1
                            {
                                USB_SleepStatus &= ~HOST_SET_FEATURE;   // Clear the lowest position
                                USB_SleepStatus |= HOST_WAKEUP_ENABLE;
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                    }
                        break;

                    case USB_SET_FEATURE:       // Turn on the features/functions of the USB device.It can be at the device or endpoint level.
                        if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP ) // Determine whether it is an endpoint feature (make the endpoint stop working)
                        {
                            /* Endpoint */
                            switch( pSetupReqPak->wIndex )
                            // Judge index
                            {// The highest bit of 16 bits determines the data transmission direction, 0 is OUT and 1 is IN.The low position is the endpoint number.
                                case 0x81:      // Clear _TOG and _T_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x01:      // Clear _TOG and _R_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                case 0x82:      // Clear _TOG and _T_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x02:      // Clear _TOG and _R_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP2_CTRL = (R8_UEP2_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                case 0x83:      // Clear _TOG and _T_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x03:      // Clear _TOG and _R_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP3_CTRL = (R8_UEP3_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                case 0x85:      // Clear _TOG and _T_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP5_CTRL = (R8_UEP5_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x05:      // Clear _TOG and _R_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP5_CTRL = (R8_UEP5_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                case 0x86:      // Clear _TOG and _T_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP6_CTRL = (R8_UEP6_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_STALL;
                                    break;
                                case 0x06:      // Clear _TOG and _R_RES, and write the latter as _STALL to stop the endpoint's work according to the host instruction.
                                    R8_UEP6_CTRL = (R8_UEP6_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_STALL;
                                    break;
                                default:
                                    /* Unsupported endpoints */
                                    errflag = 0xFF; // Unsupported endpoints
                                    break;
                            }
                        }
                        else if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE ) // Determine whether it is a device feature (make the device sleepy)
                        {
#if CONFIG_USB_DEBUG
                            PRINT( "设置睡眠\n" );
#endif
                            if( pSetupReqPak->wValue == 1 )
                            {
                                USB_SleepStatus |= HOST_SET_FEATURE;    // Setting up sleep
                            }
                        }
                        else
                        {
                            errflag = 0xFF;
                        }
                        break;

                    case USB_GET_INTERFACE:     // The host wants to obtain the selection setting value for the interface currently working
                        pEP0_DataBuf[0] = 0x00;
                        if( SetupReqLen > 1 )
                            SetupReqLen = 1;    // Set the number of bytes in the data stage by 1.Because there is only one byte of data to be transmitted
                        break;

                    case USB_SET_INTERFACE:     // The host wants to activate an interface of the device
                        break;

                    case USB_GET_STATUS:        // The host wants to obtain the status of the device, interface, or endpoint
                        if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP ) // Determine whether it is endpoint status
                        {
                            /* Endpoint */
                            pEP0_DataBuf[0] = 0x00;
                            switch( pSetupReqPak->wIndex )
                            {       // The highest bit of 16 bits determines the data transmission direction, 0 is OUT and 1 is IN.The low position is the endpoint number.
                                case 0x81:      // Judgment _TOG and _T_RES, if in STALL state, enter if statement
                                    if( (R8_UEP1_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01; // Returning D0 is 1, indicating that the endpoint has stopped working.This bit is configured by the SET_FEATURE and CLEAR_FEATURE commands.
                                    }
                                    break;

                                case 0x01:      // Judge the three digits _TOG and _R_RES, if in STALL state, enter the if statement
                                    if( (R8_UEP1_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                                case 0x82:      // Judgment _TOG and _T_RES, if in STALL state, enter if statement
                                    if( (R8_UEP2_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01; // Returning D0 is 1, indicating that the endpoint has stopped working.This bit is configured by the SET_FEATURE and CLEAR_FEATURE commands.
                                    }
                                    break;

                                case 0x02:      // Judge the three digits _TOG and _R_RES, if in STALL state, enter the if statement
                                    if( (R8_UEP2_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                                case 0x83:      // Judgment _TOG and _T_RES, if in STALL state, enter if statement
                                    if( (R8_UEP3_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01; // Returning D0 is 1, indicating that the endpoint has stopped working.This bit is configured by the SET_FEATURE and CLEAR_FEATURE commands.
                                    }
                                    break;

                                case 0x03:      // Judge the three digits _TOG and _R_RES, if in STALL state, enter the if statement
                                    if( (R8_UEP3_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                                case 0x85:      // Judgment _TOG and _T_RES, if in STALL state, enter if statement
                                    if( (R8_UEP5_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01; // Returning D0 is 1, indicating that the endpoint has stopped working.This bit is configured by the SET_FEATURE and CLEAR_FEATURE commands.
                                    }
                                    break;

                                case 0x05:      // Judge the three digits _TOG and _R_RES, if in STALL state, enter the if statement
                                    if( (R8_UEP5_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                                case 0x86:      // Judgment _TOG and _T_RES, if in STALL state, enter if statement
                                    if( (R8_UEP6_CTRL & (RB_UEP_T_TOG | MASK_UEP_T_RES)) == UEP_T_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01; // Returning D0 is 1, indicating that the endpoint has stopped working.This bit is configured by the SET_FEATURE and CLEAR_FEATURE commands.
                                    }
                                    break;

                                case 0x06:      // Judge the three digits _TOG and _R_RES, if in STALL state, enter the if statement
                                    if( (R8_UEP6_CTRL & (RB_UEP_R_TOG | MASK_UEP_R_RES)) == UEP_R_RES_STALL )
                                    {
                                        pEP0_DataBuf[0] = 0x01;
                                    }
                                    break;
                            }
                        }
                        else if( (pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE ) // Determine whether it is the device status
                        {
                            pEP0_DataBuf[0] = 0x00;
                            if( USB_SleepStatus&HOST_SET_FEATURE )     // If the device is sleeping
                            {
                                pEP0_DataBuf[0] = 0x02;     // The lowest bit D0 is 0, which means the device is powered by the bus, and 1 means the device is powered by the device.A D1 bit of 1 means that remote wake-up is supported, and a 0 means that it is not supported.
                            }
                            else
                            {
                                pEP0_DataBuf[0] = 0x00;
                            }
                        }
                        pEP0_DataBuf[1] = 0;    // The format of the return status information is 16 digits, and the high eight digits are reserved as 0
                        if( SetupReqLen >= 2 )
                        {
                            SetupReqLen = 2;    // Set the number of bytes in the data stage by 2.Because there are only 2 bytes of data to be transferred
                        }
                        break;

                    default:
                        errflag = 0xff;
                        break;
                }
            }
            if( errflag == 0xff ) // Error or not supported
            {
                //                  SetupReqCode = 0xFF;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL; // STALL
                USB_READY_FLAG = 1;
#if CONFIG_USB_DEBUG
                PRINT( "Ready_Stall = %d\n", USB_READY_FLAG );
#endif
            }
            else
            {
                if( chtype & 0x80 )   // Upload.The highest bit is 1, and the data transmission direction is the transmission of the device to the host.
                {
                    len = (SetupReqLen > DevEP0SIZE) ? DevEP0SIZE : SetupReqLen;
                    SetupReqLen -= len;
                }
                else
                    len = 0;        // Download.The highest bit is 0, and the data transmission direction is the transmission of the host to the device.
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;     // The default packet is DATA1
            }

            R8_USB_INT_FG = RB_UIF_TRANSFER;    // Write 1 clear interrupt flag
        }
    }

    else if( intflag & RB_UIF_BUS_RST )   // Determine the bus reset flag bit in _INT_FG, triggered by 1.
    {
        USB_READY_FLAG = 1;
        tmos_stop_task(tran_taskID, SBP_ENTER_SLEEP_EVT);
        R8_USB_DEV_AD = 0;      // The device address is written as 0, and the host will reassign a new address to the device.
        R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;   // Write the control register of endpoint 0 as: the reception response ACK means normal reception, and the transmission response NAK means no data to be returned.
        R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP5_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_UEP6_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        R8_USB_INT_FG = RB_UIF_BUS_RST; // Write 1 clear interrupt flag
    }
    else if( intflag & RB_UIF_SUSPEND )   // Determines the bus suspend or wake-up event interrupt flag in _INT_FG.Both hang and wake up trigger this interrupt
    {
        if( R8_USB_MIS_ST & RB_UMS_SUSPEND )  // Get the suspend status bit in the miscellaneous status register. It is 1 that indicates that the USB bus is in a suspended state, and it is 0 that indicates that the bus is in a non-hanged state.
        {
            USB_READY_FLAG = 0;
            USB_SleepStatus |= HOST_SET_SUSPEND;
            if(USB_SleepStatus & HOST_SET_FEATURE)
            {
                tmos_set_event(tran_taskID, SBP_ENTER_SLEEP_EVT);
            }
#if CONFIG_USB_DEBUG
            PRINT( "Ready_Sleep = %x\n", USB_SleepStatus );
#endif
        } // Hang // When the device is idle for more than 3ms, the host requires the device to hang (similar to computer hibernation)
        else    // The suspend or wake-up interrupt is triggered and is not judged to be suspended
        {
            USB_READY_FLAG = 1;
            USB_SleepStatus &= ~HOST_SET_SUSPEND;
            USB_SleepStatus |= HOST_WAKEUP_ENABLE;
            tmos_stop_task(tran_taskID, SBP_ENTER_SLEEP_EVT);
#if CONFIG_USB_DEBUG
            PRINT( "Ready_WeakUp = %x\n", USB_SleepStatus );
#endif
        } // wake
        R8_USB_INT_FG = RB_UIF_SUSPEND; // Write 1 clear interrupt sign
    }
    else
    {
        R8_USB_INT_FG = intflag;    // There is no interrupt flag in _INT_FG, and then write the original value back to the original register
    }
}

/* ***************************************************************************
* @fn USB_IRQHandler
*
* @brief USB interrupt function
*
* @return none */
__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void USB_IRQHandler( void ) /* USB interrupt service program, use register group 1 */
{
    intflag = R8_USB_INT_FG;        // Get the value of the interrupt identification register

    if( intflag & RB_UIF_TRANSFER )   // Determines the USB transmission completion interrupt flag in _INT_FG.If the transmission is interrupted, enter the if statement
    {
        if( R8_USB_INT_ST & RB_UIS_SETUP_ACT ) // Setup package processing
        {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
        }
    }
    USB_IRQ_trans_process();

}

/******************************** endfile @ usb ******************************/
