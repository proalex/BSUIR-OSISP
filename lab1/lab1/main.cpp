#include <Windows.h>
#include <Windowsx.h>
#include <strsafe.h>
#include "resource.h"

struct Point
{
    int x;
    int y;

    Point(int _x, int _y) : x(_x), y(_y) {}
};

enum Tool
{
    PEN,
    LINE
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
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
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

    hMainWnd = CreateWindow(lpszClassName, lpszClassName, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
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
    static HDC hDC, bufferedDC, tempDC;
    static HBITMAP bufferedBMP, tempBMP;
    static int beginX, beginY, endX, endY, width, height, penWidth = 0;
    static BOOL bTracking = FALSE;
    static HMENU hMenu;
    static Tool currentTool = PEN;
    static RECT rect;
    static COLORREF custColors[16];
    static HPEN hPen;

    switch (uMsg)
    {
    case WM_CREATE:
        hMenu = GetMenu(hWnd);
        hDC = GetDC(hWnd);
        GetClientRect(hWnd, &rect);
        bufferedDC = CreateCompatibleDC(hDC);
        tempDC = CreateCompatibleDC(hDC);
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
        bufferedBMP = CreateCompatibleBitmap(hDC, width, height);
        SelectObject(bufferedDC, bufferedBMP);
        tempBMP = CreateCompatibleBitmap(hDC, width, height);
        SelectObject(tempDC, tempBMP);
        PatBlt(bufferedDC, 0, 0, width, height, WHITENESS);
        hPen = GetStockPen(BLACK_PEN);
        SelectObject(tempDC, hPen);
        break;
    case WM_LBUTTONDOWN:
        bTracking = TRUE;
        beginX = GET_X_LPARAM(lParam);
        beginY = GET_Y_LPARAM(lParam);
        BitBlt(tempDC, 0, 0, width, height, bufferedDC, 0, 0, SRCCOPY);
        MoveToEx(tempDC, beginX, beginY, NULL);
        break;
    case WM_LBUTTONUP:
        if (bTracking)
        {
            bTracking = FALSE;
            BitBlt(bufferedDC, 0, 0, width, height, tempDC, 0, 0, SRCCOPY);
        }

        break;
    case WM_MOUSEMOVE:
        if (bTracking)
        {
            endX = GET_X_LPARAM(lParam);
            endY = GET_Y_LPARAM(lParam);

            switch (currentTool)
            {
            case PEN:
                LineTo(tempDC, endX, endY);
                break;
            case LINE:
                BitBlt(tempDC, 0, 0, width, height, bufferedDC, 0, 0, SRCCOPY);
                MoveToEx(tempDC, beginX, beginY, NULL);
                LineTo(tempDC, endX, endY);
                break;
            }

            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }

        break;
    case WM_PAINT:
        HDC hdc;
        PAINTSTRUCT ps;

        hdc = BeginPaint(hWnd, &ps);

        if (bTracking)
            BitBlt(hdc, 0, 0, width, height, tempDC, 0, 0, SRCCOPY);
        else
            BitBlt(hdc, 0, 0, width, height, bufferedDC, 0, 0, SRCCOPY);

        EndPaint(hWnd, &ps);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_FILE_EXIT:
            SendMessage(hWnd, WM_DESTROY, 0, 0);
            break;
        case ID_TOOL_PEN:
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_PEN, MF_CHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_LINE, MF_UNCHECKED);
            currentTool = PEN;
            break;
        case ID_TOOL_LINE:
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_PEN, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_LINE, MF_CHECKED);
            currentTool = LINE;
            break;
        case ID_COLOR:
            CHOOSECOLOR chooseColor;

            chooseColor.lStructSize = sizeof(CHOOSECOLOR);
            chooseColor.hwndOwner = hWnd;
            chooseColor.hInstance = 0;
            chooseColor.rgbResult = 0;
            chooseColor.lpCustColors = custColors;
            chooseColor.Flags = CC_RGBINIT | CC_FULLOPEN;
            chooseColor.lCustData = 0;
            chooseColor.lpfnHook = 0;
            chooseColor.lpTemplateName = 0;

            if (ChooseColor(&chooseColor))
            {
                hPen = CreatePen(PS_SOLID, penWidth, chooseColor.rgbResult);
                SelectObject(tempDC, hPen);
            }

            break;
        case ID_CLEAR:
            if (!bTracking)
            {
                HBRUSH clearBrush = CreateSolidBrush(RGB(0, 0, 0));

                PatBlt(bufferedDC, 0, 0, width, height, WHITENESS);
                BitBlt(hDC, 0, 0, width, height, bufferedDC, 0, 0, SRCCOPY);
                InvalidateRect(hWnd, &rect, TRUE);
                UpdateWindow(hWnd);
            }

            break;
        }
    
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        DeleteObject(hPen);
        ReleaseDC(hWnd, hDC);
        DeleteDC(bufferedDC);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}