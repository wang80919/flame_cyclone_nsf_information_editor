#pragma once

#include <windows.h>
#include <atomic>

extern "C" {
#include <hidsdi.h> 
#include <setupapi.h>  
}

#define USB_READ_BUFFER_SIZE             (1024 * 4)
#define USB_WRITE_BUFFER_SIZE            (1024 * 4)

class CUsbHelper
{
public:

    CUsbHelper();

    ~CUsbHelper();

    CUsbHelper operator = (const CUsbHelper& r) = delete;

    //打开设备
    bool Open(WORD wVID, WORD wPID);

    //是否打开
    bool IsOpen();

    //关闭设备
    void Close();

    //读取数据
    bool Read(LPVOID lpBuffer, USHORT nNumberOfuint8_tsToRead, USHORT& dwuint8_tsRead, DWORD dwTimeOut = INFINITE);

    //取消读取
    void CancelRead();

    //写入数据
    bool Write(LPVOID lpBuffer, USHORT nNumberOfuint8_tsToWrite, USHORT& dwuint8_tsWrite, DWORD dwTimeOut = INFINITE);

    //取消写入
    void CancelWrite();

private:

    //获取信息集合
    bool GetHidCaps(PHIDP_CAPS pCaps);

private:

    uint8_t* m_lpBufRead = nullptr;
    uint8_t* m_lpBufWrite = nullptr;
    HANDLE m_hReadEvent = nullptr;
    HANDLE m_hWriteEvent = nullptr;
    std::atomic<bool> m_bReadCancel = false;
    std::atomic<bool> m_bWriteCancel = false;
    HANDLE m_hDevice = INVALID_HANDLE_VALUE;
};

