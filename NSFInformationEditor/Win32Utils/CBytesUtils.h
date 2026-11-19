#pragma once

#include <wtypesbase.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
using _tfstream = std::wfstream;
#else
using _tstring = std::string;
using _tfstream = std::fstream;
#endif

class CBytesUtils
{
public:

    //
    // @brief: 将指定地址内存转字节序列
    // @param: pData                地址内存
    // @param: nSize                内存长度
    // @ret: std::vector<uint8_t>   字节序列
    static std::vector<uint8_t> BytesFromMemory(
        const void* pData, 
        size_t nSize
    );

    //
    // @brief: 将文件数据转字节序列
    // @param: vBytes               字节容器
    // @param: strFilePath          文件路径
    // @ret: bool                   执行结果
    static bool BytesFromFile(
        std::vector<uint8_t>& vBytes,
        const _tstring& strFilePath
    );

    //
    // @brief: 十六进制字符串转字节序列
    // @param: strInput             十六进制字符串
    // @ret: std::vector<uint8_t>   字节序列
    static std::vector<uint8_t> BytesFromHexString(
        const _tstring& strInput
    );

    //
    // @brief: 字节序列转字符串
    // @param: vBytes               字节容器
    // @param: nLineSize            每行转换字节数
    // @param: strDelimiter         字符分隔
    // @param: strPrefix            字符前缀
    // @param: strLinePrefix        单行前缀
    // @param: strReturn            转行内容
    // @ret: _tstring               执行结果字符串
    static _tstring BytesFormat(
        const std::vector<uint8_t>& vBytes, 
        int nStart = 0,
        size_t nLineSize = 16,
        const _tstring& strDelimiter = _T(""),
        const _tstring& strPrefix = _T(""),
        const _tstring& strLinePrefix = _T(""),
        size_t addr = -1,
        const _tstring& strReturn = _T("\r\n")
    );

    //
    // @brief: 字节序列存到文件
    // @param: strFilePath          文件路径
    // @param: vBytes               字节容器
    // @ret: bool                   执行结果
    static bool BytesToFile(
        const _tstring& strFilePath,
        const std::vector<uint8_t>& vBytes
    );

    //
    // @brief: 指定地址内存存到文件
    // @param: strFilePath          文件路径
    // @param: pData                地址内存
    // @param: nSize                内存长度
    // @ret: bool                   执行结果
    static bool MemoryToFile(
        const _tstring& strFilePath,
        const void* pData,
        size_t nSize
    );

    //
    // @brief: 字节序列转字符串存到文件
    // @param: strFilePath          文件路径
    // @param: vBytes               字节容器
    // @param: nLineSize            每行转换字节数
    // @param: strDelimiter         字符分隔
    // @param: strPrefix            字符前缀
    // @param: strLinePrefix        单行前缀
    // @param: strReturn            转行内容
    // @ret: bool                   执行结果
    static bool BytesFormatToFile(
        const _tstring& strFilePath,
        const std::vector<uint8_t>& vBytes,
        size_t nLineSize = 16,
        const _tstring& strDelimiter = _T(""),
        const _tstring& strPrefix = _T(""),
        const _tstring& strLinePrefix = _T(""),
        const _tstring& strReturn = _T("\r\n")
    );

    //
    // @brief: 格式化的字符串转字节序列
    // @param: strFilePath          文件路径
    // @param: strDelimiter         字符分隔
    // @param: strPrefix            字符前缀
    // @param: strLinePrefix        单行前缀
    // @param: strReturn            转行内容
    // @ret: std::vector<uint8_t>   字节序列
    static std::vector<uint8_t> BytesFormatFromFile(
        const _tstring& strFilePath,
        const _tstring& strDelimiter = _T(""),
        const _tstring& strPrefix = _T(""),
        const _tstring& strLinePrefix = _T(""),
        const _tstring& strReturn = _T("\r\n")
    );

    //
    // @brief: 格式化的字符串转字节序列
    // @param: strInput             格式化的字符串, 如 " .DB $4D,$69,$63,$72,$6F,$73,$6F,$66,$74,$20,$43,$2F,$43,$2B,$2B,$20"
    // @param: vBytes               字节容器
    // @param: strDelimiter         字符分隔, 如 ","
    // @param: strPrefix            字符前缀, 如 "$"
    // @param: strLinePrefix        单行前缀, 如 " .DB"
    // @param: strReturn            转行内容, 如 "\r\n"
    // @ret: std::vector<uint8_t>   字节序列
    static std::vector<uint8_t> BytesFromString(
        const _tstring& strInput,
        const _tstring& strDelimiter = _T(""),
        const _tstring& strPrefix = _T(""),
        const _tstring& strLinePrefix = _T(""),
        const _tstring& strReturn = _T("\r\n")
    );

    static _tstring Format(LPCTSTR pFormat, ...);

private:

    //
    // @brief: 分割字符串
    // @param: str                  待分割字符串
    // @param: delim                分割标识字符串
    // @ret: std::vector<_tstring>  分割结果
    static std::vector<_tstring> SplitStr(const _tstring& str, const _tstring& delim);
};