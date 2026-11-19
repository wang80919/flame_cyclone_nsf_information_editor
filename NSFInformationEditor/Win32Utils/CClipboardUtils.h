#pragma once

#include <string>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CClipboardUtils
{
public:

    static bool SetText(const _tstring& strText);
    static bool GetText(_tstring& strText);
    static bool Empty();

};