#pragma once

#include <wtypesbase.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <map>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

namespace CPathUtils
{
    typedef struct _FILEINFO
    {
        _tstring m_filePath;
        unsigned long long m_fileSize;
        DWORD m_fileAttributes;
        unsigned long long m_ftCreationTime;
        unsigned long long m_ftLastAccessTime;
        unsigned long long m_ftLastWriteTime;

        _FILEINFO() :
            m_fileSize(0),
            m_fileAttributes(0),
            m_ftCreationTime(0),
            m_ftLastAccessTime(0),
            m_ftLastWriteTime(0)
        {

        };
    }FILEINFO;

    // 
    // @brief: 获取当前进程完全路径
    // @ret: 当前进程完全路径 如 D:\Software\HxDPortableSetup.exe
    _tstring GetCurrentModulePath();

    // 
    // @brief: 获取当前进程所在目录
    // @ret: 当前进程所在目录 如 D:\Software
    _tstring GetCurrentModuleDir();

    // 
    // @brief: 获取当前进程名
    // @ret: 当前进程名 如 HxDPortableSetup.exe
    _tstring GetCurrentModuleName(bool bHasExt = false);

    // 
    // @brief: 获取文件所在驱动器
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 文件夹        如 D:
    _tstring GetFileDriver(const _tstring& strPath);

    // 
    // @brief: 获取文件所在文件夹
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 文件夹        如 D:\Software
    _tstring GetFileDir(const _tstring& strPath);

    // 
    // @brief: 获取文件名
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @param: bHasExt     是否包含扩展名
    // @ret: 文件夹        如 HxDPortableSetup
    _tstring GetFileName(const _tstring& strPath, bool bHasExt = false);

    // 
    // @brief: 获取文件扩展名
    // @param: strPath     文件名, 如: D:\Software\HxDPortableSetup.exe
    // @param: bHasDot     是否包含点号
    // @ret: 文件夹        如 exe
    _tstring GetFileExt(const _tstring& strPath, bool bHasDot = false);

    // 
    // @brief: 获取短路径
    // @param: strPath     输入长路径(必须存在)
    // @ret: 短路径
    _tstring GetShortPath(const _tstring& strPath);

    // 
    // @brief: 获取长路径
    // @param: strPath     输入短路径(必须存在)
    // @ret: 长路径
    _tstring GetLongPath(const _tstring& strPath);

    // 
    // @brief: 获取全路径
    // @param: strPath     输入路径
    // @ret: 全路径
    _tstring GetFullPath(const _tstring& strPath);

    //
    // @brief: DOS路径转Nt路径
    // @param: strPath      路径
    // @ret: _tstring       Win32 样式的 NT 路径
    _tstring DosPathToNtPath(const _tstring& strPath);

    //
    // @brief: Nt路径转DOS路径
    // @param: strPath      路径
    // @ret: _tstring       DOS路径
    _tstring NtPathToDosPath(const _tstring& strPath);

    //
    // @brief: 获取路径属性
    // @param: strPath      路径
    // @ret: DWORD          属性
    bool GetAttributes(const _tstring& strPath, DWORD& dwAttr);
    DWORD GetAttributes(const _tstring& strPath);

    //
    // @brief: 设置路径属性
    // @param: strPath      路径
    // @ret: 成功返回true
    bool SetAttributes(const _tstring& strPath, const DWORD& dwAttr);

    //
    // @brief: 获取文件大小
    // @param: strPath      路径
    // @ret: 文件大小
    unsigned long long GetFileSize(const _tstring& strPath);

    // 
    // @brief: 检查是否为文件
    // @param: strPath      文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在       存在返回 true
    bool IsFile(const _tstring& strPath);

    // 
    // @brief: 检查是否为文件夹
    // @param: strPath      文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在       存在返回 true
    bool IsDirectory(const _tstring& strPath);

    // 
    // @brief: 检查路径是否存在
    // @param: strPath      文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在       存在返回 true
    bool IsExist(const _tstring& strPath);

    // 
    // @brief: 检查路径是否隐藏
    // @param: strPath      文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在       存在返回 true
    bool IsHidden(const _tstring& strPath);

    // 
    // @brief: 检查路径是否只读
    // @param: strPath      文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是否存在       只读返回 true
    bool IsReadonly(const _tstring& strPath);

    // 
    // @brief: 判断是否是完整路径
    // @param: strPath      文件名, 如: D:\Software\HxDPortableSetup.exe
    // @ret: 是返回 true
    bool IsFullPath(const _tstring& strPath);

    // 
    // @brief: 获取从 strSrc 到 strDest 的相对路径
    // @param: strDest      路径目标
    // @param: strSrc       基于路径源
    // @ret: 是返回 _tstring   相对路径
    _tstring GetRelativePath(const _tstring& strDest, const _tstring& strSrc, bool bFromIsDir = true);

    // 
    // @brief: 判断是否是相对路径
    // @param: strPath
    // @ret: 是返回 true
    bool IsRelative(const _tstring& strPath);

    // 
    // @brief: 判断是否是绝路径
    // @param: strPath
    // @ret: 是返回 true
    bool IsAbsolutePath(const _tstring& strPath);

    // 
    // @brief: 如果路径存在空格, 则使用引号括起来
    // @param: strPath      文件名, 如: C:\Program Files (x86)
    // @ret: 添加引号       添加引号返回 true
    bool QuoteSpaces(_tstring& strPath);

    // 
    // @brief: 删除路径开头和末尾的引号
    // @param: strPath      文件名, 如: "C:\Program Files (x86)"
    // @ret: 去除引号       去除引号返回 true
    bool UnquoteSpaces(_tstring& strPath);

    //
    // @brief: 获取相对路径的绝对路径
    // @param: strPath      文件(夹)相对路径
    // @ret: _tstring       转换后的绝对路径
    _tstring GetAbsolutePath(const _tstring& strPath);

    //
    // @brief: 重命名(夹)
    // @param: strSrc       原始文件名
    // @param: strDest      目标文件名(绝对路径或仅目标名)
    // @ret: 成功返回true
    bool Rename(const _tstring& strSrc, const _tstring& strDest);

    //
    // @brief: 移动文件(夹)
    // @param: strSrc       原始文件名
    // @param: strDest      目标文件名(绝对路径或仅目标名)
    // @ret: 成功返回true
    bool Move(const _tstring& strSrc, const _tstring& strDest);

    //
    // @brief: 拷贝文件(夹)
    // @param: strSrc       源路径
    // @param: strDest      目的路径(末尾为'\'则认为是复制到文件夹)
    // @param: bEmptyDir    是否拷贝空文件夹
    // @param: bOverwrites  是否覆盖文件
    // @param: depth        目录深度 如: 0：指定目录为根目录, 遍历0层子目录 
    // @ret: 成功返回true
    bool Copy(
        const _tstring& strSrc,
        const _tstring& strDest,
        bool bEmptyDir = true,
        bool bOverwrites = false,
        int32_t depth = MAX_PATH
    );

    //
    // @brief: 拷贝文件夹
    // @param: strSrc       源文件夹
    // @param: strDest      目的文件夹
    // @param: bEmptyDir    是否拷贝空文件夹
    // @param: bOverwrites  是否覆盖文件
    // @param: depth        目录深度 如: 0：指定目录为根目录, 遍历0层子目录 
    // @ret: 成功返回true
    bool CopyDir(
        const _tstring& strSrc,
        const _tstring& strDest,
        bool bEmptyDir = true,
        bool bOverwrites = false,
        int32_t depth = MAX_PATH
    );

    //
    // @brief: 删除文件或文件夹
    // @param: strPath      路径
    // @ret: 成功返回true
    bool Delete(const _tstring& strPath);

    //
    // @brief: 删除文件
    // @param: strPath      路径
    // @ret: 成功返回true
    bool DeleteArchive(const _tstring& strPath);

    //
    // @brief: 删除目录(递归删除子目录及文件)
    // @param: strPath      路径
    // @param: bSubOnly     仅删除子项
    // @ret: 成功返回true
    bool DeleteDir(const _tstring& strPath, bool bSubOnly = false);

    //
    // @brief: 创建目录(递归)
    // @param: strPath      路径
    // @ret: 成功返回true
    bool CreateDir(const _tstring& strPath);

    //
    // @brief: 拷贝文件(夹)(使用SHFileOperation)
    // @param: strSrcPath   源路径
    // @param: strDestPath  目的路径(末尾为'\'则认为是复制到文件夹)
    // @ret: 错误代码       成功返回0
    int SHCopy(const _tstring& strSrc, const _tstring& strDest);

    //
    // @brief: 重命名(夹)(使用SHFileOperation)
    // @param: strSrc       原始文件名
    // @param: strDest      目标文件名(绝对路径或仅目标名)
    // @ret: 错误代码       成功返回0
    int SHMove(const _tstring& strSrc, const _tstring& strDest);

    //
    // @brief: 移动文件(夹)(使用SHFileOperation)
    // @param: strSrc       原始文件名
    // @param: strDest      目标文件名(绝对路径或仅目标名)
    // @ret: 错误代码       成功返回0
    int SHRename(const _tstring& strSrc, const _tstring& strDest);

    //
    // @brief: 删除文件(使用SHFileOperation)
    // @param: strPath      路径
    // @ret: 错误代码       成功返回0
    int SHDelete(const _tstring& strSrc);

    //
    // @brief: 获取展开环境变量字符串
    // @param: strPath      路径
    // @ret: _tstring       展开字符串
    _tstring GetExpandEnvStrings(const _tstring& strPath);

    //
    // @brief: 获取环境变量字符串
    // @param: strPath      路径
    // @ret: _tstring       展开字符串
    _tstring GetUnExpandEnvStrings(const _tstring& strPath);

    //
    // @brief: 获取目录下文件路径
    // @param: fileList     文件路径字符串容器
    // @param: strDir       指定目录
    // @param: extList      扩展名过滤列表, 如: {("exe"), ("txt")}
    // @param: bIncludeHide 是否包含隐藏文件
    // @param: depth        目录深度 如: 0：指定目录为根目录, 遍历0层子目录 
    // @param: bIgnoreCase  是否忽略扩展名过滤大小写
    // @ret: std::vector<_tstring> 文件路径列表
    std::vector<_tstring> GetFileList(
        const _tstring& strDir,
        const std::vector<_tstring>& extList = {},
        bool bIncludeHide = false,
        int32_t depth = MAX_PATH,
        bool bIgnoreCase = true
    );

    //
    // @brief: 获取目录下文件路径
    // @param: strDir       指定目录
    // @param: depth        目录深度 如: 0：指定目录为根目录, 遍历0层子目录 
    // @param: dwExclude    排除属性
    // @param: dwContain    包含属性
    std::vector<_tstring> GetFileList(
        const _tstring& strDir,
        int32_t depth = MAX_PATH,
        DWORD dwExclude = 0,
        DWORD dwContain = 0
    );

    //
    // @brief: 获取目录下文件路径
    // @param: strDir       指定目录
    // @param: depth        目录深度 如: 0：指定目录为根目录, 遍历0层子目录 
    // @param: dwExclude    排除属性
    // @param: dwContain    包含属性
    std::vector<FILEINFO> GetFileListEx(
        const _tstring& strDir,
        int32_t depth = MAX_PATH,
        DWORD dwExclude = 0,
        DWORD dwContain = 0
    );

    //
    // @brief: 获取符合搜索要求的文件路径
    // @param: strDir       指定目录
    // @param: strSrcName   指定目录
    // @param: depth        目录深度 如: 0：指定目录为根目录, 遍历0层子目录 
    // @param: dwExclude    排除属性
    // @param: dwContain    包含属性
    std::vector<_tstring> SearchFiles(
        const _tstring& strDir,
        const _tstring& strSrcName,
        int32_t depth = MAX_PATH,
        DWORD dwExclude = 0,
        DWORD dwContain = 0
    );

    //
    // @brief: 获取磁盘卷可用空间
    // @param: strDir       指定目录, 如 "C:\"
    // @ret: ULONGLONG      可用空间
    ULONGLONG GetDiskFreeSize(const _tstring& strDir);

    //
    // @brief: 获取磁盘卷总空间
    // @param: strDir       指定目录, 如 "C:\"
    // @ret: ULONGLONG      总空间
    ULONGLONG GetDiskTotalSize(const _tstring& strDir);
}