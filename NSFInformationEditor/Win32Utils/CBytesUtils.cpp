#include "CBytesUtils.h"

std::vector<uint8_t> CBytesUtils::BytesFromMemory(
    const void* pData, 
    size_t nSize
)
{
    std::vector<uint8_t> vResult;

    if (nullptr != pData && 0 != nSize)
    {
        //分配容器内存
        vResult.resize(nSize);
        const uint8_t* pBytes = reinterpret_cast<const uint8_t*>(pData);
        for (size_t i = 0; i < nSize; i++)
        {
            //直接赋值
            vResult[i] = pBytes[i];
        }
    }

    return vResult;
}

bool CBytesUtils::BytesFromFile(
    std::vector<uint8_t>& vBytes,
    const _tstring& strFilePath
)
{
    std::vector<uint8_t> vOutput;
    bool bResult = false;
    std::fstream inputFile(strFilePath, std::ios::binary | std::ios::in);
    if (!inputFile.is_open())
    {
        return false;
    }

    //获取文件大小
    inputFile.seekg(0, std::ios::end);
    std::streampos nSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    do
    {
        if (nSize <= 0)
        {
            break;
        }

        //分配读取缓冲
        uint8_t* pByteBuf = new (std::nothrow) uint8_t[(unsigned int)nSize];
        if (nullptr == pByteBuf)
        {
            break;
        }

        //读取文件到缓冲
        inputFile.read(reinterpret_cast<char*>(pByteBuf), nSize);
        std::streamsize nByteSize = inputFile.gcount();

        //分配容器内存
        vOutput.resize((size_t)nSize);
        for (size_t i = 0; i < (size_t)nByteSize; i++)
        {
            //直接赋值
            vOutput[i] = pByteBuf[i];
        }

        delete[]pByteBuf;
        pByteBuf = nullptr;

        bResult = true;

    } while (false);

    inputFile.close();

    if (bResult)
    {
        vBytes = vOutput;
    }

    return bResult;
}

std::vector<_tstring> CBytesUtils::SplitStr(const _tstring& str, const _tstring& delim)
{
    std::vector<_tstring> vectorOut;
    size_t iStart = 0;
    size_t iEnd = 0;

    while ((iStart = str.find_first_not_of(delim, iEnd)) != _tstring::npos)
    {
        iEnd = str.find(delim, iStart);
        vectorOut.push_back(str.substr(iStart, iEnd - iStart));
    }

    return vectorOut;
}

std::vector<uint8_t> CBytesUtils::BytesFromString(
    const _tstring& strInput,
    const _tstring& strDelimiter/* = _T("")*/,
    const _tstring& strPrefix/* = _T("")*/,
    const _tstring& strLinePrefix/* = _T("")*/,
    const _tstring& strReturn/* = _T("\r\n")*/
)
{
    _tstring strHexTemp;

    size_t nLinePrefixLength = strLinePrefix.size();
    size_t nElementPrefixLength = strPrefix.size();

    //单行分离
    std::vector<_tstring> vLineContent = SplitStr(strInput, strReturn);

    for (const auto& item : vLineContent)
    {
        //去除行前缀
        _tstring strLineContent = item.substr(nLinePrefixLength);

        //没有分割与前缀
        if (strDelimiter.empty() && strPrefix.empty())
        {
            strHexTemp += strLineContent;
            continue;
        }

        std::vector<_tstring> vLineElements;
        {
            //单个元素分离
            if (!strDelimiter.empty())
            {
                //去除分隔符
                vLineElements = SplitStr(strLineContent, strDelimiter);
            }

            //去除元素前缀
            if (!strPrefix.empty())
            {
                if (!vLineElements.empty())
                {
                    for (auto& item : vLineElements)
                    {
                        item = item.substr(nElementPrefixLength);
                    }

                }
                else
                {
                    vLineElements = SplitStr(strLineContent, strPrefix);
                }
            }

            //连接元素
            if (!vLineElements.empty())
            {
                for (auto& item : vLineElements)
                {
                    strHexTemp += item;
                }
            }
            else
            {
                strHexTemp += strLineContent;
            }
        }
    }

    return BytesFromHexString(strHexTemp);
}

std::vector<uint8_t> CBytesUtils::BytesFromHexString(
    const _tstring& strInput
)
{
    std::vector<uint8_t> vResult;

    uint8_t byte = 0;
    size_t nIndex = 0;
    for (const auto& ch : strInput)
    {
        uint8_t byteHalf = 0;
        if (ch >= _T('0') && ch <= _T('9'))
        {
            byteHalf = (uint8_t)(ch - _T('0'));
        }
        else if (ch >= _T('A') && ch <= _T('F'))
        {
            byteHalf = (uint8_t)(ch - _T('A')) + 10;
        }
        else if (ch >= _T('a') && ch <= _T('f'))
        {
            byteHalf = (uint8_t)(ch - _T('a')) + 10;
        }
        else
        {
            continue;
        }

        if (!(nIndex & 0x01))
        {
            byte = byteHalf << 4;
        }
        else
        {
            vResult.push_back(byte |= byteHalf);
        }
        nIndex++;
    }

    return vResult;
}

_tstring CBytesUtils::BytesFormat(
    const std::vector<uint8_t>& vBytes,
    int nStart/* = 0*/,
    size_t nLineSize/* = 16*/,
    const _tstring& strDelimiter/* = _T("")*/,
    const _tstring& strPrefix/* = _T("")*/,
    const _tstring& strLinePrefix/* = _T("")*/,
    size_t addr/* = -1*/,
    const _tstring& strReturn/* = _T("\r\n")*/
)
{
    _tstring strHex;
    const TCHAR szHex[] = _T("0123456789ABCDEF");

    nStart = nStart % nLineSize;

    for (int i = 0; i < nStart; i++)
    {
        strHex += Format(_T("   "), addr);
    }

    size_t nIndex = 0;

    for (const auto& byte : vBytes)
    {
        if (0 == (nIndex + nStart))
        {
            if (-1 != addr)
            {
                strHex += Format(_T("%08X "), addr);
            }

            strHex += strLinePrefix;
        }

        if ((0 != (nIndex + nStart)) && (0 == (nIndex + nStart) % nLineSize))
        {
            strHex += strReturn;
            if (-1 != addr)
            {
                addr += nLineSize;
                strHex += Format(_T("%08X "), addr);
            }
            strHex += strLinePrefix;
        }

        strHex += strPrefix;
        strHex += szHex[byte >> 4];
        strHex += szHex[byte & 0x0F];

        if (0 != ((nIndex + nStart) + 1) % nLineSize && ((nIndex + 1) != vBytes.size()))
        {
            strHex += strDelimiter;
        }

        nIndex++;
    }

    return strHex;
}

_tstring CBytesUtils::Format(LPCTSTR pFormat, ...)
{
    size_t nCchCount = MAX_PATH;
    _tstring strResult(nCchCount, 0);
    va_list args;

    va_start(args, pFormat);

    do
    {
        //格式化输出字符串
        int nSize = _vsntprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
        if (-1 != nSize)
        {
            strResult.resize(nSize);
            break;
        }

        //缓冲大小超限终止
        if (nCchCount >= INT32_MAX)
        {
            break;
        }

        //重新分配缓冲
        nCchCount *= 2;
        strResult.resize(nCchCount);

    } while (true);

    va_end(args);

    return strResult;
}

bool CBytesUtils::BytesToFile(
    const _tstring& strFilePath,
    const std::vector<uint8_t>& vBytes
)
{
    return MemoryToFile(strFilePath, vBytes.data(), vBytes.size());
}

bool CBytesUtils::MemoryToFile(
    const _tstring& strFilePath,
    const void* pData,
    size_t nSize
)
{
    std::ofstream outputFile(strFilePath, std::ios::binary | std::ios::trunc);

    if (!outputFile.is_open())
    {
        return false;
    }

    outputFile.write(reinterpret_cast<const char*>(pData), nSize);
    outputFile.close();

    return true;
}

bool CBytesUtils::BytesFormatToFile(
    const _tstring& strFilePath,
    const std::vector<uint8_t>& vBytes,
    size_t nLineSize/* = 16*/,
    const _tstring& strDelimiter/* = _T("")*/,
    const _tstring& strPrefix/* = _T("")*/,
    const _tstring& strLinePrefix/* = _T("")*/,
    const _tstring& strReturn/* = _T("\r\n")*/
)
{
    std::ofstream outputFile(strFilePath, std::ios::binary | std::ios::trunc);

    if (!outputFile.is_open())
    {
        return false;
    }

    _tstring strOut = BytesFormat(vBytes, 0, nLineSize, strDelimiter, strPrefix, strLinePrefix, -1, strReturn);
    outputFile.write(reinterpret_cast<const char*>(strOut.c_str()), strOut.size() * sizeof(TCHAR));
    outputFile.close();

    return true;
}

std::vector<uint8_t> CBytesUtils::BytesFormatFromFile(
    const _tstring& strFilePath,
    const _tstring& strDelimiter/* = _T("")*/,
    const _tstring& strPrefix/* = _T("")*/,
    const _tstring& strLinePrefix/* = _T("")*/,
    const _tstring& strReturn/* = _T("\r\n")*/
)
{
    _tstring strContent;

    std::fstream inputFile(strFilePath, std::ios::binary | std::ios::in);
    if (!inputFile.is_open())
    {
        return std::vector<uint8_t>();
    }

    //获取文件大小
    inputFile.seekg(0, std::ios::end);
    std::streampos nSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    do
    {
        if (nSize <= 0)
        {
            break;
        }

        TCHAR* pBuf = reinterpret_cast<TCHAR*> (new (std::nothrow) uint8_t[(unsigned int)nSize]);
        if (nullptr == pBuf)
        {
            break;
        }

        inputFile.read(reinterpret_cast<char*>(pBuf), nSize);
        inputFile.close();

        strContent = pBuf;
        delete[]pBuf;

    } while (false);

    return BytesFromString(strContent, strDelimiter, strPrefix, strLinePrefix, strReturn);
}