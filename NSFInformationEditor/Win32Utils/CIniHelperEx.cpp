#include "CIniHelperEx.h"

#include <windows.h>
#include <strsafe.h>
#include "CStrUtils.h"

#define FROFILE_EXTERN_NAME                     _T(".ini")

CIniHelperEx CIniHelperEx::m_Instance;

CIniHelperEx::CIniHelperEx()
    :
    m_strFilePath(GetCurrentModuleDir())
{
    m_strFilePath += _T("\\");
    m_strFilePath += GetCurrentModuleName(false);
    m_strFilePath += FROFILE_EXTERN_NAME;

}

CIniHelperEx::CIniHelperEx(
    const _tstring& strFileName, 
    bool isAbsolutePath
)
{
    if (isAbsolutePath)
    {
        m_strFilePath = strFileName;
    }
    else
    {
        m_strFilePath = GetCurrentModuleDir();
        m_strFilePath += _T("\\");
        m_strFilePath += strFileName;
    }
}

CIniHelperEx::~CIniHelperEx()
{
}

_tstring CIniHelperEx::GetCurrentModulePath()
{
    TCHAR szCurPath[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szCurPath, _countof(szCurPath));

    _tstring strDir = szCurPath;
    return strDir;
}

_tstring CIniHelperEx::GetCurrentModuleDir()
{
    _tstring strDir = GetCurrentModulePath();
    strDir.resize(strDir.find_last_of(_T('\\')));
    return strDir;
}

_tstring CIniHelperEx::GetCurrentModuleName(bool bHasExt/* = false*/)
{
    _tstring strDir = GetCurrentModulePath();
    size_t nIndex = strDir.find_last_of(_T('\\'));
    if (nIndex != _tstring::npos)
    {
        strDir = strDir.substr(nIndex + 1);
    }

    if (!bHasExt)
    {
        nIndex = strDir.find_last_of(_T('.'));
        if (nIndex != _tstring::npos)
        {
            strDir.resize(nIndex);
        }
    }

    return strDir;
}

bool CIniHelperEx::IsDirectory(const _tstring& strPath)
{
    DWORD dwAttr = ::GetFileAttributes(strPath.c_str());
    if (INVALID_FILE_ATTRIBUTES == dwAttr)
    {
        return false;
    }

    return FILE_ATTRIBUTE_DIRECTORY == (FILE_ATTRIBUTE_DIRECTORY & dwAttr);
}

bool CIniHelperEx::Load()
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    bool bSuccess = true;

    hFile = ::CreateFile(m_strFilePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        return false;
    }

    m_kvData.clear();
    LPBYTE lpData = nullptr;
    // 读取文本
    do
    {
        LARGE_INTEGER liSize = { 0 };
        if (!::GetFileSizeEx(hFile, &liSize))
        {
            break;
        }

        lpData = (LPBYTE)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, (SIZE_T)liSize.QuadPart + 2);
        if (nullptr == lpData)
        {
            break;
        }

        DWORD nBytesToRead = 0;
        if (!::ReadFile(hFile, (LPVOID)lpData, liSize.LowPart, &nBytesToRead, NULL))
        {
            break;
        }

        bSuccess = LoadFromData(lpData, liSize.LowPart);

    } while (false);

    if (nullptr != lpData)
    {
        ::HeapFree(::GetProcessHeap(), 0, (LPVOID)lpData);
    }

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
    }

    return bSuccess;
}

bool CIniHelperEx::LoadFromData(LPCBYTE lpData, DWORD dwSize)
{
    _tstring strText;

    // UTF-16 LE
    if (dwSize > 2 &&
        (
            lpData[0] == 0xFF &&
            lpData[1] == 0xFE)
        )
    {
        strText = (LPCTSTR)(lpData + 2);
    }
    else if (dwSize > 3 &&
        (
            lpData[0] == 0xEF &&
            lpData[1] == 0xBB &&
            lpData[2] == 0xBF
            )
        )
    {
        strText = CStrUtils::U8StrToTStr(reinterpret_cast<const char*>(lpData + 3));
    }
    else
    {
        strText = CStrUtils::U8StrToTStr(reinterpret_cast<const char*>(lpData));
    }

    if (!strText.empty())
    {
        Parse(strText);
    }

    return true;
}

std::wstring CIniHelperEx::DumpWStr() const
{
    return CStrUtils::TStrToWStr(DumpStr(m_kvData));
}

std::string CIniHelperEx::DumpU8Str() const
{
    return CStrUtils::TStrToU8Str(DumpStr(m_kvData));
}

_tstring CIniHelperEx::DumpTStr() const
{
    return DumpStr(m_kvData);
}

bool CIniHelperEx::Save()
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    bool bSuccess = true;

    hFile = ::CreateFile(m_strFilePath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        return false;
    }

    do
    {
        std::string strEncode = "\xEF\xBB\xBF";
        _tstring strDump = DumpStr(m_kvData);
        std::string strUtf8 = CStrUtils::TStrToU8Str(strDump);

        bSuccess = ::WriteFile(hFile,
            strEncode.c_str(),
            3,
            nullptr,
            nullptr);

        if (!bSuccess)
        {
            break;
        }

        bSuccess = ::WriteFile(hFile, 
            strUtf8.c_str(),
            (DWORD)strUtf8.size(),
            nullptr, 
            nullptr);
        if (!bSuccess)
        {
            break;
        }

    } while (false);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
    }

    return bSuccess;
}

bool CIniHelperEx::Parse(const _tstring& strCfg)
{
    // 分割文本
    std::vector<_tstring> vTestLine = CStrUtils::SplitStr(strCfg, _T("\r\n"));

    //构建配置键值对
    _tstring strAppName;
    std::map<_tstring, _tstring> mapKeyValue;

    for (const auto& item : vTestLine)
    {
        //查找到配置节
        if (item.size() > 0 && _T('[') == item[0] && _T(']') == item[item.size() - 1])
        {
            //插入节配置
            if (!strAppName.empty())
            {
                m_kvData.insert(std::make_pair(strAppName, mapKeyValue));
            }
            strAppName = item.substr(1, item.size() - 2);
            mapKeyValue.clear();
        }
        else
        {
            auto itFind = item.find(_T("="));
            _tstring strName;
            _tstring strValue;
            if (_tstring::npos != itFind)
            {
                strName = item.substr(0, itFind);
                strValue = item.substr(itFind + 1);
            }
            else
            {
                strName = item;
            }

            // 过滤调注释
            if (strName.size() > 0 && _T('#') == strName[0])
            {
                continue;
            }

            mapKeyValue.insert(std::make_pair(strName, strValue));
        }
    }

    //插入节配置
    if (!strAppName.empty())
    {
        m_kvData.insert(std::make_pair(strAppName, mapKeyValue));
    }

    return true;
}

_tstring CIniHelperEx::DumpStr(const std::map<_tstring, IniKeyValue>& mapData) const
{
    _tstring strDump;
    size_t nAppIndex = 0;

    for (const auto& item : mapData)
    {
        strDump += _T("[") + item.first + _T("]") + _T("\r\n");
        size_t nKvIndex = 0;
        for (const auto& kv : item.second)
        {
            strDump += kv.first + _T("=") + kv.second;
            nKvIndex++;

            if (nKvIndex < item.second.size())
            {
                strDump += _T("\r\n");
            }
        }

        nAppIndex++;
        if (nAppIndex < mapData.size())
        {
            strDump += _T("\r\n");
        }
    }

    return strDump;
}

bool
CIniHelperEx::SetString(
    const _tstring& strAppName, 
    const _tstring& strKeyName, 
    const _tstring& strString
)
{
    auto itAppName = m_kvData.find(strAppName);
    if (itAppName != m_kvData.end())
    {
        auto itKeyName = itAppName->second.find(strKeyName);
        if (itKeyName != itAppName->second.end())
        {
            itKeyName->second = strString;
        }
        else
        {
            itAppName->second.insert(std::make_pair(strKeyName, strString));
        }
    }
    else
    {
        IniKeyValue kvTemp;
        kvTemp.insert(std::make_pair(strKeyName, strString));
        m_kvData.insert(
            std::make_pair(strAppName,
                kvTemp)
        );
    }

    return true;
}

_tstring
CIniHelperEx::GetString(
    const _tstring& strAppName,
    const _tstring& strKeyName,
    const _tstring& strDefault/* = _T("")*/) const
{
    _tstring strText = strDefault;

    auto itAppName = m_kvData.find(strAppName);
    if (itAppName != m_kvData.end())
    {
        auto itKeyName = itAppName->second.find(strKeyName);
        if (itKeyName != itAppName->second.end())
        {
            strText = itKeyName->second;
        }
    }

    return strText;
}

bool
CIniHelperEx::SetInt(
    const _tstring& strAppName, 
    const _tstring& strKeyName, 
    int dwNumber)
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    _itot_s(dwNumber, szBuf, 10);

    auto itAppName = m_kvData.find(strAppName);
    if (itAppName != m_kvData.end())
    {
        auto itKeyName = itAppName->second.find(strKeyName);
        if (itKeyName != itAppName->second.end())
        {
            itKeyName->second = szBuf;
        }
        else
        {
            itAppName->second.insert(std::make_pair(strKeyName, szBuf));
        }
    }
    else
    {
        IniKeyValue kvTemp;
        kvTemp.insert(std::make_pair(strKeyName, szBuf));
        m_kvData.insert(
            std::make_pair(strAppName,
                kvTemp)
        );
    }

    return true;
}

int
CIniHelperEx::GetInt(
    const _tstring& strAppName, 
    const _tstring& strKeyName, 
    int dwDefault/* = -1*/) const
{
    auto itAppName = m_kvData.find(strAppName);
    if (itAppName != m_kvData.end())
    {
        auto itKeyName = itAppName->second.find(strKeyName);
        if (itKeyName != itAppName->second.end())
        {
            _tstring strText = itKeyName->second;
            return _tcstoul(strText.c_str(), nullptr, 10);
        }
    }

    return dwDefault;
}

bool CIniHelperEx::DeleteValue(
    const _tstring& strAppName,
    const _tstring& strKeyName/* = _T("")*/)
{
    auto itAppName = m_kvData.find(strAppName);
    if (itAppName == m_kvData.end())
    {
        return false;
    }

    if (_T("") == strKeyName)
    {
        m_kvData.erase(itAppName);
        return true;
    }

    auto itKeyName = itAppName->second.find(strKeyName);
    if (itKeyName == itAppName->second.end())
    {
        return false;
    }

    itAppName->second.erase(itKeyName);
    return true;
}