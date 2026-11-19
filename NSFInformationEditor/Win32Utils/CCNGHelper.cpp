#include "CCNGHelper.h"
#include <Wincrypt.h>

#pragma comment(lib,"Bcrypt.lib")

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)
#define FILE_HASH_BLOCK_SIZE        (1024 * 1024 * 4)

CCNGHelper::CCNGHelper()
    :
    m_hAlg(NULL),
    m_hHash(NULL),
    m_pbHashObject(NULL),
    m_pbHash(NULL),
    m_cbHash(0)
{

}

CCNGHelper::~CCNGHelper()
{
    Uninitialize();
}

bool CCNGHelper::Initialize(
    const _tstring& strAlgorithm/* = _T("MD5")*/
)
{
    bool bSuccess = false;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    DWORD cbData = 0;
    DWORD cbHashObject = 0;

    _tstring strAlgorithmName = _ToUpper(strAlgorithm);
    if (m_strAlgorithm == strAlgorithmName)
    {
        return true;
    }

    Uninitialize();

    do
    {
        std::wstring strAlgId = _TStrToWStr(strAlgorithmName);
        // 打开一个算法句柄
        // https://learn.microsoft.com/zh-cn/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
        status = BCryptOpenAlgorithmProvider(
            &m_hAlg, //指向接收 CNG 提供程序句柄 的 BCRYPT_ALG_HANDLE 变量的指针
            strAlgId.c_str(), //指向以 null 结尾的 Unicode 字符串的指针，该字符串标识请求的加密算法
            NULL, //指向以 null 结尾的 Unicode 字符串的指针，该字符串标识要加载的特定提供程序
            BCRYPT_HASH_REUSABLE_FLAG//修改函数行为的标志
        );

        if (!NT_SUCCESS(status))
        {
            break;
        }

        // 检索提供程序的子对象的大小（以字节为单位）
        // https://learn.microsoft.com/zh-cn/windows/win32/api/bcrypt/nf-bcrypt-bcryptgetproperty
        status = BCryptGetProperty(
            m_hAlg, //表示要获取其属性值的 CNG 对象的句柄
            BCRYPT_OBJECT_LENGTH, //指向以 null 结尾的 Unicode 字符串的指针，该字符串包含要检索的属性的名称
            (PBYTE)&cbHashObject, //接收属性值的缓冲区的地址
            sizeof(DWORD), //pbOutput 缓冲区的大小（以字节为单位）
            &cbData, //指向 ULONG 变量的指针，该变量接收复制到 pbOutput 缓冲区的字节数
            0 //一组标志，用于修改此函数的行为。 未为此函数定义任何标志
        );

        if (!NT_SUCCESS(status))
        {
            break;
        }

        // 在堆上分配哈希对象
        m_pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbHashObject);
        if (NULL == m_pbHashObject)
        {
            break;
        }

        // 检索哈希提供程序的哈希值的大小（以字节为单位）
        // https://learn.microsoft.com/zh-cn/windows/win32/api/bcrypt/nf-bcrypt-bcryptgetproperty
        status = BCryptGetProperty(
            m_hAlg, //表示要获取其属性值的 CNG 对象的句柄
            BCRYPT_HASH_LENGTH, //指向以 null 结尾的 Unicode 字符串的指针，该字符串包含要检索的属性的名称
            (PBYTE)&m_cbHash, //接收属性值的缓冲区的地址
            sizeof(DWORD), //pbOutput 缓冲区的大小（以字节为单位）
            &cbData, //指向 ULONG 变量的指针，该变量接收复制到 pbOutput 缓冲区的字节数
            0 //一组标志，用于修改此函数的行为。 未为此函数定义任何标志
        );

        if (!NT_SUCCESS(status))
        {
            break;
        }

        // 在堆上分配哈希缓冲区
        m_pbHash = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, m_cbHash);
        if (NULL == m_pbHash)
        {
            break;
        }

        // 创建哈希对象
        // https://learn.microsoft.com/zh-cn/windows/win32/api/bcrypt/nf-bcrypt-bcryptcreatehash
        status = ::BCryptCreateHash(
            m_hAlg, //创建提供程序时指定的算法必须支持哈希接口
            &m_hHash, //哈希或 MAC 对象的句柄
            m_pbHashObject, //接收哈希或 MAC 对象的缓冲区的指针
            cbHashObject, //pbHashObject 缓冲区的大小（以字节为单位）
            NULL, //指向缓冲区的指针
            0, //pbSecret 缓冲区的大小（以字节为单位）
            BCRYPT_HASH_REUSABLE_FLAG //修改函数行为的标志
        );

        if (!NT_SUCCESS(status))
        {
            break;
        }

        bSuccess = true;

    } while (false);

    if (bSuccess)
    {
        m_strAlgorithm = strAlgorithmName;
    }
    else
    {
        Uninitialize();
    }

    return bSuccess;
}

void CCNGHelper::Uninitialize()
{
    if (m_hAlg)
    {
        ::BCryptCloseAlgorithmProvider(m_hAlg, 0);
        m_hAlg = NULL;
    }

    if (m_hHash)
    {
        ::BCryptDestroyHash(m_hHash);
        m_hHash = NULL;
    }

    if (m_pbHashObject)
    {
        ::HeapFree(GetProcessHeap(), 0, m_pbHashObject);
        m_pbHashObject = NULL;
    }

    if (m_pbHash)
    {
        ::HeapFree(GetProcessHeap(), 0, m_pbHash);
        m_pbHash = NULL;
    }

    m_strAlgorithm.clear();
}

void CCNGHelper::Reset()
{
    if (!m_strAlgorithm.empty())
    {
        // 检索从先前调用 BCryptHashData 累积的数据的哈希值
        (void)::BCryptFinishHash(
            m_hHash, //用于计算哈希或 MAC 的哈希或 MAC 对象的句柄
            m_pbHash, //指向接收哈希或 MAC 值的缓冲区的指针
            m_cbHash, //pbOutput 缓冲区的大小（以字节为单位）
            0 //一组标志，用于修改此函数的行为。 当前未定义任何标志，因此此参数应为零
        );
    }
}

bool CCNGHelper::HashData(
    const void* lpData, 
    unsigned long long ullSize
)
{
    const char* pDataBegin = (const char*)lpData;
    const unsigned long ulMaxBlockSize = UINT32_MAX;
    bool bSuccess = false;

    if (m_strAlgorithm.empty())
    {
        return false;
    }

    // 小于32位最大值则直接计算哈希值
    if (ullSize <= ulMaxBlockSize)
    {
        return _HashData(pDataBegin, (unsigned long)ullSize);
    }

    // 分段计算哈希值
    while(ullSize > 0)
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

_tstring CCNGHelper::FinishHash(
    bool bUpper/* = true*/
)
{
    _tstring strResult;

    if (m_strAlgorithm.empty())
    {
        return strResult;
    }

    // 检索从先前调用 BCryptHashData 累积的数据的哈希值
    NTSTATUS status = ::BCryptFinishHash(
        m_hHash, //用于计算哈希或 MAC 的哈希或 MAC 对象的句柄
        m_pbHash, //指向接收哈希或 MAC 值的缓冲区的指针
        m_cbHash, //pbOutput 缓冲区的大小（以字节为单位）
        0 //一组标志，用于修改此函数的行为。 当前未定义任何标志，因此此参数应为零
    );

    if (NT_SUCCESS(status))
    {
        strResult = _BytesToString(m_pbHash, m_cbHash, bUpper);
    }

    return strResult;
}

_tstring CCNGHelper::GetFileHash(
    const _tstring& strPath, 
    bool bUpper/* = true*/
)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwBlockSize = FILE_HASH_BLOCK_SIZE;
    DWORD dwBytesRead = 0;

    if (m_strAlgorithm.empty())
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

_tstring CCNGHelper::GetDataHash(
    const void* pData, 
    unsigned long long ullSize,
    bool bUpper/* = true*/
)
{
    _tstring strResult;
    if (m_strAlgorithm.empty())
    {
        return strResult;
    }

    if (HashData(pData, ullSize))
    {
        strResult = FinishHash(bUpper);
    }

    return strResult;
}

bool CCNGHelper::_HashData(
    const void* lpData, 
    unsigned long ulSize
)
{
    // 数据缓冲区上执行单向哈希
    NTSTATUS status = ::BCryptHashData(
        m_hHash,
        (PUCHAR)lpData,
        ulSize,
        0);

    if (!NT_SUCCESS(status))
    {
        return false;
    }

    return true;
}

_tstring CCNGHelper::_BytesToString(
    const void* lpData, 
    size_t nSize,
    bool bUpper/* = true*/
)
{
    const TCHAR rgbDigitsUpper[] = _T("0123456789ABCDEF");
    const TCHAR rgbDigitsLower[] = _T("0123456789abcdef");
    _tstring strResult(nSize * 2, 0);
    LPCBYTE lpBytes = (LPCBYTE)lpData;

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

_tstring CCNGHelper::_ToUpper(
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

std::wstring CCNGHelper::_MultiStrToWStr(
    UINT CodePage, 
    const std::string& str
)
{
    //计算缓冲区所需的字符长度
    int cchWideChar = ::MultiByteToWideChar(CodePage, 0, str.c_str(), -1, NULL, NULL);
    std::wstring strResult(cchWideChar, 0);

    //成功则返回写入到指示的缓冲区的字符数
    size_t nConverted = ::MultiByteToWideChar(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size());

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
}

std::wstring CCNGHelper::_TStrToWStr(
    const _tstring& str
)
{
#ifdef _UNICODE
    return str;
#else
    return _MultiStrToWStr(CP_ACP, str);
#endif
}