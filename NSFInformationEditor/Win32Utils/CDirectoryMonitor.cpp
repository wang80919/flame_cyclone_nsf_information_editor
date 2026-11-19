#include "CDirectoryMonitor.h"
#include "CStrUtils.h"

CDirectoryMonitor::CDirectoryMonitor()
    :
    m_hDirectory(INVALID_HANDLE_VALUE),
    m_bStop(false)
{

}

CDirectoryMonitor::~CDirectoryMonitor()
{
    Close();
}

bool CDirectoryMonitor::Start(_tstring strDirectory, DirectoryActionCallback cb/* = nullptr*/)
{
    m_Callback = cb;
    Close();

    if (strDirectory.empty())
    {
        return false;
    }

    m_strDirectory = strDirectory;

    //去除目录末尾反斜杠
    if (_T('\\') == m_strDirectory.back())
    {
        m_strDirectory.resize(m_strDirectory.size() - 1);
    }

    //打开目录(需要加上反斜杠)
    m_hDirectory = ::CreateFile(
        (m_strDirectory + _T('\\')).c_str(),
        GENERIC_READ | GENERIC_WRITE | FILE_LIST_DIRECTORY,//请求对文件或设备的访问权限
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,//请求的文件或设备的共享模式
        NULL,//指向 SECURITY_ATTRIBUTES 结构的指针
        OPEN_EXISTING,//存在或不存在的文件或设备执行的操作
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,//包含可用文件属性的任意组合
        NULL
    );

    if (INVALID_HANDLE_VALUE == m_hDirectory)
    {
        return false;
    }

    //启动任务线程
    m_taskThread = std::move(std::thread([this]() {
        _DoTask();
    }));

    return true;
}

bool CDirectoryMonitor::_DoTask()
{
    std::string strBuffer(8192, 0);
    DWORD nBytesReturned = 0;

    while (!m_bStop)
    {
        ZeroMemory(&strBuffer[0], strBuffer.size());
        if (::ReadDirectoryChangesW(m_hDirectory,
            &strBuffer[0],//读取结果的 DWORD 对齐格式化缓冲区
            (DWORD)strBuffer.size(),//缓冲区的大小（以字节为单位）
            TRUE,//是否函数监视位于指定目录的根目录树
            FILE_NOTIFY_CHANGE_FILE_NAME |    //监视文件创建, 重命名、创建或删除
            FILE_NOTIFY_CHANGE_DIR_NAME |    //监视目录创建, 重命名、创建或删除
            FILE_NOTIFY_CHANGE_ATTRIBUTES |
            FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            &nBytesReturned,//接收传输到 lpBuffer 参数的字节数
            NULL,
            NULL
        ))
        {
            PFILE_NOTIFY_INFORMATION pNotify = (FILE_NOTIFY_INFORMATION*)&strBuffer[0];
            do
            {
                //转换路径, 这里的 pNotify->FileNameLength 是以字节为单位的, 注意区分
                std::wstring wstrPath(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));
                _tstring strFilePath = _WStrToTStr(wstrPath);

                //执行回调
                if (m_Callback)
                {
                    m_Callback(m_strDirectory, strFilePath, pNotify->Action);
                }

                pNotify = (PFILE_NOTIFY_INFORMATION)((LPBYTE)pNotify + pNotify->NextEntryOffset);

            } while (0 != pNotify->NextEntryOffset);
        }
    }

    return true;
}

void CDirectoryMonitor::Close()
{
    if (m_taskThread.joinable())
    {
        m_bStop = true;
        ::CancelIoEx(m_hDirectory, NULL);
        m_taskThread.join();
    }

    if (INVALID_HANDLE_VALUE != m_hDirectory)
    {
        ::CloseHandle(m_hDirectory);
        m_hDirectory = INVALID_HANDLE_VALUE;
    }

    m_bStop = false;
}

std::string CDirectoryMonitor::_WStrToMultiStr(UINT CodePage, const std::wstring& str)
{
    //计算缓冲区所需的字节长度
    int cbMultiByte = ::WideCharToMultiByte(CodePage, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
    std::string strResult(cbMultiByte, 0);

    //成功则返回写入到指示的缓冲区的字节数
    size_t nConverted = ::WideCharToMultiByte(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size(), NULL, NULL);

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
}

_tstring CDirectoryMonitor::_WStrToTStr(const std::wstring& str)
{
#ifdef _UNICODE
    return str;
#else
    return _WStrToMultiStr(CP_ACP, str);
#endif
}
