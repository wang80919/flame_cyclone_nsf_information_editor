#pragma once
#include <string>
#include <vector>
#include <set>
#include <windows.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

//数字化仪到监视器映射工具实用类
class CMultiDigiUtils
{
public:

    //获取触摸设备路径
    static std::vector<_tstring> GetTouchDevPath();

    //获取显示设备路径
    static std::set<_tstring> GetDisplayDevPath();

    //获取触摸设备句柄
    static HANDLE GetTouchDeviceHandle(const _tstring& pDevPath);

    //获取显示设备句柄
    static HMONITOR GetDisplayDeviceHandle(const _tstring& pDevPath);

    //设置显示映射
    static bool SetDisplayMapping(const _tstring& touchPath, const _tstring& displayPath);

    //设置显示映射
    static bool SetDisplayMapping(HANDLE touch, HMONITOR display);

    //通知屏幕映射发生变化
    static bool BroadcastMappingChange();
};

