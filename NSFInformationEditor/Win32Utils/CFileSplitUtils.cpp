#include "CFileSplitUtils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <tchar.h>
#include "CCrc32Utils.h"
#include "CStrUtils.h"
#include "CPathUtils.h"

long long CFileSplitUtils::GetFileSize(const _tstring& strFilePath)
{
    struct _stat64 statbuf = { 0 };
    if (0 == _tstat64(strFilePath.c_str(), &statbuf))
    {
        return statbuf.st_size;
    }

    return 0;
}

bool CFileSplitUtils::SplitFile(
    const _tstring& strFilePath,
    const _tstring& strFileOutDir,
    const _tstring& strSplitCfg,
    const _tstring& strPrefixName,
    const _tstring& strExtName,
    long long nIndex/* = 0*/,
    bool bHexIndex/* = false*/,
    bool bHasRange/* = false*/,
    bool bHasCrc32/* = false*/,
    std::function<bool(
        unsigned long long ullFileCount,            //已处理文件数
        unsigned long long ullFileTotal,       //总文件数
        unsigned long long ullBytesCount,           //已处理数据量
        unsigned long long ullBytesTotal       //总数据量
        )
    > cbProgress/* = nullptr*/
)
{
    _tstring strOutDir = strFileOutDir;
    _tstring strPrefix = strPrefixName;
    _tstring strExt = strExtName;

    //获取分割信息
    std::vector<SPLIT_DATA_INFO> vFileSplitInfos = GetSplitDataInfos(strSplitCfg);

    //获取文件大小
    long long llFileSize = GetFileSize(strFilePath);

    //统计分数数量
    long long llSplitCount = GetSplitFileCount(vFileSplitInfos, llFileSize);

    //获取最大数据位置
    long long llSplitMaxPos = GetSplitMaxPos(vFileSplitInfos, llFileSize);

    //获取文件大小
    long long llTotalSize = GetSplitTotalSize(vFileSplitInfos, llFileSize);

    //分解分割任务
    std::vector<SPLIT_FILE_INFO> vSplitInfos = GetSpliFilenfos(vFileSplitInfos, llFileSize);

    if (0 == llFileSize || 0 == llSplitCount)
    {
        return false;
    }

    std::ifstream inputFile(strFilePath, std::ios::binary | std::ios::in);
    if (!inputFile.is_open())
    {
        return false;
    }

    long long ullFileCount = 0;
    long long ullFileTotal = llSplitCount;
    long long ullBytesCount = 0;
    long long ullBytesTotal = llTotalSize;
    bool bCancel = false;

    //计算索引字符串格式
    _tstring strIndexFormat;
    if (bHexIndex)
    {
        strIndexFormat = CStrUtils::Format(_T("%%0%dX"), CStrUtils::Format(_T("%X"), llSplitCount + nIndex - 1).size());
    }
    else
    {
        strIndexFormat = CStrUtils::Format(_T("%%0%dd"), CStrUtils::Format(_T("%d"), llSplitCount + nIndex - 1).size());
    }

    //计算数据范围字符串格式
    int nRangeLength = (int)CStrUtils::Format(_T("%llX"), llSplitMaxPos).size();
    _tstring strRangeFormat = CStrUtils::Format(_T("%%0%dllX-%%0%dllX"), nRangeLength, nRangeLength);

    //前缀名检查
    if (strPrefix.empty())
    {
        strPrefix = CPathUtils::GetFileName(strFilePath, false);
    }

    //输出目录检查
    if (strOutDir.empty())
    {
        strOutDir = CPathUtils::GetFileDir(strFilePath);
        strOutDir += _T("\\");
        strOutDir += CPathUtils::GetFileName(strFilePath, false);
        strOutDir += _T("_Split");
    }

    //创建文件夹
    if (!CPathUtils::IsExist(strOutDir))
    {
        CPathUtils::CreateDir(strOutDir);
    }

    //开始分割文件
    _tstring strOutTemp = strOutDir + _T("\\Temp");

    //读写文件缓冲分配
    const size_t bufSize = static_cast<size_t>(1024 * 1024) * 4;
    char* pDataBuf = new (std::nothrow) char[bufSize];
    if (nullptr == pDataBuf)
    {
        return false;
    }

    //开始分割文件
    for (size_t i = 0; i < vSplitInfos.size(); i++)
    {
        SPLIT_FILE_INFO& info = vSplitInfos[i];

        std::ofstream outFile(strOutTemp, std::ios::binary | std::ios::out);
        if (!outFile.is_open())
        {
            return 0;
        }

        inputFile.seekg(info.llBeginPos, std::ios::beg);

        uint32_t uCrc32 = 0;
        long long nBlockSize = bufSize;
        long long llSize = info.llSize;

        while (llSize > 0)
        {
            if (llSize > bufSize)
            {
                nBlockSize = bufSize;
            }
            else
            {
                nBlockSize = llSize;
            }

            inputFile.read(pDataBuf, nBlockSize);
            size_t llRead = (size_t)inputFile.gcount();

            if (bHasCrc32)
            {
                uCrc32 = CCrc32Utils::GetPartCrc32(uCrc32, pDataBuf, llRead);
            }

            outFile.write(pDataBuf, llRead);
            llSize -= nBlockSize;
            ullBytesCount += llRead;

            //输出单个文件结束处理
            if (0 == llSize)
            {
                ullFileCount++;
                long long llBegin = info.llBeginPos;
                long long llEnd = info.llBeginPos + info.llSize - 1;

                _tstring strOutPath = strOutDir + _T("\\");

                //文件名前缀
                if (!strPrefix.empty())
                {
                    strOutPath += strPrefix;
                    strOutPath += _T("_");
                }

                //文件名序号
                strOutPath += CStrUtils::Format(strIndexFormat.c_str(), nIndex + i);

                //文件名添加数据范围
                if (bHasRange)
                {
                    _tstring strRange = CStrUtils::Format(strRangeFormat.c_str(), llBegin, llEnd);
                    strOutPath += _T("_");
                    strOutPath += strRange;
                }

                //文件名添加CRC32校验码
                if (bHasCrc32)
                {
                    strOutPath += _T("_");
                    strOutPath += CCrc32Utils::GetStrFromValue(uCrc32, true);
                }

                //文件名添加扩展名
                if (!strExt.empty())
                {
                    strOutPath += _T(".");
                    strOutPath += strExt;
                }

                outFile.close();

                //删除文件, 防止影响重命名失败
                CPathUtils::DeleteArchive(strOutPath);

                //临时文件重命名
                CPathUtils::Rename(strOutTemp, strOutPath);
            }

            //回调检查
            if (nullptr == cbProgress)
            {
                continue;
            }

            //回调返回false则停止合并文件
            if (!cbProgress(ullFileCount, ullFileTotal, ullBytesCount, ullBytesTotal))
            {
                outFile.close();

                //中断则删除临时文件
                CPathUtils::Delete(strOutTemp);
                bCancel = true;
                break;
            }
        }
    }

    if (nullptr != pDataBuf)
    {
        delete[]pDataBuf;
        pDataBuf = nullptr;
    }

    return true;
}

bool CFileSplitUtils::CombineFile(
    const std::vector<_tstring>& fileList,
    const _tstring& strFilePath,
    std::function<bool(
        unsigned long long ullFileCount,            //已处理文件数
        unsigned long long ullFileTotal,       //总文件数
        unsigned long long ullBytesCount,           //已处理数据量
        unsigned long long ullBytesTotal       //总数据量
        )
    > cbProgress/* = nullptr*/
)
{
    const size_t bufSize = static_cast<size_t>(1024 * 1024) * 4;
    char* pDataBuf = new (std::nothrow) char[bufSize];
    if (nullptr == pDataBuf)
    {
        return false;
    }

    long long ullFileCount = 0;
    long long ullTotalFileCount = 0;
    long long ullBytesCount = 0;
    long long ullTotalBytesCount = 0;
    bool bCancel = false;

    //打开输出文件
    std::ofstream outFile(strFilePath, std::ios::binary | std::ios::out);
    if (!outFile.is_open())
    {
        return false;
    }

    //统计文件数量与数据量
    for (const auto& item : fileList)
    {
        long long llFileSize = GetFileSize(item);
        if (-1 != llFileSize)
        {
            ullTotalBytesCount += llFileSize;
            ullTotalFileCount++;
        }
    }

    //开始将文件合并输出
    for (const auto& item : fileList)
    {
        //打开文件
        std::ifstream inputFile(item, std::ios::binary | std::ios::in);
        if (!inputFile.is_open())
        {
            continue;
        }

        //块大小
        long long nBlockSize = bufSize;

        //文件大小
        long long llSize = GetFileSize(item);

        while (llSize > 0)
        {
            if (llSize > bufSize)
            {
                nBlockSize = bufSize;
            }
            else
            {
                nBlockSize = llSize;
            }

            inputFile.read(pDataBuf, nBlockSize);
            long long llRead = inputFile.gcount();
            outFile.write(pDataBuf, llRead);
            llSize -= nBlockSize;
            ullBytesCount += llRead;

            if (0 == llSize)
            {
                ullFileCount++;
            }

            //回调检查
            if (cbProgress)
            {
                //回调返回false则停止合并文件
                if (!cbProgress(ullFileCount, ullTotalFileCount, ullBytesCount, ullTotalBytesCount))
                {
                    bCancel = true;
                    break;
                }
            }
        }

        inputFile.close();

        if (bCancel)
        {
            break;
        }
    }

    //善后处理
    if (nullptr != pDataBuf)
    {
        delete[]pDataBuf;
        pDataBuf = nullptr;
    }

    outFile.close();

    return true;
}

std::vector<SPLIT_DATA_INFO> CFileSplitUtils::GetSplitDataInfos(
    const _tstring& strSplitCfg
)
{
    std::vector<SPLIT_DATA_INFO> infos;

    //解析分割范围
    _tstring strSplitList = strSplitCfg;

    CStrUtils::Replace(strSplitList, _T(" "), _T(""), false);
    std::vector<_tstring> vSplitList = CStrUtils::SplitStr(strSplitList, _T("\r\n"));

    for (const auto& lineItem : vSplitList)
    {
        //开头 ; 认为是注释, 忽略不管
        if (_T(';') == lineItem.front())
        {
            continue;
        }

        //拆分子项
        std::vector<_tstring> vSplitItem = CStrUtils::SplitStr(lineItem, _T(";"));
        for (const auto& subItem : vSplitItem)
        {
            long long llValues[4] = { 0, -1, 0, 0 };

            std::vector<_tstring> vSplitValue = CStrUtils::SplitStr(subItem, _T(","));
            for (int i = 0; i < (int)vSplitValue.size(); i++)
            {
                if (i >= _countof(llValues))
                {
                    break;
                }

                _tstring strHex = _T("0x");
                _tstring strNegative = _T("-");
                bool bNegative = false;

                TCHAR* pEndChar = nullptr;

                _tstring itemValue = vSplitValue[i];

                if (0 == CStrUtils::FindNoCase(itemValue, strNegative))
                {
                    bNegative = true;
                    itemValue = itemValue.substr(1, (size_t)-1);
                }

                if (0 == CStrUtils::FindNoCase(itemValue, strHex))
                {
                    llValues[i] = _tcstoll(itemValue.c_str(), &pEndChar, 16);
                }
                else
                {
                    llValues[i] = _tcstoll(itemValue.c_str(), &pEndChar, 10);
                }

                if (nullptr != pEndChar)
                {
                    if (_T('K') == *pEndChar || _T('k') == *pEndChar)
                    {
                        llValues[i] *= 1024;
                    }
                    if (_T('M') == *pEndChar || _T('m') == *pEndChar)
                    {
                        llValues[i] *= 1024 * 1024;
                    }
                    if (_T('G') == *pEndChar || _T('g') == *pEndChar)
                    {
                        llValues[i] *= 1024 * 1024 * 1024;
                    }
                }

                if (bNegative)
                {
                    llValues[i] = 0 - llValues[i];
                }
            }

            SPLIT_DATA_INFO info;
            info.llBeginPos = llValues[0];
            info.llEndPos = llValues[1];
            info.llSize = llValues[2];
            info.llCount = llValues[3];

            infos.push_back(info);
        }
    }

    return infos;
}

long long CFileSplitUtils::GetSplitFileCount(
    const std::vector<SPLIT_DATA_INFO>& splitInfo,
    long long llFileSize
)
{
    long long llSplitCopunt = 0;

    for (const auto& item : splitInfo)
    {
        long long llBeginPos = item.llBeginPos;
        long long llEndPos = item.llEndPos;
        long long llSize = item.llSize;
        long long llCount = item.llCount;

        //结束位置
        if (item.llEndPos < 0)
        {
            llEndPos = llFileSize + item.llEndPos;
        }

        //最大结束位置不得超过文件末尾
        if (item.llEndPos >= llFileSize)
        {
            llEndPos = llFileSize - 1;
        }

        //确定起始位置
        if (item.llBeginPos < 0)
        {
            llBeginPos = llFileSize + item.llBeginPos;
        }

        //起始位置大小排序
        if (llEndPos < llBeginPos)
        {
            long long llTemp = llBeginPos;
            llBeginPos = llEndPos;
            llEndPos = llTemp;
        }

        //起始位置必须位于文件范围内
        if (llBeginPos < 0 || llBeginPos >= llFileSize || llEndPos < 0 || llEndPos >= llFileSize)
        {
            continue;
        }

        if (llCount < 0)//文件分割个数不能为负数
        {
            continue;
        }

        //未指定数量, 则认为是分割为1个文件
        if (0 == llCount)
        {
            if (0 == llSize)
            {
                llCount = 1;
            }
            else
            {
                llCount = llFileSize;
            }
        }

        if (0 == llSize)//大小为0, 则按照个数平均分割
        {
            if (llCount <= 1)
            {
                llCount = 1;
            }

            //统计分割大小
            long long packSize = 0;
            packSize = (llEndPos - llBeginPos + 1) / llCount;
            if (0 != (llEndPos - llBeginPos + 1) % llCount)
            {
                packSize++;
            }

            //最多分割个数
            llCount = (llEndPos - llBeginPos + 1) / packSize;
            if (0 != (llEndPos - llBeginPos + 1) % packSize)
            {
                llCount++;
            }

            //文件个数比实际数据还大, 则按照一个字节分割
            if (llEndPos - llBeginPos + 1 < llCount)
            {
                packSize = 1;
                llCount = llEndPos - llBeginPos + 1;
            }

            llSplitCopunt += llCount;
        }
        else if (llSize < 0)//从文件末尾向文件头部分割
        {
            llSize = abs(llSize);

            //大小限定为文件大小
            if (abs(llSize) > llFileSize)
            {
                llSize = llFileSize;
            }

            long long llMaxSplitCount = (llEndPos - llBeginPos + 1) / llSize;
            if (0 != (llEndPos - llBeginPos + 1) % llSize)
            {
                llMaxSplitCount++;
            }

            if (llCount > llMaxSplitCount)
            {
                llCount = llMaxSplitCount;
            }

            llSplitCopunt += llCount;
        }
        else if (llSize > 0)//从文件头部向文件末尾分割
        {
            //大小限定为文件大小
            if (abs(llSize) > llFileSize)
            {
                llSize = llFileSize;
            }

            long long llMaxSplitCount = (llEndPos - llBeginPos + 1) / llSize;
            if (0 != (llEndPos - llBeginPos + 1) % llSize)
            {
                llMaxSplitCount++;
            }

            if (llCount > llMaxSplitCount)
            {
                llCount = llMaxSplitCount;
            }

            llSplitCopunt += llCount;
        }
    }

    return llSplitCopunt;
}

long long CFileSplitUtils::GetSplitMaxPos(
    const std::vector<SPLIT_DATA_INFO>& splitInfo,
    long long llFileSize
)
{
    long long llSplitMaxPos = 0;

    for (const auto& item : splitInfo)
    {
        long long llBeginPos = item.llBeginPos;
        long long llEndPos = item.llEndPos;
        long long llSize = item.llSize;
        long long llCount = item.llCount;

        //结束位置
        if (item.llEndPos < 0)
        {
            llEndPos = llFileSize + item.llEndPos;
        }

        //最大结束位置不得超过文件末尾
        if (item.llEndPos >= llFileSize)
        {
            llEndPos = llFileSize - 1;
        }

        //确定起始位置
        if (item.llBeginPos < 0)
        {
            llBeginPos = llFileSize + item.llBeginPos;
        }

        //起始位置大小排序
        if (llEndPos < llBeginPos)
        {
            long long llTemp = llBeginPos;
            llBeginPos = llEndPos;
            llEndPos = llTemp;
        }

        //起始位置必须位于文件范围内
        if (llBeginPos < 0 || llBeginPos >= llFileSize || llEndPos < 0 || llEndPos >= llFileSize)
        {
            continue;
        }

        if (llCount < 0)//文件分割个数不能为负数
        {
            continue;
        }

        //未指定数量, 则认为是分割为1个文件
        if (0 == llCount)
        {
            if (0 == llSize)
            {
                llCount = 1;
            }
            else
            {
                llCount = llFileSize;
            }
        }

        if (llSize <= 0)//大小为0, 则按照个数平均分割
        {
            llSplitMaxPos = llEndPos;
        }
        else if (llSize > 0)//从文件头部向文件末尾分割
        {
            //大小限定为文件大小
            if (abs(llSize) > llFileSize)
            {
                llSize = llFileSize;
            }

            long long llMaxSplitCount = (llEndPos - llBeginPos + 1) / llSize;
            if (0 != (llEndPos - llBeginPos + 1) % llSize)
            {
                llMaxSplitCount++;
            }

            if (llCount > llMaxSplitCount)
            {
                llCount = llMaxSplitCount;
            }

            long long llMaxSplitSize = llCount * llSize;
            if (llMaxSplitSize > (llEndPos - llBeginPos + 1))
            {
                llSplitMaxPos = llEndPos;
            }
            else
            {
                llSplitMaxPos = llBeginPos + llCount * llSize - 1;
            }
        }

    }

    return llSplitMaxPos;
}

long long CFileSplitUtils::GetSplitTotalSize(
    const std::vector<SPLIT_DATA_INFO>& splitInfo,
    long long llFileSize
)
{
    long long llSplitSize = 0;

    for (const auto& item : splitInfo)
    {
        long long llBeginPos = item.llBeginPos;
        long long llEndPos = item.llEndPos;
        long long llSize = item.llSize;
        long long llCount = item.llCount;

        //结束位置
        if (item.llEndPos < 0)
        {
            llEndPos = llFileSize + item.llEndPos;
        }

        //最大结束位置不得超过文件末尾
        if (item.llEndPos >= llFileSize)
        {
            llEndPos = llFileSize - 1;
        }

        //确定起始位置
        if (item.llBeginPos < 0)
        {
            llBeginPos = llFileSize + item.llBeginPos;
        }

        //起始位置大小排序
        if (llEndPos < llBeginPos)
        {
            long long llTemp = llBeginPos;
            llBeginPos = llEndPos;
            llEndPos = llTemp;
        }

        //起始位置必须位于文件范围内
        if (llBeginPos < 0 || llBeginPos >= llFileSize || llEndPos < 0 || llEndPos >= llFileSize)
        {
            continue;
        }

        if (llCount < 0)//文件分割个数不能为负数
        {
            continue;
        }

        //未指定数量, 则认为是分割为1个文件
        if (0 == llCount)
        {
            if (0 == llSize)
            {
                llCount = 1;
            }
            else
            {
                llCount = llFileSize;
            }
        }

        if (0 == llSize)//大小为0, 则按照个数平均分割
        {
            if (llCount <= 1)
            {
                llCount = 1;
            }

            //统计分割大小
            long long packSize = 0;
            packSize = (llEndPos - llBeginPos + 1) / llCount;
            if (0 != (llEndPos - llBeginPos + 1) % llCount)
            {
                packSize++;
            }

            //最多分割个数
            llCount = (llEndPos - llBeginPos + 1) / packSize;
            if (0 != (llEndPos - llBeginPos + 1) % packSize)
            {
                llCount++;
            }

            //文件个数比实际数据还大, 则按照一个字节分割
            if (llEndPos - llBeginPos + 1 < llCount)
            {
                packSize = 1;
                llCount = llEndPos - llBeginPos + 1;
            }

            long long llMaxSplitSize = llCount * packSize;
            if (llMaxSplitSize > (llEndPos - llBeginPos + 1))
            {
                llMaxSplitSize = (llEndPos - llBeginPos + 1);
            }

            llSplitSize += llMaxSplitSize;
        }
        else if (llSize < 0)//从文件末尾向文件头部分割
        {
            llSize = abs(llSize);

            //大小限定为文件大小
            if (abs(llSize) > llFileSize)
            {
                llSize = llFileSize;
            }

            long long llMaxSplitCount = (llEndPos - llBeginPos + 1) / llSize;
            if (0 != (llEndPos - llBeginPos + 1) % llSize)
            {
                llMaxSplitCount++;
            }

            if (llCount > llMaxSplitCount)
            {
                llCount = llMaxSplitCount;
            }

            long long llMaxSplitSize = llCount * llSize;
            if (llMaxSplitSize > (llEndPos - llBeginPos + 1))
            {
                llMaxSplitSize = (llEndPos - llBeginPos + 1);
            }

            llSplitSize += llMaxSplitSize;
        }
        else if (llSize > 0)//从文件头部向文件末尾分割
        {
            //大小限定为文件大小
            if (abs(llSize) > llFileSize)
            {
                llSize = llFileSize;
            }

            long long llMaxSplitCount = (llEndPos - llBeginPos + 1) / llSize;
            if (0 != (llEndPos - llBeginPos + 1) % llSize)
            {
                llMaxSplitCount++;
            }

            if (llCount > llMaxSplitCount)
            {
                llCount = llMaxSplitCount;
            }

            long long llMaxSplitSize = llCount * llSize;
            if (llMaxSplitSize > (llEndPos - llBeginPos + 1))
            {
                llMaxSplitSize = (llEndPos - llBeginPos + 1);
            }

            llSplitSize += llMaxSplitSize;
        }
    }

    return llSplitSize;
}

std::vector<SPLIT_FILE_INFO> CFileSplitUtils::GetSpliFilenfos(
    const std::vector<SPLIT_DATA_INFO>& splitInfo,
    long long llFileSize
)
{
    std::vector<SPLIT_FILE_INFO> infos;

    for (const auto& item : splitInfo)
    {
        long long llBeginPos = item.llBeginPos;
        long long llEndPos = item.llEndPos;
        long long llSize = item.llSize;
        long long llCount = item.llCount;

        //结束位置
        if (item.llEndPos < 0)
        {
            llEndPos = llFileSize + item.llEndPos;
        }

        //最大结束位置不得超过文件末尾
        if (item.llEndPos >= llFileSize)
        {
            llEndPos = llFileSize - 1;
        }

        //确定起始位置
        if (item.llBeginPos < 0)
        {
            llBeginPos = llFileSize + item.llBeginPos;
        }

        //起始位置大小排序
        if (llEndPos < llBeginPos)
        {
            long long llTemp = llBeginPos;
            llBeginPos = llEndPos;
            llEndPos = llTemp;
        }

        //起始位置必须位于文件范围内
        if (llBeginPos < 0 || llBeginPos >= llFileSize || llEndPos < 0 || llEndPos >= llFileSize)
        {
            continue;
        }

        if (llCount < 0)//文件分割个数不能为负数
        {
            llCount = 0;
        }

        //未指定数量, 则认为是分割为1个文件
        if (0 == llCount)
        {
            if (0 == llSize)
            {
                llCount = 1;
            }
            else
            {
                llCount = llFileSize;
            }
        }

        if (0 == llSize)//大小为0, 则按照个数平均分割
        {
            if (llCount <= 1)
            {
                llCount = 1;
            }

            //统计分割大小
            long long packSize = 0;
            packSize = (llEndPos - llBeginPos + 1) / llCount;
            if (0 != (llEndPos - llBeginPos + 1) % llCount)
            {
                packSize++;
            }

            //最多分割个数
            llCount = (llEndPos - llBeginPos + 1) / packSize;
            if (0 != (llEndPos - llBeginPos + 1) % packSize)
            {
                llCount++;
            }

            //文件个数比实际数据还大, 则按照一个字节分割
            if (llEndPos - llBeginPos + 1 < llCount)
            {
                packSize = 1;
                llCount = llEndPos - llBeginPos + 1;
            }

            //平均分配
            for (int i = 0; i < llCount; i++)
            {
                SPLIT_FILE_INFO info;
                info.llBeginPos = llBeginPos + i * packSize;
                info.llSize = packSize;

                //超限处理
                if ((info.llBeginPos + info.llSize - 1) > llEndPos)
                {
                    info.llSize = llEndPos - info.llBeginPos + 1;
                }

                infos.push_back(info);
            }
        }
        else if (llSize < 0)//从文件末尾向文件头部分割
        {
            llSize = abs(llSize);

            //大小限定为文件大小
            if (abs(llSize) > llFileSize)
            {
                llSize = llFileSize;
            }

            //最大分割数量
            long long llMaxSplitCount = (llEndPos - llBeginPos + 1) / llSize;
            if (0 != (llEndPos - llBeginPos + 1) % llSize)
            {
                llMaxSplitCount++;
            }

            if (llCount > llMaxSplitCount)
            {
                llCount = llMaxSplitCount;
            }

            for (int i = 0; i < llCount; i++)
            {
                SPLIT_FILE_INFO info;
                info.llSize = llSize;
                info.llBeginPos = llEndPos - ((i + 1) * llSize) + 1;

                //越界处理
                if (info.llBeginPos + info.llSize < llBeginPos)
                {
                    break;
                }

                //越界处理
                if (info.llBeginPos < llBeginPos)
                {
                    info.llBeginPos = llBeginPos;
                    info.llSize = llEndPos - (i * llSize) + 1;
                }

                infos.push_back(info);
            }
        }
        else if (llSize > 0)//从文件头部向文件末尾分割
        {
            //大小限定为文件大小
            if (abs(llSize) > llFileSize)
            {
                llSize = llFileSize;
            }

            //最大分割数量
            long long llMaxSplitCount = (llEndPos - llBeginPos + 1) / llSize;
            if (0 != (llEndPos - llBeginPos + 1) % llSize)
            {
                llMaxSplitCount++;
            }

            if (llCount > llMaxSplitCount)
            {
                llCount = llMaxSplitCount;
            }

            for (int i = 0; i < llCount; i++)
            {
                SPLIT_FILE_INFO info;
                info.llBeginPos = llBeginPos + i * llSize;
                info.llSize = llSize;

                //越界处理
                if (info.llBeginPos > llEndPos)
                {
                    break;
                }

                //越界处理
                if ((info.llBeginPos + info.llSize - 1) > llEndPos)
                {
                    info.llSize = llEndPos - info.llBeginPos + 1;
                }

                infos.push_back(info);
            }
        }
    }

    return infos;
}