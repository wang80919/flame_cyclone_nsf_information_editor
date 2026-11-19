#pragma once

#include <wtypesbase.h>
#include <tchar.h>
#include <string>
#include <vector>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

#define HKCR                (HKEY_CLASSES_ROOT)
#define HKCU                (HKEY_CURRENT_USER)
#define HKLM                (HKEY_LOCAL_MACHINE)
#define HKU                 (HKEY_USERS)

#define HKPD                (HKEY_PERFORMANCE_DATA)
#define HKPT                (HKEY_PERFORMANCE_TEXT)
#define HKPN                (HKEY_PERFORMANCE_NLSTEXT)

#if(WINVER >= 0x0400)
#define HKCC                (HKEY_CURRENT_CONFIG)
#define HKDD                (HKEY_DYN_DATA)
#define HKCULS              (HKEY_CURRENT_USER_LOCAL_SETTINGS)
#endif

namespace CRegUtils
{
    // 写入 REG_DWORD 类型数据
    bool SetDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD dwValue);

    // 读取 REG_DWORD 类型数据
    bool GetDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD& dwValue);
    DWORD GetDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName);

    // 写入 REG_QWORD 类型数据
    bool SetQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, ULONGLONG ullValue);

    // 读取 REG_QWORD 类型数据
    bool GetQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, ULONGLONG& ullValue);
    ULONGLONG GetQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName);

    // 写入 REG_SZ 类型数据
    bool SetString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, const _tstring& strValue);

    // 读取 REG_SZ 类型数据
    bool GetString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, _tstring& strValue);
    _tstring GetString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName);

    // 写入 REG_BINARY 类型数据
    bool SetBinary(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, LPCVOID lpData, DWORD dwSize);

    // 读取 REG_BINARY 类型数据
    bool GetBinary(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, LPVOID lpData, DWORD cbSize);
    std::vector<uint8_t> GetBinary(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName);

    // 读取 指定值 所需字节大小
    bool GetValueSize(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD& cbSize);
    DWORD GetValueSize(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName);

    // 创建 指定键
    bool CreateKey(HKEY hKey, const _tstring& lpSubKey);

    // 删除 指定键(递归删除)
    bool DeleteKey(HKEY hKey, const _tstring& lpSubKey);

    // 删除 指定值
    bool DeleteValue(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName);

    // 枚举 指定键 的子键
    bool EnumKeyName(HKEY hKey, const _tstring& lpSubKey, std::vector<_tstring>& keyList);
    std::vector<_tstring> EnumKeyName(HKEY hKey, const _tstring& lpSubKey);

    // 枚举 指定键 的值名
    bool EnumValueName(HKEY hKey, const _tstring& lpSubKey, std::vector<_tstring>& valueList);
    std::vector<_tstring> EnumValueName(HKEY hKey, const _tstring& lpSubKey);
}