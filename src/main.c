#include <cursor-tracker.h>

static void DebugHit() {
	MessageBoxA(NULL, "Hit!", "Hit!", MB_OK | MB_ICONINFORMATION);
}

// REMARK: No way to pass in user data in hook events :(.
struct {
	WindowState *vertWindowState, *horizWindowState;
	HBRUSH *windowColorBrush;
} globalProcData;

static LRESULT CALLBACK WindowProc(
	HWND windowHandle,
	UINT messageType,
	WPARAM extraMessage, LPARAM dataParam) {
	WindowState *state = (WindowState *)(GetWindowLongPtr(
		windowHandle, GWLP_USERDATA));

	switch(messageType) {
	case WM_CREATE: { return 0; }
	case WM_DESTROY: {
		state->isOpen = FALSE;
		PostQuitMessage(0);
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
	case VK_ESCAPE: { PostQuitMessage(0); return 0; }
	case VK_RCONTROL: {
		Bool isKeyUp = extraMessage == WM_KEYUP || extraMessage == WM_SYSKEYUP;
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

#define WINDOW_THICKNESS 16
static LRESULT CALLBACK MouseProc(
	int messageType, WPARAM extraMessage, LPARAM dataParam) {
	if(
		messageType != HC_ACTION || extraMessage != WM_MOUSEMOVE
	) { goto end; }

	MSLLHOOKSTRUCT *mouseEvent = (PMSLLHOOKSTRUCT)(dataParam);
	POINT mousePos = mouseEvent->pt;

	// Horiz.
	int screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int screenSizeX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	SetWindowPos(
		globalProcData.horizWindowState->windowHandle, HWND_TOPMOST,
		screenX, mousePos.y, screenSizeX, WINDOW_THICKNESS,
		0);

	// Vert.
	int screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int screenSizeY = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	SetWindowPos(
		globalProcData.vertWindowState->windowHandle, HWND_TOPMOST,
		mousePos.x, screenY, WINDOW_THICKNESS, screenSizeY,
		0);

	end: {
		return CallNextHookEx(NULL, messageType, extraMessage, dataParam);
	}
}

int WINAPI WinMain(
	HINSTANCE programHandle,
	HINSTANCE legacyPrevAppHandle,
	LPSTR commandLine,
	int windowShowFlags
) {
	(void)(legacyPrevAppHandle);
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
		MSG message = { 0 };
		while(GetMessage(&message, NULL, 0, 0) > 0) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	UnhookWindowsHookEx(mouseHook);
	UnhookWindowsHookEx(keyboardHook);
	DeinitWindowState(&horizWindowState);
	DeinitWindowState(&vertWindowState);
	DeleteObject(windowColorBrush);
	DeinitWindowClass(programHandle, className);
	return 0;
}
