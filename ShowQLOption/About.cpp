
// ShowQLOptionDlg.cpp : implementation file
//

#include "pch.h"
#include "../../profile/cpp/Profile/include/ambiesoft.profile.h"
#include "../../lsMisc/GetVersionString.h"

#include "framework.h"
#include "ShowQLOption.h"
#include "ShowQLOptionDlg.h"
#include "afxdialogex.h"
#include "About.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Ambiesoft;

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
, m_strInfo(_T(""))
{
	m_strInfo = stdFormat(L"%s v%s",
		(LPCWSTR)AfxGetAppName(),
		GetVersionString(theApp.GetShowQLExe().c_str(), 3).c_str()).c_str();
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_INFO, m_strInfo);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()




BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(stdFormat(I18N(L"About %s"), L"ShowQL").c_str());

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
