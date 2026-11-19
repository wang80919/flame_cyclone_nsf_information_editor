#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <tchar.h>

#pragma warning(disable:4200)

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CFileMapping
{
public:

    CFileMapping();
    ~CFileMapping();

    //删除拷贝构造与赋值重载
    CFileMapping(const CFileMapping&) = delete;
    CFileMapping& operator = (const CFileMapping&) = delete;

    //
    // @brief: 初始化一个文件映射
    // @param: strName    文件映射名
    // @param: dwSize     缓冲大小
    // @ret: bool
    bool Initialize(
        const _tstring& strName = _T(""),
        DWORD dwSize = 4096
    );

    //
    // @brief: 反初始化
    // @param: 无
    // @ret: void
    void Uninitialize();

    //
    // @brief: 是否已初始化
    // @param: 无
    // @ret: bool           初始化与否
    bool IsInitialized() const;

    //
    // @brief: 写入文件映射
    // @param: lpData       要写入数据
    // @param: dwSize       写入长度
    // @ret: bool           操作成功与否
    bool Write(LPCVOID lpData, DWORD dwSize);

    //
    // @brief: 读取文件映射
    // @param: lpData       读入的缓冲
    // @param: dwSize       缓冲长度
    // @param: lpBytesRead  成功读取的长度
    // @ret: bool           操作成功与否
    bool Read(LPVOID lpData, DWORD dwSize, LPDWORD lpBytesRead = nullptr);

    //
    // @brief: 获取文件映射容量
    // @ret: DWORD          文件映射容量
    DWORD GetCapacity() const;

    //
    // @brief: 获取数据大小
    // @ret: DWORD           数据大小
    DWORD GetSize();

    //
    // @brief: 获取原始数据指针
    // @ret: LPVOID           原始数据指针
    LPVOID GetDataPtr();

private:

    HANDLE m_hFileMapping;
    LPVOID m_hData;
    HANDLE m_hMutex;
    bool m_bInit;
};
