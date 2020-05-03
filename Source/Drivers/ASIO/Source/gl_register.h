#pragma once

//Includes...


//Register ASIO Driver
extern LONG RegisterAsioDriver( CLSID clsid, char* szdllname, char* szregname, char* szasiodesc, char* szthreadmodel );
extern LONG UnregisterAsioDriver( CLSID clsid, char* szdllname, char* szregname );


