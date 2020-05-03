#pragma once

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

	public: virtual BOOL OnInitDialog();

//Text...
	static const char*		m_Request;
	afx_msg void OnBnClickedDonateEur();
	afx_msg void OnBnClickedDonateUsd();
	afx_msg void OnBnClickedExit();

//Browser...
	BOOL GetDefaultBrowser( CString& defaultBrowser );
};
