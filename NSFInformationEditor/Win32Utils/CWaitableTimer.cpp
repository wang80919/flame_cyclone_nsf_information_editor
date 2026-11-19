#include "CWaitableTimer.h"
#include <future>
#include <thread>

CWaitableTimer::CWaitableTimer()
    :
    m_hTimer(NULL),
    m_bQuit(false)
{

}

CWaitableTimer::~CWaitableTimer()
{
    this->Close();
}

bool CWaitableTimer::Create(const _tstring& strName)
{
    if (m_hTimer)
    {
        return true;
    }

    SECURITY_ATTRIBUTES sa = { 0 };
    SECURITY_DESCRIPTOR sd = { 0 };

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    //设置权限, 防止低权限进程不能打开
    (void)::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    (void)::SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

    m_hTimer = ::CreateWaitableTimer(&sa, false, strName.c_str());
    return NULL != m_hTimer;
}

bool CWaitableTimer::Open(const _tstring& strName)
{
    if (m_hTimer)
    {
        return true;
    }

    m_hTimer = ::OpenWaitableTimer(TIMER_ALL_ACCESS, false, strName.c_str());
    return NULL != m_hTimer;
}

bool CWaitableTimer::SetCompletionRoutine(std::function<void(void)> cb, DWORD dwBeginTime, DWORD dwPeriodTime)
{
    if (NULL == m_hTimer)
    {
        return false;
    }

    m_cb = cb;

    std::promise<bool> promiseValue;
    std::future<bool> futureValue = promiseValue.get_future();

    //创建定时器任务线程
    m_bQuit = false;

    std::thread([this, &promiseValue, dwBeginTime, dwPeriodTime]()
        {
            //正值表示绝对时间, 负值表示相对时间
            LARGE_INTEGER DueTime = { 0 };
            DueTime.QuadPart = 0 - (dwBeginTime * 10000LL);

            BOOL bRes = ::SetWaitableTimer(m_hTimer, &DueTime, dwPeriodTime, nullptr, this, false);
            promiseValue.set_value(bRes);

            while (!m_bQuit)
            {
                //等待定时器信号
                DWORD dwRet = ::WaitForSingleObject(m_hTimer, INFINITE);
                if (WAIT_OBJECT_0 == dwRet)
                {
                    if (m_cb)
                    {
                        m_cb();
                    }
                }
                else
                {
                    break;
                }

                //单次定时器则执行一次后退出等待逻辑
                if (0 == dwPeriodTime)
                {
                    break;
                }
            }
            m_bQuit = false;
        }
    ).detach();

    bool bRes = futureValue.get();
    return bRes;
}

void CWaitableTimer::Close()
{
    m_bQuit = true;

    if (NULL != m_hTimer)
    {
        //立即触发定时器, 让任务线程退出
        LARGE_INTEGER DueTime = { 0 };
        (void)::SetWaitableTimer(m_hTimer, &DueTime, 0, nullptr, this, false);

        //将计时器设置为非活动状态
        ::CancelWaitableTimer(m_hTimer);

        ::CloseHandle(m_hTimer);
        m_hTimer = NULL;
    }
}
