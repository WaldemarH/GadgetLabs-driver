#include "main.h"
#include "dlg_main.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL CControlPanelDlg::OnTooltip_NeedText( UINT id, NMHDR* pNMHDR, LRESULT* pResult )
{
	TOOLTIPTEXT*	pTooltip = (TOOLTIPTEXT*)pNMHDR;
	BOOL			Status = FALSE;

//Is it tooltip of a dlg item?
	if ( ( pTooltip->uFlags & TTF_IDISHWND ) != 0 )
	{
	//Yes -> get dlgItem ID.
		int			nID = ::GetDlgCtrlID( (HWND)pTooltip->hdr.idFrom );

	//Get selected card.
		ioctl_cardInfo_s*	pCard = Cards_Tree_GetSelected();
		if ( pCard != NULL )
		{
		//Get tooltip text.
			LPSTR		text_tooltip = NULL;		//if NULL property is not supported

			switch ( pCard->m_Type )
			{
			case ioctl_cardInfo_s::CARD_TYPE_424:
			{
				switch ( nID )
				{
				case IDC_IN_GAIN_1:
					text_tooltip = _T("-10dBV = higher input volume / +4dBu = lower input volume");
					break;
				//case IDC_IN_GAIN_2: break;
				//case IDC_IN_GAIN_3: break;
				//case IDC_IN_GAIN_4: break;
				//case IDC_IN_GAIN_5: break;
				//case IDC_IN_GAIN_6: break;
				//case IDC_IN_GAIN_7: break;
				//case IDC_IN_GAIN_8: break;
				case IDC_MON_1:
				case IDC_MON_2:
				case IDC_MON_3:
				case IDC_MON_4:
					if ( FALSE == ::IsWindowEnabled( (HWND)pTooltip->hdr.idFrom ) )
					{
						text_tooltip = _T("ASIO channel: use ASIO application's direct monitoring");
					}
					else
					{
						text_tooltip = _T("direct(hardware) monitoring");
					}
					break;
				//case IDC_MON_5:break;
				//case IDC_MON_6:break;
				//case IDC_MON_7:break;
				//case IDC_MON_8:break;
				case IDC_MUTE_1:
				case IDC_MUTE_2:
				case IDC_MUTE_3:
				case IDC_MUTE_4:
					text_tooltip = _T("mute output channels");
					break;
				//case IDC_MUTE_5:break;
				//case IDC_MUTE_6:break;
				//case IDC_MUTE_7:break;
				//case IDC_MUTE_8:break;
				//case IDC_OUT_GAIN_1:break;
				//case IDC_OUT_GAIN_2:break;
				//case IDC_OUT_GAIN_3:break;
				//case IDC_OUT_GAIN_4:break;
				//case IDC_OUT_GAIN_5:break;
				//case IDC_OUT_GAIN_6:break;
				//case IDC_OUT_GAIN_7:break;
				//case IDC_OUT_GAIN_8:break;
				}
				break;
			}
			case ioctl_cardInfo_s::CARD_TYPE_496:
			{
				switch ( nID )
				{
				case IDC_IN_GAIN_1:
					text_tooltip = _T("-10dBV = higher input volume / +4dBu = lower input volume");
					break;
				//case IDC_IN_GAIN_2: break;
				//case IDC_IN_GAIN_3: break;
				//case IDC_IN_GAIN_4: break;
				//case IDC_IN_GAIN_5: break;
				//case IDC_IN_GAIN_6: break;
				//case IDC_IN_GAIN_7: break;
				//case IDC_IN_GAIN_8: break;
				case IDC_MON_1:
				case IDC_MON_2:
				case IDC_MON_3:
				case IDC_MON_4:
					if ( FALSE == ::IsWindowEnabled( (HWND)pTooltip->hdr.idFrom ) )
					{
						text_tooltip = _T("ASIO channel: use ASIO application's direct monitoring");
					}
					else
					{
						text_tooltip = _T("direct(hardware) monitoring");
					}
					break;
				//case IDC_MON_5:break;
				//case IDC_MON_6:break;
				//case IDC_MON_7:break;
				//case IDC_MON_8:break;
				case IDC_MUTE_1:
				case IDC_MUTE_2:
				case IDC_MUTE_3:
				case IDC_MUTE_4:
					text_tooltip = _T("mute output channels");
					break;
				//case IDC_MUTE_5:break;
				//case IDC_MUTE_6:break;
				//case IDC_MUTE_7:break;
				//case IDC_MUTE_8:break;
				case IDC_OUT_GAIN_1:
					text_tooltip = _T("-10dBV = higher input volume / +4dBu = lower input volume");
					break;
				//case IDC_OUT_GAIN_2:break;
				//case IDC_OUT_GAIN_3:break;
				//case IDC_OUT_GAIN_4:break;
				//case IDC_OUT_GAIN_5:break;
				//case IDC_OUT_GAIN_6:break;
				//case IDC_OUT_GAIN_7:break;
				//case IDC_OUT_GAIN_8:break;
				}
				break;
			}
			case ioctl_cardInfo_s::CARD_TYPE_824:
			{
				switch ( nID )
				{
				case IDC_IN_GAIN_1:
				case IDC_IN_GAIN_2:
				case IDC_IN_GAIN_3:
				case IDC_IN_GAIN_4:
				case IDC_IN_GAIN_5:
				case IDC_IN_GAIN_6:
				case IDC_IN_GAIN_7:
				case IDC_IN_GAIN_8:
					text_tooltip = _T("-10dBV = higher input volume / +4dBu = lower input volume");
					break;
				case IDC_MON_1:
				case IDC_MON_2:
				case IDC_MON_3:
				case IDC_MON_4:
				case IDC_MON_5:
				case IDC_MON_6:
				case IDC_MON_7:
				case IDC_MON_8:
					if ( FALSE == ::IsWindowEnabled( (HWND)pTooltip->hdr.idFrom ) )
					{
						text_tooltip = _T("ASIO channel: use ASIO application's direct monitoring");
					}
					else
					{
						text_tooltip = _T("direct(hardware) monitoring");
					}
					break;
				case IDC_MUTE_1:
				case IDC_MUTE_2:
				case IDC_MUTE_3:
				case IDC_MUTE_4:
				case IDC_MUTE_5:
				case IDC_MUTE_6:
				case IDC_MUTE_7:
				case IDC_MUTE_8:
					text_tooltip = _T("mute output channels");
					break;
				case IDC_OUT_GAIN_1:
				case IDC_OUT_GAIN_2:
				case IDC_OUT_GAIN_3:
				case IDC_OUT_GAIN_4:
				case IDC_OUT_GAIN_5:
				case IDC_OUT_GAIN_6:
				case IDC_OUT_GAIN_7:
				case IDC_OUT_GAIN_8:
					text_tooltip = _T("-10dBV = higher input volume / +4dBu = lower input volume");
					break;
				}
				break;
			}
			}
			if ( text_tooltip == NULL )
			{
				text_tooltip = _T("Property not supported on current card.");

			}

		//Display tooltip...
			pTooltip->lpszText = text_tooltip;

		//Message processed.
			Status = TRUE;
		}
	}

//Exit..
	*pResult = 0;
	return Status;
}

