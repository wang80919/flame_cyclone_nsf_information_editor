#pragma once
#include <windows.h>
#include <sensorsapi.h>
#include <tchar.h>
#include <comdef.h>
#include <functional>

#pragma warning(disable:4100)

enum eSensorType
{
    eSensorUnknown,
    eSensorGyrometer,
    eSensorAccelerometer
};

//传感器事件
class CSensorEvent : public ISensorEvents
{
public:

    CSensorEvent(eSensorType eTypeID);
    ~CSensorEvent();

    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);

    STDMETHODIMP_(ULONG) AddRef();

    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP OnEvent(ISensor* pSensor, REFGUID eventID, IPortableDeviceValues* pEventData);

    STDMETHODIMP OnDataUpdated(ISensor* pSensor, ISensorDataReport* pNewData);

    STDMETHODIMP OnLeave(REFSENSOR_ID sensorID);

    STDMETHODIMP OnStateChanged(ISensor* pSensor, SensorState state);

    void SetSwapXY(bool fSwap);

public:

    void SetDataCallback(std::function<void(int nType, double x, double y, double z)> cb);

private:
    std::function<void(int nType, double x, double y, double z)> m_DataCallback;
    eSensorType     m_eTypeID;
    long            m_cRef;
    bool            m_fSwapXY;
};

class CSensorUtils
{
public:

    CSensorUtils();
    ~CSensorUtils();

    CSensorUtils(const CSensorUtils& r) = delete;
    CSensorUtils& operator = (const CSensorUtils& r) = delete;

    BOOL Initialize();
    VOID Uninitialize();

    bool GetSwapAccelerometerXY();
    void SetSwapAccelerometerXY(bool fSwap);
    bool GetSwapGyrometerXY();
    void SetSwapGyrometerXY(bool fSwap);
    void PulseAccelerometer();

    void SetGyrometerCallback(std::function<void(int nType, double x, double y, double z)> cb);
    void SetAccelerometerCallback(std::function<void(int nType, double x, double y, double z)> cb);

private:

    BOOL _InitializeGyrometer();
    BOOL _InitializeAccelerometer();
    BOOL CheckForSupportedDataFields(ISensor* pSensor);

private:

    ISensor*        m_pGyrometerSensor;
    ISensor*        m_pAccelerometerSensor;
    CSensorEvent*   m_pGyrometerEvent;
    CSensorEvent*   m_pAccelerometerEvent;
    BOOL            m_fInit;
    bool            m_fSwapAccelerometerXY;
    bool            m_fSwapGyrometerXY;
};

