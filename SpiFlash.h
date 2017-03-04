/*****************************************************************************************************
* FileName:                    SpiFlash.h
*
* Description:                 SpiFlash操作相关头文件
*
* Author:                      YanDengxue, Fiberhome-Fuhua
*
* Rev History:
*       <Author>        <Data>        <Hardware>     <Version>        <Description>
*     YanDengxue   2011-03-29 15:30       --           1.00             Create
*****************************************************************************************************/
#ifndef _Spi_Flash_H
#define _Spi_Flash_H

#ifdef __cplusplus
extern "C" {
#endif

//====================================================================================================
// 宏定义
//====================================================================================================
#define WINBOND_W25_ID_W25X16         0x3015u
#define WINBOND_W25_ID_W25X32         0x3016u
#define WINBOND_W25_ID_W25X64         0x3017u

#define WINBOND_W25_CLK               50000000u
#define WINBOND_W25_DATA_LENGTH       8u
#define WINBOND_W25_PRESCALE          (((FREQ_PLL0_SYSCLK2 / WINBOND_W25_CLK) - 1u) & 0xffu)

#define WINBOND_W25_STATUS_BUSY       0x01u// Write-in-Progress

#define WINBOND_W25_CMD_WRENABLE       0x06u
#define WINBOND_W25_CMD_WRDISABLE      0x04u
#define WINBOND_W25_CMD_RD_STATUS      0x05u
#define WINBOND_W25_CMD_WR_STATUS      0x01u
#define WINBOND_W25_CMD_READ           0x03u
#define WINBOND_W25_CMD_WRITE          0x02u
#define WINBOND_W25_CMD_FASTREAD       0x0Bu
#define WINBOND_W25_CMD_FASTREAD_DUAL_OUTPUT   0x3Bu
#define WINBOND_W25_CMD_BLOCKERASE         0xD8u
#define WINBOND_W25_CMD_SECTORERASE        0x20u
#define WINBOND_W25_CMD_CHIPERASE          0xC7u
#define WINBOND_W25_CMD_POWERDOWN          0xB9u
#define WINBOND_W25_CMD_RELEASE_POWERDOWN  0xABu
#define WINBOND_W25_CMD_SIMPLE_ID          0x90u
#define WINBOND_W25_CMD_JEDEC_ID           0x9Fu

#define WINBOND_W25_DUMMY_BYTE  0x00u

#define WINBOND_W25_PAGE_SIZE    0x100u
#define WINBOND_W25_SECTOR_SIZE  0x1000u
#define WINBOND_W25_BLOCK_SIZE   0x10000u

// Common parameters
#define WINBOND_W25_CFG_HZ                1000u
#define WINBOND_W25_PROG_TIMEOUT          ((100000u * WINBOND_W25_CFG_HZ) / 1000u)
#define WINBOND_W25_PAGE_ERASE_TIMEOUT    ((500000u * WINBOND_W25_CFG_HZ) / 1000u)
#define WINBOND_W25_SECTOR_ERASE_TIMEOUT  (10000u * WINBOND_W25_CFG_HZ)
#define WINBOND_W25_BLOCK_ERASE_TIMEOUT   (10000u * WINBOND_W25_CFG_HZ)

#define pSPIFLASH pSPI0

//====================================================================================================
// 外部函数声明
//====================================================================================================
extern int32 WinbondW25ClaimBus(void);
extern int32 WinbondW25ReleaseBus(void);
extern int32 WinbondW25Read(int32 addr, int32 length, Uint8 *buffer);
extern int32 WinbondW25FastRead(int32 addr, int32 length, Uint8 *buffer);
extern int32 WinbondW25ReadLine(int32 addr, int32 *p_length, Uint8 *buffer);
extern int32 WinbondW25PageWrite(int32 addr, int32 length, Uint8 const *buffer);
extern int32 WinbondW25SectorErase(int32 addr, int32 length);
extern int32 WinbondW25BlockErase(int32 addr, int32 length);

#ifdef __cplusplus
}
#endif

#endif

