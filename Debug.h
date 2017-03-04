/*****************************************************************************************************
* FileName:                    Debug.h
*
* Description:                 调试用定义
*
* Author:                      YanDengxue, Fiberhome-Fuhua
*
* Rev History:
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-03-29 15:30       --           1.00             Create
*****************************************************************************************************/
#ifndef _Debug_H
#define _Debug_H

#ifdef __cplusplus
extern "C" {
#endif

//====================================================================================================
// 宏定义
//====================================================================================================
#define XMODEM_SOH 0x01u
#define XMODEM_EOT 0x04u
#define XMODEM_ACK 0x06u
#define XMODEM_NAK 0x15u
#define XMODEM_CAN 0x18u

//====================================================================================================
// 调试相关
//====================================================================================================
#define DEBUG_UART_DESIRED_BAUD   (115200u)
#define DEBUG_UART_OVERSAMPLE_CNT (13u)

//====================================================================================================
// 调试用公用宏定义 Create by YanDengxue at 2010-01-25 10:00
//====================================================================================================
#define TRACE(fmt) DebugSendString(fmt)

//====================================================================================================
// 外部函数声明
//====================================================================================================
extern char DebugSendChar(char c);
extern int32 DebugSendString(char const *seq);
extern int32 DebugSendHexLong(Uint32 value);
extern int32 DebugSendLong(Uint32 value);
extern int32 DebugSendHexByte(Uint8 value);

#ifdef __cplusplus
}
#endif

#endif

