#include "CNtDllUtils.h"
#include <tchar.h>

namespace CNtDllUtils
{
    typedef NTSTATUS(NTAPI* pFnNtQuerySystemInformation)(
        IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
        OUT PVOID SystemInformation,
        IN ULONG SystemInformationLength,
        OUT PULONG ReturnLength OPTIONAL
        );

    pFnNtQuerySystemInformation _NtQuerySystemInformation;

    NTSTATUS NTAPI NtQuerySystemInformation(
        IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
        OUT PVOID SystemInformation,
        IN ULONG SystemInformationLength,
        OUT PULONG ReturnLength OPTIONAL
    )
    {
        if (!_NtQuerySystemInformation)
        {
            do
            {
                HMODULE hModule = ::GetModuleHandle(_T("Ntdll.dll"));
                if (!hModule)
                {
                    break;
                }

                _NtQuerySystemInformation = (pFnNtQuerySystemInformation)::GetProcAddress(
                    hModule,
                    "NtQuerySystemInformation"
                );
                if (!_NtQuerySystemInformation)
                {
                    break;
                }

            } while (false);
        }

        if (!_NtQuerySystemInformation)
        {
            return 0;
        }

        return _NtQuerySystemInformation(
            SystemInformationClass,
            SystemInformation,
            SystemInformationLength,
            ReturnLength
        );
    }
}