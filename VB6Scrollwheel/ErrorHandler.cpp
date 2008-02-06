#include "Windows.h"
#include "ErrorHandler.h"
#include <TCHAR.h>
#include <stdio.h>

void DisplayLastError()
{
	TCHAR lpszBuffer[256];
	_sntprintf_s(lpszBuffer, 256, _T("Error: %i"), GetLastError());
	MessageBox(NULL, lpszBuffer, _T("Error"), MB_OK);
}
