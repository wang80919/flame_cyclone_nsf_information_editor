#include "CVolumeCtrl.h"

CVolumeCtrl::CVolumeCtrl()
    : m_pCallback(NULL)
    , m_pContext(NULL)
    , m_lRefCount(1)
    , m_pEnumerator(NULL)
    , m_pDevice(NULL)
    , m_pVolume(NULL)
    , m_bComInit(FALSE)
{

}

CVolumeCtrl::~CVolumeCtrl()
{
    Uninitialize();
}

BOOL CVolumeCtrl::GetVolume(DWORD* pdwVolume)
{
    HRESULT hr = S_OK;
    BOOL bRet = FALSE;

    do
    {
        if (NULL == m_pVolume)
        {
            break;
        }

        float fLevel = 0.0f;
        if (FAILED(hr = m_pVolume->GetMasterVolumeLevelScalar(&fLevel)))
        {
            break;
        }

        *pdwVolume = (DWORD)round(fLevel * 100.0f);
        bRet = TRUE;
    } while (FALSE);

    return bRet;
}

BOOL CVolumeCtrl::SetVolume(DWORD dwVolume)
{
    HRESULT hr = S_OK;
    BOOL bRet = FALSE;

    do
    {
        if (NULL == m_pVolume)
        {
            break;
        }

        float fLevel = (float)dwVolume / 100.0f;
        hr = m_pVolume->SetMasterVolumeLevelScalar(fLevel, &GUID_NULL);
        if (FAILED(hr))
        {
            break;
        }

        bRet = TRUE;
    } while (FALSE);

    return bRet;
}

BOOL CVolumeCtrl::GetMute(BOOL* pbMute)
{
    HRESULT hr = S_OK;
    BOOL bRet = FALSE;

    do
    {
        if (NULL == m_pVolume)
        {
            break;
        }

        hr = m_pVolume->GetMute(pbMute);
        if (FAILED(hr))
        {
            break;
        }

        bRet = TRUE;
    } while (FALSE);

    return bRet;
}

BOOL CVolumeCtrl::SetMute(BOOL bMute)
{
    HRESULT hr = S_OK;
    BOOL bRet = FALSE;

    do
    {
        if (NULL == m_pVolume)
        {
            break;
        }

        hr = m_pVolume->SetMute(bMute, &GUID_NULL);
        if (FAILED(hr))
        {
            break;
        }

        bRet = TRUE;
    } while (FALSE);

    return bRet;
}

BOOL CVolumeCtrl::Notify(BOOL bRegister)
{
    HRESULT hr = S_OK;
    BOOL bRet = FALSE;

    do
    {
        if (NULL == m_pVolume)
        {
            break;
        }

        if (bRegister)
        {
            hr = m_pVolume->RegisterControlChangeNotify(this);
            if (FAILED(hr))
            {
                break;
            }
        }
        else
        {
            hr = m_pVolume->UnregisterControlChangeNotify(this);
            if (FAILED(hr))
            {
                break;
            }
        }

        bRet = TRUE;
    } while (FALSE);

    return bRet;
}

void CVolumeCtrl::SetNotifyCallBack(PFN_NOTIFYCALLBACK pCallback, LPVOID lpContext)
{
    m_pCallback = pCallback;
    m_pContext = lpContext;
}

BOOL CVolumeCtrl::Initialize(void)
{
    HRESULT hr = S_OK;
    BOOL bRet = FALSE;

    do
    {
        hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr))
        {
            break;
        }

        m_bComInit = TRUE;

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&m_pEnumerator);
        if (FAILED(hr))
        {
            break;
        }
        
        hr = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_pDevice);
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&m_pVolume);
        if (FAILED(hr))
        {
            break;
        }

        bRet = TRUE;
    } while (FALSE);

    if (!bRet)
    {
        Uninitialize();
    }

    return bRet;
}

void CVolumeCtrl::Uninitialize(void)
{
    if (NULL != m_pVolume)
    {
        Notify(FALSE);
        m_pVolume->Release();
        m_pVolume = NULL;
    }

    if (NULL != m_pDevice)
    {
        m_pDevice->Release();
        m_pDevice = NULL;
    }

    if (NULL != m_pEnumerator)
    {
        m_pEnumerator->Release();
        m_pEnumerator = NULL;
    }

    if (m_bComInit)
    {
        ::CoUninitialize();
    }
}
