
// ShowQLOption.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CShowQLOptionApp:
// See ShowQLOption.cpp for the implementation of this class
//

class CShowQLOptionApp : public CWinApp
{
public:
	CShowQLOptionApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

protected:
	CSessionGlobalMemory<LONGLONG>* m_pSingleHandle = nullptr;
public:
	void SetSingleHWND(HWND h) {
		ASSERT(!m_pSingleHandle);
		ASSERT(h && ::IsWindow(h));
		m_pSingleHandle = new CSessionGlobalMemory<LONGLONG>("ShowQLOptionSingleHWND");
		*m_pSingleHandle = (LONGLONG)h;
	}
	void ReleaseSingleHWND() {
		ASSERT(m_pSingleHandle);
		delete m_pSingleHandle;
		m_pSingleHandle = NULL;
	}
	HWND GetSingleHWND() {
		ASSERT(!m_pSingleHandle);
		m_pSingleHandle = new CSessionGlobalMemory<LONGLONG>("ShowQLOptionSingleHWND");
		return (HWND)(LONGLONG)*m_pSingleHandle;
	}
};

extern CShowQLOptionApp theApp;
