#pragma once
#include <string>
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <thread>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

typedef struct _PIPE_INFO
{
    HANDLE hThread = nullptr;            //线程句柄
    HANDLE hPipeSrv = nullptr;            //服务端管道句柄
    HANDLE hPipeClient = nullptr;        //客户端管道句柄
}PIPE_INFO, *PPIPE_INFO;

class CNamePipeSrv
{
public:
    CNamePipeSrv();
    ~CNamePipeSrv();

    CNamePipeSrv(const CNamePipeSrv& r) = delete;
    CNamePipeSrv& operator = (const CNamePipeSrv& r) = delete;

    bool Initialize(LPCTSTR lpPipeName, DWORD nMaxInstances = 1);
    void Uninitialize();

private:

    void PipeProc();

private:

    _tstring m_strPipeName;
    std::vector<std::thread> m_PipeTasks;
    DWORD m_nMaxConnect = 0;
};

