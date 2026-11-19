#pragma once
#include <vector>
#include <string>
#include <functional>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

typedef struct _SPLIT_DATA_INFO
{
    long long llBeginPos = 0;   //分割起始位置
    long long llEndPos = 0;     //分割结束位置
    long long llSize = 0;       //分割大小
    long long llCount = 0;      //分割数量

    _SPLIT_DATA_INFO() : llBeginPos(0), llEndPos(0), llSize(0), llCount(0)
    {
        memset(this, 0, sizeof(*this));
    }
}SPLIT_DATA_INFO;

typedef struct _SPLIT_FILE_INFO
{
    long long llBeginPos = 0;   //分割起始位置
    long long llSize = 0;       //分割大小

    _SPLIT_FILE_INFO() : llBeginPos(0), llSize(0)
    {
        memset(this, 0, sizeof(*this));
    }
}SPLIT_FILE_INFO;

class CFileSplitUtils
{
public:

    //
    // @brief: 合并文件
    // @param: fileList         文件路径字符串容器
    // @param: strFilePath      保存文件路径
    // @param: cbProgress       进度回调函数, 返回false则终止合并
    // @ret: void
    static bool CombineFile(
        const std::vector<_tstring>& fileList,
        const _tstring& strFilePath,
        std::function<bool(
            unsigned long long ullFileCount,            //已处理文件数
            unsigned long long ullFileTotal,       //总文件数
            unsigned long long ullBytesCount,           //已处理数据量
            unsigned long long ullBytesTotal       //总数据量
            )> cbProgress = nullptr
    );

    //
    // @brief: 分割文件
    // @param: strFilePath      文件路径
    // @param: strFileOutDir    保存文件夹
    // @param: strSplitCfg      分割配置字符串, 参数: 起始, 结束, 块大小, 份数
    //                              如: "0, -1, 8K, 16", 从文件头开始, 文件尾结束, 每个块8KB, 最多分割为16个文件
    // @param: strPrefixName    文件名前缀
    // @param: strExtName       文件名后缀
    // @param: nIndex           文件名起始索引
    // @param: bHexnIndex       文件名起始索引使用十六进制
    // @param: bHasRange        文件名包含数据范围
    // @param: bHasCrc32        文件名包含CRC32校验码
    // @param: cbProgress       进度回调函数, 返回false则终止合并
    // @ret: void
    static bool SplitFile(
        const _tstring& strFilePath,
        const _tstring& strFileOutDir,
        const _tstring& strSplitCfg,
        const _tstring& strPrefixName,
        const _tstring& strExtName,
        long long nIndex = 0,
        bool bHexIndex = false,
        bool bHasRange = false,
        bool bHasCrc32 = false,

        std::function<bool(
            unsigned long long ullFileCount,            //已处理文件数
            unsigned long long ullFileTotal,       //总文件数
            unsigned long long ullBytesCount,           //已处理数据量
            unsigned long long ullBytesTotal       //总数据量
            )> cbProgress = nullptr
    );

    //
    // @brief: 获取文件大小
    // @param: strFilePath      保存文件路径
    // @ret: long long          文件大小
    static long long GetFileSize(
        const _tstring& strFilePath
    );

    //
    // @brief: 从字符串解析分割信息
    // @param: strSplitCfg      分割配置字符串
    // @ret: std::vector<FILE_SPLIT_INFO>        分割信息
    static std::vector<SPLIT_DATA_INFO> GetSplitDataInfos(
        const _tstring& strSplitCfg
    );

    //
    // @brief: 从分割信息分解出分割元素
    // @param: splitInfo        分割信息
    // @param: llFileSize       文件大小
    // @ret: std::vector<SPLIT_DATA_INFO>        单个文件分割信息
    static std::vector<SPLIT_FILE_INFO> GetSpliFilenfos(
        const std::vector<SPLIT_DATA_INFO>& splitInfo,
        long long llFileSize
    );

    //
    // @brief: 从分割信息统计分割后文件总数
    // @param: splitInfo        分割信息
    // @param: llFileSize       文件大小
    // @ret: long long          分割后文件总数
    static long long GetSplitFileCount(
        const std::vector<SPLIT_DATA_INFO>& splitInfo,
        long long llFileSize
    );

    //
    // @brief: 从分割信息统计分割后数据总量
    // @param: splitInfo        分割信息
    // @param: llFileSize       文件大小
    // @ret: long long          分割后数据总量
    static long long GetSplitTotalSize(
        const std::vector<SPLIT_DATA_INFO>& splitInfo,
        long long llFileSize
    );

    //
    // @brief: 从分割信息统计分割后最大数据位置
    // @param: splitInfo        分割信息
    // @param: llFileSize       文件大小
    // @ret: long long          分割后最大数据位置
    static long long GetSplitMaxPos(
        const std::vector<SPLIT_DATA_INFO>& splitInfo,
        long long llFileSize
    );
};