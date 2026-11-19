#pragma once

#include <stdint.h>
#include <string>
#include <shobjidl.h>

#ifdef _UNICODE
using _tstring = std::wstring;
using _tfstream = std::wfstream;
#else
using _tstring = std::string;
using _tfstream = std::fstream;
#endif

typedef struct _SHELL_LINK_INFO
{
    _tstring strFile;       //目标路径
    _tstring strDesc;       //描述
    _tstring strArgs;       //参数
    _tstring strDir;        //工作目录
    _tstring strIconPath;   //图标路径
    _tstring strPathRel;    //相对路径
    WORD wHotkey;           //快捷键
    int iShowCmd;           //显示命令
    int iIcon;              //图标索引

    _SHELL_LINK_INFO()
        :
        wHotkey(0),
        iShowCmd(0),
        iIcon(0)
    {

    }

}SHELL_LINK_INFO;

class CLnkHelper
{
public:
    CLnkHelper();
    ~CLnkHelper();

    //
    // @brief: 加载快捷方式
    // @param: strFile          文件路径
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/objidl/nf-objidl-ipersistfile-load}.
    bool Load(const _tstring& strFile);

    //
    // @brief: 快捷方式保存
    // @param: strFile          文件路径
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/objidl/nf-objidl-ipersistfile-save}.
    bool Save(const _tstring& strFile);

    //
    // @brief: 获取信息
    // @param: void
    // @ret: SHELL_LINK_INFO    信息内容
    SHELL_LINK_INFO GetInfo() const;

    //
    // @brief: 设置信息
    // @param: info             信息内容
    // @ret: bool               操作结果
    bool SetInfo(const SHELL_LINK_INFO& info);

    //
    // @remark 
    // @brief: 尝试查找快捷方式目标
    // @param: hwnd             对话框的父窗口的句柄
    // @param: fFlags           操作标志
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-resolve}.
    bool Resolve(HWND hwnd, DWORD fFlags = SLR_NO_UI);

    //
    // @brief: 获取/设置目标路径
    // @param: strFile          目标路径
    // @param: fFlags           路径类型标志(SLGP_SHORTPATH, SLGP_RAWPATH, SLGP_RELATIVEPRIORITY)
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-getpath}.
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-setpath}.
    bool GetPath(_tstring& strFile, DWORD fFlags = SLGP_RAWPATH) const;
    _tstring GetPath(DWORD fFlags = SLGP_RAWPATH) const;
    bool SetPath(const _tstring& strFile);

    //
    // @brief: 获取/设置描述文本
    // @param: strDesc          描述文本
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-getdescription}.
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-setdescription}.
    bool GetDescription(_tstring& strDesc) const;
    _tstring GetDescription() const;
    bool SetDescription(const _tstring& strDesc);

    //
    // @brief: 获取/设置工作目录
    // @param: strDir           工作目录
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-getworkingdirectory}.
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-setworkingdirectory}.
    bool GetWorkingDirectory(_tstring& strDir) const;
    _tstring GetWorkingDirectory() const;
    bool SetWorkingDirectory(const _tstring& strDir);

    //
    // @brief: 获取/设置命令行参数
    // @param: strArgs          命令行参数
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-getarguments}.
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-setarguments}.
    bool GetArguments(_tstring& strArgs) const;
    _tstring GetArguments() const;
    bool SetArguments(const _tstring& strArgs);

    //
    // @brief: 获取/设置快捷键
    // @param: bModifier        修饰键(HOTKEYF_ALT, HOTKEYF_CONTROL, HOTKEYF_EXT, HOTKEYF_SHIFT)
    // @param: bKey             普通键
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-gethotkey}.
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-sethotkey}.
    bool GetHotkey(BYTE& bModifier, BYTE& bKey) const;
    WORD GetHotkey() const;
    bool SetHotkey(BYTE bModifier, BYTE bKey);
    bool SetHotkey(WORD wHotkey);

    //
    // @brief: 获取/设置显示命令
    // @param: iShowCmd         显示命令(SW_SHOWNORMAL, SW_SHOWMAXIMIZED, SW_SHOWMINIMIZED)
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-getshowcmd}.
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-setshowcmd}.
    bool GetShowCmd(int& iShowCmd) const;
    int GetShowCmd() const;
    bool SetShowCmd(int iShowCmd);

    //
    // @brief: 获取/设置图标位置
    // @param: strIconPath      图标路径
    // @param: iIcon            图标索引
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-geticonlocation}.
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-seticonlocation}.
    bool GetIconLocation(_tstring& strIconPath, int& iIcon) const;
    _tstring GetIconLocation(int* piIcon = nullptr) const;
    bool SetIconLocation(const _tstring& IconPath, int iIcon);

    //
    // @brief: 设置相对路径
    // @param: strPathRel       相对路径
    // @ret: bool               操作结果
    // {@link https://learn.microsoft.com/zh-cn/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinka-setrelativepath}.
    bool GetRelativePath(_tstring& strPathRel) const;
    _tstring GetRelativePath() const;
    bool SetRelativePath(const _tstring& strPathRel);

private:

    //
    // @brief: 初始化
    // @ret: bool               操作结果
    bool Initialized();

    //
    // @brief: 反初始化
    // @ret: void               无
    void Uninitialize();

    std::wstring _TStrToWStr(const _tstring& str);

private:

    IShellLink* m_SheelLink;    //Shell 链接接口对象指针
    IPersistFile* m_ppf;        //存储链接信息接口对象指针
    bool m_bInit;
};

