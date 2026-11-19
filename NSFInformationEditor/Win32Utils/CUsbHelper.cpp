#include "CUsbHelper.h"
#include <new>
#include <iostream>

#pragma comment (lib, "setupapi.lib")
#pragma comment (lib, "hid.lib")

CUsbHelper::CUsbHelper()
    :
    m_lpBufRead(nullptr),
    m_lpBufWrite(nullptr),
    m_hDevice(INVALID_HANDLE_VALUE),
    m_hReadEvent(nullptr)
{

}

CUsbHelper::~CUsbHelper()
{
    this->Close();
}

bool CUsbHelper::Open(WORD wVID, WORD wPID)
{
    this->Close();

    if (nullptr == m_hReadEvent)
    {
        m_hReadEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }

    if (nullptr == m_hWriteEvent)
    {
        m_hWriteEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }

    if (nullptr == m_hReadEvent || nullptr == m_hWriteEvent)
    {
        this->Close();
        return false;
    }

    GUID hidGuid = { 0 };
    HDEVINFO hardwareDeviceInfo = { 0 };
    SP_INTERFACE_DEVICE_DATA deviceInfoData = { 0 };
    PSP_INTERFACE_DEVICE_DETAIL_DATA functionClassDeviceData = NULL;
    ULONG predictedLength = 0;
    ULONG requiredLength = 0;
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    bool bOpenDevice = false;

    deviceInfoData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
    (void)::HidD_GetHidGuid(&hidGuid);

    hardwareDeviceInfo = ::SetupDiGetClassDevs(&hidGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
    if (INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        return false;
    }

    for (DWORD dwIndex = 0; ; dwIndex++)
    {
        BOOL bRet = ::SetupDiEnumDeviceInterfaces(hardwareDeviceInfo, 0, &hidGuid, dwIndex, &deviceInfoData);
        if (!bRet && (ERROR_NO_MORE_ITEMS == ::GetLastError()))
        {
            break;
        }

        if (!bRet)
        {
            continue;
        }

        if (!::SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, &deviceInfoData, NULL, 0, &requiredLength, NULL))
        {
            if (ERROR_INSUFFICIENT_BUFFER != ::GetLastError())
            {
                continue;
            }
        }

        predictedLength = requiredLength;
        functionClassDeviceData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)new (std::nothrow) BYTE[predictedLength];
        if (nullptr == functionClassDeviceData)
        {
            continue;
        }

        do
        {
            functionClassDeviceData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
            if (!SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, &deviceInfoData, functionClassDeviceData, predictedLength, &requiredLength, NULL))
            {
                break;
            }

            hDevice = CreateFile(functionClassDeviceData->DevicePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);//   倒数第二个参数可以指定为异步FILE_FLAG_OVERLAPPED   0为同步
            if (INVALID_HANDLE_VALUE == hDevice)
            {
                break;
            }

            HIDD_ATTRIBUTES attri = { 0 };
            if (!::HidD_GetAttributes(hDevice, &attri))
            {
                break;
            }

            //printf("%X %X\r\n", attri.VendorID, attri.ProductID);
            if ((attri.VendorID == wVID) && (attri.ProductID == wPID))
            {
                m_hDevice = hDevice;
                bOpenDevice = true;
                break;
            }

            ::CloseHandle(hDevice);
            hDevice = INVALID_HANDLE_VALUE;

        } while (FALSE);

        delete[] functionClassDeviceData;
        functionClassDeviceData = nullptr;

        if (bOpenDevice)
        {
            break;
        }
    }

    ::SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
    return bOpenDevice;
}

bool CUsbHelper::IsOpen()
{
    return INVALID_HANDLE_VALUE != m_hDevice;
}

void CUsbHelper::Close()
{
    if (m_hReadEvent)
    {
        ::SetEvent(m_hReadEvent);
        ::CloseHandle(m_hReadEvent);
        m_hReadEvent = nullptr;
    }

    if (m_hWriteEvent)
    {
        ::SetEvent(m_hWriteEvent);
        ::CloseHandle(m_hWriteEvent);
        m_hWriteEvent = nullptr;
    }

    if (INVALID_HANDLE_VALUE != m_hDevice)
    {
        ::CancelIoEx(m_hDevice, nullptr);
        ::CloseHandle(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
    }

    if (nullptr != m_lpBufRead)
    {
        delete[] m_lpBufRead;
        m_lpBufRead = nullptr;
    }

    if (nullptr != m_lpBufWrite)
    {
        delete[] m_lpBufWrite;
        m_lpBufWrite = nullptr;
    }
}

bool CUsbHelper::GetHidCaps(PHIDP_CAPS pCaps)
{
    if (INVALID_HANDLE_VALUE == m_hDevice || nullptr == pCaps)
    {
        return false;
    }

    PHIDP_PREPARSED_DATA PreparsedData = { 0 };

    if (!::HidD_GetPreparsedData(m_hDevice, &PreparsedData))
    {
        return false;
    }

    NTSTATUS ntStatus = ::HidP_GetCaps(PreparsedData, pCaps);
    if (HIDP_STATUS_SUCCESS != ntStatus)
    {
        return false;
    }

    return true;
}

bool CUsbHelper::Read(LPVOID lpBuffer, USHORT uNumberOfBytesToRead, USHORT& uBytesRead, DWORD dwTimeOut/* = INFINITE*/)
{
    if (INVALID_HANDLE_VALUE == m_hDevice)
    {
        return false;
    }

    HIDP_CAPS Capabilities = { 0 };
    if (!this->GetHidCaps(&Capabilities))
    {
        return false;
    }

    if (uNumberOfBytesToRead != Capabilities.InputReportByteLength - 1)
    {
        return false;
    }

    //分配读缓冲
    if (nullptr == m_lpBufRead)
    {
        m_lpBufRead = new (std::nothrow) BYTE[USB_READ_BUFFER_SIZE];
        if (nullptr == m_lpBufRead)
        {
            return false;
        }
    }

    ::memset(m_lpBufRead, 0x00, USB_READ_BUFFER_SIZE);

    OVERLAPPED ov = { 0 };
    ov.hEvent = m_hReadEvent;

    m_bReadCancel = false;
    BOOL bRes = ::ReadFile(m_hDevice, m_lpBufRead, Capabilities.OutputReportByteLength, nullptr, &ov);

    //重叠操作中
    if (!bRes && ERROR_IO_PENDING == ::GetLastError())
    {
        DWORD waitRes = WaitForSingleObject(ov.hEvent, dwTimeOut);
        if (WAIT_OBJECT_0 == waitRes && !m_bReadCancel)
        {
            bRes = true;
        }
    }

    if (bRes && ov.InternalHigh == Capabilities.OutputReportByteLength)
    {
        uBytesRead = (USHORT)(ov.InternalHigh - 1);
        memcpy_s(lpBuffer, uNumberOfBytesToRead, m_lpBufRead + 1, uBytesRead);
    }

    return bRes;
}

bool CUsbHelper::Write(LPVOID lpBuffer, USHORT uNumberOfBytesToWrite, USHORT& uBytesWrite, DWORD dwTimeOut/* = INFINITE*/)
{
    if (INVALID_HANDLE_VALUE == m_hDevice)
    {
        return false;
    }

    HIDP_CAPS Capabilities = { 0 };
    if (!this->GetHidCaps(&Capabilities))
    {
        return false;
    }

    if (uNumberOfBytesToWrite != Capabilities.InputReportByteLength - 1)
    {
        return false;
    }

    //分配写缓冲
    if (nullptr == m_lpBufWrite)
    {
        m_lpBufWrite = new (std::nothrow) BYTE[USB_WRITE_BUFFER_SIZE];
        if (nullptr == m_lpBufWrite)
        {
            return false;
        }
    }

    ::memset(m_lpBufWrite, 0x00, USB_WRITE_BUFFER_SIZE);
    ::memcpy_s(m_lpBufWrite + 1, uNumberOfBytesToWrite, (LPBYTE)lpBuffer, uNumberOfBytesToWrite);

    OVERLAPPED ov = { 0 };
    ov.hEvent = m_hWriteEvent;

    m_bWriteCancel = false;
    BOOL bRes = ::WriteFile(m_hDevice, m_lpBufWrite, Capabilities.OutputReportByteLength, nullptr, &ov);

    //重叠操作中
    if (!bRes && ERROR_IO_PENDING == ::GetLastError())
    {
        DWORD waitRes = WaitForSingleObject(ov.hEvent, dwTimeOut);
        if (WAIT_OBJECT_0 == waitRes && !m_bWriteCancel)
        {
            bRes = true;
        }
    }

    if (bRes && ov.InternalHigh == Capabilities.OutputReportByteLength)
    {
        uBytesWrite = (USHORT)(ov.InternalHigh - 1);
    }


    return bRes;
}

void CUsbHelper::CancelRead()
{
    if (m_hReadEvent)
    {
        m_bReadCancel = true;
        ::SetEvent(m_hReadEvent);
    }
}

void CUsbHelper::CancelWrite()
{
    if (m_hWriteEvent)
    {
        m_bWriteCancel = true;
        ::SetEvent(m_hWriteEvent);
    }
}