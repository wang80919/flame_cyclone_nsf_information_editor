#pragma once
#include <windows.h>

class CMessageBoxCenter
{

public:

    CMessageBoxCenter();
    ~CMessageBoxCenter();

    // 获取子窗口位于父窗口的居中位置
    static POINT GetChildWindowCenterPos(
        int nWidth,         //子窗口宽度
        int nHeight,        //子窗口高度
        HWND hParent        //父窗口句柄
    );

private:

    // WH_CBT 
    // 线程或全局
    // https://learn.microsoft.com/zh-cn/windows/win32/winmsg/cbtproc
    // 安装用于接收对 CBT 应用程序有用的通知的挂钩过程
    static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);

private:

    static HHOOK m_hHook;
};
