#pragma once

#include <Windows.h>
#include <WinHttp.h>
#include <string>
#include <vector>
#include <tchar.h>

#define WIN_HTTP_USER_AGENT LR"(Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36 Edg/122.0.0.0)"

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

typedef struct _WIN_HTTP_URL_COMPONENTS WIN_HTTP_URL_COMPONENTS, *LPWIN_HTTP_URL_COMPONENTS;

class CWinHttpResponse
{
public:
    CWinHttpResponse() :code(0){}
    std::string result;         //响应结果
    DWORD code;                 //响应状态码
};

class CWinHttp
{
public:

    CWinHttp();
    ~CWinHttp();

    //
    // @brief: 发送Get请求
    // @param: strUrl           URL链接
    // @param: strHeader        请求头
    // @ret: CWinHttpResponse   响应结果
    CWinHttpResponse Get(
        const _tstring& strUrl,
        const _tstring& strHeader = _T("Accept: */*\r\nContent-Type:application/json;charset=UTF-8")
    );

    //
    // @brief: 发送Post请求
    // @param: strUrl           URL链接
    // @param: strParam         请求参数
    // @param: strHeader        请求头
    // @ret: CWinHttpResponse   响应结果
    CWinHttpResponse Post(
        const _tstring& strUrl,
        const _tstring& strParam,
        const _tstring& strHeader = _T("Accept: */*\r\nContent-Type:application/json;charset=UTF-8")
    );

private:

    //
    // @brief: 执行请求
    // @param: strUrl           URL链接
    // @param: strMethod        请求方法
    // @param: strParam         请求参数
    // @param: strHeader        请求头
    // @ret: CWinHttpResponse   响应结果
    CWinHttpResponse _DoSendRequest(
        const _tstring& strUrl,
        const _tstring& strMethod,
        const std::string& strParam,
        const _tstring& strHeader
    );

    //
    // @brief: 分解链接
    // @param: strUrl           URL链接
    // @param: lpUci            分解输出缓冲
    // @ret: bool               分解成功与否
    bool _CrackUrl(
        const _tstring& strUrl,
        LPWIN_HTTP_URL_COMPONENTS lpUci
    );

    // 获取状态码
    //
    // @param: hRequest         请求句柄
    // @ret: DWORD              状态码
    DWORD _GetStatusCodes(
        HINTERNET hRequest
    );

    // 查询资源大小
    //
    // @param: hRequest         请求句柄
    // @ret: DWORD              状态码
    ULONGLONG _QueryContentLength(
        HINTERNET hRequest
    );

    // 发送请求
    // @param: hRequest         请求句柄
    // @param: lpszHeaders      请求头
    // @param: dwHeadersLength  请求头长度
    // @param: lpData           请求数据
    // @param: dwSize           请求数据长度
    // @ret: bool               请求成功与否
    bool _SendRequest(
        HINTERNET hRequest,
        LPCWSTR lpszHeaders,
        DWORD dwHeadersLength,
        LPVOID lpData,
        DWORD dwSize
    );

    // 读取响应数据
    // @param: hRequest         请求句柄
    // @param: strResponse      响应结果
    // @ret: bool               请求成功与否
    bool _ReadResponseData(
        HINTERNET hRequest,
        std::string& strResponse
    );

    //
    // @brief: 宽字节字符串转多字节字符串
    // @param: CodePage         代码页
    // @param: str              字符串
    // @ret: std::string        转换后的多字节字符串
    static std::string _WStrToMultiStr(
        UINT CodePage, 
        const std::wstring& str
    );

    //
    // @brief: 多字节字符串转宽字节字符串
    // @param: CodePage         代码页
    // @param: str              字符串
    // @ret: std::wstring       转换后的宽字节字符串
    static std::wstring _MultiStrToWStr(
        UINT CodePage, 
        const std::string& str
    );

    //
    // @brief: 字符串转UTF-8编码字符串
    // @param: str              字符串
    // @ret: std::string        转换后的UTF-8编码字符串
    static std::string _TStrToU8Str(
        const _tstring& str
    );

    //
    // @brief: 字符串转宽字节字符串
    // @param: str              字符串
    // @ret: std::wstring       转换后的宽字节字符串
    static std::wstring _TStrToWStr(
        const _tstring& str
    );
};