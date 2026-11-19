#include "CStrUtils.h"
#include <fstream>
#include <tchar.h>

// ANSI GBK 编码标准
// 
// 汉字区
// GBK/2：0XBOA1-F7FE 收录 GB 2312 汉字 6763 个，按原序排列
// GBK/3：0X8140-AOFE，收录 CJK 汉字 6080 个
// GBK/4：0XAA40-FEAO，收录 CJK 汉字和增补的汉字 8160 个
// 
// 图形符号区
// GBK/1：0XA1A1-A9FE，除 GB 2312 的符号外，还增补了其它符号
// GBK/5：0XA840-A9AO，扩除非汉字区
// 
// 用户自定义区
// GBK 区域中的空白区，用户可以自己定义字符

// UTF-8 编码标准
// 
// 1字节 U+0000000 - U+0000007F 0xxxxxxx
// 2字节 U+0000080 - U+000007FF 110xxxxx 10xxxxxx
// 3字节 U+0000800 - U+0000FFFF 1110xxxx 10xxxxxx 10xxxxxx
// 4字节 U+0010000 - U+001FFFFF 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
// 5字节 U+0200000 - U+03FFFFFF 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
// 6字节 U+4000000 - U+7FFFFFFF 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

// UTF16 编码标准
// 
// 基本多语言平面(U+0000 - U+FFFF)
// 
// 辅助平面(U+10000 - U+10FFFF)
// 1.码位减去 0x10000，得到20位的代理值(0x00 - 0xFFFFF)
// 2.高10位（范围0 - 0x3FF）加 0xD800 得到高位代理(0xD800 - 0xDBFF)
// 3.低10位（范围0 - 0x3FF）加 0xDC00 得到低位代理(0xDC00 - 0xDFFF)

namespace CStrUtils
{
    const int FORMAT_COUNT_MAX = (1024 * 1024 * 64);

    // UTF-8 Byte count table
    unsigned char g_utf8_bytes_count_table[0x100] = {
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
        0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
        0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
        0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x05,0x05,0x05,0x05,0x06,0x06,0x00,0x00,
    };

    // UTF-8 Data mask table
    unsigned char g_utf8_data_mask_table[0x100] = {
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,
        0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,
        0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,
        0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x03,0x03,0x03,0x03,0x01,0x01,0x00,0x00,
    };

    std::string _WStrToMultiStr(UINT CodePage, const std::wstring& str)
    {
        //计算缓冲区所需的字节长度
        int cbMultiByte = ::WideCharToMultiByte(CodePage, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
        std::string strResult(cbMultiByte, 0);

        //成功则返回写入到指示的缓冲区的字节数
        size_t nConverted = ::WideCharToMultiByte(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size(), NULL, NULL);

        //调整内容长度
        strResult.resize(nConverted);
        return strResult;
    }

    std::wstring _MultiStrToWStr(UINT CodePage, const std::string& str)
    {
        //计算缓冲区所需的字符长度
        int cchWideChar = ::MultiByteToWideChar(CodePage, 0, str.c_str(), -1, NULL, 0);
        std::wstring strResult(cchWideChar, 0);

        //成功则返回写入到指示的缓冲区的字符数
        size_t nConverted = ::MultiByteToWideChar(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size());

        //调整内容长度
        strResult.resize(nConverted);
        return strResult;
    }

    std::string WStrToAStr(const std::wstring& str)
    {
        return _WStrToMultiStr(CP_ACP, str);
    }

    std::string WStrToU8Str(const std::wstring& str)
    {
        return _WStrToMultiStr(CP_UTF8, str);
    }

    _tstring WStrToTStr(const std::wstring& str)
    {
#ifdef _UNICODE
        return str;
#else
        return _WStrToMultiStr(CP_ACP, str);
#endif
    }

    std::wstring AStrToWStr(const std::string& str)
    {
        return _MultiStrToWStr(CP_ACP, str);
    }

    std::string AStrToU8Str(const std::string& str)
    {
        return WStrToU8Str(AStrToWStr(str));
    }

    _tstring AStrToTStr(const std::string& str)
    {
#ifdef _UNICODE
        return _MultiStrToWStr(CP_ACP, str);
#else
        return str;
#endif
    }

    std::wstring U8StrToWStr(const std::string& str)
    {
        return _MultiStrToWStr(CP_UTF8, str);
    }

    std::string U8StrToAStr(const std::string& str)
    {
        return WStrToAStr(U8StrToWStr(str));
    }

    _tstring U8StrToTStr(const std::string& str)
    {
#ifdef _UNICODE
        return _MultiStrToWStr(CP_UTF8, str);
#else
        return WStrToAStr(U8StrToWStr(str));
#endif
    }

    std::string TStrToAStr(const _tstring& str)
    {
#ifdef _UNICODE
        return _WStrToMultiStr(CP_ACP, str);
#else
        return str;
#endif
    }

    std::wstring TStrToWStr(const _tstring& str)
    {
#ifdef _UNICODE
        return str;
#else
        return AStrToWStr(str);
#endif
    }

    std::string TStrToU8Str(const _tstring& str)
    {
#ifdef _UNICODE
        return WStrToU8Str(str);
#else
        return WStrToU8Str(AStrToWStr(str));
#endif
    }

    int32_t _GetGbkCount(const void* pData, size_t size/* = -1*/, std::string* pGbk/* = nullptr*/)
    {
        const uint8_t* pCpData = (const uint8_t*)pData;
        std::string strOutGbk;      // 输出字符编码
        uint16_t gbkCode = 0;       // GBK编码
        int32_t nByteCount = 0;     // 字节计数
        int32_t nChCount = 0;       // 字符计数
        bool fResult = true;        // 操作结果

        while ((0 != *pCpData) && (0 != size))
        {
            uint8_t ch = *pCpData;

            // ASCII编码 兼容
            if (ch <= 0x7F)
            {
                gbkCode = ch;
                nChCount++;
            }
            else
            {
                // 高字节范围: 0x81 - 0xFE
                if (0 == nByteCount)
                {
                    gbkCode = 0;
                    if (ch >= 0x81 && ch <= 0xFE)
                    {
                        gbkCode = ch;
                        nByteCount = 1;
                    }
                    else
                    {
                        fResult = false;
                        break;
                    }
                }
                else
                {
                    // 低字节范围: 0x40 - 0xFE (不包括0x7F)
                    if (1 == nByteCount)
                    {
                        if (!(ch >= 0x40 && ch <= 0xFE) || 0x7F == ch)
                        {
                            fResult = false;
                            break;
                        }
                    }

                    gbkCode = gbkCode << 8;
                    gbkCode |= ch;

                    nByteCount--;

                    if (0 == nByteCount)
                    {
                        nChCount++;
                    }
                }
            }

            if (0 == nByteCount)
            {
                if (gbkCode <= 0x7F)
                {
                    strOutGbk.push_back((uint8_t)gbkCode);
                }
                else
                {
                    strOutGbk.push_back(gbkCode >> 8);
                    strOutGbk.push_back(gbkCode & 0xFF);
                }
            }

            pCpData++;

            if (-1 != size)
            {
                size--;
            }
        }

        if (!fResult)
        {
            return -1;
        }

        if (pGbk)
        {
            *pGbk = std::move(strOutGbk);
        }

        return nChCount;
    }

    int GetGbkCount(const void* pData, size_t size/* = -1*/)
    {
        return _GetGbkCount(pData, size, nullptr);
    }

    int GetUtf8Count(const void* data_ptr, size_t size/* = -1*/)
    {
        const uint8_t* data_pos = (const uint8_t*)data_ptr;
        size_t size_count = size;
        uint32_t code_point = 0;
        int32_t byte_count = 0;
        int32_t char_count = 0;
        bool result_flag = true;

        while ((0 != *data_pos) && (0 != size_count))
        {
            uint8_t ch = *data_pos;

            // ASCII 范围
            if (ch < 0x7F)
            {
                code_point = ch;
                char_count++;
            }
            else
            {
                if (0 == byte_count)
                {
                    code_point = 0;
                    if (ch >= 0xC0)
                    {
                        // 获取字符编码字节数
                        byte_count = g_utf8_bytes_count_table[ch];

                        // 获取首字节码点
                        code_point = ch & g_utf8_data_mask_table[ch];

                        /*
                        if (ch >= 0xC0 && ch <= 0xDF)
                        {
                            byte_count = 2;
                            cp32 = ch & 0x1F;
                        }
                        else if (ch >= 0xE0 && ch <= 0xEF)
                        {
                            byte_count = 3;
                            cp32 = ch & 0x0F;
                        }
                        else if (ch >= 0xF0 && ch <= 0xF7)
                        {
                            byte_count = 4;
                            cp32 = ch & 0x07;
                        }
                        else if (ch >= 0xF8 && ch <= 0xFB)
                        {
                            byte_count = 5;
                            cp32 = ch & 0x03;
                        }
                        else if (ch >= 0xFC && ch <= 0xFD)
                        {
                            byte_count = 6;
                            cp32 = ch & 0x01;
                        }
                        */

                        if (0 == byte_count)
                        {
                            result_flag = false;
                            break;
                        }

                        byte_count--;
                    }
                    else
                    {
                        result_flag = false;
                        break;
                    }
                }
                else
                {
                    // 非首字节码点字节掩码检查
                    if (0x80 != (ch & 0xC0))
                    {
                        result_flag = false;
                        break;
                    }

                    // 码点组合
                    code_point = code_point << 6;
                    code_point |= ch & 0x3F;

                    byte_count--;

                    // 统计字符计数, 忽略字节顺序标记(BOM)
                    if ((0 == byte_count) && (0xFEFF != code_point))
                    {
                        char_count++;
                    }
                }
            }

            data_pos++;

            if (-1 != size_count)
            {
                size_count--;
            }
        }

        if (!result_flag)
        {
            return -1;
        }

        return char_count;
    }

    int32_t GetUtf16Count(const void* data_ptr, size_t size/* = -1*/)
    {
        const uint16_t* data_pos = (const uint16_t*)data_ptr;
        size_t size_count = size;
        uint32_t code_point = 0;
        uint16_t code_point_high = 0;
        uint16_t code_point_low = 0;
        uint16_t code_point_tmp = 0;
        int32_t byte_count = 0;
        int32_t char_count = 0;
        bool big_endian_flag = false;
        bool little_endian_flag = false;
        bool result_flag = true;

        if (-1 != size_count)
        {
            if ((size_count < 2) || (0 != (size_count % 2)))
            {
                return -1;
            }
        }

        while ((0 != *data_pos) && (0 != size_count))
        {
            code_point_tmp = *data_pos;

            // 检查字节序
            if (0xFFFE == code_point_tmp || 0xFEFF == code_point_tmp)
            {
                if (0 == byte_count)
                {
                    // 小端序
                    if (0xFFFE == code_point_tmp)
                    {
                        big_endian_flag = true;
                    }

                    // 大端序
                    if (0xFEFF == code_point_tmp)
                    {
                        little_endian_flag = true;
                    }
                }
                else
                {
                    result_flag = false;
                    break;
                }

                // 不可能同时存在小端序和大端序
                if (big_endian_flag && little_endian_flag)
                {
                    result_flag = false;
                    break;
                }

                data_pos++;

                if (-1 != size_count)
                {
                    size_count -= 2;
                }

                continue;
            }

            //字节序转换
            if (big_endian_flag)
            {
                code_point_tmp = ((code_point_tmp >> 8) | (code_point_tmp << 8));
            }

            // 代理区检查
            if (!(code_point_tmp >= 0xD800 && code_point_tmp <= 0xDFFF))
            {
                if (code_point_high > 0)
                {
                    result_flag = false;
                    break;
                }

                code_point = code_point_tmp;
                char_count++;
            }
            else
            {
                if (0 == byte_count)
                {
                    // 高位代理(高10位)
                    if (code_point_tmp >= 0xD800 && code_point_tmp <= 0xDBFF)
                    {
                        code_point_high = (code_point_tmp - 0xD800);
                        byte_count = 1;
                    }
                    else
                    {
                        result_flag = false;
                        break;
                    }
                }
                else
                {
                    if (1 == byte_count)
                    {
                        // 低位代理(低10位)
                        if ((code_point_tmp >= 0xDC00) && (code_point_tmp <= 0xDFFF))
                        {
                            code_point_low = (code_point_tmp - 0xDC00);
                            code_point = 0x10000 + ((uint32_t)code_point_high << 10 | code_point_low);
                            code_point_low = 0;
                            code_point_high = 0;
                        }
                        else
                        {
                            result_flag = false;
                            break;
                        }
                    }

                    byte_count--;

                    if (0 == byte_count)
                    {
                        char_count++;
                    }
                }
            }

            data_pos++;

            if (-1 != size_count)
            {
                size_count -= 2;
            }
        }

        if (!result_flag)
        {
            return -1;
        }

        return char_count;
    }

    int GetGbkCount(const std::string& str)
    {
        return _GetGbkCount(str.c_str(), str.size(), nullptr);
    }

    int GetUtf8Count(const std::string& str)
    {
        return GetUtf8Count(str.c_str(), str.size());
    }

    int GetUtf16Count(const std::wstring& str)
    {
        return GetUtf16Count(str.c_str(), str.size() * sizeof(wchar_t));
    }

    std::string GetUpperStrA(const std::string& str)
    {
        std::string strResult = str;
        for (auto& item : strResult)
        {
            if (item >= _T('a') && item <= _T('z')) item -= 0x20;
        }

        return strResult;
    }

    std::wstring GetUpperStrW(const std::wstring& str)
    {
        std::wstring strResult = str;
        for (auto& item : strResult)
        {
            if (item >= _T('a') && item <= _T('z')) item -= 0x20;
        }

        return strResult;
    }

    _tstring GetUpperStr(const _tstring& str)
    {
#ifdef _UNICODE
        return GetUpperStrW(str);
#else
        return GetUpperStrA(str);
#endif
    }

    std::string GetLowerStrA(const std::string& str)
    {
        std::string strResult = str;
        for (auto& item : strResult)
        {
            if (item >= _T('A') && item <= _T('Z')) item += 0x20;
        }

        return strResult;
    }

    std::wstring GetLowerStrW(const std::wstring& str)
    {
        std::wstring strResult = str;
        for (auto& item : strResult)
        {
            if (item >= _T('A') && item <= _T('Z')) item += 0x20;
        }

        return strResult;
    }

    _tstring GetLowerStr(const _tstring& str)
    {
#ifdef _UNICODE
        return GetLowerStrW(str);
#else
        return GetLowerStrA(str);
#endif
    }

    std::string& ToUpperStrA(std::string& str)
    {
        for (auto& item : str)
        {
            if (item >= _T('a') && item <= _T('z')) item -= 0x20;
        }

        return str;
    }

    std::wstring& ToUpperStrW(std::wstring& str)
    {
        for (auto& item : str)
        {
            if (item >= _T('a') && item <= _T('z')) item -= 0x20;
        }

        return str;
    }

    _tstring& ToUpperStr(_tstring& str)
    {
#ifdef _UNICODE
        return ToUpperStrW(str);
#else
        return ToUpperStrA(str);
#endif
    }

    std::string& ToLowerStrA(std::string& str)
    {
        for (auto& item : str)
        {
            if (item >= _T('A') && item <= _T('Z')) item += 0x20;
        }

        return str;
    }

    std::wstring& ToLowerStrW(std::wstring& str)
    {
        for (auto& item : str)
        {
            if (item >= _T('A') && item <= _T('Z')) item += 0x20;
        }

        return str;
    }

    _tstring& ToLowerStr(_tstring& str)
    {
#ifdef _UNICODE
        return ToLowerStrW(str);
#else
        return ToLowerStrA(str);
#endif
    }

    double StrToDoubleA(const std::string& str)
    {
        return ::strtod(str.c_str(), nullptr);
    }

    double StrToDoubleW(const std::wstring& str)
    {
        return ::wcstod(str.c_str(), nullptr);
    }

    double StrToDouble(const _tstring& str)
    {
        return ::_tcstod(str.c_str(), nullptr);
    }

    std::string DoubleToStrA(double val)
    {
        char szBuf[MAX_PATH] = { 0 };
        ::sprintf_s(szBuf, _countof(szBuf), "%g", val);
        return szBuf;
    }

    std::wstring DoubleToStrW(double val)
    {
        wchar_t szBuf[MAX_PATH] = { 0 };
        ::swprintf_s(szBuf, _countof(szBuf), L"%g", val);
        return szBuf;
    }

    _tstring DoubleToStr(double val)
    {
        TCHAR szBuf[MAX_PATH] = { 0 };
        ::_stprintf_s(szBuf, _countof(szBuf), _T("%g"), val);
        return szBuf;
    }

    int64_t StrToIntA(const std::string& str, int radix/* = 10*/)
    {
        return ::strtoll(str.c_str(), nullptr, radix);
    }

    int64_t StrToIntW(const std::wstring& str, int radix/* = 10*/)
    {
        return ::wcstoll(str.c_str(), nullptr, radix);
    }

#ifndef StrToInt
    int64_t StrToInt(const _tstring& str, int radix/* = 10*/)
    {
        return ::_tcstoll(str.c_str(), nullptr, radix);
    }
#endif

    std::string IntToStrA(int64_t val, int radix/* = 10*/)
    {
        char szBuf[MAX_PATH] = { 0 };
        ::_i64toa_s(val, szBuf, _countof(szBuf), radix);
        return szBuf;
    }

    std::wstring IntToStrW(int64_t val, int radix/* = 10*/)
    {
        wchar_t szBuf[MAX_PATH] = { 0 };
        ::_i64tow_s(val, szBuf, _countof(szBuf), radix);
        return szBuf;
    }

    _tstring IntToStr(int64_t val, int radix/* = 10*/)
    {
        TCHAR szBuf[MAX_PATH] = { 0 };
        ::_i64tot_s(val, szBuf, _countof(szBuf), radix);
        return szBuf;
    }

    uint64_t StrToUIntA(const std::string& str, int radix/* = 10*/)
    {
        return ::strtoull(str.c_str(), nullptr, radix);
    }

    uint64_t StrToUIntW(const std::wstring& str, int radix/* = 10*/)
    {
        return ::wcstoull(str.c_str(), nullptr, radix);
    }

    uint64_t StrToUInt(const _tstring& str, int radix/* = 10*/)
    {
        return ::_tcstoull(str.c_str(), nullptr, radix);
    }

    std::string UIntToStrA(uint64_t val, int radix/* = 10*/)
    {
        char szBuf[MAX_PATH] = { 0 };
        ::_ui64toa_s(val, szBuf, _countof(szBuf), radix);
        return szBuf;
    }

    std::wstring UIntToStrW(uint64_t val, int radix/* = 10*/)
    {
        wchar_t szBuf[MAX_PATH] = { 0 };
        ::_ui64tow_s(val, szBuf, _countof(szBuf), radix);
        return szBuf;
    }

    _tstring UIntToStr(uint64_t val, int radix/* = 10*/)
    {
        TCHAR szBuf[MAX_PATH] = { 0 };
        ::_ui64tot_s(val, szBuf, _countof(szBuf), radix);
        return szBuf;
    }

    size_t FindNoCaseA(const std::string& strMain, const std::string& strSub, size_t offset/* = 0*/)
    {
        size_t findPos = std::string::npos;

        do
        {
            // 主串长度比字串小, 不需要比较
            if (strMain.size() < strSub.size() || offset > (strMain.size() - 1))
            {
                break;
            }

            std::string strMainTmp = strMain;
            std::string strSubTmp = strSub;

            // 转换大小写
            for (auto& item : strMainTmp)
            {
                if (item >= _T('A') && item <= _T('Z')) item += 0x20;
            }

            for (auto& item : strSubTmp)
            {
                if (item >= _T('A') && item <= _T('Z')) item += 0x20;
            }

            //findPos = strMainTmp.find(strSubTmp, offset);
            const char* pFind = std::strstr(strMainTmp.c_str() + offset, strSubTmp.c_str());
            findPos = pFind ? pFind - strMainTmp.c_str() : findPos;

        } while (false);

        return findPos;
    }

    size_t FindNoCaseW(const std::wstring& strMain, const std::wstring& strSub, size_t offset/* = 0*/)
    {
        size_t findPos = std::wstring::npos;

        do
        {
            // 主串长度比字串小, 不需要比较
            if (strMain.size() < strSub.size() || offset > (strMain.size() - 1))
            {
                break;
            }

            std::wstring strMainTmp = strMain;
            std::wstring strSubTmp = strSub;

            // 转换大小写
            for (auto& item : strMainTmp)
            {
                if (item >= _T('A') && item <= _T('Z')) item += 0x20;
            }

            for (auto& item : strSubTmp)
            {
                if (item >= _T('A') && item <= _T('Z')) item += 0x20;
            }

            //findPos = strMainTmp.find(strSubTmp, offset);
            const wchar_t* pFind = std::wcsstr(strMainTmp.c_str() + offset, strSubTmp.c_str());
            findPos = pFind ? pFind - strMainTmp.c_str() : findPos;

        } while (false);

        return findPos;
    }

    size_t FindNoCase(const _tstring& strMain, const _tstring& strSub, size_t offset/* = 0*/)
    {
#ifdef _UNICODE
        return FindNoCaseW(strMain, strSub, offset);
#else
        return FindNoCaseA(strMain, strSub, offset);
#endif
    }

    size_t FindFirstNotOfNoCaseA(const std::string& strMain, const std::string& strSub, size_t offset/* = 0*/)
    {
        size_t findPos = std::string::npos;

        do
        {
            // 主串长度比字串小, 不需要比较
            if (strMain.size() < strSub.size())
            {
                break;
            }

            std::string strMainTmp = strMain;
            std::string strSubTmp = strSub;

            // 转换大小写
            for (auto& item : strMainTmp)
            {
                if (item >= _T('A') && item <= _T('Z')) item += 0x20;
            }

            for (auto& item : strSubTmp)
            {
                if (item >= _T('A') && item <= _T('Z')) item += 0x20;
            }

            findPos = strMainTmp.find_first_not_of(strSubTmp, offset);

        } while (false);

        return findPos;
    }

    size_t FindFirstNotOfNoCaseW(const std::wstring& strMain, const std::wstring& strSub, size_t offset/* = 0*/)
    {
        size_t findPos = std::wstring::npos;

        do
        {
            // 主串长度比字串小, 不需要比较
            if (strMain.size() < strSub.size())
            {
                break;
            }

            std::wstring strMainTmp = strMain;
            std::wstring strSubTmp = strSub;

            // 转换大小写
            for (auto& item : strMainTmp)
            {
                if (item >= _T('A') && item <= _T('Z')) item += 0x20;
            }

            for (auto& item : strSubTmp)
            {
                if (item >= _T('A') && item <= _T('Z')) item += 0x20;
            }

            findPos = strMainTmp.find_first_not_of(strSubTmp, offset);

        } while (false);

        return findPos;
    }

    size_t FindFirstNotOfNoCase(const _tstring& strMain, const _tstring& strSub, size_t offset/* = 0*/)
    {
#ifdef _UNICODE
        return FindFirstNotOfNoCaseW(strMain, strSub, offset);
#else
        return FindFirstNotOfNoCaseA(strMain, strSub, offset);
#endif
    }

    int CompareNoCaseA(const std::string& strSrc, const std::string& strDest)
    {
        return _stricmp(strSrc.c_str(), strDest.c_str());
    }

    int CompareNoCaseW(const std::wstring& strSrc, const std::wstring& strDest)
    {
        return _wcsicmp(strSrc.c_str(), strDest.c_str());
    }

    int CompareNoCase(const _tstring& strSrc, const _tstring& strDest)
    {
#ifdef _UNICODE
        return CompareNoCaseW(strSrc, strDest);
#else
        return CompareNoCaseA(strSrc, strDest);
#endif
    }

    size_t FindA(const std::string& strMain, const std::string& strSub, size_t offset/* = 0*/, bool fCase/* = false*/)
    {
        if (fCase)
        {
            //return strMain.find(strSub, offset);

            if (offset <= (strMain.size() - 1))
            {
                const char* pFind = std::strstr(strMain.c_str() + offset, strSub.c_str());
                if (pFind)
                {
                    return pFind - strMain.c_str();
                }
            }

            return std::wstring::npos;
        }

        return FindNoCaseA(strMain, strSub, offset);
    }

    size_t FindW(const std::wstring& strMain, const std::wstring& strSub, size_t offset/* = 0*/, bool fCase/* = false*/)
    {
        if (fCase)
        {
            //return strMain.find(strSub, offset);

            if (offset <= (strMain.size() - 1))
            {
                const wchar_t* pFind = std::wcsstr(strMain.c_str() + offset, strSub.c_str());
                if (pFind)
                {
                    return pFind - strMain.c_str();
                }
            }

            return std::wstring::npos;
        }

        return FindNoCaseW(strMain, strSub, offset);
    }

    size_t Find(const _tstring& strMain, const _tstring& strSub, size_t offset/* = 0*/, bool fCase/* = false*/)
    {
#ifdef _UNICODE
        return FindW(strMain, strSub, offset, fCase);
#else
        return FindA(strMain, strSub, offset, fCase);
#endif
    }

    std::string& ReplaceNoCaseA(std::string& strMain, const std::string& strSub, const std::string& strTarget)
    {
        if (strSub.empty())
        {
            return strMain;
        }

        size_t nFind = 0;
        while (_tstring::npos != (nFind = FindNoCaseA(strMain, strSub, nFind)))
        {
            strMain.replace(nFind, strSub.size(), strTarget);
            nFind += strTarget.size();
        };

        return strMain;
    }

    std::wstring& ReplaceNoCaseW(std::wstring& strMain, const std::wstring& strSub, const std::wstring& strTarget)
    {
        if (strSub.empty())
        {
            return strMain;
        }

        size_t nFind = 0;
        while (_tstring::npos != (nFind = FindNoCaseW(strMain, strSub, nFind)))
        {
            strMain.replace(nFind, strSub.size(), strTarget);
            nFind += strTarget.size();
        };

        return strMain;
    }

    _tstring& ReplaceNoCase(_tstring& strMain, const _tstring& strSub, const _tstring& strTarget)
    {
#ifdef _UNICODE
        return ReplaceNoCaseW(strMain, strSub, strTarget);
#else
        return ReplaceNoCaseA(strMain, strSub, strTarget);
#endif
    }

    std::string& ReplaceA(std::string& strMain, const std::string& strSub, const std::string& strTarget, bool fCase/* = false*/)
    {
        if (strSub.empty())
        {
            return strMain;
        }

        if (fCase)
        {
            size_t nFind = 0;
            while (std::string::npos != (nFind = strMain.find(strSub, nFind)))
            {
                strMain.replace(nFind, strSub.size(), strTarget);
                nFind += strTarget.size();
            }
        }
        else
        {
            return ReplaceNoCaseA(strMain, strSub, strTarget);
        }

        return strMain;
    }

    std::wstring& ReplaceW(std::wstring& strMain, const std::wstring& strSub, const std::wstring& strTarget, bool fCase/* = false*/)
    {
        if (strSub.empty())
        {
            return strMain;
        }

        if (fCase)
        {
            size_t nFind = 0;
            while (std::wstring::npos != (nFind = strMain.find(strSub, nFind)))
            {
                strMain.replace(nFind, strSub.size(), strTarget);
                nFind += strTarget.size();
            };
        }
        else
        {
            return ReplaceNoCaseW(strMain, strSub, strTarget);
        }

        return strMain;
    }

    _tstring& Replace(_tstring& strMain, const _tstring& strSub, const _tstring& strTarget, bool fCase/* = false*/)
    {
#ifdef _UNICODE
        return ReplaceW(strMain, strSub, strTarget, fCase);
#else
        return ReplaceA(strMain, strSub, strTarget, fCase);
#endif
    }

    int CompareA(const std::string& strSrc, const std::string& strDest, bool fCase /* = true*/)
    {
        if (fCase)
        {
            return strcmp(strSrc.c_str(), strDest.c_str());
        }

        return _stricmp(strSrc.c_str(), strDest.c_str());
    }

    int CompareW(const std::wstring& strSrc, const std::wstring& strDest, bool fCase /* = true*/)
    {
        if (fCase)
        {
            return wcscmp(strSrc.c_str(), strDest.c_str());
        }

        return _wcsicmp(strSrc.c_str(), strDest.c_str());
    }

    int Compare(const _tstring& strSrc, const _tstring& strDest, bool fCase /* = true*/)
    {
#ifdef _UNICODE
        return CompareW(strSrc, strDest, fCase);
#else
        return CompareA(strSrc, strDest, fCase);
#endif
    }

    std::vector<std::string> SplitStrA(const std::string& str, const std::string& delim, bool fCase /* = true*/)
    {
        std::vector<std::string> vectorOut;
        size_t iStart = 0;
        size_t iEnd = 0;

        if (delim.empty())
        {
            vectorOut.push_back(str);
        }
        else
        {
            if (fCase)
            {
                while ((iStart = str.find_first_not_of(delim, iEnd)) != std::string::npos)
                {
                    iEnd = str.find(delim, iStart);
                    vectorOut.push_back(str.substr(iStart, iEnd - iStart));
                }
            }
            else
            {
                while ((iStart = FindFirstNotOfNoCaseA(str, delim, iEnd)) != std::string::npos)
                {
                    iEnd = FindNoCaseA(str, delim, iStart);
                    vectorOut.push_back(str.substr(iStart, iEnd - iStart));
                }
            }
        }

        return vectorOut;
    }

    std::vector<std::wstring> SplitStrW(const std::wstring& str, const std::wstring& delim, bool fCase /* = true*/)
    {
        std::vector<std::wstring> vectorOut;
        size_t iStart = 0;
        size_t iEnd = 0;

        if (delim.empty())
        {
            vectorOut.push_back(str);
        }
        else
        {
            if (fCase)
            {
                while ((iStart = str.find_first_not_of(delim, iEnd)) != std::wstring::npos)
                {
                    iEnd = str.find(delim, iStart);
                    vectorOut.push_back(str.substr(iStart, iEnd - iStart));
                }
            }
            else
            {
                while ((iStart = FindFirstNotOfNoCaseW(str, delim, iEnd)) != std::string::npos)
                {
                    iEnd = FindNoCaseW(str, delim, iStart);
                    vectorOut.push_back(str.substr(iStart, iEnd - iStart));
                }
            }
        }

        return vectorOut;
    }

    std::vector<_tstring> SplitStr(const _tstring& str, const _tstring& delim, bool fCase /* = true*/)
    {
#ifdef _UNICODE
        return SplitStrW(str, delim, fCase);
#else
        return SplitStrA(str, delim, fCase);
#endif
    }

    std::string FromFileA(const std::string& strFile)
    {
        std::string strContent;

        std::ifstream inputFile(strFile.c_str(), std::ios::binary | std::ios::in);
        if (!inputFile.is_open())
        {
            return strContent;
        }

        //获取文件大小
        inputFile.seekg(0, std::ios::end);
        std::streamoff nSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);

        std::string strBuffer(nSize, 0);
        inputFile.read(&strBuffer[0], nSize);
        size_t nByteSize = (size_t)inputFile.gcount();

        if (GetUtf8Count(strBuffer.data(), nSize) > 0)
        {
            // UTF8 -> ANSI
            std::string strUtf8((const char*)strBuffer.data(), nSize);

            // 去除字节序标识
            while (strUtf8.size() >= 3 && (0xEF == (uint8_t)strUtf8[0] && 0xBB == (uint8_t)strUtf8[1] && 0xBF == (uint8_t)strUtf8[2]))
            {
                strUtf8 = strUtf8.substr(3);
            }

            strContent = U8StrToAStr(strUtf8);
        }
        else if (GetUtf16Count(strBuffer.data(), nSize) > 0)
        {
            // UTF16 -> ANSI
            std::wstring strUtf16((const wchar_t*)strBuffer.data(), nSize / sizeof(wchar_t));

            // 去除字节序标识
            while (!strUtf16.empty() && 0xFEFF == strUtf16[0])
            {
                strUtf16 = strUtf16.substr(1);
            }

            strContent = WStrToAStr(strUtf16);
        }
        else
        {
            strContent = strBuffer;
        }

        return strContent;
    }

    std::wstring FromFileW(const std::wstring& strFile)
    {
        std::wstring strContent;

        std::ifstream inputFile(strFile.c_str(), std::ios::binary | std::ios::in);
        if (!inputFile.is_open())
        {
            return strContent;
        }

        //获取文件大小
        inputFile.seekg(0, std::ios::end);
        std::streamoff nSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);

        std::string strBuffer(nSize, 0);
        inputFile.read((char*)&strBuffer[0], nSize);
        size_t nByteSize = (size_t)inputFile.gcount();

        inputFile.close();

        // 编码自动转换
        if (GetUtf8Count(strBuffer.data(), nSize) > 0)
        {
            // UTF8 -> UTF16
            std::string strUtf8((const char*)strBuffer.data(), nSize);
            strContent = U8StrToWStr(strUtf8);
        }
        else if (GetUtf16Count(strBuffer.data(), nSize) < 0)
        {
            // ANSI -> UTF16
            std::string strAnsi((const char*)strBuffer.data(), nSize);
            strContent = AStrToWStr(strAnsi);
        }
        else
        {
            strContent = std::wstring((wchar_t*)strBuffer.data(), strBuffer.size() / sizeof(wchar_t));
        }

        // 去除字节序标识
        while (!strContent.empty() && 0xFEFF == strContent[0])
        {
            strContent = strContent.substr(1);
        }

        return strContent;
    }

    _tstring FromFile(const _tstring& strFile)
    {
#ifdef _UNICODE
        return FromFileW(strFile);
#else
        return FromFileA(strFile);
#endif
    }

    bool ToFileA(const std::string& str, const std::string& strFile)
    {
        std::ofstream outputFile(strFile.c_str(), std::ios::binary | std::ios::out);
        if (!outputFile.is_open())
        {
            return false;
        }

        outputFile.write(str.c_str(), str.size());
        outputFile.close();

        return true;
    }

    bool ToFileW(const std::wstring& str, const std::wstring& strFile)
    {
        std::ofstream outputFile(strFile.c_str(), std::ios::binary | std::ios::out);
        if (!outputFile.is_open())
        {
            return false;
        }

        // 添加字节序标识
        if (!str.empty() && 0xFEFF != str[0])
        {
            std::wstring strContent;
            strContent += 0xFEFF;
            outputFile.write((const char*)strContent.c_str(), strContent.size() * sizeof(wchar_t));
        }

        outputFile.write((const char*)str.c_str(), str.size() * sizeof(wchar_t));
        outputFile.close();

        return true;
    }

    bool ToFile(const _tstring& str, const _tstring& strFile)
    {
#ifdef _UNICODE
        return ToFileW(str, strFile);
#else
        return ToFileA(str, strFile);
#endif
    }

    std::string FormatA(LPCSTR pFormat, ...)
    {
        size_t nCchCount = MAX_PATH;
        std::string strResult(nCchCount, 0);
        va_list args;

        va_start(args, pFormat);

        do
        {
            //成功则赋值字符串并终止循环
            int nSize = _vsnprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
            if (-1 != nSize)
            {
                strResult.resize(nSize);
                break;
            }

            //缓冲大小超限终止
            if (nCchCount >= FORMAT_COUNT_MAX)
            {
                break;
            }

            //重新分配缓冲
            nCchCount *= 2;
            strResult.resize(nCchCount);

        } while (true);

        va_end(args);

        return strResult;
    }

    std::wstring FormatW(LPCWSTR pFormat, ...)
    {
        size_t nCchCount = MAX_PATH;
        std::wstring strResult(nCchCount, 0);
        va_list args;

        va_start(args, pFormat);

        do
        {
            //格式化输出字符串
            int nSize = _vsnwprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
            if (-1 != nSize)
            {
                strResult.resize(nSize);
                break;
            }

            //缓冲大小超限终止
            if (nCchCount >= FORMAT_COUNT_MAX)
            {
                break;
            }

            //重新分配缓冲
            nCchCount *= 2;
            strResult.resize(nCchCount);

        } while (true);

        va_end(args);

        return strResult;
    }

    _tstring Format(LPCTSTR pFormat, ...)
    {
        size_t nCchCount = MAX_PATH;
        _tstring strResult(nCchCount, 0);
        va_list args;

        va_start(args, pFormat);

        do
        {
            //格式化输出字符串
            int nSize = _vsntprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
            if (-1 != nSize)
            {
                strResult.resize(nSize);
                break;
            }

            //缓冲大小超限终止
            if (nCchCount >= FORMAT_COUNT_MAX)
            {
                break;
            }

            //重新分配缓冲
            nCchCount *= 2;
            strResult.resize(nCchCount);

        } while (true);

        va_end(args);

        return strResult;
    }

    void OutputDebugA(LPCSTR pFormat, ...)
    {
        size_t nCchCount = MAX_PATH;
        std::string strResult(nCchCount, 0);
        va_list args;

        va_start(args, pFormat);

        do
        {
            //格式化输出字符串
            int nSize = _vsnprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
            if (-1 != nSize)
            {
                ::OutputDebugStringA(strResult.c_str());
                break;
            }

            //缓冲大小超限终止
            if (nCchCount >= FORMAT_COUNT_MAX)
            {
                break;
            }

            //重新分配缓冲
            nCchCount *= 2;
            strResult.resize(nCchCount);

        } while (true);

        va_end(args);
    }

    void OutputDebugW(LPCWSTR pFormat, ...)
    {
        size_t nCchCount = MAX_PATH;
        std::wstring strResult(nCchCount, 0);
        va_list args;

        va_start(args, pFormat);

        do
        {
            //格式化输出字符串
            int nSize = _vsnwprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
            if (-1 != nSize)
            {
                ::OutputDebugStringW(strResult.c_str());
                break;
            }

            //缓冲大小超限终止
            if (nCchCount >= FORMAT_COUNT_MAX)
            {
                break;
            }

            //重新分配缓冲
            nCchCount *= 2;
            strResult.resize(nCchCount);

        } while (true);

        va_end(args);
    }

    void OutputDebug(LPCTSTR pFormat, ...)
    {
        size_t nCchCount = MAX_PATH;
        _tstring strResult(nCchCount, 0);
        va_list args;

        va_start(args, pFormat);

        do
        {
            //格式化输出字符串
            int nSize = _vsntprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
            if (-1 != nSize)
            {
                ::OutputDebugString(strResult.c_str());
                break;
            }

            //缓冲大小超限终止
            if (nCchCount >= FORMAT_COUNT_MAX)
            {
                break;
            }

            //重新分配缓冲
            nCchCount *= 2;
            strResult.resize(nCchCount);

        } while (true);

        va_end(args);
    }

    void ConsoleClear(SHORT nStartLine/* = -1*/, SHORT nCount/* = -1*/)
    {
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFOEX ScreenBuffer = { 0 };
        ScreenBuffer.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
        ::GetConsoleScreenBufferInfoEx(console, &ScreenBuffer);
        SHORT nClearCount = ScreenBuffer.dwMaximumWindowSize.Y;

        if (nStartLine >= 0)
        {
            ScreenBuffer.dwCursorPosition.Y = nStartLine;
        }

        if (nCount >= 0)
        {
            nClearCount = ScreenBuffer.dwCursorPosition.Y + nCount;
            if (nClearCount > ScreenBuffer.dwMaximumWindowSize.Y)
            {
                nClearCount = ScreenBuffer.dwMaximumWindowSize.Y;
            }
        }

        for (SHORT i = ScreenBuffer.dwCursorPosition.Y; i < nClearCount; i++)
        {
            DWORD dwNumberOfCharsWritten = 0;
            ::FillConsoleOutputCharacter(
                console,
                _T(' '),
                ScreenBuffer.dwMaximumWindowSize.X,
                { 0, i },
                &dwNumberOfCharsWritten
            );
        }
    }

    void ConsoleSetPos(SHORT x, SHORT y)
    {
        ::SetConsoleCursorPosition(::GetStdHandle(STD_OUTPUT_HANDLE), { x, y });
    }

    void ConsolePrintf(LPCTSTR pFormat, ...)
    {
        size_t nCchCount = MAX_PATH;
        _tstring strResult(nCchCount, 0);
        va_list args;

        va_start(args, pFormat);

        do
        {
            // Formatted String
            int nSize = _vsntprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
            if (-1 != nSize)
            {
                HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
                CONSOLE_SCREEN_BUFFER_INFOEX ScreenBuffer = { 0 };
                ScreenBuffer.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);

                ::GetConsoleScreenBufferInfoEx(console, &ScreenBuffer);

                // If text is entered at the beginning of a line, the current line content is cleared first.
                if (0 == ScreenBuffer.dwCursorPosition.X)
                {
                    DWORD dwNumberOfCharsWritten = 0;
                    ::FillConsoleOutputCharacter(
                        console,
                        _T(' '),
                        ScreenBuffer.dwMaximumWindowSize.X,
                        { 0, ScreenBuffer.dwCursorPosition.Y },
                        &dwNumberOfCharsWritten
                    );
                }

                ::WriteConsole(console, strResult.c_str(), nSize, NULL, NULL);
                break;
            }

            if (nCchCount >= INT32_MAX)
            {
                break;
            }

            nCchCount *= 2;
            strResult.resize(nCchCount);

        } while (true);

        va_end(args);
    }

    void ConsoleOutputA(LPCSTR pFormat, ...)
    {
        size_t nCchCount = MAX_PATH;
        std::string strResult(nCchCount, 0);
        va_list args;

        va_start(args, pFormat);

        do
        {
            //格式化输出字符串
            int nSize = _vsnprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
            if (-1 != nSize)
            {
                HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);
                ::WriteConsoleA(console, strResult.c_str(), nSize, NULL, NULL);
                break;
            }

            //缓冲大小超限终止
            if (nCchCount >= FORMAT_COUNT_MAX)
            {
                break;
            }

            //重新分配缓冲
            nCchCount *= 2;
            strResult.resize(nCchCount);

        } while (true);

        va_end(args);
    }

    void ConsoleOutputW(LPCWSTR pFormat, ...)
    {
        size_t nCchCount = MAX_PATH;
        std::wstring strResult(nCchCount, 0);
        va_list args;

        va_start(args, pFormat);

        do
        {
            //格式化输出字符串
            int nSize = _vsnwprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
            if (-1 != nSize)
            {
                HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);
                ::WriteConsoleW(console, strResult.c_str(), nSize, NULL, NULL);
                break;
            }

            //缓冲大小超限终止
            if (nCchCount >= FORMAT_COUNT_MAX)
            {
                break;
            }

            //重新分配缓冲
            nCchCount *= 2;
            strResult.resize(nCchCount);

        } while (true);

        va_end(args);
    }

    void ConsoleOutput(LPCTSTR pFormat, ...)
    {
        size_t nCchCount = MAX_PATH;
        _tstring strResult(nCchCount, 0);
        va_list args;

        va_start(args, pFormat);

        do
        {
            //格式化输出字符串
            int nSize = _vsntprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
            if (-1 != nSize)
            {
                HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);
                ::WriteConsole(console, strResult.c_str(), nSize, NULL, NULL);
                break;
            }

            //缓冲大小超限终止
            if (nCchCount >= FORMAT_COUNT_MAX)
            {
                break;
            }

            //重新分配缓冲
            nCchCount *= 2;
            strResult.resize(nCchCount);

        } while (true);

        va_end(args);
    }

    std::string GetErrorMessageA(DWORD dwMessageId, DWORD dwLanguageId/* = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)*/)
    {
        std::string strResult;

        LPSTR pBuffer = NULL;
        DWORD dwFlags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |    //函数分配一个足够大的缓冲区来保存格式化消息，并将指针放置在 pBuffer 指定的地址处分配的缓冲区
            FORMAT_MESSAGE_FROM_SYSTEM |        //函数应在系统消息表资源搜索请求的消息
            FORMAT_MESSAGE_IGNORE_INSERTS |     //将忽略消息定义中的插入序列（如 %1）
            FORMAT_MESSAGE_MAX_WIDTH_MASK;      //函数忽略消息定义文本中的常规

        ::FormatMessageA(
            dwFlags,            //格式设置选项以及如何解释 lpSource 参数
            NULL,               //消息定义的位置。 此参数的类型取决于 dwFlags 参数中的设置
            dwMessageId,        //请求的消息的消息标识符。 如果 dwFlags 包含 FORMAT_MESSAGE_FROM_STRING，则忽略此参数
            dwLanguageId,       //所请求消息 的语言标识符
            (LPSTR)&pBuffer,    //指向缓冲区的指针
            MAX_PATH,           //如果未设置 FORMAT_MESSAGE_ALLOCATE_BUFFER 标志，则此参数以 TCHAR 为单位指定输出缓冲区的大小。 如果设置了 FORMAT_MESSAGE_ALLOCATE_BUFFER ，则此参数指定要为输出缓冲区分配的最小 TCHAR 数
            NULL                //一个值数组，这些值用作格式化消息中的插入值。 格式字符串中的 %1 指示 Arguments 数组中的第一个值;%2 指示第二个参数;等等
        );

        if (NULL != pBuffer)
        {
            strResult = pBuffer;
            ::LocalFree(pBuffer);
        }

        return strResult;
    }

    std::wstring GetErrorMessageW(DWORD dwMessageId, DWORD dwLanguageId/* = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)*/)
    {
        std::wstring strResult;

        LPWSTR pBuffer = NULL;
        DWORD dwFlags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |    //函数分配一个足够大的缓冲区来保存格式化消息，并将指针放置在 pBuffer 指定的地址处分配的缓冲区
            FORMAT_MESSAGE_FROM_SYSTEM |        //函数应在系统消息表资源搜索请求的消息
            FORMAT_MESSAGE_IGNORE_INSERTS |     //将忽略消息定义中的插入序列（如 %1）
            FORMAT_MESSAGE_MAX_WIDTH_MASK;      //函数忽略消息定义文本中的常规

        ::FormatMessageW(
            dwFlags,            //格式设置选项以及如何解释 lpSource 参数
            NULL,               //消息定义的位置。 此参数的类型取决于 dwFlags 参数中的设置
            dwMessageId,        //请求的消息的消息标识符。 如果 dwFlags 包含 FORMAT_MESSAGE_FROM_STRING，则忽略此参数
            dwLanguageId,       //所请求消息 的语言标识符
            (LPWSTR)&pBuffer,   //指向缓冲区的指针
            MAX_PATH,           //如果未设置 FORMAT_MESSAGE_ALLOCATE_BUFFER 标志，则此参数以 TCHAR 为单位指定输出缓冲区的大小。 如果设置了 FORMAT_MESSAGE_ALLOCATE_BUFFER ，则此参数指定要为输出缓冲区分配的最小 TCHAR 数
            NULL                //一个值数组，这些值用作格式化消息中的插入值。 格式字符串中的 %1 指示 Arguments 数组中的第一个值;%2 指示第二个参数;等等
        );

        if (NULL != pBuffer)
        {
            strResult = pBuffer;
            ::LocalFree(pBuffer);
        }

        return strResult;
    }

    _tstring GetErrorMessage(DWORD dwMessageId, DWORD dwLanguageId/* = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)*/)
    {
#ifdef _UNICODE
        return GetErrorMessageW(dwMessageId, dwLanguageId);
#else
        return GetErrorMessageA(dwMessageId, dwLanguageId);
#endif
    }

    DATA_UNIT_INFO FormatByteSize(
        double nBytesSize,
        eUnitType eSrcUnit/* = eUnitType::eUT_Auto*/,
        eUnitType eDestUnit/* = eUnitType::eUT_Auto*/,
        bool fHasUnits/* = true*/,
        bool fSpace/* = true*/,
        int nInteger/* = 1*/,
        int nPrecision/* = 3*/
    )
    {
        TCHAR szFormatBuf[MAX_PATH] = { 0 };
        TCHAR szResultBuf[MAX_PATH] = { 0 };
        DATA_UNIT_INFO dataUnitInfo;
        bool fSrcBit = false;
        bool fDestBit = false;

        LPCTSTR strUnitByteName[] = {
            _T("B"),
            _T("KB"),
            _T("MB"),
            _T("GB"),
            _T("TB"),
            _T("PB"),
            _T("EB"),
            _T("ZB"),
            _T("YB"),
            _T("BB"),
            _T("NB"),
            _T("DB"),
            _T("CB"),
            _T("XB"),
        };

        LPCTSTR strUnitBitName[] = {
            _T("b"),
            _T("Kb"),
            _T("Mb"),
            _T("Gb"),
            _T("Tb"),
            _T("Pb"),
            _T("Eb"),
            _T("Zb"),
            _T("Yb"),
            _T("Bb"),
            _T("Nb"),
            _T("Db"),
            _T("Cb"),
            _T("Xb"),
        };

        // 原始单位 比特 -> 字节
        if (eSrcUnit >= eUnitType::eUT_b && eSrcUnit < eUnitType::eUT_B)
        {
            fSrcBit = true;
            eSrcUnit = (eUnitType)(eSrcUnit + (eUnitType::eUT_B - eUnitType::eUT_b));
        }

        // 目标单位 比特 -> 字节
        if (eDestUnit >= eUnitType::eUT_b && eDestUnit < eUnitType::eUT_B)
        {
            fDestBit = true;
            eDestUnit = (eUnitType)(eDestUnit + (eUnitType::eUT_B - eUnitType::eUT_b));
        }

        // 原始单位转换
        for (int i = eUnitType::eUT_B; i < eSrcUnit; i++)
        {
            nBytesSize *= 1024.0f;
        }

        // 自动
        int nUnitTypeIndex = eUnitType::eUT_B;
        if (eUnitType::eUT_Auto == eDestUnit)
        {
            double nCurUnitSize = 1.0f;
            double nNextUnitSize = 1024.0f;
            int nUnitTypeMaxIndex = eUnitType::eUT_Max - 1;
            for (int i = 0; i < _countof(strUnitByteName) && nUnitTypeIndex < nUnitTypeMaxIndex; i++)
            {
                if ((nBytesSize >= nCurUnitSize && nBytesSize < nNextUnitSize) || 0 == nNextUnitSize || 0 == nBytesSize)
                {
                    break;
                }

                nCurUnitSize *= 1024.0f;
                nNextUnitSize *= 1024.0f;
                nUnitTypeIndex++;
            }
            eDestUnit = (eUnitType)nUnitTypeIndex;
        }

        {
            ::_stprintf_s(szFormatBuf, _countof(szFormatBuf), _T("%%%d.%dlf"), nInteger + nPrecision + 1, nPrecision);
            double fUnitSize = 1.0f;
            for (int i = eUnitType::eUT_B; i < eDestUnit; i++)
            {
                fUnitSize *= 1024.0f;
            }

            if (fSrcBit)
            {
                fUnitSize *= 8.0f;
            }

            if (fDestBit)
            {
                nBytesSize *= 8.0f;
            }

            double lfResult = nBytesSize / fUnitSize;
            ::_stprintf_s(szResultBuf, _countof(szResultBuf), szFormatBuf, lfResult);
            dataUnitInfo.strOutput = szResultBuf;
            dataUnitInfo.value = lfResult;

            if (fHasUnits)
            {
                if (fSpace)
                {
                    dataUnitInfo.strOutput += _T(" ");
                }

                if (fDestBit)
                {
                    dataUnitInfo.strOutput += strUnitBitName[eDestUnit - eUnitType::eUT_B];
                    dataUnitInfo.strUnitStr = strUnitBitName[eDestUnit - eUnitType::eUT_B];
                    dataUnitInfo.eUnit = (eUnitType)(eDestUnit + (eUnitType::eUT_B - eUnitType::eUT_b));
                }
                else
                {
                    dataUnitInfo.strOutput += strUnitByteName[eDestUnit - eUnitType::eUT_B];
                    dataUnitInfo.strUnitStr = strUnitByteName[eDestUnit - eUnitType::eUT_B];
                    dataUnitInfo.eUnit = eDestUnit;
                }
            }
        }

        return dataUnitInfo;
    }
}