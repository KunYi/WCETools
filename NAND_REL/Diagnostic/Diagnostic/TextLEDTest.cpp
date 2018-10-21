#include <windows.h>
#include "resource.h"
#include "api.h"

extern HANDLE g_hFile;

//----------------------------------------------------------------------
// Text_LED_Test
//
BOOL Text_LED_Test(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    PFNSetTextLEDStatus pfnSetTextLEDStatus;
    CHAR szText[32];
    DWORD dwBytesWriten;

    HMODULE hApiDll = LoadLibrary(TEXT("API.DLL"));
    if (hApiDll)
    {
        pfnSetTextLEDStatus = (PFNSetTextLEDStatus)GetProcAddress(hApiDll, TEXT("SetTextLEDStatus"));
    }

    pfnSetTextLEDStatus(ON);
    if (MessageBox(hWnd, TEXT("Is the Text LED on?"), TEXT("Text LED Test"), MB_YESNO|MB_ICONQUESTION) == IDYES)
    {
        SetDlgItemText(hWnd, IDC_EDIT7, TEXT("PASS"));
        sprintf(szText, "Text LED Test: PASS\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }
    else
    {
        SetDlgItemText(hWnd, IDC_EDIT7, TEXT("FAIL"));
        sprintf(szText, "Text LED Test: FAIL\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }

    pfnSetTextLEDStatus(OFF);
    return 0;
}