
// ShowQLOptionDlg.h : header file
//

#pragma once


// CShowQLOptionDlg dialog
class CShowQLOptionDlg : public CDialogEx
{
// Construction
public:
	CShowQLOptionDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHOWQLOPTION_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	BOOL m_bShowHidden;
	afx_msg void OnBnClickedOk();
	BOOL m_bNoIcons;
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonPintotaskbar();
	BOOL m_bShowNoRecentItems;
	afx_msg void OnDeltaposSpinRecentitems(NMHDR* pNMHDR, LRESULT* pResult);
	CSpinButtonCtrl m_spinRecentItems;
	CEdit m_editRecentItems;
	int m_nSpinPos;
};
