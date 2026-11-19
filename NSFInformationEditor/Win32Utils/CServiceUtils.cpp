#include "CServiceUtils.h"
#include <winsvc.h>
#include <winioctl.h>

bool CServiceUtils::InstallService(
    const _tstring& strSrvName,
    const _tstring& strDisplayName,
    const _tstring& strExecFile
)
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSvc = NULL;
    bool bResult = false;

    do
    {
        //打开hScm数据库并返回句柄
        hScm = ::OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
        if (NULL == hScm)
        {
            break;
        }

        //创建服务
        hSvc = ::CreateService(hScm, strSrvName.c_str(),
            strDisplayName.c_str(),
            SERVICE_ALL_ACCESS,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE,
            strExecFile.c_str(),
            NULL, NULL, NULL, NULL, NULL);

        //如果创建服务失败并且服务不存在, 则认为失败
        if (NULL == hSvc && (ERROR_SERVICE_EXISTS != ::GetLastError()))
        {
            break;
        }

        bResult = true;

    } while (false);

    if (NULL != hSvc)
    {
        ::CloseServiceHandle(hSvc); //关闭服务句柄
    }

    if (NULL != hScm)
    {
        ::CloseServiceHandle(hScm); //关闭hScm数据库句柄
    }

    return bResult;
}

bool CServiceUtils::UninstallService(const _tstring& strSrvName)
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSvc = NULL;
    SERVICE_STATUS ServiceStatus = { 0 };
    bool bResult = false;

    do
    {
        //指定服务控制管理器建立连接并打开服务管理数据库
        hScm = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (NULL == hScm)
        {
            break;
        }

        hSvc = ::OpenService(hScm, strSrvName.c_str(), SERVICE_ALL_ACCESS | DELETE);

        //打开服务失败并且服务不存在, 则不需要删除
        if (NULL == hSvc)
        {
            if (ERROR_SERVICE_DOES_NOT_EXIST != ::GetLastError())
            {
                bResult = true;
            }
            break;
        }

        //查询服务运行状态
        if (!::QueryServiceStatus(hSvc, &ServiceStatus))
        {
            break;
        }

        //服务处于运行状态先通知服务停止
        if (SERVICE_RUNNING == ServiceStatus.dwCurrentState)
        {
            if (!::ControlService(hSvc, SERVICE_CONTROL_STOP, &ServiceStatus))
            {
                break;
            }
        }

        bResult = ::DeleteService(hSvc);

    } while (false);

    if (NULL != hSvc)
    {
        ::CloseServiceHandle(hSvc);
    }

    if (NULL != hScm)
    {
        ::CloseServiceHandle(hScm);
    }

    return bResult;
}

bool CServiceUtils::RunService(const _tstring& strSrvName)
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSvc = NULL;
    SERVICE_STATUS ServiceStatus = { 0 };
    bool bResult = false;

    do
    {
        //指定服务控制管理器建立连接并打开服务管理数据库
        hScm = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (NULL == hScm)
        {
            break;
        }

        //打开服务
        hSvc = ::OpenService(hScm, strSrvName.c_str(), SERVICE_ALL_ACCESS | DELETE);
        if (NULL == hSvc)
        {
            break;
        }

        //查询服务运行状态
        if (!::QueryServiceStatus(hSvc, &ServiceStatus))
        {
            break;
        }

        //服务已经处于运行状态则不用管
        if (SERVICE_RUNNING == ServiceStatus.dwCurrentState)
        {
            bResult = true;
            break;
        }

        //启动服务
        bResult = ::StartService(hSvc, 0, NULL);

    } while (false);

    if (NULL != hSvc)
    {
        ::CloseServiceHandle(hSvc);
    }

    if (NULL != hScm)
    {
        ::CloseServiceHandle(hScm);
    }

    return bResult;
}

bool CServiceUtils::StopService(const _tstring& strSrvName)
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSvc = NULL;
    SERVICE_STATUS ServiceStatus = { 0 };
    bool bResult = false;

    do
    {
        //指定服务控制管理器建立连接并打开服务管理数据库
        hScm = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (NULL == hScm)
        {
            break;
        }

        //打开服务失败并且服务不存在, 则不需要停止
        hSvc = ::OpenService(hScm, strSrvName.c_str(), SERVICE_ALL_ACCESS | DELETE);
        if (NULL == hSvc)
        {
            if (ERROR_SERVICE_DOES_NOT_EXIST != ::GetLastError())
            {
                bResult = true;
            }
            break;
        }

        //查询服务运行状态
        if (!::QueryServiceStatus(hSvc, &ServiceStatus))
        {
            break;
        }

        //服务处于运行状态先通知服务停止
        if (SERVICE_RUNNING == ServiceStatus.dwCurrentState)
        {
            if (!::ControlService(hSvc, SERVICE_CONTROL_STOP, &ServiceStatus))
            {
                break;
            }
        }

        bResult = true;

    } while (false);

    if (NULL != hSvc)
    {
        ::CloseServiceHandle(hSvc);
    }

    if (NULL != hScm)
    {
        ::CloseServiceHandle(hScm);
    }

    return bResult;
}

bool CServiceUtils::IsServiceExist(const _tstring& strSrvName)
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSvc = NULL;
    SERVICE_STATUS ServiceStatus = { 0 };
    bool bResult = false;

    do
    {
        //指定服务控制管理器建立连接并打开服务管理数据库
        hScm = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (NULL == hScm)
        {
            break;
        }

        //打开服务
        hSvc = ::OpenService(hScm, strSrvName.c_str(), SERVICE_ALL_ACCESS | DELETE);
        if (NULL == hSvc)
        {
            break;
        }

        bResult = true;

    } while (false);

    if (NULL != hSvc)
    {
        ::CloseServiceHandle(hSvc);
    }

    if (NULL != hScm)
    {
        ::CloseServiceHandle(hScm);
    }

    return bResult;
}

bool CServiceUtils::IsServiceRunning(const _tstring& strSrvName)
{
    SC_HANDLE hScm = NULL;
    SC_HANDLE hSvc = NULL;
    SERVICE_STATUS ServiceStatus = { 0 };
    bool bResult = false;

    do
    {
        //指定服务控制管理器建立连接并打开服务管理数据库
        hScm = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (NULL == hScm)
        {
            break;
        }

        //打开服务
        hSvc = ::OpenService(hScm, strSrvName.c_str(), SERVICE_ALL_ACCESS | DELETE);
        if (NULL == hSvc)
        {
            break;
        }

        //查询服务运行状态
        if (!::QueryServiceStatus(hSvc, &ServiceStatus))
        {
            break;
        }

        //服务已经处于运行状态则不用管
        if (SERVICE_RUNNING != ServiceStatus.dwCurrentState)
        {
            break;
        }

        bResult = true;

    } while (false);

    if (NULL != hSvc)
    {
        ::CloseServiceHandle(hSvc);
    }

    if (NULL != hScm)
    {
        ::CloseServiceHandle(hScm);
    }

    return bResult;
}