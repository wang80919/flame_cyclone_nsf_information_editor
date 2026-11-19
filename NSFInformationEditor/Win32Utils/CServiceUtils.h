#pragma once

#include <string>
#include <windows.h>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CServiceUtils
{
public:

    CServiceUtils(CServiceUtils&) = delete;
    CServiceUtils& operator = (const CServiceUtils&) = delete;

    /**
     * @brief 安装服务
     * @details 安装服务
     *
     * @param strSrvName 服务名称
     * @param strDisplayName 服务显示名称
     * @param strExecFile 可执行文件路径
     * @return @c true 操作成功
     *         @c false 操作失败
    */
    static bool InstallService(
        const _tstring& strSrvName, 
        const _tstring& strDisplayName, 
        const _tstring& strExecFile
    );

    /**
     * @brief 卸载服务
     * @details 卸载服务
     *
     * @param strSrvName 服务名称
     * @return @c true 操作成功
     *         @c false 操作失败
    */
    static bool UninstallService(const _tstring& strSrvName);

    /**
     * @brief 运行服务
     * @details 运行服务
     *
     * @param strSrvName 服务名称
     * @return @c true 操作成功
     *         @c false 操作失败
    */
    static bool RunService(const _tstring& strSrvName);

    /**
     * @brief 停止服务
     * @details 停止服务
     *
     * @param strSrvName 服务名称
     * @return @c true 操作成功
     *         @c false 操作失败
    */
    static bool StopService(const _tstring& strSrvName);

    /**
     * @brief 检查服务是否存在
     * @details 运行服务
     *
     * @param strSrvName 服务名称
     * @return @c true 存在
     *         @c false 不存在
    */
    static bool IsServiceExist(const _tstring& strSrvName);

    /**
     * @brief 检查服务是否运行中
     * @details 运行服务
     *
     * @param strSrvName 服务名称
     * @return @c true 运行中
     *         @c false 未运行
    */
    static bool IsServiceRunning(const _tstring& strSrvName);
};