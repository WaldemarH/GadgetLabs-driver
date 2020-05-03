#include "main.h"
#include "dlg_main.h"


void CControlPanelDlg::OnBnClickedBufferSize32()
{
	BufferSize_Set( 32 );
}

void CControlPanelDlg::OnBnClickedBufferSize64()
{
	BufferSize_Set( 64 );
}

void CControlPanelDlg::OnBnClickedBufferSize128()
{
	BufferSize_Set( 128 );
}

void CControlPanelDlg::OnBnClickedBufferSize256()
{
	BufferSize_Set( 256 );
}

void CControlPanelDlg::OnBnClickedBufferSize512()
{
	BufferSize_Set( 512 );
}

void CControlPanelDlg::OnBnClickedBufferSize1024()
{
	BufferSize_Set( 1024 );
}
