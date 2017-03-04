/*****************************************************************************************************
* FileName     :    Utf8.c
*
* Reference    :    GNU C Library
*
* Description  :    Utf8解码编码模块
*
* History      :
*       <AUTHOR>        <DATA>        <VERSION>        <DESCRIPTION>
*      YanDengxue     2009-12-19        *.**              Created
*
*****************************************************************************************************/
//====================================================================================================
// 本文件使用的头文件
//====================================================================================================
#include "IconvBase.h"
#include "IconvData.h"

//====================================================================================================
// 函数实现
//====================================================================================================
//----------------------------------------------------------------------------------------------------
//   Function: Utf8ToUcs4
//      Input: int8 *const source: 待转换utf8字符串首地址
//             int32 source_str_length: 待转换utf8的字符串长度
//     Output: int32 *destination:  转换后的ucs4的地址
//     Return: int32: 当前utf8字符长度,返回-1程序执行失败
//Description: utf8转换成unicode,最长4字节
//    <AUTHOR>        <MODIFYTIME>            <REASON>
//   YanDengxue     2009-12-19 17:00          Created
//----------------------------------------------------------------------------------------------------
#pragma CODE_SECTION(Utf8ToUcs4, USER_SLOWCODE)
int32 Utf8ToUcs4(int8 const *source, int32 source_char_length, Uint32 *destination)
{
    int32  i;
    int32  lv_char_length;
    Uint32 lv_unicode_wchar;
    Uint8  lv_inital_char;

    if ((NULL == source) || (NULL == destination))
    {
        return -1;
    }

    lv_inital_char = source[0];
    if (0 == (lv_inital_char & 0x80))// 首字符小于0x80,则直接拷贝
    {
        lv_unicode_wchar = lv_inital_char;
        lv_char_length = 1;
    }
    else// 首字符大于0x80,转换成Unicode
    {
        // 计算UTF-8编码的字符长度
        lv_char_length = 0;
        while (0 != (lv_inital_char & 0x80))
        {
            lv_char_length++;
            lv_inital_char = lv_inital_char << 1;
        }

        // 只处理UTF-8编码长度小于6字节的字符
        if ((source_char_length < lv_char_length) || (lv_char_length > 6) )
        {
            return -1;
        }

        // UTF-8解码为unsigned int
        lv_unicode_wchar = ((Uint32)lv_inital_char) >> lv_char_length;// lv_inital_char必须为uchar
        for (i = 1; i < lv_char_length; i++)
        {
            if (0x80 != (source[i] & 0xc0))// 前两位必须为10
            {
                return -1;
            }
            lv_unicode_wchar = (lv_unicode_wchar << 6) + (source[i] & 0x3f);
        }
    }

    *destination = lv_unicode_wchar;
    return lv_char_length;
}

