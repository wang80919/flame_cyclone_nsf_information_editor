#include <strsafe.h>
#include "CConsoleHelper.h"

#define PIPE_READ_DATA_SIZE         (1024 * 1024)
#define PIPE_READ_TIME_INTERVAL     (50)

std::string CConsoleHelper::_WStrToMultiStr(UINT CodePage, const std::wstring& str)
{
    //计算缓冲区所需的字节长度
    int cbMultiByte = ::WideCharToMultiByte(CodePage, 0, str.c_str(), -1, NULL, 0, NULL, 0);
    std::string strResult(cbMultiByte, 0);

    //成功则返回写入到指示的缓冲区的字节数
    size_t nConverted = ::WideCharToMultiByte(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size(), NULL, NULL);

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
}

std::wstring CConsoleHelper::_MultiStrToWStr(UINT CodePage, const std::string& str)
{
    //计算缓冲区所需的字符长度
    int cchWideChar = ::MultiByteToWideChar(CodePage, 0, str.c_str(), -1, NULL, 0);
    std::wstring strResult(cchWideChar, 0);

    //成功则返回写入到指示的缓冲区的字符数
    size_t nConverted = ::MultiByteToWideChar(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size());

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
}

_tstring CConsoleHelper::_AStrToTStr(const std::string& str)
{
#ifdef _UNICODE
    return _MultiStrToWStr(CP_ACP, str);
#else
    return str;
#endif
}

std::string CConsoleHelper::_TStrToAStr(const _tstring& str)
{
#ifdef _UNICODE
    return _WStrToMultiStr(CP_ACP, str);
#else
    return str;
#endif
}

CConsoleHelper::CConsoleHelper()
    :
    m_hPipeInWr(NULL),
    m_hPipeInRd(NULL),
    m_hPipeOutWr(NULL),
    m_hPipeOutRd(NULL)
{
    ZeroMemory(&m_childPI, sizeof(m_childPI));
}

CConsoleHelper::~CConsoleHelper()
{
    Close();
}

bool CConsoleHelper::RunProcess(
    const _tstring& strExePath/* = _T("")*/,
    const _tstring& strCmd/* = _T("")*/,
    const _tstring& strCurDir/* = _T("")*/,
    bool bNeedInput/* = true*/,
    bool bNeedOutput/* = true*/
)
{
    return _RunProcess(strExePath, strCmd, strCurDir, bNeedInput, bNeedOutput, false, 0);
}

bool CConsoleHelper::RunProcessWithInputOutput(
    const _tstring& strExePath/* = _T("")*/,
    const _tstring& strCmd/* = _T("")*/,
    const _tstring& strCurDir/* = _T("")*/
)
{
    return _RunProcess(strExePath, strCmd, strCurDir, true, true, false, 0);
}

_tstring CConsoleHelper::RunProcessGetOutput(
    const _tstring& strExePath/* = _T("")*/,
    const _tstring& strCmd/* = _T("")*/,
    const _tstring& strCurDir/* = _T("")*/,
    DWORD dwTimeout/* = INFINITE*/
)
{
    CConsoleHelper Console;

    _tstring strContent;
    Console._RunProcess(strExePath, strCmd, strCurDir, false, true, false, dwTimeout);
    Console.ReadOutput(strContent, dwTimeout, false);
    Console.Close();
    return strContent;
}

bool CConsoleHelper::RunProcessOnlyInput(
    const _tstring& strExePath/* = _T("")*/,
    const _tstring& strCmd/* = _T("")*/,
    const _tstring& strCurDir/* = _T("")*/
)
{
    return _RunProcess(strExePath, strCmd, strCurDir, true, false, false, 0);
}

bool CConsoleHelper::RunProcessOnlyOutput(
    const _tstring& strExePath/* = _T("")*/,
    const _tstring& strCmd/* = _T("")*/,
    const _tstring& strCurDir/* = _T("")*/
)
{
    return _RunProcess(strExePath, strCmd, strCurDir, false, true, false, 0);
}

bool CConsoleHelper::Close(void)
{
    if (NULL != m_hPipeInWr)
    {
        ::CloseHandle(m_hPipeInWr);
        m_hPipeInWr = NULL;
    }
    if (NULL != m_hPipeInRd)
    {
        ::CloseHandle(m_hPipeInRd);
        m_hPipeInRd = NULL;
    }
    if (NULL != m_hPipeOutWr)
    {
        ::CloseHandle(m_hPipeOutWr);
        m_hPipeOutWr = NULL;
    }
    if (NULL != m_hPipeOutRd)
    {
        ::CloseHandle(m_hPipeOutRd);
        m_hPipeOutRd = NULL;
    }

    if (IsRunning())
    {
        ::TerminateProcess(m_childPI.hProcess, (UINT)(-1));
        ::CloseHandle(m_childPI.hProcess);
        m_childPI.hProcess = NULL;
    }

    return true;
}

bool CConsoleHelper::ReadOutput(_tstring& strContent, DWORD dwTimeout/* = INFINITE*/, bool bNotEmptyReturn/* = false*/)
{
    strContent.clear();

    if (NULL == m_hPipeOutRd)
    {
        return false;
    }

    std::string strRead;
    DWORD readBytes = 0;
    ULONGLONG ullBegin = ::GetTickCount64();

    LPSTR lpReadBuf = NULL;
    lpReadBuf = (char*)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, PIPE_READ_DATA_SIZE);
    if (nullptr == lpReadBuf)
    {
        return false;
    }

    //先检索管道中的数据
    while (::PeekNamedPipe(m_hPipeOutRd, lpReadBuf, PIPE_READ_DATA_SIZE - 1, &readBytes, 0, NULL))
    {
        //无数据则等待
        if (0 == readBytes)
        {
            //超时则退出
            if (INFINITE != dwTimeout && (::GetTickCount64() - ullBegin >= dwTimeout))
            {
                break;
            }

            //进程未运行则退出, 防止进程退出了还在这里浪费时间
            if (!IsRunning())
            {
                break;
            }

            ::Sleep(PIPE_READ_TIME_INTERVAL);
            continue;
        }

        //有数据则读取数据
        if (::ReadFile(m_hPipeOutRd, lpReadBuf, PIPE_READ_DATA_SIZE - 1, &readBytes, NULL))
        {
            lpReadBuf[readBytes] = '\0';
            strRead += lpReadBuf;
        }

        //如果设置了读取到数据就返回(最好不要设置, 读取到数据后进程不一定后续没有数据)
        if (bNotEmptyReturn && !strRead.empty())
        {
            break;
        }
    }

    if (nullptr != lpReadBuf)
    {
        ::HeapFree(::GetProcessHeap(), 0, lpReadBuf);
        lpReadBuf = nullptr;
    }

    strContent = _AStrToTStr(strRead);

    return false;
}

bool CConsoleHelper::WriteInput(const _tstring& strContent)
{
    if (!IsRunning())
    {
        return false;
    }

    if (NULL == m_hPipeInRd)
    {
        return false;
    }

    std::string strWrite = _TStrToAStr(strContent);
    strWrite += "\r\n";

    DWORD writeBytes = 0;
    if (FALSE == ::WriteFile(m_hPipeInWr, strWrite.c_str(), (DWORD)strWrite.size(), &writeBytes, NULL))
    {
        return false;
    }

    return true;
}

bool CConsoleHelper::IsRunning(void) const
{
    DWORD dwExitCode = 0;

    if ((NULL != m_childPI.hProcess) &&
        (::GetExitCodeProcess(m_childPI.hProcess, &dwExitCode))
        )
    {
        return STILL_ACTIVE == dwExitCode;
    }

    return false;
}

DWORD CConsoleHelper::GetExitCode()
{
    DWORD dwExitCode = 0;

    if ((NULL != m_childPI.hProcess) &&
        (::GetExitCodeProcess(m_childPI.hProcess, &dwExitCode))
        )
    {
        return dwExitCode;
    }

    return (DWORD)-1;
}

_tstring CConsoleHelper::GetFileDir(const _tstring& strPath)
{
    _tstring strResult;
    size_t nIndex = strPath.find_last_of(_T('\\'));
    if (nIndex != _tstring::npos)
    {
        strResult = strPath.substr(0, nIndex);
    }

    return strResult;
}

bool CConsoleHelper::_RunProcess(
    const _tstring& strExePath/* = _T("")*/,
    const _tstring& strCmd/* = _T("")*/,
    const _tstring& strCurDir/* = _T("")*/,
    bool bNeedInput/* = true*/,
    bool bNeedOutput/* = true*/,
    bool bWait/* = false*/,
    DWORD dwTimeout/* = INFINITE*/
)
{
    TCHAR szCmdBuf[MAX_PATH] = { 0 };
    SECURITY_ATTRIBUTES   sa = { 0 };
    STARTUPINFO si = { 0 };
    LPCTSTR lpCurrentDir = NULL;
    _tstring strExeCurrentDir = strCurDir;

    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    sa.nLength = sizeof(sa);

    //创建子进程的标准输出管道
    if (!::CreatePipe(&m_hPipeOutRd, &m_hPipeOutWr, &sa, 0))
    {
        return false;
    }

    ::SetHandleInformation(m_hPipeOutRd, HANDLE_FLAG_INHERIT, 0);

    //创建子进程的标准输入管道
    if (!::CreatePipe(&m_hPipeInRd, &m_hPipeInWr, &sa, 0))
    {
        ::CloseHandle(m_hPipeInWr);
        ::CloseHandle(m_hPipeInRd);
        ::CloseHandle(m_hPipeOutWr);
        ::CloseHandle(m_hPipeOutRd);
        return false;
    }

    ::SetHandleInformation(m_hPipeInWr, HANDLE_FLAG_INHERIT, 0);

    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_ERROR_HANDLE);

    if (bNeedInput)
    {
        si.hStdInput = m_hPipeInRd;
    }

    if (bNeedOutput)
    {
        si.hStdOutput = m_hPipeOutWr;
        si.hStdError = m_hPipeOutWr;
    }

    //exe路径不为空
    if (!strExePath.empty())
    {
        if (!strCmd.empty())
        {
            ::StringCchPrintf(szCmdBuf, _countof(szCmdBuf), _T("\"%s\" %s"), strExePath.c_str(), strCmd.c_str());
        }
        else
        {
            ::StringCchPrintf(szCmdBuf, _countof(szCmdBuf), _T("\"%s\""), strExePath.c_str());
        }
    }
    //exe路径为空
    else
    {
        ::StringCchPrintf(szCmdBuf, _countof(szCmdBuf), _T("%s"), strCmd.c_str());
    }

    //如果未指定进程当前目录, 则使用进程所在目录
    if (strExeCurrentDir.empty())
    {
        strExeCurrentDir = GetFileDir(strExePath);
    }

    if (!strExeCurrentDir.empty())
    {
        lpCurrentDir = strExeCurrentDir.c_str();
    }

    if (!::CreateProcess(NULL, szCmdBuf, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, lpCurrentDir, &si, &m_childPI))
    {
        ::CloseHandle(m_hPipeInWr);
        ::CloseHandle(m_hPipeInRd);
        ::CloseHandle(m_hPipeOutWr);
        ::CloseHandle(m_hPipeOutRd);
        m_hPipeInWr = NULL;
        m_hPipeInRd = NULL;
        m_hPipeOutWr = NULL;
        m_hPipeOutRd = NULL;
        ZeroMemory(&m_childPI, sizeof(m_childPI));
        return false;
    }

    ::CloseHandle(m_childPI.hThread);
    m_childPI.hThread = NULL;

    if (bWait)
    {
        ::WaitForSingleObject(m_childPI.hProcess, dwTimeout);
    }

    return true;
}
