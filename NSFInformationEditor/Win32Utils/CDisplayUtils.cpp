#include "CDisplayUtils.h"

#pragma warning(disable:4100)
#pragma comment(lib, "Advapi32.lib")

#define SCREEN_AUTOROTATION     R"(SOFTWARE\Microsoft\Windows\CurrentVersion\AutoRotation)"
#define HKLM                    HKEY_LOCAL_MACHINE

namespace CDisplayUtils
{
    MONITOR_DESC_LIST CDisplayUtils::GetDisplayMonitorInfos()
    {
        MONITOR_DESC_LIST vMonitorInfos;

        if (!::EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor,
            HDC hDC,
            LPRECT lpRect,
            LPARAM lparam) -> BOOL {
                MONITORINFOEX monitorInfo;
                monitorInfo.cbSize = sizeof(MONITORINFOEX);
                ::GetMonitorInfo(hMonitor, &monitorInfo);

                MONITOR_DESC_INFO info;
                info.hMonitor = hMonitor;
                info.rect = *lpRect;
                info.bPrimary = false;
                info.strDevice = monitorInfo.szDevice;
                if (0 == info.rect.left && 0 == info.rect.top)
                {
                    info.bPrimary = true;
                }

                MONITOR_DESC_LIST* pInfos = (MONITOR_DESC_LIST*)lparam;
                pInfos->push_back(info);
                return TRUE;

            }, (LPARAM)&vMonitorInfos))
        {
        }

        return vMonitorInfos;
    }

    MONITOR_DESC_INFO CDisplayUtils::GetPrimaryDisplayMonitorInfo()
    {
        MONITOR_DESC_LIST vInfos = GetDisplayMonitorInfos();
        MONITOR_DESC_INFO emptyInfo;

        for (size_t i = 0; i < vInfos.size(); i++)
        {
            if (vInfos[i].bPrimary)
            {
                return vInfos[i];
            }
        }

        return emptyInfo;
    }

    DISPLAY_DESC_LIST CDisplayUtils::GetDisplayMode(DWORD iDevNum)
    {
        DISPLAY_DESC_LIST vInfos;
        DISPLAY_DEVICE DisplayDevice = { 0 };
        DisplayDevice.cb = sizeof(DISPLAY_DEVICE);

        if (::EnumDisplayDevices(NULL, iDevNum, &DisplayDevice, EDD_GET_DEVICE_INTERFACE_NAME))
        {
            DEVMODE dm = { 0 };
            dm.dmSize = sizeof(dm);
            DWORD    iModeNum = 0;

            DWORD lastDmPelsWidth = 0;
            DWORD lastDmPelsHeight = 0;
            DWORD lastDmDisplayFrequency = 0;

            while (::EnumDisplaySettings(DisplayDevice.DeviceName, iModeNum++, &dm))
            {
                if (!(lastDmPelsWidth == dm.dmPelsWidth &&
                    lastDmPelsHeight == dm.dmPelsHeight &&
                    lastDmDisplayFrequency == dm.dmDisplayFrequency
                    )
                    )
                {
                    DISPLAY_MODE_INFO info;
                    info.dmPelsWidth = dm.dmPelsWidth;
                    info.dmPelsHeight = dm.dmPelsHeight;
                    info.FrequencyList.push_back(dm.dmDisplayFrequency);

                    bool fFind = false;
                    for (auto& item : vInfos)
                    {
                        if (item.dmPelsWidth == info.dmPelsWidth &&
                            item.dmPelsHeight == info.dmPelsHeight
                            )
                        {
                            item.FrequencyList.push_back(dm.dmDisplayFrequency);
                            fFind = true;
                            break;
                        }
                    }

                    if (!fFind)
                    {
                        vInfos.push_back(info);
                    }

                    lastDmPelsWidth = dm.dmPelsWidth;
                    lastDmPelsHeight = dm.dmPelsHeight;
                    lastDmDisplayFrequency = dm.dmDisplayFrequency;
                }
            }
        }

        return vInfos;
    }

    DISPLAY_DESC_LIST CDisplayUtils::GetPrimaryDisplayMode()
    {
        return GetDisplayMode(GetPrimaryMonitorIndex());
    }

    DISPLAY_FREQUENCY_LIST GetCurrentFrequencyList()
    {
        DISPLAY_FREQUENCY_LIST frequencyList;
        DWORD nWidth = 0;
        DWORD nHeight = 0;

        DEVMODE dm = { 0 };
        dm.dmSize = sizeof(dm);

        if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            nWidth = dm.dmPelsWidth;
            nHeight = dm.dmPelsHeight;
        }

        DISPLAY_DEVICE DisplayDevice = { 0 };
        DisplayDevice.cb = sizeof(DISPLAY_DEVICE);

        if (::EnumDisplayDevices(NULL, 0, &DisplayDevice, EDD_GET_DEVICE_INTERFACE_NAME))
        {
            DEVMODE dm = { 0 };
            dm.dmSize = sizeof(dm);
            DWORD    iModeNum = 0;

            while (::EnumDisplaySettings(DisplayDevice.DeviceName, iModeNum++, &dm))
            {
                if (nWidth == dm.dmPelsWidth && nHeight == dm.dmPelsHeight)
                {
                    DISPLAY_MODE_INFO info;
                    info.dmPelsWidth = dm.dmPelsWidth;
                    info.dmPelsHeight = dm.dmPelsHeight;
                    info.FrequencyList.push_back(dm.dmDisplayFrequency);

                    bool fFind = false;
                    for (auto& item : frequencyList)
                    {
                        if (item == dm.dmDisplayFrequency)
                        {
                            fFind = true;
                            break;
                        }
                    }

                    if (!fFind)
                    {
                        frequencyList.push_back(dm.dmDisplayFrequency);
                    }
                }
            }
        }

        return frequencyList;
    }

    DWORD CDisplayUtils::GetPrimaryMonitorIndex()
    {
        DWORD dwMonitorIndex = 0;

        MONITOR_DESC_LIST descInfos = GetDisplayMonitorInfos();
        for (size_t i = 0; i < descInfos.size(); i++)
        {
            if (descInfos[i].bPrimary)
            {
                dwMonitorIndex = (DWORD)i;
            }
        }

        return dwMonitorIndex;
    }

    bool CDisplayUtils::GetCurrentSize(DWORD& nW, DWORD& nH)
    {
        DEVMODE dm = { 0 };
        dm.dmSize = sizeof(dm);

        if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            nW = dm.dmPelsWidth;
            nH = dm.dmPelsHeight;
            return true;
        }

        return false;
    }

    bool CDisplayUtils::SetCurrentSize(DWORD nW, DWORD nH)
    {
        DEVMODE dm = { 0 };
        dm.dmSize = sizeof(dm);

        if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            dm.dmDisplayFixedOutput = DMDFO_DEFAULT;
            dm.dmPelsWidth = nW;
            dm.dmPelsHeight = nH;

            long lRet = ::ChangeDisplaySettings(&dm, 0);
            if (DISP_CHANGE_SUCCESSFUL == lRet)
            {
                return true;
            }
        }

        return false;
    }

    bool CDisplayUtils::GetCurrentFrequency(DWORD& nFrequency)
    {
        DEVMODE dm = { 0 };
        dm.dmSize = sizeof(dm);

        if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            nFrequency = dm.dmDisplayFrequency;
            return true;
        }

        return false;
    }

    bool CDisplayUtils::SetCurrentFrequency(DWORD nFrequency)
    {
        DEVMODE dm = { 0 };
        dm.dmSize = sizeof(dm);

        if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            dm.dmDisplayFixedOutput = DMDFO_DEFAULT;
            dm.dmDisplayFrequency = nFrequency;

            long lRet = ::ChangeDisplaySettings(&dm, 0);
            if (DISP_CHANGE_SUCCESSFUL == lRet)
            {
                return true;
            }
        }

        return false;
    }

    bool CDisplayUtils::GetCurrentOrientation(DisplayOrientation& nOri)
    {
        DEVMODE dm = { 0 };
        dm.dmSize = sizeof(dm);

        if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            nOri = (DisplayOrientation)dm.dmDisplayOrientation;
            return true;
        }

        return false;
    }

    bool CDisplayUtils::SetCurrentOrientation(DisplayOrientation nOri)
    {
        DEVMODE dm = { 0 };
        dm.dmSize = sizeof(dm);

        if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            dm.dmDisplayOrientation = nOri;

            long lRet = ::ChangeDisplaySettings(&dm, CDS_UPDATEREGISTRY);
            if (DISP_CHANGE_SUCCESSFUL == lRet)
            {
                return true;
            }
        }

        return false;
    }

    bool CDisplayUtils::SetCurrentAutoRotation(bool bEnable)
    {
        LSTATUS lr = ERROR_SUCCESS;
        DWORD dwData = bEnable ? 1 : 0;

        lr = ::RegSetKeyValue(HKLM,
            _T(SCREEN_AUTOROTATION),
            _T(R"(Enable)"),
            REG_DWORD,
            &dwData,
            sizeof(DWORD)
        );

        if (ERROR_SUCCESS != lr)
        {
            return false;
        }

        return ERROR_SUCCESS == lr;
    }

    bool CDisplayUtils::GetCurrentAutoRotation(bool& bEnable)
    {
        LSTATUS lr = ERROR_SUCCESS;
        DWORD dwData = 0;
        DWORD cbSize = sizeof(dwData);

        lr = ::RegGetValue(HKLM,
            _T(SCREEN_AUTOROTATION),
            _T(R"(Enable)"),
            RRF_RT_REG_DWORD | RRF_ZEROONFAILURE,
            NULL,
            &dwData,
            &cbSize
        );

        if (ERROR_SUCCESS != lr)
        {
            return false;
        }

        bEnable = (1 == dwData ? true : false);

        return ERROR_SUCCESS == lr;
    }

    bool CDisplayUtils::IsCurrentLandscape(DWORD nW, DWORD nH, DWORD nOri)
    {
        if (nW > nH && (DMDO_DEFAULT == nOri || DMDO_180 == nOri))
        {
            return true;
        }

        if (nW < nH && (DMDO_90 == nOri || DMDO_270 == nOri))
        {
            return true;
        }

        return false;
    }

    bool CDisplayUtils::SetCurrentLandscape(bool bFlipped)
    {
        DEVMODE dm = { 0 };
        DWORD dwW = 0;
        DWORD dwH = 0;

        dm.dmSize = sizeof(dm);

        if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            dwW = dm.dmPelsWidth;
            dwH = dm.dmPelsHeight;

            // 检查是否横屏
            if (IsCurrentLandscape(dwW, dwH, dm.dmDisplayOrientation))
            {
                if (dwW < dwH)
                {
                    dm.dmPelsWidth = dwH;
                    dm.dmPelsHeight = dwW;
                }

                dm.dmDisplayOrientation = bFlipped ? DMDO_180 : DMDO_DEFAULT;
            }
            else
            {
                if (dwW < dwH)
                {
                    dm.dmPelsWidth = dwH;
                    dm.dmPelsHeight = dwW;
                }

                dm.dmDisplayOrientation = bFlipped ? DMDO_90 : DMDO_270;
            }

            long lRet = ::ChangeDisplaySettings(&dm, 0);
            if (DISP_CHANGE_SUCCESSFUL == lRet)
            {
                return true;
            }
        }

        return false;
    }

    bool CDisplayUtils::SetCurrentPortrait(bool bFlipped)
    {
        DEVMODE dm = { 0 };
        DWORD dwW = 0;
        DWORD dwH = 0;

        dm.dmSize = sizeof(dm);

        if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
        {
            dwW = dm.dmPelsWidth;
            dwH = dm.dmPelsHeight;

            // 检查是否横屏
            if (IsCurrentLandscape(dwW, dwH, dm.dmDisplayOrientation))
            {
                if (dwW >= dwH)
                {
                    dm.dmPelsWidth = dwH;
                    dm.dmPelsHeight = dwW;
                }

                dm.dmDisplayOrientation = bFlipped ? DMDO_270 : DMDO_90;
            }
            else
            {
                if (dwW >= dwH)
                {
                    dm.dmPelsWidth = dwH;
                    dm.dmPelsHeight = dwW;
                }

                dm.dmDisplayOrientation = bFlipped ? DMDO_180 : DMDO_DEFAULT;
            }

            long lRet = ::ChangeDisplaySettings(&dm, 0);
            if (DISP_CHANGE_SUCCESSFUL == lRet)
            {
                return true;
            }
        }

        return false;
    }
}