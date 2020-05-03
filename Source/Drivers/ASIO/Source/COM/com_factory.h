#pragma once

//Includes...
#include "combase.h"


//An instance of this is created by the DLLGetClassObject entrypoint it uses the CFactoryTemplate object it is given to support the IClassFactory interface.
class CClassFactory : public IClassFactory
{
//Constructor/Desctructor...
	public: CClassFactory( const CFactoryTemplate * );

	private: const CFactoryTemplate*		m_pTemplate;

//IUnknown
	public: STDMETHODIMP_(ULONG)AddRef();
	public: STDMETHODIMP_(ULONG)Release();

	public: STDMETHODIMP QueryInterface( REFIID riid, void ** ppv );

	private: ULONG							m_cRef;

//IClassFactory
	public: STDMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void **pv);
	public: STDMETHODIMP LockServer(BOOL fLock);

	//Allow DLLGetClassObject to know about global server lock status.
	public: static BOOL IsLocked();

	private: static int						m_cLocked;
};

