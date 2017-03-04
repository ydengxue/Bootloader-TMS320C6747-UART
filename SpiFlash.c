/*****************************************************************************************************
* FileName:                    SpiFlash.c
*
* Description:                 SpiFlash操作相关函数
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
#include "SpiFlash.h"

//====================================================================================================
// 本地函数声明,此处声明的函数不与外部接口
//====================================================================================================
static int32 WinbondW25WaitReady(Uint32 timeout);

//====================================================================================================
// 函数实现
//====================================================================================================
//----------------------------------------------------------------------------------------------------
//   Function: WinbondW25ClaimBus
//      Input: void
//     Output: void
//     Return: int32: 函数执行情况
//Description: Initialze SPI flash interface
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 WinbondW25ClaimBus(void)
{
    // Enable the SPI hardware
    pSPIFLASH->SPIGCR0 = 0;
    DelayLoop(2000);
    pSPIFLASH->SPIGCR0 = 1;

    // Set master mode, powered up and not activated
    pSPIFLASH->SPIGCR1 = 3;

    // CS, CLK, SIMO and SOMI are functional pins
    pSPIFLASH->SPIPC0 = (1 << 0) | (1 << 9) | (1 << 10) | (1 << 11);

    // setup format
    //scalar = ((clk_get(DAVINCI_SPI_CLKID) / ds->freq) - 1 ) & 0xFF;
    pSPIFLASH->SPIFMT0 = WINBOND_W25_DATA_LENGTH | (WINBOND_W25_PRESCALE << 8) | (0 << 16) | (1 << 17) | (0 << 20);

    // release cs after transfer
    pSPIFLASH->SPIDAT1 = (0 << 28) | (0 << 16);

    // including a minor delay. No science here. Should be good even with no delay
    pSPIFLASH->SPIDELAY = (8 << 24) | (8 << 16);

    // default chip select register
    pSPIFLASH->SPIDEF = 1;

    // no interrupts
    pSPIFLASH->SPIINT = 0;
    pSPIFLASH->SPILVL = 0;

    // enable SPI
    pSPIFLASH->SPIGCR1 |= ( 1 << 24 );

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: SPIFlashReleaseBus
//      Input: void
//     Output: void
//     Return: int32: 函数执行情况
//Description: 释放WinbondW25总线
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 WinbondW25ReleaseBus(void)
{
    // Disable the SPI hardware
    pSPIFLASH->SPIGCR0 = 0;

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: WinbondW25FastRead
//      Input: Uint32 addr: 读入地址
//             Uint32 length: 读入数据的长度
//     Output: Uint8 *buffer: 读入数据缓存首地址
//     Return: int32: 函数执行情况
//Description: WinbondW25快速读出函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 WinbondW25FastRead(int32 addr, int32 length, Uint8 *buffer)
{
    Uint8 lv_cmd[5];

    if (NULL == buffer)
    {
        return NORMAL_ERROR;
    }

    if (0 == length)
    {
        return NORMAL_SUCCESS;
    }

    lv_cmd[0] = WINBOND_W25_CMD_FASTREAD;
    lv_cmd[1] = HLSB(addr);
    lv_cmd[2] = LHSB(addr);
    lv_cmd[3] = LLSB(addr);
    lv_cmd[4] = WINBOND_W25_DUMMY_BYTE;

//    WinbondW25ClaimBus();
    SPIXferBytes(pSPIFLASH, sizeof(lv_cmd), lv_cmd, NULL, 0);
    SPIXferBytes(pSPIFLASH, length, NULL, buffer, 1);
//    WinbondW25ReleaseBus();

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: WinbondW25PageWrite
//      Input: Uint32 addr: 写入地址
//             Uint32 length: 写入数据的长度
//             Uint8 const *buffer: 待写入数据首地址
//     Output: void
//     Return: int32: 函数执行情况
//Description: WinbondW25页写入函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 WinbondW25PageWrite(int32 addr, int32 length, Uint8 const *buffer)
{
    Uint32 lv_chunk_start_offset;
    Uint32 lv_write_chunk_len;
    Uint32 lv_spi_write_addr;
    int32 lv_remainder_length;
    Uint8 lv_cmd[4];
    Uint8 const *lv_p_write_buffer;

    if (NULL == buffer)
    {
        return NORMAL_ERROR;
    }

    lv_spi_write_addr = addr;
    lv_p_write_buffer = buffer;
    lv_remainder_length = length;

//    WinbondW25ClaimBus();

    lv_chunk_start_offset = lv_spi_write_addr % WINBOND_W25_PAGE_SIZE;
    lv_cmd[0] = WINBOND_W25_CMD_WRITE;
    while (lv_remainder_length > 0)
    {
        lv_write_chunk_len = (WINBOND_W25_PAGE_SIZE - lv_chunk_start_offset);
        if (lv_remainder_length < lv_write_chunk_len)
        {
            lv_write_chunk_len = lv_remainder_length;
        }
        lv_chunk_start_offset = 0;

        SPIXferOneByte(pSPIFLASH, WINBOND_W25_CMD_WRENABLE, 1);

        lv_cmd[1] = HLSB(lv_spi_write_addr);
        lv_cmd[2] = LHSB(lv_spi_write_addr);
        lv_cmd[3] = LLSB(lv_spi_write_addr);

        SPIXferBytes(pSPIFLASH, sizeof(lv_cmd), lv_cmd, NULL, 0);
        SPIXferBytes(pSPIFLASH, lv_write_chunk_len, lv_p_write_buffer, NULL, 1);

        if (NORMAL_SUCCESS != WinbondW25WaitReady(WINBOND_W25_PROG_TIMEOUT))
        {
            return NORMAL_ERROR;
        }

        lv_spi_write_addr += lv_write_chunk_len;
        lv_p_write_buffer += lv_write_chunk_len;
        lv_remainder_length -= lv_write_chunk_len;
    }

//    WinbondW25ReleaseBus();
    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
//   Function: WinbondW25BlockErase
//      Input: Uint32 addr: 擦出地址
//             Uint32 length: 擦出长度
//     Output: void
//     Return: int32: 函数执行情况
//Description: WinbondW25块擦出函数,擦出地址及长度都必须块对齐
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int32 WinbondW25BlockErase(int32 addr, int32 length)
{
    Uint32 i;
    Uint32 lv_block_num;
    Uint8 lv_cmd[4];

    if ((0 != (addr % WINBOND_W25_BLOCK_SIZE)) || (0 != (length % WINBOND_W25_BLOCK_SIZE)))
    {
        return NORMAL_ERROR;
    }

//    WinbondW25ClaimBus();

    lv_cmd[0] = WINBOND_W25_CMD_BLOCKERASE;
    lv_block_num = length / WINBOND_W25_BLOCK_SIZE;
    for (i = 0; i < lv_block_num; i++)
    {
        SPIXferOneByte(pSPIFLASH, WINBOND_W25_CMD_WRENABLE, 1);

        lv_cmd[1] = HLSB(addr);
        lv_cmd[2] = LHSB(addr);
        lv_cmd[3] = LLSB(addr);

        SPIXferBytes(pSPIFLASH, sizeof(lv_cmd), lv_cmd, NULL, 1);

        if (NORMAL_SUCCESS != WinbondW25WaitReady(WINBOND_W25_BLOCK_ERASE_TIMEOUT))
        {
            return NORMAL_ERROR;
        }

        addr += WINBOND_W25_BLOCK_SIZE;
    }

//    WinbondW25ReleaseBus();

    return NORMAL_SUCCESS;
}

//----------------------------------------------------------------------------------------------------
// 本地函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: WinbondW25WaitReady
//      Input: Uint32 timeout: 最长等待时间
//     Output: void
//     Return: int32: 函数执行情况
//Description: WinbondW25等待操作完成函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
static int32 WinbondW25WaitReady(Uint32 timeout)
{
    Uint8  lv_status;
    Uint32 lv_count;

    SPIXferOneByte(pSPIFLASH, WINBOND_W25_CMD_RD_STATUS, 0);

    lv_count = timeout;
    do
    {
        SPIXferBytes(pSPIFLASH, 1, NULL, &lv_status, 0);
        if (0 == (lv_status & WINBOND_W25_STATUS_BUSY))
        {
            break;
        }

        lv_count--;
    } while (0 != lv_count);

    SPIXferBytes(pSPIFLASH, 0, NULL, NULL, 1);

    if (0 != lv_count)
    {
        return NORMAL_SUCCESS;
    }
    else
    {
        return NORMAL_ERROR;
    }
}



