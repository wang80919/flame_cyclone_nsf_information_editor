#pragma once

#include <Windows.h>
#include <io.h>
#include <tchar.h>
#include <string>
#include <commctrl.h>
#include <vector>
#include <shobjidl_core.h>

#ifdef UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CComFileDlg
{
public:

    //
    // @brief: 选择保存路径
    // @param: hOwner           父窗口句柄, 可为NULL
    // @param: strPath          保存文件路径
    // @param: nFiltersIndex    文件类型过滤器文件类型索引(从1开始)
    // @param: pFilters         文件类型过滤器数组 如 COMDLG_FILTERSPEC filters[] = { { _T("nes"), _T("*.nes") }, { _T("unf"), _T("*.unf") }, { _T("bin"), _T("*.bin") }};
    // @param: filterCnt        文件类型过滤器个数 如 filters中元素的个数
    // @ret: 操作结果成功与否
    static bool GetSavePath(HWND hOwner, _tstring& strPath, UINT& nFiltersIndex, const COMDLG_FILTERSPEC* pFilters, int filterCnt);

    //
    // @brief: 选择保存文件夹
    // @param: hOwner           父窗口句柄, 可为NULL
    // @param: strPath          保存路径
    // @ret: 操作结果成功与否
    static bool GetSaveDir(HWND hOwner, _tstring& strDir);

    //
    // @brief: 选择打开文件路径
    // @param: hOwner           父窗口句柄, 可为NULL
    // @param: strPath          打开文件路径
    // @param: nFiltersIndex    文件类型过滤器文件类型索引(从1开始)
    // @param: pFilters         文件类型过滤器数组 如 COMDLG_FILTERSPEC filters[] = { { _T("nes"), _T("*.nes") }, { _T("unf"), _T("*.unf") }, { _T("bin"), _T("*.bin") }};
    // @param: filterCnt        文件类型过滤器个数 如 filters中元素的个数
    // @ret: 操作结果成功与否
    static bool GetOpenPath(HWND hOwner, _tstring& strPath, UINT& nFiltersIndex, const COMDLG_FILTERSPEC* pFilters, int filterCnt);

    //
    // @brief: 获取所选文件列表
    // @param: hOwner           父窗口句柄, 可为NULL
    // @param: strPath          保存路径
    // @param: pFilters         文件类型过滤器数组 如 COMDLG_FILTERSPEC filters[] = { { _T("nes"), _T("*.nes") }, { _T("unf"), _T("*.unf") }, { _T("bin"), _T("*.bin") }};
    // @param: filterCnt        文件类型过滤器个数 如 filters中元素的个数
    // @ret: 操作结果成功与否
    static std::vector<_tstring> GetFileList(HWND hOwner, _tstring& strDir, const COMDLG_FILTERSPEC* pFilters, int filterCnt);

    // 
    // @brief: 获取当前进程完全路径
    // @ret: 当前进程完全路径 如 D:\Software\HxDPortableSetup.exe
    static _tstring GetCurrentModulePath();

    // 
    // @brief: 获取当前进程所在目录
    // @ret: 当前进程所在目录 如 D:\Software
    static _tstring GetCurrentModuleDir();

    // 
    // @brief: 获取当前进程名
    // @ret: 当前进程名 如 HxDPortableSetup.exe
    static _tstring GetCurrentModuleName(bool bHasExt = false);

    // 
    // @brief: 获取文件名
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @param: bHasExt     是否包含扩展名
    // @ret: 文件夹        如 HxDPortableSetup
    static _tstring GetFileName(const _tstring& strPath, bool bHasExt/* = true*/);

    // 
    // @brief: 获取文件所在文件夹
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 文件夹        如 D:\Software
    static _tstring GetFileDir(const _tstring& strPath);

    // 
    // @brief: 检查路径是否存在
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在      存在返回 true
    static bool IsExist(const _tstring& strPath);

    // 
    // @brief: 检查是否为文件夹
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在      存在返回 true
    static bool IsDirectory(const _tstring& strPath);

    // 
    // @brief: 检查是否为文件
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在      存在返回 true
    static bool IsArchive(const _tstring& strPath);
};

