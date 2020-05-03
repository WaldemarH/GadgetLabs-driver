#include "main.h"
#include "dlg_main.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


void CControlPanelDlg::OnTreeSelectionChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
//Set the checkBoxes data...
	ioctl_cardInfo_s*	pSelected = Cards_Tree_GetSelected();
	if (
	  ( pSelected == NULL )
	  ||
	  ( pSelected != m_Cards_Tree_lastSelectedCard )
	)
	{
		ChannelStatus_GetAll();
	}
	m_Cards_Tree_lastSelectedCard = pSelected;
}
