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
#include "resource.h"

#if _MSC_VER > 1500
#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif	// DEBUG
#endif // _MSC_VER > 1500

#include "playsound.h"

void SetThreadName(DWORD dwThreadID, char* threadName);
void GetConfig();
extern HWND hWnd;
extern HINSTANCE hInst;
extern HWND MasterWindowHandle;
extern INT_PTR CALLBACK MainFrm(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE h0, LPTSTR lpCmdLine, int nCmdShow)
{
    HWND hDlg;
    MSG msg;
    BOOL ret;
#if _MSC_VER > 1500
#ifdef DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif	// DEBUG
#endif // _MSC_VER > 1500

    hInst = hInstance;
    SetThreadName(-1, (char*)"PlayMe");
    GetConfig();
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