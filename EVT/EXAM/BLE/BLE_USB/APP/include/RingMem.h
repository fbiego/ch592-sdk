/********************************** (C) COPYRIGHT *******************************
 * File Name          : RingMem.h
 * Author             : WCH
 * Version            : V1.4
 * Date               : 2019/12/20
 * Description        :
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
/*********************************************************************
 * CONSTANTS
 */

#ifndef RINGMEM_H
#define RINGMEM_H

#ifndef SUCCESS
  #define SUCCESS    0
#endif
//#ifndef ENABLE
//  #define ENABLE    1
//#endif
//#ifndef DISABLE
//  #define DISABLE    0
//#endif

/*********************************************************************
 * TYPEDEFS
 */

typedef int (*RingMemProtection_t)(uint8_t enable);

//
typedef struct
{
    uint8_t volatile *pData;       //
    uint8_t volatile *pWrite;      //
    uint8_t volatile *pRead;       //
    uint8_t volatile *pEnd;        //
    uint32_t volatile RemanentLen; //
    uint32_t volatile CurrentLen;  //
    uint32_t volatile MaxLen;      //
} RingMemParm_t;

/*********************************************************************
 * Global Variables
 */

/*********************************************************************
 * FUNCTIONS
 */

extern void RingMemInit(RingMemParm_t *Parm, uint8_t *StartAddr, uint32_t MaxLen, RingMemProtection_t Protection);

extern uint8_t RingMemWrite(RingMemParm_t *Parm, uint8_t *pData, uint32_t len);

extern uint8_t RingMemRead(RingMemParm_t *Parm, uint8_t *pData, uint32_t len);

extern uint8_t RingMemCopy(RingMemParm_t *Parm, uint8_t *pData, uint32_t len);

extern uint8_t RingMemDelete(RingMemParm_t *Parm, uint32_t len);

extern uint8_t RingAddInStart(RingMemParm_t *Parm, uint8_t *pData, uint32_t len);

extern uint8_t RingReturnSingleData(RingMemParm_t *Parm, uint32_t Num);
/*********************************************************************
*********************************************************************/

#endif
