#pragma once
#include <string>
#include <windows.h>
#include <tchar.h>

#pragma warning(disable:4200)

class CNamedPipe
{
public:
    CNamedPipe();

    ~CNamedPipe();

    CNamedPipe(const CNamedPipe& r) = delete;
    CNamedPipe& operator = (const CNamedPipe& r) = delete;

    //
    // @brief: 创建命名管道
    // @param: lpName           管道名
    // @ret: bool               true: 创建成功 false: 创建失败
    bool Create(LPCTSTR lpName);

    //
    // @brief: 等待客户端连接命名管道
    // @param: nTimeOut         超时等待(毫秒)
    // @ret: bool               true: 连接成功 false: 连接失败
    bool WaitConnect(DWORD nTimeOut = INFINITE);

    //
    // @brief: 关闭由Create 创建的管道
    // @param: void
    // @ret: bool               true: 关闭 成功 false: 关闭 失败
    bool Disconnect();

    //
    // @brief: 打开已存在的命名管道
    // @param: lpName           管道名
    // @ret: bool               true: 打开成功 false: 打开失败
    bool Open(LPCTSTR lpName, DWORD nTimeOut = INFINITE);

    //
    // @brief: 管道是否有效
    // @param: void
    // @ret: bool               true: 可用 false: 无效
    bool IsValid();

    //
    // @brief: 关闭管道
    // @param: void
    // @ret: void
    void Close(void);

    //
    // @brief: 从读取管道数据
    // @param: lpData           数据存放缓冲
    // @param: nSize            缓冲大小(字节)
    // @param: lpBytesRead      指向实际读取大小(字节)的指针
    // @param: nTimeOut         读取超时(毫秒)
    // @ret: bool               true: 读取成功 false: 读取失败
    bool Read(LPVOID lpData, DWORD nSize, LPDWORD lpBytesRead = nullptr, DWORD nTimeOut = INFINITE);

    //
    // @brief: 向管道写入数据
    // @param: lpData           写入数据指针
    // @param: nSize            写入数据大小(字节)
    // @param: lpBytesWritten   指向实际写入大小(字节)的指针
    // @param: nTimeOut         写入超时(毫秒)
    // @ret: bool               true: 写入成功 false: 写入失败
    bool Write(LPCVOID lpData, DWORD nSize, LPDWORD lpBytesWritten = nullptr, DWORD nTimeOut = INFINITE);

private:

    //
    // @brief: 初始化对象占用
    // @param: void
    // @ret: void
    bool Initialize();

    //
    // @brief: 释放对象占用
    // @param: void
    // @ret: void
    void Uninitialize();

private:
    HANDLE m_hNamedPipe = INVALID_HANDLE_VALUE;
    HANDLE m_hReadEvent = NULL;
    HANDLE m_hWriteEvent = NULL;
    LPVOID m_pBuffer = nullptr;
    bool m_bInit = false;
    bool m_bConnected = false;
};

