#include "CLnkHelper.h"
#include <shlguid.h>

CLnkHelper::CLnkHelper()
    :
    m_SheelLink(nullptr),
    m_ppf(nullptr),
    m_bInit(false)
{
    this->Initialized();
}


CLnkHelper::~CLnkHelper()
{
    this->Uninitialize();
}

bool CLnkHelper::Initialized()
{
    HRESULT hr = S_OK;
    hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        return false;
    }

    m_bInit = true;

    hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&m_SheelLink);
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_SheelLink->QueryInterface(IID_IPersistFile, (void**)&m_ppf);
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void CLnkHelper::Uninitialize()
{
    if (m_ppf)
    {
        m_ppf->Release();
        m_ppf = nullptr;
    }

    if (m_SheelLink)
    {
        m_SheelLink->Release();
        m_SheelLink = nullptr;
    }

    if (m_bInit)
    {
        (void)::CoUninitialize();
    }
}

std::wstring CLnkHelper::_TStrToWStr(const _tstring& str)
{
#ifdef _UNICODE
    return str;
#else
    //计算缓冲区所需的字符长度
    int cchWideChar = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    std::wstring strResult(cchWideChar, 0);

    //成功则返回写入到指示的缓冲区的字符数
    size_t nConverted = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size());

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
#endif
}

bool CLnkHelper::Load(const _tstring& strFile)
{
    HRESULT hr = S_OK;

    if (!m_ppf)
    {
        return false;
    }

    hr = m_ppf->Load(_TStrToWStr(strFile).c_str(), STGM_READWRITE);
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

bool CLnkHelper::Save(const _tstring& strFile)
{
    HRESULT hr = S_OK;
    if (!m_ppf)
    {
        return false;
    }

    hr = m_ppf->Save(_TStrToWStr(strFile).c_str(), true);
    return SUCCEEDED(hr);
}

SHELL_LINK_INFO CLnkHelper::GetInfo() const
{
    SHELL_LINK_INFO info;

    if (m_SheelLink)
    {
        WIN32_FIND_DATA wfd = { 0 };
        TCHAR szBuf[MAX_PATH * 4] = { 0 };
        HRESULT hr = S_OK;
        hr = m_SheelLink->GetPath(szBuf, _countof(szBuf), &wfd, SLGP_RAWPATH);
        if (SUCCEEDED(hr))
        {
            info.strFile = szBuf;
        }

        hr = m_SheelLink->GetPath(szBuf, _countof(szBuf), &wfd, SLGP_RELATIVEPRIORITY);
        if (SUCCEEDED(hr))
        {
            info.strPathRel = szBuf;
        }

        hr = m_SheelLink->GetDescription(szBuf, _countof(szBuf));
        if (SUCCEEDED(hr))
        {
            info.strDesc = szBuf;
        }

        hr = m_SheelLink->GetWorkingDirectory(szBuf, _countof(szBuf));
        if (SUCCEEDED(hr))
        {
            info.strDir = szBuf;
        }

        hr = m_SheelLink->GetArguments(szBuf, _countof(szBuf));
        if (SUCCEEDED(hr))
        {
            info.strArgs = szBuf;
        }

        int iIcon = 0;
        hr = m_SheelLink->GetIconLocation(szBuf, _countof(szBuf), &iIcon);
        if (SUCCEEDED(hr))
        {
            info.strIconPath = szBuf;
            info.iIcon = iIcon;
        }

        int iShowCmd = 0;
        hr = m_SheelLink->GetShowCmd(&iShowCmd);
        if (SUCCEEDED(hr))
        {
            info.iShowCmd = iShowCmd;
        }

        WORD wHotkey = 0;
        hr = m_SheelLink->GetHotkey(&wHotkey);
        if (SUCCEEDED(hr))
        {
            info.wHotkey = wHotkey;
        }
    }

    return info;
}

bool CLnkHelper::SetInfo(const SHELL_LINK_INFO& info)
{
    if (!m_SheelLink)
    {
        return false;
    }

    m_SheelLink->SetPath(info.strFile.c_str());
    m_SheelLink->SetRelativePath(info.strPathRel.c_str(), 0);
    m_SheelLink->SetDescription(info.strDesc.c_str());
    m_SheelLink->SetArguments(info.strArgs.c_str());
    m_SheelLink->SetIconLocation(info.strIconPath.c_str(), info.iIcon);
    m_SheelLink->SetWorkingDirectory(info.strDir.c_str());
    m_SheelLink->SetHotkey(info.wHotkey);
    m_SheelLink->SetShowCmd(info.iShowCmd);

    return true;
}

bool CLnkHelper::Resolve(HWND hwnd, DWORD fFlags)
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->Resolve(hwnd, fFlags);
    return SUCCEEDED(hr);
}

bool CLnkHelper::GetPath(_tstring& strFile, DWORD fFlags/* = SLGP_RAWPATH*/) const
{
    if (!m_SheelLink)
    {
        return false;
    }

    WIN32_FIND_DATA wfd = { 0 };
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    HRESULT hr = S_OK;
    hr = m_SheelLink->GetPath(szBuf, _countof(szBuf), &wfd, fFlags);
    if (SUCCEEDED(hr))
    {
        strFile = szBuf;
    }

    return SUCCEEDED(hr);
}

_tstring CLnkHelper::GetPath(DWORD fFlags/* = SLGP_RAWPATH*/) const
{
    if (!m_SheelLink)
    {
        return _tstring();
    }

    WIN32_FIND_DATA wfd = { 0 };
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    HRESULT hr = S_OK;
    hr = m_SheelLink->GetPath(szBuf, _countof(szBuf), &wfd, fFlags);
    if (SUCCEEDED(hr))
    {
        return szBuf;
    }

    return _tstring();
}

bool CLnkHelper::SetPath(const _tstring& strFile)
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->SetPath(strFile.c_str());
    return SUCCEEDED(hr);
}

bool CLnkHelper::GetDescription(_tstring& strDesc) const
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    hr = m_SheelLink->GetDescription(szBuf, _countof(szBuf));
    if (SUCCEEDED(hr))
    {
        strDesc = szBuf;
    }

    return SUCCEEDED(hr);
}

_tstring CLnkHelper::GetDescription() const
{
    if (!m_SheelLink)
    {
        return _tstring();
    }

    HRESULT hr = S_OK;
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    hr = m_SheelLink->GetDescription(szBuf, _countof(szBuf));
    if (SUCCEEDED(hr))
    {
        return szBuf;
    }

    return _tstring();
}

bool CLnkHelper::SetDescription(const _tstring& strDesc)
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->SetDescription(strDesc.c_str());

    return SUCCEEDED(hr);
}

bool CLnkHelper::GetWorkingDirectory(_tstring& strDir) const
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    hr = m_SheelLink->GetWorkingDirectory(szBuf, _countof(szBuf));
    if (SUCCEEDED(hr))
    {
        strDir = szBuf;
    }

    return SUCCEEDED(hr);
}

_tstring CLnkHelper::GetWorkingDirectory() const
{
    if (!m_SheelLink)
    {
        return _tstring();
    }

    HRESULT hr = S_OK;
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    hr = m_SheelLink->GetWorkingDirectory(szBuf, _countof(szBuf));
    if (SUCCEEDED(hr))
    {
        return szBuf;
    }

    return _tstring();
}

bool CLnkHelper::SetWorkingDirectory(const _tstring& strDir)
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->SetWorkingDirectory(strDir.c_str());
    return SUCCEEDED(hr);
}

bool CLnkHelper::GetArguments(_tstring& strArgs) const
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    hr = m_SheelLink->GetArguments(szBuf, _countof(szBuf));
    if (SUCCEEDED(hr))
    {
        strArgs = szBuf;
    }

    return SUCCEEDED(hr);
}

_tstring CLnkHelper::GetArguments() const
{
    if (!m_SheelLink)
    {
        return _tstring();
    }

    HRESULT hr = S_OK;
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    hr = m_SheelLink->GetArguments(szBuf, _countof(szBuf));
    if (SUCCEEDED(hr))
    {
        return szBuf;
    }

    return _tstring();
}

bool CLnkHelper::SetArguments(const _tstring& strArgs)
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->SetArguments(strArgs.c_str());
    return SUCCEEDED(hr);
}

bool CLnkHelper::GetHotkey(BYTE& bModifier, BYTE& bKey) const
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    WORD wHotkey = 0;
    hr = m_SheelLink->GetHotkey(&wHotkey);
    if (SUCCEEDED(hr))
    {
        bModifier = HIBYTE(wHotkey);
        bKey = LOBYTE(wHotkey);
    }

    return SUCCEEDED(hr);
}

WORD CLnkHelper::GetHotkey() const
{
    if (!m_SheelLink)
    {
        return 0;
    }

    HRESULT hr = S_OK;
    WORD wHotkey = 0;
    hr = m_SheelLink->GetHotkey(&wHotkey);
    if (SUCCEEDED(hr))
    {
        return wHotkey;
    }

    return 0;
}

bool CLnkHelper::SetHotkey(BYTE bModifier, BYTE bKey)
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->SetHotkey(MAKEWORD(bKey, bModifier));
    return SUCCEEDED(hr);
}


bool CLnkHelper::SetHotkey(WORD wHotkey)
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->SetHotkey(wHotkey);
    return SUCCEEDED(hr);
}

bool CLnkHelper::GetShowCmd(int& iShowCmd) const
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->GetShowCmd(&iShowCmd);
    return SUCCEEDED(hr);
}

int CLnkHelper::CLnkHelper::GetShowCmd() const
{
    if (!m_SheelLink)
    {
        return 0;
    }

    HRESULT hr = S_OK;
    int iShowCmd = 0;
    hr = m_SheelLink->GetShowCmd(&iShowCmd);

    if (SUCCEEDED(hr))
    {
        return iShowCmd;
    }

    return 0;
}

bool CLnkHelper::SetShowCmd(int iShowCmd)
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->SetShowCmd(iShowCmd);
    return SUCCEEDED(hr);
}

bool CLnkHelper::GetIconLocation(_tstring& IconPath, int& iIcon) const
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    hr = m_SheelLink->GetIconLocation(szBuf, _countof(szBuf), &iIcon);
    if (SUCCEEDED(hr))
    {
        IconPath = szBuf;
    }

    return SUCCEEDED(hr);
}

_tstring CLnkHelper::GetIconLocation(int* piIcon/* = nullptr*/) const
{
    if (!m_SheelLink)
    {
        return _tstring();
    }

    HRESULT hr = S_OK;
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    int iIcon = 0;
    hr = m_SheelLink->GetIconLocation(szBuf, _countof(szBuf), &iIcon);
    if (SUCCEEDED(hr))
    {
        if (piIcon)
        {
            *piIcon = iIcon;
        }

        return szBuf;
    }

    return _tstring();
}

bool CLnkHelper::SetIconLocation(const _tstring& IconPath, int iIcon)
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->SetIconLocation(IconPath.c_str(), iIcon);
    return SUCCEEDED(hr);
}

bool CLnkHelper::GetRelativePath(_tstring& strPathRel) const
{
    if (!m_SheelLink)
    {
        return false;
    }

    WIN32_FIND_DATA wfd = { 0 };
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    HRESULT hr = S_OK;
    hr = m_SheelLink->GetPath(szBuf, _countof(szBuf), &wfd, SLGP_RELATIVEPRIORITY);
    if (SUCCEEDED(hr))
    {
        strPathRel = szBuf;
    }

    return SUCCEEDED(hr);
}

_tstring CLnkHelper::GetRelativePath() const
{
    if (!m_SheelLink)
    {
        return _tstring();
    }

    WIN32_FIND_DATA wfd = { 0 };
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    HRESULT hr = S_OK;
    hr = m_SheelLink->GetPath(szBuf, _countof(szBuf), &wfd, SLGP_RELATIVEPRIORITY);
    if (SUCCEEDED(hr))
    {
        return szBuf;
    }

    return _tstring();
}

bool CLnkHelper::SetRelativePath(const _tstring& strRelativePath)
{
    if (!m_SheelLink)
    {
        return false;
    }

    HRESULT hr = S_OK;
    hr = m_SheelLink->SetRelativePath(strRelativePath.c_str(), 0);
    return SUCCEEDED(hr);
}