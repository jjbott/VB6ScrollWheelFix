#pragma once

#include <string>
/*
class Window
{
public:
	Window();
	Window(const std::basic_string<TCHAR>& astrProcess,
		         const std::basic_string<TCHAR>& astrWindowClass,
				 const std::basic_string<TCHAR>& astrParentClass,
				 int aiVertMsgCount,
				 int aiHorzMsgCount);

	bool Match(HWND aHwnd, HWND& arTargetHwnd);

	int HorzMsgCnt() { return m_iHorzMsgCount; }
	int VertMsgCnt() { return m_iVertMsgCount; }
private:
	static const int MAX_PROCESS = (_MAX_FNAME + _MAX_EXT);
	*/
static const int MAX_PROCESS = (_MAX_FNAME + _MAX_EXT);
struct Window
{
	/*std::basic_string<TCHAR>*/ TCHAR m_strProcess[MAX_PROCESS];
	/*std::basic_string<TCHAR>*/ TCHAR m_strWindowClass[MAX_PROCESS];
	/*std::basic_string<TCHAR>*/ TCHAR m_strParentClass[MAX_PROCESS];
	int m_iVertMsgCount;
	int m_iHorzMsgCount;
};

bool Match(const Window& aWindow, HWND aHwnd, HWND& arTargetHwnd);