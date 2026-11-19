#pragma once
#include <wtypesbase.h>
#include <tchar.h>
#include <vector>
#include <string>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

namespace CDisplayUtils
{
    typedef struct _MONITOR_DESC_INFO
    {
        HMONITOR hMonitor;
        RECT rect;
        bool bPrimary;
        _tstring strDevice;

        _MONITOR_DESC_INFO() :hMonitor(NULL), rect{ 0 }, bPrimary(false) {}

    }MONITOR_DESC_INFO;

    using DISPLAY_FREQUENCY_LIST = std::vector<DWORD>;

    // 显示模式信息
    typedef struct _DISPLAY_MODE_INFO
    {
        DWORD dmPelsWidth;                      // 水平分辨率
        DWORD dmPelsHeight;                     // 垂直分辨率
        DISPLAY_FREQUENCY_LIST FrequencyList;   // 刷新率列表

        _DISPLAY_MODE_INFO() :dmPelsWidth(0), dmPelsHeight(0) {}

    }DISPLAY_MODE_INFO;

    enum DisplayOrientation
    {
        DoDefault = DMDO_DEFAULT,
        Do90 = DMDO_90,
        Do180 = DMDO_180,
        Do270 = DMDO_270,
    };

    using MONITOR_DESC_LIST = std::vector<MONITOR_DESC_INFO>;
    using DISPLAY_DESC_LIST = std::vector<DISPLAY_MODE_INFO>;

    //
    // @brief: 获取显示监视器信息
    // @param: void                             无
    // @ret: MONITOR_DESC_LIST                  显示监视器信息列表
    MONITOR_DESC_LIST GetDisplayMonitorInfos();

    //
    // @brief: 获取主显示监视器信息
    // @param: void                             无
    // @ret: MONITOR_DESC_INFO                  显示监视器信息
    MONITOR_DESC_INFO GetPrimaryDisplayMonitorInfo();

    //
    // @brief: 获取显示监视器信息
    // @param: iDevNum                          显示设备索引(从0开始)
    // @ret: DISPLAY_DESC_LIST                  显示监视器显示模式列表
    DISPLAY_DESC_LIST GetDisplayMode(DWORD iDevNum = 0);

    //
    // @brief: 获取主显示监视器信息
    // @param: iDevNum                          显示设备索引(从0开始)
    // @ret: DISPLAY_DESC_LIST                  显示监视器显示模式列表
    DISPLAY_DESC_LIST GetPrimaryDisplayMode();

    //
    // @brief: 获取当前活动屏幕刷新率列表
    // @param: nFrequency                       刷新率
    // @ret: DISPLAY_FREQUENCY_LIST             刷新率列表
    DISPLAY_FREQUENCY_LIST GetCurrentFrequencyList();

    //
    // @brief: 获取主显示监视器索引
    // @param: void                             无
    // @ret: DWORD                              主显示监视器索引
    DWORD GetPrimaryMonitorIndex();

    //
    // @brief: 获取当前活动屏幕宽高
    // @param: nW                               屏幕宽度
    // @param: nH                               屏幕高度
    // @ret: bool                               操作结果
    bool GetCurrentSize(DWORD& nW, DWORD& nH);

    //
    // @brief: 设置当前活动屏幕宽高
    // @param: nW                               屏幕宽度
    // @param: nH                               屏幕高度
    // @ret: bool                               操作结果
    bool SetCurrentSize(DWORD nW, DWORD nH);

    //
    // @brief: 获取当前活动屏幕刷新率
    // @param: nFrequency                       刷新率
    // @ret: bool                               操作结果
    bool GetCurrentFrequency(DWORD& nFrequency);

    //
    // @brief: 设置当前活动屏幕刷新率
    // @param: nFrequency                       刷新率
    // @ret: bool                               操作结果
    bool SetCurrentFrequency(DWORD nFrequency);

    //
    // @brief: 获取当前活动屏幕旋转方向
    // @param: nOri                             屏幕旋转方向
    // @ret: bool                               操作结果
    bool GetCurrentOrientation(DisplayOrientation& nOri);

    //
    // @brief: 设置当前活动屏幕旋转方向
    // @param: nOri                             屏幕旋转方向
    // @ret: bool                               操作结果
    bool SetCurrentOrientation(DisplayOrientation nOri);

    //
    // @brief: 设置当前活动屏幕自动旋转
    // @param: bEnable                          是否启用自动旋转
    // @ret: bool                               操作结果
    bool SetCurrentAutoRotation(bool bEnable = false);

    //
    // @brief: 获取当前活动屏幕自动旋转
    // @param: bEnable                          是否启用自动旋转
    // @ret: bool                               操作结果
    bool GetCurrentAutoRotation(bool& bEnable);

    //
    // @brief: 设置当前活动屏幕旋转横向
    // @param: bFlipped                         是否翻转
    // @ret: bool                               操作结果
    bool SetCurrentLandscape(bool bFlipped = false);

    //
    // @brief: 设置当前活动屏幕旋转纵向
    // @param: bFlipped                         是否翻转
    // @ret: bool                               操作结果
    bool SetCurrentPortrait(bool bFlipped = false);

    //
    // @brief: 检查当前活动屏幕是否为横屏
    // @param: nW                               屏幕宽度
    // @param: nH                               屏幕高度
    // @param: nOri                             屏幕旋转方向
    // @ret: bool                               操作结果
    bool IsCurrentLandscape(DWORD nW, DWORD nH, DWORD nOri);
};
