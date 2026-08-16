#include <cursor-tracker.h>

// REMARK: No way to pass in user data in hook events :(.
#define WINDOW_THICKNESS 16
#define USER_MESSAGE_TRAY_ICON (WM_USER + 1)
struct {
	WindowState *vertWindowState, *horizWindowState;
	HBRUSH *windowColorBrush;
	Bool isShowingWindows;
} globalProcData;

static void DebugHit() {
	MessageBoxA(NULL, "Hit!", "Hit!", MB_OK | MB_ICONINFORMATION);
}

static void QuitProgram(WindowState *state) {
	state->isOpen = FALSE;
	PostQuitMessage(0);
}

static LRESULT CALLBACK WindowProc(
	HWND windowHandle,
	UINT messageType,
	WPARAM extraMessage, LPARAM dataParam) {
	WindowState *state = (WindowState *)(GetWindowLongPtr(
		windowHandle, GWLP_USERDATA));

	switch(messageType) {
	case WM_CREATE: { return 0; }

	case USER_MESSAGE_TRAY_ICON: {
		if(LOWORD(dataParam) == WM_RBUTTONUP) {
			QuitProgram(state);
			return 0;
		}
	} break;

	case WM_DESTROY: {
		QuitProgram(state);
		return 0;
	}

	case WM_PAINT: {
		PAINTSTRUCT paintStruct = { 0 };
		HDC deviceContextHandle = BeginPaint(windowHandle, &paintStruct);
			FillRect(
				deviceContextHandle, &paintStruct.rcPaint,
				*globalProcData.windowColorBrush);
		EndPaint(windowHandle, &paintStruct);
	} break;

	case WM_GETMINMAXINFO: {
		MINMAXINFO *minMaxInfo = (MINMAXINFO *)(dataParam);
		minMaxInfo->ptMaxTrackSize.x = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		minMaxInfo->ptMaxTrackSize.y = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		return 0;
	}

	default: {} break;
	}

	return DefWindowProc(
		windowHandle, messageType, extraMessage, dataParam);
}

static void InitWindowClass(HINSTANCE programHandle, uint8_t *className) {
	WNDCLASSA windowClass = {
		.lpfnWndProc = WindowProc,
		.hInstance = programHandle,
		.lpszClassName = className,
		.hCursor = LoadCursor(NULL, IDC_ARROW)
	};

	RegisterClass(&windowClass);
}

static void DeinitWindowClass(HINSTANCE programHandle, uint8_t *className) {
	UnregisterClass(className, programHandle);
}

static WindowState InitWindowState(
	HINSTANCE programHandle, uint8_t *className) {
	WindowState result = {
		.isOpen = TRUE, .programHandle = programHandle
	};

	result.windowHandle = CreateWindowExA(
		WS_EX_TOOLWINDOW, className, "", WS_POPUP,
		0, 0, 0, 0,
		NULL, NULL, programHandle, (void *)(&result)
	);

	SetWindowLongPtr(
		result.windowHandle, GWLP_USERDATA, (LONG_PTR)(&result));

	if(result.windowHandle == NULL) { return (WindowState) { 0 }; }
	return result;
}

static void DeinitWindowState(WindowState *state) {
	DestroyWindow(state->windowHandle);
}

static LRESULT CALLBACK KeyboardProc(
	int messageType, WPARAM extraMessage, LPARAM dataParam) {
	if(
		messageType != HC_ACTION || (
			extraMessage != WM_KEYDOWN && extraMessage != WM_SYSKEYDOWN &&
			extraMessage != WM_KEYUP && extraMessage != WM_SYSKEYUP
		)
	) { goto end; }

	KBDLLHOOKSTRUCT *keyboardEvent = (PKBDLLHOOKSTRUCT)(dataParam);
	DWORD virtKeyCode = keyboardEvent->vkCode;
	switch(virtKeyCode) {
	case VK_RCONTROL: {
		Bool isKeyUp = extraMessage == WM_KEYUP || extraMessage == WM_SYSKEYUP;
		globalProcData.isShowingWindows = !isKeyUp;

		int showWindowFlags = isKeyUp ? SW_HIDE : SW_NORMAL;
		ShowWindow(
			globalProcData.vertWindowState->windowHandle, showWindowFlags);

		ShowWindow(
			globalProcData.horizWindowState->windowHandle, showWindowFlags);
	} break;

	default: {} break;
	}

	end: {
		return CallNextHookEx(NULL, messageType, extraMessage, dataParam);
	}
}

static LRESULT CALLBACK MouseProc(
	int messageType, WPARAM extraMessage, LPARAM dataParam) {
	if(
		messageType != HC_ACTION || extraMessage != WM_MOUSEMOVE
	) { goto end; }

	MSLLHOOKSTRUCT *mouseEvent = (PMSLLHOOKSTRUCT)(dataParam);
	POINT mousePos = mouseEvent->pt;
	SetWindowPos(
		globalProcData.horizWindowState->windowHandle, HWND_TOPMOST,
		GetSystemMetrics(SM_XVIRTUALSCREEN),
		mousePos.y - (WINDOW_THICKNESS / 2),
		GetSystemMetrics(SM_CXVIRTUALSCREEN), WINDOW_THICKNESS,
		0);

	SetWindowPos(
		globalProcData.vertWindowState->windowHandle, HWND_TOPMOST,
		mousePos.x - (WINDOW_THICKNESS / 2),
		GetSystemMetrics(SM_YVIRTUALSCREEN),
		WINDOW_THICKNESS, GetSystemMetrics(SM_CYVIRTUALSCREEN),
		0);

	end: {
		return CallNextHookEx(NULL, messageType, extraMessage, dataParam);
	}
}

static void AddTrayIcon(WindowState *state, HICON icon) {
	NOTIFYICONDATAA notifyIconData = {
		.cbSize = sizeof(NOTIFYICONDATAA), .hWnd = state->windowHandle,
		.hIcon = icon,
		.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP,
		.uCallbackMessage = USER_MESSAGE_TRAY_ICON
	};

	strcpy(notifyIconData.szTip, "Right click to quit program");
	Shell_NotifyIconA(NIM_ADD, &notifyIconData);
}

static void RemoveTrayIcon(WindowState *state) {
	NOTIFYICONDATAA notifyIconData = {
		.cbSize = sizeof(NOTIFYICONDATAA), .hWnd = state->windowHandle,
		.hWnd = state->windowHandle
	};

	Shell_NotifyIconA(NIM_DELETE, &notifyIconData);
}

int WINAPI WinMain(
	HINSTANCE programHandle,
	HINSTANCE legacyPrevProgramHandle,
	LPSTR commandLine,
	int windowShowFlags
) {
	(void)(legacyPrevProgramHandle);
	(void)(commandLine);
	(void)(windowShowFlags);

	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

	uint8_t *className = "cursor-tracker";
	InitWindowClass(programHandle, className);
	WindowState horizWindowState = InitWindowState(
		programHandle, className);

	globalProcData.horizWindowState = &horizWindowState;

	HBRUSH windowColorBrush = CreateSolidBrush(RGB(255, 0, 0));
	globalProcData.windowColorBrush = &windowColorBrush;

	WindowState vertWindowState = InitWindowState(programHandle, className);
	globalProcData.vertWindowState = &vertWindowState;

	HHOOK keyboardHook = SetWindowsHookExA(
		WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

	HHOOK mouseHook = SetWindowsHookExA(WH_MOUSE_LL, MouseProc, NULL, 0);
	AddTrayIcon(
		&horizWindowState, (HICON)(LoadImageA(
			GetModuleHandle(NULL), MAKEINTRESOURCEA(1),
			IMAGE_ICON,
			GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON),
			LR_SHARED)));
		MSG message = { 0 };
		while(GetMessage(&message, NULL, 0, 0) > 0) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	RemoveTrayIcon(&horizWindowState);
	UnhookWindowsHookEx(mouseHook);
	UnhookWindowsHookEx(keyboardHook);
	DeinitWindowState(&horizWindowState);
	DeinitWindowState(&vertWindowState);
	DeleteObject(windowColorBrush);
	DeinitWindowClass(programHandle, className);
	return 0;
}
