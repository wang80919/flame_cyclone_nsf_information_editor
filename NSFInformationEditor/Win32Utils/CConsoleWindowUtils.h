#pragma once
#include <Windows.h>

#include <wtypesbase.h>
#include <string>
#include <vector>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CConsoleWindowUtils
{
public:
    bool Show(bool bShow);
    _tstring GetTitle();
    bool SetTitle(const _tstring& strTitle);
    bool DeleteCloseMenu();
    static CConsoleWindowUtils& GetInstance();

    bool Initialize();
    void Uninitialize();
    HWND GetHandle();
    DWORD GetInputMode();
    bool SetInputMode(DWORD dwMode);

private:
    CConsoleWindowUtils();
    ~CConsoleWindowUtils();

    void _UpdateHandle();
    _tstring _GetCurrentModulePath();
private:
    static CConsoleWindowUtils m_Inst;
    FILE* m_fStreamStdOut = nullptr;
    FILE* m_fStreamStdIn = nullptr;
    HWND m_hWnd;
};

