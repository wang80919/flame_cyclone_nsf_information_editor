#include "CSystemUtils.h"
#include <intrin.h>

const PKUSER_SHARED_DATA CSystemUtils::GetKUserData()
{
    return (PKUSER_SHARED_DATA)0x7FFE0000;
}

NTVERSIONNUMBERS CSystemUtils::_GetNtVersionNumbers()
{
    NTVERSIONNUMBERS NtVersion = { 0 };
    HMODULE hModule = NULL;

    do
    {
        hModule = ::LoadLibrary(_T("ntdll.dll"));
        if (NULL == hModule)
        {
            return NtVersion;
        }

        DWORD dwMajor = 0;
        DWORD dwMinor = 0;
        DWORD dwBuildNumber = 0;

        {
            typedef void(NTAPI* NTPROC)(DWORD*, DWORD*, DWORD*);
            NTPROC RtlGetNtVersionNumbers = (NTPROC)::GetProcAddress(hModule, "RtlGetNtVersionNumbers");
            if (NULL != RtlGetNtVersionNumbers)
            {
                RtlGetNtVersionNumbers(&dwMajor, &dwMinor, &dwBuildNumber);
                dwBuildNumber &= 0xffff;
                NtVersion.dwMajor = dwMajor;
                NtVersion.dwMinor = dwMinor;
                NtVersion.dwBuildNumber = dwBuildNumber;
            }
        }

        {
            typedef void(NTAPI* NTPROC)(POSVERSIONINFOW);
            OSVERSIONINFOEXW osVer = { 0 };
            osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

            NTPROC RtlGetVersion = (NTPROC)::GetProcAddress(hModule, "RtlGetVersion");
            if (NULL != RtlGetVersion)
            {
                RtlGetVersion((POSVERSIONINFOW)&osVer);
                RtlGetVersion((POSVERSIONINFOW)&osVer);
            }
        }

        {
            typedef ULONG (NTAPI* NTPROC)(void);
            OSVERSIONINFOEXW osVer = { 0 };
            osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

            NTPROC RtlGetSuiteMask = (NTPROC)::GetProcAddress(hModule, "RtlGetSuiteMask");
            if (NULL != RtlGetSuiteMask)
            {
                ULONG ulMask = RtlGetSuiteMask();
                ulMask = RtlGetSuiteMask();
            }
        }

        {
            typedef ULONG(NTAPI* NTPROC)(DWORD, DWORD, DWORD, DWORD, PDWORD);
            OSVERSIONINFOEXW osVer = { 0 };
            osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

            NTPROC RtlGetProductInfo = (NTPROC)::GetProcAddress(hModule, "RtlGetProductInfo");
            if (NULL != RtlGetProductInfo)
            {
                DWORD dwProductType = 0;
                DWORD dwSpMajor = 0;
                DWORD dwSpMinor = 0;
                PDWORD pdwReturnedProductType = 0;
                ULONG ulMask = RtlGetProductInfo(dwMajor, dwMinor, dwSpMajor, dwSpMinor, &dwProductType);
                ulMask = RtlGetProductInfo(dwMajor, dwMinor, dwSpMajor, dwSpMinor, &dwProductType);
            }
        }

        // https://learn.microsoft.com/zh-cn/windows/win32/api/sysinfoapi/nf-sysinfoapi-getproductinfo
        // 最低受支持的客户端    Windows Vista [仅限桌面应用]
        // 最低受支持的服务器    Windows Server 2008 [仅限桌面应用]
        {
            DWORD dwProductType;
            DWORD dwSpMajor = 0;
            DWORD dwSpMinor = 0;
            PDWORD pdwReturnedProductType = 0;

            if (!::GetProductInfo(dwMajor, dwMinor, dwSpMajor, dwSpMinor, &dwProductType))
            {
            }
        }

    }while(false);

    if (NULL != hModule)
    {
        ::FreeLibrary(hModule);
    }

    return NtVersion;
}

std::string CSystemUtils::GetCpuName()
{
    std::string cpuName;
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 0x80000000);
    unsigned int extMaxCpuid = cpuInfo[0];
    if (extMaxCpuid >= 0x80000004)
    {
        char cpuBrand[0x40] = { 0 };
        for (unsigned int i = 0x80000002; i <= 0x80000004; ++i)
        {
            __cpuid(cpuInfo, i);
            memcpy(cpuBrand + (i - 0x80000002) * 16, cpuInfo, sizeof(cpuInfo));
        }
        cpuName = cpuBrand;
    }

    size_t nLength = cpuName.size();

    for (auto it = cpuName.rbegin(); it != cpuName.rend(); it++)
    {
        if (*it != ' ')
        {
            break;
        }

        if (*it == ' ')
        {
            nLength--;
        }
    }

    cpuName.resize(nLength);

    return cpuName;
}
