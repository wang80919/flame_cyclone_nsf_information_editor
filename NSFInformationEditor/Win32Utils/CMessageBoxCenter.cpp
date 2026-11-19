#include "CMessageBoxCenter.h"

HHOOK CMessageBoxCenter::m_hHook;

CMessageBoxCenter::CMessageBoxCenter()
{
    m_hHook = ::SetWindowsHookEx(WH_CBT, CBTProc, NULL, ::GetCurrentThreadId());
}

CMessageBoxCenter::~CMessageBoxCenter()
{
    ::UnhookWindowsHookEx(m_hHook);
}

POINT CMessageBoxCenter::GetChildWindowCenterPos(
    int nWidth,         //子窗口宽度
    int nHeight,        //子窗口高度
    HWND hParent        //父窗口句柄
)
{
    RECT rectParent = { 0 };
    LONG nParentW = 0;
    LONG nParentH = 0;

    // 如果父窗口句柄为空, 则以桌面窗口为父窗口
    if (NULL == hParent)
    {
        hParent = ::GetDesktopWindow();
    }

    // 获取并统计父窗口宽度和高度
    ::GetWindowRect(hParent, &rectParent);
    nParentW = rectParent.right - rectParent.left;
    nParentH = rectParent.bottom - rectParent.top;

    return {rectParent.left + (nParentW - nWidth) / 2, rectParent.top + (nParentH - nHeight) / 2};
}

LRESULT CALLBACK CMessageBoxCenter::CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // 即将创建一个窗口
    if (HCBT_CREATEWND == nCode)
    {
        HWND hWndNew = (HWND)wParam;
        LPCBT_CREATEWND pStruct = (LPCBT_CREATEWND)lParam;
        HWND hParent = pStruct->lpcs->hwndParent;

        // 检查是对话框, 则修改显示位置为相对父窗口居中位置
        if (32770 == ::GetClassLongPtr(hWndNew, GCW_ATOM))
        {
            POINT nPos = GetChildWindowCenterPos(pStruct->lpcs->cx, pStruct->lpcs->cy, hParent);
            pStruct->lpcs->x = nPos.x;
            pStruct->lpcs->y = nPos.y;
        }
    }

    return ::CallNextHookEx(m_hHook, nCode, wParam, lParam);
}