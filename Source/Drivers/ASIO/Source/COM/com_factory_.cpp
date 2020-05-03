#include "..\main.h"
#include "com_factory.h"
#include "wxdebug.h"


CClassFactory::CClassFactory( const CFactoryTemplate *pTemplate )
{
	m_cRef = 0;
	m_pTemplate = pTemplate;
}


