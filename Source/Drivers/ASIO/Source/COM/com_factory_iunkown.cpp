#include "..\main.h"
#include "com_factory.h"
#include "wxdebug.h"

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
	return ( ++m_cRef );
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
	LONG	rc;

	if ( ( --m_cRef ) == 0)
	{
		delete this;
		rc = 0;
	}
	else
	{
		rc = m_cRef;
	}

	return rc;
}

STDMETHODIMP CClassFactory::QueryInterface( REFIID riid,void **ppv )
{
	CheckPointer( ppv,E_POINTER );
	ValidateReadWritePtr( ppv,sizeof(PVOID) );
	*ppv = NULL;

//Any interface on this object is the object pointer.
	if (
	  ( riid == IID_IUnknown )
	  ||
	  ( riid == IID_IClassFactory )
	)
	{
		*ppv = (LPVOID) this;

	// AddRef returned interface pointer
		( (LPUNKNOWN)*ppv )->AddRef();
		return NOERROR;
	}

	return ResultFromScode( E_NOINTERFACE );
}
