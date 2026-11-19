#pragma once
#include <Windows.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <stdint.h>
#include <tchar.h>
#pragma comment(lib, "winmm.lib")

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CPCMPlayer
{
public:
    CPCMPlayer();
    ~CPCMPlayer();

    bool SyncPlay(_tstring strFile);

private:

    std::vector<int8_t> m_pcmData;
    HANDLE m_hPlayEvent;
};

