
// ShowQLOptionDlg.cpp : implementation file
//

#include "pch.h"
#include "../../profile/cpp/Profile/include/ambiesoft.profile.h"

#include "framework.h"
#include "ShowQLOption.h"
#include "ShowQLOptionDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Ambiesoft;
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	CString m_strInfo;
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
, m_strInfo(_T(""))
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_INFO, m_strInfo);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


