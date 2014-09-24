#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <Windowsx.h>
#include <strsafe.h>
#include <string>
#include "resource.h"

using namespace std;

struct Point
{
    int x;
    int y;

    Point(int _x, int _y) : x(_x), y(_y) {}
};

enum Tool
{
    PEN,
    LINE,
    ELLIPSE,
    RECTANGLE,
    TEXT
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

Output:    HWND - window handle on success, NULL on failure
*/
HWND CreateDefaultWindow(LPSTR lpszClassName, LPSTR lpszWindowName, INT nCmdShow)
{
    HWND hMainWnd;

    hMainWnd = CreateWindow(lpszClassName, lpszClassName, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(NULL), 0);

    if (hMainWnd)
    {
        ShowWindow(hMainWnd, nCmdShow);
        UpdateWindow(hMainWnd);
    }

    return hMainWnd;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    MSG msg;
    HWND hWnd;
    HACCEL accelerators;
    LPSTR lpszClassName = "lab1";

    if (!RegisterWindowClass(lpszClassName, WndProc))
        ErrorExit("RegisterClassEx");

    if (!(hWnd = CreateDefaultWindow(lpszClassName, lpszClassName, nCmdShow)))
        ErrorExit("CreateWindow");

    accelerators = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_ACCELERATOR1));

    if (!accelerators)
        ErrorExit("LoadAccelerators");

    while (GetMessage(&msg, 0, 0, 0))
    {
        if (!TranslateAccelerator(hWnd, accelerators, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam;
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, 0);
            break;
        }

        break;
    }

    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HDC hDC, bufferedDC, tempDC;
    static HBITMAP bufferedBMP, tempBMP;
    static INT beginX, beginY, endX, endY, width, height, textX = 0, textY = 0, penWidth = 0;
    static BOOL bTracking = FALSE;
    static HMENU hMenu;
    static Tool currentTool = PEN;
    static RECT rect;
    static COLORREF custColors[16];
    static HPEN hPen;
    static COLORREF color = RGB(0, 0, 0);
    static string text;

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
        SelectObject(tempDC, GetStockObject(HOLLOW_BRUSH));
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

            switch (currentTool)
            {
            case TEXT:
                text.clear();
                textX = GET_X_LPARAM(lParam);
                textY = GET_Y_LPARAM(lParam);
                break;
            }

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
            case ELLIPSE:
                BitBlt(tempDC, 0, 0, width, height, bufferedDC, 0, 0, SRCCOPY);
                Ellipse(tempDC, beginX, beginY, endX, endY);
                break;
            case RECTANGLE:
                BitBlt(tempDC, 0, 0, width, height, bufferedDC, 0, 0, SRCCOPY);
                Rectangle(tempDC, beginX, beginY, endX, endY);
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
        if (bTracking)
            break;

        switch (LOWORD(wParam))
        {
        case ID_FILE_EXIT:
            SendMessage(hWnd, WM_DESTROY, 0, 0);
            break;
        case ID_TOOL_PEN:
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_PEN, MF_CHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_LINE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_ELLIPSE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_RECTANGLE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_TEXT, MF_UNCHECKED);
            currentTool = PEN;
            break;
        case ID_TOOL_LINE:
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_PEN, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_LINE, MF_CHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_ELLIPSE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_RECTANGLE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_TEXT, MF_UNCHECKED);
            currentTool = LINE;
            break;
        case ID_TOOL_ELLIPSE:
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_PEN, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_LINE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_ELLIPSE, MF_CHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_RECTANGLE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_TEXT, MF_UNCHECKED);
            currentTool = ELLIPSE;
            break;
        case ID_TOOL_RECTANGLE:
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_PEN, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_LINE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_ELLIPSE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_RECTANGLE, MF_CHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_TEXT, MF_UNCHECKED);
            currentTool = RECTANGLE;
            break;
        case ID_TOOL_TEXT:
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_PEN, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_LINE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_ELLIPSE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_RECTANGLE, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 1), ID_TOOL_TEXT, MF_CHECKED);
            textX = 0;
            textY = 0;
            text.clear();
            currentTool = TEXT;
            break;
        case ID_COLOR:
            {
                CHOOSECOLOR chooseColor;

                chooseColor.lStructSize = sizeof(CHOOSECOLOR);
                chooseColor.hwndOwner = hWnd;
                chooseColor.hInstance = NULL;
                chooseColor.rgbResult = NULL;
                chooseColor.lpCustColors = custColors;
                chooseColor.Flags = CC_RGBINIT | CC_FULLOPEN;
                chooseColor.lCustData = NULL;
                chooseColor.lpfnHook = NULL;
                chooseColor.lpTemplateName = NULL;

                if (ChooseColor(&chooseColor))
                {
                    color = chooseColor.rgbResult;
                    hPen = CreatePen(PS_SOLID, penWidth, chooseColor.rgbResult);
                    SelectObject(tempDC, hPen);
                }
            }

            break;
        case ID_FILE_OPEN:
            {
                OPENFILENAME openFileName;
                char lpszFullPath[MAX_PATH], lpszFileName[MAX_PATH], lpszInitialDir[MAX_PATH];
                HENHMETAFILE hemf;
                ENHMETAHEADER emh;
                RECT emfRect;
                INT x1, x2, y1, y2;

                ZeroMemory(&openFileName, sizeof(OPENFILENAME));
                ZeroMemory(&lpszFullPath, sizeof(lpszFullPath));
                ZeroMemory(&lpszFileName, sizeof(lpszFileName));
                ZeroMemory(&lpszInitialDir, sizeof(lpszInitialDir));
                openFileName.lStructSize = sizeof(OPENFILENAME);
                openFileName.hwndOwner = hWnd;
                openFileName.hInstance = GetModuleHandle(NULL);
                openFileName.lpstrFilter = "Metafile (*.emf)\0*.emf\0";
                openFileName.nFilterIndex = 1;
                openFileName.lpstrFile = lpszFullPath;
                openFileName.nMaxFile = sizeof(lpszFullPath);
                openFileName.lpstrFileTitle = lpszFileName;
                openFileName.nMaxFileTitle = sizeof(lpszFileName);
                openFileName.lpstrInitialDir = lpszInitialDir;
                openFileName.lpstrTitle = "Open file...";
                openFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

                if (GetOpenFileName(&openFileName))
                {
                    hemf = GetEnhMetaFile(lpszFullPath);

                    if (!hemf)
                        ErrorExit("GetEnhMetaFile");

                    GetEnhMetaFileHeader(hemf, sizeof(ENHMETAHEADER), &emh);
                    x1 = emh.rclBounds.left;
                    x2 = emh.rclBounds.right;
                    y1 = emh.rclBounds.top;
                    y2 = emh.rclBounds.bottom;
                    SetRect(&emfRect, x1, y1, x2, y2);
                    PatBlt(bufferedDC, 0, 0, width, height, WHITENESS);
                    BitBlt(hDC, 0, 0, width, height, bufferedDC, 0, 0, SRCCOPY);
                    PlayEnhMetaFile(bufferedDC, hemf, &emfRect);
                    InvalidateRect(hWnd, &rect, TRUE);
                    UpdateWindow(hWnd);
                    DeleteEnhMetaFile(hemf);
                }
            }

            break;
        case ID_FILE_SAVE:
            {
                OPENFILENAME openFileName;
                char lpszFullPath[MAX_PATH], lpszFileName[MAX_PATH], lpszInitialDir[MAX_PATH];

                ZeroMemory(&openFileName, sizeof(OPENFILENAME));
                ZeroMemory(&lpszFullPath, sizeof(lpszFullPath));
                ZeroMemory(&lpszFileName, sizeof(lpszFileName));
                ZeroMemory(&lpszInitialDir, sizeof(lpszInitialDir));
                openFileName.lStructSize = sizeof(OPENFILENAME);
                openFileName.hwndOwner = hWnd;
                openFileName.hInstance = GetModuleHandle(NULL);
                openFileName.lpstrFilter = "Metafile (*.emf)\0*.emf\0";
                openFileName.nFilterIndex = 1;
                openFileName.lpstrFile = lpszFullPath;
                openFileName.nMaxFile = sizeof(lpszFullPath);
                openFileName.lpstrFileTitle = lpszFileName;
                openFileName.nMaxFileTitle = sizeof(lpszFileName);
                openFileName.lpstrInitialDir = lpszInitialDir;
                openFileName.lpstrTitle = "Save file As...";
                openFileName.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

                if (GetSaveFileName(&openFileName))
                {
                    HDC metaFile;
                    INT end = strlen(lpszFullPath);
                    INT ext = end - 4;

                    if (strcmp(&lpszFullPath[ext], ".emf"))
                        sprintf(&lpszFullPath[end], "%s", ".emf");
                    
                    metaFile = CreateEnhMetaFile(bufferedDC, lpszFullPath, NULL, NULL);

                    if (!metaFile)
                        ErrorExit("CreateEnhMetaFile");

                    BitBlt(metaFile, 0, 0, width, height, bufferedDC, 0, 0, SRCCOPY);
                    DeleteEnhMetaFile(CloseEnhMetaFile(metaFile));
                }
            }

            break;
        case ID_CLEAR:
            PatBlt(bufferedDC, 0, 0, width, height, WHITENESS);
            BitBlt(hDC, 0, 0, width, height, bufferedDC, 0, 0, SRCCOPY);
            InvalidateRect(hWnd, &rect, TRUE);
            UpdateWindow(hWnd);
            break;
        case ID_THICKNESS_1:
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_1, MF_CHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_2, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_3, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_4, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_5, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_6, MF_UNCHECKED);
            penWidth = 1;
            hPen = CreatePen(PS_SOLID, penWidth, color);
            SelectObject(tempDC, hPen);
            break;
        case ID_THICKNESS_2:
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_1, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_2, MF_CHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_3, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_4, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_5, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_6, MF_UNCHECKED);
            penWidth = 2;
            hPen = CreatePen(PS_SOLID, penWidth, color);
            SelectObject(tempDC, hPen);
            break;
        case ID_THICKNESS_3:
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_1, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_2, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_3, MF_CHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_4, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_5, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_6, MF_UNCHECKED);
            penWidth = 3;
            hPen = CreatePen(PS_SOLID, penWidth, color);
            SelectObject(tempDC, hPen);
            break;
        case ID_THICKNESS_4:
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_1, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_2, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_3, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_4, MF_CHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_5, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_6, MF_UNCHECKED);
            penWidth = 4;
            hPen = CreatePen(PS_SOLID, penWidth, color);
            SelectObject(tempDC, hPen);
            break;
        case ID_THICKNESS_5:
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_1, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_2, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_3, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_4, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_5, MF_CHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_6, MF_UNCHECKED);
            penWidth = 5;
            hPen = CreatePen(PS_SOLID, penWidth, color);
            SelectObject(tempDC, hPen);
            break;
        case ID_THICKNESS_6:
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_1, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_2, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_3, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_4, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_5, MF_UNCHECKED);
            CheckMenuItem(GetSubMenu(hMenu, 3), ID_THICKNESS_6, MF_CHECKED);
            penWidth = 6;
            hPen = CreatePen(PS_SOLID, penWidth, color);
            SelectObject(tempDC, hPen);
            break;
        case ID_ABOUT:
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, &AboutDlgProc);
            break;
        }
    
        break;
        case WM_CHAR:
            if (currentTool == TEXT && textX && textY && bTracking == FALSE)
            {
                CHAR ch = (CHAR)wParam;

                if (ch == VK_RETURN)
                {
                    textX = 0;
                    textY = 0;
                    text.clear();
                    break;
                }
                else if (ch == VK_BACK)
                    break;
                else
                    text += ch;


                TextOut(bufferedDC, textX, textY, text.c_str(), strlen(text.c_str()));
                InvalidateRect(hWnd, NULL, FALSE);
                UpdateWindow(hWnd);
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