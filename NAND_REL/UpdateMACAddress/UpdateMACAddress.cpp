//======================================================================
// TBIcons - Taskbar icon demonstration for Windows CE
//
// Written for the book Programming Windows CE
// Copyright (C) 2003 Douglas Boling
//======================================================================
#include <windows.h>                 // For all that Windows stuff
#include <CommCtrl.h>
#include "UpdateMACAddress.h"        // Program-specific stuff
#include "nand.h"
#include "resource.h"

//----------------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = TEXT ("UpdateMACAddress");
PBOOT_CFG g_pBootCfg;
UCHAR g_TOC[4096];
const PTOC g_pTOC = (PTOC)g_TOC;

// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_INITDIALOG, DoInitDlgMain,
    WM_COMMAND, DoCommandMain,
};
// Command Message dispatch for MainWindowProc
const struct decodeCMD MainCommandItems[] = {
    IDOK, DoUpdateMACAddress,
    IDCANCEL, DoMainCommandExit,
};

//======================================================================
// Program entry point
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX icex;

    // Load the command bar common control class.
    icex.dwSize = sizeof (INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx (&icex);

    // Display dialog box as main window.
    DialogBoxParam (hInstance, szAppName, NULL, MainDlgProc, 0);
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
    SetFocus(GetDlgItem(hWnd, IDC_EDIT1));
    SendDlgItemMessage (hWnd, IDC_EDIT1, EM_SETLIMITTEXT, 12, 0);
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
LPARAM DoUpdateMACAddress (HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	TCHAR szString[64];
	UCHAR szLANID[13], bMac[6];
    DWORD i;

    GetDlgItemText (hWnd, IDC_EDIT1, szString, sizeof(szString));
    WideCharToMultiByte(CP_ACP, 0, szString, -1, (LPSTR)szLANID, 12, NULL, NULL);

    for (i = 0; i < 6; i++)
    {
        bMac[i] = Hex2Dec(&szLANID[i << 1]);
    }

    if (!TOC_Read(g_pTOC, sizeof(g_TOC))) 
    {
        MessageBox(hWnd, TEXT("MAC Update Failed"), TEXT("Update MAC Address"), MB_OK);
        return 0;
    }

    g_pBootCfg->EdbgAddr.wMAC[0] = (bMac[1] << 8) | bMac[0];
    g_pBootCfg->EdbgAddr.wMAC[1] = (bMac[3] << 8) | bMac[2];
    g_pBootCfg->EdbgAddr.wMAC[2] = (bMac[5] << 8) | bMac[4];

    if (!TOC_Write(g_pTOC, sizeof(g_TOC))) 
    {
        MessageBox(hWnd, TEXT("MAC update failed"), TEXT("Update MAC Address"), MB_OK|MB_ICONQUESTION);
    }
    else
    {
        MessageBox(hWnd, TEXT("MAC update succeeded"), TEXT("Update MAC Address"), MB_OK|MB_ICONQUESTION);
    }

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
