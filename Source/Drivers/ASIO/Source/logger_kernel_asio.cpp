#include "main.h"
#include "logger.h"
#include "..\..\Kernel\Source\driver_ioctl_asio.h"
#include "registry.h"

#ifdef ASIO_LOGGER
#pragma warning( disable : 4996 )

void logger_c::Logger_Channels_Open( IN char* pBuffer, IN void** ppVariables )
{
//Get data.
	ioctl_asio_channels_header_s*		pData_kernel = (ioctl_asio_channels_header_s*)ppVariables[0];

//Log...
	pBuffer += sprintf( pBuffer, "  data_kernel:\n" );
	pBuffer += sprintf( pBuffer, "    m_hAsio      = %d\n", pData_kernel->m_hAsio );

	switch ( pData_kernel->m_Request )
	{
	case ASIO_CHANNELS_OPEN:
		pBuffer += sprintf( pBuffer, "    m_Request    = ASIO_CHANNELS_OPEN\n" );
		break;
	case ASIO_CHANNELS_CLOSE:
		pBuffer += sprintf( pBuffer, "    m_Request    = ASIO_CHANNELS_CLOSE\n" );
		break;
	default:
		pBuffer += sprintf( pBuffer, "    m_Request    = %d\n", pData_kernel->m_Request );
		break;
	}
	switch ( pData_kernel->m_SampleType )
	{
	case SAMPLETYPE_32BIT_LSB24:
		pBuffer += sprintf( pBuffer, "    m_SampleType = SAMPLETYPE_32BIT_LSB24\n" );
		break;
	case SAMPLETYPE_32BIT_LSB:
		pBuffer += sprintf( pBuffer, "    m_SampleType = SAMPLETYPE_32BIT_LSB\n" );
		break;
	case SAMPLETYPE_32BIT_MSB24:
		pBuffer += sprintf( pBuffer, "    m_SampleType = SAMPLETYPE_32BIT_MSB24\n" );
		break;
	default:
		pBuffer += sprintf( pBuffer, "    m_SampleType = unknown (%d)\n", pData_kernel->m_SampleType );
		break;
	}
	
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( pData_kernel->m_Channels_Opened ); i++ )
	{
		pBuffer += sprintf( pBuffer, "      Card: %d\n", i );
		pBuffer += sprintf( pBuffer, "        m_Type          = %d\n", pData_kernel->m_Channels_Opened[i].m_Type );
		pBuffer += sprintf( pBuffer, "        m_SerialNumber  = %d\n", pData_kernel->m_Channels_Opened[i].m_SerialNumber );
		pBuffer += sprintf( pBuffer, "        m_Channels_IN   = 0x%x\n", pData_kernel->m_Channels_Opened[i].m_Channels_IN );
		pBuffer += sprintf( pBuffer, "        m_Channels_OUT  = 0x%x\n", pData_kernel->m_Channels_Opened[i].m_Channels_OUT );
	}

	return;
}

void logger_c::Logger_Channels_Open2( IN char* pBuffer, IN void** ppVariables )
{
//Get data.
	registry_data_s*		pData_registry = (registry_data_s*)ppVariables[0];

//Log...
	pBuffer += sprintf( pBuffer, "  data_registry:\n" );

	switch ( pData_registry->m_SampleType )
	{
	case SAMPLETYPE_32BIT_LSB24:
		pBuffer += sprintf( pBuffer, "    m_SampleType = SAMPLETYPE_32BIT_LSB24\n" );
		break;
	case SAMPLETYPE_32BIT_LSB:
		pBuffer += sprintf( pBuffer, "    m_SampleType = SAMPLETYPE_32BIT_LSB\n" );
		break;
	case SAMPLETYPE_32BIT_MSB24:
		pBuffer += sprintf( pBuffer, "    m_SampleType = SAMPLETYPE_32BIT_MSB24\n" );
		break;
	default:
		pBuffer += sprintf( pBuffer, "    m_SampleType = unknown (%d)\n", pData_registry->m_SampleType );
		break;
	}
	
	for ( int i = 0; i < ELEMENTS_IN_ARRAY( pData_registry->m_Channels_Opened ); i++ )
	{
		pBuffer += sprintf( pBuffer, "      Card: %d\n", i );
		pBuffer += sprintf( pBuffer, "        m_Type          = %d\n", pData_registry->m_Channels_Opened[i].m_Type );
		pBuffer += sprintf( pBuffer, "        m_SerialNumber  = %d\n", pData_registry->m_Channels_Opened[i].m_SerialNumber );
		pBuffer += sprintf( pBuffer, "        m_Channels_IN   = 0x%x\n", pData_registry->m_Channels_Opened[i].m_Channels_IN );
		pBuffer += sprintf( pBuffer, "        m_Channels_OUT  = 0x%x\n", pData_registry->m_Channels_Opened[i].m_Channels_OUT );
	}

	return;
}

#endif //ASIO_LOGGER
