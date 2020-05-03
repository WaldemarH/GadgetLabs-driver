#pragma once

//Includes...


//COM Object stuff...
//
//32 bit
//Registry: HKEY_LOCAL_MACHINE\SOFTWARE\ASIO
//ClassID: {0469AB8B-8466-43a1-84ED-E4D38F97B9AF}
//
//64 bit
//Registry_32: HKEY_LOCAL_MACHINE\Software\Wow6432Node\ASIO
//ClassID: {0469AB8B-8466-43a1-84ED-E4D38F97B9AF}
//Registry_64: HKEY_LOCAL_MACHINE\SOFTWARE\ASIO
//ClassID: {051A46AB-C2D6-48e8-8320-187984242203}

#if defined( _X86_ )
	static const CLSID IID_GL_ASIO_DRIVER = { 0x0469AB8B, 0x8466, 0x43A1, { 0x84, 0xED, 0xE4, 0xD3, 0x8F, 0x97, 0xB9, 0xAF } };

	#define NAME_GL_ASIO_DRIVER				"GadgetLabs WavePro (32 bit)"
	#define NAME_GL_ASIO_DRIVER_WIDE		L"GadgetLabs WavePro (32 bit)"

	#define FILE_NAME_ASIO_DRIVER			"gadgetlabs_asio_32.dll"
#elif defined( _AMD64_ )
	static const CLSID IID_GL_ASIO_DRIVER = { 0x051A46AB, 0xC2D6, 0x48e8, { 0x83, 0x20, 0x18, 0x79, 0x84, 0x24, 0x22, 0x03 } };

	#define NAME_GL_ASIO_DRIVER				"GadgetLabs WavePro (64 bit)"
	#define NAME_GL_ASIO_DRIVER_WIDE		L"GadgetLabs WavePro (64 bit)"

	#define FILE_NAME_ASIO_DRIVER			"gadgetlabs_asio_64.dll"
#else
	#pragma message("Error: Bit type not defined.")
#endif
