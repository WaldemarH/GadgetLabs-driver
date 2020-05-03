#include "main.h"
#include "gl_asio.h"


HANDLE glAsio_c::GetDeviceIoControl()
{
	if ( m_hDeviceIoControl_1 != INVALID_HANDLE_VALUE )
	{
		return m_hDeviceIoControl_1;
	}
	else if ( m_hDeviceIoControl_2 != INVALID_HANDLE_VALUE )
	{
		return m_hDeviceIoControl_2;
	}
	else if ( m_hDeviceIoControl_3 != INVALID_HANDLE_VALUE )
	{
		return m_hDeviceIoControl_3;
	}
	else if ( m_hDeviceIoControl_4 != INVALID_HANDLE_VALUE )
	{
		return m_hDeviceIoControl_4;
	}
	else
	{
		return INVALID_HANDLE_VALUE;
	}
}
