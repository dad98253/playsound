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
#define MAINDEFS
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
bool LoadDirectoryContents(const wchar_t* sDir, int AudioOrText);
bool LoadSubDirectoryContents(const wchar_t* sDir, int * chapterindex, int booknumber, int AudioFiles);
int parseadname(char * scratchstring, char ** bookname, int * booknumber);
bool FindDirectoryContents();
void cleanupheap();

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
                        TimmerID = SetTimer(hDlg, nID120SecondEvent, 1200 * 1000, NULL);
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
    WriteConfig(jackiesfile % TOTNUMCHAPSP1);

    wcscpy(wfile, dbloc);
    wcscpy(foundPath, L"");
    breaknow = 0;
    filecount = 0;
//   if (!ListDirectoryContents(wfile)) {
    if (!FindDirectoryContents()) {
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
    free(tempstr);

    return (int)opReturn;

//    return 0; // dead code??
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

bool LoadDirectoryContents(const wchar_t* sDir, int AudioFiles)
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
    int bookIndex = -1;
    int chapterIndex = -1;
    if (AudioFiles) {
        Booknames = (char**)calloc(67,sizeof(char*));
        ChapterData = (ChapterListStructure**)calloc(TOTNUMCHAPSP1,sizeof(ChapterListStructure*));
        AudioFolderData = (AudioFoldersStructure**)calloc(67,sizeof(AudioFoldersStructure*));
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
                    PrintError(TEXT("parse of directory name failed"));
                    return(false);
                }
                if (AudioFiles) {
                    if (*(Booknames + booknumber) != NULL) {
                        PrintError(TEXT("duplicate book name found in audio directory"));
                        return(false);
                    }
                    *(Booknames + booknumber) = (char*)calloc(strlen(bookname)+1, sizeof(char *));
                    strcpy(*(Booknames + booknumber), bookname);
                    *(AudioFolderData + booknumber) = (AudioFoldersStructure*)calloc( 1, sizeof(AudioFoldersStructure));
                    AudioFoldersStructure *tempAF;
                    tempAF = *(AudioFolderData + booknumber);
                    tempAF->BookNumber = booknumber;
                    tempAF->NumberOfChapters = -1;
                    tempAF->Bookname = *(Booknames + booknumber);
                    tempAF->BookDirectoryPath = (char*)malloc(strlen(scratchstring)+1);
                    strcpy(tempAF->BookDirectoryPath, scratchstring);
                    tempAF->FirstChapterDataPointer = NULL;
                } else {
                    if (*(Booknames + booknumber) != NULL) {
                        PrintError(TEXT("empty book name found while processing text directory"));
                        return(false);
                    }
                    *(TextFolderData + booknumber) = (TextFoldersStructure*)calloc(1, sizeof(TextFoldersStructure));
                    TextFoldersStructure* tempTF;
                    tempTF = *(TextFolderData + booknumber);
                    tempTF->BookNumber = booknumber;
                    tempTF->NumberOfChapters = -1;
                    tempTF->Bookname = *(Booknames + booknumber);
                    tempTF->BookDirectoryPath = (char*)malloc(strlen(scratchstring) + 1);
                    strcpy(tempTF->BookDirectoryPath, scratchstring);
                    tempTF->FirstChapterDataPointer = NULL;

                }

                free(scratchstring);
                LoadSubDirectoryContents(sPath, &chapterIndex, booknumber, AudioFiles);
            }
            else {
                // sould be no files in this folder - skip it
                continue;
                /*               filecount++;
                               if (filecount == jackiesfile) {
                                   breaknow = 1;
                                   wcscpy(foundPath, sPath);
                                   //                    FindClose(hFind); //Always, Always, clean things up!
                                   //                    return false;
                                   break;
                               } */
            }
            //           if (breaknow)break;
        }
    } while (FindNextFile(hFind, &fdFile)); //Find the next file.

//    if (!breaknow)FindClose(hFind); //Always, Always, clean things up!
    FindClose(hFind); //Always, Always, clean things up!

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
    //Specify a file mask. *.* = We want everything!
    wsprintf(sPath, L"%s\\*.*", sDir);

    if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        return false;
    }
/*
    if (AudioFiles) {
        Booknames = (char**)calloc(67, sizeof(char*));
        ChapterData = (ChapterListStructure**)calloc(3000, sizeof(ChapterListStructure*));
        AudioFolderData = (AudioFoldersStructure**)calloc(67, sizeof(AudioFoldersStructure*));
    }
    else {
        TextFolderData = (TextFoldersStructure**)calloc(67, sizeof(TextFoldersStructure*));
    }
*/
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
                    tempCS->Bookname = bookname;
                    tempCS->AudioFilePath = (char*)malloc(strlen(scratchstring) + 1);
                    strcpy(tempCS->AudioFilePath, scratchstring);
                    tempCS->TextFilePath = NULL;
                    tempCS->NextChapter = NULL;
                    tempCS->PreviousChapter = NULL;
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
                /*               filecount++;
                               if (filecount == jackiesfile) {
                                   breaknow = 1;
                                   wcscpy(foundPath, sPath);
                                   //                    FindClose(hFind); //Always, Always, clean things up!
                                   //                    return false;
                                   break;
                               } */
            }
            //           if (breaknow)break;
        }
    } while (FindNextFile(hFind, &fdFile)); //Find the next file.

//    if (!breaknow)FindClose(hFind); //Always, Always, clean things up!
    FindClose(hFind); //Always, Always, clean things up!

    return true;
}

bool FindDirectoryContents() {
    ChapterListStructure* tempCS;
    tempCS = *(ChapterData + jackiesfile - 1);
    int num_chars = MultiByteToWideChar(CP_UTF8, 0, tempCS->AudioFilePath, strlen(tempCS->AudioFilePath), NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, tempCS->AudioFilePath, strlen(tempCS->AudioFilePath), foundPath, num_chars);
    foundPath[num_chars] = L'\0';

    //    wcscpy(foundPath, sPath);
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