#pragma once

#include <string>
#include <tchar.h>
#include <map>
#include <vector>
#include <windows.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

using IniKeyValue = std::map<_tstring, _tstring>;
using IniAppKv = std::map<_tstring, IniKeyValue>;

class CIniHelperEx
{
public:
    CIniHelperEx();
    CIniHelperEx(const _tstring& strFileName, bool isAbsolutePath = false);
    ~CIniHelperEx();

    bool Load();
    bool LoadFromData(LPCBYTE lpData, DWORD dwSize);
    std::wstring DumpWStr() const;
    std::string DumpU8Str() const;
    _tstring DumpTStr() const;
    bool Save();

    //读写字符串
    bool SetString(
        const _tstring& strAppName, 
        const _tstring& strKeyName, 
        const _tstring& strString);

    _tstring GetString(
        const _tstring& strAppName, 
        const _tstring& strKeyName, 
        const _tstring& strDefault = _T("")) const;

    //读写数字
    bool SetInt(
        const _tstring& strAppName, 
        const _tstring& strKeyName, 
        int dwNumber);

    int GetInt(
        const _tstring& strAppName, 
        const _tstring& strKeyName, 
        int dwDefault = -1) const;

    //删除配置值
    bool DeleteValue(
        const _tstring& strAppName, 
        const _tstring& strKeyName = _T(""));

    static CIniHelperEx& Instance()
    {
        return m_Instance;
    }

private:

    bool Parse(const _tstring& strCfg);
    _tstring DumpStr(const std::map<_tstring, IniKeyValue>& mapData) const;

    static _tstring GetCurrentModulePath();
    static _tstring GetCurrentModuleDir();
    static _tstring GetCurrentModuleName(bool bHasExt = false);
    static bool IsDirectory(const _tstring& strPath);

private:
    _tstring m_strFilePath;
    std::map<_tstring, IniKeyValue> m_kvData;
    static CIniHelperEx m_Instance;
};