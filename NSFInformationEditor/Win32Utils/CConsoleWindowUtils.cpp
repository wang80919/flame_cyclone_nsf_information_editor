#include "CConsoleWindowUtils.h"
#include <strsafe.h>
#include <tchar.h>

CConsoleWindowUtils CConsoleWindowUtils::m_Inst;

CConsoleWindowUtils::CConsoleWindowUtils()
    :
    m_fStreamStdOut(nullptr),
    m_fStreamStdIn(nullptr),
    m_hWnd(NULL)
{

}

CConsoleWindowUtils::~CConsoleWindowUtils()
{
    Uninitialize();
}

CConsoleWindowUtils& CConsoleWindowUtils::GetInstance()
{
    return m_Inst;
}

bool CConsoleWindowUtils::Initialize()
{
    HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE == hStdOut)
    {
        return false;
    }

    // 已存在关联的标准句柄
    if (NULL != hStdOut)
    {
        _UpdateHandle();
        return true;
    }

    //分配控制台
    ::AllocConsole();

    _UpdateHandle();

    //重定向输入输出
    _tfreopen_s(&m_fStreamStdOut, _T("CONOUT$"), _T("w+t"), stdout);
    _tfreopen_s(&m_fStreamStdIn, _T("CONIN$"), _T("r+t"), stdin);

    return true;
}

DWORD CConsoleWindowUtils::GetInputMode()
{
    DWORD dwMode;
    ::GetConsoleMode(::GetStdHandle(STD_INPUT_HANDLE), &dwMode);
    return dwMode;
}

bool CConsoleWindowUtils::SetInputMode(DWORD dwMode)
{
    return SetConsoleMode(::GetStdHandle(STD_INPUT_HANDLE), dwMode);
}

void CConsoleWindowUtils::_UpdateHandle()
{
    if (NULL == m_hWnd)
    {
        _tstring strConsoleTitle = GetTitle();
        _tstring strCheckTitle(MAX_PATH, 0);
        DWORD dwPid = GetCurrentProcessId();
        ::StringCchPrintf(&strCheckTitle[0], strCheckTitle.size(), _T("%s PID: %d (HEX: %X)"), strConsoleTitle.c_str(), dwPid, dwPid);
        SetTitle(strCheckTitle);

        ULONGLONG ullBeginTime = ::GetTickCount64();
        do
        {
            m_hWnd = ::FindWindow(NULL, strCheckTitle.c_str());
            if (NULL != m_hWnd)
            {
                SetTitle(strConsoleTitle);
                break;
            }

        } while (::GetTickCount64() - ullBeginTime < 500);
    }
}

_tstring CConsoleWindowUtils::_GetCurrentModulePath()
{
    TCHAR szCurPath[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szCurPath, _countof(szCurPath));

    return szCurPath;
}

void CConsoleWindowUtils::Uninitialize()
{
    if (m_fStreamStdOut)
    {
        ::fflush(m_fStreamStdOut);
        ::fclose(m_fStreamStdOut);
    }

    if (m_fStreamStdIn)
    {
        ::fclose(m_fStreamStdIn);
    }

    ::FreeConsole();
}

HWND CConsoleWindowUtils::GetHandle()
{
    return m_hWnd;
}

bool CConsoleWindowUtils::Show(bool bShow)
{
    if (NULL == m_hWnd)
    {
        return false;
    }

    if (bShow && ::IsIconic(m_hWnd))
    {
        ::ShowWindow(m_hWnd, SW_NORMAL);
    }

    return ::ShowWindow(m_hWnd, bShow ? SW_SHOW : SW_HIDE);
}

bool CConsoleWindowUtils::DeleteCloseMenu()
{
    if (NULL != m_hWnd)
    {
        //禁用关闭按钮(会导致整个进程结束掉)
        HMENU hMenu = ::GetSystemMenu(m_hWnd, FALSE);
        if (NULL != hMenu)
        {
            return ::DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
        }
    }

    return false;
}

_tstring CConsoleWindowUtils::GetTitle()
{
    _tstring strConsoleTitle(MAX_PATH, 0);
    DWORD dwTitleLength = ::GetConsoleTitle(&strConsoleTitle[0], strConsoleTitle.size());
    strConsoleTitle.resize(dwTitleLength);

    return strConsoleTitle;
}

bool CConsoleWindowUtils::SetTitle(const _tstring& strTitle)
{
    if (NULL != m_hWnd)
    {
        ::SetWindowText(m_hWnd, strTitle.c_str());
    }

    return ::SetConsoleTitle(strTitle.c_str());
}