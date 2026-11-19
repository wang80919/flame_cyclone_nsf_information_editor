#include "CSensorUtils.h"
#include <Sensors.h>

#pragma comment(lib, "Propsys.lib")
#pragma comment(lib, "PortableDeviceGuids.lib")
#pragma comment(lib, "sensorsapi.lib")

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

CSensorEvent::CSensorEvent(eSensorType eTypeID)
    :
    m_eTypeID(eTypeID),
    m_cRef(0),
    m_fSwapXY(false)
{

}

CSensorEvent::~CSensorEvent()
{

}

STDMETHODIMP CSensorEvent::QueryInterface(REFIID iid, void** ppv)
{
    if (ppv == NULL)
    {
        return E_POINTER;
    }
    if (iid == __uuidof(IUnknown))
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == __uuidof(ISensorEvents))
    {
        *ppv = static_cast<ISensorEvents*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CSensorEvent::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSensorEvent::Release()
{
    ULONG count = InterlockedDecrement(&m_cRef);
    if (count == 0)
    {
        delete this;
        return 0;
    }

    return count;
}

STDMETHODIMP CSensorEvent::OnEvent( ISensor* pSensor, REFGUID eventID, IPortableDeviceValues* pEventData)
{
    HRESULT hr = S_OK;
    return hr;
}

STDMETHODIMP CSensorEvent::OnDataUpdated(ISensor* pSensor, ISensorDataReport* pNewData)
{
    HRESULT hr = S_OK;

    if (NULL == pNewData ||
        NULL == pSensor)
    {
        return E_INVALIDARG;
    }

    double dX = 0.0f;
    double dY = 0.0f;
    double dZ = 0.0f;

    PROPVARIANT var = { 0 };

    if (eSensorGyrometer == m_eTypeID)
    {
        hr = pNewData->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND, &var);
        if (SUCCEEDED(hr))
        {
            if (var.vt == VT_R8)
            {
                dX = var.dblVal;
            }
        }

        PropVariantClear(&var);

        hr = pNewData->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND, &var);
        if (SUCCEEDED(hr))
        {
            if (var.vt == VT_R8)
            {
                dY = var.dblVal;
            }
        }

        PropVariantClear(&var);

        hr = pNewData->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND, &var);
        if (SUCCEEDED(hr))
        {
            if (var.vt == VT_R8)
            {
                dZ = var.dblVal;
            }
        }
    }
    else if (eSensorAccelerometer == m_eTypeID)
    {
        hr = pNewData->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_X_G, &var);
        if (SUCCEEDED(hr))
        {
            if (var.vt == VT_R8)
            {
                dX = var.dblVal;
            }
        }

        hr = pNewData->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Y_G, &var);
        if (SUCCEEDED(hr))
        {
            if (var.vt == VT_R8)
            {
                dY = var.dblVal;
            }
        }

        hr = pNewData->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Z_G, &var);
        if (SUCCEEDED(hr))
        {
            if (var.vt == VT_R8)
            {
                dZ = var.dblVal;
            }
        }
    }

    PropVariantClear(&var);

    if (m_DataCallback)
    {
        if (m_fSwapXY)
        {
            m_DataCallback(m_eTypeID, dY, dX, dZ);
        }
        else
        {
            m_DataCallback(m_eTypeID, dX, dY, dZ);
        }
    }

    return hr;
}

HRESULT CSensorEvent::OnLeave(REFSENSOR_ID sensorID)
{
    HRESULT hr = S_OK;

    //执行要离开的传感器的任何内务处理任务, 例如，如果保留了对传感器的引用, 立即释放它并将指针设置为 NULL.
    return hr;
}

HRESULT CSensorEvent::OnStateChanged(ISensor* pSensor, SensorState state)
{
    HRESULT hr = S_OK;

    if (NULL == pSensor)
    {
        return E_INVALIDARG;
    }

    if (state == SENSOR_STATE_READY)
    {
        wprintf_s(L"\nTime sensor is now ready.");
    }
    else if (state == SENSOR_STATE_ACCESS_DENIED)
    {
        wprintf_s(L"\nNo permission for the time sensor.\n");
        wprintf_s(L"Enable the sensor in the control panel.\n");
    }

    return hr;
}

void CSensorEvent::SetSwapXY(bool fSwap)
{
    m_fSwapXY = fSwap;
}

void CSensorEvent::SetDataCallback(std::function<void(int nType, double x, double y, double z)> cb)
{
    m_DataCallback = cb;
}

CSensorUtils::CSensorUtils() :
    m_pGyrometerSensor(nullptr),
    m_pAccelerometerSensor(nullptr),
    m_pGyrometerEvent(nullptr),
    m_pAccelerometerEvent(nullptr),
    m_fSwapAccelerometerXY(false),
    m_fSwapGyrometerXY(false),
    m_fInit(false)
{

}

CSensorUtils::~CSensorUtils()
{
    Uninitialize();
}

BOOL CSensorUtils::CheckForSupportedDataFields(ISensor* pSensor)
{
    IPortableDeviceKeyCollection* pKeys = NULL;
    BOOL bRet = FALSE;

    do
    {
        DWORD cKeys = 0;
        HRESULT hr = S_OK;

        // 创建密钥集合以存储属性密钥。
        hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pKeys));
        if (FAILED(hr))
        {
            break;
        }

        hr = pSensor->GetSupportedDataFields(&pKeys);
        if (FAILED(hr))
        {
            break;
        }

        hr = pKeys->GetCount(&cKeys);
        if (FAILED(hr))
        {
            break;
        }

        PROPERTYKEY pk = { 0 };
        BOOL bX = FALSE;
        BOOL bY = FALSE;
        BOOL bZ = FALSE;
        BOOL bTimestamp = FALSE;

        for (DWORD i = 0; i < cKeys; i++)
        {
            hr = pKeys->GetAt(i, &pk);
            if (FAILED(hr))
            {
                continue;
            }

            if (IsEqualPropertyKey(pk, SENSOR_DATA_TYPE_TIMESTAMP))
            {
                bTimestamp = true;
            }
            else if (IsEqualPropertyKey(pk, SENSOR_DATA_TYPE_ANGULAR_VELOCITY_X_DEGREES_PER_SECOND))
            {
                bX = TRUE;
            }
            else if (IsEqualPropertyKey(pk, SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Y_DEGREES_PER_SECOND))
            {
                bY = TRUE;
            }
            else if (IsEqualPropertyKey(pk, SENSOR_DATA_TYPE_ANGULAR_VELOCITY_Z_DEGREES_PER_SECOND))
            {
                bZ = TRUE;
            }
        }

        bRet = bX && bY && bZ;

    } while (false);

    SafeRelease(&pKeys);

    return bRet;
}

BOOL CSensorUtils::_InitializeGyrometer()
{
    ISensorManager* pSensorManager = NULL;
    ISensorCollection* pSensorColl = NULL;
    HRESULT hr = S_OK;
    BOOL bRet = FALSE;

    if (m_pGyrometerSensor)
    {
        return true;
    }

    do
    {
        // 创建传感器管理器
        hr = CoCreateInstance(CLSID_SensorManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSensorManager));
        if (FAILED(hr))
        {
            break;
        }

        // 获取传感器集合  SENSOR_TYPE_GYROMETER_3D SENSOR_TYPE_ACCELEROMETER_3D
        hr = pSensorManager->GetSensorsByType(SENSOR_TYPE_GYROMETER_3D, &pSensorColl);
        if (FAILED(hr))
        {
            break;
        }

        //获取传感器数量
        ULONG ulCount = 0;
        hr = pSensorColl->GetCount(&ulCount);
        if (FAILED(hr) || ulCount < 1)
        {
            break;
        }

        //获取第0个传感器
        hr = pSensorColl->GetAt(0, &m_pGyrometerSensor);
        if (FAILED(hr))
        {
            break;
        }

        //检测状态
        SensorState state = SENSOR_STATE_NOT_AVAILABLE;
        hr = m_pGyrometerSensor->GetState(&state);
        if (FAILED(hr))
        {
            break;
        }


        PROPVARIANT var = { 0 };
        m_pGyrometerSensor->GetProperty(SENSOR_PROPERTY_DESCRIPTION, &var);
        PropVariantClear(&var);

        if (state == SENSOR_STATE_ACCESS_DENIED)
        {
            _tprintf_s(_T("\nSensor not enabled, requesting permissions...\n"));
            hr = pSensorManager->RequestPermissions(0, pSensorColl, TRUE);
            if (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) ||
                hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                _tprintf_s(_T("\nYou have previously denied access to this sensor.\n"));
                _tprintf_s(_T("Please use the Location and Other Sensors control panel\n"));
                _tprintf_s(_T("to enable the WDK Time Sensor and run this program again.\n"));
            }

            break;
        }

        ISensorDataReport* pData = nullptr;
        hr = m_pGyrometerSensor->GetData(&pData);
        if (nullptr != pData)
        {
            pData->Release();
        }

        //设置传感器事件
        m_pGyrometerEvent = new (std::nothrow) CSensorEvent(eSensorGyrometer);
        if (nullptr != m_pGyrometerEvent)
        {
            m_pGyrometerEvent->AddRef();
            m_pGyrometerSensor->SetEventSink(m_pGyrometerEvent);
            m_pGyrometerEvent->SetSwapXY(m_fSwapGyrometerXY);
        }

        bRet = TRUE;

    } while (false);

    SafeRelease(&pSensorColl);
    SafeRelease(&pSensorManager);

    return bRet;
}

BOOL CSensorUtils::_InitializeAccelerometer()
{
    ISensorManager* pSensorManager = NULL;
    ISensorCollection* pSensorColl = NULL;
    HRESULT hr = S_OK;
    BOOL bRet = FALSE;

    if (m_pAccelerometerSensor)
    {
        return true;
    }

    do
    {
        // 创建传感器管理器
        hr = CoCreateInstance(CLSID_SensorManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSensorManager));
        if (FAILED(hr))
        {
            break;
        }

        // 获取传感器集合
        hr = pSensorManager->GetSensorsByType(SENSOR_TYPE_ACCELEROMETER_3D, &pSensorColl);
        if (FAILED(hr))
        {
            break;
        }

        //获取传感器数量
        ULONG ulCount = 0;
        hr = pSensorColl->GetCount(&ulCount);
        if (FAILED(hr) || ulCount < 1)
        {
            break;
        }

        //获取第0个传感器
        hr = pSensorColl->GetAt(0, &m_pAccelerometerSensor);
        if (FAILED(hr))
        {
            break;
        }

        //检测状态
        SensorState state = SENSOR_STATE_NOT_AVAILABLE;
        hr = m_pAccelerometerSensor->GetState(&state);
        if (FAILED(hr))
        {
            break;
        }

        PROPVARIANT var = { 0 };
        m_pAccelerometerSensor->GetProperty(SENSOR_PROPERTY_DESCRIPTION,  &var);
        PropVariantClear(&var);

        if (state == SENSOR_STATE_ACCESS_DENIED)
        {
            _tprintf_s(_T("\nSensor not enabled, requesting permissions...\n"));
            hr = pSensorManager->RequestPermissions(0, pSensorColl, TRUE);
            if (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) ||
                hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                _tprintf_s(_T("\nYou have previously denied access to this sensor.\n"));
                _tprintf_s(_T("Please use the Location and Other Sensors control panel\n"));
                _tprintf_s(_T("to enable the WDK Time Sensor and run this program again.\n"));
            }

            break;
        }

        //设置传感器事件
        m_pAccelerometerEvent = new (std::nothrow) CSensorEvent(eSensorAccelerometer);
        if (nullptr != m_pAccelerometerEvent)
        {
            m_pAccelerometerEvent->AddRef();
            m_pAccelerometerSensor->SetEventSink(m_pAccelerometerEvent);
            m_pAccelerometerEvent->SetSwapXY(m_fSwapAccelerometerXY);
        }

        //主动获取一次数据, 不然休眠后唤醒不会触发传感器事件通知
        PulseAccelerometer();

        bRet = TRUE;

    } while (false);

    SafeRelease(&pSensorColl);
    SafeRelease(&pSensorManager);

    return bRet;
}

bool CSensorUtils::GetSwapAccelerometerXY()
{
    return m_fSwapAccelerometerXY;
}

void CSensorUtils::SetSwapAccelerometerXY(bool fSwap)
{
    m_fSwapAccelerometerXY = fSwap;
    if (m_pAccelerometerEvent)
    {
        m_pAccelerometerEvent->SetSwapXY(m_fSwapAccelerometerXY);
    }
}

bool CSensorUtils::GetSwapGyrometerXY()
{
    return m_fSwapGyrometerXY;
}

void CSensorUtils::SetSwapGyrometerXY(bool fSwap)
{
    m_fSwapGyrometerXY = fSwap;
    if (m_pGyrometerEvent)
    {
        m_pGyrometerEvent->SetSwapXY(m_fSwapGyrometerXY);
    }
}

void CSensorUtils::PulseAccelerometer()
{
    HRESULT hr = S_OK;
    //主动获取一次数据, 不然休眠后唤醒不会触发传感器事件通知
    if (m_pAccelerometerSensor)
    {
        ISensorDataReport* pData = nullptr;
        hr = m_pAccelerometerSensor->GetData(&pData);
        if (nullptr != pData)
        {
            double dX = 0.0f;
            double dY = 0.0f;
            double dZ = 0.0f;

            PROPVARIANT var = { 0 };

            hr = pData->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_X_G, &var);
            if (SUCCEEDED(hr))
            {
                if (var.vt == VT_R8)
                {
                    dX = var.dblVal;
                }
            }

            hr = pData->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Y_G, &var);
            if (SUCCEEDED(hr))
            {
                if (var.vt == VT_R8)
                {
                    dY = var.dblVal;
                }
            }

            hr = pData->GetSensorValue(SENSOR_DATA_TYPE_ACCELERATION_Z_G, &var);
            if (SUCCEEDED(hr))
            {
                if (var.vt == VT_R8)
                {
                    dZ = var.dblVal;
                }
            }

            pData->Release();
        }
    }
}

BOOL CSensorUtils::Initialize()
{
    if (m_fInit)
    {
        return m_fInit;
    }

    m_fInit =  _InitializeGyrometer() && _InitializeAccelerometer();

    return m_fInit;
}

VOID CSensorUtils::Uninitialize()
{
    if (m_fInit)
    {
        SafeRelease(&m_pAccelerometerSensor);
        SafeRelease(&m_pGyrometerSensor);
        SafeRelease(&m_pAccelerometerEvent);
        SafeRelease(&m_pGyrometerEvent);
    }

    m_fInit = false;
}

void CSensorUtils::SetGyrometerCallback(std::function<void(int nType, double x, double y, double z)> cb)
{
    m_pGyrometerEvent->SetDataCallback(cb);
}

void CSensorUtils::SetAccelerometerCallback(std::function<void(int nType, double x, double y, double z)> cb)
{
    m_pAccelerometerEvent->SetDataCallback(cb);
}
