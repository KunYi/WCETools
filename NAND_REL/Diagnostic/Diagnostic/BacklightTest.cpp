#include <windows.h>
#include "resource.h"
#include "api.h"

//----------------------------------------------------------------------
// Global data
//
PFNSetBacklightValue pfnSetBacklightValue = NULL;
PFNGetBacklightValue pfnGetBacklightValue = NULL;
DWORD g_dwOldValue, g_dwNewValue;
extern HWND g_hWnd;
extern HINSTANCE g_hInst;
extern HANDLE g_hFile;

//======================================================================
// Backlight Test Dialog procedure
//
BOOL CALLBACK BacklightTestDlgProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg)
    {
        case WM_INITDIALOG:
        {
            SetWindowPos (hWnd, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0);
            g_dwOldValue = pfnGetBacklightValue();
            pfnSetBacklightValue(g_dwNewValue);
            break;
        }

        case WM_LBUTTONDOWN:
        {
            g_dwNewValue -= 20;
            if (g_dwNewValue != 0)
                pfnSetBacklightValue(g_dwNewValue);
            else
                EndDialog(hWnd, 0);
            break;
        }
    }
    return FALSE;
}
//----------------------------------------------------------------------
// Backlight_Test
//
BOOL Backlight_Test(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    CHAR szText[32];
    DWORD dwBytesWriten;

    HMODULE hApiDll = LoadLibrary(TEXT("API.DLL"));
    if (hApiDll)
    {
        pfnSetBacklightValue = (PFNSetBacklightValue)GetProcAddress(hApiDll, TEXT("SetBacklightValue"));
        pfnGetBacklightValue = (PFNGetBacklightValue)GetProcAddress(hApiDll, TEXT("GetBacklightValue"));
    }

    g_dwNewValue = 100;
    DialogBoxParam(g_hInst, TEXT("BacklightTestDlg"), NULL, BacklightTestDlgProc, 0);
    pfnSetBacklightValue(g_dwOldValue);

    if (MessageBox(hWnd, TEXT("Is the backlight brightness correct?"), TEXT("Backlight Test"), MB_YESNO|MB_ICONQUESTION) == IDYES)
    {
        SetDlgItemText(hWnd, IDC_EDIT4, TEXT("PASS"));
        sprintf(szText, "Backlight Test: PASS\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }
    else
    {
        SetDlgItemText(hWnd, IDC_EDIT4, TEXT("FAIL"));
        sprintf(szText, "Backlight Test: FAIL\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }

    return 0;
}