//======================================================================
// TBIcons - Taskbar icon demonstration for Windows CE
//
// Written for the book Programming Windows CE
// Copyright (C) 2003 Douglas Boling
//======================================================================
#include <windows.h>                 // For all that Windows stuff
#include <CommCtrl.h>
#include "DumpImage.h"               // Program-specific stuff
#include "fmd.h"
#include "resource.h"

extern "C" BOOL RAW_LB_ReadPage(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, LPBYTE pSectorInfoBuff);
extern "C" BOOL RAW_SB_ReadPage(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, LPBYTE pSectorInfoBuff);

//----------------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = TEXT ("DumpImage");

// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_INITDIALOG, DoInitDlgMain,
    WM_COMMAND, DoCommandMain,
};
// Command Message dispatch for MainWindowProc
const struct decodeCMD MainCommandItems[] = {
    IDCANCEL, DoMainCommandExit,
    IDC_BUTTON1, DoDumpImage,
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
LPARAM DoDumpImage (HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{	
    TCHAR szString[8];
    BYTE pMBuf[2048], pSBuf[64];
    DWORD dwStartBlock, dwEndBlock, dwStartPage, dwTotalPage, i, dwBytesWriten;
    HANDLE hFile = INVALID_HANDLE_VALUE;
	FMDInterface	fmdIF;
	DWORD	BytesReturned;
	FlashInfo   flashInfo;
	DWORD	nShift = 0;
	DWORD	nSpareSize = 0;

	
	if(FALSE == FMD_OEMIoControl(IOCTL_FMD_GET_INTERFACE, NULL, NULL, (PBYTE)(&fmdIF), sizeof(FMDInterface), &BytesReturned))
	{
		// Failed
		RETAILMSG(1, (TEXT("FAILED:: IOCTL_FMD_GET_INTERFACE\r\n")));
        MessageBox(hWnd, TEXT("Failed, FMD GetInterface, IOCTL_FMD_GET_INTERFACE"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        return 0;		
	}
	
	fmdIF.pGetInfo(&flashInfo);
	
	RETAILMSG(1, (TEXT("***** dwNumBlocks %d Bytes*****\r\n"), flashInfo.dwNumBlocks));
	RETAILMSG(1, (TEXT("***** wSectorsPerBlock %d Bytes*****\r\n"), flashInfo.wSectorsPerBlock));
	RETAILMSG(1, (TEXT("***** wDataBytesPerSector %d Bytes *****\r\n"), flashInfo.wDataBytesPerSector));
	RETAILMSG(1, (TEXT("***** dwBytesPerBlock %dKB *****\r\n"), flashInfo.dwBytesPerBlock/1024));

    GetDlgItemText (hWnd, IDC_EDIT1, szString, sizeof(szString));
    dwStartBlock = _wtoi(szString);
    GetDlgItemText (hWnd, IDC_EDIT2, szString, sizeof(szString));
    dwEndBlock = _wtoi(szString);
	
	// check the NAND support
	if (flashInfo.wDataBytesPerSector == 512)
	{
		nShift = 5;
		nSpareSize = 16;
	}
	else if (flashInfo.wDataBytesPerSector == 2048)
	{
		nShift = 6;
		nSpareSize = 64;
	}
	else
	{
		RETAILMSG(1, (TEXT("FAILED:: Page size not support\r\n")));
        MessageBox(hWnd, TEXT("Failed, Page size not support!, current just support 512 & 2048 bytes"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
		return 0;
	}
	
	dwStartPage = dwStartBlock << nShift;
	dwTotalPage = (dwEndBlock - dwStartBlock + 1) << nShift;

    hFile = CreateFile(TEXT("\\Hard Disk\\FLASHIMG.img"), GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(hWnd, TEXT("Create File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        return 0;
    }

    for (i = dwStartPage; i < dwTotalPage; i++)
    {
		if (flashInfo.wDataBytesPerSector ==	512)
		{
			RAW_SB_ReadPage(i, pMBuf, pSBuf);
		}
		else
		{
			RAW_LB_ReadPage(i, pMBuf, pSBuf);
		}
		
        if(!WriteFile(hFile, pMBuf, flashInfo.wDataBytesPerSector, &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("File write failed"), TEXT("Dump Image"), MB_OK|MB_ICONQUESTION);
            goto Exit;
        }

        if(!WriteFile(hFile, pSBuf, nSpareSize, &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("File write failed"), TEXT("Dump Image"), MB_OK|MB_ICONQUESTION);
            goto Exit;
        }

        SendMessage(GetDlgItem(hWnd, IDC_PROGRESS1), PBM_SETPOS, i * 100 / dwTotalPage, NULL);
    }

    MessageBox(hWnd, TEXT("Image dump succeeded"), TEXT("Dump Image"), MB_OK|MB_ICONQUESTION);
Exit:
    CloseHandle(hFile);
    return 0;
}
