#include "COSVersion.h"
#include <versionhelpers.h>
#include <strsafe.h>
#include <map>

#define NT_SUCCESS(Status)  (((NTSTATUS)(Status)) >= 0)
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)

typedef struct _NTVERSIONNUMBERS_INFO
{
    DWORD dwMajor;          // 主版本号
    DWORD dwMinor;          // 次版本号
    DWORD dwBuild;          // 构建版本号
}NTVERSIONNUMBERS_INFO, * PNTVERSIONNUMBERS_INFO;

// https://learn.microsoft.com/zh-cn/windows/win32/sysinfo/operating-system-version
// 操作系统                版本号    dwMajorVersion    dwMinorVersion     其他
// Windows 10              10.0*       10              0                   OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
// Windows Server 2016     10.0*       10              0                   OSVERSIONINFOEX.wProductType ！= VER_NT_WORKSTATION
// Windows 8.1             6.3*        6               3                   OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
// Windows Server 2012 R2  6.3*        6               3                   OSVERSIONINFOEX.wProductType ！= VER_NT_WORKSTATION
// Windows 8               6.2         6               2                   OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
// Windows Server 2012     6.2         6               2                   OSVERSIONINFOEX.wProductType ！= VER_NT_WORKSTATION
// Windows 7               6.1         6               1                   OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
// Windows Server 2008 R2  6.1         6               1                   OSVERSIONINFOEX.wProductType ！= VER_NT_WORKSTATION
// Windows Server 2008     6.0         6               0                   OSVERSIONINFOEX.wProductType ！= VER_NT_WORKSTATION
// Windows Vista           6.0         6               0                   OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION
// Windows Server 2003 R2  5.2         5               2                   GetSystemMetrics (SM_SERVERR2) ！= 0
// Windows Home Server     5.2         5               2                   OSVERSIONINFOEX.wSuiteMask & VER_SUITE_WH_SERVER
// Windows Server 2003     5.2         5               2                   GetSystemMetrics (SM_SERVERR2) == 0
// Windows XP Professional 5.2         5               2                   (OSVERSIONINFOEX.wProductType == VER_NT_WORKSTATION) &&
//             x64 Edition                                                 (SYSTEM_INFO.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
// Windows XP              5.1    5    1    不适用
// Windows 2000            5.0    5    0    不适用

typedef struct _WINDOWS_VERSIONNUMBERS
{
    DWORD dwMajor;          // 主版本号
    DWORD dwMinor;          // 次版本号
    DWORD dwBuild;          // 构建号
}WINDOWS_VERSIONNUMBERS, * PWINDOWS_VERSIONNUMBERS;

std::map<_tstring, WINDOWS_VERSIONNUMBERS> g_mapOSServerVersion =
{
    // Windows Server
    {_T("Windows Server 2003"), {5, 2, 0}},
    {_T("Windows Server 2003 R2"), {5, 2, 0}},

    {_T("Windows Server 2008"), {6, 0, 0}},
    {_T("Windows Server 2008 R2"), {6, 1, 0}},

    {_T("Windows Server 2012"), {6, 2, 0}},
    {_T("Windows Server 2012 R2"), {6, 2, 0}},

    {_T("Windows Server 2016 1607"), {10, 0, 14393}},
    {_T("Windows Server 2019 1809"), {10, 0, 17763}},
    {_T("Windows Server 2022"), {10, 0, 20348}},
};

std::map<_tstring, WINDOWS_VERSIONNUMBERS> g_mapOSClientVersion =
{
    // Windows XP
    {_T("Windows XP"), {5, 1, 0}},
    {_T("Windows XP x64"), {5, 2, 0}},

    // Windows Vista
    {_T("Windows Vista"), {6, 0, 0}},

    // Windows 7
    {_T("Windows 7"), {6, 1, 0}},

    // Windows 8
    {_T("Windows 8"), {6, 2, 0}},

    // Windows 8.1
    {_T("Windows 8.1"), {6, 3, 0}},

    // Windows 10
    {_T("Windows 10 1507"), {10, 0, 10240}},
    {_T("Windows 10 1511"), {10, 0, 10586}},
    {_T("Windows 10 1607"), {10, 0, 14393}},
    {_T("Windows 10 1703"), {10, 0, 15036}},
    {_T("Windows 10 1709"), {10, 0, 16299}},
    {_T("Windows 10 1803"), {10, 0, 17134}},
    {_T("Windows 10 1809"), {10, 0, 17753}},
    {_T("Windows 10 1903"), {10, 0, 18362}},
    {_T("Windows 10 1909"), {10, 0, 18363}},
    {_T("Windows 10 2004"), {10, 0, 19041}},
    {_T("Windows 10 20H2"), {10, 0, 19042}},
    {_T("Windows 10 21H1"), {10, 0, 19043}},
    {_T("Windows 10 21H2"), {10, 0, 19044}},
    {_T("Windows 10 22H2"), {10, 0, 19045}},

    // Windows 11
    {_T("Windows 11 21H2"), {10, 0, 22000}},
    {_T("Windows 11 22H2"), {10, 0, 22621}},
    {_T("Windows 11 23H2"), {10, 0, 22631}},
    {_T("Windows 11 24H2"), {10, 0, 26100}},
};

bool COSVersion::_RtlGetNtVersionNumbers(DWORD* pMajor, DWORD* pMinor, DWORD* pBuild)
{
    typedef VOID (NTAPI* NTPROC)(DWORD* dwMajor, DWORD* dwMinor, DWORD* dwBuild);
    HMODULE hModule = NULL;
    static NTPROC ProcAddress = NULL;
    bool fResult = false;

    if (NULL == ProcAddress)
    {
        hModule = ::LoadLibrary(_T("ntdll.dll"));
        if (NULL != hModule)
        {
            ProcAddress = (NTPROC)::GetProcAddress(hModule, "RtlGetNtVersionNumbers");
            ::FreeLibrary(hModule);
        }
    }

    if (NULL != ProcAddress)
    {
        fResult = true;

        ProcAddress(pMajor, pMinor, pBuild);
        *pBuild &= 0xffff;
    }

    return fResult;
}

bool COSVersion::_RtlGetVersion(PRTL_OSVERSIONINFOEXW lpVersionInformation)
{
    typedef NTSTATUS (NTAPI* NTPROC)(PRTL_OSVERSIONINFOEXW lpVersionInformation);
    HMODULE hModule = NULL;
    static NTPROC ProcAddress = NULL;
    bool fResult = false;

    memset(lpVersionInformation, 0, sizeof(RTL_OSVERSIONINFOEXW));
    lpVersionInformation->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

    if (NULL == ProcAddress)
    {
        hModule = ::LoadLibrary(_T("ntdll.dll"));
        if (NULL != hModule)
        {
            ProcAddress = (NTPROC)::GetProcAddress(hModule, "RtlGetVersion");
            ::FreeLibrary(hModule);
        }
    }

    if (NULL != ProcAddress)
    {
        fResult = STATUS_SUCCESS == ProcAddress(lpVersionInformation);
    }

    return fResult;
}

bool COSVersion::_RtlGetProductInfo(DWORD dwOSMajor, DWORD dwOSMinor, DWORD dwSpMajor, DWORD dwSpMinor, PDWORD pdwProductType)
{
    typedef BOOL (NTAPI* NTPROC)(DWORD, DWORD, DWORD, DWORD, PDWORD);
    HMODULE hModule = NULL;
    static NTPROC ProcAddress = NULL;
    bool fResult = false;

    if (NULL == ProcAddress)
    {
        hModule = ::LoadLibrary(_T("ntdll.dll"));
        if (NULL != hModule)
        {
            ProcAddress = (NTPROC)::GetProcAddress(hModule, "RtlGetProductInfo");
            ::FreeLibrary(hModule);
        }
    }

    if (NULL != ProcAddress)
    {
        fResult = ProcAddress(dwOSMajor, dwOSMinor, dwSpMajor, dwSpMinor, pdwProductType);
    }

    return fResult;
}

#pragma pack(push)
#pragma pack(1)
// 包含文件的版本信息。 此信息与语言和代码页无关
// https://learn.microsoft.com/zh-cn/windows/win32/menurc/vs-versioninfo
typedef struct {
    WORD             wLength;       // VS_VERSIONINFO 结构的长度（以字节为单位）,此长度不包括在 32 位边界上对齐任何后续版本资源数据的填充
    WORD             wValueLength;  // Value 成员的长度（以字节为单位）
    WORD             wType;         // 版本资源中的数据类型, 1: 资源包含文本数据 0: 版本资源包含二进制数据
    WCHAR            szKey[15];     // Unicode 字符串 L“VS_VERSION_INFO”
    WORD             Padding1;      // 在 32 位边界上对齐 Children 成员所需的任意或零个 WORD
    //VS_FIXEDFILEINFO Value
    //WORD             Padding2
    //WORD             Children
} VS_VERSIONINFO, * PVS_VERSIONINFO;

#pragma pack(pop)

bool COSVersion::_GetFileProductVersion(const _tstring& strFile, PFILE_VERSION_NUMBERS pVersion)
{
    struct LANGANDCODEPAGE {
        WORD wLanguage;
        WORD wCodePage;

        LANGANDCODEPAGE()
            :
            wLanguage(0),
            wCodePage(0)
        {

        }
    };

    PVOID pFsRedirectionOldValue = NULL;
    bool isDisableWow64Fs = false;
    bool fResult = false;
    UINT cbTranslate = 0;
    DWORD dwVerSize;
    LPVOID lpVerData = NULL;
    LANGANDCODEPAGE Translate;

    do
    {
        if (strFile.empty())
        {
            break;
        }

        // 禁用文件重定向
        isDisableWow64Fs = ::Wow64DisableWow64FsRedirection(&pFsRedirectionOldValue);

        // 获取版本信息数据大小
        dwVerSize = ::GetFileVersionInfoSize(strFile.c_str(), 0);
        if (0 == dwVerSize)
        {
            break;
        }

        // 分配版本信息缓冲
        lpVerData = ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwVerSize);
        if (!lpVerData)
        {
            break;
        }

        // 获取版本信息
        if (!::GetFileVersionInfo(strFile.c_str(), 0, dwVerSize, lpVerData))
        {
            break;
        }

        // 获取语言代码
        LANGANDCODEPAGE* lpTranslate = NULL;
        if (!::VerQueryValue(lpVerData, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbTranslate))
        {
            break;
        }

        Translate = *lpTranslate;

        // 获取 VS_FIXEDFILEINFO 信息
        PVS_VERSIONINFO lpVersion = (PVS_VERSIONINFO)lpVerData;
        if (0 != lpVersion->wValueLength)
        {
            VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)lpVersion + sizeof(VS_VERSIONINFO));

            DWORD dwAlign = 4;
            DWORD_PTR dwPadding = ((DWORD_PTR)pFixedFileInfo % dwAlign);
            if (0 != dwPadding)
            {
                pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)pFixedFileInfo + (dwAlign - dwPadding));
            }

            pVersion->dwMajor = HIWORD(pFixedFileInfo->dwProductVersionMS);
            pVersion->dwMinor = LOWORD(pFixedFileInfo->dwProductVersionMS);
            pVersion->dwBuildMajor = HIWORD(pFixedFileInfo->dwProductVersionLS);
            pVersion->dwBuildMinor = LOWORD(pFixedFileInfo->dwProductVersionLS);
        }

        fResult = true;

    } while (false);

    // 恢复文件重定向
    if (isDisableWow64Fs)
    {
        ::Wow64RevertWow64FsRedirection(pFsRedirectionOldValue);
    }

    if (!lpVerData)
    {
        HeapFree(::GetProcessHeap(), 0, lpVerData);
    }

    return fResult;
}

_tstring COSVersion::_GetRegString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName)
{
    _tstring strResult;
    (void)_GetRegString(hKey, lpSubKey, lpValueName, strResult);
    return strResult;
}

bool COSVersion::_GetRegDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD& dwValue)
{
    LSTATUS ls = ERROR_SUCCESS;
    DWORD cbSize = sizeof(dwValue);

    ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_DWORD/* | RRF_ZEROONFAILURE*/, NULL, &dwValue, &cbSize);
    if (ERROR_SUCCESS != ls)
    {
        return false;
    }

    return ERROR_SUCCESS == ls;
}

DWORD COSVersion::_GetRegDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName)
{
    DWORD dwResult = 0;
    (void)_GetRegDword(hKey, lpSubKey, lpValueName, dwResult);
    return dwResult;
}

bool COSVersion::_GetRegString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, _tstring& strValue)
{
    LSTATUS ls = ERROR_SUCCESS;
    DWORD dwSize = 0;
    LPVOID lpBuf = NULL;

    //获取数据长度
    ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_SZ/* | RRF_ZEROONFAILURE*/, NULL, 0, &dwSize);
    if (ERROR_SUCCESS != ls)
    {
        return false;
    }

    // 数据长度超过常规大小限制, 则重新分配空间
    lpBuf = HeapAlloc(GetProcessHeap(), 0, dwSize);
    if (lpBuf)
    {
        ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_SZ /*| RRF_ZEROONFAILURE*/, NULL, lpBuf, &dwSize);
        if (ERROR_SUCCESS == ls)
        {
            strValue = (LPCTSTR)lpBuf;
        }

        HeapFree(GetProcessHeap(), 0, lpBuf);
    }

    return ERROR_SUCCESS == ls;
}

bool COSVersion::_GetRegQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, int64_t& ullValue)
{
    LSTATUS ls = ERROR_SUCCESS;
    DWORD cbSize = sizeof(ullValue);

    ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_QWORD/* | RRF_ZEROONFAILURE*/, NULL, &ullValue, &cbSize);
    if (ERROR_SUCCESS != ls)
    {
        return false;
    }

    return ERROR_SUCCESS == ls;
}

int64_t COSVersion::_GetRegQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName)
{
    int64_t ullResult = 0;
    (void)_GetRegQword(hKey, lpSubKey, lpValueName, ullResult);
    return ullResult;
}

bool COSVersion::_GetNtVersionNumbers(PNTVERSIONNUMBERS_INFO pNtVerInfo)
{
    return _RtlGetNtVersionNumbers(&pNtVerInfo->dwMajor, &pNtVerInfo->dwMinor, &pNtVerInfo->dwBuild);
}

bool COSVersion::_GetVersion(RTL_OSVERSIONINFOEXW* pVerInfo)
{
    return _RtlGetVersion(pVerInfo);
}

bool COSVersion::_GetNtdllVersion(PFILE_VERSION_NUMBERS pVersion)
{
    return _GetFileProductVersion(_T(R"(C:\Windows\System32\ntdll.dll)"), pVersion);
}

_tstring COSVersion::_TimestampToString(int64_t timestamp, bool bHasDate/* = true*/)
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    SYSTEMTIME st = { 0 };
    FILETIME ftFile = { 0 };
    FILETIME ftLocal = { 0 };

    timestamp = timestamp * 10000 + 116444736000000000;

    ftFile.dwLowDateTime = timestamp & 0xFFFFFFFF;
    ftFile.dwHighDateTime = timestamp >> 32;

    ::FileTimeToLocalFileTime(&ftFile, &ftLocal);
    ::FileTimeToSystemTime(&ftLocal, &st);

    if (bHasDate)
    {
        ::StringCchPrintf(szBuf, _countof(szBuf),
            _T("%04d-%02d-%02d %02d:%02d:%02d.%d"),
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds
        );
    }
    else
    {
        ::StringCchPrintf(szBuf, _countof(szBuf),
            _T("%02d:%02d:%02d.%d"), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds
        );
    }

    return szBuf;
}

_tstring COSVersion::_GetDisplayVersion()
{
    return _GetRegString(HKEY_LOCAL_MACHINE, _T(R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)"), _T("DisplayVersion"));
}

DWORD COSVersion::_GetUpdateBuildRevision()
{
    return _GetRegDword(HKEY_LOCAL_MACHINE, _T(R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)"), _T("UBR"));
}
DWORD COSVersion::_GetInstallDate()
{
    return _GetRegDword(HKEY_LOCAL_MACHINE, _T(R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)"), _T("InstallDate"));
}

int64_t COSVersion::_GetInstallTime()
{
    return _GetRegQword(HKEY_LOCAL_MACHINE, _T(R"(SOFTWARE\Microsoft\Windows NT\CurrentVersion)"), _T("InstallTime"));
}

bool COSVersion::IsWindows10Or11()
{
    NTVERSIONNUMBERS_INFO verInfo = { 0 };
    COSVersion::_GetNtVersionNumbers(&verInfo);

    if (!(10 == verInfo.dwMajor && 10 == verInfo.dwMinor))
    {
        return false;
    }

    return true;
}

bool COSVersion::IsWindows10()
{
    NTVERSIONNUMBERS_INFO verInfo = { 0 };
    COSVersion::_GetNtVersionNumbers(&verInfo);

    if (!(10 == verInfo.dwMajor && 10 == verInfo.dwMinor))
    {
        return false;
    }

    return verInfo.dwBuild >= 10240 && verInfo.dwBuild < 22000;
}

bool COSVersion::IsWindows11()
{
    NTVERSIONNUMBERS_INFO verInfo = { 0 };
    COSVersion::_GetNtVersionNumbers(&verInfo);

    if (!(10 == verInfo.dwMajor && 10 == verInfo.dwMinor))
    {
        return false;
    }

    return verInfo.dwBuild >= 22000;
}

bool COSVersion::IsWindows11_24H2()
{
    NTVERSIONNUMBERS_INFO verInfo = { 0 };
    COSVersion::_GetNtVersionNumbers(&verInfo);

    if (!(10 == verInfo.dwMajor && 10 == verInfo.dwMinor))
    {
        return false;
    }

    return verInfo.dwBuild >= 26100;
}

bool COSVersion::IsWindows11_23H2()
{
    NTVERSIONNUMBERS_INFO verInfo = { 0 };
    COSVersion::_GetNtVersionNumbers(&verInfo);

    if (!(10 == verInfo.dwMajor && 10 == verInfo.dwMinor))
    {
        return false;
    }

    return verInfo.dwBuild >= 22631 && verInfo.dwBuild < 26100;
}

bool COSVersion::IsWindows11_22H2()
{
    NTVERSIONNUMBERS_INFO verInfo = { 0 };
    COSVersion::_GetNtVersionNumbers(&verInfo);

    if (!(10 == verInfo.dwMajor && 10 == verInfo.dwMinor))
    {
        return false;
    }

    return verInfo.dwBuild >= 22621 && verInfo.dwBuild < 22631;
}

bool COSVersion::IsWindows11_21H2()
{
    NTVERSIONNUMBERS_INFO verInfo = { 0 };
    COSVersion::_GetNtVersionNumbers(&verInfo);

    if (!(10 == verInfo.dwMajor && 10 == verInfo.dwMinor))
    {
        return false;
    }

    return verInfo.dwBuild >= 22000 && verInfo.dwBuild < 22621;
}

SYSTEMTIME COSVersion::_TimestampToSystemTime(int64_t timestamp)
{
    SYSTEMTIME st = { 0 };
    FILETIME ftFile = { 0 };
    FILETIME ftLocal = { 0 };

    timestamp = timestamp * 10000 + 116444736000000000;

    ftFile.dwLowDateTime = timestamp & 0xFFFFFFFF;
    ftFile.dwHighDateTime = timestamp >> 32;

    ::FileTimeToLocalFileTime(&ftFile, &ftLocal);
    ::FileTimeToSystemTime(&ftLocal, &st);

    return st;
}

OS_VERSION_INFO COSVersion::GetOSVersionInfo()
{
    OS_VERSION_INFO osVersion;

    // 获取系统版本号(不含次构建版本号)
    _RtlGetNtVersionNumbers(&osVersion.osVersion.dwMajor, &osVersion.osVersion.dwMinor, &osVersion.osVersion.dwBuildMajor);

    // 显示版本
    osVersion.strDisplayVersion = _GetDisplayVersion();

    // 更新构建修订
    osVersion.osVersion.dwBuildMinor = _GetUpdateBuildRevision();

    // 安装日期
    int64_t installDate = _GetInstallDate();
    osVersion.stInstallDate = _TimestampToSystemTime(installDate * 1000);
    osVersion.strInstallDate = _TimestampToString(installDate * 1000, true);

    // ntdll文件版本号
    _GetNtdllVersion(&osVersion.ntdllVersion);

    // 系统产品类型 
    osVersion.osVersionEx.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
    _RtlGetVersion((PRTL_OSVERSIONINFOEXW)&osVersion.osVersionEx);

    _RtlGetProductInfo(osVersion.osVersionEx.dwMajorVersion,
        osVersion.osVersionEx.dwMinorVersion,
        osVersion.osVersionEx.wServicePackMajor,
        osVersion.osVersionEx.wServicePackMinor,
        &osVersion.dwProductType
    );

    SYSTEM_INFO si = { 0 };
    GetNativeSystemInfo(&si); 

    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
    {
        bool fX64 = true;
    }


    return osVersion;
}