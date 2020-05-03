#include "..\main.h"
#include "com_factory.h"
#include "wxdebug.h"


int		CClassFactory::m_cLocked = 0;		//Process-wide dll locked state.


STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,REFIID riid,void **pv)
{
	CheckPointer( pv, E_POINTER )
	ValidateReadWritePtr( pv, sizeof(void *) );

//Enforce the normal OLE rules regarding interfaces and delegation.
	if (pUnkOuter != NULL)
	{
		if (IsEqualIID(riid,IID_IUnknown) == FALSE)
		{
			return ResultFromScode(E_NOINTERFACE);
		}
	}

//Create the new object through the derived class's create function.
	HRESULT hr = NOERROR;
	CUnknown *pObj = m_pTemplate->CreateInstance( pUnkOuter, &hr );

	CheckPointer( pObj, E_OUTOFMEMORY );

//Delete the object if we got a construction error.
	if ( FAILED(hr) )
	{
		delete pObj;
		return hr;
	}

//Get a reference counted interface on the object.
//
//We wrap the non-delegating QI with NDAddRef & NDRelease.
//This protects any outer object from being prematurely released by an inner object that may have to be created in
//order to supply the requested interface.
	pObj->NonDelegatingAddRef();
	hr = pObj->NonDelegatingQueryInterface(riid, pv);
	pObj->NonDelegatingRelease();

//Note that if NonDelegatingQueryInterface fails, it will not increment the ref count,
//so the NonDelegatingRelease will drop the ref back to zero and the object will "self-destruct".
//Hence we don't need additional tidy-up code to cope with NonDelegatingQueryInterface failing.
	if ( SUCCEEDED(hr) )
	{
		ASSERT(*pv);
	}

	return hr;
}

STDMETHODIMP CClassFactory::LockServer( BOOL fLock )
{
	if ( fLock )
	{
		m_cLocked++;
	}
	else
	{
		m_cLocked--;
	}
	return NOERROR;
}

BOOL CClassFactory::IsLocked()
{
	return ( m_cLocked > 0 );
};
