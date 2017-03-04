/*****************************************************************************************************
* FileName:                    Spi.h
*
* Description:                 Spi公用头文件
*                              
* Author:                      YanDengxue, Fiberhome-Fuhua
*                              
* Rev History:  
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-03-29 15:30       --           1.00             Create
*****************************************************************************************************/
#ifndef _Spi_H
#define _Spi_H

#ifdef __cplusplus
extern "C" {
#endif

//====================================================================================================
// 外部函数声明
//====================================================================================================
extern Uint8 SPIXferOneByte(SPI_REGS *pSPI, Uint8 data_tx, Uint8 flags);
extern int32 SPIXferBytes(SPI_REGS *pSPI, int32 length, const Uint8 *dout, Uint8 *din, Uint8 flags);

#ifdef __cplusplus
}
#endif

#endif

