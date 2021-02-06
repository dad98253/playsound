//
// GetProductAndVersion.cpp : gets the name and version information for the application.
//


#include <windows.h>
#include <stdio.h>

extern void PrintError(LPCTSTR errDesc);

#define MIN(x, y) (((x) > (y)) ? (y) : (x))


bool GetProductAndVersion(char* strProductName, int iProductNameLenIn, char* strProductVersion, int iProductVersionLenIn, char* strProductCopyright, int iProductCopyrightnLenIn)
{
    wchar_t sTemp[2048];
    // get the filename of the executable containing the version resource
    TCHAR szFilename[MAX_PATH + 1] = { 0 };
    if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
    {
        wsprintf(sTemp, L"GetModuleFileName failed in GetProductAndVersion with error % lu\n", GetLastError());
        PrintError(sTemp);
        return false;
    }

    // allocate a block of memory for the version info
    DWORD dummy;
    BYTE* data = NULL;
    DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
    if (dwSize > 0)
    {
        data = (unsigned char*)realloc(data, dwSize);
        if (data == NULL) {
            wsprintf(sTemp, L"realloc failed with in GetProductAndVersion\n");
            PrintError(sTemp);
            return false;
        }

        // load the version info
        if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0]))
        {
            wsprintf(sTemp, L"GetFileVersionInfo failed in GetProductAndVersion with error %lu\n", GetLastError());
            PrintError(sTemp);
            return false;
        }
    }


    // get the name and version strings
    LPVOID pvProductName = NULL;
    unsigned int iProductNameLen = 0;
    LPVOID pvProductVersion = NULL;
    unsigned int iProductVersionLen = 0;
    LPVOID pvProductCopyright = NULL;
    unsigned int iProductCopyrightnLen = 0;

    // replace "040904e4" with the language ID of your resources
    // note : 040904e4 coresponds to "multilingial" in your resource editor. Your default might be set to "unicode".. check it!
    //if (!VerQueryValue(&data[0], L"\\StringFileInfo\\040904e4\\ProductName", &pvProductName, &iProductNameLen))
    if (!VerQueryValue(&data[0], L"\\StringFileInfo\\040904b0\\ProductName", &pvProductName, &iProductNameLen))
        {
        wsprintf(sTemp, L"Can't obtain ProductName from resources in GetProductAndVersion\n");
        PrintError(sTemp);
        free(data);
        return false;
    }
    if (!VerQueryValue(&data[0], L"\\StringFileInfo\\040904b0\\ProductVersion", &pvProductVersion, &iProductVersionLen))
    {
        wsprintf(sTemp, L"Can't obtain ProductVersion from resources in GetProductAndVersion\n");
        PrintError(sTemp);
        free(data);
        return false;
    }
    if (!VerQueryValue(&data[0], L"\\StringFileInfo\\040904b0\\LegalCopyright", &pvProductCopyright, &iProductCopyrightnLen))
    {
        wsprintf(sTemp, L"Can't obtain ProductCopyright from resources in GetProductAndVersion\n");
        PrintError(sTemp);
        free(data);
        return false;
    }

    wcstombs(strProductName, (const wchar_t*)pvProductName, wcslen((const wchar_t*)pvProductName) + 1);
    wcstombs(strProductVersion, (const wchar_t*)pvProductVersion, wcslen((const wchar_t*)pvProductVersion) + 1);
    wcstombs(strProductCopyright, (const wchar_t*)pvProductCopyright, wcslen((const wchar_t*)pvProductCopyright) + 1);

    if (data != NULL) free(data);


    return true;
}
