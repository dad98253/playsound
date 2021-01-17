// playsound.cpp : Defines the entry point for the application.
//


#include <windows.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include<iostream>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")
#include <CommCtrl.h>
#include <tchar.h>

#include "framework.h"
#include "playsound.h"
#include "resource.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
int playme();

BOOL  CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
DWORD MCIClose(MCIDEVICEID wDeviceID);
void PrintError(LPCTSTR errDesc);
void showError(DWORD dwError);
DWORD MCIPause(MCIDEVICEID wDeviceID);
DWORD MCIResume(MCIDEVICEID wDeviceID);
void GetConfig();
void WriteConfig(int chapter);

int jackiesfile = 916;
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
HWND hWnd = NULL;

INT_PTR CALLBACK  CALLBACK MainFrm(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    hMyWnd = hDlg;
    DWORD opReturn = 0;
    switch (message)
    {
    case WM_COMMAND:
            switch (LOWORD(wParam))
            {
             
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
                break;

            case   ID_FILE_PLAY:
                // set up our window display timer
                if (paused == 0) {
                    if (keepPlaying) {
                        MCIPause(wDeviceID);
                        paused = 1;
                    } else {
                        TimmerID = SetTimer(hWnd, nID120SecondEvent, 1200 * 1000, NULL);
                        keepPlaying = 1;
                        playme();
                    }
                } else {
                    if (keepPlaying) {
                        MCIResume(wDeviceID);
                    }
                    paused = 0;
                }
                if (paused) {
                    SendMessage(GetDlgItem(hDlg, ID_FILE_PLAY), WM_SETTEXT, 0, (LPARAM)_T("Continue"));
                }
                else {
                    SendMessage(GetDlgItem(hDlg, ID_FILE_PLAY), WM_SETTEXT, 0, (LPARAM)_T("Pause"));
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
            case IDCANCEL:
                SendMessage(hDlg, WM_CLOSE, 0, 0);
                return TRUE;
            }
        break;


    case WM_TIMER:
        if (wParam != nID120SecondEvent) return FALSE;
        keepPlaying = 0;
        KillTimer(hWnd, TimmerID);
        SendMessage(GetDlgItem(hDlg, ID_FILE_PLAY), WM_SETTEXT, 0, (LPARAM)_T("Play"));
#ifdef DEBUG
        MessageBeep(MB_ICONEXCLAMATION);
#endif
        return TRUE;
        break;

    case MM_MCINOTIFY:
        switch (wParam)
        {
        case MCI_NOTIFY_ABORTED:
            PrintError(TEXT("Playback of the audio file was somehow aborted."));
            return TRUE;
            break;
        case MCI_NOTIFY_FAILURE:
            PrintError(TEXT("An error occured while playing the audio file."));
            return TRUE;
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
            return TRUE;
            break;
        case MCI_NOTIFY_SUPERSEDED:
            PrintError(TEXT("Another command requested notification from this audio device."));
            return TRUE;
            break;
        }
        return false;

    case WM_INITDIALOG:
    {
        HICON hIcon;

        hIcon = (HICON)LoadImage(hInst,
            MAKEINTRESOURCE(IDI_ICON1),
            IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON),
            0);
        if (hIcon)
        {
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }
        else {
            PrintError(TEXT("LoadImage failed"));
        }
    }
    return TRUE;

    case WM_CLOSE:
        if (MessageBox(hDlg, TEXT("Close the program?"), TEXT("Close"),
            MB_ICONQUESTION | MB_YESNO) == IDYES)
        {
            DestroyWindow(hDlg);
        }
        return TRUE;

    case WM_DESTROY:
        KillTimer(hWnd, TimmerID);
        PostQuitMessage(0);
        return (INT_PTR)TRUE;

    }
    return FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDOK:
            case IDCANCEL:
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
                break;

            default:
                return FALSE;
            }
        }
        break;

    case WM_INITDIALOG:
    {
        HICON hIcon;

        hIcon = (HICON)LoadImage(hInst,
            MAKEINTRESOURCE(IDI_ICON1),
            IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON),
            0);
        if (hIcon)
        {
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }
        else {
            PrintError(TEXT("LoadImage failed"));
        }
    }
    return TRUE;

    default:
        return (INT_PTR)FALSE;
     }
    return 0;
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
//   size_t pReturnValue;
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

    GetConfig();
    WriteConfig(jackiesfile % 1175 + 1);

    wcscpy(wfile, dbloc);
    wcscpy(foundPath, L"");
    breaknow = 0;
    filecount = 0;
    if (!ListDirectoryContents(wfile)) {
        return(0);
    }

    wDeviceID = MCIOpen((LPCTSTR)foundPath);  //Save DeviceID
    opReturn = 0;
    if (wDeviceID != -1)
    {
        ////play
        opReturn = MCIPlay(wDeviceID);
        if (opReturn) {
            DWORD dwError = 0;
            showError(dwError);
   //         char *ResErrorMsg = MCIMsgErr();
        }

    }
    return (int)opReturn;

    return 0;
}

void GetConfig()
{
    HKEY key;
    LSTATUS lResult;

    if ((lResult  = RegOpenKeyEx(HKEY_CURRENT_USER,
        L"Software\\Island Christian Academy\\PlayMe", //lpctstr
        0,                      //reserved
        KEY_QUERY_VALUE,
        &key)) == ERROR_SUCCESS) {
        DWORD dsize = sizeof(jackiesfile);
        DWORD dwtype = 0;
        LSTATUS StatusReturn;

        StatusReturn = RegQueryValueEx(key, L"Chapter", NULL, &dwtype,
            (BYTE*)&jackiesfile, &dsize);

        if (StatusReturn) jackiesfile = 915;

        //Finished with key
        RegCloseKey(key);
    } else {
        //key isn't there yet -- normal condition for first time execution by a user -- set defaults
        if (lResult == ERROR_FILE_NOT_FOUND) {
#ifdef DEBUG
            PrintError(TEXT("Chapter Key not found.\n"));
#endif
        } else {
            PrintError(TEXT("Error opening chapter key.\n"));
            return ;
        }
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
    } else {
        //key isn't there yet--set defaults
        if (lResult == ERROR_FILE_NOT_FOUND) {
            PrintError(TEXT("Audio files location not specified - program not properly installed.\n"));
            return;
        } else {
            PrintError(TEXT("Error opening database location key.\n"));
            return;
        }
    }
}

void WriteConfig(int chapter)
{
    HKEY key;
    DWORD lpdw;
    LSTATUS lResult;

    if ((lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
        L"Software\\Island Christian Academy\\PlayMe", //lpctstr
        0,                      //reserved
        (LPWSTR)L"",                     //ptr to null-term string specifying the object type of this key
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &key,
        &lpdw)) == ERROR_SUCCESS) {
        if ((lResult = RegSetValueEx(key, L"chapter", 0, REG_DWORD, (const BYTE *)&chapter, (DWORD)sizeof(chapter) )) == ERROR_SUCCESS) {

        } else {
            if (lResult == ERROR_FILE_NOT_FOUND) {
                PrintError(TEXT("Chapter Key not found - save failed.\n"));
                return;
            } else {
                PrintError(TEXT("Error saving chapter key.\n"));
                return;
            }

        }
        //Finished with keys
        RegCloseKey(key);
    } else {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            PrintError(TEXT("User Key not found - creation failed.\n"));
            return;
        } else {
            PrintError(TEXT("Error creating user key.\n"));
            return;
        }
    }
}


