#include "CRegUtils.h"

namespace CRegUtils
{
    // 注册表大小限制, 参见 https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits
#define REG_KEY_NAME_CCH_LIMITS             (255)            // 注册表键名大小限制(项名称包括注册表中项的绝对路径，始终从基项开始)
#define REG_VALUE_NAME_CB_LIMITS            (MAX_PATH)       // 注册表值名大小限制(260 个 ANSI 字符或 16,383 个 Unicode 字符。)
#define REG_VALUE_DATA_CB_LIMITS            (1024 * 1024)    // 注册表值数据大小限制(可用内存（最新格式） 1 MB（标准格式）)

    bool SetDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD dwValue)
    {
        LSTATUS ls = ERROR_SUCCESS;

        ls = ::RegSetKeyValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), REG_DWORD, &dwValue, sizeof(dwValue));
        if (ERROR_SUCCESS != ls)
        {
            return false;
        }

        return ERROR_SUCCESS == ls;
    }

    bool GetDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD& dwValue)
    {
        LSTATUS ls = ERROR_SUCCESS;
        DWORD cbSize = sizeof(dwValue);

        ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_DWORD/* | RRF_ZEROONFAILURE*/, NULL, &dwValue, &cbSize);
        if (ERROR_SUCCESS != ls)
        {
            return false;
        }

        return ERROR_SUCCESS == ls;
    }

    DWORD GetDword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName)
    {
        DWORD dwResult = 0;
        (void)GetDword(hKey, lpSubKey, lpValueName, dwResult);
        return dwResult;
    }

    bool SetQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, ULONGLONG ullValue)
    {
        LSTATUS ls = ERROR_SUCCESS;

        ls = ::RegSetKeyValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), REG_QWORD, &ullValue, sizeof(ullValue));
        if (ERROR_SUCCESS != ls)
        {
            return false;
        }

        return ERROR_SUCCESS == ls;
    }

    bool GetQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, ULONGLONG& ullValue)
    {
        LSTATUS ls = ERROR_SUCCESS;
        DWORD cbSize = sizeof(ullValue);

        ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_QWORD/* | RRF_ZEROONFAILURE*/, NULL, &ullValue, &cbSize);
        if (ERROR_SUCCESS != ls)
        {
            return false;
        }

        return ERROR_SUCCESS == ls;
    }

    ULONGLONG GetQword(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName)
    {
        ULONGLONG ullResult = 0;
        (void)GetQword(hKey, lpSubKey, lpValueName, ullResult);
        return ullResult;
    }

    bool SetString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, const _tstring& strValue)
    {
        LSTATUS ls = ERROR_SUCCESS;

        ls = ::RegSetKeyValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), REG_SZ, strValue.c_str(), (DWORD)((strValue.length() + 1) * sizeof(TCHAR)));
        if (ERROR_SUCCESS != ls)
        {
            return false;
        }

        return ERROR_SUCCESS == ls;
    }

    bool GetString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, _tstring& strValue)
    {
        LSTATUS ls = ERROR_SUCCESS;
        DWORD dwSize = 0;
        LPVOID lpBuf = NULL;

        //获取数据长度
        ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_SZ/* | RRF_ZEROONFAILURE*/, NULL, 0, &dwSize);
        if (ERROR_SUCCESS != ls)
        {
            return false;
        }

        // 数据长度超过常规大小限制, 则重新分配空间
        lpBuf = HeapAlloc(GetProcessHeap(), 0, dwSize);
        if (lpBuf)
        {
            ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_SZ /*| RRF_ZEROONFAILURE*/, NULL, lpBuf, &dwSize);
            if (ERROR_SUCCESS == ls)
            {
                strValue = (LPCTSTR)lpBuf;
            }

            HeapFree(GetProcessHeap(), 0, lpBuf);
        }

        return ERROR_SUCCESS == ls;
    }

    _tstring GetString(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName)
    {
        _tstring strResult;
        (void)GetString(hKey, lpSubKey, lpValueName, strResult);
        return strResult;
    }

    bool SetBinary(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, LPCVOID lpData, DWORD cbSize)
    {
        LSTATUS ls = ERROR_SUCCESS;
        ls = ::RegSetKeyValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), REG_BINARY, lpData, cbSize);
        return ERROR_SUCCESS == ls;
    }

    bool GetBinary(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, LPVOID lpData, DWORD cbSize)
    {
        LSTATUS ls = ERROR_SUCCESS;
        ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_REG_BINARY/* | RRF_ZEROONFAILURE*/, NULL, lpData, &cbSize);
        return ERROR_SUCCESS == ls;
    }

    std::vector<uint8_t> GetBinary(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName)
    {
        std::vector<uint8_t> vResult;
        DWORD dwSize = GetValueSize(hKey, lpSubKey, lpValueName);

        if (dwSize > 0)
        {
            vResult.resize(dwSize);
            (void)GetBinary(hKey, lpSubKey, lpValueName, &vResult[0], (DWORD)vResult.size());
        }
        return vResult;
    }

    bool GetValueSize(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName, DWORD& cbSize)
    {
        LSTATUS ls = ERROR_SUCCESS;
        ls = ::RegGetValue(hKey, lpSubKey.c_str(), lpValueName.c_str(), RRF_RT_ANY, NULL, nullptr, &cbSize);
        return (ERROR_SUCCESS == ls) || (ERROR_MORE_DATA == ls);
    }

    DWORD GetValueSize(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName)
    {
        DWORD dwResult = 0;
        (void)GetValueSize(hKey, lpSubKey, lpValueName, dwResult);
        return dwResult;
    }

    bool CreateKey(HKEY hKey, const _tstring& lpSubKey)
    {
        LSTATUS ls = ERROR_SUCCESS;
        HKEY hNewKey = NULL;
        DWORD dwDisposition = 0;

        ls = ::RegCreateKeyEx(hKey, lpSubKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey, &dwDisposition);

        if (hNewKey)
        {
            ::RegCloseKey(hNewKey);
        }

        return ERROR_SUCCESS == ls;
    }

    bool DeleteKey(HKEY hKey, const _tstring& lpSubKey)
    {
        LSTATUS ls = ERROR_SUCCESS;
        ls = ::RegDeleteTree(hKey, lpSubKey.c_str());
        return ERROR_SUCCESS == ls;
    }

    bool DeleteValue(HKEY hKey, const _tstring& lpSubKey, const _tstring& lpValueName)
    {
        LSTATUS ls = ERROR_SUCCESS;
        ls = ::RegDeleteKeyValue(hKey, lpSubKey.c_str(), lpValueName.c_str());
        return ERROR_SUCCESS == ls;
    }

    bool EnumKeyName(HKEY hKey, const _tstring& lpSubKey, std::vector<_tstring>& keyList)
    {
        TCHAR szKeyName[MAX_PATH] = { 0 };
        DWORD dwCchName = _countof(szKeyName);
        LSTATUS ls = ERROR_SUCCESS;
        HKEY hResultKey = NULL;
        DWORD dwIndex = 0;

        keyList.clear();

        ls = ::RegOpenKeyEx(hKey, lpSubKey.c_str(), 0, KEY_READ, &hResultKey);
        if (ERROR_SUCCESS != ls)
        {
            return false;
        }

        do
        {
            dwCchName = _countof(szKeyName);
            ls = ::RegEnumKeyEx(hResultKey, dwIndex++, szKeyName, &dwCchName, NULL, NULL, NULL, NULL);
            if (ERROR_SUCCESS == ls)
            {
                keyList.push_back(szKeyName);
            }
        } while (ERROR_SUCCESS == ls);

        ::RegCloseKey(hResultKey);

        return true;
    }

    std::vector<_tstring> EnumKeyName(HKEY hKey, const _tstring& lpSubKey)
    {
        std::vector<_tstring> vResult;
        (void)EnumKeyName(hKey, lpSubKey, vResult);
        return vResult;
    }

    bool EnumValueName(HKEY hKey, const _tstring& lpSubKey, std::vector<_tstring>& valueList)
    {
        TCHAR szValueName[MAX_PATH] = { 0 };
        DWORD dwCchName = _countof(szValueName);
        LSTATUS ls = ERROR_SUCCESS;
        HKEY hResultKey = NULL;
        DWORD dwIndex = 0;

        valueList.clear();

        ls = ::RegOpenKeyEx(hKey, lpSubKey.c_str(), 0, KEY_READ, &hResultKey);
        if (ERROR_SUCCESS != ls)
        {
            return false;
        }

        do
        {
            dwCchName = _countof(szValueName);
            ls = ::RegEnumValue(hResultKey, dwIndex++, szValueName, &dwCchName, NULL, NULL, NULL, NULL);
            if (ERROR_SUCCESS == ls)
            {
                valueList.push_back(szValueName);
            }
        } while (ERROR_SUCCESS == ls);

        ::RegCloseKey(hResultKey);

        return true;
    }

    std::vector<_tstring> EnumValueName(HKEY hKey, const _tstring& lpSubKey)
    {
        std::vector<_tstring> vResult;
        (void)EnumValueName(hKey, lpSubKey, vResult);
        return vResult;
    }
}