#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")

#pragma comment(lib, "ComCtl32.lib")

#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include<iostream>
#include "resource.h"

#if _MSC_VER > 1500
#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif	// DEBUG
#endif // _MSC_VER > 1500

#include "playsound.h"

void SetThreadName(DWORD dwThreadID, char* threadName);
extern int GetConfig( int typeGet);
extern void WriteConfig(int chapter);
extern HWND hWnd;
extern HINSTANCE hInst;
extern HWND MasterWindowHandle;
extern INT_PTR CALLBACK MainFrm(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern bool LoadDirectoryContents(const wchar_t* sDir, int AudioFiles);
extern void PrintError(LPCTSTR errDesc);
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern wchar_t dbloc[2048];
extern wchar_t textdbloc[2048];
extern char * foundTextPath;
extern LOGFONT lfold;
extern int globalnCmdShow;

#define MAX_LOADSTRING 100
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the text window class name
TCHAR sztext[100000];
TCHAR* lpsztext;
char text[100000];
char* ptext;



int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE h0, LPTSTR lpCmdLine, int nCmdShow)
{
    HWND hDlg;
    MSG msg;
    BOOL ret;
    wchar_t wfile[250];
    wchar_t foundPath[2048];
#if _MSC_VER > 1500
#ifdef DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif	// DEBUG
#endif // _MSC_VER > 1500

    hInst = hInstance;
    globalnCmdShow = nCmdShow;
    SetThreadName(-1, (char*)"PlayMe");
    GetConfig(15);
    wcscpy(wfile, dbloc);
    wcscpy(foundPath, L"");
    if (!LoadDirectoryContents(wfile, AUDIOFILES)) {
        return(0);
    }
    wcscpy(wfile, textdbloc);
    wcscpy(foundPath, L"");
    if (!LoadDirectoryContents(wfile, TEXTFILES)) {
        return(0);
    }
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PLAYSOUND, szWindowClass, MAX_LOADSTRING);
   
    MyRegisterClass(hInstance);

   
    InitCommonControls();

    hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN), 0, MainFrm, 0);
    ShowWindow(hDlg, nCmdShow);

    while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
        if (ret == -1) return -1;

        if (!IsDialogMessage(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLAYSOUND));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PLAYSOUND);
//    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable
    int x;
    int y;
    int nWidth;
    int nHeight;
    if (WinLocIsSaved) {
        x = WinLocSave.left;
        y = WinLocSave.top;
    }
    else {
        x = CW_USEDEFAULT;
        y = 0;
    }
    if (WinSizeIsSaved) {
        nWidth = width;
        nHeight = heigth;
    }
    else {
        nWidth = CW_USEDEFAULT;
        nHeight = 0;
    }
    hTextWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        x, y, nWidth, nHeight, nullptr, nullptr, hInstance, nullptr);

    if (!hTextWnd)
    {
        return FALSE;
    }

    ShowWindow(hTextWnd, nCmdShow);
    UpdateWindow(hTextWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    ptext = text;
    text[0] = '\000';
    FILE* pFile;
    CHOOSEFONT cf = { 0 };
    HFONT hFontNew;
    LOGFONT lf;
    HDC hDC;
    static HANDLE hFont;
    static bool move = false;
    static bool resize = false;

    static TCHAR lpszLatin[] = L"Lorem ipsum dolor sit amet, consectetur "
        L"adipisicing elit, sed do eiusmod tempor "
        L"incididunt ut labore et dolore magna "
        L"aliqua. Ut enim ad minim veniam, quis "
        L"nostrud exercitation ullamco laboris nisi "
        L"ut aliquip ex ea commodo consequat. Duis "
        L"aute irure dolor in reprehenderit in "
        L"voluptate velit esse cillum dolore eu "
        L"fugiat nulla pariatur. Excepteur sint "
        L"occaecat cupidatat non proident, sunt "
        L"in culpa qui officia deserunt mollit "
        L"anim id est laborum.\r\n";

    switch (message)
    {
    case WM_CREATE:
    {
        lpsztext = lpszLatin;
        if (foundTextPath != NULL) {
            pFile = fopen(foundTextPath, "r");
            if (pFile != NULL)
            {
                lpsztext = sztext;
                while (fgets(ptext, 100000, pFile) != NULL) {
                    int lxx = strlen(ptext);
                    ptext[lxx - 1] = '\r';
                    ptext[lxx] = '\n';
                    ptext[lxx + 1] = '\000';
                    lxx = strlen(ptext);
                    ptext = ptext + lxx;
                }
                fclose(pFile);
                ptext = text;
                int num_chars = MultiByteToWideChar(CP_UTF8, 0, ptext, strlen(ptext), NULL, 0);
                MultiByteToWideChar(CP_UTF8, 0, ptext, strlen(ptext), lpsztext, num_chars);
                lpsztext[num_chars] = L'\0';
            }
        }
        hwndEdit = CreateWindowEx(
            0, L"EDIT",   // predefined class 
            NULL,         // no window title 
            WS_CHILD | WS_VISIBLE | WS_VSCROLL |
            ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
            0, 0, 0, 0,   // set size in WM_SIZE message 
            hWnd,         // parent window 
            (HMENU)ID_EDITCHILD,   // edit control ID 
            (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
            NULL);        // pointer not needed 

        hDC = GetDC(NULL);
        if (hDC)
        {
            HFONT hFont = (HFONT)SendMessage(hwndEdit, WM_GETFONT, 0, 0);
            if (NULL == hFont) hFont = (HFONT)GetStockObject(SYSTEM_FONT);
            if (!GetObject(hFont, sizeof(lf), (LPSTR)&lf))
            {
                PrintError(TEXT("Error getting font object.\n"));
            }
            if ((GetConfig(2) == 0) && (FontChanged == TRUE)) {
                lf = lfold;
                lf.lfHeight = -MulDiv(120, GetDeviceCaps(hDC, LOGPIXELSY), 720);
                hFontNew = CreateFontIndirect(&lf);
                if (hFontNew)
                {
                    if (hFont) DeleteObject(hFont);
                    hFont = hFontNew;
                    SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
                }
                FontChanged = FALSE;
            }
            ReleaseDC(NULL, hDC);
        }



        // Add text to the window. 
        SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)lpsztext);
        break;
    }


    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            //                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            HFONT hFontNew;
            LOGFONT lf;
            HDC hDC;
            static HANDLE hFont;
            hDC = GetDC(NULL);
            if (hDC)
            {
                cf.lStructSize = sizeof(cf);
                cf.hwndOwner = hWnd;
                cf.lpLogFont = &lf;
                cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
                cf.nFontType = SCREEN_FONTTYPE;

                lf.lfHeight = -MulDiv(120, GetDeviceCaps(hDC, LOGPIXELSY), 720);

                if (ChooseFont(&cf))
                {
                    hFontNew = CreateFontIndirect(&lf);
                    if (hFontNew)
                    {
                        if (hFont) DeleteObject(hFont);
                        hFont = hFontNew;
                        SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
                    }
                    lfold = lf;
                    FontChanged = TRUE;
                    WriteConfig(-1);
                }
                ReleaseDC(NULL, hDC);
            }
            SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)lpsztext);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    /*   case WM_PAINT:
           {
               PAINTSTRUCT ps;
               HDC hdc = BeginPaint(hWnd, &ps);
               // TODO: Add any drawing code that uses hdc here...
               EndPaint(hWnd, &ps);
           }
           break;*/
    case WM_SETFOCUS:
        SetFocus(hwndEdit);
        return 0;


    case WM_SIZE:
        // Make the edit control the size of the window's client area. 

        MoveWindow(hwndEdit,
            0, 0,                  // starting x- and y-coordinates 
            LOWORD(lParam),        // width of client area 
            HIWORD(lParam),        // height of client area 
            TRUE);                 // repaint window 
        resize = true;
        width = LOWORD(lParam);
        heigth = HIWORD(lParam);
        WinSizeIsSaved = TRUE;
        return 0;


    case WM_MOVING:
    {
        move = true;
        RECT* r = (LPRECT)lParam;
        WinLocSave = *r;
        WinLocIsSaved = TRUE;
        return 0;
    }
    break;


    case WM_EXITSIZEMOVE:
    {
        if (move || resize)
        {
//          save new location in registry
            WriteConfig(-1);
            move = false;
            resize = false;
        }
        return 0;
    }
    break;



    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
