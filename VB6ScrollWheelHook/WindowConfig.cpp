#include "StdAfx.h"
#include "WindowConfig.h"
#include "TCHAR.h"


bool ClassMatch(const TCHAR* lpWindowClass, const TCHAR* lpConfigClass)
{
	int liConfigLen = _tcslen(lpConfigClass);
	int liClassLen = _tcslen(lpWindowClass);
	if ( (3 <= liConfigLen) &&
		 (liConfigLen <= liClassLen)  &&
		 (0 == _tcsnicmp(lpConfigClass, _T("Afx"), 3)) )
	{
		return (0 == _tcsnicmp(lpConfigClass, lpWindowClass, liConfigLen));
	}
	else
	{
		return (0 == _tcsicmp(lpWindowClass, lpConfigClass));
	}
}

// Determines if the specified window handle is for a window that we care about.
// Returns true if the handle matches the WindowConfig.
// Also, arTargetHwnd is set to the handle to the window that should recieve scroll messages
bool Match(const WindowConfig& aWindowConfig, HWND aHwnd, HWND& arTargetHwnd)
{
	bool lbMatch = false;
	TCHAR lstrProcessPath[MAX_PATH];
	TCHAR lstrProcessFilename[_MAX_FNAME + _MAX_EXT];
	TCHAR lstrProcessExt[_MAX_EXT];

	::GetWindowModuleFileName(aHwnd, lstrProcessPath, MAX_PATH);

	_tsplitpath_s(lstrProcessPath, NULL, 0, NULL, 0, lstrProcessFilename, _MAX_FNAME, lstrProcessExt, _MAX_EXT);
	_tcscat_s(lstrProcessFilename, lstrProcessExt);

	if ( 0 == _tcsicmp(aWindowConfig.strProcess, lstrProcessFilename) )
	{
		TCHAR lstrClassname[256];
		GetClassName(aHwnd,lstrClassname,256);

		if ( ClassMatch(lstrClassname, aWindowConfig.strWindowClass) )
		{
			if ( 0 != _tcslen(aWindowConfig.strParentClass) )
			{
				HWND lParentHwnd = GetParent(aHwnd);
				TCHAR lstrParentClassname[256];

				if ( NULL != lParentHwnd )
				{
					GetClassName(lParentHwnd,lstrParentClassname,256);

					if ( ClassMatch(lstrParentClassname, aWindowConfig.strParentClass) )
					{
						lbMatch = true;
						arTargetHwnd = lParentHwnd;
					}
				}
			}
			else
			{
				lbMatch = true;
				arTargetHwnd = aHwnd;
			}
		}
	}

	return lbMatch;
}

// Copies config information to a WindowConfig struct
void InitializeWindowConfig(WindowConfig& arWindowConfig,
							const std::basic_string<TCHAR>& astrProcess,
							const std::basic_string<TCHAR>& astrWindowClass,
							const std::basic_string<TCHAR>& astrParentClass,
							double adVertMsgCount,
							double adHorzMsgCount)
{
	arWindowConfig.dVertMsgCount = adVertMsgCount;
	arWindowConfig.dHorzMsgCount = adHorzMsgCount;
	_tcsncpy_s(arWindowConfig.strProcess, astrProcess.c_str(), MAX_PROCESS);
	_tcsncpy_s(arWindowConfig.strWindowClass, astrWindowClass.c_str(), MAX_PROCESS);
	_tcsncpy_s(arWindowConfig.strParentClass, astrParentClass.c_str(), MAX_PROCESS);
}

