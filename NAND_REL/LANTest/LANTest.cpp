//======================================================================
// TBIcons - Taskbar icon demonstration for Windows CE
//
// Written for the book Programming Windows CE
// Copyright (C) 2003 Douglas Boling
//======================================================================
#include <windows.h>                 // For all that Windows stuff
#include <windev.h>
#include <ntddndis.h>
#include <nuiouser.h>
#include "LANTest.h"                 // Program-specific stuff
#include "resource.h"

//----------------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = TEXT ("LANTest");
HANDLE hAdapter;

// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_INITDIALOG, DoInitDlgMain,
    WM_COMMAND, DoCommandMain,
    WM_TIMER, DoTimerMain,
};
// Command Message dispatch for MainWindowProc
const struct decodeCMD MainCommandItems[] = {
    IDOK, DoMainCommandExit,
    IDCANCEL, DoMainCommandExit,
};

//======================================================================
// Program entry point
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
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

    // Display dialog box as main window.
    DialogBoxParam (hInstance, szAppName, NULL, MainDlgProc, 0);

    CloseHandle(hAdapter);
    return 0;
}
//======================================================================
// Message handling procedures for main window
//----------------------------------------------------------------------
// MainDlgProc - Callback function for application window
//
BOOL CALLBACK MainDlgProc (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    INT i;
    //
    // Search message list to see if we need to handle this
    // message. If in list, call procedure.
    //
    for (i = 0; i < dim(MainMessages); i++) {
        if (wMsg == MainMessages[i].Code)
            return (*MainMessages[i].Fxn)(hWnd, wMsg, wParam, lParam);
    }
    return FALSE;
}
//----------------------------------------------------------------------
// DoInitDlgMain - Process WM_INITDIALOG message for window.
//
BOOL DoInitDlgMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    char    Buffer[1024];
    PTCHAR  pAdapterName;
    DWORD   dwOutSize;
    PNDISUIO_QUERY_BINDING  pQueryBinding;
    PNDISUIO_QUERY_OID  queryOID;
    UCHAR QueryBuffer[sizeof(NDISUIO_QUERY_OID)+sizeof(DWORD)];
    TCHAR szString[25];

    pQueryBinding = (PNDISUIO_QUERY_BINDING)&Buffer[0];
    memset(pQueryBinding, 0x00, sizeof(NDISUIO_QUERY_BINDING));
    pQueryBinding->BindingIndex = 0;

    while(TRUE) {
        if (!DeviceIoControl(hAdapter,
                IOCTL_NDISUIO_QUERY_BINDING, pQueryBinding,
                sizeof(NDISUIO_QUERY_BINDING),
                pQueryBinding, 1024, &dwOutSize, NULL))  {
            RETAILMSG(1, (TEXT("Error at IOCTL_NDISUIO_QUERY_BINDING, GetLastError()= %d\r\n"), GetLastError()));
            return 0;
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
        wsprintf(szString, TEXT("%02X:%02X:%02X:%02X:%02X:%02X"), pMAC[0], pMAC[1], pMAC[2], pMAC[3], pMAC[4], pMAC[5]);
        SetDlgItemText(hWnd, IDC_STATIC1, szString);
    }

/*
    queryOID->Oid = OID_GEN_MEDIA_CONNECT_STATUS;

    if (!DeviceIoControl(hAdapter,
            IOCTL_NDISUIO_QUERY_OID_VALUE,
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            &dwOutSize, NULL))
    {
        RETAILMSG(1, (TEXT("Error in DeviceIoControl (OID_GEN_MEDIA_CONNECT_STATUS) : %d\n"), GetLastError()));
    }
    else
    {
        SetDlgItemText(hWnd, IDC_STATIC2, (*(DWORD *)&queryOID->Data)? TEXT("Disconnected") : TEXT("Connected"));
    }

    queryOID->Oid = OID_GEN_LINK_SPEED;

    if (!DeviceIoControl(hAdapter,
            IOCTL_NDISUIO_QUERY_OID_VALUE,
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            &dwOutSize, NULL))
    {
        RETAILMSG(1, (TEXT("Error in DeviceIoControl (OID_GEN_LINK_SPEED) : %d\n"), GetLastError()));
    }
    else
    {
        if (*(DWORD *)&queryOID->Data == 100000UL)
        {
            SetDlgItemText(hWnd, IDC_STATIC3, TEXT("10Mbps"));
        }
        else if (*(DWORD *)&queryOID->Data == 1000000UL)
        {
            SetDlgItemText(hWnd, IDC_STATIC3, TEXT("100Mbps"));
        }
    }
*/
    SetTimer (hWnd, ID_TIMER, 1000, NULL);
    return 0;
}
//----------------------------------------------------------------------
// DoCommandMain - Process WM_COMMAND message for window.
//
BOOL DoCommandMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    WORD idItem, wNotifyCode;
    HWND hwndCtl;
    INT  i;

    // Parse the parameters.
    idItem = (WORD) LOWORD (wParam);
    wNotifyCode = (WORD) HIWORD (wParam);
    hwndCtl = (HWND) lParam;

    // Call routine to handle control message.
    for (i = 0; i < dim(MainCommandItems); i++) {
        if (idItem == MainCommandItems[i].Code) {
            (*MainCommandItems[i].Fxn)(hWnd, idItem, hwndCtl,
                                       wNotifyCode);
            return TRUE;
        }
    }
    return FALSE;
}
//----------------------------------------------------------------------
// DoTimerMain - Process WM_TIMER message for window.
//
BOOL DoTimerMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    char    Buffer[1024];
    PTCHAR  pAdapterName;
    DWORD   dwOutSize;
    PNDISUIO_QUERY_BINDING  pQueryBinding;
    PNDISUIO_QUERY_OID  queryOID;
    UCHAR QueryBuffer[sizeof(NDISUIO_QUERY_OID)+sizeof(DWORD)];
    TCHAR szString[16];

    pQueryBinding = (PNDISUIO_QUERY_BINDING)&Buffer[0];
    memset(pQueryBinding, 0x00, sizeof(NDISUIO_QUERY_BINDING));
    pQueryBinding->BindingIndex = 0;

    while(TRUE) {
        if (!DeviceIoControl(hAdapter,
                IOCTL_NDISUIO_QUERY_BINDING, pQueryBinding,
                sizeof(NDISUIO_QUERY_BINDING),
                pQueryBinding, 1024, &dwOutSize, NULL))  {
            RETAILMSG(1, (TEXT("Error at IOCTL_NDISUIO_QUERY_BINDING, GetLastError()= %d\r\n"), GetLastError()));
            return 0;
        }

        pAdapterName = (PTCHAR)&Buffer[pQueryBinding->DeviceNameOffset];
        pAdapterName[(pQueryBinding->DeviceNameLength / sizeof(TCHAR)) - 1] = 0x00;
        if (!wcscmp((const wchar_t *)pAdapterName, (const wchar_t *)L"SMSC91181"))
            break;
        pQueryBinding->BindingIndex++;
    }

    queryOID = (PNDISUIO_QUERY_OID)&QueryBuffer[0];
    queryOID->ptcDeviceName = pAdapterName;
    queryOID->Oid = OID_GEN_MEDIA_CONNECT_STATUS;

    if (!DeviceIoControl(hAdapter,
            IOCTL_NDISUIO_QUERY_OID_VALUE,
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            &dwOutSize, NULL))
    {
        RETAILMSG(1, (TEXT("Error in DeviceIoControl (OID_GEN_MEDIA_CONNECT_STATUS) : %d\n"), GetLastError()));
    }
    else
    {
        SetDlgItemText(hWnd, IDC_STATIC2, (*(DWORD *)&queryOID->Data)? TEXT("Disconnected") : TEXT("Connected"));
    }

    queryOID->Oid = OID_GEN_LINK_SPEED;

    if (!DeviceIoControl(hAdapter,
            IOCTL_NDISUIO_QUERY_OID_VALUE,
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            &dwOutSize, NULL))
    {
        RETAILMSG(1, (TEXT("Error in DeviceIoControl (OID_GEN_LINK_SPEED) : %d\n"), GetLastError()));
    }
    else
    {
        if (*(DWORD *)&queryOID->Data == 100000UL)
        {
            SetDlgItemText(hWnd, IDC_STATIC3, TEXT("10Mbps"));
        }
        else if (*(DWORD *)&queryOID->Data == 1000000UL)
        {
            SetDlgItemText(hWnd, IDC_STATIC3, TEXT("100Mbps"));
        }
    }

    queryOID->Oid = OID_GEN_XMIT_OK;

    if (!DeviceIoControl(hAdapter,
            IOCTL_NDISUIO_QUERY_OID_VALUE,
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            &dwOutSize, NULL))
    {
        RETAILMSG(1, (TEXT("Error in DeviceIoControl (OID_GEN_XMIT_OK) : %d\n"), GetLastError()));
    }
    else
    {
        wsprintf (szString, TEXT ("%d"), *(DWORD *)&queryOID->Data);
        SetDlgItemText(hWnd, IDC_STATIC4, szString);
    }

    queryOID->Oid = OID_GEN_RCV_OK;

    if (!DeviceIoControl(hAdapter,
            IOCTL_NDISUIO_QUERY_OID_VALUE,
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            &dwOutSize, NULL))
    {
        RETAILMSG(1, (TEXT("Error in DeviceIoControl (OID_GEN_RCV_OK) : %d\n"), GetLastError()));
    }
    else
    {
        wsprintf (szString, TEXT ("%d"), *(DWORD *)&queryOID->Data);
        SetDlgItemText(hWnd, IDC_STATIC5, szString);
    }

    queryOID->Oid = OID_GEN_XMIT_ERROR;

    if (!DeviceIoControl(hAdapter,
            IOCTL_NDISUIO_QUERY_OID_VALUE,
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            &dwOutSize, NULL))
    {
        RETAILMSG(1, (TEXT("Error in DeviceIoControl (OID_GEN_XMIT_ERROR) : %d\n"), GetLastError()));
    }
    else
    {
        wsprintf (szString, TEXT ("%d"), *(DWORD *)&queryOID->Data);
        SetDlgItemText(hWnd, IDC_STATIC6, szString);
    }

    queryOID->Oid = OID_GEN_RCV_ERROR;

    if (!DeviceIoControl(hAdapter,
            IOCTL_NDISUIO_QUERY_OID_VALUE,
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            (LPVOID)queryOID,
            sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
            &dwOutSize, NULL))
    {
        RETAILMSG(1, (TEXT("Error in DeviceIoControl (OID_GEN_RCV_ERROR) : %d\n"), GetLastError()));
    }
    else
    {
        wsprintf (szString, TEXT ("%d"), *(DWORD *)&queryOID->Data);
        SetDlgItemText(hWnd, IDC_STATIC7, szString);
    }

    return 0;
}
//======================================================================
// Command handler routines
//----------------------------------------------------------------------
// DoMainCommandExit - Process Program Exit command.
//
LPARAM DoMainCommandExit (HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    EndDialog (hWnd, 0);
    return 0;
}
