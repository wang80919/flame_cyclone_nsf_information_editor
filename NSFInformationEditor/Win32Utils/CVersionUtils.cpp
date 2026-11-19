#include "CVersionUtils.h"
#include <winver.h>
#include <strsafe.h>

#pragma comment(lib, "Version.lib")

namespace CVersionUtils
{

#pragma pack(push)
#pragma pack(1)
    // 包含文件的版本信息。 此信息与语言和代码页无关
    // https://learn.microsoft.com/zh-cn/windows/win32/menurc/vs-versioninfo
    typedef struct {
        WORD             wLength;       // VS_VERSIONINFO 结构的长度（以字节为单位）,此长度不包括在 32 位边界上对齐任何后续版本资源数据的填充
        WORD             wValueLength;  // Value 成员的长度（以字节为单位）
        WORD             wType;         // 版本资源中的数据类型, 1: 资源包含文本数据 0: 版本资源包含二进制数据
        WCHAR            szKey[15];     // Unicode 字符串 L“VS_VERSION_INFO”
        WORD             Padding1;      // 在 32 位边界上对齐 Children 成员所需的任意或零个 WORD
        //VS_FIXEDFILEINFO Value
        //WORD             Padding2
        //WORD             Children
    } VS_VERSIONINFO, * PVS_VERSIONINFO;

#pragma pack(pop)

    CVersionNumber::CVersionNumber()
        :m_nVer{ 0 }
    {
    };

    CVersionNumber::CVersionNumber(const _tstring& strVer)
        :
        m_nVer{ 0 }
    {
        _stscanf_s(strVer.c_str(), _T("%hd.%hd.%hd.%hd"), &m_nVer[0], &m_nVer[1], &m_nVer[2], &m_nVer[3]);
    }

    CVersionNumber::CVersionNumber(WORD v1, WORD v2, WORD v3, WORD v4)
        :m_nVer{ v1, v2, v3, v4 }
    {
    }

    CVersionNumber& CVersionNumber::operator = (const _tstring& strVer)
    {
        _stscanf_s(strVer.c_str(), _T("%hd.%hd.%hd.%hd"), &m_nVer[0], &m_nVer[1], &m_nVer[2], &m_nVer[3]);
        return *this;
    }

    int CVersionNumber::_Compare(const CVersionNumber& ref) const
    {
        for (int i = 0; i < _countof(m_nVer); i++)
        {
            if (m_nVer[i] != ref.m_nVer[i])
            {
                return (m_nVer[i] > ref.m_nVer[i] ? 1 : -1);
            }
        }

        return 0;
    }

    _tstring CVersionNumber::GetString() const
    {
        TCHAR szBuf[MAX_PATH] = { 0 };
        (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("%hd.%hd.%hd.%hd"),
            m_nVer[0],
            m_nVer[1],
            m_nVer[2],
            m_nVer[3]
        );

        return szBuf;
    }

    bool CVersionNumber::IsEmpty() const
    {
        for (const auto& item : m_nVer)
        {
            if (0 != item)
            {
                return false;
            }
        }

        return true;
    }

    void CVersionNumber::Clear()
    {
        for (auto& item : m_nVer)
        {
            item = 0;
        }
    }

    bool CVersionNumber::operator == (const CVersionNumber& ref)
    {
        return _Compare(ref) == 0;
    }

    bool CVersionNumber::operator != (const CVersionNumber& ref)
    {
        return _Compare(ref) != 0;
    }

    bool CVersionNumber::operator < (const CVersionNumber& ref)
    {
        return _Compare(ref) < 0;
    }

    bool CVersionNumber::operator <= (const CVersionNumber& ref)
    {
        return _Compare(ref) <= 0;
    }

    bool CVersionNumber::operator > (const CVersionNumber& ref)
    {
        return _Compare(ref) > 0;
    }

    bool CVersionNumber::operator >= (const CVersionNumber& ref)
    {
        return _Compare(ref) >= 0;
    }

    _LANGANDCODEPAGE::_LANGANDCODEPAGE() : wLangId(0), wCodePage(0)
    {

    }

    bool _LANGANDCODEPAGE::operator < (const _LANGANDCODEPAGE& r) const
    {
        if (this->wLangId < r.wLangId)
        {
            return true;
        }

        if (this->wCodePage < r.wCodePage)
        {
            return true;
        }

        return false;
    }

    bool _LANGANDCODEPAGE::operator == (const _LANGANDCODEPAGE& r) const
    {
        return this->wLangId == r.wLangId && this->wCodePage == r.wCodePage;
    }

    LANG_INFO GetLanguageInfo(LANGID dwLang)
    {
        TCHAR szBuf[MAX_PATH] = { 0 };
        LANG_INFO langInfo;

        //获取当前线程 语言ID
        LANGID LangId = ::GetThreadUILanguage();

        ::GetLocaleInfo(dwLang, LOCALE_SNAME, szBuf, _countof(szBuf));

        langInfo.wLangId = dwLang;
        langInfo.strLocaleName = szBuf;

        ::SetThreadUILanguage(dwLang);
        ::VerLanguageName(dwLang, szBuf, _countof(szBuf));

        langInfo.strLangName = szBuf;

        // 恢复当前线程 语言ID
        ::SetThreadUILanguage(LangId);

        return langInfo;
    }

    LANG_LIST GetLangList()
    {
        LANG_LIST langList;
        LANGID LangId = ::GetThreadUILanguage();

        ::EnumUILanguages([](LPTSTR  name, LONG_PTR lparam) -> BOOL {
            LANG_LIST* pList = (LANG_LIST*)lparam;
            LANGID LangId = _tcstoul(name, nullptr, 16);

            LANG_INFO langInfo = GetLanguageInfo(LangId);
            pList->emplace(LangId, langInfo);

            return TRUE;

            },
            MUI_LANGUAGE_ID,
            (LONG_PTR)&langList
        );

        ::SetThreadUILanguage(LangId);

        return langList;
    }

    static VERSION_LIST _GetFileVersionList(const _tstring& strFile, bool fMultilingual)
    {
        VERSION_LIST infoResult;
        PVOID pFsRedirectionOldValue = NULL;
        bool isDisableWow64Fs = false;
        UINT cbTranslate = 0;
        DWORD dwVerSize;
        LPVOID lpVerData = NULL;

        // 加载文件
        if (strFile.empty())
        {
            return infoResult;
        }

        // 禁用文件重定向
        isDisableWow64Fs = ::Wow64DisableWow64FsRedirection(&pFsRedirectionOldValue);

        LANG_LIST langList = GetLangList();
        LANGID LangId = ::GetThreadUILanguage();

        // 多语言获取信息
        for (const auto& item : langList)
        {
            if (fMultilingual)
            {
                ::SetThreadUILanguage(item.first);
            }
            else
            {
                ::SetThreadUILanguage(LangId);
            }

            do
            {
                // 获取版本信息数据大小
                dwVerSize = ::GetFileVersionInfoSize(strFile.c_str(), NULL);
                if (0 == dwVerSize)
                {
                    break;
                }

                // 分配版本信息缓冲
                lpVerData = ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwVerSize);
                if (!lpVerData)
                {
                    break;
                }

                // 获取版本信息
                BOOL fResult = FALSE;
                fResult = ::GetFileVersionInfo(strFile.c_str(), NULL, dwVerSize, lpVerData);
                if (!fResult)
                {
                    break;
                }

                // 获取语言代码
                LANGANDCODEPAGE* lpTranslate = NULL;
                if (!::VerQueryValue(lpVerData, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbTranslate))
                {
                    break;
                }

                DWORD dwCount = cbTranslate / sizeof(LANGANDCODEPAGE);
                for (DWORD i = 0; i < dwCount; i++)
                {
                    LANGANDCODEPAGE langCodePage = lpTranslate[i];
                    VERSION_INFO versionInfo;

                    // 获取 VS_FIXEDFILEINFO 信息
                    PVS_VERSIONINFO lpVersion = (PVS_VERSIONINFO)lpVerData;
                    if (0 != lpVersion->wValueLength)
                    {
                        VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)lpVersion + sizeof(VS_VERSIONINFO));

                        DWORD dwAlign = 4;
                        DWORD_PTR dwPadding = ((DWORD_PTR)pFixedFileInfo % dwAlign);
                        if (0 != dwPadding)
                        {
                            pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)pFixedFileInfo + (dwAlign - dwPadding));
                        }

                        versionInfo.vsFixedInfo = *((PVS_VER_FIXEDFILEINFO)pFixedFileInfo);
                    }

                    // 查询信息回调
                    auto _QueryInfo = [](const _tstring& strName, const LPVOID lpData, const LANGANDCODEPAGE& code) {

                        TCHAR strQuery[MAX_PATH] = { 0 };
                        LPCTSTR lpQueryRes = NULL;
                        UINT uQueryCchSize;
                        (void)::StringCchPrintf(strQuery, _countof(strQuery),
                            _T("\\StringFileInfo\\%04x%04x\\%s"),
                            code.wLangId, code.wCodePage, strName.c_str());
                        if (::VerQueryValue(lpData, strQuery, (LPVOID*)&lpQueryRes, &uQueryCchSize))
                        {
                            return lpQueryRes;
                        }

                        return _T("");

                        };

                    // 获取版本字符串回调
                    auto _GetVersionStr = [](VERSON_NUMBER hi, VERSON_NUMBER lo) -> _tstring {
                        TCHAR szBuf[MAX_PATH] = { 0 };
                        (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("%hd.%hd.%hd.%hd"),
                            hi.Version.wHigh,
                            hi.Version.wLow,
                            lo.Version.wHigh,
                            lo.Version.wLow
                        );

                        return szBuf;

                        };

                    versionInfo.langAndCodePage = langCodePage;
                    versionInfo.strComments = _QueryInfo(_T("Comments"), lpVerData, langCodePage);
                    versionInfo.strInternalName = _QueryInfo(_T("InternalName"), lpVerData, langCodePage);
                    versionInfo.strProductName = _QueryInfo(_T("ProductName"), lpVerData, langCodePage);
                    versionInfo.strCompanyName = _QueryInfo(_T("CompanyName"), lpVerData, langCodePage);
                    versionInfo.strLegalCopyright = _QueryInfo(_T("LegalCopyright"), lpVerData, langCodePage);
                    versionInfo.strProductVersion = _QueryInfo(_T("ProductVersion"), lpVerData, langCodePage);
                    versionInfo.strFileDescription = _QueryInfo(_T("FileDescription"), lpVerData, langCodePage);
                    versionInfo.strLegalTrademarks = _QueryInfo(_T("LegalTrademarks"), lpVerData, langCodePage);
                    versionInfo.strPrivateBuild = _QueryInfo(_T("PrivateBuild"), lpVerData, langCodePage);
                    versionInfo.strFileVersion = _QueryInfo(_T("FileVersion"), lpVerData, langCodePage);
                    versionInfo.strOriginalFilename = _QueryInfo(_T("OriginalFilename"), lpVerData, langCodePage);
                    versionInfo.strSpecialBuild = _QueryInfo(_T("SpecialBuild"), lpVerData, langCodePage);

                    VS_VER_FIXEDFILEINFO& vsFixedInfo = versionInfo.vsFixedInfo;
                    versionInfo.strFileVersionEx = _GetVersionStr(vsFixedInfo.dwFileVersionMS, vsFixedInfo.dwFileVersionLS);
                    versionInfo.strProductVersionEx = _GetVersionStr(vsFixedInfo.dwProductVersionMS, vsFixedInfo.dwProductVersionLS);

                    versionInfo.wLangId = item.second.wLangId;
                    versionInfo.strLocaleName = item.second.strLocaleName;
                    versionInfo.strLangName = item.second.strLangName;

                    versionInfo.FileVerNumber = versionInfo.strFileVersionEx.c_str();
                    versionInfo.ProductVerNumber = versionInfo.strProductVersionEx.c_str();

                    infoResult.emplace(versionInfo.strLocaleName, versionInfo);
                }

            } while (false);

            if (!fMultilingual)
            {
                break;
            }
        }

        if (fMultilingual)
        {
            ::SetThreadUILanguage(LangId);
        }

        // 恢复文件重定向
        if (isDisableWow64Fs)
        {
            ::Wow64RevertWow64FsRedirection(pFsRedirectionOldValue);
        }

        if (lpVerData)
        {
            ::HeapFree(::GetProcessHeap(), 0, lpVerData);
            lpVerData = NULL;
        }

        return infoResult;
    }

    VERSION_LIST GetFileVersionList(const _tstring& strFile, bool fMultilingual/* = true*/)
    {
        VERSION_LIST fileInfos = _GetFileVersionList(strFile, fMultilingual);
        return fileInfos;
    }

    VERSION_INFO GetFileVersion(const _tstring& strFile, bool fMultilingual/* = true*/)
    {
        VERSION_INFO info;
        VERSION_LIST fileInfos = _GetFileVersionList(strFile, fMultilingual);
        if (!fileInfos.empty())
        {
            info = fileInfos.begin()->second;
        }

        return info;
    }
}
