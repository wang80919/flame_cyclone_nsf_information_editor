#pragma once

#include <Windows.h>
#include <powersetting.h>

//节能
//DEFINE_GUID(GUID_MAX_POWER_SAVINGS, 0xA1841308, 0x3541, 0x4FAB, 0xBC, 0x81, 0xF7, 0x15, 0x56, 0xF2, 0x0B, 0x4A);
// 
//高性能
//DEFINE_GUID(GUID_MIN_POWER_SAVINGS, 0x8C5E7FDA, 0xE8BF, 0x4A96, 0x9A, 0x85, 0xA6, 0xE2, 0x3A, 0x8C, 0x63, 0x5C);
// 
//平衡
//DEFINE_GUID(GUID_TYPICAL_POWER_SAVINGS, 0x381B4222, 0xF694, 0x41F0, 0x96, 0x85, 0xFF, 0x5B, 0xB2, 0x60, 0xDF, 0x2E);
// 
//所有电源计划
//DEFINE_GUID(ALL_POWERSCHEMES_GUID, 0x68A1E95E, 0x13EA, 0x41E1, 0x80, 0x11, 0x0C, 0x49, 0x6C, 0xA4, 0x90, 0xB0);

class CPowerCfgUtils
{
public:

    //
    // @brief: 设置处理器性能加速模式
    // @param: isEnable     是否启用处理器性能加速
    // @param: isACValue    修改接通电源时的配置
    // @param: isDCValue    修改使用电池时的配置
    // @param: PolicyGuid   指定修改的电源计划, 为空则指定当前活动的电源计划
    // @ret: BOOL           操作结果
    static BOOL SetProcessorBoostMode(
        int nACValue,
        int nDCValue,
        CONST GUID* PolicyGuid = nullptr
    );

    //
    // @brief: 设置显示器亮度
    // @param: nValue     显示器亮度
    // @param: PolicyGuid   指定修改的电源计划, 为空则指定当前活动的电源计划
    // @ret: BOOL           操作结果
    static BOOL SetVideoNormalLevel(
        int nACValue,
        int nDCValue,
        CONST GUID* PolicyGuid = nullptr
    );

    //
    // @brief: 获取显示器亮度
    // @param: nValue     显示器亮度
    // @param: PolicyGuid   指定修改的电源计划, 为空则指定当前活动的电源计划
    // @ret: BOOL           操作结果
    static BOOL GetVideoNormalLevel(
        int& nACValue,
        int& nDCValue,
        CONST GUID* PolicyGuid = nullptr
    );

    //
    // @brief: 设置当前活动电源计划
    // @param: PolicyGuid   指定修改的电源计划, 为空则指定当前活动的电源计划
    // @ret: BOOL           操作结果
    static BOOL SetActiveScheme(CONST GUID* PolicyGuid = nullptr);
};