#include <Windows.h>
#include <shlobj_core.h>

#include <string>
#include <map>
#include "../../lsMisc/CreateSimpleWindow.h"
#include "../../lsMisc/GetLastErrorString.h"
#include "../../lsMisc/GetFilesInfo.h"
#include "../../lsMisc/OpenCommon.h"
#include "../../lsMisc/DebugMacro.h"
#include "../../lsMisc/stdosd/stdosd.h"

#pragma comment(lib, "Shell32.lib")

using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;
using namespace std;

#define APPNAME L"ShowQL"
#define I18N(s) s

HMENU ghPopup;
TCHAR szT[MAX_PATH];

#define MENUID_DUMMY 1
#define MENUID_START 2
WORD gMenuIndex;
map<UINT, wstring> gCmdMap;
// wstring glastSelectedPopup;
map<HMENU, wstring> gPopupMap;
wstring qlRoot;
void ErrorExit(const wchar_t* pMessage, int ret = -1)
{
	MessageBox(nullptr,
		pMessage,
		APPNAME,
		MB_ICONERROR);
	ExitProcess(ret);
}
void ErrorExit(const wstring& message)
{
	ErrorExit(message.c_str());
}
void ErrorExit(DWORD le)
{
	ErrorExit(GetLastErrorString(le).c_str(), le);
}

//wstring GetSelectedMenuString()
//{
//	wstring ret;
//	// if (hMenu != ghPopup)
//	{
//		for (int i = 0; i < GetMenuItemCount(ghPopup); ++i)
//		{
//			MENUITEMINFO mii = { 0 };
//			mii.cbSize = sizeof(mii);
//			mii.fMask= MIIM_DATA
//			GetMenuItemInfo(ghPopup, i, TRUE, &mii);
//		}
//	}
//}
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		//case WM_MENUSELECT:
		//{
		//	WORD index = LOWORD(wParam);
		//	HMENU hMenu = (HMENU)lParam;
		//	GetMenuString(hMenu, index, szT, _countof(szT), MF_BYPOSITION);
		//	DTRACE(wstring() + L"WM_MENUSELECT : " + szT);
		//	glastSelectedPopup = szT;
		//}
		//break;
		case WM_INITMENUPOPUP:
		{
			HMENU hMenu = (HMENU)wParam;
			WORD index = LOWORD(lParam);
			if (true) //hMenu == ghPopup)
			{
				while (DeleteMenu(hMenu, 0, MF_BYPOSITION))
					;
				
				wstring sel = gPopupMap[hMenu];
				DTRACE(L"sel=" + sel);
				wstring qlDir = sel.empty() ? qlRoot : sel;
				DTRACE(L"qlDir=" + qlDir);
				FILESINFOW fi;
				if (!GetFilesInfoW(qlDir.c_str(), fi))
					ErrorExit(GetLastError());
				for (UINT i = 0; i < fi.GetCount(); ++i)
				{
					if (fi[i].dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
						continue;
					if (fi[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						HMENU hPopup = CreatePopupMenu();
						InsertMenu(hPopup, 0, MF_BYCOMMAND, MENUID_DUMMY, L"<DUMMY>");
						AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hPopup, fi[i].cFileName);
						gPopupMap[hPopup] = stdCombinePath(qlDir, fi[i].cFileName);
					}
				}
				for (UINT i = 0; i < fi.GetCount(); ++i)
				{
					if (fi[i].dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
						continue;
					if (!(fi[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						UINT cmd = gMenuIndex++ + MENUID_START;
						AppendMenu(hMenu, MF_BYCOMMAND, cmd, fi[i].cFileName);
						//MENUITEMINFO mii = { 0 };
						//mii.cbSize = sizeof(mii);
						//mii.fMask = MIIM_DATA;
						//mii.dwItemData = 

						//SetMenuItemInfo(hMenu, cmd, FALSE,)
						gCmdMap[cmd] = stdCombinePath(qlDir, fi[i].cFileName);
					}
				}
			}
			else
			{
				// sub dir
				UINT cmd = gMenuIndex++ + MENUID_START;
			}
		}
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	HWND hWnd = CreateSimpleWindow(WndProc);
	if (!SHGetSpecialFolderPath(hWnd, szT, CSIDL_APPDATA, FALSE))
	{
		ErrorExit(GetLastError());
	}
	qlRoot = stdCombinePath(szT, L"Microsoft\\Internet Explorer\\Quick Launch");
	if ((GetFileAttributes(qlRoot.c_str()) & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		ErrorExit(stdFormat(I18N(L"'%s' is not a directory."), qlRoot.c_str()));
	}
	ghPopup = CreatePopupMenu();
	gPopupMap[ghPopup] = wstring();
	InsertMenu(ghPopup, 0, MF_BYCOMMAND, MENUID_DUMMY, L"<DUMMY>");

	POINT curPos;
	GetCursorPos(&curPos);
	SetForegroundWindow(hWnd);
	UINT cmd = TrackPopupMenu(ghPopup,
		TPM_RETURNCMD,
		curPos.x, curPos.y,
		0,
		hWnd,
		NULL);
	if (gCmdMap.find(cmd) != gCmdMap.end())
	{
		wstring shortcut = gCmdMap[cmd];
		OpenCommon(hWnd, shortcut.c_str());
	}
	return 0;
}