#pragma once

#include <string>

static const int MAX_PROCESS = (_MAX_FNAME + _MAX_EXT);

// This started life as a class, but the ints
// weren't working correctly in the shared data segment.
// As a struct it works fine for some reason.
struct WindowConfig
{
	TCHAR strProcess[MAX_PROCESS];
	TCHAR strWindowClass[MAX_PROCESS];
	TCHAR strParentClass[MAX_PROCESS];
	double dVertMsgCount;
	double dHorzMsgCount;
};

bool Match(const WindowConfig& aWindowConfig, HWND aHwnd, HWND& arTargetHwnd);

void InitializeWindowConfig(WindowConfig& arWindowConfig,
							const std::basic_string<TCHAR>& astrProcess,
							const std::basic_string<TCHAR>& astrWindowClass,
							const std::basic_string<TCHAR>& astrParentClass,
							double adVertMsgCount,
							double adHorzMsgCount);
