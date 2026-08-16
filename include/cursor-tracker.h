#ifndef PROJECT_NAME_H
#define PROJECT_NAME_H

	//// Base includes, defines, and typedefs (N/A).
	#define WIN32_LEAN_AND_MEAN
	#define COBJMACROS
	#include <windows.h>
	#include <winuser.h>
	#include <winternl.h>
	#include <shlobj.h>
	#include <shobjidl.h>
	#include <shellapi.h>

	#include <intrin.h>

	#pragma comment(lib, "user32.lib")
	#pragma comment(lib, "gdi32.lib")
	#pragma comment(lib, "shell32.lib")
	#pragma comment(lib, "ole32.lib")
	#pragma comment(lib, "ntdll.lib")

	#include <stdarg.h>
	#include <stdint.h>
	#include <wchar.h>

	// Any single header libraries imported would be included here
	// and also be in the include folder.

	// Misc. macros.
	#define ARRAY_SIZE(_array) \
		(sizeof(_array) / sizeof((_array)[0]))

	#ifdef DEBUG_MODE
		#define DEBUG_BREAK(...) __debugbreak()
	#else
		#define DEBUG_BREAK(...) OutputDebugStringA("DEBUG_BREAK at " __FILE__)
	#endif

	// Misc. typedefs.
	typedef uint8_t Bool;
	typedef int Error;

	typedef struct WindowState {
		Bool isOpen;
		HWND windowHandle;
		HINSTANCE programHandle;
	} WindowState;
#endif
