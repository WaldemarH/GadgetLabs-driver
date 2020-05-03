#include "main.h"
#include "gl_register.h"
#include "gl_com_object.h"


// Server registration, called on REGSVR32.EXE "the dllname.dll"
extern "C" HRESULT _stdcall DllRegisterServer()
{
#if defined( _X86_ )
	LONG	rc = RegisterAsioDriver(
	  IID_GL_ASIO_DRIVER,								//CLSID
	  "gadgetlabs_asio_32.dll",							//DLL name
	  "GadgetLabs WavePro Series 32 bit ASIO Driver",	//ASIO name
	  "GadgetLabs WavePro Series 32 bit ASIO Driver",	//ASIO description
	  "Apartment"										//Thread type: single threaded
	);
	if ( rc != 0 )
	{
		char	text[128];

		sprintf_s( text, sizeof( text ), "Register Server failed ! (%d)", rc );
		MessageBox( 0, (LPCTSTR)text, (LPCTSTR)"GadgetLabs WavePro Series 32 bit ASIO Driver", MB_OK );
		return -1;
	}
#elif defined( _AMD64_ )
	LONG	rc = RegisterAsioDriver(
	  IID_GL_ASIO_DRIVER,								//CLSID
	  "gadgetlabs_asio_64.dll",							//DLL name
	  "GadgetLabs WavePro Series 64 bit ASIO Driver",	//ASIO name
	  "GadgetLabs WavePro Series 64 bit ASIO Driver",	//ASIO description
	  "Apartment"										//Thread type: single threaded
	);
	if ( rc != 0 )
	{
		char	text[128];

		sprintf_s( text, sizeof( text ), "Register Server failed ! (%d)", rc );
		MessageBox( 0, (LPCTSTR)text, (LPCTSTR)"GadgetLabs WavePro Series 64 bit ASIO Driver", MB_OK );
		return -1;
	}
#else
	#pragma message("Error: Bit type not defined.")
#endif

	return S_OK;
}

//Server unregistration
extern "C" HRESULT _stdcall DllUnregisterServer()
{
#if defined( _X86_ )
	LONG	rc = UnregisterAsioDriver(
	  IID_GL_ASIO_DRIVER,
	  "gadgetlabs_asio_32.dll",
	  "GadgetLabs WavePro Series 32 bit ASIO Driver"
	);
	if ( rc != 0 )
	{
		char	text[128];

		sprintf_s( text, sizeof( text ), "Unregister Server failed ! (%d)", rc );
		MessageBox( 0, (LPCTSTR)text, (LPCTSTR)"GadgetLabs WavePro Series 32 bit ASIO Driver", MB_OK );
		return -1;
	}
#elif defined( _AMD64_ )
	LONG	rc = UnregisterAsioDriver(
	  IID_GL_ASIO_DRIVER,
	  "gadgetlabs_asio_64.dll",
	  "GadgetLabs WavePro Series 64 bit ASIO Driver"
	);
	if ( rc != 0 )
	{
		char	text[128];

		sprintf_s( text, sizeof( text ), "Unregister Server failed ! (%d)", rc );
		MessageBox( 0, (LPCTSTR)text, (LPCTSTR)"GadgetLabs WavePro Series 64 bit ASIO Driver", MB_OK );
		return -1;
	}
#else
	#pragma message("Error: Bit type not defined.")
#endif

	return S_OK;
}