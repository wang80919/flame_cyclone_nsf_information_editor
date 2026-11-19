#pragma once

#include <wtypesbase.h>
#include <string>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CFileHelper
{
public:
    static bool GetAttributes(const _tstring& strPath, DWORD& dwAttr);
    static bool SetAttributes(const _tstring& strPath, const DWORD& dwAttr);

public:
    CFileHelper();
    CFileHelper(const CFileHelper&) = delete;
    CFileHelper(const _tstring& strPath, bool bReadonly = false);
    ~CFileHelper();

    bool Open(const _tstring& strPath, bool bCreateIfNoExist = true, bool bReadonly = false);
    void Close();
    bool IsOpen() const;
    bool GetSize(unsigned long long& ullSize) const;
    DWORD GetAttributes() const;
    bool SetAttributes(const DWORD& dwAttr) const;
    LONGLONG GetPointer() const;
    bool SetPointer(LONGLONG llPos = 0, DWORD dwFlag = FILE_CURRENT);
    bool Read(LPVOID lpBuffer, DWORD dwSize, LONGLONG llPos = 0, LPDWORD lpBytesRead = nullptr, DWORD dwFlag = FILE_CURRENT);
    bool Write(LPCVOID lpBuffer, DWORD dwSize, LONGLONG llPos = 0, LPDWORD lpBytesWritten = nullptr, DWORD dwFlag = FILE_CURRENT);
    bool Clear();
    bool SetEnd(LONGLONG llPos);

private:
    HANDLE m_hFile;
    _tstring m_strFile;
};
