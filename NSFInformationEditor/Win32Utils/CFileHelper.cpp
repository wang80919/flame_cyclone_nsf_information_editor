#include "CFileHelper.h"

CFileHelper::CFileHelper()
    :
    m_hFile(INVALID_HANDLE_VALUE)
{

}

CFileHelper::CFileHelper(const _tstring& strPath, bool bReadonly/* = false*/)
{
    Open(strPath, true, bReadonly);
}

CFileHelper::~CFileHelper()
{
    Close();
}

bool CFileHelper::IsOpen() const
{
    return INVALID_HANDLE_VALUE != m_hFile;
}

bool CFileHelper::GetSize(unsigned long long& ullSize) const
{
    if (!IsOpen())
    {
        return false;
    }

    LARGE_INTEGER ullFileSize = { 0 };
    if (!::GetFileSizeEx(m_hFile, &ullFileSize))
    {
        return false;
    }

    ullSize = ullFileSize.QuadPart;
    return true;
}

bool CFileHelper::GetAttributes(const _tstring& strPath, DWORD& dwAttr)
{
    WIN32_FILE_ATTRIBUTE_DATA attr = { 0 };
    if (!::GetFileAttributesEx(strPath.c_str(), GetFileExInfoStandard, &attr))
    {
        return false;
    }

    dwAttr = attr.dwFileAttributes;
    return true;
}

bool CFileHelper::SetAttributes(const _tstring& strPath, const DWORD& dwAttr)
{
    return ::SetFileAttributes(strPath.c_str(), dwAttr);
}

DWORD CFileHelper::GetAttributes() const
{
    DWORD dwAttr = 0;
    if (IsOpen())
    {
        GetAttributes(m_strFile, dwAttr);
    }
    return dwAttr;
}

bool CFileHelper::SetAttributes(const DWORD& dwAttr) const
{
    if (!IsOpen())
    {
        return false;
    }

    return SetAttributes(m_strFile, dwAttr);
}

bool CFileHelper::Open(const _tstring& strPath, bool bCreateIfNoExist/* = false*/, bool bReadonly/* = false*/)
{
    if (strPath.empty())
    {
        return false;
    }

    //不重复打开相同文件
    if (m_strFile == strPath && IsOpen())
    {
        return true;
    }

    //关闭文件, 如果已经打开
    Close();

    m_strFile = strPath;
    DWORD dwOpenFlag = GENERIC_READ;
    if (!bReadonly)
    {
        dwOpenFlag |= GENERIC_WRITE;
    }

    m_hFile = ::CreateFile(m_strFile.c_str(),
        dwOpenFlag,
        FILE_SHARE_READ,
        NULL,
        bCreateIfNoExist ? OPEN_ALWAYS : OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        return false;
    }

    return true;
}

void CFileHelper::Close()
{
    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        ::CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

LONGLONG CFileHelper::GetPointer() const
{
    LARGE_INTEGER  liDistanceToMove = { 0 };
    LARGE_INTEGER  liNewFilePointer = { 0 };
    ::SetFilePointerEx(m_hFile, liDistanceToMove, &liNewFilePointer, FILE_CURRENT);

    return liNewFilePointer.QuadPart;
}

bool CFileHelper::SetPointer(LONGLONG llPos/* = 0*/, DWORD dwFlag/* = FILE_CURRENT*/)
{
    LARGE_INTEGER  liDistanceToMove = { 0 };

    if (!IsOpen())
    {
        return false;
    }

    liDistanceToMove.QuadPart = llPos;
    return ::SetFilePointerEx(m_hFile, liDistanceToMove, NULL, dwFlag);
}

bool CFileHelper::Read(LPVOID lpBuffer, DWORD dwSize, LONGLONG llPos/* = 0*/, LPDWORD lpBytesRead/* = nullptr*/, DWORD dwFlag/* = FILE_CURRENT*/)
{
    if (!IsOpen() || nullptr == lpBuffer)
    {
        return false;
    }

    LARGE_INTEGER  liDistanceToMove = { 0 };
    BOOL bResult = FALSE;
    liDistanceToMove.QuadPart = llPos;
    ::SetFilePointerEx(m_hFile, liDistanceToMove, NULL, dwFlag);
    DWORD dwBytesRead = 0;

    bResult = ::ReadFile(m_hFile, lpBuffer, dwSize, &dwBytesRead, NULL);
    if (nullptr != lpBytesRead)
    {
        *lpBytesRead = dwBytesRead;
    }

    return bResult;
}

bool CFileHelper::Write(LPCVOID lpBuffer, DWORD dwSize, LONGLONG llPos/* = 0*/, LPDWORD lpBytesWritten/* = nullptr*/, DWORD dwFlag/* = FILE_CURRENT*/)
{
    if (!IsOpen() || nullptr == lpBuffer)
    {
        return false;
    }

    LARGE_INTEGER  liDistanceToMove = { 0 };
    BOOL bResult = FALSE;
    liDistanceToMove.QuadPart = llPos;
    ::SetFilePointerEx(m_hFile, liDistanceToMove, NULL, dwFlag);

    DWORD dwBytesWritten = 0;
    bResult = ::WriteFile(m_hFile, lpBuffer, dwSize, &dwBytesWritten, NULL);

    if (nullptr != lpBytesWritten)
    {
        *lpBytesWritten = dwBytesWritten;
    }

    return bResult;
}

bool CFileHelper::Clear()
{
    if (!IsOpen())
    {
        return false;
    }

    LARGE_INTEGER  liDistanceToMove = { 0 };
    ::SetFilePointerEx(m_hFile, liDistanceToMove, NULL, FILE_BEGIN);
    return ::SetEndOfFile(m_hFile);
}

bool CFileHelper::SetEnd(LONGLONG llPos)
{
    if (!IsOpen())
    {
        return false;
    }

    LARGE_INTEGER  liDistanceToMove = { 0 };
    liDistanceToMove.QuadPart = llPos;
    ::SetFilePointerEx(m_hFile, liDistanceToMove, NULL, FILE_BEGIN);
    return ::SetEndOfFile(m_hFile);
}