#include "CDialogFrame.h"
#include <Windows.h>
#include <strsafe.h>
#include <imm.h>
#include <thread>
#include "resource.h"
#include "Win32Utils/CStrUtils.h"
#include "Win32Utils/CComFileDlg.h"
#include "Win32Utils/CPathUtils.h"
#include "Win32Utils/CMessageBoxCenter.h"
#include "Win32Utils/CVersionUtils.h"

#pragma comment(lib ,"imm32.lib")
#pragma comment(lib, "SetupAPI.lib")
#pragma comment(lib, "Bthprops.lib")

#define WM_TRAYICON                 (WM_USER + 10)

BEGIN_DLG_MESSAGE_MAP(CDialogFrame, CDialogBase)
    ON_DLG_MESSAGE(WM_INITDIALOG, &CDialogFrame::OnInitDialog)
    ON_DLG_MESSAGE(WM_CLOSE, &CDialogFrame::OnClose)
    ON_DLG_MESSAGE(WM_WINDOWPOSCHANGING, &CDialogFrame::OnWindowPosChinging)
    ON_DLG_MESSAGE(WM_RBUTTONUP, &CDialogFrame::OnRButtonUp)
    ON_DLG_MESSAGE(WM_TRAYICON, &CDialogFrame::OnTrayIcon)
    ON_DLG_MESSAGE(WM_DROPFILES, &CDialogFrame::OnDropFiles)
    ON_DLG_COMMAND(IDC_BUTTON_OPEN_FILE, &CDialogFrame::OnFileOpen)
    ON_DLG_COMMAND(ID_FILE_OPEN, &CDialogFrame::OnFileOpen)
    ON_DLG_COMMAND(ID_FILE_SAVE, &CDialogFrame::OnFileSave)
    ON_DLG_COMMAND(ID_FILE_SAVE_AS, &CDialogFrame::OnFileSaveAs)
    ON_DLG_COMMAND(ID_FILE_EXIT, &CDialogFrame::OnFileExit)
    ON_DLG_COMMAND(ID_SETTINGS_TOPMOST, &CDialogFrame::OnSettingsTopmost)
    ON_DLG_COMMAND(ID_HELP_ABOUT, &CDialogFrame::OnHelpAbout)
    ON_DLG_COMMAND(ID_LANGUAGE_ZH_CN, &CDialogFrame::OnSettingsLang_zh_CN)
    ON_DLG_COMMAND(ID_LANGUAGE_ZH_SG, &CDialogFrame::OnSettingsLang_zh_SG)
    ON_DLG_COMMAND(ID_LANGUAGE_ZH_HK, &CDialogFrame::OnSettingsLang_zh_HK)
    ON_DLG_COMMAND(ID_LANGUAGE_ZH_TW, &CDialogFrame::OnSettingsLang_zh_TW)
    ON_DLG_COMMAND(ID_LANGUAGE_ZH_MO, &CDialogFrame::OnSettingsLang_zh_MO)
    ON_DLG_COMMAND(ID_LANGUAGE_JA_JP, &CDialogFrame::OnSettingsLang_ja_JP)
    ON_DLG_COMMAND(ID_LANGUAGE_KO_KR, &CDialogFrame::OnSettingsLang_ko_KR)
    ON_DLG_COMMAND(ID_LANGUAGE_EN_US, &CDialogFrame::OnSettingsLang_en_US)
    ON_DLG_COMMAND_RANGE(IDC_RADIO_STANDARD_NTSC, IDC_RADIO_STANDARD_NTSC_PAL, &CDialogFrame::OnRadioStandard)
END_DLG_MESSAGE_MAP()

LRESULT CDialogFrame::OnInitDialog(WPARAM wParam, LPARAM lParam)
{
    m_hHideWnd = CreateWindow(
        _T("Static"), _T("Hide"), WS_CHILDWINDOW, 0, 0, 0, 0,
        m_hWnd, NULL, GetModuleHandle(NULL), NULL);
    if (m_hHideWnd)
    {
        ::SetParent(m_hHideWnd, NULL);
    }

    m_hMainMenu = ::GetMenu(m_hWnd);
    if (m_hMainMenu)
    {
        ShowTrayIcon();
    }

    CHANGEFILTERSTRUCT cfs = { 0 };
    cfs.cbSize = sizeof(cfs);

    //允许低权限进程拖动文件进来
    ChangeWindowMessageFilterEx(m_hWnd, WM_DROPFILES, MSGFLT_ALLOW, &cfs);
    ChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA, MSGFLT_ALLOW, &cfs);
    ChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA - 1, MSGFLT_ALLOW, &cfs);

    ::SetMenu(m_hWnd, m_hMainMenu);

    HICON hIconSmall = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_MAIN));
    HICON hIconBig = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_MAIN));
    if (hIconBig)
    {
        ::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
    }

    if (hIconSmall)
    {
        ::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
    }

    // 禁用输入法
    ImmAssociateContext(GetItem(IDC_EDIT_TOTAL_SONGS), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_START_SONG), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_LOAD_ADDR), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_INIT_ADDR), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_PLAY_ADDR), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_NAME), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_ARTIST), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_COPYRIGHT), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_PLAY_SPEED_NTSC), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_PLAY_SPEED_PAL), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_BANK_0), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_BANK_1), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_BANK_2), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_BANK_3), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_BANK_4), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_BANK_5), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_BANK_6), NULL);
    ImmAssociateContext(GetItem(IDC_EDIT_BANK_7), NULL);
    
    NSF_HEADER header = { 0 };
    LoadInformationFromNSFHeader(header);

    if (m_cfg.GetInt(CONFIG_INI_ROOT, CONFIG_INI_TOPMOST, true))
    {
        PostCommand(ID_SETTINGS_TOPMOST);
    }

    _tstring strLanguage = m_cfg.GetString(CONFIG_INI_ROOT, CONFIG_INI_LANGUAGE, _T(""));
    if (_T("zh_CN") == strLanguage)
    {
        PostCommand(ID_LANGUAGE_ZH_CN);
    }
    else if (_T("zh_SG") == strLanguage)
    {
        PostCommand(ID_LANGUAGE_ZH_SG);
    }
    else if (_T("zh_HK") == strLanguage)
    {
        PostCommand(ID_LANGUAGE_ZH_HK);
    }
    else if (_T("zh_TW") == strLanguage)
    {
        PostCommand(ID_LANGUAGE_ZH_TW);
    }
    else if (_T("zh_MO") == strLanguage)
    {
        PostCommand(ID_LANGUAGE_ZH_MO);
    }
    else if (_T("ja_JP") == strLanguage)
    {
        PostCommand(ID_LANGUAGE_JA_JP);
    }
    else if (_T("ko_KR") == strLanguage)
    {
        PostCommand(ID_LANGUAGE_KO_KR);
    }
    else if (_T("en_US") == strLanguage)
    {
        PostCommand(ID_LANGUAGE_EN_US);
    }
    else
    {
        LANGID langID = GetThreadUILanguage();
        if (LANG_CHINESE == PRIMARYLANGID(langID))
        {
            if (SUBLANG_CHINESE_HONGKONG == SUBLANGID(langID))
            {
                PostCommand(ID_LANGUAGE_ZH_HK);
            }
            else if (SUBLANG_CHINESE_TRADITIONAL == SUBLANGID(langID))
            {
                PostCommand(ID_LANGUAGE_ZH_TW);
            }
            else if (SUBLANG_CHINESE_SINGAPORE == SUBLANGID(langID))
            {
                PostCommand(ID_LANGUAGE_ZH_SG);
            }
            else if (SUBLANG_CHINESE_MACAU == SUBLANGID(langID))
            {
                PostCommand(ID_LANGUAGE_ZH_MO);
            }
            else
            {
                PostCommand(ID_LANGUAGE_ZH_CN);
            }
        }
        else if(LANG_JAPANESE == PRIMARYLANGID(langID))
        {
            PostCommand(ID_LANGUAGE_JA_JP);
        }
        else if (LANG_KOREAN == PRIMARYLANGID(langID))
        {
            PostCommand(ID_LANGUAGE_KO_KR);
        }
        else
        {
            PostCommand(ID_LANGUAGE_EN_US);
        }
    }

    LoadAccel(IDR_ACCELERATOR_MAIN);

    return LRESULT(TRUE);
}

LRESULT CDialogFrame::OnRadioStandard(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    EnableItem(IDC_EDIT_PLAY_SPEED_NTSC, TRUE);
    EnableItem(IDC_EDIT_PLAY_SPEED_PAL, TRUE);
    if (wID == IDC_RADIO_STANDARD_NTSC) EnableItem(IDC_EDIT_PLAY_SPEED_PAL, FALSE);
    if (wID == IDC_RADIO_STANDARD_PAL) EnableItem(IDC_EDIT_PLAY_SPEED_NTSC, FALSE);
    return LRESULT(TRUE);
}

LRESULT CDialogFrame::OnWindowPosChinging(WPARAM wParam, LPARAM lParam)
{
    LPWINDOWPOS lpWindowPos = (LPWINDOWPOS)lParam;
    lpWindowPos->flags |= SWP_NOSIZE;
    return LRESULT(TRUE);
}

LRESULT CDialogFrame::OnClose(WPARAM wParam, LPARAM lParam)
{
    return (LRESULT)FALSE;
}

dlg_msg LRESULT CDialogFrame::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);
    POINT pt = { xPos, yPos };
    ClientToScreen(m_hWnd, &pt);
    CreatePopMenu(pt.x, pt.y, false);
    return LRESULT(TRUE);
}

LRESULT CDialogFrame::OnCommand(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    LONG_PTR style = GetWindowLongPtr(GetItem(wID), GWL_STYLE);

    if ((BS_RADIOBUTTON == (BS_RADIOBUTTON & style)) || (BS_AUTORADIOBUTTON == (BS_AUTORADIOBUTTON & style)))
    {
        if (wNotify == 0)
        {
            CheckRadioButton(wStart, wEnd, wID);
        }
    }

    return (LRESULT)FALSE;
}

LRESULT CDialogFrame::OnDropFiles(WPARAM wParam, LPARAM lParam)
{
    HDROP hDrop = (HDROP)wParam;
    TCHAR szFile[MAX_PATH] = { 0 };
    UINT nCount = DragQueryFile(hDrop, (UINT)(-1), szFile, _countof(szFile));

    for (UINT i = 0; i < nCount; i++)
    {
        ::DragQueryFile(hDrop, i, szFile, _countof(szFile));
        if (_T("nsf") == CPathUtils::GetFileExt(szFile))
        {
            LoadInformationFromFile(szFile);
            break;
        }
    }

    DragFinish(hDrop);

    return TRUE;
}

LRESULT CDialogFrame::OnFileOpen(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    if (m_fBusy)
    {
        return (LRESULT)TRUE;
    }

    m_fBusy = true;
    std::thread([this, wID]() {

        SetThreadUILanguage(m_langID);
        UINT uIndex = 0;
        COMDLG_FILTERSPEC filters[] = { { _T("nsf"), _T("*.nsf") } };
        _tstring strPath = m_strPath;
        if (CComFileDlg::GetOpenPath(m_hWnd, strPath, uIndex, filters, _countof(filters)))
        {
            LoadInformationFromFile(strPath);
        }
        m_fBusy = false;
        }
    ).detach();

    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnFileSave(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    if (!m_strPath.empty())
    {
        SaveInformationFromFile(m_strPath);
    }
    else
    {
        PostCommand(ID_FILE_SAVE_AS);
    }
    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnFileSaveAs(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    if (m_fBusy)
    {
        return (LRESULT)TRUE;
    }

    m_fBusy = true;
    std::thread([this, wID]() {

        SetThreadUILanguage(m_langID);
        UINT uIndex = 0;
        COMDLG_FILTERSPEC filters[] = { { _T("nsf"), _T("*.nsf") } };
        if (CComFileDlg::GetSavePath(m_hWnd, m_strPath, uIndex, filters, _countof(filters)))
        {
            if (0 !=  CStrUtils::CompareNoCase(CPathUtils::GetFileExt(m_strPath, false), _T("nsf")))
            {
                m_strPath += _T(".nsf");
            }

            SaveInformationFromFile(m_strPath);
        }
        m_fBusy = false;
        }
    ).detach();

    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnFileExit(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    PostMessage(WM_CLOSE);
    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnSettingsTopmost(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    HMENU hMenu = GetMenu(m_hWnd);
    bool fChecked = IsMenuItemChecked(hMenu, wID, false);
    fChecked = !fChecked;
    CheckMenuItem(hMenu, wID, fChecked, false);
    ::SetWindowPos(m_hWnd, fChecked ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    m_cfg.SetInt(CONFIG_INI_ROOT, CONFIG_INI_TOPMOST, fChecked);
    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnHelpAbout(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    ::ShellExecute(NULL, _T("open"), _T(HELP_ABOUT_URL), NULL, NULL, SW_SHOW);
    return LRESULT(TRUE);
}

LRESULT CDialogFrame::OnSettingsLang_zh_CN(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    HMENU hMenu = GetMenu(m_hWnd);
    CheckRadioMenuItem(hMenu, wID, true);
    LanguageChange(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));
    m_cfg.SetString(CONFIG_INI_ROOT, CONFIG_INI_LANGUAGE, _T("zh_CN"));
    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnSettingsLang_zh_SG(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    HMENU hMenu = GetMenu(m_hWnd);
    CheckRadioMenuItem(hMenu, wID, true);
    LanguageChange(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE));
    m_cfg.SetString(CONFIG_INI_ROOT, CONFIG_INI_LANGUAGE, _T("zh_SG"));
    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnSettingsLang_zh_HK(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    HMENU hMenu = GetMenu(m_hWnd);
    CheckRadioMenuItem(hMenu, wID, true);
    LanguageChange(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG));
    m_cfg.SetString(CONFIG_INI_ROOT, CONFIG_INI_LANGUAGE, _T("zh_HK"));
    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnSettingsLang_zh_TW(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    HMENU hMenu = GetMenu(m_hWnd);
    CheckRadioMenuItem(hMenu, wID, true);
    LanguageChange(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL));
    m_cfg.SetString(CONFIG_INI_ROOT, CONFIG_INI_LANGUAGE, _T("zh_TW"));
    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnSettingsLang_zh_MO(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    HMENU hMenu = GetMenu(m_hWnd);
    CheckRadioMenuItem(hMenu, wID, true);
    LanguageChange(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_MACAU));
    m_cfg.SetString(CONFIG_INI_ROOT, CONFIG_INI_LANGUAGE, _T("zh_MO"));
    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnSettingsLang_ja_JP(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    HMENU hMenu = GetMenu(m_hWnd);
    CheckRadioMenuItem(hMenu, wID, true);
    LanguageChange(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN));
    m_cfg.SetString(CONFIG_INI_ROOT, CONFIG_INI_LANGUAGE, _T("ja_JP"));
    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnSettingsLang_ko_KR(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    HMENU hMenu = GetMenu(m_hWnd);
    CheckRadioMenuItem(hMenu, wID, true);
    LanguageChange(MAKELANGID(LANG_KOREAN, SUBLANG_JAPANESE_JAPAN));
    m_cfg.SetString(CONFIG_INI_ROOT, CONFIG_INI_LANGUAGE, _T("ko_KR"));
    return (LRESULT)TRUE;
}

LRESULT CDialogFrame::OnSettingsLang_en_US(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd)
{
    HMENU hMenu = GetMenu(m_hWnd);
    CheckRadioMenuItem(hMenu, wID, true);
    LanguageChange(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
    m_cfg.SetString(CONFIG_INI_ROOT, CONFIG_INI_LANGUAGE, _T("en_US"));
    return (LRESULT)TRUE;
}

void CDialogFrame::LanguageChange(LANGID lang)
{
    m_langID = lang;
    SetThreadUILanguage(lang);

    //修改菜单文本
    {
        HMENU hMenu = GetMenu(m_hWnd);
        ModifyMenuText(hMenu, 0, IDS_STRING_FILE, TRUE);
        ModifyMenuText(hMenu, ID_FILE_OPEN, IDS_STRING_FILE_OPEN, FALSE);
        ModifyMenuText(hMenu, ID_FILE_SAVE, IDS_STRING_FILE_SAVE, FALSE);
        ModifyMenuText(hMenu, ID_FILE_SAVE_AS, IDS_STRING_FILE_SAVE_AS, FALSE);
        ModifyMenuText(hMenu, ID_FILE_EXIT, IDS_STRING_FILE_EXIT, FALSE);

        ModifyMenuText(hMenu, 1, IDS_STRING_SETTINGS, TRUE);
        ModifyMenuText(hMenu, ID_SETTINGS_TOPMOST, IDS_STRING_SETTINGS_TOPMOST, FALSE);
        ModifyMenuText(::GetSubMenu(hMenu, 1), 1, IDS_STRING_SETTINGS_LANGUAGE, TRUE);

        ModifyMenuText(hMenu, 2, IDS_STRING_HELP, TRUE);
        ModifyMenuText(hMenu, ID_HELP_ABOUT, IDS_STRING_HELP_ABOUT, FALSE);
    }

    //界面文本
    {
        SetItemString(IDC_STATIC_FILE_PATH, LoadString(IDS_STRING_PATH));
        SetItemString(IDC_BUTTON_OPEN_FILE, LoadString(IDS_STRING_OPEN));
        SetItemString(IDC_STATIC_TOTAL_SONGS, LoadString(IDS_STRING_TOTAL_SONGS));
        SetItemString(IDC_STATIC_START_SONG, LoadString(IDS_STRING_START_SONG));
        SetItemString(IDC_STATIC_LOAD_ADDR, LoadString(IDS_STRING_LOAD_ADDR));
        SetItemString(IDC_STATIC_INIT_ADDR, LoadString(IDS_STRING_INIT_ADDR));
        SetItemString(IDC_STATIC_PLAY_ADDR, LoadString(IDS_STRING_PLAY_ADDR));
        SetItemString(IDC_STATIC_NAME, LoadString(IDS_STRING_NAME));
        SetItemString(IDC_STATIC_ARTIST, LoadString(IDS_STRING_ARTIST));
        SetItemString(IDC_STATIC_COPYRIGHT, LoadString(IDS_STRING_COPYRIGHT));
        SetItemString(IDC_STATIC_STANDARD, LoadString(IDS_STRING_STANDARD));
        SetItemString(IDC_STATIC_PLAY_SPEED, LoadString(IDS_STRING_PLAY_SPEED));
        SetItemString(IDC_STATIC_BANKSWITCH, LoadString(IDS_STRING_BANKSWITCH));
        SetItemString(IDC_STATIC_EXTRA_SOUND, LoadString(IDS_STRING_EXTRA_SOUND));
    }

    // 标题
    {
        _tstring strCaption = LoadString(IDS_STRING_CAPTION);
        CVersionUtils::VERSION_INFO version = CVersionUtils::GetFileVersion(CPathUtils::GetCurrentModulePath());
        if (!version.strFileVersion.empty())
        {
            strCaption += _T(" ");
            strCaption += version.strFileVersion;
        }

        if (!version.strCompanyName.empty())
        {
            strCaption += _T(" ");
            strCaption += _T("(");
            strCaption += version.strCompanyName;
            strCaption += _T(")");
        }

        ::SetWindowText(m_hWnd, strCaption.c_str());
    }

    ::DrawMenuBar(m_hWnd);
}

LRESULT CDialogFrame::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
    if (WM_LBUTTONDBLCLK == lParam)
    {
        ShowForeground(m_hWnd);
    }

    if (NULL != m_hHideWnd)
    {
        ::SetForegroundWindow(m_hHideWnd);
    }

    if (WM_RBUTTONUP == lParam)
    {
        HWND hForegroundWindow = GetForegroundWindow();
        if (hForegroundWindow == m_hHideWnd || hForegroundWindow == m_hWnd)
        {
            POINT pt = { 0 };
            ::GetCursorPos(&pt);
            CreatePopMenu(pt.x, pt.y, false);
        }
    }

    return FALSE;
}

bool CDialogFrame::ShowForeground(HWND hWnd)
{
    HWND hForeWnd = ::GetForegroundWindow();
    DWORD dwForeID = ::GetWindowThreadProcessId(hForeWnd, NULL);
    DWORD dwCurID = ::GetCurrentThreadId();
    ::AttachThreadInput(dwCurID, dwForeID, TRUE);
    BOOL isSuc = ::SetForegroundWindow(hWnd);
    ::AttachThreadInput(dwCurID, dwForeID, FALSE);

    if (isSuc)
    {
        //return true;
    }

    if (!::IsWindowVisible(hWnd))
    {
        ::ShowWindow(hWnd, SW_SHOW);
    }

    if (IsIconic(hWnd))
    {
        ::ShowWindow(hWnd, SW_SHOWNORMAL);
    }

    return ::SetForegroundWindow(hWnd);
}

void CDialogFrame::ShowTrayIcon()
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = m_hWnd;
    nid.uID = 0;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_MAIN));
    StringCchCopy(nid.szTip, _countof(nid.szTip), _T("Demo"));
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void CDialogFrame::DeleteTrayIcon()
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(nid);
    nid.hWnd = m_hWnd;
    nid.uID = 0;
    nid.uFlags = 0;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

VOID CDialogFrame::CreatePopMenu(int xPos, int yPos, bool isTop)
{
    MENUITEMINFO mii = { 0 };
    TCHAR szBuf[MAX_PATH] = { 0 };

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_SUBMENU | MIIM_ID;
    mii.fType = MFT_STRING | MFT_OWNERDRAW;
    mii.dwTypeData = szBuf;

    if (!m_hWnd)
    {
        return;
    }

    {
        HMENU hMenu = m_hMainMenu;
        //ModifyMenuText(hMenu, ID_FILE_EXIT, IDS_FILE_EXIT, FALSE);
        //ModifyMenuText(hMenu, ID_SETTINGS_SHOW, IDS_SETTINGS_SHOW, FALSE);
        //ModifyMenuText(hMenu, ID_SETTINGS_HIDE, IDS_SETTINGS_HIDE, FALSE);
    }

    if (NULL == m_hPopMenu)
    {
        m_hPopMenu = CreatePopupMenu();
        int nCount = ::GetMenuItemCount(m_hMainMenu);
        for (int i = 0; i < nCount; i++)
        {
            mii.cch = _countof(szBuf);
            ::GetMenuItemInfo(m_hMainMenu, i, TRUE, &mii);
            ::InsertMenuItem(m_hPopMenu, i, TRUE, &mii);
        }
    }
    else
    {
        int nCount = ::GetMenuItemCount(m_hMainMenu);
        for (int i = 0; i < nCount; i++)
        {
            mii.cch = _countof(szBuf);
            ::GetMenuItemInfo(m_hMainMenu, i, TRUE, &mii);
            ::SetMenuItemInfo(m_hPopMenu, i, TRUE, &mii);
        }
    }

    TrackPopupMenuEx(m_hPopMenu, TPM_LEFTALIGN | (isTop ? TPM_BOTTOMALIGN : TPM_TOPALIGN),
        xPos, yPos, m_hWnd, NULL);
}

void CDialogFrame::LoadInformationFromFile(const _tstring& strFile)
{
    m_strPath = strFile;
    if (CBytesUtils::BytesFromFile(m_nsfData, m_strPath))
    {
        if (m_nsfData.size() >= sizeof(m_nsfHeader))
        {
            NSF_HEADER nsfHeader = { 0 };
            memcpy(&nsfHeader, m_nsfData.data(), sizeof(nsfHeader));

            if (IsNsfFormat(nsfHeader))
            {
                LoadInformationFromNSFHeader(nsfHeader);
                m_strPath = strFile;
                m_nsfHeader = nsfHeader;
                SetItemString(IDC_EDIT_FILE_PATH, m_strPath);
            }
            else
            {
                CMessageBoxCenter       m_boxCenter;
                MessageBox(m_hWnd,
                    LoadString(IDS_STRING_FORMAT_INCORRENT).c_str(),
                    LoadString(IDS_STRING_ERROR).c_str(),
                    MB_OK | MB_ICONERROR);
                m_strPath.clear();
                SetItemString(IDC_EDIT_FILE_PATH, m_strPath);
            }
        }
    }
}

void CDialogFrame::LoadInformationFromNSFHeader(const NSF_HEADER& header)
{
    SetItemInt(IDC_EDIT_TOTAL_SONGS, header.total_songs);
    SetItemInt(IDC_EDIT_START_SONG, header.start_song);

    SetItemString(IDC_EDIT_LOAD_ADDR, CStrUtils::Format(_T("%04X"), header.load_address));
    SetItemString(IDC_EDIT_INIT_ADDR, CStrUtils::Format(_T("%04X"), header.init_address));
    SetItemString(IDC_EDIT_PLAY_ADDR, CStrUtils::Format(_T("%04X"), header.play_address));

    SetItemString(IDC_EDIT_NAME, CStrUtils::U8StrToTStr(header.name));
    SetItemString(IDC_EDIT_ARTIST, CStrUtils::U8StrToTStr(header.artist));
    SetItemString(IDC_EDIT_COPYRIGHT, CStrUtils::U8StrToTStr(header.copyright));

    SetItemString(IDC_EDIT_PLAY_SPEED_NTSC, CStrUtils::Format(_T("%d"), header.play_speed_ntsc));
    SetItemString(IDC_EDIT_PLAY_SPEED_PAL, CStrUtils::Format(_T("%d"), header.play_speed_pal));

    for (int i = 0; i < 8; i++)
    {
        SetItemString(IDC_EDIT_BANK_0 + i, CStrUtils::Format(_T("%02X"), header.bankswitch[i]));
    }

    switch (header.standard)
    {
    case 0:
        CheckRadioButton(IDC_RADIO_STANDARD_NTSC, IDC_RADIO_STANDARD_NTSC_PAL, IDC_RADIO_STANDARD_NTSC);
        EnableItem(IDC_EDIT_PLAY_SPEED_NTSC, TRUE);
        EnableItem(IDC_EDIT_PLAY_SPEED_PAL, FALSE);
        break;
    case 1:
        CheckRadioButton(IDC_RADIO_STANDARD_NTSC, IDC_RADIO_STANDARD_NTSC_PAL, IDC_RADIO_STANDARD_PAL);
        EnableItem(IDC_EDIT_PLAY_SPEED_NTSC, FALSE);
        EnableItem(IDC_EDIT_PLAY_SPEED_PAL, TRUE);
        break;
    case 2:
        CheckRadioButton(IDC_RADIO_STANDARD_NTSC, IDC_RADIO_STANDARD_NTSC_PAL, IDC_RADIO_STANDARD_NTSC_PAL);
        EnableItem(IDC_EDIT_PLAY_SPEED_NTSC, TRUE);
        EnableItem(IDC_EDIT_PLAY_SPEED_PAL, TRUE);
        break;
    }

    for (int i = 0; i < 7; i++)
    {
        CheckButton(IDC_CHECK_AUDIO_VRC6 + i, header.extra_sound & (0x01 << i));
    }
}

bool CDialogFrame::IsNsfFormat(const NSF_HEADER& header)
{
    if (0x4D53454E == *(uint32_t*)&header.format_file &&
        0x1A == header.format_file[4]
        )
    {
        return true;
    }

    return false;
}

void CDialogFrame::SaveInformationFromFile(const _tstring& strPath)
{
    std::vector<uint8_t> vData = m_nsfData;
    NSF_HEADER  nsfHeader = { 0 };

    if (vData.size() < sizeof(nsfHeader))
    {
        vData.resize(sizeof(nsfHeader));
    }

    nsfHeader.format_file[0] = 0x4E;
    nsfHeader.format_file[1] = 0x45;
    nsfHeader.format_file[2] = 0x53;
    nsfHeader.format_file[3] = 0x4D;
    nsfHeader.format_file[4] = 0x1A;

    nsfHeader.total_songs = GetItemInt(IDC_EDIT_TOTAL_SONGS);
    nsfHeader.start_song = GetItemInt(IDC_EDIT_START_SONG);
    nsfHeader.load_address = CStrUtils::StrToUInt(GetItemString(IDC_EDIT_LOAD_ADDR), 16);
    nsfHeader.init_address = CStrUtils::StrToUInt(GetItemString(IDC_EDIT_INIT_ADDR), 16);
    nsfHeader.play_address = CStrUtils::StrToUInt(GetItemString(IDC_EDIT_PLAY_ADDR), 16);

    std::string strName = CStrUtils::TStrToU8Str(GetItemString(IDC_EDIT_NAME));
    std::string strArtist = CStrUtils::TStrToU8Str(GetItemString(IDC_EDIT_ARTIST));
    std::string strCopyRight = CStrUtils::TStrToU8Str(GetItemString(IDC_EDIT_COPYRIGHT));
    if (strName.size() > 31) strName.resize(31);
    if (strArtist.size() > 31) strArtist.resize(31);
    if (strCopyRight.size() > 31) strCopyRight.resize(31);
    memcpy(nsfHeader.name, strName.c_str(), strName.size());
    memcpy(nsfHeader.artist, strArtist.c_str(), strArtist.size());
    memcpy(nsfHeader.copyright, strCopyRight.c_str(), strCopyRight.size());

    nsfHeader.play_speed_ntsc = GetItemInt(IDC_EDIT_PLAY_SPEED_NTSC);
    nsfHeader.play_speed_pal = GetItemInt(IDC_EDIT_PLAY_SPEED_PAL);

    for (int i = 0; i < 8; i++)
    {
        nsfHeader.bankswitch[i] = CStrUtils::StrToUInt(GetItemString(IDC_EDIT_BANK_0 + i), 16);
    }

    if (IsButtonChecked(IDC_RADIO_STANDARD_NTSC)) nsfHeader.standard = 0x00;
    if (IsButtonChecked(IDC_RADIO_STANDARD_PAL)) nsfHeader.standard = 0x01;
    if (IsButtonChecked(IDC_RADIO_STANDARD_NTSC_PAL)) nsfHeader.standard = 0x02;

    for (int i = 0; i < 7; i++)
    {
        if (IsButtonChecked(IDC_CHECK_AUDIO_VRC6 + i))
        {
            nsfHeader.extra_sound |= (1 << i);
        }
    }

    memcpy(vData.data(), &nsfHeader, sizeof(nsfHeader));

    CBytesUtils::BytesToFile(strPath, vData);

    SetItemString(IDC_EDIT_FILE_PATH, m_strPath);
}

BOOL CDialogFrame::PreTranslateMessage(LPMSG pMsg)
{
    if (WM_KEYDOWN == pMsg->message)
    {
        UINT vkCode = (UINT)pMsg->wParam;
        SHORT fCtrl = GetAsyncKeyState(VK_CONTROL) & 0x8000;

        // 删除忽略
        if (VK_BACK == vkCode || VK_DELETE == vkCode)
        {
            return FALSE;
        }

        // 组合键忽略
        if (fCtrl)
        {
            if (vkCode == 'C' || vkCode == 'V' || vkCode == 'X' || vkCode == 'Z' || vkCode == 'A')
            {
                return FALSE;
            }
        }

        // 方向位置忽略
        if (vkCode >= VK_END && vkCode <= VK_DOWN)
        {
            return FALSE;
        }

        if (GetItem(IDC_EDIT_NAME) == pMsg->hwnd)
        {
            return InputLimitLength(pMsg->hwnd, 31);
        }
        if (GetItem(IDC_EDIT_ARTIST) == pMsg->hwnd)
        {
            return InputLimitLength(pMsg->hwnd, 31);
        }
        if (GetItem(IDC_EDIT_COPYRIGHT) == pMsg->hwnd)
        {
            return InputLimitLength(pMsg->hwnd, 31);
        }
    }

    return FALSE;
}

bool CDialogFrame::InputLimitLength(HWND hWnd, int nLength/* = -1*/)
{
    int iStart = 0;
    int iEnd = 0;
    int iSelect = 0;
    ::SendMessage(hWnd, EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

    iSelect = iEnd - iStart;

    int nCurLength = GetItemStringLength(hWnd);
    if (nCurLength >= nLength)
    {
        if (iSelect > 0)
        {
            return false;
        }

        return true;
    }

    _tstring strText = GetItemString(hWnd);

    return false;
}

bool CDialogFrame::InputLimitDec(HWND hWnd, int nLength/* = -1*/, UINT nMin/* = 0*/, UINT nMax/* = UINT_MAX*/)
{
    return false;
}

bool CDialogFrame::InputLimitHex(HWND hWnd, int nLength/* = -1*/, UINT nMin/* = 0*/, UINT nMax/* = UINT_MAX*/)
{
    return false;
}