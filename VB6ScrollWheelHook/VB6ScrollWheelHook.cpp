// VB6ScrollWheelHook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#include <winuser.h>
#include <stdio.h>
#include <stdlib.h>
#include <TCHAR.h>
#include <vector>
#include "WindowConfig.h"

static const int MAX_WINDOWS = 32;

#pragma data_seg("SHARED")
static HHOOK g_hHook(NULL);
static int g_iWindowCount(0);
static WindowConfig g_aWindows[MAX_WINDOWS] = {};
#pragma data_seg()
#pragma comment(linker, "/section:SHARED,RWS")

// horizontal mouse wheel message id
#define WM_MOUSEHWHEEL (0x020e)

HINSTANCE hInstance;

double g_dWheelDelta(0);
HWND g_hwndLastTarget(NULL);
UINT g_bLastScrollType(0);
UINT g_bLastScrollDirection(0);

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
		hInstance = (HINSTANCE)hModule;
    return TRUE;
}

// Get the handle of the scrollbar 
//
// The VB Code window's scrollbars have their own handles, so I
// need to find them, or else sending WM_VSCROLL wont work
//
// Edit 1/14/2004: Changed from GetVScrollHandle to GetScrollHandle
// Edit 1/22/2004: Modified Vertical Scrollbar selection. Since there 
//                 may be 2 vertical scrollbars, I need to pick the 
//                 best match
HWND GetScrollHandle(HWND hWnd, bool abHorizontal)
{
	DWORD style;
	
	HWND vertScrollHWnd = NULL; // Storage for potential vertical scrollbars
	
	// find the first "ScrollBar" child
	HWND scrollHWnd = FindWindowEx(hWnd,NULL,_T("ScrollBar"),NULL);
	
	while(scrollHWnd != NULL)
	{
		// Get the style, to determine if this is the vertical scrollbar
		style = GetWindowLong(scrollHWnd,GWL_STYLE);

		// If we want a horizontal scrollbar, return immediately.
		// I'm asuming that there will only be one
		if ( abHorizontal && 
			 (0 < (style&SBS_HORZ)) )          
		{
			return scrollHWnd;                   
		}


		// Make sure the Scrollbar is vertical and visible
		if(!abHorizontal && style&SBS_VERT && style&WS_VISIBLE)
		{
			//Found a vertical scrollbar
			if(vertScrollHWnd == NULL)
				vertScrollHWnd = scrollHWnd; // First one we've found, store it
			else
			{
				// I assume that the first vertical scrollbar (vertScrollHWnd) 
				// was the bottom one, and this one (scrollHWnd) is the top one. 
				RECT vScrollRect;
				POINT mousePt;

				// Get the scrollbar's location
				GetWindowRect(scrollHWnd,&vScrollRect);
				// Get the mouse pointer's location
				GetCursorPos(&mousePt);


				if (mousePt.y <= vScrollRect.bottom)
					return scrollHWnd; // Pointer is above the bottom of the 
				                       // top scrollbar, use top scrollbar
				else
                    return vertScrollHWnd; // Otherwise, use the bottom
			}
			
		}

		// Find the next "Scrollbar" child
		scrollHWnd = FindWindowEx(hWnd,scrollHWnd,_T("ScrollBar"),NULL);
	}

	if(vertScrollHWnd != NULL) // We never found a second scrollbar, 
		return vertScrollHWnd; // return the one we found
	
	// Nothing was found, return NULL
	return NULL;
}

LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if ( (0 <= nCode) && 
		 (PM_REMOVE == wParam) ) // Only process if the message is being removed from the
		                         //  queue, otherwise we may process it multiple times
	{
		MSG* lpMsg = (MSG*)lParam;

		// Filter out everything but the WM_MOUSEWHEEL and WM_MOUSEHWHEEL event
		if( (WM_MOUSEWHEEL == lpMsg->message) ||
			(WM_MOUSEHWHEEL == lpMsg->message) ) 
		{
			HWND lhwndMsgTarget = NULL;
			bool lbFoundMatch = false;
			WindowConfig* lpWindowConfig = NULL;
			for(int index = 0; !lbFoundMatch && (index < g_iWindowCount); ++index)
			{
				lpWindowConfig = &g_aWindows[index];
				lbFoundMatch = Match(*lpWindowConfig, lpMsg->hwnd, lhwndMsgTarget);

			}

			if (NULL != lhwndMsgTarget )
			{	
				double ldMsgCount(0);
				UINT liScrollMsg;	
				bool lbHorizontalScroll(false);
				if( (WM_MOUSEHWHEEL == lpMsg->message) ||
					(0 < (GetAsyncKeyState(VK_CONTROL) & 0x80000000)) )
				{
					lbHorizontalScroll = true;
				}

				if ( lbHorizontalScroll )
				{
					ldMsgCount = lpWindowConfig->dHorzMsgCount;
					liScrollMsg = WM_HSCROLL;                         
				}
				else                                          
				{
					ldMsgCount = lpWindowConfig->dVertMsgCount;
					liScrollMsg = WM_VSCROLL;    
				}

				// Get the handle to the scrollbar
				HWND scrollHWnd = GetScrollHandle(lhwndMsgTarget, lbHorizontalScroll);

				// Determine how much the mouse wheel has moved 
				double ldWheelDelta = (short)(HIWORD(lpMsg->wParam));
				ldWheelDelta *= ldMsgCount;

				UINT liScrollType = SB_LINEUP;
				if( ldWheelDelta < 0 )
				{
					liScrollType = SB_LINEDOWN;
					ldWheelDelta = -ldWheelDelta;
				}

				// Reset the accumulated mouse movement if necessary
				if ( (lhwndMsgTarget != g_hwndLastTarget) || 
				     (liScrollType != g_bLastScrollType) ||
				     (liScrollMsg != g_bLastScrollDirection) )
				{
					g_dWheelDelta = 0;
				}

				g_dWheelDelta += ldWheelDelta;

				g_hwndLastTarget = lhwndMsgTarget;
				g_bLastScrollType = liScrollType;
				g_bLastScrollDirection = liScrollMsg;

				while( g_dWheelDelta >= WHEEL_DELTA)
				{
					SendMessage(lhwndMsgTarget, liScrollMsg, liScrollType, (LPARAM)scrollHWnd);

					g_dWheelDelta -= WHEEL_DELTA;
				}

				SendMessage(lpMsg->hwnd, liScrollMsg, SB_ENDSCROLL, (LPARAM)scrollHWnd);
			}
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

void AddScrollWindow(const std::basic_string<TCHAR>& astrProcess,
			         const std::basic_string<TCHAR>& astrWindowClass,
			         const std::basic_string<TCHAR>& astrParentClass,
					 double adVertMsgCount,
					 double adHorzMsgCount)
{
	int index = g_iWindowCount++;
	if ( index < MAX_WINDOWS )
	{
		InitializeWindowConfig(g_aWindows[index],
			                   astrProcess, 
							   astrWindowClass, 
							   astrParentClass, 
							   adVertMsgCount, 
							   adHorzMsgCount);
	}

}

void ClearConfig()
{
	g_iWindowCount = 0;
}