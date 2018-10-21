#include <windows.h>
#include <Pwinuser.h>
#include "resource.h"

extern HANDLE g_hFile;

//----------------------------------------------------------------------
// Text_LED_Test
//
BOOL Suspend_Test(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    CHAR szText[32];
    DWORD dwBytesWriten;

    MessageBox(hWnd, TEXT("Click OK to suspend, and then tap screen to wake up after 10 seconds."), TEXT("Suspend Test"), MB_OK|MB_ICONQUESTION);

    sprintf(szText, "Suspend Test: PASS\r\n");
    if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
    {
        MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
    }

    GwesPowerOffSystem();

    SetDlgItemText(hWnd, IDC_EDIT9, TEXT("PASS"));
    return 0;
}