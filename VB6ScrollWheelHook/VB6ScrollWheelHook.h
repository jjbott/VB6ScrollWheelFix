
typedef void (*SET_HOOK)(HWND);
typedef void (*UNSET_HOOK)();
typedef void (*ADD_SCROLL_WINDOW)(const std::basic_string<TCHAR>& astrProcess,
								const std::basic_string<TCHAR>& astrWindowClass,
								const std::basic_string<TCHAR>& lstrParentClass,
								double ldVertMsgCount,
								double ldHorzMsgCount);
typedef void (*CLEAR_CONFIG)();