#include "main.h"
#include "gl_asio.h"
#include "gl_com_object.h"
#include ".\COM\combase.h"


CFactoryTemplate g_Templates[1] =
{
	{ NAME_GL_ASIO_DRIVER_WIDE, &IID_GL_ASIO_DRIVER, glAsio_c::CreateInstance, NULL }
};
int g_cTemplates = sizeof( g_Templates ) / sizeof( g_Templates[0] );


CUnknown* glAsio_c::CreateInstance
(
	LPUNKNOWN	pUnk,
	HRESULT*	phr
)
{
	return (CUnknown*)new glAsio_c( pUnk, phr );
};

STDMETHODIMP glAsio_c::NonDelegatingQueryInterface
(
	REFIID		riid,
	void**		ppvObject
)
{
	if ( riid == IID_GL_ASIO_DRIVER )
	{
		return GetInterface( this, ppvObject );
	}
	return CUnknown::NonDelegatingQueryInterface( riid, ppvObject );
}
