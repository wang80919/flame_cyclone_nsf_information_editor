#pragma once
#include <string>
#include <tchar.h>
#include <windows.h>
#include <bcrypt.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

// 下一代加密辅助类
// 客户端: Windows Vista 及以上系统可用
// 服务器: Windows Server 2008 及以上系统可用
class CCNGHelper
{
public:

    CCNGHelper();
    ~CCNGHelper();

    //
    // @brief: 初始化
    // @param: strAlgorithm     哈希算法名字符串, 可选类型如下:
    //                          常见可选 "MD5", "SHA1", "SHA256", "SHA384", "SHA512"
    // 
    //                          全部支持可选如下:
    //                          "RSA", "RSA_SIGN", "DH", "DSA", "RC2", "RC4", "AES", "DES", "DESX", "3DES", "3DES_112", 
    //                          "MD2", "MD4", "MD5", "SHA1", "SHA256", "SHA384", "SHA512",
    //                          "AES-GMAC", "AES-CMAC", "ECDSA_P256", "ECDSA_P384", "ECDSA_P521", "ECDH_P256", "ECDH_P384", "ECDH_P521", 
    //                          "RNG", "FIPS186DSARNG", "DUAECRNG"
    //                          WINDOWS 8 及以上: "SP800_108_CTR_HMAC", "SP800_56A_CONCAT", "PBKDF2", "CAPI_KDF", "TLS1_1_KDF", "TLS1_2_KDF"
    //                          Windows 10 及以上: "ECDSA", "ECDH", "XTS-AES"
    //                          Windows 10 1803: "HKDF"
    //                          "CHACHA20_POLY1305"
    // 
    // @ret: bool               操作是否成功
    bool Initialize(
        const _tstring& strAlgorithm = _T("MD5")
    );

    //
    // @brief: 反初始化
    // @ret: 无
    void Uninitialize();

    //
    // @brief: 重置
    // @ret: 无
    void Reset();

    //
    // @brief: 计算哈希值
    // @param: pData            数据缓冲
    // @param: ulSize           数据长度
    // @ret: bool               操作是否成功
    bool HashData(
        const void* pData, 
        unsigned long long ulSize
    );

    //
    // @brief: 获取累积的哈希值结果
    // @param: bUpper           是否大写
    // @ret: _tstring           哈希值字符串
    _tstring FinishHash(
        bool bUpper = true
    );

    //
    // @brief: 获取文件的哈希值结果
    // @param: strPath          文件路径
    // @param: bUpper           是否大写
    // @ret: _tstring           哈希值结果字符串
    _tstring GetFileHash(
        const _tstring& strPath, 
        bool bUpper = true
    );

    //
    // @brief: 获取数据的哈希值结果
    // @param: pData            数据指针
    // @param: ullSize          数据长度
    // @param: bUpper           是否大写
    // @ret: _tstring           哈希值结果字符串
    _tstring GetDataHash(
        const void* pData,
        unsigned long long ullSize,
        bool bUpper = true
    );

private:

    //
    // @brief: 计算哈希
    // @param: pData            数据指针
    // @param: ulSize           数据长度
    // @ret: 无
    bool _HashData(
        const void* pData, 
        unsigned long ulSize
    );

    //
    // @brief: 字节内容转字符串
    // @param: pData            数据指针
    // @param: nSize            数据长度
    // @param: bUpper           是否大写
    // @ret: _tstring           转换后的字符串
    _tstring _BytesToString(
        const void* pData, 
        size_t nSize, 
        bool bUpper = true
    );

    //
    // @brief: 字符串转大小
    // @param: str              字符串
    // @ret: _tstring           转换后的字符串
    _tstring _ToUpper(
        const _tstring& str
    );

    //
    // @brief: 多字符字符串转宽字符串
    // @param: CodePage         代码页
    // @param: str              字符串
    // @ret: std::wstring       转换后的宽字符串
    std::wstring _MultiStrToWStr(
        UINT CodePage, 
        const std::string& str
    );
    
    //
    // @brief: 字符串转宽字符串
    // @param: str              字符串
    // @ret: std::wstring       转换后的宽字符串
    std::wstring _TStrToWStr(
        const _tstring& str
    );

private:

    BCRYPT_ALG_HANDLE       m_hAlg = NULL;              // 算法提供程序句柄
    BCRYPT_HASH_HANDLE      m_hHash = NULL;             // 哈希对象句柄
    PBYTE                   m_pbHashObject = NULL;      // 哈希对象内存
    PBYTE                   m_pbHash = NULL;            // 哈希值缓冲
    DWORD                   m_cbHash = 0;               // 哈希值缓冲大小
    _tstring                m_strAlgorithm;             // 算法名
    std::string             m_dataBuf;                  // 数据缓冲(用于文件读取)
};
