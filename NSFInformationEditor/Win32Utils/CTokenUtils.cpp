#include "CTokenUtils.h"

_tstring CTokenUtils::GetModulePath()
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szBuf, _countof(szBuf));
    return szBuf;
}

_tstring CTokenUtils::GetModuleDir()
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szBuf, _countof(szBuf));
    _tstring strDir = szBuf;
    return strDir.substr(0, strDir.find_last_of(_T('\\')));
}

DWORD CTokenUtils::DuplicateProcessToken(
    const _tstring& strName,
    DWORD dwSessionId, 
    DWORD dwDesiredAccess,
    PHANDLE phToken
)
{
    PROCESSENTRY32 pe = { 0 };
    PRIVILEGE_SET ps = { 0 };
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    BOOL fResult = FALSE;
    DWORD dwErr = ERROR_SUCCESS;
    BOOL fFound = FALSE;

    ps.PrivilegeCount = 1;
    ps.Control = PRIVILEGE_SET_ALL_NECESSARY;

    do
    {
        if (!::LookupPrivilegeValue(NULL, SE_TCB_NAME, &ps.Privilege[0].Luid))
        {
            break;
        }

        hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (INVALID_HANDLE_VALUE == hSnapshot)
        {
            break;
        }

        pe.dwSize = sizeof(pe);
        if (!::Process32First(hSnapshot, &pe))
        {
            break;
        }

        do
        {
            HANDLE hProcess = NULL;
            HANDLE hToken = NULL;
            DWORD dwRetLen = 0;
            DWORD sid = 0;

            if (0 != _tcsicmp(pe.szExeFile, strName.c_str()))
            {
                continue;
            }

            hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
            if (!hProcess)
            {
                break;
            }

            do
            {
                BOOL fTcb = FALSE;

                if (!::OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE, &hToken))
                {
                    break;
                }

                if (!::PrivilegeCheck(hToken, &ps, &fTcb) && fTcb)
                {
                    break;
                }

                if (!::GetTokenInformation(hToken, TokenSessionId, &sid, sizeof(sid), &dwRetLen) && sid == dwSessionId)
                {
                    break;
                }

                if (!::DuplicateTokenEx(hToken, dwDesiredAccess, NULL, SecurityImpersonation, TokenImpersonation, phToken))
                {
                    break;
                }

                fFound = true;
                dwErr = ERROR_SUCCESS;

            } while (false);

            if (hToken)
            {
                ::CloseHandle(hToken);
            }

            if (hProcess)
            {
                ::CloseHandle(hProcess);
            }

            if (fFound)
            {
                break;
            }

        } while (::Process32Next(hSnapshot, &pe));

        if (!fFound)
        {
            dwErr = ERROR_NOT_FOUND;
            break;
        }

        fResult = true;

    } while (false);

    if (!fResult)
    {
        dwErr = ::GetLastError();
        if (!fFound)
        {
            dwErr = ERROR_NOT_FOUND;
        }
    }

    if (INVALID_HANDLE_VALUE != hSnapshot)
    {
        ::CloseHandle(hSnapshot);
    }

    return dwErr;
}

DWORD CTokenUtils::CreateUIAccessToken(
    PHANDLE phToken
)
{
    DWORD dwErr = ERROR_SUCCESS;
    HANDLE hTokenSelf = NULL;
    HANDLE hWinlogonToken = NULL;
    BOOL fResult = FALSE;

    do
    {
        DWORD dwDesiredAccess = TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_DEFAULT;
        DWORD dwSessionId = 0;
        DWORD dwRetLen = 0;
        BOOL bUIAccess = TRUE;

        if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hTokenSelf))
        {
            break;
        }

        if (!::GetTokenInformation(hTokenSelf, TokenSessionId, &dwSessionId, sizeof(dwSessionId), &dwRetLen))
        {
            break;
        }

        dwErr = DuplicateProcessToken(_T("wininit.exe"), dwSessionId, TOKEN_IMPERSONATE, &hWinlogonToken);
        if (ERROR_SUCCESS != dwErr)
        {
            break;
        }

        if (!::SetThreadToken(NULL, hWinlogonToken))
        {
            break;
        }

        if (!::DuplicateTokenEx(hTokenSelf, dwDesiredAccess, NULL, SecurityAnonymous, TokenPrimary, phToken))
        {
            break;
        }

        if (!::SetTokenInformation(*phToken, TokenUIAccess, &bUIAccess, sizeof(bUIAccess)))
        {
            break;
        }

        ::RevertToSelf();

        fResult = TRUE;

    } while (false);

    if (!fResult)
    {
        if (*phToken)
        {
            ::CloseHandle(*phToken);
            *phToken = NULL;
        }

        dwErr = ::GetLastError();
    }

    if (hWinlogonToken)
    {
        ::CloseHandle(hWinlogonToken);
    }

    if (hTokenSelf)
    {
        ::CloseHandle(hTokenSelf);
    }

    return dwErr;
}

DWORD CTokenUtils::CreateUIProcessWithSystem(
    const _tstring& strCommand
)
{
    HANDLE hToken = NULL;
    HANDLE hTokenDup = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    BOOL fResult = FALSE;
    _tstring lpCommandLine = strCommand;

    do
    {
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };

        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOW;
        si.hStdInput = NULL;
        si.hStdOutput = NULL;
        si.hStdError = NULL;

        DWORD dwSessionId = ::WTSGetActiveConsoleSessionId();
        if (!::OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
        {
            break;
        }

        if (!::DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityIdentification, TokenPrimary, &hTokenDup))
        {
            break;
        }

        if (!::SetTokenInformation(hTokenDup, TokenSessionId, &dwSessionId, sizeof(DWORD)))
        {
            break;
        }

        if (!::CreateProcessAsUser(hTokenDup, NULL, (LPTSTR)lpCommandLine.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            break;
        }

        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);

    } while (false);

    if (!fResult)
    {
        dwErr = ::GetLastError();
    }

    if (hToken)
    {
        ::CloseHandle(hToken);
    }

    if (hTokenDup)
    {
        ::CloseHandle(hTokenDup);
    }

    return dwErr;
}

DWORD CTokenUtils::CreateProcessWithUIAccess(
    const _tstring& strCommand
)
{
    HANDLE hTokenUIAccess = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    BOOL fResult = FALSE;
    _tstring lpCommandLine = strCommand;

    do
    {
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };

        dwErr = CreateUIAccessToken(&hTokenUIAccess);
        if (ERROR_SUCCESS != dwErr)
        {
            break;
        }

        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOW;
        si.hStdInput = NULL;
        si.hStdOutput = NULL;
        si.hStdError = NULL;

        ::GetStartupInfo(&si);
        if (!::CreateProcessAsUser(hTokenUIAccess, NULL, (LPTSTR)lpCommandLine.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            break;
        }

        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);

    } while (false);

    if (!fResult)
    {
        dwErr = ::GetLastError();
    }

    if (hTokenUIAccess)
    {
        ::CloseHandle(hTokenUIAccess);
    }

    return dwErr;
}

DWORD CTokenUtils::CreateProcessWithUIAccess(
    const _tstring& strExePath, 
    const _tstring& strCommand
)
{
    HANDLE hTokenUIAccess = NULL;
    DWORD dwErr = ERROR_SUCCESS;
    BOOL fResult = FALSE;
    _tstring exePath = strExePath;
    _tstring lpCommandLine = strCommand;

    do
    {
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };

        dwErr = CreateUIAccessToken(&hTokenUIAccess);
        if (ERROR_SUCCESS != dwErr)
        {
            break;
        }

        ::GetStartupInfo(&si);
        if (!::CreateProcessAsUser(hTokenUIAccess, exePath.c_str(), (LPTSTR)lpCommandLine.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            break;
        }

        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);

    } while (false);

    if (!fResult)
    {
        dwErr = ::GetLastError();
    }

    if (hTokenUIAccess)
    {
        ::CloseHandle(hTokenUIAccess);
    }

    return dwErr;
}

BOOL CTokenUtils::CheckForUIAccess(
    DWORD dwPid,
    DWORD *pdwErr, 
    DWORD *pfUIAccess
)
{
    HANDLE hProcess = NULL;
    HANDLE hToken = NULL;
    DWORD dwRetLen = 0;
    BOOL result = FALSE;

    do
    {
        hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION , FALSE, dwPid);
        if (!hProcess)
        {
            break;
        }

        if (!::OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
        {
            break;
        }

        if (!::GetTokenInformation(hToken, TokenUIAccess, pfUIAccess, sizeof(*pfUIAccess), &dwRetLen))
        {
            break;
        }

        result = TRUE;

    } while (false);

    if (!result)
    {
        *pdwErr = ::GetLastError();
    }

    if (hProcess)
    {
        ::CloseHandle(hProcess);
    }

    if (hToken)
    {
        ::CloseHandle(hToken);
    }

    return result;
}