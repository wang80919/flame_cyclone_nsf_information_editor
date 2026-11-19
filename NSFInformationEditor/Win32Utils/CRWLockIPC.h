#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <tchar.h>

#pragma warning(disable:4200)

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

//
// @brief: 跨进程读写锁
// @copyright: Copyright 2024 FlameCyclone
// @license: 
// @birth: Created by Visual Studio 2022 on 2024-12-29
// @version: V1.0.0
// @revision: last revised by FlameCyclone on 2024-12-29
//

class CRWLockIPC
{
public:

    CRWLockIPC();
    ~CRWLockIPC();

    //
    // @brief: 初始化
    // @param: strName          锁名, 如果无锁名, 则不可跨进程
    // @param: fWritePriority   是否写操作优先
    // @param: nReadCount       最大同时读取计数
    // @ret: bool
    bool Initialize(
        const _tstring& strName = _T(""), 
        bool fWritePriority = true,
        LONG nReadCount = LONG_MAX
    );

    //
    // @brief: 反初始化
    // @ret: void
    void Uninitialize();

    //
    // @brief: 获取读锁
    // @param: dwTimeout    超时时间(毫秒)
    // @ret: bool           获取是否成功
    bool AcquireReadLock(DWORD dwTimeout = INFINITE);

    //
    // @brief: 释放读锁
    // @ret: void
    void ReleaseReadLock();

    //
    // @brief: 获取写锁
    // @param: dwTimeout    超时时间(毫秒)
    // @ret: bool           获取是否成功
    bool AcquireWriteLock(DWORD dwTimeout = INFINITE);

    //
    // @brief: 释放写锁
    // @ret: void
    void ReleaseWriteLock();

private:

    // 共享数据结构
    typedef struct _SHARE_DATA
    {
        LONG nReadWaitCount;                    // 当前读取等待计数
        LONG nWriteWaitCount;                   // 当前写入等待计数
        LONG nReadCount;                        // 当前读取计数
        LONG nWriteCount;                       // 当前写入计数
    }SHARE_DATA, * PSHARE_DATA;

    PSHARE_DATA m_pShareData;                   // 共享数据(用于共享数据结构存取)
    HANDLE m_hFileMapping;                      // 共享内存(文件映射)
    HANDLE m_hReadSemaphore;                    // 读取信号量
    HANDLE m_hWritableEvent;                    // 可写事件(无读取时有信号)
    HANDLE m_hReadableEvent;                    // 可读事件(无写入时有信号)
    HANDLE m_hWriteMutex;                       // 写入互斥量
    HANDLE m_hShareMutex;                       // 共享数据互斥量
    bool   m_fInit;                             // 初始化结果
    bool   m_fWritePriority;                    // 写优先
};
