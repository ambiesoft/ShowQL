#include <Windows.h>
#include <shlobj_core.h>

#include <string>

#include "../../lsMisc/CreateSimpleWindow.h"
#include "../../lsMisc/GetLastErrorString.h"
#include "../../lsMisc/GetFilesInfo.h"
#include "../../lsMisc/stdosd/stdosd.h"

#pragma comment(lib, "Shell32.lib")

using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;
using namespace std;

#define APPNAME L"ShowQL"

HMENU ghPopup;
TCHAR szT[MAX_PATH];

void ErrorExit(DWORD le)
{
	MessageBox(nullptr,
		GetLastErrorString(le).c_str(),
		APPNAME,
		MB_ICONERROR);
	ExitProcess(le);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITMENUPOPUP:
		{
			HMENU hMenu = (HMENU)wParam;
			WORD index = LOWORD(lParam);
			if (hMenu == ghPopup)
			{
				// Top Menu
				if (!SHGetSpecialFolderPath(hWnd, szT, CSIDL_APPDATA, FALSE))
				{
					ErrorExit(GetLastError());
				}
				wstring qlDir = stdCombinePath(szT, L"Microsoft\\Internet Explorer\\Quick Launch");
				FILESINFOW fi;
				if (!GetFilesInfoW(qlDir.c_str(), fi))
					ErrorExit(GetLastError());
				for (UINT i = 0; i < fi.GetCount(); ++i)
				{
					if (fi[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						continue;

					AppendMenu(hMenu, MF_BYCOMMAND, 123, fi[i].cFileName);
				}
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
	ghPopup = CreatePopupMenu();
	InsertMenu(ghPopup, 0, MF_BYCOMMAND, 333, L"aaa");
	TrackPopupMenu(ghPopup,
		TPM_RETURNCMD,
		0, 0,
		0,
		hWnd,
		NULL);
	return 0;
}