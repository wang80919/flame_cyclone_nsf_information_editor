#pragma once

#include <wtypesbase.h>
#include <string>
#include <vector>
#include <locale>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CConsoleHelper
{
public:
    CConsoleHelper();
    ~CConsoleHelper();

    //
    // @brief: 运行Exe程序
    // @param: strExePath   可执行程序路径
    // @param: strCmd       命令行
    // @param: strCurDir    程序的当前路径
    // @param: bNeedInput   是否需要输入内容
    // @param: bNeedOutput  是否需要输出内容
    // @ret: bool
    bool RunProcess(
        const _tstring& strExePath = _T(""),
        const _tstring& strCmd = _T(""),
        const _tstring& strCurDir = _T(""),
        bool bNeedInput = true,
        bool bNeedOutput = true
    );

    //
    // @brief: 运行Exe程序并获取其输出
    // @param: strExePath   可执行程序路径
    // @param: strCmd       命令行
    // @param: strCurDir    程序的当前路径
    // @param: dwTimeout  超时(毫秒)
    // @ret: _tstring 输出内容
    static _tstring RunProcessGetOutput(
        const _tstring& strExePath = _T(""),
        const _tstring& strCmd = _T(""),
        const _tstring& strCurDir = _T(""),
        DWORD dwTimeout = INFINITE
    );

    //
    // @brief: 运行Exe程序(可输入输出)
    // @param: strExePath   可执行程序路径
    // @param: strCmd       命令行
    // @param: strCurDir    程序的当前路径
    bool RunProcessWithInputOutput(
        const _tstring& strExePath = _T(""),
        const _tstring& strCmd = _T(""),
        const _tstring& strCurDir = _T("")
    );

    //
    // @brief: 运行Exe程序(仅输入)
    // @param: strExePath   可执行程序路径
    // @param: strCmd       命令行
    // @param: strCurDir    程序的当前路径
    bool RunProcessOnlyInput(
        const _tstring& strExePath = _T(""),
        const _tstring& strCmd = _T(""),
        const _tstring& strCurDir = _T("")
    );

    //
    // @brief: 运行Exe程序(仅输出)
    // @param: strExePath   可执行程序路径
    // @param: strCmd       命令行
    // @param: strCurDir    程序的当前路径
    bool RunProcessOnlyOutput(
        const _tstring& strExePath = _T(""),
        const _tstring& strCmd = _T(""),
        const _tstring& strCurDir = _T("")
    );

    //
    // @brief: 关闭
    // @ret: bool
    bool Close(void);

    //
    // @brief: 读取内容
    // @param: strContent  输出内容
    // @param: dwTimeout  超时(毫秒)
    // @param: bNotEmptyReturn 是否读取到内容就返回
    // @ret: bool
    bool ReadOutput(_tstring& strContent, DWORD dwTimeout = INFINITE, bool bNotEmptyReturn = false);

    //
    // @brief: 写入内容
    // @param: strContent   输入内容
    // @ret: bool
    bool WriteInput(const _tstring& strContent);

    //
    // @brief: 检测是否处于运行状态
    // @ret: bool
    bool IsRunning(void) const;

    //
    // @brief: 获取退出码
    // @ret: DWORD
    DWORD GetExitCode();

    // 
    // @brief: 获取文件所在文件夹
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 文件夹        如 D:\Software
    _tstring GetFileDir(const _tstring& strPath);

private:

    //
    // @brief: 运行Exe程序
    // @param: strExePath   可执行程序路径
    // @param: strCmd       命令行
    // @param: strCurDir    程序的当前路径
    // @param: bNeedInput   是否需要输入内容
    // @param: bNeedOutput  是否需要输出内容
    // @ret: bool
    bool _RunProcess(
        const _tstring& strExePath = _T(""),
        const _tstring& strCmd = _T(""),
        const _tstring& strCurDir = _T(""),
        bool bNeedInput = true,
        bool bNeedOutput = true,
        bool bWait = false,
        DWORD dwTimeout = INFINITE
    );

    //宽字符转其他
    static std::string _WStrToMultiStr(UINT CodePage, const std::wstring& str);

    //其他转宽字符
    static std::wstring _MultiStrToWStr(UINT CodePage, const std::string& str);

    static _tstring _AStrToTStr(const std::string& str);
    static std::string _TStrToAStr(const _tstring& str);

private:
    HANDLE m_hPipeInWr;     //子进程输入写管道句柄
    HANDLE m_hPipeInRd;     //子进程输入读管道句柄
    HANDLE m_hPipeOutWr;    //子进程输出写管道句柄
    HANDLE m_hPipeOutRd;    //子进程输出读管道句柄
    PROCESS_INFORMATION m_childPI;  //控制台进程信息
};