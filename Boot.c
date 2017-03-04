/*****************************************************************************************************
* FileName:                    boot.c
*
* Description:                 程序入口
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
#include "SystemBase.h"
#include "C6747Register.h"
#include "Debug.h"
#include "Spi.h"
#include "SpiFlash.h"
#include "Debug.h"

//====================================================================================================
// 宏定义
//====================================================================================================
#define DOWNLOADER_BUFFER_SIZE  512u

#define FRAME_BUFFER_SIZE       512u
#define FRAME_BUFFER_MASK         (FRAME_BUFFER_SIZE - 1u)// 串口接收缓存长度MASK
#define FRAME_BUFFER_ADDR(addr)   ((Uint16)((addr) & (FRAME_BUFFER_MASK)))// 串口接收缓存长度有效寻址

#define UART_RX_BUF_SIZE         512u// 串口接收缓存长度
#define UART_RX_BUF_MASK         (UART_RX_BUF_SIZE - 1u)// 串口接收缓存长度MASK
#define UART_RX_BUF_ADDR(addr)   ((Uint16)((addr) & (UART_RX_BUF_MASK)))// 串口接收缓存长度有效寻址

//====================================================================================================
// 本文件引用的外部函数声明
//====================================================================================================
extern int32 HardwareInitial(void);

static int32 ProgramDownload(void);

//====================================================================================================
// 本文件定义的与外部的接口变量
//====================================================================================================
VUint32 system_ms_count;
VUint16 uart_rx_buffer_rd_ptr;
VUint16 uart_rx_buffer_wr_ptr;
VUint8  uart_rx_buffer[UART_RX_BUF_SIZE];

//====================================================================================================
// 本地变量声明,此处声明的变量不与外部接口
//====================================================================================================
static Uint32 system_ms_timer0b12_last_value;
static Uint8 download_buffer[2 * 1024u * 1024u];

//====================================================================================================
// 函数实现
//====================================================================================================
//----------------------------------------------------------------------------------------------------
// 接口函数
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//   Function: main
//      Input: void
//     Output: void
//     Return: int32: 函数执行情况
//Description: 系统主函数入口
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
int main(void)
{
    Uint32  lv_uart_rx_system_ms_count_last_value;

    FeedWatchDog();
    system_ms_count = 0;
    uart_rx_buffer_rd_ptr = 0;
    uart_rx_buffer_wr_ptr = 0;

    // 初始化硬件
    HardwareInitial();

    DebugSendString("--------------------------------------------------\r\n");
    DebugSendString("Eeprom Boot Compile Time: "__DATE__" "__TIME__"\r\n");
    DebugSendString("Eeprom Boot Author      : YanDengxue\r\n");
    DebugSendString("--------------------------------------------------\r\n");
    DebugSendString("Enter eeprom boot mode!\r\n");

    while (1)
    {
        lv_uart_rx_system_ms_count_last_value = system_ms_count;
        while (uart_rx_buffer_wr_ptr == uart_rx_buffer_rd_ptr)
        {
            if ((system_ms_count - lv_uart_rx_system_ms_count_last_value) >= 2000u)
            {
                lv_uart_rx_system_ms_count_last_value += 2000u;
                DebugSendChar(NAK);
            }
        }
        ProgramDownload();
    }
}

//----------------------------------------------------------------------------------------------------
//   Function: Timer0InterruptService
//      Input: void
//     Output: void
//     Return: void
//Description: Timer0中断处理函数
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2011-03-21 16:30           Create
//----------------------------------------------------------------------------------------------------
#pragma INTERRUPT(Timer0InterruptService)
void Timer0InterruptService(void)
{
    Uint32 i;
    Uint32 lv_time_cal;

    lv_time_cal = pTIMER0->TIM12;
    if ((lv_time_cal - system_ms_timer0b12_last_value) >=  SYSTRM_MS_TICKS)
    {
        system_ms_count++;
        system_ms_timer0b12_last_value += SYSTRM_MS_TICKS;
    }

    FeedWatchDog();

    for (i = 0; i < 0x10; i++)
    {
        if (0 == (pDEBUG->LSR & UART_LSR_DR))
        {
            break;
        }
        uart_rx_buffer[UART_RX_BUF_ADDR(uart_rx_buffer_wr_ptr++)] = pDEBUG->RBR;
    }
}

static int32 ProgramDownload(void)
{
    Uint32  i;
    Uint8   lv_xmodem_frame_counter;
    Uint16  lv_uart_rx_buffer_rd_ptr_temp;
    Uint32  lv_uart_rx_system_ms_count_last_value;
    Uint8   lv_buffer[SELF_BIN_HEADER_LENGTH];
    Uint16  lv_bin_file_type;
    Uint32  lv_bin_file_length;
    Uint32  lv_flash_file_length;
    Uint32  lv_max_file_length;
    Uint32  lv_spi_flash_start_addr;
    Uint32  lv_current_frame_valid_data_length;
    Uint32  lv_bin_file_downloading_data_count;

    if (SOH != uart_rx_buffer[UART_RX_BUF_ADDR(uart_rx_buffer_rd_ptr)])
    {
        goto error_handler;
    }

    lv_uart_rx_system_ms_count_last_value = system_ms_count;
    lv_uart_rx_buffer_rd_ptr_temp = uart_rx_buffer_wr_ptr;
    while (UART_RX_BUF_ADDR(uart_rx_buffer_wr_ptr - uart_rx_buffer_rd_ptr) < (SELF_BIN_HEADER_LENGTH + 3u))
    {
        if (lv_uart_rx_buffer_rd_ptr_temp != uart_rx_buffer_wr_ptr)
        {
            lv_uart_rx_buffer_rd_ptr_temp = uart_rx_buffer_wr_ptr;
            lv_uart_rx_system_ms_count_last_value = system_ms_count;
        }
        else
        {
            if ((system_ms_count - lv_uart_rx_system_ms_count_last_value) >= 60000u)
            {
                goto error_handler;
            }
        }
    }

    if (    (0x01u != uart_rx_buffer[UART_RX_BUF_ADDR(uart_rx_buffer_rd_ptr + 1)])
         || ((Uint8)(~0x01u) != uart_rx_buffer[UART_RX_BUF_ADDR(uart_rx_buffer_rd_ptr + 2)]))
    {
        goto error_handler;
    }

    for (i = 0; i < SELF_BIN_HEADER_LENGTH; i++)
    {
        lv_buffer[i] = uart_rx_buffer[UART_RX_BUF_ADDR(uart_rx_buffer_rd_ptr + 3 + i)];
    }

    if (    ((SELF_BIN_HEADER_LENGTH >> 2u) != lv_buffer[4])
         || (SELF_BIN_VERSION != lv_buffer[5])
         || ((0x01u) != lv_buffer[SELF_BIN_HEADER_LENGTH - 2])
         || ((0xA0u) != lv_buffer[SELF_BIN_HEADER_LENGTH - 1]))
    {
        goto error_handler;
    }

    lv_bin_file_length = (lv_buffer[0]) | (lv_buffer[1] << 8) | (lv_buffer[2] << 16) | (lv_buffer[3] << 24);
    lv_flash_file_length = lv_bin_file_length + SELF_BIN_HEADER_LENGTH;
    lv_bin_file_type = (lv_buffer[6]) | (lv_buffer[7] << 8);
    switch (lv_bin_file_type)
    {
        case 0x01:
        {
            lv_max_file_length = FLASH_APP_LENGTH;
            lv_spi_flash_start_addr = FLASH_APP_START_ADDR;
            break;
        }
        case 0x02:
        {
            lv_max_file_length = FLASH_FPGA_LENGTH;
            lv_spi_flash_start_addr = FLASH_FPGA_START_ADDR;
            break;
        }
        case 0x10:
        {
            lv_max_file_length = FLASH_DEV_CFG_LENGTH;
            lv_spi_flash_start_addr = FLASH_DEV_CFG_START_ADDR;
            break;
        }
        case 0x11:
        {
            lv_max_file_length = FLASH_PRJ_CFG_LENGTH;
            lv_spi_flash_start_addr = FLASH_PRJ_CFG_START_ADDR;
            break;
        }
        default:
        {
            goto error_handler;
        }
    }

    if (lv_flash_file_length > lv_max_file_length)
    {
        goto error_handler;
    }

    lv_xmodem_frame_counter = 1;
    lv_bin_file_downloading_data_count = 0;
    while (lv_bin_file_downloading_data_count < lv_flash_file_length)
    {
        lv_uart_rx_buffer_rd_ptr_temp = uart_rx_buffer_wr_ptr;
        lv_uart_rx_system_ms_count_last_value = system_ms_count;
        while (UART_RX_BUF_ADDR(uart_rx_buffer_wr_ptr - uart_rx_buffer_rd_ptr) < 132u)
        {
            if (lv_uart_rx_buffer_rd_ptr_temp != uart_rx_buffer_wr_ptr)
            {
                lv_uart_rx_buffer_rd_ptr_temp = uart_rx_buffer_wr_ptr;
                lv_uart_rx_system_ms_count_last_value = system_ms_count;
            }
            else
            {
                if ((system_ms_count - lv_uart_rx_system_ms_count_last_value) >= 60000u)
                {
                    goto error_handler;
                }
            }
        }

        if (    (SOH != uart_rx_buffer[UART_RX_BUF_ADDR(uart_rx_buffer_rd_ptr)])
             || (lv_xmodem_frame_counter != uart_rx_buffer[UART_RX_BUF_ADDR(uart_rx_buffer_rd_ptr + 1)])
             || ((Uint8)(~lv_xmodem_frame_counter) != uart_rx_buffer[UART_RX_BUF_ADDR(uart_rx_buffer_rd_ptr + 2)]) )
        {
            goto error_handler;
        }
        lv_xmodem_frame_counter++;

        uart_rx_buffer_rd_ptr += 3;
        if ((lv_flash_file_length - lv_bin_file_downloading_data_count) < 128u)
        {
            lv_current_frame_valid_data_length = lv_flash_file_length - lv_bin_file_downloading_data_count;
        }
        else
        {
            lv_current_frame_valid_data_length = 128u;
        }
        for (i = 0; i < lv_current_frame_valid_data_length; i++)
        {
            download_buffer[lv_bin_file_downloading_data_count++] = uart_rx_buffer[UART_RX_BUF_ADDR(uart_rx_buffer_rd_ptr + i)];
        }
        uart_rx_buffer_rd_ptr += 129u;
        DebugSendChar(ACK);
    }

    lv_uart_rx_system_ms_count_last_value = system_ms_count;
    while (uart_rx_buffer_wr_ptr == uart_rx_buffer_rd_ptr)
    {
        if ((system_ms_count - lv_uart_rx_system_ms_count_last_value) >= 60000u)
        {
            goto error_handler;
        }
    }

    if (EOT != uart_rx_buffer[UART_RX_BUF_ADDR(uart_rx_buffer_rd_ptr++)])
    {
        goto error_handler;
    }

    DebugSendChar(ACK);

    if (1 == lv_bin_file_type)
    {
        WinbondW25BlockErase(lv_spi_flash_start_addr, ((lv_bin_file_length + WINBOND_W25_BLOCK_SIZE - 1) / WINBOND_W25_BLOCK_SIZE) * WINBOND_W25_BLOCK_SIZE);
        WinbondW25BlockErase((FLASH_APP_END_ADDR - FLASH_APP_END_ADDR % WINBOND_W25_BLOCK_SIZE), WINBOND_W25_BLOCK_SIZE);
        WinbondW25PageWrite(lv_spi_flash_start_addr,  lv_bin_file_length , &download_buffer[SELF_BIN_HEADER_LENGTH]);
        WinbondW25PageWrite((FLASH_APP_END_ADDR + 1 - SELF_BIN_HEADER_LENGTH),  SELF_BIN_HEADER_LENGTH , download_buffer);
    }
    else
    {
        WinbondW25BlockErase(lv_spi_flash_start_addr, ((lv_flash_file_length + WINBOND_W25_BLOCK_SIZE - 1) / WINBOND_W25_BLOCK_SIZE) * WINBOND_W25_BLOCK_SIZE);
        WinbondW25PageWrite(lv_spi_flash_start_addr,  lv_flash_file_length , download_buffer);
    }

    DebugSendString("Download success!\r\n");
    return NORMAL_SUCCESS;

//----------------------------------------------------------------------------------------------------
// 错误处理
//----------------------------------------------------------------------------------------------------
error_handler:
    uart_rx_buffer_rd_ptr = uart_rx_buffer_wr_ptr;
    return NORMAL_ERROR;
}


