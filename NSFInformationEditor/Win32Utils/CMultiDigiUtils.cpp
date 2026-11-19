#include "CMultiDigiUtils.h"
#include <Hidsdi.h>
#include <setupapi.h>
#include <tchar.h>

#pragma comment(lib, "Hid.lib")
#pragma comment(lib, "SetupAPI.lib")

std::vector<_tstring> CMultiDigiUtils::GetTouchDevPath()
{
    GUID hidGuid{};
    ::HidD_GetHidGuid(&hidGuid);

    std::vector<_tstring> vResult;

    HDEVINFO infoSet = ::SetupDiGetClassDevs(&hidGuid, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
    if (INVALID_HANDLE_VALUE == infoSet)
    {
        return vResult;
    }

    SP_DEVICE_INTERFACE_DATA interfaceData = { 0 };
    interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (int nIndex = 0; ::SetupDiEnumDeviceInterfaces(infoSet, nullptr, &hidGuid, nIndex, &interfaceData); ++nIndex)
    {
        DWORD detailReqSize = 0;
        if (!SetupDiGetDeviceInterfaceDetail(infoSet, &interfaceData, nullptr, 0, &detailReqSize, nullptr) && ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
        {
            PSP_DEVICE_INTERFACE_DETAIL_DATA detailPtr = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(new (std::nothrow) uint8_t[detailReqSize]);
            if (nullptr == detailPtr)
            {
                continue;
            }

            detailPtr->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            if (::SetupDiGetDeviceInterfaceDetail(infoSet, &interfaceData, detailPtr, detailReqSize, &detailReqSize, nullptr))
            {
                HANDLE devPtr = INVALID_HANDLE_VALUE;
                devPtr = CreateFile(detailPtr->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
                if (devPtr != INVALID_HANDLE_VALUE)
                {
                    PHIDP_PREPARSED_DATA preparsedPtr = nullptr;
                    if (::HidD_GetPreparsedData(devPtr, &preparsedPtr))
                    {
                        HIDP_CAPS caps{};
                        if (HIDP_STATUS_SUCCESS == HidP_GetCaps(preparsedPtr, &caps))
                        {
                            if (caps.UsagePage == HID_USAGE_PAGE_DIGITIZER && caps.Usage == HID_USAGE_DIGITIZER_TOUCH_SCREEN)
                            {
                                vResult.push_back(detailPtr->DevicePath);
                            }
                        }

                        ::HidD_FreePreparsedData(preparsedPtr);
                    }

                    ::CloseHandle(devPtr);
                }
            }

            delete[] detailPtr;
        }
    }

    ::SetupDiDestroyDeviceInfoList(infoSet);

    return vResult;
}

std::set<_tstring> CMultiDigiUtils::GetDisplayDevPath()
{
    std::set<_tstring> vResult;
    UINT32 pathCount, modeCount;
    if (::GetDisplayConfigBufferSizes(QDC_ALL_PATHS, &pathCount, &modeCount) == ERROR_SUCCESS)
    {
        DISPLAYCONFIG_PATH_INFO* pathPtr = new (std::nothrow) DISPLAYCONFIG_PATH_INFO[pathCount]{};
        DISPLAYCONFIG_MODE_INFO* modePtr = new (std::nothrow) DISPLAYCONFIG_MODE_INFO[modeCount]{};
        if (pathPtr && modePtr)
        {
            if (::QueryDisplayConfig(QDC_ALL_PATHS, &pathCount, pathPtr, &modeCount, modePtr, nullptr) == ERROR_SUCCESS)
            {
                for (UINT32 i = 0; i < pathCount; i++)
                {
                    const DISPLAYCONFIG_PATH_INFO& path = pathPtr[i];
                    const LUID& dstAdapterId = path.targetInfo.adapterId;
                    const UINT& dstId = path.targetInfo.id;

                    DISPLAYCONFIG_TARGET_DEVICE_NAME targetName{};
                    targetName.header.size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
                    targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
                    targetName.header.adapterId = dstAdapterId;
                    targetName.header.id = dstId;
                    if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&targetName.header))
                    {
                        bool hasPath = ::wcslen(targetName.monitorDevicePath); //有路径则说明设备在硬件层面上已连接
                        if (hasPath)
                        {
                            vResult.insert(targetName.monitorDevicePath);
                        }
                    }

                }
            }
        }

        if (pathPtr)
        {
            delete[] pathPtr;
        }

        if (modePtr)
        {
            delete[] modePtr;
        }
    }

    return vResult;
}

bool CMultiDigiUtils::BroadcastMappingChange()
{
    DWORD_PTR dwResult = 0;
    //WM_SETTINGCHANGE = 0x1A
    if (SendMessageTimeout(HWND_BROADCAST, 0x1A, NULL, LPARAM(_T("TabletPCDigitizerMappingChanged")), SMTO_ABORTIFHUNG, 200, &dwResult))
    {
        return true;
    }

    return ::GetLastError() == ERROR_SUCCESS;
}

bool CMultiDigiUtils::SetDisplayMapping(const _tstring& touchPath, const _tstring& displayPath)
{
    HANDLE touch = GetTouchDeviceHandle(touchPath);
    HMONITOR display = GetDisplayDeviceHandle(displayPath);

    if (touch && display)
    {
        return SetDisplayMapping(touch, display);
    }

    return false;
}

typedef WINUSERAPI LRESULT(WINAPI* NtUserSetDisplayMapping) (_In_ HANDLE touch, _In_ HMONITOR display);
bool CMultiDigiUtils::SetDisplayMapping(HANDLE touch, HMONITOR display)
{
    LRESULT ret = 0;
    NtUserSetDisplayMapping pFunNtUserSetDisplayMapping = reinterpret_cast<NtUserSetDisplayMapping>(GetProcAddress(GetModuleHandle(_T("user32")), MAKEINTRESOURCEA(2532)));
    if (nullptr != pFunNtUserSetDisplayMapping)
    {
        ret = pFunNtUserSetDisplayMapping(touch, display);
    }
    return ret == 1;
}

HANDLE CMultiDigiUtils::GetTouchDeviceHandle(const _tstring& pDevPath)
{
    HANDLE hDevice = nullptr;
    UINT count = 0;

    if (0 != ::GetRawInputDeviceList(nullptr, &count, sizeof(RAWINPUTDEVICELIST)))
    {
        return nullptr;
    }

    PRAWINPUTDEVICELIST pRawInputDeviceList = new (std::nothrow) RAWINPUTDEVICELIST[count];
    if (nullptr == pRawInputDeviceList)
    {
        return nullptr;
    }

    if (::GetRawInputDeviceList(pRawInputDeviceList, &count, sizeof(RAWINPUTDEVICELIST)))
    {
        for (UINT i = 0; i < count; i++)
        {
            if (RIM_TYPEHID != pRawInputDeviceList[i].dwType)
            {
                continue;
            }

            UINT length = MAX_PATH;
            wchar_t path[MAX_PATH]{};
            if (::GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, path, &length))
            {
                if (0 == _tcsicmp(pDevPath.c_str(), path))
                {
                    hDevice = pRawInputDeviceList[i].hDevice;
                    break;
                }
            }
        }
    }

    delete[]pRawInputDeviceList;

    return hDevice;
}

typedef struct _ENUM_MONITOR_INFO
{
    _tstring strPath;
    HANDLE hDev;

    _ENUM_MONITOR_INFO()
        :hDev(nullptr)
    {

    }
}ENUM_MONITOR_INFO, * PENUM_MONITOR_INFO;

HMONITOR CMultiDigiUtils::GetDisplayDeviceHandle(const _tstring& pDevPath)
{
    ENUM_MONITOR_INFO info;
    info.strPath = pDevPath;
    ::EnumDisplayMonitors(nullptr, nullptr,
        [](HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM param) {

            UNREFERENCED_PARAMETER(hdc);
            UNREFERENCED_PARAMETER(rect);

            PENUM_MONITOR_INFO pInfo = (PENUM_MONITOR_INFO)param;
            MONITORINFOEX info{};
            info.cbSize = sizeof(MONITORINFOEX);

            if (!::GetMonitorInfo(monitor, &info))
            {
                return TRUE;
            }

            int i = 0;
            while (true)
            {
                DISPLAY_DEVICE dev{};
                dev.cb = sizeof(DISPLAY_DEVICE);
                if (::EnumDisplayDevices(info.szDevice, i++, &dev, EDD_GET_DEVICE_INTERFACE_NAME))
                {
                    if (0 == _tcsicmp(pInfo->strPath.c_str(), dev.DeviceID))
                    {
                        pInfo->hDev = monitor;
                        return FALSE;
                    }
                }
                else
                {
                    break;
                }
            }

            return TRUE;
        }
    , (LPARAM)&info);

    return static_cast<HMONITOR>(info.hDev);
}