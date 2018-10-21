#include <windows.h>
#include "resource.h"
#include "api.h"

extern HANDLE g_hFile;

//----------------------------------------------------------------------
// Frame_LED_Test
//
BOOL Frame_LED_Test(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    PFNSetGreenLEDFrameValue pfnSetGreenLEDFrameValue = NULL;
    PFNSetRedLEDFrameValue pfnSetRedLEDFrameValue = NULL;
    PFNSetLEDFrameColor pfnSetLEDFrameColor = NULL;
    PFNSetLEDStatus pfnSetLEDStatus = NULL;
    CHAR szText[32];
    DWORD dwBytesWriten;

    HMODULE hApiDll = LoadLibrary(TEXT("API.DLL"));
    if (hApiDll)
    {
        pfnSetGreenLEDFrameValue = (PFNSetGreenLEDFrameValue)GetProcAddress(hApiDll, TEXT("SetGreenLEDFrameValue"));
        pfnSetRedLEDFrameValue = (PFNSetRedLEDFrameValue)GetProcAddress(hApiDll, TEXT("SetRedLEDFrameValue"));
        pfnSetLEDFrameColor = (PFNSetLEDFrameColor)GetProcAddress(hApiDll, TEXT("SetLEDFrameColor"));
        pfnSetLEDStatus = (PFNSetLEDStatus)GetProcAddress(hApiDll, TEXT("SetLEDStatus"));
    }

    pfnSetGreenLEDFrameValue(50);
    pfnSetLEDFrameColor(GREEN);
    pfnSetLEDStatus(ON);

    if (MessageBox(hWnd, TEXT("Is the Green LED on?"), TEXT("Frame LED Test"), MB_YESNO|MB_ICONQUESTION) == IDYES)
    {
        pfnSetRedLEDFrameValue(50);
        pfnSetLEDFrameColor(RED);
        pfnSetLEDStatus(ON);

        if (MessageBox(hWnd, TEXT("Is the Red LED on?"), TEXT("Frame LED Test"), MB_YESNO|MB_ICONQUESTION) == IDYES)
        {
            SetDlgItemText(hWnd, IDC_EDIT6, TEXT("PASS"));
            sprintf(szText, "Frame LED Test: PASS\r\n");
            if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
            {
                MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
            }
        }
        else
        {
            SetDlgItemText(hWnd, IDC_EDIT6, TEXT("FAIL"));
            sprintf(szText, "Frame LED Test: FAIL\r\n");
            if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
            {
                MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
            }
        }
    }
    else
    {
        SetDlgItemText(hWnd, IDC_EDIT6, TEXT("FAIL"));
        sprintf(szText, "Frame LED Test: FAIL\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }

    pfnSetLEDStatus(OFF);
    return 0;
}