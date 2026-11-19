#include "CFileMapping.h"

typedef struct _FILE_MAPPING_DATA
{
    DWORD dwCapacity = 0;       //最大容量
    DWORD dwPid = 0;            //进程ID
    DWORD dwSize = 0;           //数据大小
    BYTE Data[0];               //数据内容
}FILE_MAPPING_DATA;

typedef const FILE_MAPPING_DATA* LCPFILE_MAPPING_DATA;
typedef FILE_MAPPING_DATA* LPFILE_MAPPING_DATA;

CFileMapping::CFileMapping()
    :
    m_hFileMapping(nullptr),
    m_hData(nullptr),
    m_hMutex(nullptr),
    m_bInit(false)
{

}

CFileMapping::~CFileMapping()
{
    Uninitialize();
}

bool CFileMapping::Initialize(
    const _tstring& strName/* = _T("")*/,
    DWORD dwSize/* = 4096*/
)
{
    SECURITY_ATTRIBUTES sa = { 0 };
    SECURITY_DESCRIPTOR sd = { 0 };
    bool fSuccess = false;
    bool fCreate = false;

    if (m_bInit)
    {
        return true;
    }

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

    (void)::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    (void)::SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

    _tstring strFileMapping = strName.empty() ? strName : strName + _T("_ShareData");
    _tstring strMutex = strName.empty() ? strName : strName + _T("_Mutex");

    do
    {
        // 创建访问互斥量
        m_hMutex = ::CreateMutex(
            &sa, 
            FALSE, 
            strMutex.empty() ? NULL : strMutex.c_str()
        );
        if (nullptr == m_hMutex)
        {
            break;
        }

        // 创建共享内存
        m_hFileMapping = ::CreateFileMapping(
            INVALID_HANDLE_VALUE, 
            &sa, 
            PAGE_READWRITE, 
            0, 
            dwSize + sizeof(FILE_MAPPING_DATA), 
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

        m_hData = (LPVOID)::MapViewOfFile(m_hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
        if (nullptr == m_hData)
        {
            break;
        }

        //首次创建, 设置信息
        ::WaitForSingleObject(m_hMutex, INFINITE);
        if (fCreate)
        {
            LPFILE_MAPPING_DATA pData = reinterpret_cast<LPFILE_MAPPING_DATA>(m_hData);
            pData->dwCapacity = dwSize;
            pData->dwPid = ::GetCurrentProcessId();
        }
        ::ReleaseMutex(m_hMutex);

        fSuccess = true;

    } while (false);

    if (!fSuccess)
    {
        Uninitialize();
    }

    m_bInit = fSuccess;

    return fSuccess;
}

void CFileMapping::Uninitialize()
{
    if (false == m_bInit)
    {
        return;
    }

    if (m_hData)
    {
        ::UnmapViewOfFile(m_hData);
        m_hData = nullptr;
    }

    if (m_hFileMapping)
    {
        ::CloseHandle(m_hFileMapping);
        m_hFileMapping = nullptr;
    }

    if (m_hMutex)
    {
        ::CloseHandle(m_hMutex);
        m_hMutex = nullptr;
    }

    m_bInit = false;
}

bool CFileMapping::IsInitialized() const
{
    return m_bInit;
}

bool CFileMapping::Write(LPCVOID lpData, DWORD dwSize)
{
    bool isSuccess = false;
    DWORD dwWait = WAIT_OBJECT_0;

    if ((nullptr == m_hMutex) || (nullptr == m_hData) || (nullptr == lpData) || (0 == dwSize))
    {
        return false;
    }

    LPFILE_MAPPING_DATA pData = reinterpret_cast<LPFILE_MAPPING_DATA>(m_hData);

    do
    {
        dwWait = ::WaitForSingleObject(m_hMutex, INFINITE);
        if (WAIT_OBJECT_0 != dwWait)
        {
            break;
        }

        if (dwSize <= pData->dwCapacity)
        {
            ::memcpy_s(pData->Data, pData->dwCapacity, lpData, dwSize);
            pData->dwSize = dwSize;
            isSuccess = true;
        }

    } while (false);

    ::ReleaseMutex(m_hMutex);

    return isSuccess;
}

bool CFileMapping::Read(LPVOID lpData, DWORD dwSize, LPDWORD lpBytesRead/* = nullptr*/)
{
    bool isSuccess = false;
    DWORD dwWait = WAIT_OBJECT_0;

    if ((nullptr == m_hMutex) || (nullptr == m_hData) || (nullptr == lpData) || (0 == dwSize))
    {
        return false;
    }

    LCPFILE_MAPPING_DATA pData = reinterpret_cast<LCPFILE_MAPPING_DATA>(m_hData);

    do
    {
        dwWait = ::WaitForSingleObject(m_hMutex, INFINITE);
        if (WAIT_OBJECT_0 != dwWait)
        {
            break;
        }

        if (dwSize <= (pData->dwCapacity))
        {
            ::memcpy_s(lpData, dwSize, pData->Data, pData->dwSize);
            if (nullptr != lpBytesRead)
            {
                *lpBytesRead = pData->dwSize;
            }
            isSuccess = true;
        }

    } while (false);

    ::ReleaseMutex(m_hMutex);

    return isSuccess;
}

DWORD CFileMapping::GetCapacity() const
{
    if ((nullptr == m_hMutex) || (nullptr == m_hData))
    {
        return 0;
    }

    LCPFILE_MAPPING_DATA pData = reinterpret_cast<LCPFILE_MAPPING_DATA>(m_hData);

    return pData->dwCapacity;
}

DWORD CFileMapping::GetSize()
{
    if ((nullptr == m_hMutex) || (nullptr == m_hData))
    {
        return 0;
    }

    LCPFILE_MAPPING_DATA pData = reinterpret_cast<LCPFILE_MAPPING_DATA>(m_hData);

    return pData->dwSize;
}

LPVOID CFileMapping::GetDataPtr()
{
    if ((nullptr == m_hMutex) || (nullptr == m_hData))
    {
        return NULL;
    }

    LCPFILE_MAPPING_DATA pData = reinterpret_cast<LCPFILE_MAPPING_DATA>(m_hData);

    return (LPVOID)&pData->Data;
}