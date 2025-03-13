/* ********************************* (C) COPYRIGHT ***************************
* File Name: Main.c
* Author: WCH
* Version: V1.0
* Date: 2020/08/06
* Description: rt-thread nano migration routine, using hardware stack pressing, interrupt nesting is optional, interrupt function is no longer modified
* __attribute__((interrupt("WCH-Interrupt-fast"))),
* The interrupt function is directly defined according to the normal function, and only uses HIGHCODE to modify it.
************************************************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
********************************************************************************************* */

#include "CH59x_common.h"
#include <rtthread.h>

ALIGN(RT_ALIGN_SIZE)
static char task1_stack[512];
static struct rt_thread task1_thread;

/* ***************************************************************************
* @fn task1_entry
*
* @brief task1 task function
*
* @return none */
__HIGH_CODE
void task1_entry(void *parameter)
{
    while(1)
    {
        rt_kprintf("task1\r\n");
        rt_thread_delay(1000);
    }
}

ALIGN(RT_ALIGN_SIZE)
static char task2_stack[512];
static struct rt_thread task2_thread;

/* ***************************************************************************
* @fn task2_entry
*
* @brief task2 task function
*
* @return none */
__HIGH_CODE
void task2_entry(void *parameter)
{
    while(1)
    {
        rt_kprintf("task2\r\n");
        rt_thread_delay(800);
    }
}

ALIGN(RT_ALIGN_SIZE)
static char task3_stack[512];
static struct rt_thread task3_thread;
static rt_sem_t gpioa_sem = RT_NULL;

/* ***************************************************************************
* @fn task3_entry
*
* @brief task3 task function
*
* @return none */
__HIGH_CODE
void task3_entry(void *parameter)
{
    gpioa_sem = rt_sem_create("gpioa sem", 0, RT_IPC_FLAG_FIFO);
    if(gpioa_sem != NULL)
    {
        GPIOA_ModeCfg(GPIO_Pin_12, GPIO_ModeIN_PU);
        GPIOA_ITModeCfg(GPIO_Pin_12, GPIO_ITMode_FallEdge);
        PFIC_EnableIRQ(GPIO_A_IRQn);
        while(1)
        {
            rt_sem_take(gpioa_sem, RT_WAITING_FOREVER); /* Wait for semaphore */
            rt_kprintf("gpioa sem get\r\n");
        }
    }
    else
    {
        rt_kprintf("sem create failed\r\n");
    }
}

/* ********************************************************************
* @fn      main
*
* @brief   Ö÷º¯Êý
*
* @note    main is one of threads in rt-thread.
*
* @return  none */
int main()
{
    rt_enter_critical();

    rt_thread_init(&task1_thread,
                   "task1",
                   task1_entry,
                   RT_NULL,
                   &task1_stack[0],
                   sizeof(task1_stack),
                   4, 20);
    rt_thread_startup(&task1_thread);

    rt_thread_init(&task2_thread,
                   "task2",
                   task2_entry,
                   RT_NULL,
                   &task2_stack[0],
                   sizeof(task2_stack),
                   4, 20);
    rt_thread_startup(&task2_thread);

    rt_thread_init(&task3_thread,
                   "task3",
                   task3_entry,
                   RT_NULL,
                   &task3_stack[0],
                   sizeof(task3_stack),
                   4, 20);
    rt_thread_startup(&task3_thread);

    rt_exit_critical();

    return 0;
}

void msh_test_print(void)
{
    rt_kprintf("this is a test for msh.\n");
}
MSH_CMD_EXPORT(msh_test_print, this is a msh test);

/*********************************************************************
 * @fn      GPIOA_IRQHandler
 *
 * @brief   GPIOA_IRQHandler.
 *
 * @param   none
 *
 * @return  none
 */
__HIGH_CODE
void GPIOA_IRQHandler(void)
{
    /* This function can be used as an example of interrupt function writing in rt-thread nano in this project */
    uint16_t flag;
    flag = GPIOA_ReadITFlagPort();
    if((flag & GPIO_Pin_12) != 0)
    {
        rt_sem_release(gpioa_sem);  /* Release semaphore */
    }
    GPIOA_ClearITFlagBit(flag); /* Clear the interrupt flag */
}

