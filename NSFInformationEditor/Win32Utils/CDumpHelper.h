#pragma once

#include <string>
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <map>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CDumpHelper
{
public:

    // 
    // @brief: 安装Dump
    // @param: strDir               存放目录
    // @param: fFull                是否完全转储
    // @param: nCount               内存转储文件数量(0: 无限制)
    // @ret: void
    static void Install(const _tstring& strDir = _T("Dump"), bool fFull = false, size_t nCount = 8);

private:

    // 
    // @brief: 构造
    // @param: strDir               存放目录
    // @param: fFull                是否完全转储
    // @param: nCount               内存转储文件数量(0: 无限制)
    CDumpHelper(const _tstring & = _T("Dump"), bool fFull = false, size_t nCount = 8);
    ~CDumpHelper();

    // 
    // @brief: 安装Dump
    // @param: strDir               存放目录
    // @param: fFull                是否完全转储
    // @param: nCount               内存转储文件数量(0: 无限制)
    // @ret: void
    void _Install(const _tstring & = _T("Dump"), bool fFull = false, size_t nCount = 8);

    // 
    // @brief: 获取当前进程名
    // @ret: 当前进程名 如 HxDPortableSetup.exe
    static _tstring GetCurrentModuleName(bool bHasExt = false);

    // 
    // @brief: 获取当前进程完全路径
    // @ret: 当前进程完全路径 如 D:\Software\HxDPortableSetup.exe
    static _tstring GetCurrentModulePath();

    // 
    // @brief: 获取文件名
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @param: bHasExt     是否包含扩展名
    // @ret: 文件夹        如 HxDPortableSetup
    static _tstring GetFileName(const _tstring& strPath, bool bHasExt = false);

    // 
    // @brief: 获取当前进程所在目录
    // @ret: 当前进程所在目录 如 D:\Software
    static _tstring GetCurrentModuleDir();

    // 
    // @brief: 获取文件所在文件夹
    // @param: strPath              文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 文件夹                 如 D:\Software
    static _tstring GetFileDir(const _tstring& strPath);

    //
    // @brief: 创建目录(递归)
    // @param: strPath              路径
    // @ret: 成功返回true
    static bool CreateDir(const _tstring& strPath);

    //
    // @brief: 格式化字符串
    // @param: void
    // @ret: bool 执行结果
    static _tstring Format(LPCTSTR pstrFormat, ...);

    //
    // @brief: 获取目录下文件路径
    // @ret: std::vector<_tstring> 日志文件列表
    static std::map<int64_t, _tstring> _GetDumpFileList(const _tstring& strDir);

    //
    // @brief: 异常处理回调
    // @param: pExceptionInfo       异常信息
    // @ret: LONG                   处理结果
    static LONG WINAPI UnhandledExceptionFilter(struct _EXCEPTION_POINTERS* pExceptionInfo);

private:

    static CDumpHelper m_Instance;
    _tstring m_strFileDir;
    size_t m_nFileCount;
    bool m_fFull;
};