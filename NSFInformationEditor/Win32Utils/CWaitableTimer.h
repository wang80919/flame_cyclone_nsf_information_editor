#pragma once
#include <string>
#include <functional>
#include <atomic>
#include <windows.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CWaitableTimer
{
public:

    CWaitableTimer();
    ~CWaitableTimer();
    CWaitableTimer(const CWaitableTimer& r) = delete;
    CWaitableTimer& operator = (const CWaitableTimer& r) = delete;

    //
    // @brief: 创建定时器
    // @param: strName                  定时器名称
    // @ret: bool                       操作成功与否
    bool Create(const _tstring& strName);

    //
    // @brief: 打开定时器
    // @param: strName                  定时器名称
    // @ret: bool                       操作成功与否
    bool Open(const _tstring& strName);

    //
    // @brief: 设置完成例程回调
    // @param: cb                       例程回调
    // @param: dwStartMilliseconds      首次触发间隔时间(毫秒)
    // @param: dwPeriodMilliseconds     后续触发间隔时间(毫秒), 如果为零，则计时器将只发出一次信号
    // @ret: bool       操作成功与否
    bool SetCompletionRoutine(std::function<void(void)> cb, DWORD dwStartMilliseconds, DWORD dwPeriodMilliseconds);

    //
    // @brief: 关闭定时器
    // @param: void
    // @ret: void
    void Close();

private:

    HANDLE m_hTimer;                            //计时器句柄
    std::function<void(void)> m_cb;             //回调函数
    std::atomic<bool> m_bQuit;                  //退出标志
};