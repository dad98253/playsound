// playsound.cpp : Defines the entry point for the application.
//


#include <windows.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include<iostream>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")

#include "framework.h"
#include "playsound.h"


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
int playme();

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    mciCallBack(HWND, UINT, WPARAM, LPARAM);
DWORD MCIClose(MCIDEVICEID wDeviceID);
void PrintError(LPCTSTR errDesc);
void showError(DWORD dwError);
DWORD MCIPause(MCIDEVICEID wDeviceID);
DWORD MCIResume(MCIDEVICEID wDeviceID);
void GetConfig();
void WriteConfig(int chapter);

int jackiesfile = 0;
int filecount = 0;
int breaknow = 0;
UINT TimmerID = NULL;
UINT nID120SecondEvent = 1;
HWND hMyWnd = NULL;
int keepPlaying = 0;
MCIDEVICEID wDeviceID;
wchar_t foundPath[2048];
wchar_t dbloc[2048];
int paused = 0;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PLAYSOUND, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PLAYSOUND));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PLAYSOUND));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PLAYSOUND);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 500, 250, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

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
    hMyWnd = hWnd;
    DWORD opReturn = 0;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case   ID_FILE_PLAY:
                // set up our window display timer
                if (paused == 0) {
                    if (keepPlaying) {
#ifdef DEBUG
                        PrintError(TEXT("Timer still running"));
#endif
                    }
                    else {
                        TimmerID = SetTimer(hWnd, nID120SecondEvent, 1200 * 1000, NULL);
                        keepPlaying = 1;
                        playme();
                    }
                }
                break;

            case   ID_FILE_PAUSE:
                MCIPause(wDeviceID);
                paused = 1;
                break;

            case   ID_FILE_CONTINUE:
                MCIResume(wDeviceID);
                paused = 0;
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_TIMER:
        if (wParam != nID120SecondEvent) return FALSE;
        keepPlaying = 0;
        KillTimer(hWnd, TimmerID);
#ifdef DEBUG
        MessageBeep(MB_ICONEXCLAMATION);
#endif
        return TRUE;

    case MM_MCINOTIFY:
        switch (wParam)
        {
        case MCI_NOTIFY_ABORTED:
            PrintError(TEXT("Playback of the audio file was somehow aborted."));
            break;
        case MCI_NOTIFY_FAILURE:
            PrintError(TEXT("An error occured while playing the audio file."));
            break;
        case MCI_NOTIFY_SUCCESSFUL:
#ifdef DEBUG
            PrintError(TEXT("Playback of the audio file concluded successfully."));
#endif
            if (paused == 0) {
                opReturn = MCIClose(wDeviceID);
                if (opReturn) {
                    DWORD dwError = 0;
                    showError(dwError);
                    //         char *ResErrorMsg = MCIMsgErr();
                }
                if (keepPlaying) playme();
            }
            break;
        case MCI_NOTIFY_SUPERSEDED:
            PrintError(TEXT("Another command requested notification from this audio device."));
            break;
        }
 //     wParam = (WPARAM)wFlags;
 //     lParam = (LONG)lDevID;
        return false;

    case WM_DESTROY:
        KillTimer(hWnd, TimmerID);
        PostQuitMessage(0);
        break;

    case WM_CREATE:
        GetConfig();
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
     }
    return (INT_PTR)FALSE;
}

void showError(DWORD dwError)
{
    char szErrorBuf[MAXERRORLENGTH];
    MessageBeep(MB_ICONEXCLAMATION);
    if (mciGetErrorString(dwError, (LPWSTR)szErrorBuf, MAXERRORLENGTH))
    {
        MessageBox(NULL, (LPWSTR)szErrorBuf, (LPWSTR)"MCI Error",
            MB_ICONEXCLAMATION);
    }
    else
    {
        MessageBox(NULL, (LPWSTR)"Unknown Error", (LPWSTR)"MCI Error",
            MB_ICONEXCLAMATION);
    }
}

MCIDEVICEID MCIOpen(LPCTSTR strPath)
{
    MCI_OPEN_PARMS mciOP;
    DWORD opReturn;

    mciOP.lpstrDeviceType = NULL;
    mciOP.lpstrElementName = strPath;  //Set the .wav file name to open
    opReturn = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT, (DWORD)(LPVOID)&mciOP);
    if (!opReturn) return mciOP.wDeviceID;

    showError(opReturn);
        //         char *ResErrorMsg = MCIMsgErr();
    return -1;
}

DWORD MCIClose(MCIDEVICEID wDeviceID)
{
    MCI_GENERIC_PARMS lpClose;
    DWORD opReturn;
    lpClose.dwCallback = (DWORD)hMyWnd;
//    MCI_CLOSE_PARMS lpClose;
    opReturn = mciSendCommand(wDeviceID, MCI_CLOSE, MCI_WAIT, (DWORD)(LPMCI_GENERIC_PARMS)&lpClose);
//    return mciSendCommand( wDeviceID, MCI_CLOSE, MCI_WAIT,(DWORD)(LPMCI_GENERIC_PARMS)&lpClose);
    if (!opReturn) return(0);

    showError(opReturn);
    //         char *ResErrorMsg = MCIMsgErr();
    return (-1);

}

DWORD MCISeek(MCIDEVICEID wDeviceID, int sec)
{
    MCI_SEEK_PARMS SeekParms;
    SeekParms.dwTo = (sec) * 1000;
    return mciSendCommand(wDeviceID, MCI_SEEK, MCI_TO, (DWORD)(LPVOID)&SeekParms);
}

DWORD MCIPlay(MCIDEVICEID wDeviceID)
{
    MCI_PLAY_PARMS mciPP;
//    mciPP.dwCallback = (DWORD_PTR)mciCallBack;
    mciPP.dwCallback = (DWORD)hMyWnd;
    return mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY , (DWORD)&mciPP);
}

DWORD MCIPause(MCIDEVICEID wDeviceID)
{
    MCI_GENERIC_PARMS mciPause;
    DWORD opReturn;
    //    mciPP.dwCallback = (DWORD_PTR)mciCallBack;
    mciPause.dwCallback = (DWORD)hMyWnd;
    opReturn = mciSendCommand(wDeviceID, MCI_PAUSE, 0, (DWORD)&mciPause);
    if (!opReturn) return(0);

    showError(opReturn);
    //         char *ResErrorMsg = MCIMsgErr();
    return (-1);
}

DWORD MCIResume(MCIDEVICEID wDeviceID)
{
    MCI_GENERIC_PARMS mciPause;
    DWORD opReturn;
    //    mciPP.dwCallback = (DWORD_PTR)mciCallBack;
    mciPause.dwCallback = (DWORD)hMyWnd;
    opReturn = mciSendCommand(wDeviceID, MCI_RESUME, 0, (DWORD)&mciPause);
    if (!opReturn) return(0);

    showError(opReturn);
    //         char *ResErrorMsg = MCIMsgErr();
    return (-1);
}

DWORD MCIPlayFrom(MCIDEVICEID wDeviceID, int sec)
{
    MCI_PLAY_PARMS play;
    play.dwFrom = sec * 1000;//Play From sec*1000 ms
    return mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY | MCI_FROM | MCI_WAIT, (DWORD)&play);
}

bool ListDirectoryContents(const wchar_t* sDir)
{
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;

    wchar_t sPath[2048];

    if (breaknow) return false;
    //Specify a file mask. *.* = We want everything!
    wsprintf(sPath, L"%s\\*.*", sDir);

    if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    do
    {
        //Find first file will always return "."
        //    and ".." as the first two directories.
        if (wcscmp(fdFile.cFileName, L".") != 0
            && wcscmp(fdFile.cFileName, L"..") != 0)
        {
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);

            //Is the entity a File or Folder?
            if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                ListDirectoryContents(sPath); //Recursion, I love it!
            }
            else {
                filecount++;
                if (filecount == jackiesfile) {
                    breaknow = 1;
                    wcscpy(foundPath, sPath);
//                    FindClose(hFind); //Always, Always, clean things up!
//                    return false;
                    break;
                }
            }
            if (breaknow)break;
        }
    } while (FindNextFile(hFind, &fdFile)); //Find the next file.

//    if (!breaknow)FindClose(hFind); //Always, Always, clean things up!
    FindClose(hFind); //Always, Always, clean things up!

    return true;
}

LPCTSTR ErrorMessage(DWORD error)
{
    LPVOID lpMsgBuf;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL);

    return((LPCTSTR)lpMsgBuf);
}

void PrintError(LPCTSTR errDesc)
{
    LPCTSTR errMsg = ErrorMessage(GetLastError());
   // _tprintf(TEXT("\n** ERROR ** %s: %s\n"), errDesc, errMsg);
    MessageBox(NULL, errMsg,
        errDesc,
        MB_ICONWARNING);
    LocalFree((LPVOID)errMsg);
}

int playme()
{
    size_t pReturnValue;
    wchar_t wfile[250];
    size_t sizeInWords = 250;
    char mbstr[251];
    size_t count;
    mbstr[0] = '\000';
    count = 250 - 1;
    wchar_t szFileName[] = L"jakciesfile";
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BOOL fSuccess = FALSE;
    DWORD dwRetVal = 0;
    UINT uRetVal = 0;
    DWORD opReturn = 0;

    DWORD dwBytesRead = 0;
    DWORD dwBytesWritten = 0;

    TCHAR szTempFileName[MAX_PATH];
    TCHAR lpTempPathBuffer[MAX_PATH];
#define BUFSIZE 250
    char  chBuffer[BUFSIZE];

    GetConfig();
    WriteConfig(jackiesfile + 1);
/*
    hFile = CreateFile(szFileName,             // name of the read
        (GENERIC_READ | GENERIC_WRITE),           // open for reading and writing
        FILE_SHARE_READ,        // share
        NULL,                   // default security
        OPEN_ALWAYS,          // open existing file or crfeate o9ne
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no attr. template
    if (hFile == INVALID_HANDLE_VALUE)
    {
        PrintError(TEXT("CreateFile failed"));
        return 0;
    }
    else
    {
        if (ReadFile(hFile, chBuffer, BUFSIZE-1, &dwBytesRead, NULL))
        {
            if(dwBytesRead)sscanf(chBuffer,"%i", &jackiesfile);
            sprintf(chBuffer, "%i\n", jackiesfile + 1);
            SetFilePointer(hFile,0,NULL,FILE_BEGIN);
            dwBytesRead = strlen(chBuffer);

            fSuccess = WriteFile(hFile,
                chBuffer,
                dwBytesRead,
                &dwBytesWritten,
                NULL);
            if (!fSuccess)
            {
                PrintError(TEXT("WriteFile failed"));
                return (5);
            }
        }
        else
        {
            PrintError(TEXT("ReadFile failed"));
            return (6);
        }

    }
    
    if (!CloseHandle(hFile))
    {
        PrintError(TEXT("CloseHandle failed"));
        return 0;
    }

    */

    errno_t err;
//    strcat(mbstr, "\\\\TURK\\dad\\hdb2\\hdb2\\Bible stuff\\NIV\\");
//    strcat(mbstr, dbloc);
    
//    err = mbstowcs_s(&pReturnValue,wfile,sizeInWords,mbstr,count);

    wcscpy(wfile, dbloc);
    wcscpy(foundPath, L"");
    breaknow = 0;
    filecount = 0;
    if(!ListDirectoryContents(wfile))return(0);


    wDeviceID = MCIOpen((LPCTSTR)foundPath);  //Save DeviceID
    opReturn = 0;
    if (wDeviceID != -1)
    {
        //MCI_SET_PARMS mciSet;
        //mciSet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;//set time format to milliseconds
        //opReturn = mciSendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)(LPVOID)&mciSet);

        ////set the position at 30s.
        //opReturn = MCISeek(wDeviceID, 30);

        ////play
        opReturn = MCIPlay(wDeviceID);
        if (opReturn) {
            DWORD dwError = 0;
            showError(dwError);
   //         char *ResErrorMsg = MCIMsgErr();
        }
//        MCIPause(wDeviceID);
       // opReturn = MCIPlayFrom(wDeviceID, 30);

    }
    return (int)opReturn;

    return 0;
}

void GetConfig()
{

    HKEY key;
    DWORD lptumble;
    LSTATUS lResult;
    //DWORD lpdw;

    if ((lResult  = RegOpenKeyEx(HKEY_CURRENT_USER,
        L"Software\\Island Christian Academy\\PlayMe", //lpctstr
        0,                      //reserved
        KEY_QUERY_VALUE,
        &key)) == ERROR_SUCCESS)
    {
        DWORD dsize = sizeof(jackiesfile);
        DWORD dwtype = 0;
        LSTATUS StatusReturn;

        StatusReturn = RegQueryValueEx(key, L"Chapter", NULL, &dwtype,
            (BYTE*)&jackiesfile, &dsize);

        if (StatusReturn) jackiesfile = 915;

        //Finished with key
        RegCloseKey(key);
    }
    else //key isn't there yet--set defaults
    {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            PrintError(TEXT("Key not found.\n"));
            return ;
        }
        else {
            PrintError(TEXT("Error opening key.\n"));
            return ;
        }
        //jackiesfile = 915;
    }

    if ((lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"Software\\Island Christian Academy\\PlayMe", //lpctstr
        0,                      //reserved
        KEY_QUERY_VALUE,
        &key)) == ERROR_SUCCESS)
    {
        DWORD dsize = sizeof(dbloc);
        DWORD dwtype = 0;
        LSTATUS StatusReturn;

        StatusReturn = RegQueryValueEx(key, L"DataBaseLocation", NULL, &dwtype,
            (BYTE*)&dbloc, &dsize);
        if (StatusReturn) wcscpy(dbloc, L"\\\\TURK\\dad\\hdb2\\hdb2\\Bible stuff\\NIV\\");

        //Finished with key
        RegCloseKey(key);
    }
    else //key isn't there yet--set defaults
    {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            PrintError(TEXT("Key not found.\n"));
            return;
        }
        else {
            PrintError(TEXT("Error opening key.\n"));
            return;
        }

  //      wcscpy(dbloc, L"\\\\TURK\\dad\\hdb2\\hdb2\\Bible stuff\\NIV\\");
    }

}

void WriteConfig(int chapter)
{

    HKEY key;
    DWORD lpdw;
    DWORD lptumble;
    LSTATUS lResult;

    if ((lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
        L"Software\\Island Christian Academy\\PlayMe", //lpctstr
        0,                      //reserved
        (LPWSTR)L"",                     //ptr to null-term string specifying the object type of this key
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &key,
        &lpdw)) == ERROR_SUCCESS)

    {

        if ((lResult = RegSetValueEx(key, L"chapter", 0, REG_DWORD,
            (const BYTE *)&chapter, (DWORD)sizeof(chapter) )) == ERROR_SUCCESS) {

        }
        else {
            if (lResult == ERROR_FILE_NOT_FOUND) {
                PrintError(TEXT("Key not found.\n"));
                return;
            }
            else {
                PrintError(TEXT("Error opening key.\n"));
                return;
            }

        }

        //Finished with keys
        RegCloseKey(key);
    }
    else {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            PrintError(TEXT("Key not found.\n"));
            return;
        }
        else {
            PrintError(TEXT("Error opening key.\n"));
            return;
        }

    }
    
}
