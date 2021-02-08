// playsound.cpp : Defines the entry point for the application.
//


#include <windows.h>
#include <windowsx.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include<iostream>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")
#include <CommCtrl.h>
#include <tchar.h>
#include <time.h> 

#include "framework.h"
#define MAINDEFS
#include "playsound.h"
#include "Resource.h"

#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE hInst;                                // current instance

int playme();

BOOL  CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SettingsFM(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
DWORD MCIClose(MCIDEVICEID wDeviceID);
void PrintError(LPCTSTR errDesc);
void showError(DWORD dwError);
DWORD MCIPause(MCIDEVICEID wDeviceID);
DWORD MCIResume(MCIDEVICEID wDeviceID);
int GetConfig(int typeGet);
void WriteConfig(int chapter);
bool LoadDirectoryContents(const wchar_t* sDir, int AudioOrText);
bool LoadSubDirectoryContents(const wchar_t* sDir, int * chapterindex, int booknumber, int AudioFiles);
int parseadname(char * scratchstring, char ** bookname, int * booknumber);
bool FindDirectoryContents();
void cleanupheap();
extern BOOL InitInstance(HINSTANCE, int);
extern ATOM MyRegisterClass(HINSTANCE hInstance);
extern bool GetProductAndVersion(char* strProductName, int iProductNameLenIn, char* strProductVersion, int iProductVersionLenIn, char* strProductCopyright, int iProductCopyrightnLenIn);
extern TCHAR sztext[100000];
extern TCHAR* lpsztext;
extern char text[100000];
extern char* ptext;

int jackiesfile = 916;
int filecount = 0;
int breaknow = 0;
UINT TimmerID = NULL;
UINT nID120SecondEvent = 1;
HWND hMyWnd = NULL;
int keepPlaying = 0;
MCIDEVICEID wDeviceID;
wchar_t foundPath[2048];
char * foundTextPath;
wchar_t dbloc[2048];
wchar_t textdbloc[2048];
int paused = 0;
HWND hWnd = NULL;
int globalnCmdShow;


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
                        if (TimeoutSetting) TimmerID = SetTimer(hDlg, nID120SecondEvent, TimeoutSetting * 60000, NULL);
                        keepPlaying = 1;
                        // Perform text window initialization:
                        if (!hTextWnd) {
                            if (!InitInstance(hInst, globalnCmdShow)) {
                                PrintError(TEXT("InitInstance failed."));
                            }
                        }
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

            case IDC_SETTINGS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hDlg, SettingsFM);
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
                if (keepPlaying) {
                    playme();
                } else {
                    SendMessage(GetDlgItem(hDlg, ID_FILE_PLAY), WM_SETTEXT, 0, (LPARAM)_T("Play"));
 //                   CloseWindow(hTextWnd);
                }
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
 //       MyRegisterClass(hInstance);
 //       if (!InitInstance(hInst, globalnCmdShow)) {
 //           PrintError(TEXT("InitInstance failed."));
 //       }

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
        cleanupheap();
#ifdef DEBUG
        fclose(fdb);
#endif
        PostQuitMessage(0);
        return (INT_PTR)TRUE;

    }
    return FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
//    time_t t = time(0);
    HWND hWndStaticCompileDate;
    HWND hWndStaticSavedDate;
    HWND hWndStaticAudioFolderLocation;
    HWND hWndStaticTextFolderLocation;
    HWND hWndStaticVersion;
    HWND hWndStaticCopyRight;
    WCHAR szTemp[2048];                 

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

        GetProductAndVersion(strProductName, iProductNameLenIn, strProductVersion, iProductVersionLenIn, strProductCopyright, iProductCopyrightnLenIn);
        // IDC_STATIC2                 
        char templine[300];
        strcpy(templine, strProductName);
        strcat(templine, " Version ");
        strcat(templine, strProductVersion);
        hWndStaticVersion = GetDlgItem(hDlg, IDC_STATIC2);
        //wcstombs(scratchstring, sPath, wcslen(sPath) + 1);
        //size_t mbstowcs(wchar_t* dest, const char* src, size_t n);
        mbstowcs(szTemp, templine, sizeof(szTemp));
        SetWindowText(hWndStaticVersion, (LPWSTR)szTemp);

        // IDC_STATIC3
        hWndStaticCopyRight = GetDlgItem(hDlg, IDC_STATIC3);
        mbstowcs(szTemp, strProductCopyright, sizeof(szTemp));
        SetWindowText(hWndStaticCopyRight, (LPWSTR)szTemp);

        // IDC_STATIC7                 
        hWndStaticCompileDate = GetDlgItem(hDlg, IDC_STATIC7);
        wsprintf(szTemp, L"Compiled on " __DATE__ " at " __TIME__);
        SetWindowText(hWndStaticCompileDate, (LPWSTR)szTemp);

        // IDC_STATIC4  
        hWndStaticAudioFolderLocation = GetDlgItem(hDlg, IDC_STATIC4);
        wcscpy(szTemp, dbloc);
        SetWindowText(hWndStaticAudioFolderLocation, (LPWSTR)szTemp);

        // IDC_STATIC5  
        hWndStaticTextFolderLocation = GetDlgItem(hDlg, IDC_STATIC5);
        wcscpy(szTemp, textdbloc);
        SetWindowText(hWndStaticTextFolderLocation, (LPWSTR)szTemp);

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
    FILE* pFile;

    GetConfig(1);
    WriteConfig(jackiesfile % TOTNUMCHAPSP1);

    wcscpy(wfile, dbloc);
    wcscpy(foundPath, L"");
    breaknow = 0;
    filecount = 0;
//   if (!ListDirectoryContents(wfile)) {
    if (!FindDirectoryContents()) {
        return(0);
    }
    // find the text
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
            if (!IncludeFootnotes) {
                ptext = text;
                char* left;
                char* right;
                char* crlf;
                while (ptext) {
                    // check for a '{', if none, we are done
                    if ( !(left = strchr(ptext, '{')) ) break;
                    // '{' found, find the matching '}', if none, continue
                    if ( !(right = strchr(left, '}')) ) {
                        ptext = left + 1;
                        continue;
                    }
                    // found matchinf '}', now check for a '\r' between them
                    if ( (crlf = strchr(left, '\r')) ) {
                        if (crlf < right) {
                            // '\r' found, replace left with a new '\r\n' (unless right+1 = '\r') & increment left past them
                            if (*(right + 1) != '\r') {
                                *left = '\r';
                                left++;
                                *left = '\n';
                                left++;
                                if (*(right + 1) != '[') {
                                    *left = '\t';
                                    left++;
                                }
                            }
                        }
                    }
                    // delete everything from left to the '}' and continue
                    right++;
                    ptext = left;
                    while (*right) {
                        *left = *right;
                        left++;
                        right++;
                    }
                    *left = '\000';
                }
            }
            ptext = text;
            int num_chars = MultiByteToWideChar(CP_UTF8, 0, ptext, strlen(ptext), NULL, 0);
            MultiByteToWideChar(CP_UTF8, 0, ptext, strlen(ptext), lpsztext, num_chars);
            lpsztext[num_chars] = L'\0';
        }
    }
    // display the text
    SetDlgItemText( hTextWnd, ID_EDITCHILD, lpsztext);
//    SendMessage(hwndEdit, WM_SETTEXT, 0, (LPARAM)lpsztext);

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
    LPCSTR lpTitleString;
    char* tempstr;
    tempstr = (char*)malloc(300);
    char* presentBook;
    int presentChapter;
    int chapterIndex;
    chapterIndex = (jackiesfile - 1) % TOTNUMCHAPS;
    presentBook = (*(ChapterData + chapterIndex))->Bookname;
    presentChapter = (*(ChapterData + chapterIndex))->ChapterNumber;
    sprintf(tempstr, "PlayMe - %s   Chapter %d", presentBook, presentChapter);
    lpTitleString = tempstr;
    SetWindowTextA( hMyWnd, lpTitleString);
    sprintf(tempstr, "%s   Chapter %d", presentBook, presentChapter);
    lpTitleString = tempstr;
    SetWindowTextA( hTextWnd, lpTitleString);
    free(tempstr);

    return (int)opReturn;

//    return 0; // dead code??
}

int GetConfig(int typeGet)
{
    HKEY key;
    LSTATUS lResult;
    DWORD dwtype = REG_NONE;
    DWORD dsize;
    LSTATUS StatusReturn;
    FontChanged = FALSE;
    WinSizeIsSaved = FALSE;
    WinLocIsSaved = FALSE;
    int getChapt;
    int getFont;
    int getSize;
    int getDbLoc;
    getChapt = typeGet % 2;
    getFont = (typeGet >> 1) % 2;
    getSize = (typeGet >> 2) % 2;
    getDbLoc = (typeGet >> 3) % 2;
    int iret = 0;

    if (getChapt) {
        // get the current chapter number, footnote setting, and timeout value
        if ((lResult = RegOpenKeyEx(HKEY_CURRENT_USER,
            L"Software\\Island Christian Academy\\PlayMe", //lpctstr
            0,                      //reserved
            KEY_QUERY_VALUE,
            &key)) == ERROR_SUCCESS) {         
            // get chapter
            dsize = sizeof(jackiesfile);
            StatusReturn = RegQueryValueEx(key, L"Chapter", NULL, &dwtype,
                (BYTE*)&jackiesfile, &dsize);
            if (StatusReturn) jackiesfile = 915;
            // get footnote setting
            dsize = sizeof(IncludeFootnotes);
            StatusReturn = RegQueryValueEx(key, L"IncludeFootnotes", NULL, &dwtype,
                (BYTE*)&IncludeFootnotes, &dsize);
            if (StatusReturn) IncludeFootnotes = TRUE;
            // get timeout value
            dsize = sizeof(TimeoutSetting);
            StatusReturn = RegQueryValueEx(key, L"TimeoutSetting", NULL, &dwtype,
                (BYTE*)&TimeoutSetting, &dsize);
            if (StatusReturn) TimeoutSetting = 20;

            //Finished with key
            RegCloseKey(key);
        }
        else {
            //key isn't there yet -- normal condition for first time execution by a user -- set defaults
            if (lResult == ERROR_FILE_NOT_FOUND) {
#ifdef DEBUG
                PrintError(TEXT("Chapter Key not found.\n"));
#endif
                iret += 1;
            }
            else {
                PrintError(TEXT("Error opening chapter key.\n"));
                iret += 2;
            }
        }
    }

    if (getSize) {
        // get the current window size
        if ((lResult = RegOpenKeyEx(HKEY_CURRENT_USER,
            L"Software\\Island Christian Academy\\PlayMe", //lpctstr
            0,                      //reserved
            KEY_QUERY_VALUE,
            &key)) == ERROR_SUCCESS) {

            // get width
            dsize = sizeof(width);
            StatusReturn = RegQueryValueEx(key, L"width", NULL, &dwtype,(BYTE*)&width, &dsize);
            // get heigth
            dsize = sizeof(heigth);
            StatusReturn = StatusReturn | RegQueryValueEx(key, L"heigth", NULL, &dwtype, (BYTE*)&heigth, &dsize);
            if (!StatusReturn) WinSizeIsSaved = TRUE;
            // get left
            dsize = sizeof(WinLocSave.left);
            StatusReturn = RegQueryValueEx(key, L"left", NULL, &dwtype, (BYTE*)&(WinLocSave.left), &dsize);
            // get top
            dsize = sizeof(WinLocSave.top);
            StatusReturn = StatusReturn | RegQueryValueEx(key, L"top", NULL, &dwtype, (BYTE*)&(WinLocSave.top), &dsize);
            // get right
            dsize = sizeof(WinLocSave.right);
            StatusReturn = StatusReturn | RegQueryValueEx(key, L"right", NULL, &dwtype, (BYTE*)&(WinLocSave.right), &dsize);
            // get bottom
            dsize = sizeof(WinLocSave.bottom);
            StatusReturn = StatusReturn | RegQueryValueEx(key, L"bottom", NULL, &dwtype, (BYTE*)&(WinLocSave.bottom), &dsize);
            if (!StatusReturn) WinLocIsSaved = TRUE;
            // get the WinSizeLoc struct
            dsize = sizeof(WINDOWPLACEMENT);
            StatusReturn = RegQueryValueEx(key, L"WinSizeLoc", NULL, &dwtype, (BYTE*)&(WinSizeLoc), &dsize);
            if ( !StatusReturn && ( WinSizeLoc.length == sizeof(WINDOWPLACEMENT) ) ) ValidWinSizeLoc = TRUE;
            
            //Finished with key
            RegCloseKey(key);
        }
        else {
            //key isn't there yet -- normal condition for first time execution by a user -- set defaults
            if (lResult == ERROR_FILE_NOT_FOUND) {
#ifdef DEBUG
                PrintError(TEXT("Chapter Key not found.\n"));
#endif
                iret += 10;
            }
            else {
                PrintError(TEXT("Error opening chapter key.\n"));
                iret += 20;
            }
        }
    }

    if (getFont) {
        // get the font settings
        if ((lResult = RegOpenKeyEx(HKEY_CURRENT_USER,
            L"Software\\Island Christian Academy\\PlayMe\\font", //lpctstr
            0,                      //reserved
            KEY_QUERY_VALUE,
            &key)) == ERROR_SUCCESS) {
            /*
            typedef struct tagLOGFONTW
            {
                LONG      lfHeight;
                LONG      lfWidth;
                LONG      lfEscapement;
                LONG      lfOrientation;
                LONG      lfWeight;
                BYTE      lfItalic;
                BYTE      lfUnderline;
                BYTE      lfStrikeOut;
                BYTE      lfCharSet;
                BYTE      lfOutPrecision;
                BYTE      lfClipPrecision;
                BYTE      lfQuality;
                BYTE      lfPitchAndFamily;
                WCHAR     lfFaceName[LF_FACESIZE];
            } LOGFONTW
            */
            dsize = sizeof(lfold.lfFaceName);
            dwtype = REG_NONE;
            DWORD tempdword;

            // get font
            StatusReturn = RegQueryValueEx(key, L"lfFaceName", NULL, &dwtype,
                (BYTE*)&lfold.lfFaceName, &dsize);
            if (!StatusReturn) {
                lfset = 1;
                dsize = sizeof(lfold.lfHeight);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfHeight", NULL, &dwtype, (BYTE*)&lfold.lfHeight, &dsize);
                dsize = sizeof(lfold.lfWidth);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfWidth", NULL, &dwtype, (BYTE*)&lfold.lfWidth, &dsize);
                dsize = sizeof(lfold.lfEscapement);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfEscapement", NULL, &dwtype, (BYTE*)&lfold.lfEscapement, &dsize);
                dsize = sizeof(lfold.lfOrientation);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfOrientation", NULL, &dwtype, (BYTE*)&lfold.lfOrientation, &dsize);
                dsize = sizeof(lfold.lfWeight);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfWeight", NULL, &dwtype, (BYTE*)&lfold.lfWeight, &dsize);
                dsize = sizeof(tempdword);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfItalic", NULL, &dwtype, (BYTE*)&tempdword, &dsize);
                lfold.lfItalic = (BYTE)tempdword;
                dsize = sizeof(tempdword);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfUnderline", NULL, &dwtype, (BYTE*)&tempdword, &dsize);
                lfold.lfUnderline = (BYTE)tempdword;
                dsize = sizeof(tempdword);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfStrikeOut", NULL, &dwtype, (BYTE*)&tempdword, &dsize);
                lfold.lfStrikeOut = (BYTE)tempdword;
                dsize = sizeof(tempdword);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfCharSet", NULL, &dwtype, (BYTE*)&tempdword, &dsize);
                lfold.lfCharSet = (BYTE)tempdword;
                dsize = sizeof(tempdword);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfOutPrecision", NULL, &dwtype, (BYTE*)&tempdword, &dsize);
                lfold.lfOutPrecision = (BYTE)tempdword;
                dsize = sizeof(tempdword);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfClipPrecision", NULL, &dwtype, (BYTE*)&tempdword, &dsize);
                lfold.lfClipPrecision = (BYTE)tempdword;
                dsize = sizeof(tempdword);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfQuality", NULL, &dwtype, (BYTE*)&tempdword, &dsize);
                lfold.lfQuality = (BYTE)tempdword;
                dsize = sizeof(tempdword);
                StatusReturn = StatusReturn | RegQueryValueEx(key, L"lfPitchAndFamily", NULL, &dwtype, (BYTE*)&tempdword, &dsize);
                lfold.lfPitchAndFamily = (BYTE)tempdword;
                if (!StatusReturn) FontChanged = TRUE;
            }
            //Finished with key
            RegCloseKey(key);
        }
        else {
            //key isn't there yet -- normal condition for first time execution by a user -- set defaults
            if (lResult == ERROR_FILE_NOT_FOUND) {
#ifdef DEBUG
                PrintError(TEXT("font Key not found.\n"));
#endif
//                iret += 100;
            }
            else {
                PrintError(TEXT("Error opening font key.\n"));
                iret += 200;
            }
        }
    }

    if (getDbLoc) {
        // get the database locations
        if ((lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"Software\\Island Christian Academy\\PlayMe", //lpctstr
            0,                      //reserved
            KEY_QUERY_VALUE,
            &key)) == ERROR_SUCCESS)
        {

            dsize = sizeof(dbloc);
            DWORD dtsize = sizeof(textdbloc);

            StatusReturn = RegQueryValueEx(key, L"DataBaseLocation", NULL, &dwtype,
                (BYTE*)&dbloc, &dsize);
            if (StatusReturn) wcscpy(dbloc, L"\\\\TURK\\dad\\hdb2\\hdb2\\Bible stuff\\NIV\\");
            StatusReturn = RegQueryValueEx(key, L"TextFileLocation", NULL, &dwtype,
                (BYTE*)&textdbloc, &dtsize);
            if (StatusReturn) wcscpy(textdbloc, L"\\\\TURK\\dad\\hdb2\\hdb2\\Bible stuff\\NIVtext\\");

            //Finished with key
            RegCloseKey(key);
        }
        else {
            //key isn't there yet--set defaults
            if (lResult == ERROR_FILE_NOT_FOUND) {
                PrintError(TEXT("Audio files location not specified - program not properly installed.\n"));
                iret += 1000;
            }
            else {
                PrintError(TEXT("Error opening database location key.\n"));
                iret += 2000;
            }
        }
    }

    return(iret);
}


void WriteConfig(int chapter)
{
    HKEY key;
    DWORD lpdw;
    LSTATUS lResult;
    DWORD tempdword;

    if (chapter > -1) {
        // set chapter, footnote, and timeout
        if ((lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
            L"Software\\Island Christian Academy\\PlayMe", //lpctstr
            0,                      //reserved
            (LPWSTR)L"",                     //ptr to null-term string specifying the object type of this key
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,
            &key,
            &lpdw)) == ERROR_SUCCESS) {
            // save chapter
            if ((lResult = RegSetValueEx(key, L"chapter", 0, REG_DWORD, (const BYTE*)&chapter, (DWORD)sizeof(chapter))) == ERROR_SUCCESS) {

            } else {
                if (lResult == ERROR_FILE_NOT_FOUND) {
                    PrintError(TEXT("Chapter Key not found - save failed.\n"));
                    return;
                }
                else {
                    PrintError(TEXT("Error saving chapter key.\n"));
                    return;
                }
            }
            // save footnote
            if ((lResult = RegSetValueEx(key, L"IncludeFootnotes", 0, REG_DWORD, (const BYTE*)&IncludeFootnotes, (DWORD)sizeof(IncludeFootnotes))) == ERROR_SUCCESS) {

            }
            else {
                if (lResult == ERROR_FILE_NOT_FOUND) {
                    PrintError(TEXT("IncludeFootnotes Key not found - save failed.\n"));
                    return;
                }
                else {
                    PrintError(TEXT("Error saving IncludeFootnotes key.\n"));
                    return;
                }
            }
            // save timeout
            if ((lResult = RegSetValueEx(key, L"TimeoutSetting", 0, REG_DWORD, (const BYTE*)&TimeoutSetting, (DWORD)sizeof(TimeoutSetting))) == ERROR_SUCCESS) {

            }
            else {
                if (lResult == ERROR_FILE_NOT_FOUND) {
                    PrintError(TEXT("TimeoutSetting Key not found - save failed.\n"));
                    return;
                }
                else {
                    PrintError(TEXT("Error saving TimeoutSetting key.\n"));
                    return;
                }
            }
            //Finished with keys
            RegCloseKey(key);
        } else {
            if (lResult == ERROR_FILE_NOT_FOUND) {
                PrintError(TEXT("User Key not found - creation failed.\n"));
                return;
            }
            else {
                PrintError(TEXT("Error creating user key.\n"));
                return;
            }
        }
    } else {

        // save window size
        if ((lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
            L"Software\\Island Christian Academy\\PlayMe", //lpctstr
            0,                      //reserved
            (LPWSTR)L"",                     //ptr to null-term string specifying the object type of this key
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,
            &key,
            &lpdw)) == ERROR_SUCCESS) {

            if (WinSizeIsSaved) {
                if ((lResult = RegSetValueEx(key, L"width", 0, REG_DWORD, (const BYTE*)&width, (DWORD)sizeof(width))) == ERROR_SUCCESS) {

                }
                else {
                    if (lResult == ERROR_FILE_NOT_FOUND) {
                        PrintError(TEXT("Window width Key not found - save failed.\n"));
                        return;
                    }
                    else {
                        PrintError(TEXT("Error saving window width key.\n"));
                        return;
                    }

                }

                if ((lResult = RegSetValueEx(key, L"heigth", 0, REG_DWORD, (const BYTE*)&heigth, (DWORD)sizeof(heigth))) == ERROR_SUCCESS) {

                }
                else {
                    if (lResult == ERROR_FILE_NOT_FOUND) {
                        PrintError(TEXT("Window heigth Key not found - save failed.\n"));
                        return;
                    }
                    else {
                        PrintError(TEXT("Error saving window heigth key.\n"));
                        return;
                    }
                }
                WinSizeIsSaved = FALSE;
            }

            if (WinLocIsSaved) {
                if ((lResult = RegSetValueEx(key, L"left", 0, REG_DWORD, (const BYTE*)&(WinLocSave.left), (DWORD)sizeof(WinLocSave.left))) == ERROR_SUCCESS) {

                }
                else {
                    if (lResult == ERROR_FILE_NOT_FOUND) {
                        PrintError(TEXT("Window left location Key not found - save failed.\n"));
                        return;
                    }
                    else {
                        PrintError(TEXT("Error saving window left location key.\n"));
                        return;
                    }

                }

                if ((lResult = RegSetValueEx(key, L"top", 0, REG_DWORD, (const BYTE*)&(WinLocSave.top), (DWORD)sizeof(WinLocSave.top))) == ERROR_SUCCESS) {

                }
                else {
                    if (lResult == ERROR_FILE_NOT_FOUND) {
                        PrintError(TEXT("Window top location Key Key not found - save failed.\n"));
                        return;
                    }
                    else {
                        PrintError(TEXT("Error saving window top location Key key.\n"));
                        return;
                    }

                }

                if ((lResult = RegSetValueEx(key, L"right", 0, REG_DWORD, (const BYTE*)&(WinLocSave.right), (DWORD)sizeof(WinLocSave.right))) == ERROR_SUCCESS) {

                }
                else {
                    if (lResult == ERROR_FILE_NOT_FOUND) {
                        PrintError(TEXT("Window right location Key Key not found - save failed.\n"));
                        return;
                    }
                    else {
                        PrintError(TEXT("Error saving window right location Key key.\n"));
                        return;
                    }

                }

                if ((lResult = RegSetValueEx(key, L"bottom", 0, REG_DWORD, (const BYTE*)&(WinLocSave.bottom), (DWORD)sizeof(WinLocSave.bottom))) == ERROR_SUCCESS) {

                }
                else {
                    if (lResult == ERROR_FILE_NOT_FOUND) {
                        PrintError(TEXT("Window bottom location Key Key not found - save failed.\n"));
                        return;
                    }
                    else {
                        PrintError(TEXT("Error saving window bottom location Key key.\n"));
                        return;
                    }

                }
                WinLocIsSaved = FALSE;
            }

            if (ValidWinSizeLoc) {
                if ((WinSizeLoc.showCmd != SW_SHOWMINIMIZED) && (WinSizeLoc.showCmd != SW_SHOWMINNOACTIVE)) {
                    if ((lResult = RegSetValueEx(key, L"WinSizeLoc", 0, REG_BINARY, (const BYTE*)&(WinSizeLoc), (DWORD)sizeof(WINDOWPLACEMENT))) == ERROR_SUCCESS) {

                    }
                    else {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("Window WinSizeLoc Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving WinSizeLoc key.\n"));
                            return;
                        }

                    }
                }
            }

            //Finished with size and location keys
            RegCloseKey(key);
        } else {
            if (lResult == ERROR_FILE_NOT_FOUND) {
                PrintError(TEXT("User Key not found - creation failed.\n"));
                return;
            }
            else {
                PrintError(TEXT("Error creating user key.\n"));
                return;
            }
        }

        if (FontChanged) {
            // save font data
            if ((lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
                L"Software\\Island Christian Academy\\PlayMe\\font", //lpctstr
                0,                      //reserved
                (LPWSTR)L"",                     //ptr to null-term string specifying the object type of this key
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                NULL,
                &key,
                &lpdw)) == ERROR_SUCCESS) {
                /*
                typedef struct tagLOGFONTW
                {
                    LONG      lfHeight;
                    LONG      lfWidth;
                    LONG      lfEscapement;
                    LONG      lfOrientation;
                    LONG      lfWeight;
                    BYTE      lfItalic;
                    BYTE      lfUnderline;
                    BYTE      lfStrikeOut;
                    BYTE      lfCharSet;
                    BYTE      lfOutPrecision;
                    BYTE      lfClipPrecision;
                    BYTE      lfQuality;
                    BYTE      lfPitchAndFamily;
                    WCHAR     lfFaceName[LF_FACESIZE];
                } LOGFONTW
                */
                // fetch lfFaceName
                if ((lResult = RegSetValueEx(key, L"lfFaceName", 0, REG_SZ, (const BYTE*)&lfold.lfFaceName, (DWORD)sizeof(lfold.lfFaceName))) == ERROR_SUCCESS) {
#ifdef DEBUG
                    PrintError(TEXT("lfFaceName value saved.\n"));
#endif
                    // fetch lfHeight
                    if ((lResult = RegSetValueEx(key, L"lfHeight", 0, REG_DWORD, (const BYTE*)&lfold.lfHeight, (DWORD)sizeof(lfold.lfHeight))) == ERROR_SUCCESS) {
#ifdef DEBUG
                        PrintError(TEXT("lfHeight value saved.\n"));
#endif
                    }
                    else {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfHeight Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfHeight key.\n"));
                            return;
                        }
                    }
                    // fetch lfWidth
                    if ((lResult = RegSetValueEx(key, L"lfWidth", 0, REG_DWORD, (const BYTE*)&lfold.lfWidth, (DWORD)sizeof(lfold.lfWidth))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfWidth Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfWidth key.\n"));
                            return;
                        }
                    }
                    // fetch lfEscapement
                    if ((lResult = RegSetValueEx(key, L"lfEscapement", 0, REG_DWORD, (const BYTE*)&lfold.lfEscapement, (DWORD)sizeof(lfold.lfEscapement))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfEscapement Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfEscapement key.\n"));
                            return;
                        }
                    }
                    // fetch lfOrientation
                    if ((lResult = RegSetValueEx(key, L"lfOrientation", 0, REG_DWORD, (const BYTE*)&lfold.lfOrientation, (DWORD)sizeof(lfold.lfOrientation))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfOrientation Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfOrientation key.\n"));
                            return;
                        }
                    }
                    // fetch lfWeight
                    if ((lResult = RegSetValueEx(key, L"lfWeight", 0, REG_DWORD, (const BYTE*)&lfold.lfWeight, (DWORD)sizeof(lfold.lfWeight))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfWeight Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfWeight key.\n"));
                            return;
                        }
                    }
                    // fetch lfItalic
                    tempdword = (DWORD)lfold.lfItalic;
                    if ((lResult = RegSetValueEx(key, L"lfItalic", 0, REG_DWORD, (const BYTE*)&tempdword, (DWORD)sizeof(tempdword))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfItalic Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfItalic key.\n"));
                            return;
                        }
                    }
                    // fetch lfUnderline
                    tempdword = (DWORD)lfold.lfUnderline;
                    if ((lResult = RegSetValueEx(key, L"lfUnderline", 0, REG_DWORD, (const BYTE*)&tempdword, (DWORD)sizeof(tempdword))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfUnderline Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfUnderline key.\n"));
                            return;
                        }
                    }
                    // fetch lfStrikeOut
                    tempdword = (DWORD)lfold.lfStrikeOut;
                    if ((lResult = RegSetValueEx(key, L"lfStrikeOut", 0, REG_DWORD, (const BYTE*)&tempdword, (DWORD)sizeof(tempdword))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfStrikeOut Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfStrikeOut key.\n"));
                            return;
                        }
                    }
                    // fetch lfCharSet
                    tempdword = (DWORD)lfold.lfCharSet;
                    if ((lResult = RegSetValueEx(key, L"lfCharSet", 0, REG_DWORD, (const BYTE*)&tempdword, (DWORD)sizeof(tempdword))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfCharSet Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfCharSet key.\n"));
                            return;
                        }
                    }
                    // fetch lfOutPrecision
                    tempdword = (DWORD)lfold.lfOutPrecision;
                    if ((lResult = RegSetValueEx(key, L"lfOutPrecision", 0, REG_DWORD, (const BYTE*)&tempdword, (DWORD)sizeof(tempdword))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfOutPrecision Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfOutPrecision key.\n"));
                            return;
                        }
                    }
                    // fetch lfClipPrecision
                    tempdword = (DWORD)lfold.lfClipPrecision;
                    if ((lResult = RegSetValueEx(key, L"lfClipPrecision", 0, REG_DWORD, (const BYTE*)&tempdword, (DWORD)sizeof(tempdword))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfClipPrecision Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfClipPrecision key.\n"));
                            return;
                        }
                    }
                    // fetch lfQuality
                    tempdword = (DWORD)lfold.lfQuality;
                    if ((lResult = RegSetValueEx(key, L"lfQuality", 0, REG_DWORD, (const BYTE*)&tempdword, (DWORD)sizeof(tempdword))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfQuality Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfQuality key.\n"));
                            return;
                        }
                    }
                    // fetch lfPitchAndFamily
                    tempdword = (DWORD)lfold.lfPitchAndFamily;
                    if ((lResult = RegSetValueEx(key, L"lfPitchAndFamily", 0, REG_DWORD, (const BYTE*)&tempdword, (DWORD)sizeof(tempdword))) != ERROR_SUCCESS) {
                        if (lResult == ERROR_FILE_NOT_FOUND) {
                            PrintError(TEXT("lfPitchAndFamily Key not found - save failed.\n"));
                            return;
                        }
                        else {
                            PrintError(TEXT("Error saving lfPitchAndFamily key.\n"));
                            return;
                        }
                    }
                }
                else {
                    if (lResult == ERROR_FILE_NOT_FOUND) {
                        PrintError(TEXT("lfFaceName Key not found - save failed.\n"));
                        return;
                    }
                    else {
                        PrintError(TEXT("Error saving lfFaceName value.\n"));
                        return;
                    }

                }
                //Finished with keys
                RegCloseKey(key);
            }
            else {
                if (lResult == ERROR_FILE_NOT_FOUND) {
                    PrintError(TEXT("font Key not found - creation failed.\n"));
                    return;
                }
                else {
                    PrintError(TEXT("Error creating font key.\n"));
                    return;
                }
            }
            FontChanged = FALSE;
        }
    }
}

bool LoadDirectoryContents(const wchar_t* sDir, int AudioFiles)
{
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;

    wchar_t sPath[2048];

    if (breaknow) return false;
#ifdef DEBUG
    fprintf(fdb, "in LoadDirectoryContents sDir=\"%S\", AudioFiles=%i\n", sDir, AudioFiles);
#endif

    //Specify a file mask. *.* = We want everything!
    wsprintf(sPath, L"%s\\*.*", sDir);

    if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    int bookIndex = -1;
    int chapterIndex = -1;
    if (AudioFiles) {
        Booknames = (char**)calloc(67,sizeof(char*));
        ChapterData = (ChapterListStructure**)calloc(TOTNUMCHAPSP1,sizeof(ChapterListStructure*));
        AudioFolderData = (AudioFoldersStructure**)calloc(67,sizeof(AudioFoldersStructure*));
        PreviousChapterPointer = NULL;
    }
    else {
        TextFolderData = (TextFoldersStructure**)calloc(67,sizeof(TextFoldersStructure*));
    }

    do
    {
        //Find first file will always return "."
        //    and ".." as the first two directories.
        if (wcscmp(fdFile.cFileName, L".") != 0
            && wcscmp(fdFile.cFileName, L"..") != 0)
        {
#ifdef DEBUG
            fprintf(fdb, "in LoadDirectoryContents, found \"%S\"\n", fdFile.cFileName);
#endif
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);
            //Is the entity a File or Folder?
            if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                bookIndex++;
                char* scratchstring;
                char* bookname;
                int booknumber;
                scratchstring = (char*)calloc((unsigned int)(wcslen(sPath) + 30), (unsigned int)sizeof(*scratchstring));
                wcstombs(scratchstring, sPath, wcslen(sPath) + 1);
                if (parseadname(scratchstring, &bookname, &booknumber)) {
                    wchar_t sTemp[2048];
                    wsprintf(sTemp, L"parse of ind=%i,\"%S\"\\\"%S\" directory name failed", bookIndex, sDir, fdFile.cFileName);
                    PrintError(sTemp);
#ifdef DEBUG
                    fprintf(fdb,"parse of \"%S\" failed, AudioFiles=%i\n", sDir, AudioFiles);
#endif
                    return(false);
                }
                AudioFoldersStructure* tempAF =NULL;
                TextFoldersStructure* tempTF = NULL;
                if (AudioFiles == AUDIOFILES) {
                    if (*(Booknames + booknumber) != NULL) {
                        PrintError(TEXT("duplicate book name found in audio directory"));
                        return(false);
                    }
                    *(Booknames + booknumber) = (char*)calloc(strlen(bookname)+1, sizeof(char *));
                    strcpy(*(Booknames + booknumber), bookname);
                    *(AudioFolderData + booknumber) = (AudioFoldersStructure*)calloc( 1, sizeof(AudioFoldersStructure));
                    tempAF = *(AudioFolderData + booknumber);
                    tempAF->BookNumber = booknumber;
                    tempAF->NumberOfChapters = -1;
                    tempAF->Bookname = *(Booknames + booknumber);
                    tempAF->BookDirectoryPath = (char*)malloc(strlen(scratchstring)+1);
                    strcpy(tempAF->BookDirectoryPath, scratchstring);
                    tempAF->FirstChapterDataPointer = NULL;
                } else {
                    if (*(Booknames + booknumber) == NULL) {
                        if (booknumber == 38) continue;
                        PrintError(TEXT("empty book name found while processing text directory"));
                        return(false);
                    }
                    *(TextFolderData + booknumber) = (TextFoldersStructure*)calloc(1, sizeof(TextFoldersStructure));
                    tempTF = *(TextFolderData + booknumber);
                    tempTF->BookNumber = booknumber;
                    tempTF->NumberOfChapters = -1;
                    tempTF->Bookname = *(Booknames + booknumber);
                    tempTF->BookDirectoryPath = (char*)malloc(strlen(scratchstring) + 1);
                    strcpy(tempTF->BookDirectoryPath, scratchstring);
                    tempTF->FirstChapterDataPointer = NULL;

                }

                free(scratchstring);
                int startchapter = chapterIndex;
                LoadSubDirectoryContents(sPath, &chapterIndex, booknumber, AudioFiles);
                if (AudioFiles == AUDIOFILES) {
                    tempAF->NumberOfChapters = chapterIndex - startchapter;
                    tempAF->FirstChapterDataPointer = ChapterData[startchapter+1];
                }
                else {
                    tempTF->NumberOfChapters = chapterIndex - startchapter;
                    tempTF->FirstChapterDataPointer = ChapterData[startchapter + 1];
                }
            }
            else {
                // sould be no files in this folder - skip it
                continue;
            }

        }
    } while (FindNextFile(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    if (AudioFiles) {
        ChapterListStructure* NextChapterPointer;
        NextChapterPointer = PreviousChapterPointer;
        PreviousChapterPointer = PreviousChapterPointer->PreviousChapter;
        while (PreviousChapterPointer) {
            PreviousChapterPointer->NextChapter = NextChapterPointer;
            NextChapterPointer = PreviousChapterPointer;
            PreviousChapterPointer = PreviousChapterPointer->PreviousChapter;
        }
    }

    return true;
}

int parseadname(char* scratchstring, char** bookname, int* booknumber) {
    char* pch;
    char* pch2;
    pch = strrchr(scratchstring, '\\');
    if (pch == NULL) return(1);
    pch2 = strchr(pch+1, ' ');
    if (pch2 == NULL) return(2);
    if (pch == pch2) return(3); // should be imposible
    if (((pch2 - pch - 1) > 7) || ((pch2 - pch - 1) < 0) ) return(4);
    int ret = sscanf(pch+1, "%d", booknumber);
    if ( ret != 1) return(5);
    *bookname = strchr(pch + 1, ' ');
    if (*bookname == NULL) return(6);
    (*bookname)++;
    return(0);
}

bool LoadSubDirectoryContents(const wchar_t* sDir, int* chapterindex, int booknumber, int AudioFiles) {
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;

    wchar_t sPath[2048];
    int numChaptersInBook = 0;

    if (breaknow) return false;
#ifdef DEBUG
    fprintf(fdb, "in LoadSubDirectoryContents, sDir=\"%S\", AudioFiles=%i\n", sDir,AudioFiles);
#endif

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
#ifdef DEBUG
            fprintf(fdb, "in LoadSubDirectoryContents, found \"%S\"\n", fdFile.cFileName);
#endif
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            wsprintf(sPath, L"%s\\%s", sDir, fdFile.cFileName);
            //Is the entity a File or Folder?
            if ( !(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
            {
                numChaptersInBook++;
                (*chapterindex)++;
                char* scratchstring;
                char* bookname;
                bookname = *(Booknames + booknumber);
                scratchstring = (char*)calloc((unsigned int)(wcslen(sPath) + 30), (unsigned int)sizeof(*scratchstring));
                wcstombs(scratchstring, sPath, wcslen(sPath) + 1);
                if (AudioFiles) {
                    *(ChapterData + *chapterindex) = (ChapterListStructure*)calloc(1, sizeof(ChapterListStructure));
                    ChapterListStructure* tempCS;
                    tempCS = *(ChapterData + *chapterindex);
                    tempCS->GlobalChapterIndex = *chapterindex;
                    tempCS->ChapterNumber = numChaptersInBook;
                    tempCS->BookNumber = booknumber;
                    tempCS->Bookname = bookname;
                    tempCS->AudioFilePath = (char*)malloc(strlen(scratchstring) + 1);
                    strcpy(tempCS->AudioFilePath, scratchstring);
                    tempCS->TextFilePath = NULL;
                    tempCS->NextChapter = NULL;
                    tempCS->PreviousChapter = PreviousChapterPointer;
                    PreviousChapterPointer = tempCS;
                }
                else {
                    ChapterListStructure* tempCS;
                    tempCS = *(ChapterData + *chapterindex);
                    tempCS->TextFilePath = (char*)malloc(strlen(scratchstring) + 1);
                    strcpy(tempCS->TextFilePath, scratchstring);
                }

                free(scratchstring);
            }
            else {
                // sould be no directories in this folder - skip it
                continue;
            }
        }
    } while (FindNextFile(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    return true;
}

bool FindDirectoryContents() {
    // find audio path
    ChapterListStructure* tempCS;
    tempCS = *(ChapterData + jackiesfile - 1);
    int num_chars = MultiByteToWideChar(CP_UTF8, 0, tempCS->AudioFilePath, strlen(tempCS->AudioFilePath), NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, tempCS->AudioFilePath, strlen(tempCS->AudioFilePath), foundPath, num_chars);
    foundPath[num_chars] = L'\0';

    // find text path
    foundTextPath = tempCS->TextFilePath;

    return(true);
}

void cleanupheap() {
    int i;
    ChapterListStructure* tempCS;
    AudioFoldersStructure* tempAF;
    TextFoldersStructure* tempTF;
    if (ChapterData != NULL) {
        for (i = 0; i < TOTNUMCHAPSP1; i++) {
            if (*(ChapterData + i) != NULL) {
                tempCS = *(ChapterData + i);
                if (tempCS->AudioFilePath != NULL)free(tempCS->AudioFilePath);
                if (tempCS->TextFilePath != NULL)free(tempCS->TextFilePath);
                free(tempCS);
            }
        }
        free(ChapterData);
    }

    if (AudioFolderData != NULL) {
        for (i = 0; i < 67; i++) {
            if (*(AudioFolderData + i) != NULL) {
                tempAF = *(AudioFolderData + i);
                if (tempAF->BookDirectoryPath != NULL)free(tempAF->BookDirectoryPath);
                free(tempAF);
            }
        }
        free(AudioFolderData);
    }

    if (TextFolderData != NULL) {
        for (i = 0; i < 67; i++) {
            if (*(TextFolderData + i) != NULL) {
                tempTF = *(TextFolderData + i);
                if (tempTF->BookDirectoryPath != NULL)free(tempTF->BookDirectoryPath);
                free(tempTF);
            }
        }
        free(TextFolderData);
    }

    if (Booknames != NULL) {
        for (i = 0; i < 67; i++) {
            if (*(Booknames + i) != NULL) {
                free(*(Booknames + i));
            }
        }
        free(Booknames);
    }

    return;
}

// Message handler for Settings Dialog box.
INT_PTR CALLBACK SettingsFM(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HICON hIcon;
    HWND hWndComboBoook;
    HWND hWndFootnoteRadio;
    HWND hWndPlaytimeSpin;
    int err;
    wchar_t  A[256];
    int  k;
    AudioFoldersStructure* TempAudio;
    ChapterListStructure* TempChap;
    static int BookIndex;

    switch (message)
    {

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDOK:
        {
            int booknumber;
            int StartChapter;
            int chapterIndex;
            // initialize the book combo
            hWndComboBoook = GetDlgItem(hDlg, IDC_BOOK);
            booknumber = SendMessage(hWndComboBoook, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0) + 1;
            TempAudio = *(AudioFolderData + booknumber);
            TempChap = TempAudio->FirstChapterDataPointer;
            StartChapter = TempChap->GlobalChapterIndex;
            HWND hWndComboChapter = GetDlgItem(hDlg, IDC_CHAPTER);
            chapterIndex = SendMessage(hWndComboChapter, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0)+1 + StartChapter;
            jackiesfile = chapterIndex;

            // get the state of the footnote radio buttons
            hWndFootnoteRadio = GetDlgItem(hDlg, IDC_FOOTNOTEYES);
            if (Button_GetCheck(hWndFootnoteRadio) == BST_CHECKED) {
                IncludeFootnotes = TRUE;
            }
            else {
                IncludeFootnotes = FALSE;
            }

            // get the value of the playtime edit control
            TimeoutSetting = (int)GetDlgItemInt(hDlg, IDC_PLAYTIME, NULL, FALSE);

            // update what's saved in the registry
            WriteConfig(jackiesfile);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
            break;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;

        case IDC_BOOK:
            {
                if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                    BookIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0) + 1;
                    if (BookIndex > 66) {
                        PrintError(TEXT("Book Combo Box index out of range"));
                        return TRUE;
                    }
                    if (BookIndex == 0)
                    {
                        PrintError(TEXT("Select a book name from the list"));
                        return TRUE;
                    }
                    if (BookIndex == 38)
                    {
                        PrintError(TEXT("Sorry, this book is missing"));
                        BookIndex = 1;
                        return TRUE;
                    }
                    HWND hWndComboChapter = GetDlgItem(hDlg, IDC_CHAPTER);
                    err = SendMessage(hWndComboChapter, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
                    //				if ( err != CB_OKAY )  MsgBox ("Impossible ComboBox error in MainFrm.");
                    TempAudio = *(AudioFolderData + BookIndex);
                    for (k = 1; k <= TempAudio->NumberOfChapters; k++)
                    {
                        wsprintf(A, L"%i", k);
                        // Add string to combobox.
                        SendMessage(hWndComboChapter, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
                    }

                    // Send the CB_SETCURSEL message to display an initial item 
                    //  in the selection field  
                    SendMessage(hWndComboChapter, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

                }
                return TRUE;
            }		// end of if on LOWORD(wParam) == IDC_COMBO1

        default:
            return FALSE;
        }
    }
    break;

    case WM_INITDIALOG:
    {
        int booknumber;
        int presentChapter;
        int chapterIndex;
        // initialize the book combo
        hWndComboBoook = GetDlgItem(hDlg, IDC_BOOK);
        err = SendMessage(hWndComboBoook, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
        //				if ( err != CB_OKAY )  MsgBox ("Impossible ComboBox error in MainFrm.");
        for (k = 1; k < 67; k += 1)
        {
            if (Booknames[k] != NULL) {
                mbstowcs(A, (char*)Booknames[k], strlen(Booknames[k]) + 1);
            }
            else {
                wcscpy(A, L"--missing--");
            }
            // Add string to combobox.
            SendMessage(hWndComboBoook, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
        }
        // Send the CB_SETCURSEL message to display the initial book in the selection field
        chapterIndex = (jackiesfile - 1) % TOTNUMCHAPS;
        booknumber = (*(ChapterData + chapterIndex))->BookNumber;
        presentChapter = (*(ChapterData + chapterIndex))->ChapterNumber;
        SendMessage(hWndComboBoook, CB_SETCURSEL, (WPARAM)(booknumber-1), (LPARAM)0);

        HWND hWndComboChapter = GetDlgItem(hDlg, IDC_CHAPTER);
        err = SendMessage(hWndComboChapter, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
        //				if ( err != CB_OKAY )  MsgBox ("Impossible ComboBox error in MainFrm.");

        // initialize the chapter combo
        TempAudio = *(AudioFolderData + booknumber);
        for (k = 1; k <= TempAudio->NumberOfChapters; k ++)
        {
            wsprintf(A, L"%i", k);
            // Add string to combobox.
            SendMessage(hWndComboChapter, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
        }

        // Send the CB_SETCURSEL message to display an initial item 
        //  in the selection field  
        SendMessage(hWndComboChapter, CB_SETCURSEL, (WPARAM)(presentChapter-1), (LPARAM)0);

        // set the default state of the footnote radio buttons   
        if (IncludeFootnotes) {
            hWndFootnoteRadio = GetDlgItem(hDlg, IDC_FOOTNOTEYES);
        }
        else {
            hWndFootnoteRadio = GetDlgItem(hDlg, IDC_FOOTNOTENO);
        }
        Button_SetCheck(hWndFootnoteRadio, BST_CHECKED);

        // Set the range of playtime spin control.
        hWndPlaytimeSpin = GetDlgItem(hDlg, IDC_PLAYTIMESPIN);
        SendMessage(hWndPlaytimeSpin, UDM_SETRANGE, 0L, MAKELONG((short)3000, (short)0));
        // Set the initial value of maximum number of backups spin control
        SendMessage(hWndPlaytimeSpin, UDM_SETPOS, 0L, (TimeoutSetting));


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
