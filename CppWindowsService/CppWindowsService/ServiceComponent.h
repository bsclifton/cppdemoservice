/****************************** Module Header ******************************\
* Module Name:  ServiceComponent.h
* Project:      CppWindowsService
*
* COM component interface and class definitions for the service.
*
\***************************************************************************/

#pragma once

#include <windows.h>
#include <comdef.h>
#include <atlbase.h>
#include <atlcom.h>

// Forward declarations
class CServiceComponent;

// Interface IIDs
// {134A2E61-32DA-4656-BBE1-E9A38C4C9DDE}
static const GUID IID_IServiceComponent = {
    0x134a2e61,
    0x32da,
    0x4656,
    {0xbb, 0xe1, 0xe9, 0xa3, 0x8c, 0x4c, 0x9d, 0xde}};

// Class ID
// {65E16120-F60D-47ED-B467-840DD40D609A}
static const GUID CLSID_ServiceComponent = {
    0x65e16120,
    0xf60d,
    0x47ed,
    {0xb4, 0x67, 0x84, 0xd, 0xd4, 0xd, 0x60, 0x9a}};

// AppID for service activation (distinct from CLSID)
// {F4F6B048-3A2A-4A39-B2F4-5F6D8B3A0A9C}
static const GUID APPID_ServiceComponent = {
    0xf4f6b048,
    0x3a2a,
    0x4a39,
    {0xb2, 0xf4, 0x5f, 0x6d, 0x8b, 0x3a, 0xa, 0x9c}};

// Type Library ID (LIBID)
// {FC0D4F9D-18FC-4007-8168-B61DA0D61FA4}
static const GUID LIBID_ServiceComponentLib = {
    0xfc0d4f9d,
    0x18fc,
    0x4007,
    {0x81, 0x68, 0xb6, 0x1d, 0xa0, 0xd6, 0x1f, 0xa4}};

// IServiceComponent interface
DECLARE_INTERFACE_(IServiceComponent, IDispatch) {
  STDMETHOD(GetTypeInfoCount)(UINT * pctinfo) PURE;
  STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo * *ppTInfo) PURE;
  STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR * rgszNames, UINT cNames,
                           LCID lcid, DISPID * rgDispId) PURE;
  STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
                    DISPPARAMS * pDispParams, VARIANT * pVarResult,
                    EXCEPINFO * pExcepInfo, UINT * puArgErr) PURE;

  // Custom methods
  STDMETHOD(GetServerList)(BSTR* pJSON) PURE;
};

// CServiceComponent class
class CServiceComponent : public IServiceComponent {
 public:
  CServiceComponent();
  virtual ~CServiceComponent();

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

  // IDispatch
  STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
  STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
  STDMETHOD(GetIDsOfNames)(REFIID riid,
                           LPOLESTR* rgszNames,
                           UINT cNames,
                           LCID lcid,
                           DISPID* rgDispId);
  STDMETHOD(Invoke)(DISPID dispIdMember,
                    REFIID riid,
                    LCID lcid,
                    WORD wFlags,
                    DISPPARAMS* pDispParams,
                    VARIANT* pVarResult,
                    EXCEPINFO* pExcepInfo,
                    UINT* puArgErr);

  // IServiceComponent
  STDMETHOD(GetServerList)(BSTR* pJSON);

 private:
  LONG m_cRef;
  ITypeInfo* m_pTypeInfo;
};

// CServiceComponentFactory class
class CServiceComponentFactory : public IClassFactory {
 public:
  CServiceComponentFactory();
  virtual ~CServiceComponentFactory();

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

  // IClassFactory
  STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, void** ppvObject);
  STDMETHOD(LockServer)(BOOL fLock);

 private:
  LONG m_cRef;
};

// Helper functions
HRESULT RegisterCOMComponent();
HRESULT UnregisterCOMComponent();
HRESULT RegisterCOMInRegistry(PWSTR pszServicePath);
HRESULT UnregisterCOMFromRegistry();
HRESULT LoadTypeInfo(ITypeInfo** ppTypeInfo);
