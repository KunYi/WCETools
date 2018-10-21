//======================================================================
// TBIcons - Taskbar icon demonstration for Windows CE
//
// Written for the book Programming Windows CE
// Copyright (C) 2003 Douglas Boling
//======================================================================
#include <windows.h>                 // For all that Windows stuff
#include <CommCtrl.h>
#include <string.h>
#include "UpdateOS.h"                // Program-specific stuff
#include "resource.h"
#include "api.h"

#define ERM1_NAME  TEXT("ERM")
#define	ERM2_NAME  TEXT("ERM2")
#define ERM2A_NAME TEXT("ERM2A")
#define	REG_IDENT  TEXT("Ident")
#define	REG_NAME  TEXT("Name")

#define	MODEL_ERM1		1
#define	MODEL_ERM2		2
#define MODEL_ERM2A		3
#define	ERROR_READMODEL		-1
#define	ERROR_NOTSUPPORTMODEL	-2
int compareMachineModel(VOID);
//----------------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = TEXT ("UpdateERM");
HWND g_hWnd;
PFNStartUpgrade pfnStartUpgrade = NULL;
PFNGetUpgradeProgress pfnGetUpgradeProgress = NULL;
PFNEnabledUpdate2ndOS pfnEnabledUpdate2ndOS = NULL;
PFNDisabledUpdate2ndOS pfnDisabledUpdate2ndOS = NULL;
UINT	gExit = 0;
INT32	iModel = 0;

// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_INITDIALOG, DoInitDlgMain,
    WM_COMMAND, DoCommandMain,
};
// Command Message dispatch for MainWindowProc
const struct decodeCMD MainCommandItems[] = {
    IDOK, DoMainCommandExit,
    IDCANCEL, DoMainCommandExit,
    IDC_BUTTON1, DoOSUpgrade,
};

//======================================================================
// Program entry point
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;

	HMODULE hApiDll = LoadLibrary(TEXT("API.DLL"));
    if (hApiDll) {
        pfnStartUpgrade = (PFNStartUpgrade)GetProcAddress(hApiDll, TEXT("StartUpgrade"));
        pfnGetUpgradeProgress = (PFNGetUpgradeProgress)GetProcAddress(hApiDll, TEXT("GetUpgradeProgress"));
		pfnEnabledUpdate2ndOS = (PFNEnabledUpdate2ndOS)(GetProcAddress(hApiDll, TEXT("EnabledUpdate2ndOS")));
		pfnDisabledUpdate2ndOS = (PFNDisabledUpdate2ndOS)(GetProcAddress(hApiDll, TEXT("DisabledUpdate2ndOS")));
    }
	else {
		RETAILMSG(1, (TEXT("Cannot load api.dll\r\n")));
	}
	
	if ((NULL == pfnStartUpgrade) || (NULL == pfnGetUpgradeProgress))
	{
		MessageBox(NULL, TEXT("Loading API.DLL failed!!\r\nincorrect version or missing it!"), 
			TEXT("OS Upgrade"), MB_OK | MB_ICONERROR);
		RETAILMSG(1, (TEXT("Cannot locate function\r\n")));
		return -1;
	}

	switch (compareMachineModel()) {
	case ERROR_READMODEL:
			// Warring Dialog box
		MessageBox(NULL, TEXT("Failed! Cannot found device model!"), 
			TEXT("OS Upgrade"), MB_OK | MB_ICONERROR);
		return -2;
	case  ERROR_NOTSUPPORTMODEL:
		MessageBox(NULL, (TEXT("Failed! Only support ERM2 model!")),
			TEXT("ERROR"), MB_OK | MB_ICONERROR );
		return -3;
	case MODEL_ERM1:
			iModel = MODEL_ERM1;
			break;
	case MODEL_ERM2:
			iModel = MODEL_ERM2;
			break;
	case MODEL_ERM2A:
			iModel = MODEL_ERM2A;
			break;
	default:
		MessageBox(NULL, TEXT("Failed: Unknow defined in the program"),
			TEXT("ERROR"), MB_OK | MB_ICONERROR);
		return -4;
	}

	if ((NULL == pfnEnabledUpdate2ndOS) || (NULL == pfnDisabledUpdate2ndOS))
	{
		if (iModel != MODEL_ERM1)
		{
			MessageBox(NULL, TEXT("Loading API.DLL for ERM2 failed!!\r\nincorrect version or missing it!"), 
				TEXT("OS Upgrade"), MB_OK | MB_ICONERROR);
			return -5;
		}
	}

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
BOOL CALLBACK MainDlgProc (HWND hWnd, UINT wMsg, WPARAM wParam, 
                           LPARAM lParam) {
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
BOOL DoInitDlgMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam){

    return 0;
}
//----------------------------------------------------------------------
// DoCommandMain - Process WM_COMMAND message for window.
//
BOOL DoCommandMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam){
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
LPARAM DoMainCommandExit (HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode) {
    EndDialog (hWnd, 0);
    return 0;
}
//----------------------------------------------------------------------
LPARAM DoOSUpgrade (HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode) {
    HANDLE hThread = NULL;
    DWORD dwThreadID = 0, dwProgress = 0, dwPreProgress = 0;

    if (wNotifyCode == BN_CLICKED)
    {
        g_hWnd = hWnd;
		gExit = 0;

        EnableWindow (GetDlgItem (hWnd, IDC_BUTTON1), FALSE);

        if (!(hThread = CreateThread(NULL, 0, DoOSUpgradeThread, hWnd, 0 , &dwThreadID)))
        {
            MessageBox(NULL, TEXT("Upgrade fail!"), TEXT("OS Upgrade"), MB_OK);
            return 0;
        }
        else
        {
            CloseHandle(hThread);
        }

        HWND hwndProgressBar = GetDlgItem (hWnd, IDC_PROGRESS1);

        while (((dwProgress = pfnGetUpgradeProgress()) != 100) && (gExit != 1))
        {
            if (dwPreProgress != dwProgress)
            {
                dwPreProgress = dwProgress;
                SendMessage(hwndProgressBar, PBM_SETPOS, dwProgress, NULL);    
            }
            Sleep(10);
			if (gExit == 1) {
				break;
			}
        }

		if (gExit == 1) {
			EndDialog (hWnd, 0);
		}
    }
    return 0;
}

//----------------------------------------------------------------------
BOOL CheckFileExist(const TCHAR* filename)
{
	HANDLE hFile = NULL;

	hFile = CreateFile(filename, 
					GENERIC_READ, 
					FILE_SHARE_READ,
					0,
					OPEN_EXISTING,
					0,
					NULL);

	if (hFile == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}

	CloseHandle(hFile);

	return TRUE;
}

//----------------------------------------------------------------------
DWORD WINAPI DoOSUpgradeThread (PVOID pArg) {
	TCHAR const *strERM1PATH1 = TEXT("\\Hard Disk\\ERM1.BIN");
	TCHAR const	*strERM1PATH2 = TEXT("\\Storage Card\\ERM1.BIN");
	TCHAR const	*strERM2PATH1 = TEXT("\\Hard Disk\\ERM2.BIN");
	TCHAR const *strERM2PATH2 = TEXT("\\Storage Card\\ERM2.BIN");
	TCHAR const	*strERM2APATH1 = TEXT("\\Hard Disk\\ERM2A.BIN");
	TCHAR const *strERM2APATH2 = TEXT("\\Storage Card\\ERM2A.BIN");

	TCHAR const *os_path1 = NULL;
	TCHAR const *os_path2 = NULL;

	RETAILMSG(1, (TEXT("++DoOSUpgradeThread()\r\n")));

	switch (iModel) {
	case MODEL_ERM1:
			os_path1 = strERM1PATH1;
			os_path2 = strERM1PATH2;
			break;
	case MODEL_ERM2:
			os_path1 = strERM2PATH1;
			os_path2 = strERM2PATH2;
			pfnDisabledUpdate2ndOS();
			break;
	case MODEL_ERM2A:
			os_path1 = strERM2APATH1;
			os_path2 = strERM2APATH2;
			pfnDisabledUpdate2ndOS();
			break;
	default:
			gExit = 1;
			return -1;
	}

    if ( (FALSE == CheckFileExist(os_path1)) && 
		 (FALSE == CheckFileExist(os_path2)) )
	{
		const TCHAR* pStr =  (iModel == MODEL_ERM1) ? TEXT("EMR1.BIN") : 
							 (iModel == MODEL_ERM2) ? TEXT("ERM2.BIN") : 
							 (iModel == MODEL_ERM2A) ? TEXT("ERM2A.BIN") :
							 TEXT("Unknow Model") ;

		RETAILMSG(1, (TEXT("Cannot found %s"), pStr));
		MessageBox(NULL, TEXT("OS image not found!"),
			TEXT("Warring"), MB_OK | MB_ICONWARNING);

		gExit = 1;
		return -2;
	}

    if (OK == pfnStartUpgrade(os_path1))
    {
        EnableWindow (GetDlgItem (g_hWnd, IDC_BUTTON1), TRUE);
        MessageBox(NULL, TEXT("OS update succeeded! Please restart your device."), szAppName, MB_OK);
    }
	else
	{
		if (OK == pfnStartUpgrade(os_path2))
		{
			EnableWindow (GetDlgItem (g_hWnd, IDC_BUTTON1), TRUE);
			MessageBox(NULL, TEXT("OS update succeeded! Please restart your device."), szAppName, MB_OK);
		}
		else
		{
			RETAILMSG(1, (TEXT("Failed:: OS update error!!\n")));
			MessageBox(NULL, TEXT("OS update failed!"), szAppName, MB_OK);
			gExit = 1;
		}
	}

    return 0;
}

BOOL ReadDevice(LPTSTR buff, DWORD len)
{
	HKEY  hKey = NULL;
	BOOL	result = FALSE;
	DWORD	regtype = REG_SZ;

	RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_IDENT,  0, 0,  &hKey);

	if (hKey == NULL)
	{
		return result;
	}

	if (ERROR_SUCCESS == RegQueryValueEx(hKey,  REG_NAME, NULL,  &regtype, (LPBYTE) buff, &len))
	{
		result = TRUE;
	}

	RegCloseKey(hKey);

	return result;
}

int compareMachineModel(VOID)
{
	TCHAR   buff[20];
	BOOL	result = ERROR_NOTSUPPORTMODEL; // not correct model
	int r = 0;

	if (FALSE == ReadDevice(buff, sizeof(buff)-1))
	{
		return ERROR_READMODEL;
	}

	buff[5] =0;
	buff[6] =0;

	if ( 0 == _tcscmp(ERM1_NAME, buff))
	{
		result = MODEL_ERM1;
	} else if ( 0 == _tcscmp(ERM2_NAME, buff))
	{
		result = MODEL_ERM2;
	} else if ( 0 == _tcscmp(ERM2A_NAME, buff))
	{
		result = MODEL_ERM2A;
	}

	return result;
}