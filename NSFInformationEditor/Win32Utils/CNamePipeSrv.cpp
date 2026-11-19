#include "CNamePipeSrv.h"
#include <StrSafe.h>
#include <tchar.h>

#define PIPE_NAME_PREFIX             TEXT(R"(\\.\pipe\)")    //管道前缀名
#define PIPE_MAX_TIMEOUT             (3000)                  //管道打开超时
#define PIPE_BUF_MAX_SIZE            (1024 * 4)              //管道发送缓冲大小(字节)
#define PIPE_MAX_CONNECT             (16)                    //IPC最大连接数

CNamePipeSrv::CNamePipeSrv()
{

}

CNamePipeSrv::~CNamePipeSrv()
{
}

void CNamePipeSrv::Uninitialize()
{
    for (auto& item : m_PipeTasks)
    {
        if (item.joinable())
        {
            item.join();
        }
    }
}

bool CNamePipeSrv::Initialize(LPCTSTR lpPipeName, DWORD nMaxInstances/* = 1*/)
{
    TCHAR szPipeName[MAX_PATH];
    (void)::StringCchPrintf(szPipeName, _countof(szPipeName), TEXT("%s%s"), PIPE_NAME_PREFIX, lpPipeName);
    m_strPipeName = szPipeName;

    if (nMaxInstances < 1)
    {
        nMaxInstances = 1;
    }

    if (nMaxInstances > PIPE_UNLIMITED_INSTANCES)
    {
        nMaxInstances = PIPE_UNLIMITED_INSTANCES;
    }

    m_nMaxConnect = nMaxInstances;
    //创建几个线程来处理
    for (DWORD i = 0; i < m_nMaxConnect; i++)
    {
        m_PipeTasks.push_back(std::thread(
            [this]()
            {
                PipeProc();
            }
        ));
    }

    return true;
}

void CNamePipeSrv::PipeProc()
{
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    BYTE dataBuf[PIPE_BUF_MAX_SIZE] = { 0 };
    bool isSuccess = false;

    //创建管道
    SECURITY_ATTRIBUTES sa = { 0 };
    SECURITY_DESCRIPTOR sd = { 0 };
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    //设置权限, 防止低权限进程不能打开高权限进程创建的管道
    (void)::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    (void)::SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

    hPipe = ::CreateNamedPipe(m_strPipeName.c_str(),
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        m_nMaxConnect,
        PIPE_BUF_MAX_SIZE,
        PIPE_BUF_MAX_SIZE,
        PIPE_MAX_TIMEOUT,
        &sa
    );
    if (INVALID_HANDLE_VALUE == hPipe)
    {
        return;
    }

    OVERLAPPED Overlapped = { 0 };
    Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (nullptr == Overlapped.hEvent)
    {
        return;
    }

    while (true)
    {
        //等待连接
        bool isConnected = ::ConnectNamedPipe(hPipe, &Overlapped);
        if (!isConnected)
        {
            DWORD dwError = GetLastError();
            if (ERROR_PIPE_CONNECTED == dwError)
            {
                isConnected = true;
            }

            if (ERROR_IO_PENDING == dwError)
            {
                if (WAIT_OBJECT_0 == WaitForSingleObject(Overlapped.hEvent, INFINITE))
                {
                    isConnected = true;
                }
            }
        }

        //读取数据
        while (true)
        {
            //读取管道中的数据
            isSuccess = ::ReadFile(hPipe, dataBuf, PIPE_BUF_MAX_SIZE, NULL, &Overlapped);
            if (!isSuccess && ERROR_IO_PENDING == GetLastError())
            {
                if (WAIT_OBJECT_0 == WaitForSingleObject(Overlapped.hEvent, INFINITE))
                {
                    isSuccess = (0 != Overlapped.InternalHigh);
                }
            }

            if (isSuccess)
            {
                printf("%s\r\n", (char*)dataBuf);
                continue;
            }

            //如果客户端管道关闭, 则先断开再重新等待连接
            if (ERROR_BROKEN_PIPE == GetLastError())
            {
                ::FlushFileBuffers(hPipe);
                ::DisconnectNamedPipe(hPipe);
                break;
            }
            else
            {
                //其他情况任务终止接收线程, 比方接收端关闭
                ::FlushFileBuffers(hPipe);
                ::DisconnectNamedPipe(hPipe);
                ::CloseHandle(hPipe);
                ::CloseHandle(Overlapped.hEvent);
                return;
            }
        }
    }

    ::CloseHandle(hPipe);
}
