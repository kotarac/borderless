#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

LPCWSTR cn = L"borderless";
HWND lb, btn1, btn2;

void Error(LPWSTR msg)
{
	MessageBox(NULL, msg, L"borderless", MB_ICONERROR | MB_OK);
}

void Info(LPWSTR msg)
{
	MessageBox(NULL, msg, L"borderless", MB_ICONINFORMATION | MB_OK);
}

BOOL Borderless(LPWSTR x)
{
	BOOL r = FALSE;
	HWND w = FindWindow(NULL, x);

	if (w) {
		LONG_PTR s;

		s = GetWindowLongPtr(w, GWL_STYLE);
		s &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
		SetWindowLongPtr(w, GWL_STYLE, s);

		s = GetWindowLongPtr(w, GWL_EXSTYLE);
		s &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
		SetWindowLongPtr(w, GWL_EXSTYLE, s);

		SetWindowPos(w, NULL, 0, 0, 1680, 1050, SWP_SHOWWINDOW);

		r = TRUE;
	} else {
		Error(L"specified window not found");
	}

	return r;
}

BOOL CALLBACK EnumWindowsProc(HWND w, LPARAM lp)
{
	WINDOWINFO wi;
	wi.cbSize = sizeof(WINDOWINFO);
	GetWindowInfo(w, &wi);
	if (wi.dwStyle & WS_VISIBLE) {
		UINT l = GetWindowTextLength(w);
		LPWSTR t = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR) * l);
		GetWindowText(w, t, l + 1);
		if (lstrcmp(t, L"borderless") && lstrcmp(t, L"")) {
			SendMessage(lb, LB_ADDSTRING, 0, (LPARAM) t);
		}
		LocalFree(t);
	}

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_CREATE: {
		WINDOWINFO wi;
		wi.cbSize = sizeof(WINDOWINFO);
		GetWindowInfo(hwnd, &wi);
		RECT r;
		r = wi.rcClient;

		btn1 = CreateWindow(L"BUTTON", L"Refresh list", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 2, 2, r.right - r.left - 4, 24, hwnd, NULL, (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
		lb = CreateWindow(L"LISTBOX", NULL, WS_TABSTOP | WS_CHILD | WS_VISIBLE | LBS_STANDARD, 2, 28, r.right - r.left - 4, r.bottom - r.top - 44, hwnd, NULL, (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
		btn2 = CreateWindow(L"BUTTON", L"Make it borderless", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 2, r.bottom - r.top - 26, r.right - r.left - 4, 24, hwnd, NULL, (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

		SendMessage(lb, LB_RESETCONTENT, 0, 0);
		EnumWindows(EnumWindowsProc, 0);

		HFONT f;
		f = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Segoe UI");
		SendMessage(hwnd, WM_SETFONT, (WPARAM) f, TRUE);
		SendMessage(btn1, WM_SETFONT, (WPARAM) f, TRUE);
		SendMessage(btn2, WM_SETFONT, (WPARAM) f, TRUE);

		f = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Courier New");
		SendMessage(lb, WM_SETFONT, (WPARAM) f, TRUE);
		break;
	}
	case WM_COMMAND:
		if (HIWORD(wp) == BN_CLICKED) {
			if ((HWND) lp == btn1) {
				SendMessage(lb, LB_RESETCONTENT, 0, 0);
				EnumWindows(EnumWindowsProc, 0);
			}
			if ((HWND) lp == btn2) {
				UINT i = (UINT) SendMessage(lb, LB_GETCURSEL, 0, 0);
				LPWSTR x = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR) * (UINT) SendMessage(lb, LB_GETTEXTLEN, i, 0));
				SendMessage(lb, LB_GETTEXT, i, (LPARAM) x);
				Borderless(x);
				LocalFree(x);
			}
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	return 0;
}

INT Interface(HINSTANCE hi, HINSTANCE hpi, INT ncs)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hi;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = 0;
	wc.hbrBackground = (HBRUSH) (COLOR_MENU + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = cn;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	if (!RegisterClassEx(&wc)) {
		Error(L"window registration failed");
		return 0;
	}

	HWND hwnd = CreateWindowEx(WS_EX_DLGMODALFRAME, cn, L"borderless", WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 240, 294, NULL, NULL, hi, NULL);
	if (hwnd == NULL) {
		Error(L"window creation failed");
		return 0;
	}

	ShowWindow(hwnd, ncs);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (INT) msg.wParam;
}

INT WINAPI WinMain(HINSTANCE hi, HINSTANCE hpi, LPSTR lpcmd, INT ncs)
{
	INT argc;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLine(), &argc);

	if (argc == 2) {
		Borderless(argv[1]);
	} else {
		Interface(hi, hpi, ncs);
	}
	LocalFree(argv);

	return 0;
}
