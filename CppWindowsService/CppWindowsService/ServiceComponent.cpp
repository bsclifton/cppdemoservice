/****************************** Module Header ******************************\
* Module Name:  ServiceComponent.cpp
* Project:      CppWindowsService
*
* COM component implementation for the service.
*
\***************************************************************************/

#include "ServiceComponent.h"
#include <olectl.h>
#include <initguid.h>
#include <new>

// Global class factory instance
CServiceComponentFactory* g_pClassFactory = NULL;
DWORD g_dwRegisterCookie = 0;
bool g_fComInitialized = false;

//=============================================================================
// CServiceComponent Implementation
//=============================================================================

CServiceComponent::CServiceComponent() : m_cRef(1), m_pTypeInfo(NULL) {
  LoadTypeInfo(&m_pTypeInfo);
}

CServiceComponent::~CServiceComponent() {
  if (m_pTypeInfo) {
    m_pTypeInfo->Release();
    m_pTypeInfo = NULL;
  }
}

// IUnknown
STDMETHODIMP CServiceComponent::QueryInterface(REFIID riid, void** ppvObject) {
  if (ppvObject == NULL) {
    return E_POINTER;
  }

  if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDispatch) ||
      IsEqualIID(riid, IID_IServiceComponent)) {
    *ppvObject = static_cast<IServiceComponent*>(this);
    AddRef();
    return S_OK;
  }

  *ppvObject = NULL;
  return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CServiceComponent::AddRef() {
  return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CServiceComponent::Release() {
  ULONG cRef = InterlockedDecrement(&m_cRef);
  if (cRef == 0) {
    delete this;
  }
  return cRef;
}

// IDispatch
STDMETHODIMP CServiceComponent::GetTypeInfoCount(UINT* pctinfo) {
  if (pctinfo == NULL)
    return E_POINTER;
  *pctinfo = (m_pTypeInfo != NULL) ? 1 : 0;
  return S_OK;
}

STDMETHODIMP CServiceComponent::GetTypeInfo(UINT iTInfo,
                                            LCID lcid,
                                            ITypeInfo** ppTInfo) {
  if (ppTInfo == NULL)
    return E_POINTER;

  if (iTInfo != 0)
    return DISP_E_BADINDEX;

  if (m_pTypeInfo) {
    m_pTypeInfo->AddRef();
    *ppTInfo = m_pTypeInfo;
    return S_OK;
  }

  return E_NOTIMPL;
}

STDMETHODIMP CServiceComponent::GetIDsOfNames(REFIID riid,
                                              LPOLESTR* rgszNames,
                                              UINT cNames,
                                              LCID lcid,
                                              DISPID* rgDispId) {
  if (m_pTypeInfo)
    return m_pTypeInfo->GetIDsOfNames(rgszNames, cNames, rgDispId);

  // Simple name-to-DISPID mapping
  if (cNames == 1) {
    if (_wcsicmp(rgszNames[0], L"GetServiceStatus") == 0) {
      rgDispId[0] = 1;
      return S_OK;
    } else if (_wcsicmp(rgszNames[0], L"ExecuteCommand") == 0) {
      rgDispId[0] = 2;
      return S_OK;
    }
  }

  return DISP_E_UNKNOWNNAME;
}

STDMETHODIMP CServiceComponent::Invoke(DISPID dispIdMember,
                                       REFIID riid,
                                       LCID lcid,
                                       WORD wFlags,
                                       DISPPARAMS* pDispParams,
                                       VARIANT* pVarResult,
                                       EXCEPINFO* pExcepInfo,
                                       UINT* puArgErr) {
  if (m_pTypeInfo)
    return m_pTypeInfo->Invoke(static_cast<IServiceComponent*>(this),
                               dispIdMember, wFlags, pDispParams, pVarResult,
                               pExcepInfo, puArgErr);

  // Direct method invocation
  HRESULT hr = S_OK;

  switch (dispIdMember) {
    // GetServerList
    case 1:
      if (pVarResult != NULL && pDispParams != NULL &&
          pDispParams->cArgs == 0) {
        BSTR bstrStatus = NULL;
        hr = GetServerList(&bstrStatus);
        if (SUCCEEDED(hr)) {
          VariantInit(pVarResult);
          pVarResult->vt = VT_BSTR;
          pVarResult->bstrVal = bstrStatus;
        }
      } else {
        hr = E_INVALIDARG;
      }
      break;

    default:
      hr = DISP_E_MEMBERNOTFOUND;
      break;
  }

  return hr;
}

// IServiceComponent
STDMETHODIMP CServiceComponent::GetServerList(BSTR* pJSON) {
  if (pJSON == NULL)
    return E_POINTER;

  // edited version of output captured 12/09/2025 from:
  // https://connect-api.guardianapp.com/api/v1.3/servers/all-server-regions/city-by-country
  *pJSON = SysAllocString(
      LR"json([{"name":"eu-dk","name-pretty":"Denmark","country":"Denmark","continent":"Europe","country-iso-code":"DK","region-precision":"country","cities":[{"name":"eu-cph","name-pretty":"Copenhagen","country":"Denmark","continent":"Europe","country-iso-code":"DK","region-precision":"city","latitude":12.578964037667108,"longitude":55.6843928421415,"server-count":8,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"}],"latitude":10.327889179924739,"longitude":55.30869540361275,"server-count":8,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"},{"name":"sa-co","name-pretty":"Colombia","country":"Colombia","continent":"South-America","country-iso-code":"CO","region-precision":"country","cities":[{"name":"co-bog","name-pretty":"Bogotá","country":"Colombia","continent":"South-America","country-iso-code":"CO","region-precision":"city","latitude":-74.09710852683152,"longitude":4.668568632587634,"server-count":10,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"}],"latitude":-73.086241587976,"longitude":3.7038764804835274,"server-count":10,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"},{"name":"eu-ua","name-pretty":"Ukraine","country":"Ukraine","continent":"Europe","country-iso-code":"UA","region-precision":"country","cities":[{"name":"eu-ky","name-pretty":"Kyiv","country":"Ukraine","continent":"Europe","country-iso-code":"UA","region-precision":"city","latitude":30.515942430190414,"longitude":50.458272443027305,"server-count":10,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"}],"latitude":31.270893817426348,"longitude":49.125122273171755,"server-count":10,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"},{"name":"asia-jp","name-pretty":"Japan","country":"Japan","continent":"Asia","country-iso-code":"JP","region-precision":"country","cities":[{"name":"jp-tky","name-pretty":"Tokyo","country":"Japan","continent":"Asia","country-iso-code":"JP","region-precision":"city","latitude":139.69105468946967,"longitude":35.68940973208437,"server-count":10,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"}],"latitude":139.28533963672928,"longitude":37.334150531438006,"server-count":10,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"},{"name":"eu-de","name-pretty":"Germany","country":"Germany","continent":"Europe","country-iso-code":"DE","region-precision":"country","cities":[{"name":"de-ber","name-pretty":"Berlin","country":"Germany","continent":"Europe","country-iso-code":"DE","region-precision":"city","latitude":13.377746483672231,"longitude":52.516632417384834,"server-count":10,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"},{"name":"de-fra","name-pretty":"Frankfurt","country":"Germany","continent":"Europe","country-iso-code":"DE","region-precision":"city","latitude":8.68072449294419,"longitude":50.111452544261404,"server-count":10,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"}],"latitude":10.333714874017673,"longitude":51.1887516565091,"server-count":20,"smart-routing-proxy-servers":0,"smart-routing-proxy-state":"none"},{"name":"na-usa","name-pretty":"USA","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"country","cities":[{"name":"us-atl","name-pretty":"Atlanta","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"city","latitude":-84.37330216158185,"longitude":33.75708620650122,"server-count":10,"smart-routing-proxy-servers":10,"smart-routing-proxy-state":"all"},{"name":"us-dfw","name-pretty":"Dallas","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"city","latitude":-96.8067432098152,"longitude":32.78321563339313,"server-count":12,"smart-routing-proxy-servers":12,"smart-routing-proxy-state":"all"},{"name":"us-ash","name-pretty":"Ashburn","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"city","latitude":-77.47774982785495,"longitude":39.03002630109117,"server-count":10,"smart-routing-proxy-servers":10,"smart-routing-proxy-state":"all"},{"name":"us-sjc","name-pretty":"San Jose","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"city","latitude":-121.89499986041636,"longitude":37.33935623583346,"server-count":15,"smart-routing-proxy-servers":15,"smart-routing-proxy-state":"all"},{"name":"us-den","name-pretty":"Denver","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"city","latitude":-104.99313771530903,"longitude":39.74680190141621,"server-count":10,"smart-routing-proxy-servers":10,"smart-routing-proxy-state":"all"},{"name":"us-sea","name-pretty":"Seattle","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"city","latitude":-122.33217344138617,"longitude":47.60636039205189,"server-count":10,"smart-routing-proxy-servers":10,"smart-routing-proxy-state":"all"},{"name":"us-chi","name-pretty":"Chicago","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"city","latitude":-87.65009116519116,"longitude":41.84985765685892,"server-count":15,"smart-routing-proxy-servers":15,"smart-routing-proxy-state":"all"},{"name":"us-nyc","name-pretty":"New York City","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"city","latitude":-74.00615560237677,"longitude":40.714292433330336,"server-count":36,"smart-routing-proxy-servers":22,"smart-routing-proxy-state":"some"},{"name":"us-la","name-pretty":"Los Angeles","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"city","latitude":-118.30039020568758,"longitude":34.118819662669615,"server-count":10,"smart-routing-proxy-servers":10,"smart-routing-proxy-state":"all"},{"name":"us-mia","name-pretty":"Miami","country":"USA","continent":"North-America","country-iso-code":"US","region-precision":"city","latitude":-80.21225580786931,"longitude":25.74633172978829,"server-count":10,"smart-routing-proxy-servers":10,"smart-routing-proxy-state":"all"}],"latitude":-101.69432971778862,"longitude":39.338586642335414,"server-count":138,"smart-routing-proxy-servers":124,"smart-routing-proxy-state":"some"}])json");
  return (*pJSON != NULL) ? S_OK : E_OUTOFMEMORY;
}

//=============================================================================
// CServiceComponentFactory Implementation
//=============================================================================

CServiceComponentFactory::CServiceComponentFactory() : m_cRef(1) {}

CServiceComponentFactory::~CServiceComponentFactory() {}

// IUnknown
STDMETHODIMP CServiceComponentFactory::QueryInterface(REFIID riid,
                                                      void** ppvObject) {
  if (ppvObject == NULL) {
    return E_POINTER;
  }

  if (riid == IID_IClassFactory || riid == IID_IUnknown) {
    *((IUnknown**)(ppvObject)) = this;
  } else {
    *ppvObject = NULL;
    return E_NOINTERFACE;
  }

  AddRef();
  return S_OK;
}

STDMETHODIMP_(ULONG) CServiceComponentFactory::AddRef() {
  return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CServiceComponentFactory::Release() {
  ULONG cRef = InterlockedDecrement(&m_cRef);
  if (cRef == 0) {
    delete this;
  }
  return cRef;
}

// IClassFactory
STDMETHODIMP CServiceComponentFactory::CreateInstance(IUnknown* pUnkOuter,
                                                      REFIID riid,
                                                      void** ppvObject) {
  if (ppvObject == NULL) {
    return E_POINTER;
  }

  if (pUnkOuter != NULL) {
    return CLASS_E_NOAGGREGATION;
  }
   
  CServiceComponent* pComponent = new (std::nothrow) CServiceComponent();
  if (pComponent == NULL) {
    return E_OUTOFMEMORY;
  }

  HRESULT hr = pComponent->QueryInterface(riid, ppvObject);
  pComponent->Release();
  return hr;
}

STDMETHODIMP CServiceComponentFactory::LockServer(BOOL fLock) {
  // For simplicity, we don't track server locks
  return S_OK;
}

//=============================================================================
// Helper Functions
//=============================================================================

HRESULT LoadTypeInfo(ITypeInfo** ppTypeInfo) {
  *ppTypeInfo = NULL;

  // For a simple implementation without MIDL-generated type library,
  // we'll return NULL and handle it gracefully
  // In a production environment, you would load the type library here
  return S_OK;
}

HRESULT RegisterCOMComponent() {
  HRESULT hr = S_OK;

  // Initialize COM
  // Note: CoInitializeEx can return S_FALSE if already initialized, which is OK
  hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (SUCCEEDED(hr) || hr == S_FALSE) {
    g_fComInitialized = true;
  } else if (hr == RPC_E_CHANGED_MODE) {
    // Already initialized elsewhere; proceed without CoUninitialize
    g_fComInitialized = false;
  } else {
    return hr;
  }

  // Initialize COM security - REQUIRED for services running under LocalService
  // This allows clients to connect to the COM object
  // Using RPC_C_AUTHN_LEVEL_NONE for local access (change if you need security)
  hr = CoInitializeSecurity(
      NULL,  // Security descriptor (NULL = default)
      -1,    // Count of entries in asAuthSvc (-1 = use default)
      NULL,  // Authentication services (NULL = use default)
      NULL,  // Reserved
      RPC_C_AUTHN_LEVEL_NONE,    // Authentication level (NONE for local access)
      RPC_C_IMP_LEVEL_IDENTIFY,  // Impersonation level
      NULL,                      // Authentication list (NULL = use default)
      EOAC_NONE,                 // Additional capabilities
      NULL);                     // Reserved

  // CoInitializeSecurity can return RPC_E_TOO_LATE if already set; treat as OK
  if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
    if (g_fComInitialized) {
      CoUninitialize();
      g_fComInitialized = false;
    }
    return hr;
  }

  // Create class factory
  g_pClassFactory = new CServiceComponentFactory();

  if (g_pClassFactory == NULL) {
    if (g_fComInitialized) {
      CoUninitialize();
      g_fComInitialized = false;
    }
    return E_OUTOFMEMORY;
  }

  // Register the class object suspended, then resume
  hr = CoRegisterClassObject(CLSID_ServiceComponent, g_pClassFactory,
                             CLSCTX_LOCAL_SERVER,
                             REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED,
                             &g_dwRegisterCookie);

  if (FAILED(hr)) {
    g_pClassFactory->Release();
    g_pClassFactory = NULL;
    if (g_fComInitialized) {
      CoUninitialize();
      g_fComInitialized = false;
    }
    return hr;
  }

  hr = CoResumeClassObjects();
  if (FAILED(hr)) {
    CoRevokeClassObject(g_dwRegisterCookie);
    g_dwRegisterCookie = 0;
    g_pClassFactory->Release();
    g_pClassFactory = NULL;
    if (g_fComInitialized) {
      CoUninitialize();
      g_fComInitialized = false;
    }
    return hr;
  }

  return S_OK;
}

HRESULT UnregisterCOMComponent() {
  if (g_dwRegisterCookie != 0) {
    CoRevokeClassObject(g_dwRegisterCookie);
    g_dwRegisterCookie = 0;
  }

  if (g_pClassFactory) {
    g_pClassFactory->Release();
    g_pClassFactory = NULL;
  }

  if (g_fComInitialized) {
    CoUninitialize();
    g_fComInitialized = false;
  }
  return S_OK;
}

HRESULT RegisterCOMInRegistry(PWSTR pszServicePath) {
  HKEY hKey = NULL;
  LONG lResult = 0;
  wchar_t szKeyPath[512];
  wchar_t szCLSID[64];
  wchar_t szAppID[64];
  const wchar_t SERVICE_NAME[] = L"CppWindowsService";
  wchar_t szProgID[] = L"ServiceComponent.ServiceComponent.1";
  wchar_t szVersionIndependentProgID[] = L"ServiceComponent.ServiceComponent";

  // Convert CLSID/AppID/IID/LIBID to string
  StringFromGUID2(CLSID_ServiceComponent, szCLSID, ARRAYSIZE(szCLSID));
  StringFromGUID2(APPID_ServiceComponent, szAppID, ARRAYSIZE(szAppID));
  wchar_t szIID[64];
  StringFromGUID2(IID_IServiceComponent, szIID, ARRAYSIZE(szIID));
  wchar_t szLIBID[64];
  StringFromGUID2(LIBID_ServiceComponentLib, szLIBID, ARRAYSIZE(szLIBID));

  // Register Interface for marshalling (required for cross-process COM calls)
  // Standard OLE automation marshaler for dual interfaces
  const wchar_t PSOA_INTERFACE_MARSHALER[] = L"{00020424-0000-0000-C000-000000000046}";
  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\Interface\\%s", szIID);
  lResult =
      RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
  if (lResult == ERROR_SUCCESS) {
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)L"IServiceComponent",
                  sizeof(L"IServiceComponent"));
    HKEY hSubkey;
    swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\Interface\\%s\\ProxyStubClsid32", szIID);
    lResult =
        RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubkey,
                             NULL);
    if (lResult == ERROR_SUCCESS) {
      RegSetValueEx(
          hSubkey, NULL, 0, REG_SZ,
          (LPBYTE)PSOA_INTERFACE_MARSHALER,
          (DWORD)(wcslen(PSOA_INTERFACE_MARSHALER) + 1) * sizeof(wchar_t));
      RegCloseKey(hSubkey);
    }

    swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\Interface\\%s\\TypeLib",
               szIID);
    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubkey,
                             NULL);
    if (lResult == ERROR_SUCCESS) {
      RegSetValueEx(hSubkey, NULL, 0, REG_SZ, (LPBYTE)szLIBID,
                    (DWORD)(wcslen(szLIBID) + 1) * sizeof(wchar_t));
      RegCloseKey(hSubkey);
    }
    RegCloseKey(hKey);
    hKey = NULL;
  }

  // Register CLSID and AppID (no LocalServer32 for service activation)
  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\CLSID\\%s", szCLSID);
  lResult =
      RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
  if (lResult == ERROR_SUCCESS) {
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)L"ServiceComponent",
                  sizeof(L"ServiceComponent"));
    RegSetValueEx(hKey, L"AppID", 0, REG_SZ, (LPBYTE)szAppID,
                  (DWORD)(wcslen(szAppID) + 1) * sizeof(wchar_t));

    HKEY hSubkey;
    swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\CLSID\\%s\\TypeLib", szCLSID);
    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubkey,
                             NULL);
    if (lResult == ERROR_SUCCESS) {
      RegSetValueEx(hSubkey, NULL, 0, REG_SZ, (LPBYTE)szLIBID,
                    (DWORD)(wcslen(szLIBID) + 1) * sizeof(wchar_t));
      RegCloseKey(hSubkey);
    }

    RegCloseKey(hKey);
    hKey = NULL;
  }

  // AppID key with LocalService
  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\AppID\\%s", szAppID);
  lResult =
      RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
  if (lResult == ERROR_SUCCESS) {
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)L"ServiceComponent",
                  sizeof(L"ServiceComponent"));
    RegSetValueEx(hKey, L"LocalService", 0, REG_SZ, (LPBYTE)SERVICE_NAME,
                  (DWORD)(wcslen(SERVICE_NAME) + 1) * sizeof(wchar_t));
    // Optional: store binary path for reference
    RegSetValueEx(hKey, L"ServiceBinary", 0, REG_SZ, (LPBYTE)pszServicePath,
                  (DWORD)(wcslen(pszServicePath) + 1) * sizeof(wchar_t));
    RegCloseKey(hKey);
    hKey = NULL;
  }

  // Register ProgID -> CLSID
  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\%s", szProgID);
  lResult =
      RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
  if (lResult == ERROR_SUCCESS) {
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)L"ServiceComponent",
                  sizeof(L"ServiceComponent"));
    RegCloseKey(hKey);
    hKey = NULL;

    swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\%s\\CLSID", szProgID);
    lResult =
        RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                       REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (lResult == ERROR_SUCCESS) {
      RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)szCLSID,
                    (DWORD)(wcslen(szCLSID) + 1) * sizeof(wchar_t));
      RegCloseKey(hKey);
      hKey = NULL;
    }
  }

  // Register VersionIndependentProgID -> CLSID/CurVer
  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\%s", szVersionIndependentProgID);
  lResult =
      RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
  if (lResult == ERROR_SUCCESS) {
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)L"ServiceComponent",
                  sizeof(L"ServiceComponent"));
    RegCloseKey(hKey);
    hKey = NULL;

    swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\%s\\CLSID",
               szVersionIndependentProgID);
    lResult =
        RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                       REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (lResult == ERROR_SUCCESS) {
      RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)szCLSID,
                    (DWORD)(wcslen(szCLSID) + 1) * sizeof(wchar_t));
      RegCloseKey(hKey);
      hKey = NULL;
    }

    swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\%s\\CurVer",
               szVersionIndependentProgID);
    lResult =
        RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                       REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (lResult == ERROR_SUCCESS) {
      RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)szProgID,
                    (DWORD)(wcslen(szProgID) + 1) * sizeof(wchar_t));
      RegCloseKey(hKey);
      hKey = NULL;
    }
  }

  // Register Type Library (required for dual interfaces)
  // Construct path to .tlb file (same directory as service executable, replace .exe with .tlb)
  wchar_t szTlbPath[MAX_PATH];
  wcscpy_s(szTlbPath, ARRAYSIZE(szTlbPath), pszServicePath);
  wchar_t* pExt = wcsrchr(szTlbPath, L'.');
  if (pExt != NULL) {
    wcscpy_s(pExt, MAX_PATH - (pExt - szTlbPath), L".tlb");
  } else {
    wcscat_s(szTlbPath, ARRAYSIZE(szTlbPath), L".tlb");
  }

  // Register TypeLib\{LIBID}\1.0\0
  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0", szLIBID);
  lResult =
      RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                     REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
  if (lResult == ERROR_SUCCESS) {
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)L"ServiceComponent 1.0 Type Library",
                  sizeof(L"ServiceComponent 1.0 Type Library"));
    RegCloseKey(hKey);
    hKey = NULL;

    // Register locale 0 (neutral)
    HKEY hSubkey;
    swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\TypeLib\\%s\\1.0\\0\\win64", szLIBID);
    lResult =
        RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyPath, 0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubkey,
                             NULL);
    if (lResult == ERROR_SUCCESS) {
      RegSetValueEx(hSubkey, NULL, 0, REG_SZ, (LPBYTE)szTlbPath,
                    (DWORD)(wcslen(szTlbPath) + 1) * sizeof(wchar_t));
      RegCloseKey(hSubkey);
      hSubkey = NULL;
    }
  }

  return (lResult == ERROR_SUCCESS) ? S_OK : HRESULT_FROM_WIN32(lResult);
}

HRESULT UnregisterCOMFromRegistry() {
  wchar_t szKeyPath[512];
  wchar_t szCLSID[64];
  wchar_t szAppID[64];
  wchar_t szIID[64];
  wchar_t szLIBID[64];
  wchar_t szProgID[] = L"ServiceComponent.ServiceComponent.1";
  wchar_t szVersionIndependentProgID[] = L"ServiceComponent.ServiceComponent";

  StringFromGUID2(CLSID_ServiceComponent, szCLSID, ARRAYSIZE(szCLSID));
  StringFromGUID2(APPID_ServiceComponent, szAppID, ARRAYSIZE(szAppID));
  StringFromGUID2(IID_IServiceComponent, szIID, ARRAYSIZE(szIID));
  StringFromGUID2(LIBID_ServiceComponentLib, szLIBID, ARRAYSIZE(szLIBID));

  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\CLSID\\%s", szCLSID);
  RegDeleteTree(HKEY_LOCAL_MACHINE, szKeyPath);

  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\AppID\\%s", szAppID);
  RegDeleteTree(HKEY_LOCAL_MACHINE, szKeyPath);

  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\Interface\\%s", szIID);
  RegDeleteTree(HKEY_LOCAL_MACHINE, szKeyPath);

  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\TypeLib\\%s", szLIBID);
  RegDeleteTree(HKEY_LOCAL_MACHINE, szKeyPath);

  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\%s", szProgID);
  RegDeleteTree(HKEY_LOCAL_MACHINE, szKeyPath);

  swprintf_s(szKeyPath, L"SOFTWARE\\Classes\\%s", szVersionIndependentProgID);
  RegDeleteTree(HKEY_LOCAL_MACHINE, szKeyPath);

  return S_OK;
}
