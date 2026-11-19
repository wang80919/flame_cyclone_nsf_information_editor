#include "CDialogBase.h"
#include <Windows.h>
#include <strsafe.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

BEGIN_DLG_BASE_MESSAGE_MAP(CDialogBase)
    ON_DLG_MESSAGE(WM_CLOSE, &CDialogBase::OnClose)

    ON_DLG_MESSAGE(0/*消息*/, &CDialogBase::OnMsgTest)
    ON_DLG_COMMAND(0/*控件ID*/, &CDialogBase::OnCommandTest)
    ON_DLG_NOTIFY(0/*通知码*/, 0/*控件ID*/, &CDialogBase::OnNotifyTest)
    ON_DLG_COMMAND_RANGE(0/*控件ID起始*/, 0/*控件ID结束*/, &CDialogBase::OnCommandTest)
    ON_DLG_NOTIFY_RANGE(0/*通知码*/, 0/*控件ID起始*/, 0/*控件ID结束*/, &CDialogBase::OnNotifyTest)
END_DLG_BASE_MESSAGE_MAP()

CDialogBase::CDialogBase()
    :
    m_hWnd(NULL),
    m_resID(0),
    m_bModel(FALSE),
    m_bFakeModel(FALSE),
    m_bCenter(FALSE),
    m_bShow(TRUE)
{

}

CDialogBase::~CDialogBase()
{
    if (NULL != m_hWnd)
    {
        ::DestroyWindow(m_hWnd);
    }
}

HWND CDialogBase::DoDialog(UINT resourceID, HWND hWndParent, BOOL isShow, bool bCenter/* = true*/)
{
    m_bCenter = bCenter;
    m_bModel = FALSE;
    m_resID = resourceID;
    m_bShow = isShow;
    HWND hWnd = ::CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceID), hWndParent, DialogProc, (LPARAM)this);
    m_hWnd = hWnd;

    if (m_bShow)
    {
        ::ShowWindow(m_hWnd, SW_SHOW);
    }

    return m_hWnd;
}

INT_PTR CDialogBase::DoModalEx(UINT resourceID, HWND hWndParent, BOOL isShow, bool bCenter/* = true*/)
{
    m_bCenter = bCenter;
    m_bModel = FALSE;
    return DoFakeModal(resourceID, hWndParent, isShow);
}

INT_PTR CDialogBase::DoModal(HWND hWndParent, bool bCenter/* = true*/)
{
    m_bCenter = bCenter;
    m_bModel = TRUE;
    return ::DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(m_resID), hWndParent, DialogProc, (LPARAM)this);
}

INT_PTR CDialogBase::DoModal(UINT resourceID, HWND hWndParent, bool bCenter/* = true*/)
{
    m_bCenter = bCenter;
    m_resID = resourceID;
    m_bModel = TRUE;
    return ::DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(m_resID), hWndParent, DialogProc, (LPARAM)this);
}

BOOL CDialogBase::DialogMessage(LPMSG pMsg)
{
    return ::IsDialogMessage(m_hWnd, pMsg);
}

BOOL CDialogBase::PreTranslateMessage(LPMSG pMsg)
{
    return FALSE;
}

bool CDialogBase::EndDialog(INT_PTR nResult)
{
    if (NULL == m_hWnd)
    {
        return false;
    }

    if (m_bFakeModel)
    {
        //启用父窗口
        HWND hParent = ::GetParent(m_hWnd);
        if (hParent)
        {
            ::EnableWindow(hParent, TRUE);
        }

        // 投递 WM_QUIT 消息退出消息环 (窗口句柄指定为NULL时, 
        // 该函数的行为类似于对 PostThreadMessage 的调用， 
        // 其中 dwThreadId 参数设置为当前线程的标识符。)
        ::PostMessage(NULL, WM_QUIT, nResult, 0);
    }

    if (m_bModel)
    {
        ::EndDialog(m_hWnd, nResult);
        m_hWnd = NULL;
    }
    else
    {
        ::DestroyWindow(m_hWnd);
        m_hWnd = NULL;
    }

    return TRUE;
}

LRESULT CDialogBase::OnClose(WPARAM wParam, LPARAM lParam)
{
    return EndDialog(wParam);
}

LRESULT CDialogBase::DoFakeModal(UINT resourceID, HWND hWndParent, BOOL isShow)
{
    m_bModel = FALSE;
    m_resID = resourceID;
    m_bFakeModel = TRUE;

    m_hWnd = ::CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceID), hWndParent, DialogProc, (LPARAM)this);
    if (NULL == m_hWnd)
    {
        return -1;
    }

    if (isShow)
    {
        ::ShowWindow(m_hWnd, SW_SHOW);
    }

    //禁用父窗口
    HWND hParent = GetParent(m_hWnd);
    if (hParent)
    {
        ::EnableWindow(hParent, FALSE);
    }

    MSG msg = { 0 };
    BOOL bRet = FALSE;

    //如果 hWnd 为 NULL， 则 GetMessage 将检索属于当前线程的任何窗口的消息，以及当前线程的消息队列上 hwnd 值为 NULL 的任何消息，
    //因此，如果 hWnd 为 NULL，则同时处理窗口消息和线程消息。
    //如果 hWnd 为 - 1，则 GetMessage 仅检索当前线程的消息队列中 hwnd 值为 NULL 的消息，
    //即当 hWnd 参数为 NULL 或 PostThreadMessage 时，PostMessage 发布的线程消息。

    while (0 != (bRet = GetMessage(&msg, NULL, 0, 0)))
    {
        if (-1 == bRet)
        {
            break;
        }

        if (::TranslateAccelerator(m_hWnd, m_hAccel, &msg))
        {
            continue;
        }

        if (PreTranslateMessage(&msg))
        {
            continue;
        }

        //类捕获消息处理与对话框 TAB 按键处理
        if (DialogMessage(&msg))
        {
            continue;
        }

        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    return msg.wParam;
}

INT_PTR WINAPI CDialogBase::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDialogBase* pThis = reinterpret_cast<CDialogBase*>(GetProp(hWnd, _T("this")));

    if (pThis)
    {
        return pThis->DialogProc(uMsg, wParam, lParam);
    }

    if (WM_INITDIALOG == uMsg)
    {
        pThis = reinterpret_cast<CDialogBase*>(lParam);
        pThis->m_hWnd = hWnd;
        SetProp(hWnd, _T("this"), pThis);
        pThis->DialogProc(uMsg, wParam, lParam);

        if (pThis->m_bCenter)
        {
            pThis->MoveToCenter(::GetParent(hWnd));
        }

        return (INT_PTR)TRUE;
    }

    return (INT_PTR)FALSE;
}

INT_PTR CDialogBase::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const DLG_MSGMAP* pEntry = GetMessageMap();
    LRESULT lResult = NULL;

    while (pEntry)
    {
        const DLG_MSGMAP_ENTRY* lpEntries = pEntry->lpEntries;
        while (lpEntries->m_pFunc.m_pFnMessage)
        {
            if (uMsg == lpEntries->m_nMessage)
            {
                //处理 WM_COMMAND 消息
                if (WM_COMMAND == uMsg)
                {
                    //消息源   HIWORD(wParam)      LOWORD(wParam)     lParam 
                    //菜单     0                   菜单ID             0
                    //快捷键   1                   快捷键ID           0
                    //控件     控件定义的通知码    控件ID             控件窗口句柄
                    WORD wNotify = HIWORD(wParam);
                    WORD wID = LOWORD(wParam);
                    HWND hWnd = (HWND)lParam;

                    if (0 != lpEntries->m_nCode)
                    {
                        //通知码检查
                        if (wNotify == lpEntries->m_nCode && wID == lpEntries->m_nID)
                        {
                            lResult = this->OnCommand(wNotify, wID, hWnd, lpEntries->m_nID, lpEntries->m_nLastID);
                            if (lResult)
                            {
                                break;
                            }

                            lResult = (this->*lpEntries->m_pFunc.m_pFnCommand)(wNotify, wID, hWnd, wID, wID);
                            break;
                        }
                    }
                    else
                    {
                        //ID范围检测
                        if ((wID >= lpEntries->m_nID && wID <= lpEntries->m_nLastID))
                        {
                            lResult = this->OnCommand(wNotify, wID, hWnd, lpEntries->m_nID, lpEntries->m_nLastID);
                            if (lResult)
                            {
                                break;
                            }

                            lResult = (this->*lpEntries->m_pFunc.m_pFnCommand)(wNotify, wID, hWnd, lpEntries->m_nID, lpEntries->m_nLastID);
                            break;
                        }
                    }
                }
                else if (WM_NOTIFY == uMsg)
                {
                    WORD wID = (WORD)wParam;           //发送消息的公共控件的标识符。 不保证此标识符是唯一的。
                    LPNMHDR pNmhdr = (LPNMHDR)lParam;  //指向包含通知代码和其他信息的 NMHDR 结构的指针

                    lResult = this->OnNotify(wID, pNmhdr);
                    if (lResult)
                    {
                        break;
                    }

                    if (0 != lpEntries->m_nCode)
                    {
                        if (pNmhdr->code == lpEntries->m_nCode && wID == lpEntries->m_nID)
                        {
                            lResult = (this->*lpEntries->m_pFunc.m_pFnNotify)(wID, pNmhdr);
                            break;
                        }
                    }
                    else
                    {
                        if (pNmhdr->idFrom >= lpEntries->m_nID && pNmhdr->idFrom <= lpEntries->m_nLastID)
                        {
                            lResult = (this->*lpEntries->m_pFunc.m_pFnNotify)(wID, pNmhdr);
                            break;
                        }
                    }

                }
                else
                {
                    lResult = (this->*lpEntries->m_pFunc.m_pFnMessage)(wParam, lParam);
                    break;
                }
            }

            lpEntries++;
        }

        //子类处理了则返回
        if (lResult)
        {
            ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, lResult);
            return lResult;
        }

        //获取基类的消息映射
        pEntry = pEntry->pfnGetBaseMap();
    }

    return FALSE;
}

void CDialogBase::SetResourceID(UINT resourceID)
{
    m_resID = resourceID;
}

void CDialogBase::ShowWindow(int cmdShow)
{
    ::ShowWindow(m_hWnd, cmdShow);
}

bool CDialogBase::IsVisible() const
{
    return ::IsWindowVisible(m_hWnd);
}

SIZE CDialogBase::GetWindowCenterPos(const LPRECT pRtChild, HWND hParent)
{
    RECT rectParent = { 0 };
    RECT rectWork = { 0 };
    LONG dwParentW = 0;
    LONG dwParentH = 0;
    LONG dwChildW = 0;
    LONG dwChildH = 0;
    LONG dwChildX = 0;
    LONG dwChildY = 0;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWork, 0);

    if (NULL == hParent)
    {
        hParent = ::GetDesktopWindow();
    }

    ::GetWindowRect(hParent, &rectParent);

    dwParentW = rectParent.right - rectParent.left;
    dwParentH = rectParent.bottom - rectParent.top;
    dwChildW = pRtChild->right - pRtChild->left;
    dwChildH = pRtChild->bottom - pRtChild->top;
    dwChildX = rectParent.left + (dwParentW - dwChildW) / 2;
    dwChildY = rectParent.top + (dwParentH - dwChildH) / 2;

    if (dwChildX < 0)
    {
        dwChildX = 0;
    }

    if (dwChildY < 0)
    {
        dwChildY = 0;
    }

    if ((dwChildX + dwChildW) > rectWork.right)
    {
        dwChildX = rectWork.right - dwChildW;
    }

    if ((dwChildY + dwChildH) > rectWork.bottom)
    {
        dwChildY = rectWork.bottom - dwChildH;
    }

    return { dwChildX, dwChildY };
}

void CDialogBase::MoveWindowToCenter(HWND hWnd, HWND hParent)
{
    RECT rectParent = { 0 };
    RECT rectChild = { 0 };
    RECT rectWork = { 0 };
    LONG dwParentW = 0;
    LONG dwParentH = 0;
    LONG dwChildW = 0;
    LONG dwChildH = 0;
    LONG dwChildX = 0;
    LONG dwChildY = 0;
    ::GetWindowRect(hWnd, &rectChild);

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWork, 0);

    if (NULL == hParent)
    {
        hParent = ::GetDesktopWindow();
    }

    ::GetWindowRect(hParent, &rectParent);

    dwParentW = rectParent.right - rectParent.left;
    dwParentH = rectParent.bottom - rectParent.top;
    dwChildW = rectChild.right - rectChild.left;
    dwChildH = rectChild.bottom - rectChild.top;
    dwChildX = rectParent.left + (dwParentW - dwChildW) / 2;
    dwChildY = rectParent.top + (dwParentH - dwChildH) / 2;

    if (dwChildX < 0)
    {
        dwChildX = 0;
    }

    if (dwChildY < 0)
    {
        dwChildY = 0;
    }

    if ((dwChildX + dwChildW) > rectWork.right)
    {
        dwChildX = rectWork.right - dwChildW;
    }

    if ((dwChildY + dwChildH) > rectWork.bottom)
    {
        dwChildY = rectWork.bottom - dwChildH;
    }

    ::MoveWindow(
        hWnd, 
        dwChildX,
        dwChildY,
        dwChildW, 
        dwChildH, 
        TRUE
    );
}

void CDialogBase::MoveToCenter(HWND hParent)
{
    MoveWindowToCenter(m_hWnd, hParent);
}

_tstring CDialogBase::LoadString(UINT uID)
{
    _tstring strResult;

#ifdef _UNICODE

    do
    {
        //获取资源长度
        LPCTSTR lpCStr = nullptr;
        int nLength = ::LoadString(GetModuleHandle(NULL), uID, (LPTSTR)&lpCStr, 0);
        if (nLength <= 0 || nullptr == lpCStr)
        {
            break;
        }

        //分配字符串缓冲
        strResult.resize(nLength + 1);
        ::StringCchCopy(&strResult[0], strResult.size(), lpCStr);
        strResult.resize(nLength);

    } while (false);

#else

    //获取资源长度
    TCHAR szBuf[MAX_PATH * 4] = { 0 };
    int nLength = ::LoadString(GetModuleHandle(NULL), uID, szBuf, _countof(szBuf));
    strResult = szBuf;

#endif

    return strResult;
}

int CDialogBase::GetItemStringLength(HWND hWnd)
{
    return GetWindowTextLength(hWnd);
}

int CDialogBase::GetItemStringLength(DWORD dwID)
{
    return GetWindowTextLength(GetDlgItem(m_hWnd, dwID));
}

_tstring CDialogBase::GetItemString(HWND hWnd)
{
    _tstring strResult;
    LPTSTR lpData = nullptr;

    do
    {
        DWORD dwLength = GetWindowTextLength(hWnd);
        if (0 == dwLength)
        {
            break;
        }

        dwLength += 1;
        lpData = (LPTSTR)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
        if (NULL == lpData)
        {
            break;
        }

        ::SendMessage(hWnd, WM_GETTEXT, (WPARAM)dwLength, (LPARAM)lpData);
        strResult = lpData;

    } while (false);


    if (NULL != lpData)
    {
        ::HeapFree(::GetProcessHeap(), 0, lpData);
    }

    return strResult;
}

_tstring CDialogBase::GetItemString(DWORD dwID)
{
    return GetItemString(GetDlgItem(m_hWnd, dwID));
}

bool CDialogBase::IsButtonChecked(int nIDButton) const
{
    return BST_CHECKED == ::IsDlgButtonChecked(m_hWnd, nIDButton);
}

bool CDialogBase::CheckButton(int nIDButton, bool bCheck/* = true*/)
{
    return ::CheckDlgButton(m_hWnd, nIDButton, bCheck ? BST_CHECKED : BST_UNCHECKED);
}

bool CDialogBase::CheckRadioButton(int nIDFirst, int nIDLast, int nIDCheckButton)
{
    return ::CheckRadioButton(m_hWnd, nIDFirst, nIDLast, nIDCheckButton);
}

HWND CDialogBase::GetItem(DWORD dwID) const
{
    return ::GetDlgItem(m_hWnd, dwID);
}

bool CDialogBase::EnableItem(DWORD dwID, bool fEnable/* = true*/) const
{
    return ::EnableWindow(GetDlgItem(m_hWnd, dwID), fEnable);
}

RECT CDialogBase::GetItemRectOnClient(WORD wID) const
{
    RECT rtItem = { 0 };
    ::GetWindowRect(GetItem(wID), &rtItem);

    POINT ptClient = { rtItem.left, rtItem.top };
    ::ScreenToClient(m_hWnd, &ptClient);

    RECT rtResult = { 0 };
    rtResult.left = ptClient.x;
    rtResult.top = ptClient.y;
    rtResult.right = ptClient.x + (rtItem.right - rtItem.left);
    rtResult.bottom = ptClient.y + (rtItem.bottom - rtItem.top);

    return rtResult;
}

bool CDialogBase::IsMenuItemChecked(HMENU hMenu, UINT uId, bool fByPosition/* = false*/)
{
    MENUITEMINFO mii = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    ::GetMenuItemInfo(hMenu, uId, fByPosition, &mii);

    return mii.fState & MF_CHECKED;
}

void CDialogBase::CheckMenuItem(HMENU hMenu, UINT uId, bool bCheck, bool fByPosition/* = false*/)
{
    MENUITEMINFO mii = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    ::GetMenuItemInfo(hMenu, uId, fByPosition, &mii);

    if (bCheck)
    {
        mii.fState |= MF_CHECKED;
    }
    else
    {

        mii.fState &= ~MF_CHECKED;
    }

    ::SetMenuItemInfo(hMenu, uId, fByPosition, &mii);
}

bool CDialogBase::IsMenuItemEnable(HMENU hMenu, UINT uId, bool fByPosition/* = false*/)
{
    MENUITEMINFO mii = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    ::GetMenuItemInfo(hMenu, uId, fByPosition, &mii);

    return (mii.fState & MFS_DISABLED) ? false : true;
}

void CDialogBase::EnableMenuItem(HMENU hMenu, UINT uId, bool bCheck, bool fByPosition/* = false*/)
{
    MENUITEMINFO mii = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    ::GetMenuItemInfo(hMenu, uId, fByPosition, &mii);

    if (bCheck)
    {
        mii.fState &= ~MFS_DISABLED;
    }
    else
    {

        mii.fState |= MFS_DISABLED;
    }

    ::SetMenuItemInfo(hMenu, uId, fByPosition, &mii);
}

_tstring CDialogBase::GetMenuItemString(HMENU hMenu, UINT uId, bool bCheck, bool fByPosition/* = false*/)
{
    MENUITEMINFO mii = { 0 };
    TCHAR szBuf[MAX_PATH] = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_DATA;
    mii.dwTypeData = szBuf;
    mii.cch = _countof(szBuf);
    ::GetMenuItemInfo(hMenu, uId, fByPosition, &mii);

    if (nullptr == mii.dwTypeData)
    {
        return _tstring();
    }

    return mii.dwTypeData;
}

bool CDialogBase::LoadAccel(DWORD dwId)
{
    m_hAccel = ::LoadAccelerators(GetModuleHandle(0), MAKEINTRESOURCE(dwId));
    return  nullptr != m_hAccel;
}

bool CDialogBase::SetMenuItemString(HMENU hMenu, UINT uId, const _tstring strText, bool fByPosition/* = false*/)
{
    MENUITEMINFO mii = { 0 };
    TCHAR szBuf[MAX_PATH] = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_DATA;
    mii.dwTypeData = szBuf;
    mii.cch = _countof(szBuf);
    ::GetMenuItemInfo(hMenu, uId, fByPosition, &mii);
    ::StringCchPrintf(szBuf, _countof(szBuf), strText.c_str());
    return ::SetMenuItemInfo(hMenu, uId, fByPosition, &mii);
}

void CDialogBase::SetItemString(HWND hWnd, const _tstring& strText)
{
    ::SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)strText.c_str());
}

void CDialogBase::SetItemString(DWORD dwID, const _tstring& strText)
{
    SetItemString(GetDlgItem(m_hWnd, dwID), strText);
    //::SetDlgItemText(m_hWnd, dwID, strText.c_str());
}

int CDialogBase::GetItemInt(DWORD dwID, bool bSigned/* = true*/)
{
    return ::GetDlgItemInt(m_hWnd, dwID, nullptr, bSigned);
}

void CDialogBase::SetItemInt(DWORD dwID, int val, bool bSigned/* = true*/)
{
    ::SetDlgItemInt(m_hWnd, dwID, val, bSigned);
}

double CDialogBase::GetItemFloat(DWORD dwID, bool bSigned/* = true*/)
{
    double result = 0.0f;
    LPTSTR lpData = nullptr;

    do
    {
        DWORD dwLength = GetWindowTextLength(GetDlgItem(m_hWnd, dwID));
        if (0 == dwLength)
        {
            break;
        }

        dwLength += 1;
        lpData = (LPTSTR)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength * sizeof(TCHAR));
        if (NULL == lpData)
        {
            break;
        }

        ::GetDlgItemText(m_hWnd, dwID, lpData, dwLength);
        result = _tcstod(lpData, nullptr);

    } while (false);


    if (NULL != lpData)
    {
        ::HeapFree(::GetProcessHeap(), 0, lpData);
    }

    return result;
}

void CDialogBase::SetItemFloat(DWORD dwID, double val, bool bSigned/* = true*/)
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    StringCchPrintf(szBuf, _countof(szBuf), _T("%g"), val);
    ::SetDlgItemText(m_hWnd, dwID, szBuf);
}

VOID CDialogBase::ModifyMenuText(HMENU hMnu, UINT uPosition, LPCTSTR lpNewItem, BOOL fByPos)
{
    MENUITEMINFO mii = { 0 };
    TCHAR szBuf[MAX_PATH] = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_DATA;
    mii.fType = MFT_STRING;
    mii.dwTypeData = szBuf;
    mii.cch = _countof(szBuf);
    ::GetMenuItemInfo(hMnu, uPosition, fByPos, &mii);
    ::StringCchCopy(szBuf, _countof(szBuf), lpNewItem);
    ::SetMenuItemInfo(hMnu, uPosition, fByPos, &mii);
}

VOID CDialogBase::ModifyMenuText(HMENU hMnu, UINT uPosition, UINT resourceID, BOOL fByPos)
{
    ModifyMenuText(hMnu, uPosition, LoadString(resourceID).c_str(), fByPos);
}

VOID CDialogBase::ModifyMenuState(HMENU hMnu, UINT uPosition, UINT fState/* = MFS_ENABLED*/, BOOL fByPos/* = TRUE*/)
{
    MENUITEMINFO mii = { 0 };
    TCHAR szBuf[MAX_PATH] = { 0 };
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_DATA;
    mii.fType = MFT_STRING;
    mii.dwTypeData = szBuf;
    mii.cch = _countof(szBuf);
    ::GetMenuItemInfo(hMnu, uPosition, fByPos, &mii);
    mii.fState = fState;
    ::SetMenuItemInfo(hMnu, uPosition, fByPos, &mii);
}

HMENU CDialogBase::FindParentMenu(HMENU hMenu, UINT itemID)
{
    int count = ::GetMenuItemCount(hMenu);
    for (int i = 0; i < count; i++) {
        UINT id = ::GetMenuItemID(hMenu, i);
        if (id == itemID)
        {
            return hMenu;
        }
        else if (id == -1)
        {
            HMENU hSubMenu = ::GetSubMenu(hMenu, i);
            HMENU hParent = FindParentMenu(hSubMenu, itemID);
            if (hParent)
            {
                return hParent;
            }
        }
    }

    return NULL;
}

void CDialogBase::CheckRadioMenuItem(HMENU hMenu, UINT dwID, bool fEnable/* = true*/)
{
    HMENU hParentMenu = FindParentMenu(hMenu, dwID);
    int nCount = ::GetMenuItemCount(hParentMenu);
    for (int i = 0; i < nCount; i++)
    {
        if (dwID == ::GetMenuItemID(hParentMenu, i))
        {
            CheckMenuItem(hParentMenu, i, true, true);
        }
        else
        {
            CheckMenuItem(hParentMenu, i, false, true);
        }
    }
}

HWND CDialogBase::GetWndHandle() const
{
    if (::IsWindow(m_hWnd))
    {
        return m_hWnd;
    }

    return NULL;
}

bool CDialogBase::PostMessage(UINT uMsg, WPARAM wParam/* = 0*/, LPARAM lParam)
{
    return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

LRESULT CDialogBase::SendMessage(UINT uMsg, WPARAM wParam/* = 0*/, LPARAM lParam)
{
    return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
}

bool CDialogBase::PostCommand(WORD wID, WORD wNotify/* = 0*/, LPARAM lParam/* = 0*/)
{
    return ::PostMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(wID, wNotify), lParam);
}

LRESULT CDialogBase::SendCommand(WORD wID, WORD wNotify/* = 0*/, LPARAM lParam/* = 0*/)
{
    return ::SendMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(wID, wNotify), lParam);
}

bool CDialogBase::PostItemMessage(DWORD dwID, UINT uMsg, WPARAM wParam/* = 0*/, LPARAM lParam/* = 0*/)
{
    return ::PostMessage(GetDlgItem(m_hWnd, dwID), uMsg, wParam, lParam);
}

LRESULT CDialogBase::SendItemMessage(DWORD dwID, UINT uMsg, WPARAM wParam/* = 0*/, LPARAM lParam/* = 0*/)
{
    return ::SendMessage(GetDlgItem(m_hWnd, dwID), uMsg, wParam, lParam);
}

LRESULT CDialogBase::OnMsgTest(WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

LRESULT CDialogBase::OnCommandTest(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    return FALSE;
}

LRESULT CDialogBase::OnNotifyTest(WORD wID, LPNMHDR pNmhdr)
{
    return FALSE;
}

//命令消息处理
LRESULT CDialogBase::OnCommand(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    return (LRESULT)FALSE;
}

//通知消息处理
LRESULT CDialogBase::OnNotify(WORD wID, LPNMHDR pNmhdr)
{
    return (LRESULT)FALSE;
}

bool CDialogBase::BuildDcBitmap(HDC hdc, HDC* pDC, HBITMAP* pBitmap, int nW, int nH, DWORD dwColor/* = RGB(128, 128, 128)*/)
{
    *pDC = ::CreateCompatibleDC(hdc);
    if (!*pDC)
    {
        return false;
    }

    *pBitmap = ::CreateCompatibleBitmap(hdc, nW, nH);
    if (!*pBitmap)
    {
        return false;
    }

    ::SelectObject(*pDC, *pBitmap);

    HBRUSH hBkgBrush = ::CreateSolidBrush(dwColor);

    RECT rect = { 0 };
    ::GetClientRect(m_hWnd, &rect);
    ::FillRect(*pDC, &rect, hBkgBrush);
    ::DeleteObject(hBkgBrush);

    return true;
}

void CDialogBase::DeleteDcBitmap(HDC* pDC, HBITMAP* pBitmap)
{
    if (*pBitmap)
    {
        ::DeleteObject(*pBitmap);
        *pBitmap = NULL;
    }

    if (*pDC)
    {
        ::DeleteObject(*pDC);
        *pDC = NULL;
    }
}
