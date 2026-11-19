#include "CNamedPipe.h"
#include <StrSafe.h>
#include <tchar.h>

#define PIPE_NAME_PREFIX             TEXT(R"(\\.\pipe\)")    //管道前缀名
#define PIPE_MAX_TIMEOUT             (3000)                  //管道打开超时
#define PIPE_BUF_MAX_SIZE            (1024 * 1024)           //管道发送缓冲大小(字节)
#define PIPE_MAX_CONNECT             (16)                    //IPC最大连接数

typedef struct _PIPE_DATA
{
    DWORD dwSize = 0;
    BYTE data[0];
}PIPE_DATA, * PPIPE_DATA;

CNamedPipe::CNamedPipe() :
    m_pBuffer(nullptr),
    m_hNamedPipe(INVALID_HANDLE_VALUE),
    m_hReadEvent(NULL),
    m_hWriteEvent(NULL),
    m_bConnected(false),
    m_bInit(false)
{
    //初始化读写缓冲与事件句柄
    Initialize();
}

CNamedPipe::~CNamedPipe()
{
    //释放读写缓冲与事件句柄
    Uninitialize();
}

bool CNamedPipe::Create(LPCTSTR lpName)
{
    TCHAR szPipeName[MAX_PATH];
    SECURITY_ATTRIBUTES sa = { 0 };
    SECURITY_DESCRIPTOR sd = { 0 };
    bool isSuccess = false;

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    if (INVALID_HANDLE_VALUE != m_hNamedPipe)
    {
        return true;
    }

    //设置权限, 防止低权限进程不能打开高权限进程创建的管道
    (void)::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    (void)::SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
    (void)::StringCchPrintf(szPipeName, _countof(szPipeName), TEXT("%s%s"), PIPE_NAME_PREFIX, lpName);

    do
    {
        m_hNamedPipe = ::CreateNamedPipe(
            szPipeName,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            PIPE_MAX_CONNECT,
            PIPE_BUF_MAX_SIZE,
            PIPE_BUF_MAX_SIZE,
            PIPE_MAX_TIMEOUT,
            &sa
        );

        if (INVALID_HANDLE_VALUE == m_hNamedPipe)
        {
            break;
        }

        isSuccess = true;

    } while (false);

    if (!isSuccess)
    {
        this->Close();
    }

    return isSuccess;
}

bool CNamedPipe::Open(LPCTSTR lpName, DWORD nTimeOut/* = INFINITE*/)
{
    TCHAR szPipeName[MAX_PATH] = { 0 };
    bool isSuccess = false;

    (void)::StringCchPrintf(szPipeName, _countof(szPipeName), TEXT("%s%s"), PIPE_NAME_PREFIX, lpName);

    if (INVALID_HANDLE_VALUE != m_hNamedPipe)
    {
        return true;
    }

    ULONGLONG ullCurTick = ::GetTickCount64();

    do
    {
        m_hNamedPipe = ::CreateFile(
            szPipeName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            NULL
        );

        //管道句柄有效则终止循环
        if (INVALID_HANDLE_VALUE != m_hNamedPipe)
        {
            isSuccess = true;
            break;
        }

        //未找到管道
        if (ERROR_FILE_NOT_FOUND == ::GetLastError())
        {
            //执行操作超时则退出循环
            if (::GetTickCount64() - ullCurTick > nTimeOut)
            {
                break;
            }

            ::Sleep(100);

            continue;
        }

        //若错误原因不是因为所有管道范例都在使用中, 则退出循环
        if (ERROR_PIPE_BUSY != ::GetLastError())
        {
            break;
        }

        //等待命名管道的实例可用于连接
        if (::WaitNamedPipe(szPipeName, 100))
        {
            continue;
        }

        //无限等待则不需要检查超时
        if (INFINITE == nTimeOut)
        {
            continue;
        }

        //执行操作超时则退出循环
        if (::GetTickCount64() - ullCurTick > nTimeOut)
        {
            break;
        }

    } while (INVALID_HANDLE_VALUE == m_hNamedPipe);

    if (!isSuccess)
    {
        this->Close();
    }

    return isSuccess;
}

bool CNamedPipe::WaitConnect(DWORD nTimeOut)
{
    OVERLAPPED Overlapped = { 0 };
    bool isConnected = false;

    if (INVALID_HANDLE_VALUE == m_hNamedPipe)
    {
        return false;
    }

    Overlapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == Overlapped.hEvent)
    {
        return false;
    }

    isConnected = ::ConnectNamedPipe(m_hNamedPipe, &Overlapped);

    if (!isConnected)
    {
        DWORD dwError = ::GetLastError();

        //管道关闭中
        if (ERROR_NO_DATA == dwError)
        {
            isConnected = false;
        }

        //操作处于挂起状态
        if (ERROR_IO_PENDING == dwError)
        {
            if (WAIT_OBJECT_0 == ::WaitForSingleObject(Overlapped.hEvent, nTimeOut))
            {
                isConnected = true;
            }
        }

        //管道已经连接
        if (ERROR_PIPE_CONNECTED == dwError)
        {
            isConnected = true;
        }
    }

    if (NULL != Overlapped.hEvent)
    {
        ::CloseHandle(Overlapped.hEvent);
    }

    m_bConnected = isConnected;

    return isConnected;
}

bool CNamedPipe::Disconnect()
{
    if (INVALID_HANDLE_VALUE == m_hNamedPipe)
    {
        return false;
    }

    //参数句柄必须由 CreateNamedPipe 函数创建
    return ::DisconnectNamedPipe(m_hNamedPipe);
}

void CNamedPipe::Close()
{
    if (INVALID_HANDLE_VALUE != m_hNamedPipe)
    {
        if (m_bConnected)
        {
            ::FlushFileBuffers(m_hNamedPipe);
            ::DisconnectNamedPipe(m_hNamedPipe);
            m_bConnected = false;
        }
        ::CloseHandle(m_hNamedPipe);
        m_hNamedPipe = INVALID_HANDLE_VALUE;
    }
}

bool CNamedPipe::IsValid()
{
    return INVALID_HANDLE_VALUE != m_hNamedPipe;
}

bool CNamedPipe::Read(LPVOID lpData, DWORD nSize, LPDWORD lpBytesRead/* = nullptr*/, DWORD nTimeOut)
{
    OVERLAPPED Overlapped = { 0 };
    Overlapped.hEvent = m_hReadEvent;
    DWORD dwBytesTransferred = 0;
    bool isSuccess = false;

    if (nullptr == m_pBuffer ||
        nullptr == lpData ||
        0 == nSize ||
        nSize > PIPE_BUF_MAX_SIZE
        )
    {
        return false;
    }

    PPIPE_DATA pData = reinterpret_cast<PPIPE_DATA>(m_pBuffer);
    if (!::ReadFile(m_hNamedPipe, &pData->dwSize, sizeof(PIPE_DATA), NULL, &Overlapped))
    {
        //管道已结束
        if (ERROR_BROKEN_PIPE == ::GetLastError())
        {
            return false;
        }

        if (ERROR_IO_PENDING != ::GetLastError())
        {
            return false;
        }

        if (WAIT_OBJECT_0 != ::WaitForSingleObject(Overlapped.hEvent, nTimeOut))
        {
            return false;
        }
    }

    if (pData->dwSize > PIPE_BUF_MAX_SIZE)
    {
        return false;
    }

    if (!::ReadFile(m_hNamedPipe, pData->data, pData->dwSize, NULL, &Overlapped))
    {
        if (ERROR_IO_PENDING != ::GetLastError())
        {
            return false;
        }

        if (WAIT_OBJECT_0 != ::WaitForSingleObject(Overlapped.hEvent, nTimeOut))
        {
            return false;
        }
    }

    if (::GetOverlappedResult(m_hNamedPipe, &Overlapped, &dwBytesTransferred, true))
    {
        isSuccess = true;
        if (lpBytesRead)
        {
            *lpBytesRead = dwBytesTransferred;
        }
    }

    if (isSuccess)
    {
        if (nSize < pData->dwSize)
        {
            ::memcpy_s(lpData, nSize, pData->data, nSize);
        }
        else
        {
            ::memcpy_s(lpData, nSize, pData->data, pData->dwSize);
        }
    }

    return isSuccess;
}

bool CNamedPipe::Write(LPCVOID lpData, DWORD nSize, LPDWORD lpBytesWritten/* = nullptr*/, DWORD nTimeOut)
{
    OVERLAPPED Overlapped = { 0 };
    Overlapped.hEvent = m_hWriteEvent;
    DWORD dwBytesTransferred = 0;
    bool isSuccess = false;

    if (nullptr == m_pBuffer ||
        nullptr == lpData ||
        0 == nSize ||
        nSize > PIPE_BUF_MAX_SIZE
        )
    {
        return false;
    }

    PPIPE_DATA pData = reinterpret_cast<PPIPE_DATA>(m_pBuffer);
    DWORD dwBytesToWrite = nSize + sizeof(PIPE_DATA);
    pData->dwSize = nSize;
    ::memcpy_s(pData->data, PIPE_BUF_MAX_SIZE, lpData, nSize);

    if (::WriteFile(m_hNamedPipe, pData, dwBytesToWrite, NULL, &Overlapped))
    {
        return true;
    }

    //管道正在被关闭
    if (ERROR_NO_DATA == ::GetLastError())
    {
        return false;
    }

    //管道已结束
    if (ERROR_BROKEN_PIPE == ::GetLastError())
    {
        return false;
    }

    //重叠
    if (ERROR_IO_PENDING != ::GetLastError())
    {
        return false;
    }

    if (WAIT_OBJECT_0 != ::WaitForSingleObject(Overlapped.hEvent, nTimeOut))
    {
        return false;
    }

    if (::GetOverlappedResult(m_hNamedPipe, &Overlapped, &dwBytesTransferred, true))
    {
        isSuccess = true;
        if (lpBytesWritten)
        {
            *lpBytesWritten = dwBytesTransferred;
        }
    }

    return isSuccess;
}

bool CNamedPipe::Initialize()
{
    bool isSuccess = false;

    if (m_bInit)
    {
        return true;
    }

    do
    {
        if (nullptr == m_pBuffer)
        {
            m_pBuffer = ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, PIPE_BUF_MAX_SIZE + sizeof(PIPE_DATA));
        }

        if (nullptr == m_pBuffer)
        {
            break;
        }

        if (NULL == m_hReadEvent)
        {
            m_hReadEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        }

        if (NULL == m_hReadEvent)
        {
            break;
        }

        if (NULL == m_hWriteEvent)
        {
            m_hWriteEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        }

        if (NULL == m_hWriteEvent)
        {
            break;
        }

        isSuccess = true;

    } while (false);

    if (!isSuccess)
    {
        Uninitialize();
    }

    m_bInit = isSuccess;

    return m_bInit;
}

void CNamedPipe::Uninitialize()
{
    if (!m_bInit)
    {
        return;
    }

    //关闭管道
    this->Close();

    //释放读写缓冲
    if (nullptr != m_pBuffer)
    {
        ::HeapFree(::GetProcessHeap(), 0, m_pBuffer);
        m_pBuffer = nullptr;
    }

    //关闭事件
    if (m_hReadEvent)
    {
        CloseHandle(m_hReadEvent);
        m_hReadEvent = NULL;
    }

    if (m_hWriteEvent)
    {
        CloseHandle(m_hWriteEvent);
        m_hWriteEvent = NULL;
    }

    m_bInit = false;
}