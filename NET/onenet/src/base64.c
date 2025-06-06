/**
    ************************************************************
    ************************************************************
    ************************************************************
        文件名：     base64.c

        作者：

        日期：         2019-08-01

        版本：         V1.0

        说明：         Base64编解码

        修改记录：
    ************************************************************
    ************************************************************
    ************************************************************
**/

#include "base64.h"

//C库
#include <stdint.h>
#include <stdio.h>


#define BASE64_SIZE_T_MAX   ( (size_t) -1 ) /* SIZE_T_MAX is not standard */


static const unsigned char base64_enc_map[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
};


static const unsigned char base64_dec_map[128] =
{
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127,  62, 127, 127, 127,  63,  52,  53,
    54,  55,  56,  57,  58,  59,  60,  61, 127, 127,
    127,  64, 127, 127, 127,   0,   1,   2,   3,   4,
    5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
    25, 127, 127, 127, 127, 127, 127,  26,  27,  28,
    29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
    39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51, 127, 127, 127, 127, 127
};


/*
************************************************************
*    函数名称：    BASE64_Encode
*
*    函数功能：    Base64编码
*
*    入口参数：    dst：保存数据的缓存区
*                dlen：保存数据的缓存区长度
*                olen：记录写入的字节
*                src：输入数据
*                slen：输入数据的长度
*
*    返回参数：    0-成功    其他-失败
*
*    说明：        
************************************************************
*/
int BASE64_Encode(unsigned char *dst, size_t dlen, size_t *olen,
                          const unsigned char *src, size_t slen)
{
    
    size_t i = 0, n = 0;
    int C1 = 0, C2 = 0, C3 = 0;
    unsigned char *p = (void *)0;

    if(slen == 0)
    {
        *olen = 0;
        return 0;
    }

    n = slen / 3 + (slen % 3 != 0);

    if(n > (BASE64_SIZE_T_MAX - 1) / 4)
    {
        *olen = BASE64_SIZE_T_MAX;
        return MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL;
    }

    n *= 4;

    if((dlen < n + 1) || (NULL == dst))
    {
        *olen = n + 1;
        return MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL;
    }

    n = (slen / 3) * 3;

    for(i = 0, p = dst; i < n; i += 3)
    {
        C1 = *src++;
        C2 = *src++;
        C3 = *src++;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 &  3) << 4) + (C2 >> 4)) & 0x3F];
        *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
        *p++ = base64_enc_map[C3 & 0x3F];
    }

    if(i < slen)
    {
        C1 = *src++;
        C2 = ((i + 1) < slen) ? *src++ : 0;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

        if((i + 1) < slen)
            *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];

        else *p++ = '=';

        *p++ = '=';
    }

    *olen = p - dst;
    *p = 0;

    return 0;
    
}

/*
************************************************************
*    函数名称：    BASE64_Decode
*
*    函数功能：    Base64解码
*
*    入口参数：    dst：保存数据的缓存区
*                dlen：保存数据的缓存区长度
*                olen：记录写入的字节
*                src：输入数据
*                slen：输入数据的长度
*
*    返回参数：    0-成功    其他-失败
*
*    说明：        
************************************************************
*/
int BASE64_Decode(unsigned char *dst, size_t dlen, size_t *olen,
                          const unsigned char *src, size_t slen)
{
    
    size_t i = 0, n = 0;
    uint32_t j = 0, x = 0;
    unsigned char *p = (void *)0;

    /* First pass: check for validity and get output length */
    for(i = n = j = 0; i < slen; i++)
    {
        /* Skip spaces before checking for EOL */
        x = 0;

        while(i < slen && src[i] == ' ')
        {
            ++i;
            ++x;
        }

        /* Spaces at end of buffer are OK */
        if(i == slen)
            break;

        if((slen - i) >= 2 &&
                src[i] == '\r' && src[i + 1] == '\n')
            continue;

        if(src[i] == '\n')
            continue;

        /* Space inside a line is an error */
        if(x != 0)
            return MBEDTLS_ERR_BASE64_INVALID_CHARACTER;

        if(src[i] == '=' && ++j > 2)
            return MBEDTLS_ERR_BASE64_INVALID_CHARACTER;

        if(src[i] > 127 || base64_dec_map[src[i]] == 127)
            return MBEDTLS_ERR_BASE64_INVALID_CHARACTER;

        if(base64_dec_map[src[i]] < 64 && j != 0)
            return MBEDTLS_ERR_BASE64_INVALID_CHARACTER;

        n++;
    }

    if(n == 0)
    {
        *olen = 0;
        return 0;
    }

    /*  The following expression is to calculate the following formula without
        risk of integer overflow in n:
           n = ( ( n * 6 ) + 7 ) >> 3;
    */
    n = (6 * (n >> 3)) + ((6 * (n & 0x7) + 7) >> 3);
    n -= j;

    if(dst == NULL || dlen < n)
    {
        *olen = n;
        return MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL;
    }

    for(j = 3, n = x = 0, p = dst; i > 0; i--, src++)
    {
        if(*src == '\r' || *src == '\n' || *src == ' ')
            continue;

        j -= (base64_dec_map[*src] == 64);
        x  = (x << 6) | (base64_dec_map[*src] & 0x3F);

        if(++n == 4)
        {
            n = 0;

            if(j > 0)
                *p++ = (unsigned char)(x >> 16);

            if(j > 1)
                *p++ = (unsigned char)(x >>  8);

            if(j > 2)
                *p++ = (unsigned char)(x);
        }
    }

    *olen = p - dst;

    return 0;
    
}
