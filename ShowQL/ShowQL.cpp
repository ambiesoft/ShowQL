#include <Windows.h>
#include <shlobj_core.h>

#include <string>
#include <map>
#include "../../lsMisc/CreateSimpleWindow.h"
#include "../../lsMisc/GetLastErrorString.h"
#include "../../lsMisc/GetFilesInfo.h"
#include "../../lsMisc/OpenCommon.h"
#include "../../lsMisc/DebugMacro.h"
#include "../../lsMisc/CommandLineParser.h"
#include "../../lsMisc/GetVersionString.h"
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


//HBITMAP IconToAlphaBitmap(HICON ico)
//{
//	ICONINFO ii;
//	GetIconInfo(ico, &ii);
//	HBITMAP bmp = (ii.hbmColor);
//	DeleteObject(ii.hbmColor);
//	DeleteObject(ii.hbmMask);
//
//	if (Bitmap.GetPixelFormatSize(bmp.PixelFormat) < 32)
//		return ico.ToBitmap();
//
//	BitmapData bmData;
//	Rectangle bmBounds = new Rectangle(0, 0, bmp.Width, bmp.Height);
//
//	bmData = bmp.LockBits(bmBounds, ImageLockMode.ReadOnly, bmp.PixelFormat);
//
//	Bitmap dstBitmap = new Bitmap(bmData.Width, bmData.Height, bmData.Stride, PixelFormat.Format32bppArgb, bmData.Scan0);
//
//	bool IsAlphaBitmap = false;
//
//	for (int y = 0; y <= bmData.Height - 1; y++)
//	{
//		for (int x = 0; x <= bmData.Width - 1; x++)
//		{
//			Color PixelColor = Color.FromArgb(Marshal.ReadInt32(bmData.Scan0, (bmData.Stride * y) + (4 * x)));
//			if (PixelColor.A > 0 & PixelColor.A < 255)
//			{
//				IsAlphaBitmap = true;
//				break;
//			}
//		}
//		if (IsAlphaBitmap) break;
//	}
//
//	bmp.UnlockBits(bmData);
//
//	if (IsAlphaBitmap == true)
//		return new Bitmap(dstBitmap);
//	else
//		return new Bitmap(ico.ToBitmap());
//
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
						AppendMenu(hMenu, MF_BYCOMMAND, cmd,
							stdGetFileNameWitoutExtension(fi[i].cFileName).c_str());
					
						const wstring full = stdCombinePath(qlDir, fi[i].cFileName);
						SHFILEINFO sfi = { 0 };
						if (!SHGetFileInfo(full.c_str(),
							0,
							&sfi,
							sizeof(sfi),
							SHGFI_ICON | SHGFI_SMALLICON))
						{
							ErrorExit(GetLastError());
						}
						
						ICONINFO iconInfo;
						if (!GetIconInfo(sfi.hIcon, &iconInfo))
							ErrorExit(GetLastError());
						HBITMAP hBitmap = iconInfo.hbmColor;
						//HBITMAP hBitmap = ggg(sfi.hIcon);
						MENUITEMINFO mii;
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_BITMAP;
						mii.hbmpItem = hBitmap;
						if (!SetMenuItemInfo(hMenu, cmd, FALSE, &mii))
							ErrorExit(GetLastError());
						gCmdMap[cmd] = full;
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
	CCommandLineParser parser;

	bool bVersion = false;
	parser.AddOption(L"-v", 0, &bVersion);

	parser.Parse();

	if (bVersion || GetAsyncKeyState(VK_CONTROL) < 0)
	{
		MessageBox(nullptr,
			stdFormat(L"%s v%s", 
				APPNAME, GetVersionString(nullptr, 3).c_str()).c_str(),
			APPNAME,
			MB_ICONINFORMATION);
		return 0;
	}
	CoInitialize(nullptr);
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