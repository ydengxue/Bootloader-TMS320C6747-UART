/*****************************************************************************************************
* FileName:                    HardwareInital.c
*
* Description:                 硬件初始化函数
*
* Author:                      YanDengxue, Fiberhome-Fuhua
*
* Rev History:
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-03-29 15:30       --           1.00             Create
*****************************************************************************************************/
//====================================================================================================
// 本文件使用的头文件
//====================================================================================================
// 自定义头文件
#include "UserTypesDef.h"
#include "Debug.h"
#include "C6747Register.h"
#include "SystemBase.h"
#include "SpiFlash.h"

//====================================================================================================
// 本地函数声明,此处声明的函数不与外部接口
//====================================================================================================
static int32 GPIOInitial(void);
static int32 DebugUARTInitial(void);
static int32 Timer0Initial(void);
static int32 InterruptVectorsInitial(void);

//====================================================================================================
// 本地变量声明,此处声明的变量不与外部接口
//====================================================================================================
// 中断向量表
// create by YanDengxue 2011-04-20 10:00
#pragma DATA_ALIGN(intc_vector_table, 1024 )
#pragma DATA_SECTION(intc_vector_table, ".text:vecs")
static Uint32 intc_vector_table[] =
{
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0000002A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000,
    0x003C30F6,  0x0015502A, 0x0000006A, 0x00000362, 0x003C36E6,  0x0C6E2C6E, 0x00000000, 0xE4000000
};

//====================================================================================================
// 函数实现
//====================================================================================================
//----------------------------------------------------------------------------------------------------
// 接口函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: InterruptRegister
//      Input: INTERRUPT_FUNCTION interrupt_function: 中断服务程序
//             Uint8 priority: 优先级
//             Uint8 interrupt_src_num: 中断源
//             int8 interrupt_enable_flag: CPU中断是否使能标识
//     Output: void
//     Return: int32: 函数执行情况
//Description: 中断向量注册函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 InterruptVectorsInitial(void)
{
    intc_vector_table[(11u << 3) + 1] |= (LSHW((Uint32)Timer0InterruptService) << 7);
    intc_vector_table[(11u << 3) + 1 + 1] |= (HSHW((Uint32)Timer0InterruptService) << 7);

    pINTC->INTMUX2 &= (~(0x7F << ((11u - 8u) << 3)));
    pINTC->INTMUX2 |= (64u << ((11u - 8u) << 3));

    ISTP = (unsigned int)intc_vector_table;
    ICR  = 0xFFF0;// clear all interrupts, bits 4 thru 15
    IER  = 0x0002;// enable the bits for non maskable interrupt
    _enable_interrupts();// enable interrupts, set GIE bit

    IER |= (1 << 11u);// enable the bits for non maskable interrupt

    return NORMAL_SUCCESS;
}


//----------------------------------------------------------------------------------------------------
//   Function: HardwareInitial
//      Input: void
//     Output: void
//     Return: int32: 函数执行情况
//Description: 硬件初始化函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 HardwareInitial(void)
{
    Vint32 lv_temp_ulong;

    pSYSCFG->KICKR[0] = SYSCFG_UNLOCK0;// Kick0 register + data (unlock)
    pSYSCFG->KICKR[1] = SYSCFG_UNLOCK1;// Kick1 register + data (unlock)

    // Disable L2,L1P,L1D cache
    pCACHE->L2CFG  = 0;
    lv_temp_ulong  = pCACHE->L2CFG;
    pCACHE->L1PCFG = 0;
    lv_temp_ulong  = pCACHE->L1PCFG;
    pCACHE->L1DCFG = 0;
    lv_temp_ulong  = pCACHE->L1DCFG;

    // HardwareInitial
    InterruptVectorsInitial();
    GPIOInitial();
    DebugUARTInitial();
    WinbondW25ClaimBus();
    Timer0Initial();

    pTIMER0->TCR |= ((2 << 6) | (2 << 22));

    return NORMAL_SUCCESS;
}


//----------------------------------------------------------------------------------------------------
// 本地函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: GPIOInitial
//      Input: void
//     Output: void
//     Return: int32: 函数执行情况
//Description: 通用输入输出输出初始化
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 GPIOInitial(void)
{
    pGPIO->DIR23 = GPIO_GP2P12 | GPIO_GP3P15;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: DebugUARInitial
//      Input: void
//     Output: void
//     Return: int32: 函数执行情况
//Description: Debug串口初始化函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 DebugUARTInitial(void)
{
    Uint16 lv_divisor;

    lv_divisor = (FREQ_PLL0_SYSCLK2 / (DEBUG_UART_DESIRED_BAUD * DEBUG_UART_OVERSAMPLE_CNT));

    pDEBUG->PWREMU_MGNT = 0;
    pUART0->FCR = 0x7 ;//FIFO mode, Clear UART TX & RX FIFOs 1bytes
    pDEBUG->LCR = 0x3;
    pDEBUG->IER = 0;
    pDEBUG->MCR = 0;
    pDEBUG->MDR = 1;
    pDEBUG->DLL = (Uint8)lv_divisor;
    pDEBUG->DLH = (Uint8)(lv_divisor >> 8);
    DelayLoop(4000);
    pDEBUG->PWREMU_MGNT |= 0x6001;

    DebugSendString("\r\n");

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: Timer0Initial
//      Input: void
//     Output: void
//     Return: int32: 函数执行情况
//Description: Timer0初始化函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 Timer0Initial(void)
{
    pTIMER0->TCR  =   (0 << 1)
                    | (0 << 2)
                    | (1 << 3)
                    | (0 << 4)
                    | (0 << 6)
                    | (0 << 8)
                    | (0 << 9)
                    | (0 << 10)
                    | (0 << 11)
                    | (0 << 12)
                    | (0 << 17)
                    | (0 << 18)
                    | (1 << 19)
                    | (0 << 20)
                    | (0 << 22)
                    | (0 << 24)
                    | (0 << 25)
                    | (0 << 26)
                    | (0 << 27)
                    | (0 << 28);
    pTIMER0->TGCR =   (0 << 0)
                    | (0 << 1)
                    | (1 << 2)
                    | (0 << 4)
                    | (TIMER0B34_PERDIV << 8)
                    | (0 << 12);

    pTIMER0->PRD12 =  TIMER0B12_PERIOD;
    pTIMER0->TIM12 =  0;

    pTIMER0->PRD34 =  TIMER0B34_PERIOD;
    pTIMER0->TIM34 =  0;

    pTIMER0->TGCR =   (1 << 0)
                    | (1 << 1)
                    | (1 << 2)
                    | (0 << 4)
                    | (TIMER0B34_PERDIV << 8)
                    | (0 << 12);
    pTIMER0->INTCTLSTAT =   (0 << 0)
                          | (1 << 1)
                          | (0 << 2)
                          | (1 << 3)
                          | (1 << 16)
                          | (1 << 17)
                          | (0 << 18)
                          | (1 << 19);

    return NORMAL_SUCCESS;
}


