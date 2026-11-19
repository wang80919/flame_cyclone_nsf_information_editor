#include "CDumpHelper.h"
#include <strsafe.h>
#include <Dbghelp.h>

#define DMP_FILE_SAVE_PATH_FORMAT   _T(R"(%s\%s_%04d-%02d-%02d_%02d-%02d-%02d.dmp)")
#define DMP_FILE_NAME_FORMAT        _T(R"(%s_%%04d-%%02d-%%02d_%%02d-%%02d-%%02d.dmp)")

#pragma comment(lib, "Dbghelp.lib")

CDumpHelper CDumpHelper::m_Instance;

CDumpHelper::CDumpHelper(const _tstring& strDir/* = _T("")*/, bool fFull/* = false*/, size_t nCount/* = 8*/)
    :
    m_strFileDir(strDir),
    m_fFull(fFull),
    m_nFileCount(nCount)

{
}

CDumpHelper::~CDumpHelper()
{

}

void CDumpHelper::Install(const _tstring& strDir/* = _T("Dump")*/, bool fFull/* = false*/, size_t nCount/* = 8*/)
{
    m_Instance._Install(strDir, fFull, nCount);
}

void CDumpHelper::_Install(const _tstring& strDir/* = _T("Dump")*/, bool fFull/* = false*/, size_t nCount/* = 8*/)
{
    m_strFileDir = strDir;
    m_fFull = fFull;
    m_nFileCount = nCount;
    ::SetUnhandledExceptionFilter(UnhandledExceptionFilter);
}

_tstring CDumpHelper::GetCurrentModulePath()
{
    TCHAR szCurPath[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szCurPath, _countof(szCurPath));

    _tstring strResult = szCurPath;
    return strResult;
}

_tstring CDumpHelper::GetCurrentModuleName(bool bHasExt/* = true*/)
{
    return GetFileName(GetCurrentModulePath(), bHasExt);
}

_tstring CDumpHelper::GetFileName(const _tstring& strPath, bool bHasExt/* = true*/)
{
    _tstring strResult = strPath;
    size_t nIndex = strResult.find_last_of(_T('\\'));
    if (nIndex != _tstring::npos)
    {
        strResult = strResult.substr(nIndex + 1);
    }

    if (!bHasExt)
    {
        nIndex = strResult.find_last_of(_T('.'));
        if (nIndex != _tstring::npos)
        {
            strResult.resize(nIndex);
        }
    }

    return strResult;
}

_tstring CDumpHelper::GetCurrentModuleDir()
{
    return GetFileDir(GetCurrentModulePath());
}

_tstring CDumpHelper::GetFileDir(const _tstring& strPath)
{
    _tstring strResult;
    size_t nIndex = strPath.find_last_of(_T('\\'));
    if (nIndex != _tstring::npos)
    {
        strResult = strPath.substr(0, nIndex);
    }

    return strResult;
}

bool CDumpHelper::CreateDir(const _tstring& strPath)
{
    _tstring strDriver;              //驱动器号, 如 D:
    _tstring strSubPath = strPath;   //路径, 如 Test\1\2\3

    if (strPath.empty())
    {
        return false;
    }

    //获取盘符
    do
    {
        size_t nFindIndex = strPath.find_first_of(':');  //检查是否有驱动器号
        if (nFindIndex == _tstring::npos)
        {
            break;
        }

        strDriver = strPath.substr(0, nFindIndex + 1); //得到驱动器号, 如 D:
        nFindIndex = strPath.find(_T("\\"), nFindIndex);
        if (nFindIndex == _tstring::npos)
        {
            break;
        }

        strSubPath = strPath.substr(nFindIndex + 1); //得到路径, 如 Test\1\2\3
    } while (false);

    _tstring strDestDir;
    size_t nFindBegin = 0;
    size_t nFindIndex = 0;
    do
    {
        nFindIndex = strSubPath.find(_T("\\"), nFindBegin);
        if (nFindIndex != _tstring::npos)
        {
            strDestDir = strSubPath.substr(0, nFindIndex);
            nFindBegin = nFindIndex + 1;
        }
        else
        {
            strDestDir = strSubPath;
        }

        if (!strDriver.empty())
        {
            strDestDir = strDriver + _T("\\") + strDestDir;
        }

        if (!::CreateDirectory(strDestDir.c_str(), NULL) && ERROR_ALREADY_EXISTS != ::GetLastError())
        {
            return false;
        }

    } while (nFindIndex != _tstring::npos);

    return true;
}

_tstring CDumpHelper::Format(LPCTSTR pstrFormat, ...)
{
    _tstring strResult;
    LPTSTR lpBuf = nullptr;
    DWORD dwCchCount = MAX_PATH;

    if (nullptr == pstrFormat)
    {
        return strResult;
    }

    va_list args;
    va_start(args, pstrFormat);
    do
    {
        //分配缓冲
        lpBuf = (LPTSTR)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwCchCount * sizeof(TCHAR));
        if (nullptr == lpBuf)
        {
            break;
        }

        //成功则赋值字符串并终止循环
        if (-1 != _vsntprintf_s(lpBuf, dwCchCount, _TRUNCATE, pstrFormat, args))
        {
            strResult = lpBuf;
            break;
        }

        //释放缓冲, 待下次重新分配
        ::HeapFree(::GetProcessHeap(), 0, lpBuf);
        lpBuf = nullptr;

        //缓冲字符数在合理范围内则扩大2倍
        if (dwCchCount < INT32_MAX)
        {
            dwCchCount *= 2;
        }
        else
        {
            //超限终止处理
            break;
        }

    } while (true);
    va_end(args);

    //释放缓冲
    if (nullptr != lpBuf)
    {
        ::HeapFree(::GetProcessHeap(), 0, lpBuf);
        lpBuf = nullptr;
    }

    return strResult;
}

std::map<int64_t, _tstring> CDumpHelper::_GetDumpFileList(const _tstring& strDir)
{
    _tstring strScanDir = strDir;
    if (strScanDir.empty())
    {
        strScanDir = GetCurrentModuleDir();
    }

    std::map<int64_t, _tstring> fileList;

    WIN32_FIND_DATA findData = { 0 };
    HANDLE hFindHandle = INVALID_HANDLE_VALUE;
    hFindHandle = FindFirstFile((strScanDir + _T("\\*.*")).c_str(), &findData);
    if (INVALID_HANDLE_VALUE == hFindHandle)
    {
        return fileList;
    }

    do
    {
        _tstring strName = findData.cFileName;

        //非目录
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            //检查输入规则
            int nConverted = 0;
            SYSTEMTIME st = { 0 };

            _tstring strPath = strName;
            if (!strDir.empty())
            {
                strPath = strDir + _T("\\") + strName;
            }

            _tstring strPrefix = Format(DMP_FILE_NAME_FORMAT, GetCurrentModuleName().c_str());
            nConverted = _stscanf_s(findData.cFileName, strPrefix.c_str(),
                &st.wYear, &st.wMonth, &st.wDay, &st.wHour,
                &st.wMinute, &st.wSecond, &st.wMilliseconds);

            //检查文件名规则是否符合要求
            if (6 == nConverted)
            {
                FILETIME ftFile = { 0 };
                FILETIME ftLocal = { 0 };
                int64_t timestamp = 0;

                ::SystemTimeToFileTime(&st, &ftLocal);
                ::LocalFileTimeToFileTime(&ftLocal, &ftFile);

                timestamp = ((int64_t)ftFile.dwHighDateTime << 32) | ftFile.dwLowDateTime;
                timestamp = (timestamp - 116444736000000000) / 10000;

                fileList.insert(std::make_pair(timestamp, strPath));
            }
        }

        //上一级目录与当前目录跳过
        if (0 == _tcscmp(findData.cFileName, _T(".")) || 0 == _tcscmp(findData.cFileName, _T("..")))
        {
            continue;
        }

    } while (::FindNextFile(hFindHandle, &findData));

    ::FindClose(hFindHandle);

    return fileList;
}

LONG WINAPI CDumpHelper::UnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
    SYSTEMTIME st = { 0 };

    (void)::GetLocalTime(&st);
    CDumpHelper& dumper = m_Instance;
    _tstring strDmpDir = dumper.m_strFileDir;

    if (strDmpDir.empty())
    {
        strDmpDir = GetCurrentModuleDir();
    }

    // 创建保存文件夹
    if (!CreateDir(strDmpDir))
    {
        //return EXCEPTION_EXECUTE_HANDLER;
    }

    _tstring strFilePath = Format(DMP_FILE_SAVE_PATH_FORMAT,
        strDmpDir.c_str(),
        GetCurrentModuleName().c_str(),
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    // 创建文件
    HANDLE lhDumpFile = ::CreateFile(
        strFilePath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (INVALID_HANDLE_VALUE != lhDumpFile)
    {
        MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo = { 0 };
        loExceptionInfo.ExceptionPointers = ExceptionInfo;
        loExceptionInfo.ThreadId = GetCurrentThreadId();
        loExceptionInfo.ClientPointers = TRUE;

        // 将内存转储写入到文件
        ::MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            lhDumpFile,
            dumper.m_fFull ? MiniDumpWithFullMemory : MiniDumpNormal,
            &loExceptionInfo,
            nullptr,
            nullptr
        );

        ::CloseHandle(lhDumpFile);
    }

    // 文件数量限定
    if (dumper.m_nFileCount > 0)
    {
        std::map<int64_t, _tstring> dumpFileList = dumper._GetDumpFileList(strDmpDir);
        if (dumpFileList.size() > dumper.m_nFileCount)
        {
            size_t nDeleteCount = dumpFileList.size() - dumper.m_nFileCount;

            // 从日志文件记录列表中删除
            for (size_t i = 0; i < nDeleteCount; i++)
            {
                auto itBegin = dumpFileList.begin();
                ::DeleteFile(itBegin->second.c_str());
                dumpFileList.erase(dumpFileList.begin());
            }
        }
    }

    return EXCEPTION_EXECUTE_HANDLER;
}