// VB6ScrollWheelHook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#include <winuser.h>
#pragma data_seg("SHARED")
HHOOK g_hHook;
#pragma data_seg()

HINSTANCE hInstance;

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
		hInstance = (HINSTANCE)hModule;
    return TRUE;
}

// Get the handle of the vertical scrollbar
//
// The VB Code window's scrollbars have their own handles, so I
// need to find them, or else sending WM_VSCROLL wont work
HWND GetVScrollHandle(HWND hWnd)
{
	DWORD style;

	// find the first "ScrollBar" child
	HWND scrollHWnd = FindWindowEx(hWnd,NULL,"ScrollBar",NULL);
	
	while(scrollHWnd != NULL)
	{
		// Get the style, to determine if this is the vertical scrollbar
		style = GetWindowLong(scrollHWnd,GWL_STYLE);
		if(style&SBS_VERT)
			return scrollHWnd; // Got the vert scrollbar, all done!

		// Find the next "Scrollbar" child
		scrollHWnd = FindWindowEx(hWnd,scrollHWnd,"ScrollBar",NULL);
	}

	// Nothing was found, return NULL
	return NULL;
}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	char classname[255];

	// Get the class of the window that this message is for
	GetClassName(((MSG*)lParam)->hwnd,classname,255);

	// We only care about the "VbaWindow" class
	// TODO: Figure out why the same method doesnt work with "DesignerWindow"
	//       I never get its WM_MOUSEWHEEL messages, even though Spy++ gets them...
	if(strcmp(classname,"VbaWindow") == 0)
	{
		// Filter out everything but the WM_MOUSEWHEEL event
		if(((MSG*)lParam)->message == WM_MOUSEWHEEL)
		{
			// Get the handle to the vertical scrollbar
			HWND scrollHWnd = GetVScrollHandle(((MSG*)lParam)->hwnd);

			// Determine how much the mouse wheel has moved (multiples of WHEEL_DELTA)
			int wheel = HIWORD(((MSG*)lParam)->wParam);
			
			// Make sure negative numbers are negative. Could probaly
			// avoid this by using a 16bit int, but what fun is that?
			if(wheel&0x8000)
				wheel = wheel|0xffff0000;
	
			if(wheel<0)
			{
				// Wheel < 0
				// User wants to scroll down, so send 3 SB_LINEDOWN
				// messages for each wheel click
				for(int i = 0; i > wheel; i=i-WHEEL_DELTA)
				{
					SendMessage(((MSG*)lParam)->hwnd,WM_VSCROLL,SB_LINEDOWN,(LPARAM)scrollHWnd);
					SendMessage(((MSG*)lParam)->hwnd,WM_VSCROLL,SB_LINEDOWN,(LPARAM)scrollHWnd);
					SendMessage(((MSG*)lParam)->hwnd,WM_VSCROLL,SB_LINEDOWN,(LPARAM)scrollHWnd);
				}
			}
			else
			{
				// Wheel > 0
				// User wants to scroll up, so send 3 SB_LINEDOWN
				// messages for each wheel click
				for(int i = 0; i < wheel; i=i+WHEEL_DELTA)
				{
					SendMessage(((MSG*)lParam)->hwnd,WM_VSCROLL,SB_LINEUP,(LPARAM)scrollHWnd);
					SendMessage(((MSG*)lParam)->hwnd,WM_VSCROLL,SB_LINEUP,(LPARAM)scrollHWnd);
					SendMessage(((MSG*)lParam)->hwnd,WM_VSCROLL,SB_LINEUP,(LPARAM)scrollHWnd);
				}
			}

			// Send the SB_ENDSCROLL message
			SendMessage(((MSG*)lParam)->hwnd,WM_VSCROLL,SB_ENDSCROLL,(LPARAM)scrollHWnd);
			
		}
	}

	// Pass the message to the next hook
	return CallNextHookEx(g_hHook,nCode,wParam,lParam);
}

// Set the message hook
void SetHook(HWND hWnd)
{
	g_hHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)HookProc, hInstance,0);
}

//unset the message hook
void UnsetHook()
{
	UnhookWindowsHookEx(g_hHook);
}