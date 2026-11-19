#include "CBrightnessCtrl.h"

CBrightnessCtrl::CBrightnessCtrl()
    : m_pLocator(NULL)
    , m_pService(NULL)
    , m_pUnsecuredApartment(NULL)
    , m_pStubUnkown(NULL)
    , m_pStubSink(NULL)
    , m_bValid(FALSE)
    , m_lRefCount(1)
    , m_bComInit(FALSE)
    , m_pCallback(NULL)
    , m_pContext(NULL)
{

}

CBrightnessCtrl::~CBrightnessCtrl(void)
{
    Uninitialize();
}

BOOL CBrightnessCtrl::Initialize(void)
{
    HRESULT hr = S_OK;

    do
    {
        hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr))
        {
            break;
        }

        m_bComInit = TRUE;
        hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
        if ((S_OK != hr) && (RPC_E_TOO_LATE != hr))
        {
            break;
        }

        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&m_pLocator);
        if (FAILED(hr))
        {
            break;
        }

        BSTR strNetworkResource = ::SysAllocString(L"ROOT\\WMI");
        hr = m_pLocator->ConnectServer(strNetworkResource, NULL, NULL, 0, NULL, 0, 0, &m_pService);
        ::SysFreeString(strNetworkResource);
        if (FAILED(hr))
        {
            break;
        }

        hr = ::CoSetProxyBlanket(m_pService, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        if (FAILED(hr))
        {
            break;
        }

        hr = ::CoCreateInstance(CLSID_UnsecuredApartment, NULL, CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment, (void**)&m_pUnsecuredApartment);
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pUnsecuredApartment->CreateObjectStub(this, &m_pStubUnkown);
        if (FAILED(hr))
        {
            break;
        }

        hr = m_pStubUnkown->QueryInterface(IID_IWbemObjectSink, (void**)&m_pStubSink);
        if (FAILED(hr))
        {
            break;
        }

        BSTR strQueryLanguage = ::SysAllocString(L"WQL");
        BSTR strQuery = ::SysAllocString(L"SELECT * FROM WmiMonitorBrightnessEvent");
        hr = m_pService->ExecNotificationQueryAsync(strQueryLanguage, strQuery, WBEM_FLAG_SEND_STATUS, NULL, m_pStubSink);
        ::SysFreeString(strQueryLanguage);
        ::SysFreeString(strQuery);
        if (FAILED(hr))
        {
            break;
        }

        m_bValid = TRUE;

    } while (FALSE);

    if (!m_bValid)
    {
        Uninitialize();
    }

    return m_bValid;
}

void CBrightnessCtrl::Uninitialize(void)
{
    if ((NULL != m_pService) && (NULL != m_pStubSink))
    {
        m_pService->CancelAsyncCall(m_pStubSink);
    }

    if (NULL != m_pStubSink)
    {
        m_pStubSink->Release(); m_pStubSink = NULL;
    }

    if (NULL != m_pStubUnkown)
    {
        m_pStubUnkown->Release();
        m_pStubUnkown = NULL;
    }

    if (NULL != m_pUnsecuredApartment)
    {
        m_pUnsecuredApartment->Release();
        m_pUnsecuredApartment = NULL;
    }

    if (NULL != m_pLocator)
    {
        m_pLocator->Release();
        m_pLocator = NULL;
    }

    if (NULL != m_pService)
    {
        m_pService->Release();
        m_pService = NULL;
    }

    if (m_bComInit)
    {
        ::CoUninitialize();
    }
}

void CBrightnessCtrl::SetNotifyCallBack(PFN_CALLBACK pCallback, LPVOID lpContext)
{
    m_pCallback = pCallback;
    m_pContext = lpContext;
}

BOOL CBrightnessCtrl::Get(DWORD* pdwValue)
{
    HRESULT hr = S_OK;
    BOOL bRet = FALSE;
    IEnumWbemClassObject* pEnumerator = NULL;

    do
    {
        if (NULL == pdwValue) 
        { 
            break; 
        }

        if (!m_bValid) 
        { 
            break; 
        }

        _ASSERT(NULL != m_pService);

        BSTR strQueryLanguage = ::SysAllocString(L"WQL");
        BSTR strQuery = ::SysAllocString(L"SELECT * FROM WmiMonitorBrightnessMethods");
        hr = m_pService->ExecQuery(strQueryLanguage, strQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
        ::SysFreeString(strQueryLanguage);
        ::SysFreeString(strQuery);
        if (FAILED(hr)) 
        { 
            break; 
        }

        IWbemClassObject* papObject = NULL;

        while (NULL != pEnumerator)
        {
            ULONG uReturned = 0;

            hr = pEnumerator->Next(WBEM_INFINITE, 1, &papObject, &uReturned);
            if (FAILED(hr) || ((0 == uReturned))) 
            { 
                break; 
            }

            VARIANT var;
            VariantInit(&var);

            hr = papObject->Get(TEXT("CurrentBrightness"), 0, &var, 0, 0);
            if (FAILED(hr)) 
            { 
                break; 
            }

            *pdwValue = (DWORD)var.intVal;

            VariantClear(&var);

            bRet = TRUE;
        }

        if (NULL != papObject) { papObject->Release(); papObject = NULL; }

        // Completed
    } while (FALSE);

    if (NULL != pEnumerator) { pEnumerator->Release(); pEnumerator = NULL; }

    return bRet;
}

BOOL CBrightnessCtrl::Set(DWORD dwValue)
{
    HRESULT hr = S_OK;
    BOOL bRet = FALSE;

    IEnumWbemClassObject* pEnumerator = NULL;
    IWbemClassObject* pClsObject = NULL;
    IWbemClassObject* pInSignature = NULL;
    IWbemClassObject* pNewInstance = NULL;

    do
    {
        if (!m_bValid) 
        { 
            break; 
        }

        _ASSERT(NULL != m_pService);

        BSTR strQueryLanguage = ::SysAllocString(L"WQL");
        BSTR strQuery = ::SysAllocString(L"SELECT * FROM WmiMonitorBrightnessMethods");
        hr = m_pService->ExecQuery(strQueryLanguage, strQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
        ::SysFreeString(strQueryLanguage);
        ::SysFreeString(strQuery);
        if (FAILED(hr)) 
        { 
            break; 
        }

        IWbemClassObject* papObject = NULL;
        while (NULL != pEnumerator)
        {
            if (NULL != papObject) 
            { 
                papObject->Release(); 
                papObject = NULL; 
            }

            ULONG ulReturned = 0;
            hr = pEnumerator->Next(WBEM_INFINITE, 1, &papObject, &ulReturned);
            if (FAILED(hr) || (0 == ulReturned)) 
            { 
                break; 
            }

            BSTR strMethods = ::SysAllocString(L"WmiMonitorBrightnessMethods");
            hr = m_pService->GetObject(strMethods, 0, NULL, &pClsObject, NULL);
            ::SysFreeString(strMethods);
            if (FAILED(hr)) 
            { 
                break; 
            }

            hr = pClsObject->GetMethod(L"WmiSetBrightness", 0, &pInSignature, NULL);
            if (FAILED(hr))
            {
                break;
            }

            hr = pInSignature->SpawnInstance(0, &pNewInstance);
            if (FAILED(hr))
            {
                break;
            }

            VARIANT var;
            VariantInit(&var);
            V_VT(&var) = VT_BSTR;
            V_BSTR(&var) = SysAllocString(L"0");

            hr = pNewInstance->Put(L"Timeout", 0, &var, CIM_UINT32);
            if (FAILED(hr)) 
            { 
                VariantClear(&var); 
                break; 
            }

            VariantClear(&var);
            VariantInit(&var);
            V_VT(&var) = VT_I4;
            V_I4(&var) = (INT)dwValue;

            hr = pNewInstance->Put(L"Brightness", 0, &var, CIM_UINT8);
            if (FAILED(hr))
            {
                VariantClear(&var);
                break;
            }

            VariantClear(&var);
            VariantInit(&var);
            papObject->Get(L"__PATH", 0, &var, NULL, NULL);

            BSTR strMethodName = ::SysAllocString(L"WmiSetBrightness");
            hr = m_pService->ExecMethod(var.bstrVal, strMethodName, 0, NULL, pNewInstance, NULL, NULL);
            ::SysFreeString(strMethodName);
            if (FAILED(hr)) 
            { 
                VariantClear(&var); 
                break; 
            }

            VariantClear(&var);

            bRet = TRUE;
        }

        if (NULL != papObject) 
        { 
            papObject->Release(); 
            papObject = NULL; 
        }

    } while (FALSE);

    if (NULL != pNewInstance) 
    { 
        pNewInstance->Release(); 
        pNewInstance = NULL; 
    }

    if (NULL != pInSignature) 
    { 
        pInSignature->Release(); 
        pInSignature = NULL; 
    }

    if (NULL != pClsObject) 
    { 
        pClsObject->Release(); 
        pClsObject = NULL; 
    }

    if (NULL != pEnumerator)
    { 
        pEnumerator->Release(); 
        pEnumerator = NULL; 
    }

    return bRet;
}

ULONG CBrightnessCtrl::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

ULONG CBrightnessCtrl::Release()
{
    ULONG lRef = 0;

    do
    {
        if (0 == m_lRefCount) 
        { 
            break; 
        }

        lRef = InterlockedDecrement(&m_lRefCount);

        if (0 == lRef) 
        { 
            delete this; 
        }

    } while (FALSE);

    return lRef;
}

HRESULT CBrightnessCtrl::QueryInterface(REFIID riid, void** ppv)
{
    if ((IID_IUnknown == riid) || (IID_IWbemObjectSink == riid))
    {
        *ppv = (IWbemObjectSink*)this;
        AddRef();
        return WBEM_S_NO_ERROR;
    }

    return E_NOINTERFACE;
}

HRESULT CBrightnessCtrl::Indicate(LONG lObjectCount, IWbemClassObject __RPC_FAR* __RPC_FAR* pObjArray)
{
    HRESULT hr = S_OK;

    do
    {
        if (NULL == pObjArray) 
        { 
            break; 
        }

        for (LONG i = 0; i < lObjectCount; i++)
        {
            VARIANT var;
            VariantInit(&var);

            hr = pObjArray[i]->Get(L"Brightness", 0, &var, NULL, NULL);
            if ((NULL == pObjArray[i]) || FAILED(hr)) 
            { 
                break;
            }

            if (NULL != m_pCallback) 
            { 
                m_pCallback(var.intVal, m_pContext); 
            }

            VariantClear(&var);

            hr = WBEM_S_NO_ERROR;
            break;
        }

    } while (FALSE);

    return hr;
}

HRESULT CBrightnessCtrl::SetStatus(
    /* [in] */ LONG lFlags,
    /* [in] */ HRESULT hResult,
    /* [in] */ BSTR strParam,
    /* [in] */ IWbemClassObject __RPC_FAR* pObjParam
)
{
    UNREFERENCED_PARAMETER(lFlags);
    UNREFERENCED_PARAMETER(hResult);
    UNREFERENCED_PARAMETER(strParam);
    UNREFERENCED_PARAMETER(pObjParam);

    return WBEM_S_NO_ERROR;
}