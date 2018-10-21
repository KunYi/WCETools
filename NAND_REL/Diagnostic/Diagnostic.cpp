#include <windows.h>
#include <commctrl.h>
#include <windev.h>
#include <ntddndis.h>
#include <nuiouser.h>
#include "Diagnostic.h"
#include "resource.h"

//----------------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = TEXT ("Diagnostic");
HWND g_hWnd;
HINSTANCE g_hInst;
HANDLE g_hFile = INVALID_HANDLE_VALUE;

// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] =
{
    WM_CREATE, DoCreateDialogMain,
    WM_INITDIALOG, DoInitDialogMain,
    WM_COMMAND, DoCommandMain,
};
// Command Message dispatch for MainWindowProc
const struct decodeCMD MainCommandItems[] =
{
    IDOK, DoMainCommandExit,
    IDCANCEL, DoMainCommandExit,
    IDC_CHECK1, DoMainCommandAddTestItems,
    IDC_BUTTON1, DoMainCommandStartTest,
};

struct decodePROC TestItems[] =
{
    FALSE, Memory_Test,
    FALSE, LCD_Test,
    FALSE, Touch_Test,
    FALSE, Backlight_Test,
    FALSE, LAN_Test,
    FALSE, Frame_LED_Test,
    FALSE, Text_LED_Test,
    FALSE, Buzzer_Test,
    FALSE, Suspend_Test,
};

static void LaunchCalbration(void)
{
	PROCESS_INFORMATION pi;
	INT rc;
    const TCHAR szCmdLine[] = TEXT("TKT1: P9");


	rc = CreateProcess(TEXT("Calbration.exe"), szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, NULL, &pi);
	if (rc)
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

//======================================================================
// Program entry point
// 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    ShowCursor(FALSE);

#ifndef FOROQA
	LaunchCalbration();
#endif

    g_hInst = hInstance;

    // Display dialog box as main window.
    DialogBoxParam (hInstance, szAppName, NULL, MainDlgProc, 0);
    return 0;
}
//======================================================================
// Message handling procedures for main window
//----------------------------------------------------------------------
// MainDlgProc - Callback function for application window
//
BOOL CALLBACK MainDlgProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    INT i;

    //
    // Search message list to see if we need to handle this
    // message. If in list, call procedure.
    //
    for (i = 0; i < dim(MainMessages); i++)
    {
        if (wMsg == MainMessages[i].Code)
            return (*MainMessages[i].Fxn)(hWnd, wMsg, wParam, lParam);
    }
    return FALSE;
}
//----------------------------------------------------------------------
// DoCreateDialogMain - Process WM_CREATE message for window.
//
BOOL DoCreateDialogMain(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}
//----------------------------------------------------------------------
// DoInitDialogMain - Process WM_INITDIALOG message for window.
//
BOOL DoInitDialogMain(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    HANDLE  hAdapter;
    char    Buffer[1024];
    PTCHAR  pAdapterName;
    DWORD   dwOutSize, i;
    PNDISUIO_QUERY_BINDING  pQueryBinding;
    PNDISUIO_QUERY_OID  queryOID;
    UCHAR QueryBuffer[sizeof(NDISUIO_QUERY_OID)+sizeof(DWORD)];
    TCHAR szFileName[32];

    SetWindowPos (hWnd, HWND_NOTOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0);

    for (i = 0; i < 10; i++)
    {
        SendDlgItemMessage(hWnd, IDC_CHECK1 + i, BM_SETCHECK, 1, 0);
    }

    hAdapter = CreateFile(
         NDISUIO_DEVICE_NAME,
         GENERIC_READ | GENERIC_WRITE,
         FILE_SHARE_READ | FILE_SHARE_WRITE,
         NULL,
         OPEN_EXISTING,
         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
         INVALID_HANDLE_VALUE);

    if (hAdapter == INVALID_HANDLE_VALUE) {
        RETAILMSG(1, (TEXT("Invalid hAdapter\r\n")));
        return 0;
    }

    pQueryBinding = (PNDISUIO_QUERY_BINDING)&Buffer[0];
    memset(pQueryBinding, 0x00, sizeof(NDISUIO_QUERY_BINDING));
    pQueryBinding->BindingIndex = 0;

    while(TRUE) {
        if (!DeviceIoControl(hAdapter,
                IOCTL_NDISUIO_QUERY_BINDING, pQueryBinding,
                sizeof(NDISUIO_QUERY_BINDING),
                pQueryBinding, 1024, &dwOutSize, NULL))  {
            RETAILMSG(1, (TEXT("Error at IOCTL_NDISUIO_QUERY_BINDING, GetLastError()= %d\r\n"), GetLastError()));
            goto Exit;
        }
    
        pAdapterName = (PTCHAR)&Buffer[pQueryBinding->DeviceNameOffset];
        pAdapterName[(pQueryBinding->DeviceNameLength / sizeof(TCHAR)) - 1] = 0x00;
        if (!wcscmp((const wchar_t *)pAdapterName, (const wchar_t *)L"SMSC91181"))
            break;
        pQueryBinding->BindingIndex++;
    }

    queryOID = (PNDISUIO_QUERY_OID)&QueryBuffer[0];
    queryOID->ptcDeviceName = pAdapterName;
    queryOID->Oid = OID_802_3_PERMANENT_ADDRESS;

    if (!DeviceIoControl(hAdapter,
            IOCTL_NDISUIO_QUERY_OID_VALUE,
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            &dwOutSize, NULL))
    {
        RETAILMSG(1, (TEXT("Error in DeviceIoControl (OID_802_3_PERMANENT_ADDRESS) : %d\n"), GetLastError()));
    }
    else
    {
        PBYTE pMAC = &(queryOID->Data[0]);
        wsprintf(szFileName, TEXT("\\Hard Disk\\%02X%02X%02X%02X%02X%02X.log"), pMAC[0], pMAC[1], pMAC[2], pMAC[3], pMAC[4], pMAC[5]);
        g_hFile = CreateFile(szFileName, GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
        if (g_hFile == INVALID_HANDLE_VALUE)
        {
            MessageBox(hWnd, TEXT("Create File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
            return 0;
        }
    }

Exit:
    CloseHandle(hAdapter);
    return TRUE;
}
//----------------------------------------------------------------------
// DoCommandMain - Process WM_COMMAND message for window.
//
BOOL DoCommandMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam){
    WORD idItem, wNotifyCode;
    HWND hwndCtl;
    INT  i;

    // Parse the parameters.
    idItem = (WORD)LOWORD(wParam);
    wNotifyCode = (WORD)HIWORD(wParam);
    hwndCtl = (HWND)lParam;

    // Call routine to handle control message.
    for (i = 0; i < dim(MainCommandItems); i++)
    {
        if (idItem == MainCommandItems[i].Code)
        {
            (*MainCommandItems[i].Fxn)(hWnd, idItem, hwndCtl, wNotifyCode);
            return TRUE;
        }
    }
    return FALSE;
}
//======================================================================
// Command handler routines
//----------------------------------------------------------------------
// DoMainCommandExit - Process Program Exit command.
//
LPARAM DoMainCommandExit(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    CloseHandle(g_hFile);
    EndDialog(hWnd, 0);
    return 0;
}
//----------------------------------------------------------------------
// DoMainCommandAddTestItems
//
LPARAM DoMainCommandAddTestItems(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    INT i;

    if (idItem == IDC_CHECK1 && wNotifyCode == BN_CLICKED)
    {
        if (SendDlgItemMessage (hWnd, IDC_CHECK1, BM_GETCHECK, 0, 0) == 0)
        {
            for (i = 1; i < 10; i++)
            {
                SendDlgItemMessage(hWnd, IDC_CHECK1 + i, BM_SETCHECK, 0, 0);
            }
        }
        else
        {
            for (i = 1; i < 10; i++)
            {
                SendDlgItemMessage(hWnd, IDC_CHECK1 + i, BM_SETCHECK, 1, 0);
            }
        }
    }
    return 0;
}
//----------------------------------------------------------------------
// DoMainCommandStartTest
//
LPARAM DoMainCommandStartTest(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    DWORD i;

    if (wNotifyCode == BN_CLICKED)
    {
        EnableWindow (GetDlgItem (hWnd, IDC_BUTTON1), FALSE);
        for (i = 0; i < 10; i++)
        {
            EnableWindow (GetDlgItem (hWnd, IDC_CHECK1 + i), FALSE);
            SetDlgItemText(hWnd, IDC_EDIT1 + i, TEXT(""));
        }

        for (i = 1; i < 10; i++)
        {
            if (SendDlgItemMessage(hWnd, IDC_CHECK1 + i, BM_GETCHECK, 0, 0) == 1)
            {
                (*TestItems[i-1].Fxn)(hWnd, idItem, hwndCtl, wNotifyCode);
            }
        }

        EnableWindow (GetDlgItem (hWnd, IDC_BUTTON1), TRUE);
        for (i = 0; i < 10; i++)
        {
            EnableWindow (GetDlgItem (hWnd, IDC_CHECK1 + i), TRUE);
        }
    }

    return 0;
}

