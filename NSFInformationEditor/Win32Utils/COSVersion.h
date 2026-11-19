#pragma once

#include <wtypesbase.h>
#include <tchar.h>
#include <vector>
#include <map>
#include <string>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

typedef struct _NTVERSIONNUMBERS_INFO NTVERSIONNUMBERS_INFO, * PNTVERSIONNUMBERS_INFO;

typedef struct _FILE_VERSION_NUMBERS
{
    DWORD dwMajor;          // 主版本号
    DWORD dwMinor;          // 次版本号
    DWORD dwBuildMajor;     // 主构建版本号
    DWORD dwBuildMinor;     // 次构建版本号

    _FILE_VERSION_NUMBERS():
        dwMajor(0),
        dwMinor(0),
        dwBuildMajor(0),
        dwBuildMinor(0)
    {

    }
}FILE_VERSION_NUMBERS, * PFILE_VERSION_NUMBERS;

typedef struct _OS_VERSION_INFO
{
    _tstring strDisplayVersion;         // 显示版本
    _tstring strInstallDate;            // 安装日期
    FILE_VERSION_NUMBERS osVersion;     // 系统版本号
    FILE_VERSION_NUMBERS ntdllVersion;  // ntdll 版本号
    SYSTEMTIME stInstallDate;           // 安装日期
    RTL_OSVERSIONINFOEXW osVersionEx;   // OS版本信息
    DWORD dwProductType;                // 产品类型
    _OS_VERSION_INFO() :
        osVersionEx{ 0 },
        stInstallDate{ 0 },
        dwProductType(0)
    {

    }

}OS_VERSION_INFO, * POS_VERSION_INFO;

class COSVersion
{
public:

    // 获取操作系统版本信息
    static OS_VERSION_INFO GetOSVersionInfo();

    // 系统版本检查
    static bool IsWindows10Or11();
    static bool IsWindows10();
    static bool IsWindows11();
    static bool IsWindows11_24H2();
    static bool IsWindows11_23H2();
    static bool IsWindows11_22H2();
    static bool IsWindows11_21H2();

private:

    // 获取 Ntdll.dll 版本号
    static bool _GetNtdllVersion(PFILE_VERSION_NUMBERS pVersion);

    // 获取操作系统显示名
    static _tstring _GetDisplayVersion();

    // 获取更新构建修订版本号
    static DWORD _GetUpdateBuildRevision();

    // 获取安装日期
    static DWORD _GetInstallDate();

    static int64_t _GetInstallTime();

    // 获取版本号信息
    static bool _GetNtVersionNumbers(PNTVERSIONNUMBERS_INFO pNtVerInfo);

    // 获取版本信息
    static bool _GetVersion(RTL_OSVERSIONINFOEXW* pVerInfo);

    // 获取版本号信息
    static bool _RtlGetNtVersionNumbers(DWORD* pMajor, DWORD* pMinor, DWORD* pBuild);

    // 获取版本信息
    static bool _RtlGetVersion(PRTL_OSVERSIONINFOEXW lpVersionInformation);

    // 获取产品版本
    // 最低受支持的客户端    Windows Vista [仅限桌面应用]
    // 最低受支持的服务器    Windows Server 2008 [仅限桌面应用]
    static bool _RtlGetProductInfo(DWORD dwOSMajor, DWORD dwOSMinor, DWORD dwSpMajor, DWORD dwSpMinor, PDWORD pdwProductType);

    // 获取文件产品版本号
    static bool _GetFileProductVersion(const _tstring& strFile, PFILE_VERSION_NUMBERS pVersion);

    // 注册表操作
    static bool _GetRegString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, _tstring& strValue);
    static _tstring _GetRegString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName);
    static bool _GetRegDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD& dwValue);
    static DWORD _GetRegDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName);
    static bool _GetRegQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, int64_t& ullValue);
    static int64_t _GetRegQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName);

    // 时间戳操作
    static _tstring _TimestampToString(int64_t timestamp, bool bHasDate = true);
    static SYSTEMTIME _TimestampToSystemTime(int64_t timestamp);
};