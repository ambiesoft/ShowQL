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
WORD gMenuIndex;
map<UINT, wstring> gCmdMap;
map<HMENU, wstring> gPopupMap;
wstring qlRoot;

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
						SHFILEINFO sfi = { 0 };
						if (!SHGetFileInfo(full.c_str(),
							0,
							&sfi,
							sizeof(sfi),
							SHGFI_ICON | SHGFI_SMALLICON))
						{
							ErrorExit(GetLastError());
						}
						TRACE_STOPWATCH(L"WM_INITMENUPOPUP SHGetFileInfo");
						ICONINFO iconInfo;
						if (!GetIconInfo(sfi.hIcon, &iconInfo))
							ErrorExit(GetLastError());
						TRACE_STOPWATCH(L"WM_INITMENUPOPUP GetIconInfo");
						HBITMAP hBitmap = iconInfo.hbmColor;
						//HBITMAP hBitmap = ggg(sfi.hIcon);
						MENUITEMINFO mii;
						mii.cbSize = sizeof(mii);
						mii.fMask = MIIM_BITMAP;
						mii.hbmpItem = hBitmap;
						if (!SetMenuItemInfo(hMenu, cmd, FALSE, &mii))
							ErrorExit(GetLastError());
						TRACE_STOPWATCH(L"WM_INITMENUPOPUP SetMenuItemInfo");
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
#ifndef NDEBUG
	gsw = new wstop_watch();
#endif
	InitHighDPISupport();
	CCommandLineParser parser(I18N(L"Show QuickLaunch Menus"), APPNAME);

	bool bVersion = false;
	parser.AddOptionRange({ L"-v",L"-version",L"--version" }, 0, &bVersion, ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Show version (Press CTRL in startup)"));

	bool bHelp = false;
	parser.AddOptionRange({ L"-h",L"/?",L"-help",L"--help" }, 0, &bHelp, ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Show Help"));

	bool bExplorer = false;
	parser.AddOptionRange({ L"-e",L"--explorer" }, 0, &bExplorer, ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Show in Explorer (Press SHIFT in startup)"));

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
			APPNAME,
			MB_ICONINFORMATION);
		return 0;
	}
	if (bHelp)
	{
		MessageBox(nullptr,
			parser.getHelpMessage().c_str(),
			stdFormat(L"%s v%s",
				APPNAME, GetVersionString(nullptr, 3).c_str()).c_str(),
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