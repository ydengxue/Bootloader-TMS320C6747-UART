/*****************************************************************************************************
* FileName:                    Spi.c
*
* Description:                 Spi公用函数
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
#include "Spi.h"

//====================================================================================================
// 函数实现
//====================================================================================================
//----------------------------------------------------------------------------------------------------
// 接口函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: SPIXferOneByte
//      Input: SPI_REGS *pSPI: 待操作SPI
//             Uint32 data_tx: 待输出数据
//             Uint8 flags: 是否清出错标识
//     Output: void
//     Return: Uint8: SPI输入
//Description: SPI单个数据输入输出
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
Uint8 SPIXferOneByte(SPI_REGS *pSPI, Uint8 data_tx, Uint8 flags)
{
    VUint32 lv_temp;

    // do an empty read to clear the current contents
    lv_temp = pSPI->SPIBUF;

    lv_temp = data_tx;
    if (0 == flags)
    {
        lv_temp |= (1 << 28);
    }

    while (0 != (pSPI->SPIBUF & 0x20000000));
    pSPI->SPIDAT1 = lv_temp;
    while (0 != (pSPI->SPIBUF & 0x80000000));
    lv_temp = pSPI->SPIBUF;

    return (Uint8)lv_temp;

}


//----------------------------------------------------------------------------------------------------
//   Function: SPIXferOneByte
//      Input: SPI_REGS *pSPI: 待操作SPI
//             Uint32 length: 待传输数据数量
//             const Uint8 *dout: 输出数据buffer
//             Uint8 flags: 是否清出错标识
//     Output: Uint8 *din: 输入数据buffer
//     Return: Uint8: SPI输入
//Description: 函数执行情况
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 SPIXferBytes(SPI_REGS *pSPI, int32 length, const Uint8 *dout, Uint8 *din, Uint8 flags)
{
    Uint32      i;
    VUint32 lv_temp;
    const Uint8 *lv_p_tx_buffer;
    Uint8       *lv_p_rx_buffer;

    if (0 == length)
    {
        if (0 != flags)
        {
            while (0 != (pSPI->SPIBUF & 0x20000000));
            pSPI->SPIDAT1 = 0;
            while (0 != (pSPI->SPIBUF & 0x80000000));
        }

        return NORMAL_SUCCESS;
    }

    // do an empty read to clear the current contents
    lv_temp = pSPI->SPIBUF;

    lv_p_tx_buffer = dout;
    lv_p_rx_buffer = din;

    for (i = 0; i < length - 1; i++)
    {
        lv_temp = (1 << 28);
        if (NULL != lv_p_tx_buffer)
        {
            lv_temp |= *lv_p_tx_buffer;
            lv_p_tx_buffer++;
        }

        while (0 != (pSPI->SPIBUF & 0x20000000 ));
        pSPI->SPIDAT1 = lv_temp;
        while (0 != (pSPI->SPIBUF & 0x80000000));
        lv_temp = pSPI->SPIBUF;

        if (NULL != lv_p_rx_buffer)
        {
            *lv_p_rx_buffer = lv_temp;
            lv_p_rx_buffer++;
        }
    }

    if (0 != flags)
    {
        lv_temp = 0;
    }
    else
    {
        lv_temp = (1 << 28);
    }

    if (NULL != lv_p_tx_buffer)
    {
        lv_temp |= *lv_p_tx_buffer;
    }

    while (0 != (pSPI->SPIBUF & 0x20000000 ));
    pSPI->SPIDAT1 = lv_temp;
    while (0 != (pSPI->SPIBUF & 0x80000000));
    lv_temp = pSPI->SPIBUF;

    if (NULL != lv_p_rx_buffer)
    {
        *lv_p_rx_buffer = lv_temp;
    }

    return NORMAL_SUCCESS;
}

