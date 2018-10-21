#include <windows.h>
#include "resource.h"
#include "api.h"

extern HANDLE g_hFile;

//----------------------------------------------------------------------
// Buzzer_Test
//
BOOL Buzzer_Test(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    PFNBuzzerBeep pfnBuzzerBeep = NULL;
    CHAR szText[32];
    DWORD dwBytesWriten;

    HMODULE hApiDll = LoadLibrary(TEXT("API.DLL"));
    if (hApiDll)
    {
        pfnBuzzerBeep = (PFNBuzzerBeep)GetProcAddress(hApiDll, TEXT("BuzzerBeep"));
    }

    pfnBuzzerBeep(1000);
    if (MessageBox(hWnd, TEXT("Do you hear any sound on buzzer?"), TEXT("Buzzer Test"), MB_YESNO|MB_ICONQUESTION) == IDYES)
    {
        SetDlgItemText(hWnd, IDC_EDIT8, TEXT("PASS"));
        sprintf(szText, "Buzzer Test: PASS\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }
    else
    {
        SetDlgItemText(hWnd, IDC_EDIT8, TEXT("FAIL"));
        sprintf(szText, "Buzzer Test: FAIL\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }
    return 0;
}