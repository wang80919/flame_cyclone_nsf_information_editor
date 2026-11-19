#include "CWinHttp.h"
#include <schannel.h>

#pragma comment(lib, "winhttp.lib")

#define HTTP_READ_BLOCK_SIZE                     (1024 * 8)  // 读取块大小

#define INTERNET_MAX_HOST_NAME_LENGTH   256
#define INTERNET_MAX_USER_NAME_LENGTH   128
#define INTERNET_MAX_PASSWORD_LENGTH    128
#define INTERNET_MAX_PORT_NUMBER_LENGTH 5           // INTERNET_PORT is unsigned short
#define INTERNET_MAX_PORT_NUMBER_VALUE  65535       // maximum unsigned short value
#define INTERNET_MAX_PATH_LENGTH        2048
#define INTERNET_MAX_SCHEME_LENGTH      32          // longest protocol name length
#define INTERNET_MAX_URL_LENGTH         (INTERNET_MAX_SCHEME_LENGTH \
                                        + sizeof("://") \
                                        + INTERNET_MAX_PATH_LENGTH)

typedef struct _WIN_HTTP_URL_COMPONENTS
{
    WCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
    WCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
    WCHAR szUserName[INTERNET_MAX_USER_NAME_LENGTH];
    WCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
    WCHAR szUrlPath[INTERNET_MAX_URL_LENGTH];
    WCHAR szExtraInfo[MAX_PATH];
    URL_COMPONENTS uc = { 0 };

    _WIN_HTTP_URL_COMPONENTS()
    {
        ZeroMemory(this, sizeof(*this));
        this->uc.dwStructSize = sizeof(this->uc);
        this->uc.lpszUrlPath = this->szUrlPath;
        this->uc.dwUrlPathLength = _countof(this->szUrlPath);
        this->uc.lpszScheme = this->szScheme;
        this->uc.dwSchemeLength = _countof(this->szScheme);
        this->uc.lpszHostName = this->szHostName;
        this->uc.dwHostNameLength = _countof(this->szHostName);
        this->uc.lpszUserName = this->szUserName;
        this->uc.dwUserNameLength = _countof(this->szUserName);
        this->uc.lpszPassword = this->szPassword;
        this->uc.dwPasswordLength = _countof(this->szPassword);
        this->uc.lpszExtraInfo = this->szExtraInfo;
        this->uc.dwExtraInfoLength = _countof(this->szExtraInfo);
    }
}WIN_HTTP_URL_COMPONENTS;

CWinHttp::CWinHttp()
{

}

CWinHttp::~CWinHttp()
{

}

CWinHttpResponse CWinHttp::Get(
    const _tstring& strUrl,
    const _tstring& strHeader/* = _T("Content-Type:application/json;charset=UTF-8")*/
)
{
    return _DoSendRequest(strUrl, _T("GET"), std::string(), strHeader);
}

CWinHttpResponse CWinHttp::Post(
    const _tstring& strUrl,
    const _tstring& strParam,
    const _tstring& strHeader/* = _T("Content-Type:application/json;charset=UTF-8")*/
)
{
    return _DoSendRequest(strUrl, _T("POST"), _TStrToU8Str(strParam), strHeader);
}

CWinHttpResponse CWinHttp::_DoSendRequest(
    const _tstring& strUrl,
    const _tstring& strMethod/* = _T("GET")*/,
    const std::string& strParam,
    const _tstring& strHeader
)
{
    CWinHttpResponse responseResult;
    WIN_HTTP_URL_COMPONENTS urlParts;
    ULONGLONG ullContentLength = 0;
    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    bool bSuccess = false;

    // 分解URL
    if (!_CrackUrl(strUrl, &urlParts))
    {
        return responseResult;
    }

    do
    {
        // 初始化应用程序对 WinINet 函数的使用
        hSession = ::WinHttpOpen(
            WIN_HTTP_USER_AGENT, //指向字符串变量的指针，该变量包含调用 WinHTTP 函数的应用程序或实体的名称
            WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, //所需的访问类型
            WINHTTP_NO_PROXY_NAME, //代理访问时要使用的代理服务器的名称
            WINHTTP_NO_PROXY_BYPASS, //代理访问时要使用的代理服务器的密码
            WINHTTP_FLAG_SECURE_DEFAULTS // 指示影响此函数行为的各种选项的标志
        );

        if (NULL == hSession)
        {
            break;
        }

        // 打开给定站点的 HTTP 会话
        hConnect = ::WinHttpConnect(
            hSession,//由先前调用 WinHttpOpen 返回的有效 HINTERNETWinHTTP 会话句柄
            urlParts.szHostName, //HTTP 服务器的主机名
            urlParts.uc.nPort, //建立连接的服务器上的 TCP/IP 端口
            0 //保留参数, 必须为0
        );

        if (NULL == hConnect)
        {
            break;
        }

        // 创建 HTTPS / HTTP 请求句柄
        DWORD dwFlags = (INTERNET_SCHEME_HTTPS == urlParts.uc.nScheme) ? WINHTTP_FLAG_SECURE : 0;
        hRequest = ::WinHttpOpenRequest(
            hConnect, //WinHttpConnect 返回的 HTTP 会话的 HINTERNET 连接句柄
            _TStrToWStr(strMethod).c_str(), //请求的 HTTP 谓词
            urlParts.szUrlPath, //指定 HTTP 谓词的目标资源名称
            NULL, //HTTP 版本的字符串的指针
            WINHTTP_NO_REFERER,//指定从中获取 请求 pwszObjectName 中的 URL 的文档的 URL
            WINHTTP_DEFAULT_ACCEPT_TYPES, //指定客户端接受的媒体类型
            dwFlags   //Internet 标志值
        );
        if (NULL == hRequest)
        {
            break;
        }

        // 设置安全标志
        DWORD dwSecurityFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
            SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
            SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
            SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
        if (!::WinHttpSetOption(
            hRequest, 
            WINHTTP_OPTION_SECURITY_FLAGS, 
            &dwSecurityFlags,
            sizeof(dwSecurityFlags)
        ))
        {
            break;
        }

        // 设置客户端证书上下文
        if (!::WinHttpSetOption(
            hRequest, 
            WINHTTP_OPTION_CLIENT_CERT_CONTEXT, 
            WINHTTP_NO_CLIENT_CERT_CONTEXT, 
            0
        ))
        {
            break;
        }

        std::wstring wstrHeader = _TStrToWStr(strHeader);
        // 添加请求头
        ::WinHttpAddRequestHeaders(
            hRequest,
            wstrHeader.data(),
            (DWORD)wstrHeader.size(),
            WINHTTP_ADDREQ_FLAG_ADD
        );

        // 发送请求
        if (!_SendRequest(
            hRequest,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            (LPVOID)strParam.data(),
            (DWORD)strParam.size()
        ))
        {
            break;
        }

        // 等待接收 WinHttpSendRequest 发起的 HTTP 请求的响应
        if (!::WinHttpReceiveResponse(hRequest, NULL))
        {
            break;
        }

        // 获取状态码
        responseResult.code = _GetStatusCodes(hRequest);
        if (responseResult.code < 200 || responseResult.code >= 300)
        {
            //TODO
        }

        // 获取内容大小
        ullContentLength = _QueryContentLength(hRequest);

        // 读取响应内容
        if (!_ReadResponseData(hRequest, responseResult.result))
        {
            break;
        }

        bSuccess = true;

    } while (false);

    // 释放资源
    if (hRequest)
    {
        ::WinHttpCloseHandle(hRequest);
    }

    if (hConnect)
    {
        ::WinHttpCloseHandle(hConnect);
    }

    if (hSession)
    {
        ::WinHttpCloseHandle(hSession);
    }

    return responseResult;
}

bool CWinHttp::_CrackUrl(
    const _tstring& strUrl,
    LPWIN_HTTP_URL_COMPONENTS lpUci
)
{
    // 将 URL 分解到其组件部件中
    if (!::WinHttpCrackUrl(_TStrToWStr(strUrl).c_str(), 0, 0, &lpUci->uc))
    {
        return false;
    }

    if (0 < ::wcslen(lpUci->uc.lpszExtraInfo))
    {
        ::wcscat_s(lpUci->uc.lpszUrlPath, _countof(lpUci->szUrlPath), lpUci->uc.lpszExtraInfo);
    }

    return true;
}

DWORD CWinHttp::_GetStatusCodes(
    HINTERNET hRequest
)
{
    DWORD dwRespCode = 0;
    DWORD dwBufferLength = sizeof(dwRespCode);

    // 查询请求状态码
    if (!::WinHttpQueryHeaders(
        hRequest, //WinHttpOpenRequest 返回的 HINTERNET 请求句柄
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, //指定“查询信息标志”页上列出的属性标志和修饰符标志的组合
        WINHTTP_HEADER_NAME_BY_INDEX, //标头名称
        &dwRespCode, //接收信息的缓冲区
        &dwBufferLength, //数据缓冲区的长度
        NULL    //从零开始的标头索引的指针，用于枚举具有相同名称的多个标头
    ))
    {
        return dwRespCode;
    }

    return dwRespCode;
}

ULONGLONG CWinHttp::_QueryContentLength(
    HINTERNET hRequest
)
{
    ULONGLONG ullContentLength = 0;
    DWORD dwBufferLength = sizeof(ullContentLength);

    // 查询资源大小
    if (!::WinHttpQueryHeaders(
        hRequest, 
        WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER64,
        WINHTTP_HEADER_NAME_BY_INDEX, //标头名称
        &ullContentLength, //接收信息的缓冲区
        &dwBufferLength, //数据缓冲区的长度
        NULL    //从零开始的标头索引的指针，用于枚举具有相同名称的多个标头
    ))
    {
        return 0;
    }

    return ullContentLength;
}

bool CWinHttp::_SendRequest(
    HINTERNET hRequest,
    LPCWSTR lpszHeaders,
    DWORD dwHeadersLength,
    LPVOID lpData,
    DWORD dwSize
)
{
    // 将指定的请求发送到 HTTP 服务器
    if (::WinHttpSendRequest(
        hRequest, //WinHttpOpenRequest 返回的 HINTERNET 句柄
        lpszHeaders,//要追加到请求的其他标头
        dwHeadersLength,//附加标头的长度（以字符为单位）
        lpData,//请求标头之后发送的任何可选数据
        dwSize, //可选数据的长度（以字节为单位）
        dwSize, //发送的总数据的长度
        NULL
    ))
    {
        return true;
    }

    // 安全 HTTP 服务器需要客户端证书
    if (ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED != ::GetLastError())
    {
        return false;
    }

    /*SecPkgContext_IssuerListInfoEx* pIssuerList = NULL;
    DWORD dwBufferSize = sizeof(SecPkgContext_IssuerListInfoEx*);

    // 获取INTERNET_OPTION_SECURITY_FLAGS标志 
    if (::WinHttpQueryOption(
        hRequest, //要查询信息的 HINTERNET 句柄
        WINHTTP_OPTION_CLIENT_CERT_ISSUER_LIST, //查询的 Internet 选项
        &pIssuerList, //接收选项设置的缓冲区
        &dwBufferSize //接收选项设置的缓冲区的长度
    ))
    {
        // 使用 pIssuerList 进行证书存储筛选
        ::GlobalFree(pIssuerList);
        return false;
    }*/

    // 没有客户端证书
    if (!::WinHttpSetOption(
        hRequest,
        WINHTTP_OPTION_CLIENT_CERT_CONTEXT,
        WINHTTP_NO_CLIENT_CERT_CONTEXT,
        0
    ))
    {
        return false;
    }

    // 再次发送请求
    if (::WinHttpSendRequest(
        hRequest, //WinHttpOpenRequest 返回的 HINTERNET 句柄
        lpszHeaders,//要追加到请求的其他标头
        dwHeadersLength,//附加标头的长度（以字符为单位）
        lpData,//请求标头之后发送的任何可选数据
        dwSize, //可选数据的长度（以字节为单位）
        dwSize, //发送的总数据的长度
        NULL
    ))
    {
        return true;
    }

    return false;
}

bool CWinHttp::_ReadResponseData(
    HINTERNET hRequest,
    std::string& strResponse
)
{
    std::string strBuffer(HTTP_READ_BLOCK_SIZE, 0);
    DWORD dwRead = 0;
    DWORD dwAvailable = 0;
    bool bSuccess = false;

    while (true)
    {
        // 检查可用数据
        dwAvailable = 0;
        if (!::WinHttpQueryDataAvailable(hRequest, &dwAvailable))
        {
            break;
        }

        // 没有更多可用数据
        if (!dwAvailable)
        {
            bSuccess = true;
            break;
        }

        // 读取数据
        strBuffer.resize(dwAvailable);
        if (!::WinHttpReadData(hRequest, &strBuffer[0], dwAvailable, &dwRead))
        {
            break;
        }

        // 传输已完成
        if (0 == dwRead)
        {
            bSuccess = true;
            break;
        }

        // 追加响应数据
        strBuffer.resize(dwRead);
        strResponse += strBuffer;
    };

    return bSuccess;
}

std::string CWinHttp::_WStrToMultiStr(
    UINT CodePage, 
    const std::wstring& str
)
{
    //计算缓冲区所需的字节长度
    int cbMultiByte = ::WideCharToMultiByte(CodePage, 0, str.c_str(), -1, NULL, 0, NULL, 0);
    std::string strResult(cbMultiByte, 0);

    //成功则返回写入到指示的缓冲区的字节数
    size_t nConverted = ::WideCharToMultiByte(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size(), NULL, NULL);

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
}

std::wstring CWinHttp::_MultiStrToWStr(
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

std::string CWinHttp::_TStrToU8Str(
    const _tstring& str
)
{
#ifdef _UNICODE
    return _WStrToMultiStr(CP_UTF8, str);
#else
    return _WStrToMultiStr(CP_UTF8, _MultiStrToWStr(CP_ACP, str));
#endif
}

std::wstring CWinHttp::_TStrToWStr(
    const _tstring& str
)
{
#ifdef _UNICODE
    return str;
#else
    return _MultiStrToWStr(CP_ACP, str);
#endif
}
