#include <windows.h>
#include <stdio.h>
#include <string>
#include "resource.h"
#include "TCHAR.h"
#include "ErrorHandler.h"
#include "VB6ScrollWheelHook.h"


HINSTANCE g_hInstance = NULL;
HWND g_dlgHwnd = NULL;

// The About box proc
INT_PTR CALLBACK AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_CLOSE || (uMsg==WM_COMMAND && wParam==IDOK))
		DestroyWindow(g_dlgHwnd);
	return FALSE;
}

// Our window proc
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	switch(uMsg)
	{
	case WM_USER: // Message from the Notify Icon
		if(lParam == WM_RBUTTONDOWN) // The right mouse button came up on our icon, 
								   // the user wants the menu
		{
			POINT pt;
            GetCursorPos(&pt); 

            SetForegroundWindow(hWnd);

			// Get the menu from the resource
			HMENU hMenu = LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_MENU1));
			// Display the menu
            TrackPopupMenu(GetSubMenu(hMenu,0), 0, pt.x, pt.y, 0, hWnd, NULL);

			// User is done, kill the menu
            PostMessage(hWnd, NULL, 0, 0);
		}
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)==ID_QUIT)
		{
			// User wants to quit
			PostQuitMessage(0);
		}
		else if (LOWORD(wParam)==ID_ABOUT)
		{
			// Create the about box
			g_dlgHwnd = CreateDialog(g_hInstance,MAKEINTRESOURCE(IDD_ABOUT),hWnd,AboutProc);
			ShowWindow(g_dlgHwnd,SW_SHOWNORMAL);
		}
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void LoadConfig(ADD_SCROLL_WINDOW apAddScrollWindow)
{
	const int BUFFER_SIZE = 256;
	TCHAR lpszBuffer[BUFFER_SIZE];
	int liCharsRead(0);

	TCHAR lpszIniPath[_MAX_PATH];
	GetModuleFileName(NULL, lpszIniPath, MAX_PATH);

	TCHAR lpszDrive[_MAX_DRIVE];
	TCHAR lpszPath[_MAX_PATH];
	TCHAR lpszFilename[_MAX_FNAME];
	TCHAR lpszExtension[_MAX_EXT];
	_tsplitpath_s(lpszIniPath, lpszDrive, lpszPath, lpszFilename, lpszExtension);
	_tmakepath_s(lpszIniPath, _MAX_PATH, lpszDrive, lpszPath, lpszFilename, _T(".ini"));

	TCHAR lpszSectionNames[1024];
	DWORD ldwCharsRead = GetPrivateProfileSectionNames(lpszSectionNames, 1024, lpszIniPath);
	if ( ldwCharsRead < 1022 )
	{
		TCHAR* lpszSectionName = lpszSectionNames;
		while( '\0' != (*lpszSectionName) )
		{
			std::basic_string<TCHAR> lstrProcess;
			std::basic_string<TCHAR> lstrWindowClass;
			std::basic_string<TCHAR> lstrParentClass;
			double ldVertMsgCount(0);
			double ldHorzMsgCount(0);
			
			GetPrivateProfileString(lpszSectionName, _T("Process"), _T(""), lpszBuffer, BUFFER_SIZE, lpszIniPath);
			lstrProcess = lpszBuffer;

			GetPrivateProfileString(lpszSectionName, _T("WindowClass"), _T(""), lpszBuffer, BUFFER_SIZE, lpszIniPath);
			lstrWindowClass = lpszBuffer;

			GetPrivateProfileString(lpszSectionName, _T("ParentWindowClass"), _T(""), lpszBuffer, BUFFER_SIZE, lpszIniPath);
			lstrParentClass = lpszBuffer;

			GetPrivateProfileString(lpszSectionName, _T("VertMsgCount"), _T("3"), lpszBuffer, BUFFER_SIZE, lpszIniPath);
			ldVertMsgCount = _tstof(lpszBuffer);

			GetPrivateProfileString(lpszSectionName, _T("HorzMsgCount"), _T("3"), lpszBuffer, BUFFER_SIZE, lpszIniPath);
			ldHorzMsgCount = _tstof(lpszBuffer);

			if ( (lstrProcess != _T("")) && 
				 (lstrWindowClass != _T("")) )
			{
				apAddScrollWindow(lstrProcess, lstrWindowClass, lstrParentClass, ldVertMsgCount, ldHorzMsgCount);
			}

			lpszSectionName += _tcslen(lpszSectionName) + 1;
		}
	}
}



int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;
	
	// Make sure only one copy is running
	if(FindWindow(_T("VB6ScrollWheelFix"), NULL) != NULL)
		return -1;
		
	// Create the window class. Very minimal, since its never displayed...
	WNDCLASS wndClass;
	ZeroMemory((void*)&wndClass,sizeof(WNDCLASS));
	wndClass.lpfnWndProc = WindowProc; 
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = _T("VB6ScrollWheelFix");

	// Register the class
	if(RegisterClass(&wndClass) == 0)
	{
		DisplayLastError();
		return -1;
	}

	// Create our window
	HWND hWnd = CreateWindow(_T("VB6ScrollWheelFix"),NULL,NULL,0,0,1,1,NULL,NULL,hInstance,NULL);
	if(hWnd == NULL)
	{
		DisplayLastError();
		return -1;
	}

	// Create the Notify Icon
	NOTIFYICONDATA notifyIconData;
	ZeroMemory((void*)&notifyIconData, sizeof(NOTIFYICONDATA));
	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.uID = 0;
	notifyIconData.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));
    notifyIconData.hWnd = hWnd; 
    notifyIconData.uFlags = NIF_ICON |NIF_MESSAGE|NIF_TIP  ; 
    notifyIconData.uCallbackMessage = WM_USER; 
	_tcscpy_s(notifyIconData.szTip, 64, _T("VB6 Scroll Wheel Fix"));

	// Add the icon to the task bar
	Shell_NotifyIcon(NIM_ADD, &notifyIconData);

	HINSTANCE hinstDLL;

	// Load my DLL
	hinstDLL = LoadLibrary(_T("VB6ScrollWheelHook.dll"));
	if(hinstDLL == NULL)
	{
		MessageBox(NULL,_T("Hook DLL not loaded"), _T("Error"), MB_OK);
		return -1;
	}

	// DLL function pointers
	SET_HOOK pSetHook;
	UNSET_HOOK pUnsetHook;
	ADD_SCROLL_WINDOW pAddScrollWindow;
	CLEAR_CONFIG pClearConfig;

	// Get the functions from the DLL
	pSetHook = (SET_HOOK)GetProcAddress(hinstDLL,"SetHook");
	pUnsetHook = (UNSET_HOOK)GetProcAddress(hinstDLL,"UnsetHook");
	pAddScrollWindow = (ADD_SCROLL_WINDOW)GetProcAddress(hinstDLL,"AddScrollWindow");
	pClearConfig = (CLEAR_CONFIG)GetProcAddress(hinstDLL,"ClearConfig");
	
	(*pClearConfig)();
	LoadConfig(pAddScrollWindow);

	if(NULL ==pSetHook)
	{
		MessageBox(NULL,_T("GetProcAddress(SetHook) Failed"), _T("Error"), MB_OK);
	}
	else
		(*pSetHook)(hWnd); // Set the message hook

	
	MSG msg; 

	// Message Loop
	while (GetMessage(&msg,NULL,NULL,NULL)) 
		DispatchMessage(&msg);


	// All done, unset the message hook
	if(NULL !=pUnsetHook)
		(*pUnsetHook)();
	
	// Free the DLL
	FreeLibrary(hinstDLL);

	// Remove the icon from the task bar
	Shell_NotifyIcon(NIM_DELETE , &notifyIconData);

	return 0;
}