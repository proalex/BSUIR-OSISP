#include <Windows.h>
#include <Windowsx.h>
#include <strsafe.h>
#include <vector>

using namespace std;

struct Point
{
	int x;
	int y;

	Point(int _x, int _y) : x(_x), y(_y) {}
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
Function:  ErrorExit

Purpose:   Print WINAPI error message and exit from current process

Input:     LPSTR lpszFunction - name of the function that cause error

Output:    VOID
*/
VOID ErrorExit(LPTSTR lpszFunction)
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}


/*
Function:  RegisterWindowClass

Purpose:   Register window class with default parameters

Input:     LPSTR lpszClassName - name of the window class
WNDPROC lpfnWndProc - pointer to window callback function for message handling

Output:    BOOL - TRUE on success, FALSE on failure
*/
BOOL RegisterWindowClass(LPSTR lpszClassName, WNDPROC lpfnWndProc)
{
	WNDCLASSEX wc;

	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = lpszClassName;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	return RegisterClassEx(&wc);
}

/*
Function:  CreateDefaultWindow

Purpose:   Create window with default parameters

Input:     LPSTR lpszClassName - name of the window class
LPSTR lpszWindowName - window name
INT nCmdShow - specifies how the window is to be shown

Output:    BOOL - TRUE on success, FALSE on failure
*/
BOOL CreateDefaultWindow(LPSTR lpszClassName, LPSTR lpszWindowName, INT nCmdShow)
{
	HWND hMainWnd;

	hMainWnd = CreateWindow(lpszClassName, lpszClassName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(NULL), 0);

	if (hMainWnd)
	{
		ShowWindow(hMainWnd, nCmdShow);
		UpdateWindow(hMainWnd);
	}

	return hMainWnd ? 1 : 0;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	MSG msg;
	LPSTR lpszClassName = "lab1";

	if (!RegisterWindowClass(lpszClassName, WndProc))
		ErrorExit("RegisterClassEx");

	if (!CreateDefaultWindow(lpszClassName, lpszClassName, nCmdShow))
		ErrorExit("CreateWindow");

	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HDC hDC;
	static int x, y;
	static vector<Point> curve;
	static vector<vector<Point>> curves;
	static BOOL bTracking;
	vector<Point>::iterator it;

	switch (uMsg)
	{
	case WM_CREATE:
		hDC = GetDC(hWnd);
		break;
	case WM_LBUTTONDOWN:
		bTracking = TRUE;
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
		MoveToEx(hDC, x, y, NULL);
		curve.push_back(Point(x, y));
		break;
	case WM_LBUTTONUP:
		if (bTracking)
		{
			bTracking = FALSE;
			curves.push_back(curve);
			curve.clear();
		}
		break;
	case WM_MOUSEMOVE:
		if (bTracking)
		{
			x = GET_X_LPARAM(lParam);
			y = GET_Y_LPARAM(lParam);
			LineTo(hDC, x, y);
			curve.push_back(Point(x, y));
		}
		break;
	case WM_PAINT:
		HDC hdc;
		PAINTSTRUCT ps;

		hdc = BeginPaint(hWnd, &ps);

		for (size_t i = 0; i < curves.size(); i++)
		{
			it = curves[i].begin();
			MoveToEx(hdc, it->x, it->y, NULL);

			for (it + 1; it != curves[i].end(); it++)
				LineTo(hdc, it->x, it->y);
		}

		EndPaint(hWnd, &ps);
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		ReleaseDC(hWnd, hDC);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}