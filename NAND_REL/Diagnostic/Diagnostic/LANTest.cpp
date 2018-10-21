#include <windows.h>
#include <windev.h>
#include <ntddndis.h>
#include <nuiouser.h>
#include "resource.h"

extern HANDLE g_hFile;

//----------------------------------------------------------------------
// LAN_Test
//
BOOL LAN_Test(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    HANDLE  hAdapter;
    char    Buffer[1024];
    PTCHAR  pAdapterName;
    DWORD   dwOutSize;
    PNDISUIO_QUERY_BINDING  pQueryBinding;
    PNDISUIO_QUERY_OID  queryOID;
    UCHAR QueryBuffer[sizeof(NDISUIO_QUERY_OID)+sizeof(DWORD)];
    CHAR szText[32];
    DWORD dwBytesWriten;

    MessageBox(hWnd, TEXT("Please connect the LAN cable, then press OK."), TEXT("LAN Test"), MB_OK|MB_ICONQUESTION);

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
        SetDlgItemText(hWnd, IDC_EDIT5, TEXT("FAIL"));
        sprintf(szText, "LAN Test: FAIL\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
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
            SetDlgItemText(hWnd, IDC_EDIT5, TEXT("FAIL"));
            sprintf(szText, "LAN Test: FAIL\r\n");
            if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
            {
                MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
            }
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
        SetDlgItemText(hWnd, IDC_EDIT5, TEXT("FAIL"));
        sprintf(szText, "LAN Test: FAIL\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
        goto Exit;
    }
    else if ((*(DWORD *)&queryOID->Data))
    {
        MessageBox(hWnd, TEXT("LAN connection failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        SetDlgItemText(hWnd, IDC_EDIT5, TEXT("FAIL"));
        sprintf(szText, "LAN Test: FAIL\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
        goto Exit;
    }

    if (MessageBox(hWnd, TEXT("Is the yellow LED blinking?"), TEXT("LAN Test"), MB_YESNO|MB_ICONQUESTION) == IDYES)
    {
        if (MessageBox(hWnd, TEXT("Is the green LED on?"), TEXT("LAN Test"), MB_YESNO|MB_ICONQUESTION) == IDYES)
        {
            SetDlgItemText(hWnd, IDC_EDIT5, TEXT("PASS"));
            sprintf(szText, "LAN Test: PASS\r\n");
            if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
            {
                MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
            }
        }
        else
        {
            SetDlgItemText(hWnd, IDC_EDIT5, TEXT("FAIL"));
            sprintf(szText, "LAN Test: FAIL\r\n");
            if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
            {
                MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
            }
        }
    }
    else
    {
        SetDlgItemText(hWnd, IDC_EDIT5, TEXT("FAIL"));
        sprintf(szText, "LAN Test: FAIL\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }

Exit:
    CloseHandle(hAdapter);
    return 0;
}