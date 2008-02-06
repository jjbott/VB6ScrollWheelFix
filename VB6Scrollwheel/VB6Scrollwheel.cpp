#include <windows.h>
#include <stdio.h>
#include "resource.h"

// Macro to pop up error codes
#define ERRORPOPUP {char buffer[100];sprintf(buffer, "error: %i", GetLastError());MessageBox(NULL,buffer,"Error",MB_OK);}

// DLL functions
typedef void (*SETHOOK)(HWND);
typedef void (*UNSETHOOK)();

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

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;
	
	// Make sure only one copy is running
	if(FindWindow("VB6ScrollWheelFix",NULL) != NULL)
		return -1;
		
	// Create the window class. Very minimal, since its never displayed...
	WNDCLASS wndClass;
	ZeroMemory((void*)&wndClass,sizeof(WNDCLASS));
	wndClass.lpfnWndProc = WindowProc; 
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = "VB6ScrollWheelFix";

	// Register the class
	if(RegisterClass(&wndClass) == 0)
	{
		ERRORPOPUP;
		return -1;
	}

	// Create our window
	HWND hWnd = CreateWindow("VB6ScrollWheelFix",NULL,NULL,0,0,1,1,NULL,NULL,hInstance,NULL);
	if(hWnd == NULL)
	{
		ERRORPOPUP;
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
	strcpy(notifyIconData.szTip,"VB6 Scroll Wheel Fix");

	// Add the icon to the task bar
	Shell_NotifyIcon(NIM_ADD, &notifyIconData);


	HINSTANCE hinstDLL;

	// Load my DLL
	hinstDLL = LoadLibrary("VB6ScrollWheelHook.dll");
	if(hinstDLL == NULL)
	{
		MessageBox(NULL,"Hook DLL not loaded", "Error", MB_OK);
		return -1;
	}

	// DLL function pointers
	SETHOOK pSetHook;
	UNSETHOOK pUnsetHook;

	// Get the functions from the DLL
	pSetHook = (SETHOOK)GetProcAddress(hinstDLL,"SetHook");
	pUnsetHook = (UNSETHOOK)GetProcAddress(hinstDLL,"UnsetHook");

	if(NULL ==pSetHook)
	{
		MessageBox(NULL,"GetProcAddress(SetHook) Failed", "Error", MB_OK);
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