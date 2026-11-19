#pragma once
#include <string>
#include <tchar.h>
#include <windows.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

// 加密辅助类
// 客户端: Windows XP 及以上系统可用
// 服务器: Windows Server 2003 及以上系统可用
class CCryptHelper
{
public:
    CCryptHelper();
    ~CCryptHelper();

    //
    // @brief: 初始化
    // @param: strAlgorithm     哈希算法名字符串, 可选类型如下:
    //                          常见可选 CALG_MD5, CALG_SHA1, CALG_SHA_256, CALG_SHA_384, CALG_SHA_512
    // 
    //                          全部支持可选如下:
    //                              CALG_MD2, CALG_MD4, CALG_MD5, CALG_SHA, CALG_SHA1, CALG_RSA_SIGN, CALG_DSS_SIGN
    //                          Windows XP及以上: CALG_NO_SIGN, CALG_RSA_KEYX, CALG_DES, CALG_3DES_112, CALG_3DES, 
    //                              CALG_DESX, CALG_RC2, CALG_RC4, CALG_SEAL, CALG_DH_SF, CALG_DH_EPHEM, 
    //                              CALG_AGREEDKEY_ANY, CALG_KEA_KEYX, CALG_HUGHES_MD5, CALG_SKIPJACK, CALG_TEK, 
    //                              CALG_CYLINK_MEK, CALG_SSL3_SHAMD5, CALG_SSL3_MASTER, CALG_SCHANNEL_MASTER_HASH, 
    //                              CALG_SCHANNEL_MAC_KEY, CALG_SCHANNEL_ENC_KEY, CALG_PCT1_MASTER, CALG_SSL2_MASTER, 
    //                              CALG_TLS1_MASTER, CALG_RC5, CALG_HMAC, CALG_TLS1PRF
    //                          Windows XP及以上: CALG_HASH_REPLACE_OWF, CALG_AES_128, CALG_AES_192, CALG_AES_256, CALG_AES
    //                          Windows SP2以上: CALG_SHA_256, CALG_SHA_384, CALG_SHA_512
    //                          Windows Vista及以上: CALG_ECDH, CALG_ECDH_EPHEM, CALG_ECMQV, CALG_ECDSA, CALG_NULLCIPHER
    //                          Windows 10 1607及以上: CALG_THIRDPARTY_KEY_EXCHANGE, CALG_THIRDPARTY_SIGNATURE
    //                              CALG_THIRDPARTY_CIPHER, CALG_THIRDPARTY_HASH
    // 
    // @ret: bool               操作是否成功
    bool Initialize(
        DWORD dwAlgorithm = CALG_MD5
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
    // @param: pData            数据
    // @param: ulSize           数据长度
    // @ret: bool               操作是否成功
    bool HashData(
        const void* pData, 
        unsigned long long ullSize
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
    // @param: ulSize           数据长度
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
    HCRYPTPROV              m_hProv = NULL;             // CSP 句柄
    HCRYPTHASH              m_hHash = NULL;             // 哈希对象
    DWORD                   m_dwAlgorithm = 0;          // 算法类型
    std::string             m_dataBuf;                  // 数据缓冲(用于文件读取)
};
