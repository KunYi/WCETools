#include <windows.h>                 // For all that Windows stuff
#include <commctrl.h>
#include "resource.h"

//----------------------------------------------------------------------
// Global data
//
PDWORD g_pBuffer = NULL;
DWORD g_dwAvailPhys = 0;
BOOL g_bResult = TRUE;
extern HWND g_hWnd;
extern HINSTANCE g_hInst;
extern HANDLE g_hFile;

//======================================================================
//
DWORD WINAPI DoMemoryTestThread(PVOID pArg)
{
    CHAR szText[32];
    DWORD dwBytesWriten, i;

    for (i = 0; i< g_dwAvailPhys/4; i++)
    {
        *(g_pBuffer + i) = 0x55AA55AA;
    }

    for (i = 0; i< g_dwAvailPhys/4; i++)
    {
        if (*(g_pBuffer + i) != 0x55AA55AA)
        {
            g_bResult = FALSE;
            break;
        }
    }

    free(g_pBuffer);

    if (g_bResult)
    {
    	SetDlgItemText(g_hWnd, IDC_EDIT1, TEXT("PASS"));
        sprintf(szText, "Memory Test: PASS\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(g_hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }
    else
    {
    	SetDlgItemText(g_hWnd, IDC_EDIT1, TEXT("FAIL"));
        sprintf(szText, "Memory Test: FAIL\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(g_hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }

    return 0;
}
//----------------------------------------------------------------------
// Memory_Test
//
BOOL Memory_Test(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    HANDLE hThread;
    DWORD dwThreadID;

    g_dwAvailPhys = 20 * 1024 * 1024;
    if (!(g_pBuffer = (PDWORD)malloc(g_dwAvailPhys)))
    {
        RETAILMSG(TRUE, (TEXT("Out of memory.")));
        return (FALSE);
    }

    g_hWnd = hWnd;
    hThread = CreateThread(NULL, 0, DoMemoryTestThread, hWnd, 0 , &dwThreadID);
	CloseHandle(hThread);

    return 0;
}