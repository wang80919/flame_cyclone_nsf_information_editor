#include "CCryptHelper.h"

#define FILE_HASH_BLOCK_SIZE        (1024 * 1024 * 4)

CCryptHelper::CCryptHelper()
    :
    m_hProv(NULL),
    m_hHash(NULL),
    m_dwAlgorithm(0)
{

}

CCryptHelper::~CCryptHelper()
{
    Uninitialize();
}


bool CCryptHelper::Initialize(
    DWORD dwAlgorithm/* = CALG_MD5*/
)
{
    bool bSuccess = false;

    if (m_dwAlgorithm == dwAlgorithm)
    {
        return true;
    }

    Uninitialize();

    do
    {
        // 获取加密服务提供程序句柄
        // https://learn.microsoft.com/zh-cn/windows/win32/api/wincrypt/nf-wincrypt-cryptacquirecontextw
        if (!CryptAcquireContext(
            &m_hProv,//指向 CSP 句柄的指针
            NULL,//密钥容器名称
            NULL,//要使用的 CSP 的名称
            PROV_RSA_FULL,//要获取的提供程序的类型
            CRYPT_VERIFYCONTEXT//标记值。 此参数通常设置为零
        ))
        {
            break;
        }

        bSuccess = true;

    } while (false);

    if (bSuccess)
    {
        m_dwAlgorithm = dwAlgorithm;
    }

    return bSuccess;
}

void CCryptHelper::Uninitialize()
{
    if (m_hHash)
    {
        ::CryptDestroyHash(m_hHash);
        m_hHash = NULL;
    }

    if (m_hProv)
    {
        ::CryptReleaseContext(m_hProv, 0);
        m_hProv = NULL;
    }

    m_dwAlgorithm = 0;
}

void CCryptHelper::Reset()
{
    if (m_hHash)
    {
        ::CryptDestroyHash(m_hHash);
        m_hHash = NULL;
    }
}

bool CCryptHelper::HashData(
    const void* pData, 
    unsigned long long ullSize
)
{
    const char* pDataBegin = (const char*)pData;
    const unsigned long ulMaxBlockSize = UINT32_MAX;
    bool bSuccess = false;

    if (0 == m_dwAlgorithm)
    {
        return false;
    }

    // 小于32位最大值则直接计算哈希值
    if (ullSize <= ulMaxBlockSize)
    {
        return _HashData(pDataBegin, (unsigned long)ullSize);
    }

    // 分段计算哈希值
    while (ullSize > 0)
    {
        unsigned long ulReadSize = (ullSize > ulMaxBlockSize) ? ulMaxBlockSize : (unsigned long)ullSize;
        if (!_HashData(pDataBegin, ulReadSize))
        {
            break;
        }

        pDataBegin += ulReadSize;
        ullSize -= ulReadSize;
    }

    return bSuccess;
}

_tstring CCryptHelper::FinishHash(
    bool bUpper/* = true*/
)
{
    _tstring strResult;
    BYTE byteHash[MAX_PATH] = { 0 };
    DWORD cbHash = sizeof(byteHash);

    if (0 == m_dwAlgorithm)
    {
        return strResult;
    }

    if (NULL == m_hHash)
    {
        // 创建哈希处理对象
        // https://learn.microsoft.com/zh-cn/windows/win32/api/wincrypt/nf-wincrypt-cryptcreatehash
        ::CryptCreateHash(
            m_hProv, //通过调用 CryptAcquireContext 创建的 CSP 的句柄
            m_dwAlgorithm, //标识要使用的哈希算法 的ALG_ID 值, 此参数的有效值因使用的 CSP 而异
            0, //如果哈希算法的类型是键控哈希, 则哈希的密钥在此参数中传递
            0, //标志值
            &m_hHash//输出哈希对象的地址
        );
    }

    if (NULL == m_hHash)
    {
        return strResult;
    }

    // 检索控制哈希对象操作的数据, 获取实际哈希值
    // https://learn.microsoft.com/zh-cn/windows/win32/api/wincrypt/nf-wincrypt-cryptgethashparam
    if (::CryptGetHashParam(
        m_hHash, //要查询的哈希对象的句柄
        HP_HASHVAL, //查询类型
        byteHash, //指向接收指定值数据的缓冲区的指针
        &cbHash, //指向指定 pbData 缓冲区大小
        0 //保留供将来使用，必须为零
    ))
    {
        strResult = _BytesToString(byteHash, cbHash, bUpper);
    }

    // 销毁哈希对象
    if (m_hHash)
    {
        ::CryptDestroyHash(m_hHash);
        m_hHash = NULL;
    }

    return strResult;
}

_tstring CCryptHelper::GetFileHash(
    const _tstring& strPath, 
    bool bUpper/* = true*/
)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwBlockSize = FILE_HASH_BLOCK_SIZE;
    DWORD dwBytesRead = 0;

    if (0 == m_dwAlgorithm)
    {
        return _tstring(_T(""));
    }

    do
    {
        // 打开文件
        // https://learn.microsoft.com/zh-cn/windows/win32/api/fileapi/nf-fileapi-createfilew
        hFile = CreateFile(strPath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN,
            NULL);

        if (INVALID_HANDLE_VALUE == hFile)
        {
            break;
        }

        if (m_dataBuf.empty())
        {
            m_dataBuf.resize(dwBlockSize);
        }

        // 读取文件数据
        // https://learn.microsoft.com/zh-cn/windows/win32/api/fileapi/nf-fileapi-readfile
        while (::ReadFile(hFile, &m_dataBuf[0], dwBlockSize, &dwBytesRead, NULL))
        {
            if (0 == dwBytesRead)
            {
                break;
            }

            if (!_HashData(&m_dataBuf[0], dwBytesRead))
            {
                break;
            }
        }

    } while (false);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
    }

    return FinishHash(bUpper);
}

_tstring CCryptHelper::GetDataHash(
    const void* pData, 
    unsigned long long ullSize,
    bool bUpper/* = true*/
)
{
    _tstring strResult;
    if (0 == m_dwAlgorithm)
    {
        return strResult;
    }

    if (HashData(pData, ullSize))
    {
        strResult = FinishHash(bUpper);
    }

    return strResult;
}

bool CCryptHelper::_HashData(
    const void* pData, 
    unsigned long ulSize
)
{
    bool bSuccess = false;

    if (NULL == m_hHash)
    {
        // 创建哈希处理对象
        // https://learn.microsoft.com/zh-cn/windows/win32/api/wincrypt/nf-wincrypt-cryptcreatehash
        ::CryptCreateHash(
            m_hProv, //通过调用 CryptAcquireContext 创建的 CSP 的句柄
            m_dwAlgorithm, //标识要使用的哈希算法 的ALG_ID 值, 此参数的有效值因使用的 CSP 而异
            0, //如果哈希算法的类型是键控哈希, 则哈希的密钥在此参数中传递
            0, //标志值
            &m_hHash//输出哈希对象的地址
        );
    }

    if (NULL == m_hHash)
    {
        return false;
    }

    // 将数据添加到指定的哈希对象
    bSuccess = ::CryptHashData(
        m_hHash,
        (const BYTE*)pData,
        ulSize,
        0
    );
    return bSuccess;
}

_tstring CCryptHelper::_BytesToString(
    const void* pData, 
    size_t nSize, 
    bool bUpper/* = true*/
)
{
    const TCHAR rgbDigitsUpper[] = _T("0123456789ABCDEF");
    const TCHAR rgbDigitsLower[] = _T("0123456789abcdef");
    _tstring strResult(nSize * 2, 0);
    LPCBYTE lpBytes = (LPCBYTE)pData;

    if (bUpper)
    {
        for (DWORD i = 0; i < nSize; i++)
        {
            strResult[i * 2] = rgbDigitsUpper[lpBytes[i] >> 4];
            strResult[i * 2 + 1] = rgbDigitsUpper[lpBytes[i] & 0x0F];
        }
    }
    else
    {
        for (DWORD i = 0; i < nSize; i++)
        {
            strResult[i * 2] = rgbDigitsLower[lpBytes[i] >> 4];
            strResult[i * 2 + 1] = rgbDigitsLower[lpBytes[i] & 0x0F];
        }
    }

    return strResult;
}

_tstring CCryptHelper::_ToUpper(
    const _tstring& str
)
{
    _tstring strResult = str;
    for (auto& item : strResult)
    {
        if (item >= _T('a') && item <= _T('z'))
        {
            item -= 0x20;
        }
    }

    return strResult;
}

std::wstring CCryptHelper::_MultiStrToWStr(
    UINT CodePage, 
    const std::string& str
)
{
    // 计算缓冲区所需的字符长度
    // https://learn.microsoft.com/zh-cn/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
    int cchWideChar = ::MultiByteToWideChar(CodePage, 0, str.c_str(), -1, NULL, NULL);
    std::wstring strResult(cchWideChar, 0);

    //成功则返回写入到指示的缓冲区的字符数
    size_t nConverted = ::MultiByteToWideChar(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size());

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
}

std::wstring CCryptHelper::_TStrToWStr(
    const _tstring& str
)
{
#ifdef _UNICODE
    return str;
#else
    return _MultiStrToWStr(CP_ACP, str);
#endif
}