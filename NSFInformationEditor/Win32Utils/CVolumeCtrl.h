#pragma once

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <corecrt_math.h>

typedef void (CALLBACK* PFN_NOTIFYCALLBACK)(DWORD dwVolume, BOOL bMuted, LPVOID lpContext);

class CVolumeCtrl : public IAudioEndpointVolumeCallback
{
public:
    CVolumeCtrl();
    ~CVolumeCtrl(void);

public:

    BOOL Initialize(void);
    void Uninitialize(void);

    BOOL Notify(BOOL bRegister);
    void SetNotifyCallBack(PFN_NOTIFYCALLBACK pCallback, LPVOID lpContext);

    BOOL GetVolume(DWORD* pdwVolume);
    BOOL SetVolume(DWORD dwVolume);
    BOOL GetMute(BOOL* pbMute);
    BOOL SetMute(BOOL bMute);


public:
    STDMETHODIMP_(ULONG)AddRef()
    { 
        return InterlockedIncrement(&m_lRefCount); 
    }

    STDMETHODIMP_(ULONG)Release()
    {
        LONG lRet = InterlockedDecrement(&m_lRefCount);
        if (0 == lRet)
        { 
            delete this; 
        }

        return lRet;
    }

    STDMETHODIMP QueryInterface(REFIID IID, void** ReturnValue)
    {
        if ((IID == IID_IUnknown) || (IID == __uuidof(IAudioEndpointVolumeCallback)))
        {
            *ReturnValue = static_cast<IUnknown*>(this);
            AddRef();
            return S_OK;
        }

        *ReturnValue = NULL;

        return E_NOINTERFACE;
    }

    STDMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotificationData)
    {
        if ((NULL != pNotificationData) && (NULL != m_pCallback))
        {
            DWORD dwVolume = (DWORD)round(pNotificationData->fMasterVolume * 100.0f);
            m_pCallback(dwVolume, pNotificationData->bMuted, m_pContext);
        }

        return S_OK;
    }

private:
    LONG m_lRefCount;

    IMMDeviceEnumerator* m_pEnumerator;
    IMMDevice* m_pDevice;
    IAudioEndpointVolume* m_pVolume;
    PFN_NOTIFYCALLBACK m_pCallback;
    LPVOID m_pContext;
    BOOL m_bComInit;
};
