#include "CRWLockIPC.h"

CRWLockIPC::CRWLockIPC()
    :
    m_pShareData(nullptr),
    m_hFileMapping(nullptr),
    m_hReadSemaphore(nullptr),
    m_hWritableEvent(nullptr),
    m_hReadableEvent(nullptr),
    m_hWriteMutex(nullptr),
    m_hShareMutex(nullptr),
    m_fInit(false),
    m_fWritePriority(false)
{

}

CRWLockIPC::~CRWLockIPC()
{
    Uninitialize();
}

bool CRWLockIPC::Initialize(
    const _tstring& strName,
    bool fWritePriority/* = true*/,
    LONG nReadCount/* = LONG_MAX*/
)
{
    SECURITY_ATTRIBUTES sa = { 0 };
    SECURITY_DESCRIPTOR sd = { 0 };
    bool fResult = false;
    bool fCreate = false;

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    _tstring strFileMapping = strName.empty() ? strName : strName + _T("_FileMapping");
    _tstring strReadSemaphore = strName.empty() ? strName : strName + _T("_ReadSemaphore");
    _tstring strShareMutex = strName.empty() ? strName : strName + _T("_ShareMutex");
    _tstring strWritableEvent = strName.empty() ? strName : strName + _T("_ReadEvent");
    _tstring strReadableEvent = strName.empty() ? strName : strName + _T("_WriteEvent");
    _tstring strWriteMutex = strName.empty() ? strName : strName + _T("_WriteMutex");

    Uninitialize();

    (void)::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    (void)::SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

    do
    {
        // 创建访问互斥量
        m_hShareMutex = ::CreateMutex(
            &sa,
            FALSE,
            strShareMutex.empty() ? NULL : strShareMutex.c_str()
        );
        if (!m_hShareMutex)
        {
            break;
        }

        // 创建读取信号量
        m_hReadSemaphore = ::CreateSemaphore(
            &sa,
            nReadCount,
            nReadCount,
            strReadSemaphore.empty() ? NULL : strReadSemaphore.c_str()
        );
        if (!m_hReadSemaphore)
        {
            break;
        }

        // 创建可写手动事件
        m_hWritableEvent = ::CreateEvent(
            &sa,
            TRUE,
            TRUE,
            strWritableEvent.empty() ? NULL : strWritableEvent.c_str()
        );
        if (!m_hWritableEvent)
        {
            break;
        }

        // 创建可读手动事件
        m_hReadableEvent = ::CreateEvent(
            &sa,
            TRUE,
            TRUE,
            strReadableEvent.empty() ? NULL : strReadableEvent.c_str()
            );
        if (!m_hReadableEvent)
        {
            break;
        }

        // 创建写入互斥量
        m_hWriteMutex = ::CreateMutex(
            &sa,
            FALSE,
            strWriteMutex.empty() ? NULL : strWriteMutex.c_str()
        );
        if (!m_hWriteMutex)
        {
            break;
        }

        // 创建共享内存
        m_hFileMapping = ::CreateFileMapping(
            INVALID_HANDLE_VALUE,
            &sa,
            PAGE_READWRITE,
            0,
            sizeof(SHARE_DATA),
            strFileMapping.empty() ? NULL : strFileMapping.c_str()
        );
        if (nullptr == m_hFileMapping)
        {
            break;
        }

        if (ERROR_ALREADY_EXISTS != ::GetLastError())
        {
            fCreate = true;
        }

        m_pShareData = (PSHARE_DATA)::MapViewOfFile(
            m_hFileMapping,
            FILE_MAP_READ | FILE_MAP_WRITE,
            0,
            0,
            0
        );
        if (nullptr == m_pShareData)
        {
            break;
        }

        //首次创建, 设置信息
        if (fCreate)
        {
            ::WaitForSingleObject(m_hShareMutex, INFINITE);
            memset(m_pShareData, 0, sizeof(SHARE_DATA));
            ::ReleaseMutex(m_hShareMutex);
        }

        fResult = true;

    } while (false);

    if (!fResult)
    {
        Uninitialize();
    }

    m_fWritePriority = fWritePriority;
    m_fInit = true;
    return fResult;
}

void CRWLockIPC::Uninitialize()
{
    if (m_hReadSemaphore)
    {
        ::CloseHandle(m_hReadSemaphore);
        m_hReadSemaphore = nullptr;
    }

    if (m_hWriteMutex)
    {
        ::CloseHandle(m_hWriteMutex);
        m_hWriteMutex = nullptr;
    }

    if (m_hWritableEvent)
    {
        ::SetEvent(m_hWritableEvent);
        ::CloseHandle(m_hWritableEvent);
        m_hWritableEvent = nullptr;
    }

    if (m_hReadableEvent)
    {
        ::SetEvent(m_hReadableEvent);
        ::CloseHandle(m_hReadableEvent);
        m_hReadableEvent = nullptr;
    }

    if (m_hShareMutex)
    {
        DWORD dwWait = ::WaitForSingleObject(m_hShareMutex, INFINITE);
        if (WAIT_OBJECT_0 == dwWait)
        {
            if (m_pShareData)
            {
                ::UnmapViewOfFile(m_pShareData);
                m_pShareData = nullptr;
            }

            if (m_hFileMapping)
            {
                ::CloseHandle(m_hFileMapping);
                m_hFileMapping = nullptr;
            }
        }
        ::ReleaseMutex(m_hShareMutex);
        ::CloseHandle(m_hShareMutex);
        m_hShareMutex = nullptr;
    }

    m_fInit = false;
}

bool CRWLockIPC::AcquireReadLock(DWORD dwTimeout/* = INFINITE*/)
{
    bool fResult = false;
    bool fAbort = false;

    if (!m_fInit)
    {
        return false;
    }

    // 递增读等待计数
    {
        ::WaitForSingleObject(m_hShareMutex, INFINITE);
        m_pShareData->nReadWaitCount++;

        // 写操作优先: 若存在写操作等待，则阻止新的读操作获取锁
        if (m_fWritePriority && m_pShareData->nWriteWaitCount > 0)
        {
            ::ResetEvent(m_hReadableEvent);
        }

        ::ReleaseMutex(m_hShareMutex);
    }

    // 请求读锁
    do
    {
        // 等待可读事件有信号(无写操作时有信号)
        if (WAIT_OBJECT_0 != WaitForSingleObject(m_hReadableEvent, dwTimeout))
        {
            fAbort = true;
            break;
        }

        // 等待读信号量
        if (WAIT_OBJECT_0 != ::WaitForSingleObject(m_hReadSemaphore, dwTimeout))
        {
            fAbort = true;
            break;
        }

    } while (false);

    // 递减读等待计数
    {
        ::WaitForSingleObject(m_hShareMutex, INFINITE);
        m_pShareData->nReadWaitCount--;
        ::ReleaseMutex(m_hShareMutex);
    }

    if (!fAbort)
    {
        // 更新计数
        ::WaitForSingleObject(m_hShareMutex, INFINITE);

        // 读计数递增
        if (m_pShareData->nReadCount < LONG_MAX)
        {
            m_pShareData->nReadCount++;
        }

        // 设置可写事件无信号, 阻止写操作
        ::ResetEvent(m_hWritableEvent);
        ::ReleaseMutex(m_hShareMutex);

        fResult = true;

    } while (false);

    return fResult;
}

void CRWLockIPC::ReleaseReadLock()
{
    if (!m_fInit)
    {
        return;
    }

    // 释放读锁
    ::ReleaseSemaphore(m_hReadSemaphore, 1, NULL);

    // 计数检查
    {
        ::WaitForSingleObject(m_hShareMutex, INFINITE);

        // 更新读取计数
        if (m_pShareData->nReadCount > 0)
        {
            m_pShareData->nReadCount--;
        }

        // 没有任何读操作存在则允许写操作
        if (0 == m_pShareData->nReadCount)
        {
            // 设置可写事件有信号, 允许写操作
            ::SetEvent(m_hWritableEvent);
        }

        ::ReleaseMutex(m_hShareMutex);
    }
}

bool CRWLockIPC::AcquireWriteLock(DWORD dwTimeout/* = INFINITE*/)
{
    bool fResult = false;

    if (!m_fInit)
    {
        return false;
    }

    // 递增写入等待数
    {
        ::WaitForSingleObject(m_hShareMutex, INFINITE);
        m_pShareData->nWriteWaitCount++;
        ::ReleaseMutex(m_hShareMutex);
    }

    // 获取写锁
    do
    {
        // 等待可写事件
        if (WAIT_OBJECT_0 != ::WaitForSingleObject(m_hWritableEvent, dwTimeout))
        {
            break;
        }

        // 等待写入互斥量事件
        if (WAIT_OBJECT_0 != ::WaitForSingleObject(m_hWriteMutex, dwTimeout))
        {
            break;
        }

        fResult = true;

    } while (false);

    // 递减写入等待数
    {
        ::WaitForSingleObject(m_hShareMutex, INFINITE);
        m_pShareData->nWriteWaitCount--;
        ::ReleaseMutex(m_hShareMutex);
    }

    // 获取锁成功则递增写计数, 并且重置可读事件设为无信号, 防止读线程读取
    if (fResult)
    {
        ::WaitForSingleObject(m_hShareMutex, INFINITE);

        // 更新写计数
        if (m_pShareData->nWriteCount < LONG_MAX)
        {
            m_pShareData->nWriteCount++;
        }

        ::ResetEvent(m_hReadableEvent);
        ::ReleaseMutex(m_hShareMutex);
    }

    return fResult;
}

void CRWLockIPC::ReleaseWriteLock()
{
    if (!m_fInit)
    {
        return;
    }

    // 释放写锁
    ::ReleaseMutex(m_hWriteMutex);

    // 递减写计数
    {
        ::WaitForSingleObject(m_hShareMutex, INFINITE);

        // 更新写计数
        if (m_pShareData->nWriteCount > 0)
        {
            m_pShareData->nWriteCount--;
        }

        // 不存在写等待则设置可读事件为有信号
        if (0 == m_pShareData->nWriteWaitCount)
        {
            ::SetEvent(m_hReadableEvent);
        }

        ::ReleaseMutex(m_hShareMutex);
    }
}
