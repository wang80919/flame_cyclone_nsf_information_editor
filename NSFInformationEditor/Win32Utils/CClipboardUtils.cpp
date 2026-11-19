#include "CClipboardUtils.h"
#include <windows.h>
#include <strsafe.h>

bool CClipboardUtils::SetText(const _tstring& strText)
{
    HANDLE hGlobalMem = nullptr;
    bool bReturnResult = false;

    // 打开粘贴板
    if (!::OpenClipboard(nullptr))
    {
        return false;
    }

    do
    {
        // 分配全局内存
        SIZE_T nCbSize = (strText.size() + 1) * sizeof(TCHAR);
        hGlobalMem = ::GlobalAlloc(GMEM_MOVEABLE, nCbSize);
        if (nullptr == hGlobalMem)
        {
            break;
        }

        // 锁定全局内存
        LPTSTR pData = (LPTSTR)::GlobalLock(hGlobalMem);
        if (nullptr == pData)
        {
            break;
        }

        // 操作全局内存数据
        ::CopyMemory(pData, strText.c_str(), nCbSize);

        // 解锁全局内存
        (void)::GlobalUnlock(hGlobalMem);

        // 清空粘贴板
        if (!::EmptyClipboard())
        {
            break;
        }

        // 设置粘贴板文本数据
        if (nullptr != ::SetClipboardData(CF_TEXT, hGlobalMem))
        {
            bReturnResult = true;
        }

    } while (false);

    if (nullptr != hGlobalMem)
    {
        // 释放全局内存
        ::GlobalFree(hGlobalMem);
    }

    // 关闭粘贴板
    ::CloseClipboard();

    return bReturnResult;
}

bool CClipboardUtils::GetText(_tstring& strText)
{
    bool bReturnResult = false;

    // 打开粘贴板
    if (!::OpenClipboard(nullptr))
    {
        return false;
    }

    do
    {
        // 获取粘贴板数据
        HANDLE hGlobalMem = ::GetClipboardData(CF_TEXT);
        if (nullptr == hGlobalMem)
        {
            break;
        }

        LPTSTR pData = (LPTSTR)::GlobalLock(hGlobalMem);
        if (nullptr == pData)
        {
            break;
        }

        // 操作全局内存数据
        strText = pData;
        bReturnResult = true;

        // 解锁全局内存
        (void)::GlobalUnlock(hGlobalMem);

    } while (false);

    // 关闭粘贴板
    ::CloseClipboard();

    return bReturnResult;
}

bool CClipboardUtils::Empty()
{
    bool bOpenResult = false;

    // 打开粘贴板
    if (!::OpenClipboard(nullptr))
    {
        return false;
    }

    // 清空粘贴板
    if (::EmptyClipboard())
    {
        bOpenResult = true;
    }

    // 关闭粘贴板
    ::CloseClipboard();

    return bOpenResult;
}