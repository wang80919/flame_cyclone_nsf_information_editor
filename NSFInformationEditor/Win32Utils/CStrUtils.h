#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

// 字符串转换实用工具
namespace CStrUtils
{
    // 宽字符串转换
    std::string WStrToU8Str(const std::wstring& str);
    std::string WStrToAStr(const std::wstring& str);
    _tstring WStrToTStr(const std::wstring& str);

    // ANSI字符串转换
    std::wstring AStrToWStr(const std::string& str);
    std::string AStrToU8Str(const std::string& str);
    _tstring AStrToTStr(const std::string& str);

    // UTF-8字符串转换
    std::wstring U8StrToWStr(const std::string& str);
    std::string U8StrToAStr(const std::string& str);
    _tstring U8StrToTStr(const std::string& str);

    // 中立字符串转换
    std::string TStrToAStr(const _tstring& str);
    std::wstring TStrToWStr(const _tstring& str);
    std::string TStrToU8Str(const _tstring& str);

    // 获取字符个数
    int GetGbkCount(const void* pData, size_t size = -1);
    int GetUtf8Count(const void* data_ptr, size_t size = -1);
    int GetUtf16Count(const void* data_ptr, size_t size = -1);

    int GetGbkCount(const std::string& str);
    int GetUtf8Count(const std::string& str);
    int GetUtf16Count(const std::wstring& str);

    // 获取大写字符串
    std::string GetUpperStrA(const std::string& str);
    std::wstring GetUpperStrW(const std::wstring& str);
    _tstring GetUpperStr(const _tstring& str);

    // 获取小写字符串
    std::string GetLowerStrA(const std::string& str);
    std::wstring GetLowerStrW(const std::wstring& str);
    _tstring GetLowerStr(const _tstring& str);

    // 转换为大写字符串
    std::string& ToUpperStrA(const std::string& str);
    std::wstring& ToUpperStrW(const std::wstring& str);
    _tstring& ToUpperStr(const _tstring& str);

    // 转换为小写字符串
    std::string& ToLowerStrA(const std::string& str);
    std::wstring& ToLowerStrW(const std::wstring& str);
    _tstring& ToLowerStr(const _tstring& str);

    // 浮点数转换
    double StrToDoubleA(const std::string& str);
    double StrToDoubleW(const std::wstring& str);
    double StrToDouble(const _tstring& str);
    std::string DoubleToStrA(double val);
    std::wstring DoubleToStrW(double val);
    _tstring DoubleToStr(double val);

    // 整数转换
    int64_t StrToIntA(const std::string& str, int radix = 10);
    int64_t StrToIntW(const std::wstring& str, int radix = 10);

    // shlwapi.h 含有 StrToInt 宏定义, 此处检查防止冲突
#ifndef StrToInt
    int64_t StrToInt(const _tstring& str, int radix = 10);
#endif

    std::string IntToStrA(int64_t val, int radix = 10);
    std::wstring IntToStrW(int64_t val, int radix = 10);
    _tstring IntToStr(int64_t val, int radix = 10);

    // 无符号整数转换
    uint64_t StrToUIntA(const std::string& str, int radix = 10);
    uint64_t StrToUIntW(const std::wstring& str, int radix = 10);
    uint64_t StrToUInt(const _tstring& str, int radix = 10);
    std::string UIntToStrA(uint64_t val, int radix = 10);
    std::wstring UIntToStrW(uint64_t val, int radix = 10);
    _tstring UIntToStr(uint64_t val, int radix = 10);

    // 查找字符串位置(忽略大小写)
    size_t FindNoCaseA(const std::string& strMain, const std::string& strSub, size_t offset = 0);
    size_t FindNoCaseW(const std::wstring& strMain, const std::wstring& strSub, size_t offset = 0);
    size_t FindNoCase(const _tstring& strMain, const _tstring& strSub, size_t offset = 0);

    size_t FindFirstNotOfNoCaseA(const std::string& strMain, const std::string& strSub, size_t offset = 0);
    size_t FindFirstNotOfNoCaseW(const std::wstring& strMain, const std::wstring& strSub, size_t offset = 0);
    size_t FindFirstNotOfNoCase(const _tstring& strMain, const _tstring& strSub, size_t offset = 0);

    // 替换字符串(忽略大小写)
    std::string& ReplaceNoCaseA(std::string& strMain, const std::string& strSub, const std::string& strTarget);
    std::wstring& ReplaceNoCaseW(std::wstring& strMain, const std::wstring& strSub, const std::wstring& strTarget);
    _tstring& ReplaceNoCase(_tstring& strMain, const _tstring& strSub, const _tstring& strTarget);

    // 比较字符串(忽略大小写)
    int CompareNoCaseA(const std::string& strSrc, const std::string& strDest);
    int CompareNoCaseW(const std::wstring& strSrc, const std::wstring& strDest);
    int CompareNoCase(const _tstring& strSrc, const _tstring& strDest);

    // 查找字符串
    size_t FindA(const std::string& strMain, const std::string& strSub, size_t offset = 0, bool fCase = false);
    size_t FindW(const std::wstring& strMain, const std::wstring& strSub, size_t offset = 0, bool fCase = false);
    size_t Find(const _tstring& strMain, const _tstring& strSub, size_t offset = 0, bool fCase = false);

    // 替换字符串
    std::string& ReplaceA(std::string& strSrc, const std::string& strFind, const std::string& strReplace, bool fCase = false);
    std::wstring& ReplaceW(std::wstring& strSrc, const std::wstring& strFind, const std::wstring& strReplace, bool fCase = false);
    _tstring& Replace(_tstring& strSrc, const _tstring& strFind, const _tstring& strReplace, bool fCase = false);

    // 比较字符串
    int CompareA(const std::string& strSrc, const std::string& strDest, bool fCase = true);
    int CompareW(const std::wstring& strSrc, const std::wstring& strDest, bool fCase = true);
    int Compare(const _tstring& strSrc, const _tstring& strDest, bool fCase = true);

    // 分割字符串
    std::vector<std::string> SplitStrA(const std::string& str, const std::string& delim, bool fCase = true);
    std::vector<std::wstring> SplitStrW(const std::wstring& str, const std::wstring& delim, bool fCase = true);
    std::vector<_tstring> SplitStr(const _tstring& str, const _tstring& delim, bool fCase = true);

    // 从文件载入字符串
    std::string FromFileA(const std::string& strFile);
    std::wstring FromFileW(const std::wstring& strFile);
    _tstring FromFile(const _tstring& strFile);

    // 字符串保存到文件
    bool ToFileA(const std::string& str, const std::string& strFile);
    bool ToFileW(const std::wstring& str, const std::wstring& strFile);
    bool ToFile(const _tstring& str, const _tstring& strFile);

    // 格式化字符串
    std::string FormatA(LPCSTR pFormat, ...);
    std::wstring FormatW(LPCWSTR pFormat, ...);
    _tstring Format(LPCTSTR pFormat, ...);

    // 输出调试字符串
    void OutputDebugA(LPCSTR pFormat, ...);
    void OutputDebugW(LPCWSTR pFormat, ...);
    void OutputDebug(LPCTSTR pFormat, ...);

    // 控制台输出
    void ConsoleClear(SHORT nStartLine = -1, SHORT nCount = -1);
    void ConsoleSetPos(SHORT x = 0, SHORT y = 0);
    void ConsolePrintf(LPCTSTR pFormat, ...);
    void ConsoleOutputA(LPCSTR pFormat, ...);
    void ConsoleOutputW(LPCWSTR pFormat, ...);
    void ConsoleOutput(LPCTSTR pFormat, ...);

    // 获取GetLastError()的错误信息文本
    std::string GetErrorMessageA(DWORD dwMessageId, DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
    std::wstring GetErrorMessageW(DWORD dwMessageId, DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
    _tstring GetErrorMessage(DWORD dwMessageId, DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));

    // 数据单位
    enum eUnitType
    {
        eUT_Auto,   // 自动
        eUT_b,      // (2 ^ 000) Bit
        eUT_Kb,     // (2 ^ 010) Bit
        eUT_Mb,     // (2 ^ 020) Bit
        eUT_Gb,     // (2 ^ 030) Bit
        eUT_Tb,     // (2 ^ 040) Bit
        eUT_Pb,     // (2 ^ 050) Bit
        eUT_Eb,     // (2 ^ 060) Bit
        eUT_Zb,     // (2 ^ 070) Bit
        eUT_Yb,     // (2 ^ 080) Bit
        eUT_Bb,     // (2 ^ 090) Bit
        eUT_Nb,     // (2 ^ 100) Bit
        eUT_Db,     // (2 ^ 110) Bit
        eUT_Cb,     // (2 ^ 120) Bit
        eUT_Xb,     // (2 ^ 130) Bit

        eUT_B,      // Byte          2 ^ 000 Byte
        eUT_KB,     // Kilobyte      2 ^ 010 Byte 
        eUT_MB,     // Megabyte      2 ^ 020 Byte 
        eUT_GB,     // Gigabyte      2 ^ 030 Byte 
        eUT_TB,     // Terabyte      2 ^ 040 Byte 
        eUT_PB,     // Petabyte      2 ^ 050 Byte 
        eUT_EB,     // Exabyte       2 ^ 060 Byte 
        eUT_ZB,     // Zettabyte     2 ^ 070 Byte 
        eUT_YB,     // Yottabyte     2 ^ 080 Byte 
        eUT_BB,     // Brontobyte    2 ^ 090 Byte 
        eUT_NB,     // NonaByte      2 ^ 100 Byte 
        eUT_DB,     // DoggaByte     2 ^ 110 Byte 
        eUT_CB,     // corydonbyte   2 ^ 120 Byte 
        eUT_XB,     // Xerobyte      2 ^ 130 Byte
        eUT_Max
    };

    // 数据单位信息
    typedef struct _DATA_UNIT_INFO
    {
        double value;           // 数值
        eUnitType eUnit;        // 单位
        _tstring strUnitStr;    // 单位字符串
        _tstring strOutput;     // 内容(数值 + 单位字符串)
        _DATA_UNIT_INFO() : value(0.0f), eUnit(eUnitType::eUT_Auto) {}
    }DATA_UNIT_INFO;

    // 数据单位转换
    DATA_UNIT_INFO FormatByteSize(
        double nBytesSize,                                      // 输入值
        eUnitType eSrcUnit = eUnitType::eUT_Auto,               // 原始单位
        eUnitType eDestUnit = eUnitType::eUT_Auto,              // 目标单位
        bool fHasUnits = true,                                  // 结果字符串数值添加单位
        bool fSpace = true,                                     // 结果字符串数值与单位之间添加空格
        int nInteger = 1,                                       // 整数部分长度
        int nPrecision = 3                                      // 小数部分长度
    );
}