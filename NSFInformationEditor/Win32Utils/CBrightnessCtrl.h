#pragma once

#include <initguid.h>
#include <cguid.h>
#include <atlcomcli.h>
#include <atlcoll.h>
#include <comdef.h>
#include <WbemIdl.h>
#include <tchar.h>
#pragma comment(lib, "wbemuuid.lib")

typedef void (*PFN_CALLBACK)(DWORD dwBrightness, LPVOID lpContext);

class CBrightnessCtrl : public IWbemObjectSink
{
public:
    CBrightnessCtrl();
    ~CBrightnessCtrl(void);

public:
    BOOL Get(DWORD* pdwValue);
    BOOL Set(DWORD dwValue);

    BOOL Initialize(void);
    void Uninitialize(void);
    void SetNotifyCallBack(PFN_CALLBACK pCallback, LPVOID lpContext);

public:
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
    virtual HRESULT STDMETHODCALLTYPE Indicate(LONG lObjectCount, IWbemClassObject __RPC_FAR* __RPC_FAR* apObjArray);
    virtual HRESULT STDMETHODCALLTYPE SetStatus(
        /* [in] */ LONG lFlags,
        /* [in] */ HRESULT hResult,
        /* [in] */ BSTR strParam,
        /* [in] */ IWbemClassObject __RPC_FAR* pObjParam
    );

private:
    LONG m_lRefCount;

    IWbemLocator* m_pLocator;
    IWbemServices* m_pService;
    IUnsecuredApartment* m_pUnsecuredApartment;
    IUnknown* m_pStubUnkown;
    IWbemObjectSink* m_pStubSink;

    PFN_CALLBACK m_pCallback;
    LPVOID m_pContext;
    BOOL m_bComInit;
    BOOL m_bValid;
};