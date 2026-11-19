#pragma once
#include "CWindow/CDialogBase.h"
#include "CNSFHeader.h"
#include "Win32Utils/CBytesUtils.h"
#include "Win32Utils/CIniHelper.h"

#define HELP_ABOUT_URL "https://gitee.com/flame_cyclone/nsf_information_editor"


#define CONFIG_INI_ROOT                                   _T("config")
#define CONFIG_INI_TOPMOST                                _T("settings_topmost")
#define CONFIG_INI_LANGUAGE                               _T("settings_language")

class CDialogFrame :
	public CDialogBase
{
public:
    LRESULT OnCommand(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);

protected:

    dlg_msg LRESULT OnInitDialog(WPARAM wParam, LPARAM lParam);
	dlg_msg LRESULT OnClose(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnRButtonUp(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnDropFiles(WPARAM wParam, LPARAM lParam);
    dlg_msg LRESULT OnWindowPosChinging(WPARAM wParam, LPARAM lParam);
    dlg_command LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);
    dlg_command LRESULT OnFileOpen(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnFileSave(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnFileSaveAs(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnFileExit(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnSettingsTopmost(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnRadioStandard(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnHelpAbout(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnSettingsLang_zh_CN(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnSettingsLang_zh_SG(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnSettingsLang_zh_HK(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnSettingsLang_zh_TW(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnSettingsLang_zh_MO(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnSettingsLang_ja_JP(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnSettingsLang_ko_KR(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);
    dlg_command LRESULT OnSettingsLang_en_US(WORD wNotify, WORD wID, HWND hWnd, WORD wStart, WORD wEnd);

    virtual BOOL PreTranslateMessage(LPMSG pMsg);

private:
    bool InputLimitLength(HWND hWnd, int nLength = -1);
    bool InputLimitDec(HWND hWnd, int nLength = -1, UINT nMin = 0, UINT nMax = UINT_MAX);
    bool InputLimitHex(HWND hWnd, int nLength = -1, UINT nMin = 0, UINT nMax = UINT_MAX);
    void LanguageChange(LANGID lang);
    void LoadInformationFromFile(const _tstring& strFile);
    void SaveInformationFromFile(const _tstring& strFile);
    void LoadInformationFromNSFHeader(const NSF_HEADER& header);
    bool IsNsfFormat(const NSF_HEADER& header);

    void ShowTrayIcon();
    void DeleteTrayIcon();
    VOID CreatePopMenu(int xPos, int yPos, bool isTop);
    bool ShowForeground(HWND hWnd);

private:
    HMENU                   m_hPopMenu = NULL;
    HMENU                   m_hMainMenu = NULL;
    HWND                    m_hHideWnd = NULL;

    LANGID                  m_langID;
    CIniHelper              m_cfg;
    std::vector<uint8_t>    m_nsfData;
    NSF_HEADER              m_nsfHeader = { 0 };
    _tstring                m_strPath;
    bool                    m_fBusy = false;

	DECLARE_DLG_MESSAGE_MAP()
};