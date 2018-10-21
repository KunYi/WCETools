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
#include <Iphlpapi.h>
#include "LANTestEx.h"                 // Program-specific stuff
#include "resource.h"

//----------------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = TEXT ("LANTestEx");
const TCHAR szInputName[] = TEXT ("VerifyMAC");
HINSTANCE ghInstance;
HANDLE hAdapter;

BYTE InputMAC[6];
BYTE AdapterMAC[6];

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


// Message dispatch table for MainWindowProc
const struct decodeUINT InputMessages[] = {
    WM_INITDIALOG, DoInitDlgInput,
    WM_COMMAND, DoCommandInput,
};
// Command Message dispatch for MainWindowProc
const struct decodeCMD InputCommandItems[] = {
    IDOK, DoInputCommandExit,
    IDCANCEL, DoInputCommandExit,
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
	ghInstance = hInstance;
	DialogBoxParam (hInstance, szInputName, NULL, InputDlgProc, 0);

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


//======================================================================
// Message handling procedures for Input window
//----------------------------------------------------------------------
// MainDlgProc - Callback function for application window
//
BOOL CALLBACK InputDlgProc (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    INT i;
    //
    // Search message list to see if we need to handle this
    // message. If in list, call procedure.
    //
    for (i = 0; i < dim(InputMessages); i++) {
        if (wMsg == InputMessages[i].Code)
            return (*InputMessages[i].Fxn)(hWnd, wMsg, wParam, lParam);
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
	UINT	i = 0;
	BOOL bEqual = TRUE;
	
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

//----------------------------------------------------------------------
//
UCHAR Hex2Dec(PUCHAR pStr)
{
    UCHAR i, c, nVal = 0;

    for (i = 0; i < 2; i++)
    {
        c = pStr[i];

        if (c >= '0' && c <= '9')
        {
            c -= '0';
        }
        else if (c >= 'A' && c <= 'F')
        {
            c -= 'A';
            c  = (0xA + c);
        }
        else if (c >= 'a' && c <= 'f')
        {
            c -= 'a';
            c  = (0xA + c);
        }

        if (!i)
        {
            nVal += c * 16;
        }
        else
        {
            nVal += c;
        }
    }

    return(nVal);
}

//----------------------------------------------------------------------
// DoInitDlgMain - Process WM_INITDIALOG message for window.
//
BOOL DoInitDlgInput (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD dwRetVal;
	ULONG ulBufferSize=0;
	HRESULT hr = S_OK;
	UINT	i=0;

	PIP_ADAPTER_INFO pAdapterInfoHead = NULL;
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	
	RETAILMSG(TRUE, (TEXT("+DoInitDlgInput()\r\n")));
    dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulBufferSize );

    if ( dwRetVal == ERROR_BUFFER_OVERFLOW) {
        if (!(pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulBufferSize))) {
            RETAILMSG(TRUE, (TEXT("Insufficient Memory for IPAdapter allocation.")));
            hr = E_FAIL;
            goto exit;
        }

        dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulBufferSize);
        if (dwRetVal != ERROR_SUCCESS) {
            RETAILMSG(TRUE, (TEXT("Could not get adapter info(1)")));
            hr = E_FAIL;
            goto exit;
        }
    } else if (dwRetVal != ERROR_SUCCESS) {
        RETAILMSG(TRUE, (TEXT("Could not get adapter info (2)")));
        hr = E_FAIL;
        goto exit;
    }
	
    if (pAdapterInfo == NULL) {
        RETAILMSG(TRUE, (TEXT("No IP Interface present")));
        hr = E_FAIL;
        goto exit;
    }
	
	pAdapterInfoHead = pAdapterInfo;
	
    while (strcmp("SMSC91181", pAdapterInfo->AdapterName) != 0) {
        pAdapterInfo = pAdapterInfo->Next;
        if (pAdapterInfo == NULL) {
            RETAILMSG(TRUE, (TEXT("NETUIQC: Could not get ip info for the specified adapter")));
            hr = E_FAIL;
            goto exit;
        }
    }

	AdapterMAC[0] = pAdapterInfo->Address[0];	AdapterMAC[1] = pAdapterInfo->Address[1];
	AdapterMAC[2] = pAdapterInfo->Address[2];	AdapterMAC[3] = pAdapterInfo->Address[3];
	AdapterMAC[4] = pAdapterInfo->Address[4];	AdapterMAC[5] = pAdapterInfo->Address[5];
	{
		WCHAR AdapterName[256];
		mbstowcs(AdapterName, pAdapterInfo->AdapterName, strlen(pAdapterInfo->AdapterName));
		RETAILMSG(TRUE, (TEXT("AdapterInfo Name %s\r\n"), AdapterName));
		RETAILMSG(TRUE, (TEXT("MAC Address %02x:%02x:%02x:%02x:%02x:%02x\r\n"), 
			AdapterMAC[0], AdapterMAC[1], AdapterMAC[2],
			AdapterMAC[3], AdapterMAC[4], AdapterMAC[5]));
	}
exit:
	if (NULL != pAdapterInfoHead) {
		free(pAdapterInfoHead);
	}
	RETAILMSG(TRUE, (TEXT("-DoInitDlgInput()\r\n")));
	return 0;
}

//----------------------------------------------------------------------
// DoCommandInput - Process WM_COMMAND message for window.
//
BOOL DoCommandInput (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    WORD idItem, wNotifyCode;
    HWND hwndCtl;
    INT  i;

    // Parse the parameters.
    idItem = (WORD) LOWORD (wParam);
    wNotifyCode = (WORD) HIWORD (wParam);
    hwndCtl = (HWND) lParam;

    // Call routine to handle control message.
    for (i = 0; i < dim(InputCommandItems); i++) {
        if (idItem == InputCommandItems[i].Code) {
            (*InputCommandItems[i].Fxn)(hWnd, idItem, hwndCtl,
                                       wNotifyCode);
            return TRUE;
        }
    }
    return FALSE;
}

//======================================================================
// Command handler routines
//----------------------------------------------------------------------
// DoInputCommandExit - Process Program Exit command.
//
LPARAM DoInputCommandExit (HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	TCHAR szString[64];
	UCHAR szLANID[13];
	DWORD i = 0;
	BOOL bEqual = TRUE;
	
    GetDlgItemText (hWnd, IDC_EDIT1, szString, sizeof(szString));
    WideCharToMultiByte(CP_ACP, 0, szString, -1, (LPSTR)szLANID, 12, NULL, NULL);
	
	for (i = 0; i < 6; i++)
    {
        InputMAC[i] = Hex2Dec(&szLANID[i << 1]);
    }

	// ShowWindow(hWnd, SW_HIDE);
	for (i=0; i < 6; i++)
	{
		if (InputMAC[i] != AdapterMAC[i])
			bEqual = FALSE;
	}
	
	if (FALSE == bEqual)
	{
		RETAILMSG(TRUE, (TEXT("ATTN: Adapter MAC not equal Input MAC\r\n")));
		MessageBox(hWnd, TEXT("FAIL: LAN MAC address is Wrong!!!"), TEXT("ATTENTION!!"), MB_ICONWARNING | MB_OK);
	}
	
    // Display dialog box as main window.
    DialogBoxParam (ghInstance, szAppName, NULL, MainDlgProc, 0);

    EndDialog (hWnd, 0);
    return 0;
}
