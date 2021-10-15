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
#include "../../lsMisc/HighDPI.h"
#include "../../lsMisc/CHandle.h"
#include "../../lsMisc/stop_watch.h"
#include "../../lsMisc/Is64.h"
#include "../../lsMisc/CreateShortcutFile.h"
#include "../../lsMisc/stdosd/stdosd.h"

#include "gitrev.h"

#pragma comment(lib, "Shell32.lib")

using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;
using namespace std;

#define APPNAME L"ShowQL"
#define I18N(s) s
#define WAIT_AFTER_LAUNCH (5 * 1000)
#define WAIT_FOR_PROCESSIDLE (30 * 1000)

HMENU ghPopup;
TCHAR szT[MAX_PATH];

#define MENUID_DUMMY 1
#define MENUID_START 2
HINSTANCE ghInst;
WORD gMenuIndex;
map<UINT, wstring> gCmdMap;
map<HMENU, wstring> gPopupMap;
wstring qlRoot;
bool gbNoIcon = false;

#ifndef NDEBUG
wstop_watch* gsw;
#define TRACE_STOPWATCH(S) do { OutputDebugString(S);OutputDebugString(L" "); OutputDebugString(gsw->lookDiff().c_str()); OutputDebugString(L"\r\n"); }while(false)
#else
#define TRACE_STOPWATCH(S) do{}while(false)
#endif

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

//
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

HBITMAP MakeBitMapTransparent(HBITMAP hbmSrc)
{
	HDC hdcSrc, hdcDst;
	HBITMAP hbmOld = nullptr;
	HBITMAP hbmNew = nullptr;
	BITMAP bm;
	COLORREF clrTP, clrBK;

	if ((hdcSrc = CreateCompatibleDC(NULL)) != NULL) {
		if ((hdcDst = CreateCompatibleDC(NULL)) != NULL) {
			int nRow, nCol;
			GetObject(hbmSrc, sizeof(bm), &bm);
			hbmOld = (HBITMAP)SelectObject(hdcSrc, hbmSrc);
			hbmNew = CreateBitmap(bm.bmWidth, bm.bmHeight, bm.bmPlanes, bm.bmBitsPixel, NULL);
			SelectObject(hdcDst, hbmNew);

			BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

			clrTP = GetPixel(hdcDst, 0, 0);// Get color of first pixel at 0,0
			clrBK = GetSysColor(COLOR_MENU);// Get the current background color of the menu

			for (nRow = 0; nRow < bm.bmHeight; nRow++)// work our way through all the pixels changing their color
				for (nCol = 0; nCol < bm.bmWidth; nCol++)// when we hit our set transparency color.
					if (GetPixel(hdcDst, nCol, nRow) == clrTP)
						SetPixel(hdcDst, nCol, nRow, clrBK);

			DeleteDC(hdcDst);
		}
		DeleteDC(hdcSrc);

	}
	return hbmNew;// return our transformed bitmap.
}

HICON getIconFromShortcut(LPCWSTR pShortcut)
{
	//wstring targetExe;
	//wstring iconPath;
	//GetShortcutFileInfo(pShortcut, &targetExe, &iconPath, nullptr, nullptr, nullptr);
	if (true)
	{
		// const wstring loadString = !iconPath.empty() ? iconPath.c_str() : (!targetExe.empty() ? targetExe.c_str() : pShortcut);
		//HICON iT= ExtractIcon(ghInst, loadString.c_str(), 0);
		//if (iT)
		//	return iT;
		SHFILEINFO sfi = { 0 };
		if (!SHGetFileInfo(pShortcut,
			0,
			&sfi,
			sizeof(sfi),
			SHGFI_ICON | SHGFI_SMALLICON))
		{
			//if (!SHGetFileInfo(pShortcut,
			//	0,
			//	&sfi,
			//	sizeof(sfi),
			//	SHGFI_ICON | SHGFI_SMALLICON))
			{
				ErrorExit(GetLastError());
			}
		}
		return sfi.hIcon;
	}
	else
	{
		return ExtractIcon(ghInst, pShortcut, 0);
	}
}
HBITMAP getMenuBitmap(HICON hIcon)
{
	{
		ICONINFO iconInfo;
		if (!GetIconInfo(hIcon, &iconInfo))
			ErrorExit(GetLastError());
		return (iconInfo.hbmColor);
		// return MakeBitMapTransparent(iconInfo.hbmColor);
	}
}
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
				TRACE_STOPWATCH(L"WM_INITMENUPOPUP");
				while (DeleteMenu(hMenu, 0, MF_BYPOSITION))
					;
				
				wstring sel = gPopupMap[hMenu];
				DTRACE(L"sel=" + sel);
				wstring qlDir = sel.empty() ? qlRoot : sel;
				DTRACE(L"qlDir=" + qlDir);
				FILESINFOW fi;
				if (!GetFilesInfoW(qlDir.c_str(), fi))
					ErrorExit(GetLastError());
				TRACE_STOPWATCH(L"WM_INITMENUPOPUP GetFilesInfoW");
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
						TRACE_STOPWATCH(L"WM_INITMENUPOPUP process file");
						UINT cmd = gMenuIndex++ + MENUID_START;
						AppendMenu(hMenu, MF_BYCOMMAND, cmd,
							stdGetFileNameWitoutExtension(fi[i].cFileName).c_str());
						const wstring full = stdCombinePath(qlDir, fi[i].cFileName);
						if (!gbNoIcon)
						{
							HICON hIcon = getIconFromShortcut(full.c_str());
							TRACE_STOPWATCH(L"WM_INITMENUPOPUP SHGetFileInfo");
							HBITMAP hBitmap = getMenuBitmap(hIcon);
							MENUITEMINFO mii;
							mii.cbSize = sizeof(mii);
							mii.fMask = MIIM_BITMAP;
							mii.hbmpItem = hBitmap;
							if (!SetMenuItemInfo(hMenu, cmd, FALSE, &mii))
								ErrorExit(GetLastError());
							TRACE_STOPWATCH(L"WM_INITMENUPOPUP SetMenuItemInfo");
						}
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

wstring getMessageTitleString()
{
	return stdFormat(L"%s v%s (%s)",
		APPNAME,
		GetVersionString(nullptr, 3).c_str(),
		Is64BitProcess() ? L"x64" : L"x86");
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	ghInst = hInstance;
#ifndef NDEBUG
	gsw = new wstop_watch();
#endif
	InitHighDPISupport();
	CCommandLineParser parser(I18N(L"Show QuickLaunch Menus"), APPNAME);

	bool bVersion = false;
	parser.AddOptionRange({ L"-v",L"-version",L"--version" }, 0, &bVersion, ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Shows version (Press CTRL in startup)"));

	bool bHelp = false;
	parser.AddOptionRange({ L"-h",L"/?",L"-help",L"--help" }, 0, &bHelp, ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Shows Help"));

	bool bExplorer = false;
	parser.AddOptionRange({ L"-e",L"--explorer" }, 0, &bExplorer, ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Shows in Explorer (Press SHIFT in startup)"));

	parser.AddOptionRange({ L"-ni",L"--no-icon" }, 0, &gbNoIcon, ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Shows no icons"));

	COption mainArgs(L"",ArgCount::ArgCount_One, ArgEncodingFlags::ArgEncodingFlags_Default,
		L"Directory to show in menu");
	parser.AddOption(&mainArgs);

	parser.Parse();

	if (parser.hadUnknownOption())
	{
		ErrorExit(stdFormat(I18N(L"Unknown option:%s"), 
			parser.getUnknowOptionStrings().c_str()));
	}
	if (bVersion || GetAsyncKeyState(VK_CONTROL) < 0)
	{
		wstring message = stdFormat(L"%s v%s",
			APPNAME, GetVersionString(nullptr, 3).c_str()).c_str();
		message += L"\r\n\r\n";
		message += L"Gitrev:\r\n";
		message += stdStringReplace(GITREV::GetHashMessage(),
			L"\n", L"\r\n");
		MessageBox(nullptr,
			message.c_str(),
			getMessageTitleString().c_str(),
			MB_ICONINFORMATION);
		return 0;
	}
	if (bHelp)
	{
		MessageBox(nullptr,
			parser.getHelpMessage().c_str(),
			getMessageTitleString().c_str(),
			MB_ICONINFORMATION);
		return 0;
	}

	TRACE_STOPWATCH(L"Started");

	wstring targetFolder;
	if (mainArgs.getValueCount() > 1)
	{
		ErrorExit(I18N(L"Only one main argument is acceptable"));
	}
	if (mainArgs.getValueCount() > 0)
	{
		targetFolder = mainArgs.getFirstValue();
	}
	CcoInitializer coIniter;
	CHWnd wnd(CreateSimpleWindow(WndProc));

	if (!targetFolder.empty())
	{
		qlRoot = targetFolder;
	}
	else
	{
		if (!SHGetSpecialFolderPath(wnd, szT, CSIDL_APPDATA, FALSE))
		{
			ErrorExit(GetLastError());
		}
		qlRoot = stdCombinePath(szT, L"Microsoft\\Internet Explorer\\Quick Launch");
	}
	if ((GetFileAttributes(qlRoot.c_str()) & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		ErrorExit(stdFormat(I18N(L"'%s' is not a directory."), qlRoot.c_str()));
	}

	if (bExplorer || GetAsyncKeyState(VK_SHIFT) < 0)
	{
		qlRoot = stdAddPathSeparator(qlRoot);
		OpenCommon(wnd, qlRoot.c_str());
		return 0;
	}
	ghPopup = CreatePopupMenu();
	gPopupMap[ghPopup] = wstring();
	InsertMenu(ghPopup, 0, MF_BYCOMMAND, MENUID_DUMMY, L"<DUMMY>");

	POINT curPos;
	GetCursorPos(&curPos);
	SetForegroundWindow(wnd);
	TRACE_STOPWATCH(L"Before TrackPopup");
	UINT cmd = TrackPopupMenu(ghPopup,
		TPM_RETURNCMD,
		curPos.x, curPos.y,
		0,
		wnd,
		NULL);
	TRACE_STOPWATCH(L"After TrackPopup");
	if (gCmdMap.find(cmd) != gCmdMap.end())
	{
		wstring shortcut = gCmdMap[cmd];
		CKernelHandle processHandle;
		if (OpenCommon(wnd, shortcut.c_str(), NULL, NULL, &processHandle))
		{
			WaitForInputIdle(processHandle, WAIT_FOR_PROCESSIDLE);
		}
	}
	return 0;
}