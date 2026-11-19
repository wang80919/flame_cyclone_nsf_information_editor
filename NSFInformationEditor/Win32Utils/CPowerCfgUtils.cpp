#include "CPowerCfgUtils.h"
#pragma comment(lib, "PowrProf.lib")

BOOL CPowerCfgUtils::SetProcessorBoostMode(
    int nACValue,
    int nDCValue,
    CONST GUID* PolicyGuid/* = nullptr*/
)
{
    LPGUID lpActivePolicyGuid = NULL;
    LPCGUID lpGuidScheme = PolicyGuid;
    DWORD dwResult = ERROR_SUCCESS;

    do
    {
        if (nullptr == PolicyGuid)
        {
            //检索当前活动的电源方案
            dwResult = ::PowerGetActiveScheme(NULL, &lpActivePolicyGuid);
            if (ERROR_SUCCESS != dwResult)
            {
                break;
            }
            lpGuidScheme = lpActivePolicyGuid;
        }

        //交流电状态(接通电源)
            dwResult = ::PowerWriteACValueIndex(
                NULL,
                lpGuidScheme,
                &GUID_PROCESSOR_SETTINGS_SUBGROUP,
                &GUID_PROCESSOR_PERF_BOOST_MODE,
                nACValue ? 1 : 0
            );
            if (ERROR_SUCCESS != dwResult)
            {
                break;
            }

        //直流电状态(使用电池)
            dwResult = ::PowerWriteDCValueIndex(
                NULL,
                lpGuidScheme,
                &GUID_PROCESSOR_SETTINGS_SUBGROUP,
                &GUID_PROCESSOR_PERF_BOOST_MODE,
                nDCValue ? 1 : 0
            );
            if (ERROR_SUCCESS != dwResult)
            {
                break;
            }

    } while (FALSE);

    if (lpGuidScheme)
    {
        ::LocalFree(lpActivePolicyGuid);
    }

    return ERROR_SUCCESS == dwResult;
}

BOOL CPowerCfgUtils::SetVideoNormalLevel(
    int nACValue,
    int nDCValue,
    CONST GUID* PolicyGuid/* = nullptr*/
)
{
    LPGUID lpActivePolicyGuid = NULL;
    LPCGUID lpGuidScheme = PolicyGuid;
    DWORD dwResult = ERROR_SUCCESS;

    do
    {
        if (nullptr == PolicyGuid)
        {
            //检索当前活动的电源方案
            dwResult = ::PowerGetActiveScheme(NULL, &lpActivePolicyGuid);
            if (ERROR_SUCCESS != dwResult)
            {
                break;
            }
            lpGuidScheme = lpActivePolicyGuid;
        }

        dwResult = ::PowerWriteACValueIndex(
            NULL,
            lpGuidScheme,
            &GUID_VIDEO_SUBGROUP,
            &GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS,
            nACValue
        );

        if (ERROR_SUCCESS != dwResult)
        {
            break;
        }

        dwResult = ::PowerWriteDCValueIndex(
            NULL,
            lpGuidScheme,
            &GUID_VIDEO_SUBGROUP,
            &GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS,
            nDCValue
        );

        if (ERROR_SUCCESS != dwResult)
        {
            break;
        }
    } while (FALSE);

    if (lpActivePolicyGuid)
    {
        ::LocalFree(lpActivePolicyGuid);
    }

    return ERROR_SUCCESS == dwResult;
}

BOOL CPowerCfgUtils::GetVideoNormalLevel(
    int& nACValue,
    int& nDCValue,
    CONST GUID* PolicyGuid/* = nullptr*/
)
{
    LPGUID lpActivePolicyGuid = NULL;
    LPCGUID lpGuidScheme = PolicyGuid;
    DWORD dwResult = ERROR_SUCCESS;

    ULONG ulType = REG_DWORD;
    DWORD dwValue = 0;
    DWORD dwSize = sizeof(DWORD);

    do
    {
        if (nullptr == PolicyGuid)
        {
            //检索当前活动的电源方案
            dwResult = ::PowerGetActiveScheme(NULL, &lpActivePolicyGuid);
            if (ERROR_SUCCESS != dwResult)
            {
                break;
            }
            lpGuidScheme = lpActivePolicyGuid;
        }

        ulType = REG_DWORD;
        dwValue = 0;
        dwSize = sizeof(DWORD);
        dwResult = ::PowerReadACValue(
            NULL,
            lpGuidScheme,
            &GUID_VIDEO_SUBGROUP,
            &GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS,
            &ulType,
            (LPBYTE)&dwValue,
            &dwSize
        );

        if (ERROR_SUCCESS != dwResult)
        {
            break;
        }
        nACValue = dwValue;

        ulType = REG_DWORD;
        dwValue = 0;
        dwSize = sizeof(DWORD);
        dwResult = ::PowerReadDCValue(
            NULL,
            lpGuidScheme,
            &GUID_VIDEO_SUBGROUP,
            &GUID_DEVICE_POWER_POLICY_VIDEO_BRIGHTNESS,
            &ulType,
            (LPBYTE)&dwValue,
            &dwSize
        );

        if (ERROR_SUCCESS != dwResult)
        {
            break;
        }
        nDCValue = dwValue;

    } while (FALSE);

    if (lpActivePolicyGuid)
    {
        ::LocalFree(lpActivePolicyGuid);
    }

    return ERROR_SUCCESS == dwResult;
}

//
// @brief: 设置当前活动电源计划
// @param: PolicyGuid   指定修改的电源计划, 为空则指定当前活动的电源计划
// @ret: BOOL           操作结果
BOOL CPowerCfgUtils::SetActiveScheme(CONST GUID* PolicyGuid/* = nullptr*/)
{
    LPGUID lpActivePolicyGuid = NULL;
    LPCGUID lpGuidScheme = PolicyGuid;
    DWORD dwResult = ERROR_SUCCESS;

    do
    {
        if (nullptr == PolicyGuid)
        {
            //检索当前活动的电源方案
            dwResult = ::PowerGetActiveScheme(NULL, &lpActivePolicyGuid);
            if (ERROR_SUCCESS != dwResult)
            {
                break;
            }
            lpGuidScheme = lpActivePolicyGuid;
        }

        //设置电源方案处于活动状态
        dwResult = ::PowerSetActiveScheme(NULL, lpGuidScheme);
        if (ERROR_SUCCESS != dwResult)
        {
            break;
        }

    } while (FALSE);

    if (lpActivePolicyGuid)
    {
        ::LocalFree(lpActivePolicyGuid);
    }

    return ERROR_SUCCESS == dwResult;
}