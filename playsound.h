#pragma once

#include "resource.h"



#ifndef MAINDEFS
#define EXTERN		extern
#define INITIZERO
#define INITSZERO
#define INITBOOLFALSE
#define INITBOOLTRUE
#define INITNULL
#define INITJKNULL
#define INITBUFFERSIZE
#define INITDIRSIZE
#define INITVERNAMELEN
#define INITNULLSTRING
#else  // MAINDEFS
#define EXTERN
#define INITIZERO	=0
#define INITSZERO	={0}
#define INITBOOLFALSE	=false
#define INITBOOLTRUE	=true
#define INITNULL	=NULL
#define INITJKNULL  =JKNULL
#define INITBUFFERSIZE	=BUFFERSIZE
#define INITDIRSIZE		=DIRSIZE
#define INITVERNAMELEN  =VERNAMELEN
#define INITNULLSTRING  =""
#endif  // MAINDEFS


#define AUDIOFILES  1
#define TEXTFILES    0
#define TOTNUMCHAPS	1175
#define TOTNUMCHAPSP1	TOTNUMCHAPS+1
#define ID_EDITCHILD 1001

typedef struct ChapterList
{
	int						GlobalChapterIndex;
	int						ChapterNumber;
	char*					Bookname;
	char*					AudioFilePath;
	char*					TextFilePath;
	struct ChapterList*		NextChapter;
	struct ChapterList*		PreviousChapter;
} ChapterListStructure;


typedef struct AudioFolders
{
	int							BookNumber;
	int							NumberOfChapters;
	char*						Bookname;
	char*						BookDirectoryPath;
	ChapterListStructure*		FirstChapterDataPointer;
} AudioFoldersStructure;

typedef struct TextFolders
{
	int							BookNumber;
	int							NumberOfChapters;
	char*						Bookname;
	char*						BookDirectoryPath;
	ChapterListStructure*		FirstChapterDataPointer;
} TextFoldersStructure;

EXTERN AudioFoldersStructure** AudioFolderData;
EXTERN TextFoldersStructure** TextFolderData;
EXTERN ChapterListStructure** ChapterData;
EXTERN char** Booknames;
EXTERN HWND hTextWnd INITNULL;
EXTERN HWND hwndEdit INITNULL;
EXTERN RECT WinLocSave;
EXTERN BOOL WinLocIsSaved INITBOOLFALSE;
EXTERN DWORD width;
EXTERN DWORD heigth;
EXTERN BOOL WinSizeIsSaved INITBOOLFALSE;
EXTERN LOGFONT lfold;
EXTERN int lfset INITIZERO;
EXTERN BOOL FontChanged INITBOOLFALSE;
EXTERN WINDOWPLACEMENT WinSizeLoc;
EXTERN BOOL ValidWinSizeLoc INITBOOLFALSE;

#ifdef DEBUG
EXTERN FILE *fdb INITNULL;
#endif


