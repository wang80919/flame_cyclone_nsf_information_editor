#include "CComFileDlg.h"
#include "CStrUtils.h"

_tstring CComFileDlg::GetCurrentModulePath()
{
    TCHAR szCurPath[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szCurPath, _countof(szCurPath));

    _tstring strResult = szCurPath;
    return strResult;
}

_tstring CComFileDlg::GetCurrentModuleDir()
{
    return GetFileDir(GetCurrentModulePath());
}

_tstring CComFileDlg::GetCurrentModuleName(bool bHasExt/* = true*/)
{
    return GetFileName(GetCurrentModulePath(), bHasExt);
}

_tstring CComFileDlg::GetFileName(const _tstring& strPath, bool bHasExt/* = true*/)
{
    _tstring strResult = strPath;
    size_t nIndex = strResult.find_last_of(_T('\\'));
    if (nIndex != _tstring::npos)
    {
        strResult = strResult.substr(nIndex + 1);
    }

    if (!bHasExt)
    {
        nIndex = strResult.find_last_of(_T('.'));
        if (nIndex != _tstring::npos)
        {
            strResult.resize(nIndex);
        }
    }

    return strResult;
}

_tstring CComFileDlg::GetFileDir(const _tstring& strPath)
{
    _tstring strResult;
    size_t nIndex = strPath.find_last_of(_T('\\'));
    if (nIndex != _tstring::npos)
    {
        strResult = strPath.substr(0, nIndex);
    }

    return strResult;
}

bool CComFileDlg::IsExist(const _tstring& strPath)
{
    return 0 == _taccess_s(strPath.c_str(), 00);
}

bool CComFileDlg::IsArchive(const _tstring& strPath)
{
    WIN32_FILE_ATTRIBUTE_DATA attr = { 0 };
    if (!::GetFileAttributesEx(strPath.c_str(), GetFileExInfoStandard, &attr))
    {
        return false;
    }

    return attr.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE;
}

bool CComFileDlg::IsDirectory(const _tstring& strPath)
{
    WIN32_FILE_ATTRIBUTE_DATA attr = { 0 };
    if (!::GetFileAttributesEx(strPath.c_str(), GetFileExInfoStandard, &attr))
    {
        return false;
    }

    return attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

bool CComFileDlg::GetSavePath(HWND hOwner, _tstring& strPath, UINT& nTypeIndex, const COMDLG_FILTERSPEC* pFilters, int filterCnt)
{
    IFileSaveDialog* pSaveFileDlg = nullptr;

    bool bComSuccess = false;
    HRESULT hr = S_OK;

    hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        return false;
    }
    bComSuccess = true;

    hr = ::CoCreateInstance(
        CLSID_FileSaveDialog,
        nullptr,
        CLSCTX_ALL,
        IID_IFileSaveDialog,
        (LPVOID*)&pSaveFileDlg
    );
    if (FAILED(hr))
    {
        goto L_Cleanup;
    }
    {
        pSaveFileDlg->SetOptions(
            FOS_OVERWRITEPROMPT |
            FOS_STRICTFILETYPES |
            FOS_DONTADDTORECENT |
            FOS_NOCHANGEDIR
        );

        if (pFilters)
        {
            pSaveFileDlg->SetFileTypes(filterCnt, pFilters);
        }

        do
        {
            if (0 != nTypeIndex)
            {
                pSaveFileDlg->SetFileTypeIndex(nTypeIndex);
            }

            _tstring strFileDir = strPath;
            if (strFileDir.empty())
            {
                strFileDir = GetCurrentModuleDir();
            }

            if (IsArchive(strFileDir))
            {
                strFileDir = GetFileDir(strFileDir);
            }

            if (!strFileDir.empty() && 0 == _taccess_s(strFileDir.c_str(), 00))
            {
                IShellItem* pItem = nullptr;

                hr = ::SHCreateItemFromParsingName(CStrUtils::TStrToWStr(strFileDir).c_str(), nullptr, IID_PPV_ARGS(&pItem));
                if (FAILED(hr))
                {
                    break;
                }

                pSaveFileDlg->SetFolder(pItem);
                pItem->Release();
            }

            hr = pSaveFileDlg->Show(hOwner);
            if (SUCCEEDED(hr))
            {
                pSaveFileDlg->GetFileTypeIndex(&nTypeIndex);

                IShellItem* pItemInfo = nullptr;
                hr = pSaveFileDlg->GetResult(&pItemInfo);
                if (pItemInfo)
                {
                    LPWSTR resultptr = nullptr;
                    pItemInfo->GetDisplayName(SIGDN_FILESYSPATH, &resultptr);
                    pItemInfo->Release();
                    if (resultptr)
                    {
                        strPath = CStrUtils::WStrToTStr(resultptr);
                        ::CoTaskMemFree(resultptr);
                    }
                }
            }
        } while (FALSE);
    }

L_Cleanup:
    if (bComSuccess)
    {
        ::CoUninitialize();
    }

    if (SUCCEEDED(hr))
    {
        return true;
    }

    return false;
}

bool CComFileDlg::GetSaveDir(HWND hOwner, _tstring& strDir)
{
    IFileDialog* pFileDlg = nullptr;

    bool bComSuccess = false;
    HRESULT hr = S_OK;

    hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        return false;
    }
    bComSuccess = true;

    hr = ::CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_ALL,
        IID_IFileDialog,
        (LPVOID*)&pFileDlg
    );
    if (FAILED(hr))
    {
        goto L_Cleanup;
    }
    {
        pFileDlg->SetOptions(
            FOS_PICKFOLDERS | FOS_NOCHANGEDIR
        );

        do
        {
            _tstring strFileDir = strDir;
            if (strFileDir.empty())
            {
                strFileDir = GetCurrentModuleDir();
            }

            if (IsDirectory(strFileDir))
            {
                IShellItem* pItem = nullptr;

                hr = ::SHCreateItemFromParsingName(CStrUtils::TStrToWStr(strFileDir).c_str(), nullptr, IID_PPV_ARGS(&pItem));
                if (FAILED(hr))
                {
                    break;
                }

                pFileDlg->SetFolder(pItem);
                pItem->Release();
            }

            hr = pFileDlg->Show(hOwner);
            if (SUCCEEDED(hr))
            {
                IShellItem* pItemInfo = nullptr;
                hr = pFileDlg->GetResult(&pItemInfo);
                if (pItemInfo)
                {
                    LPWSTR resultptr = nullptr;
                    pItemInfo->GetDisplayName(SIGDN_FILESYSPATH, &resultptr);
                    pItemInfo->Release();
                    if (resultptr)
                    {
                        strDir = CStrUtils::WStrToTStr(resultptr);
                        ::CoTaskMemFree(resultptr);
                    }
                }
            }
        } while (FALSE);
    }

L_Cleanup:
    if (bComSuccess)
    {
        ::CoUninitialize();
    }

    if (SUCCEEDED(hr))
    {
        return true;
    }

    return false;
}

bool CComFileDlg::GetOpenPath(HWND hOwner, _tstring& strPath, UINT& nTypeIndex, const COMDLG_FILTERSPEC* pFilters, int filterCnt)
{
    IFileOpenDialog* pOpenFileDlg = nullptr;

    bool bComSuccess = false;
    HRESULT hr = S_OK;

    hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        return false;
    }
    bComSuccess = true;

    hr = ::CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_ALL,
        IID_IFileOpenDialog,
        (LPVOID*)&pOpenFileDlg
    );
    if (FAILED(hr))
    {
        goto L_Cleanup;
    }
    {
        pOpenFileDlg->SetOptions(
            FOS_FILEMUSTEXIST |
            FOS_STRICTFILETYPES |
            FOS_DONTADDTORECENT |
            FOS_NOCHANGEDIR
        );

        if (pFilters)
        {
            pOpenFileDlg->SetFileTypes(filterCnt, pFilters);
        }

        do
        {
            if (0 != nTypeIndex)
            {
                pOpenFileDlg->SetFileTypeIndex(nTypeIndex);
            }

            _tstring strFileDir = strPath;
            if (strFileDir.empty())
            {
                strFileDir = GetCurrentModuleDir();
            }

            if (IsArchive(strFileDir))
            {
                strFileDir = GetFileDir(strFileDir);
            }

            if (IsDirectory(strFileDir))
            {
                IShellItem* pItem = nullptr;

                hr = ::SHCreateItemFromParsingName(CStrUtils::TStrToWStr(strFileDir).c_str(), nullptr, IID_PPV_ARGS(&pItem));
                if (FAILED(hr))
                {
                    break;
                }

                pOpenFileDlg->SetFolder(pItem);
                pItem->Release();
            }

            hr = pOpenFileDlg->Show(hOwner);
            if (SUCCEEDED(hr))
            {
                pOpenFileDlg->GetFileTypeIndex(&nTypeIndex);

                IShellItem* pItemInfo = nullptr;
                hr = pOpenFileDlg->GetResult(&pItemInfo);
                if (pItemInfo)
                {
                    LPWSTR resultptr = nullptr;
                    pItemInfo->GetDisplayName(SIGDN_FILESYSPATH, &resultptr);
                    pItemInfo->Release();
                    if (resultptr)
                    {
                        strPath = CStrUtils::WStrToTStr(resultptr);
                        ::CoTaskMemFree(resultptr);
                    }
                }
            }
        } while (FALSE);
    }

L_Cleanup:

    if (bComSuccess)
    {
        ::CoUninitialize();
    }

    if (SUCCEEDED(hr))
    {
        return true;
    }

    return false;
}

std::vector<_tstring> CComFileDlg::GetFileList(HWND hOwner, _tstring& strDir, const COMDLG_FILTERSPEC* pFilters, int filterCnt)
{
    std::vector<_tstring> vFileList;

    IFileOpenDialog* pOpenFileDlg = nullptr;

    bool bComSuccess = false;
    HRESULT hr = S_OK;

    hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        goto L_Cleanup;
    }
    bComSuccess = true;

    hr = ::CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_ALL,
        IID_IFileOpenDialog,
        (LPVOID*)&pOpenFileDlg
    );
    if (FAILED(hr))
    {
        goto L_Cleanup;
    }
    {
        pOpenFileDlg->SetOptions(
            FOS_ALLOWMULTISELECT |
            FOS_FILEMUSTEXIST
        );

        if (pFilters)
        {
            pOpenFileDlg->SetFileTypes(filterCnt, pFilters);
        }

        do
        {
            _tstring strFileDir = strDir;
            if (strFileDir.empty())
            {
                strFileDir = GetCurrentModuleDir();
            }

            if (IsArchive(strFileDir))
            {
                strFileDir = GetFileDir(strFileDir);
            }

            if (IsDirectory(strFileDir))
            {
                IShellItem* pItem = nullptr;

                hr = ::SHCreateItemFromParsingName(CStrUtils::TStrToWStr(strFileDir).c_str(), nullptr, IID_PPV_ARGS(&pItem));
                if (FAILED(hr))
                {
                    break;
                }

                pOpenFileDlg->SetFolder(pItem);
                pItem->Release();
            }

            hr = pOpenFileDlg->Show(hOwner);
            if (SUCCEEDED(hr))
            {
                IShellItemArray* pItemArray = nullptr;
                hr = pOpenFileDlg->GetResults(&pItemArray);

                DWORD dwCount = 0;
                pItemArray->GetCount(&dwCount);

                for (DWORD i = 0; i < dwCount; i++)
                {
                    IShellItem* pItem = nullptr;
                    pItemArray->GetItemAt(i, &pItem);
                    if (pItem)
                    {
                        LPWSTR resultptr = nullptr;
                        pItem->GetDisplayName(SIGDN_FILESYSPATH, &resultptr);

                        //记住打开的路径
                        if (0 == i && nullptr != resultptr)
                        {
                            strDir = GetFileDir(CStrUtils::WStrToTStr(resultptr));
                        }

                        pItem->Release();
                        if (resultptr)
                        {
                            vFileList.push_back(CStrUtils::WStrToTStr(resultptr));
                            CoTaskMemFree(resultptr);
                        }
                    }
                }

                if (pItemArray)
                {
                    pItemArray->Release();
                }

            }
        } while (FALSE);
    }

L_Cleanup:

    if (bComSuccess)
    {
        ::CoUninitialize();
    }

    return vFileList;
}
