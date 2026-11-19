#pragma once

#include <windows.h>
#include <tchar.h>
#include <tlhelp32.h>

#include <string>

#ifdef UNICODE
#define _tstring        std::wstring
#else
#define _tstring        std::string
#endif

class CTokenUtils
{
public:

    // 获取当前进程路径
    static _tstring GetModulePath();

    // 获取当前进程所在目录
    static _tstring GetModuleDir();

    // 复制指定进程名的令牌
    static DWORD DuplicateProcessToken(
        const _tstring& strName, 
        DWORD dwSessionId, 
        DWORD dwDesiredAccess, 
        PHANDLE phToken
    );

    // 创建UI权限令牌
    static DWORD CreateUIAccessToken(
        PHANDLE phToken
    );

    // 创建带有UI权限的进程
    static DWORD CreateProcessWithUIAccess(
        const _tstring& strExePath, 
        const _tstring& strCommand
    );

    // 创建带有UI权限的进程
    static DWORD CreateUIProcessWithSystem(
        const _tstring& strCommand
    );

    // 创建带有UI权限的进程
    static DWORD CreateProcessWithUIAccess(
        const _tstring& strCommand
    );

    // 检查指定进程是否有UI权限
    static BOOL CheckForUIAccess(
        DWORD dwPid,
        DWORD *pdwErr, 
        DWORD *pfUIAccess
    );
};
