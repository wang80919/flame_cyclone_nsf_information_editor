#pragma once
#include <windows.h>
#include <tchar.h>
#include <string>
#include <thread>
#include <functional>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

using DirectoryActionCallback = std::function<void(const _tstring& strRoot, const _tstring& strPath, DWORD dwAction)>;

class CDirectoryMonitor
{
public:

    CDirectoryMonitor();
    ~CDirectoryMonitor();

    //启动监视任务
    bool Start(_tstring strDirectory, DirectoryActionCallback cb = nullptr);

    //关闭监视任务
    void Close();

    static _tstring _WStrToTStr(const std::wstring& str);

private:

    bool _DoTask();

    //宽字符转其他
    static std::string _WStrToMultiStr(UINT CodePage, const std::wstring& str);

private:

    _tstring m_strDirectory;
    std::thread m_taskThread;
    DirectoryActionCallback m_Callback;
    HANDLE m_hDirectory;
    bool m_bStop;
};

