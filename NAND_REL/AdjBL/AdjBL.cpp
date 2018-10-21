// AdujstBL.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#define DEBUG (0)
// Returns number of elements
#define dim(x) (sizeof(x) / sizeof(x[0])) 

//----------------------------------------------------------------------
// Generic defines and data types
//
struct decodeUINT {                            // Structure associates
    UINT Code;                                 // messages 
                                               // with a function. 
    BOOL (*Fxn)(HWND, UINT, WPARAM, LPARAM);
}; 
struct decodeCMD {                             // Structure associates
    UINT Code;                                 // menu IDs with a 
    LRESULT (*Fxn)(HWND, WORD, HWND, WORD);    // function.
};
struct decodePROC {
    BOOL Result;
    BOOL (*Fxn)(HWND, WORD, HWND, WORD);
};


const TCHAR	szDlgName[]= TEXT("ADJBL");
HINSTANCE	g_hInst;
HMODULE	g_hLib = NULL;
PFNSetBacklightValue pfnSetBacklightValue = NULL;
PFNGetBacklightValue pfnGetBacklightValue = NULL;

BOOL DoCommandMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL DoCreateDialogMain(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL DoInitDialogMain(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
LPARAM DoMainCommandExit(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode);
LPARAM DoEditCommand(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode);
BOOL DoHscrollMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);



// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] =
{
    WM_CREATE, DoCreateDialogMain,
    WM_INITDIALOG, DoInitDialogMain,
    WM_COMMAND, DoCommandMain,
	WM_HSCROLL, DoHscrollMain,
};
// Command Message dispatch for MainWindowProc
const struct decodeCMD MainCommandItems[] =
{
    IDC_EXIT, DoMainCommandExit,
	IDC_BLV, DoEditCommand,
//    IDC_CHECK1, DoMainCommandAddTestItems,
//    IDC_BUTTON1, DoMainCommandStartTest,
};

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

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	INT_PTR	result = 0;
	INITCOMMONCONTROLSEX icex;
	
	g_hInst = hInstance;
	
	RETAILMSG( DEBUG, (TEXT("Entry Adjust Backlight\r\n")));
	
	    // Load the command bar common control class.
    icex.dwSize = sizeof (INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx (&icex);
	
	// Load MainDlg
    result = DialogBoxParam (hInstance, szDlgName, NULL, MainDlgProc, 0);	
	if(result == -1)
	{
		DWORD errcode = GetLastError();
		RETAILMSG( DEBUG, (TEXT("FAILED: Create Main Dialog, GetLastError Code: %d(0x%X)\r\n"), errcode, errcode));

		switch(errcode) {
		case 1407:
				RETAILMSG(1, (TEXT("Cannot find window class.\r\n")));
				break;
		case 1814:
				RETAILMSG(1, (TEXT("The specified resource name cannot be found in the image file.\r\n")));
				break;
		default:
				RETAILMSG(1, (TEXT("if you went know the error code mean, please to visit http://msdn.microsoft.com/en-us/library/windows/desktop/ms681381.aspx\r\n")));
				break;
		}
	}
	return 0;
}

//======================================================================
// Command handler routines
//----------------------------------------------------------------------
// DoMainCommandExit - Process Program Exit command.
//
LPARAM DoMainCommandExit(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	RETAILMSG( DEBUG, (TEXT("Exit Dialog box\r\n")));
    CloseHandle(g_hLib);
    EndDialog(hWnd, 0);
    return 0;
}

static BOOL InitBLFuncs(VOID)
{
	if(g_hLib == NULL)
	{
		if( (g_hLib = LoadLibrary(TEXT("API.DLL"))) == NULL)
		{
			return FALSE;
		}
		
		RETAILMSG( DEBUG, (TEXT("Load api.dll succes!\r\n")));
		pfnSetBacklightValue = (PFNSetBacklightValue)GetProcAddress(g_hLib, TEXT("SetBacklightValue"));
		pfnGetBacklightValue = (PFNGetBacklightValue)GetProcAddress(g_hLib, TEXT("GetBacklightValue"));

		if (pfnSetBacklightValue == NULL)
		{
			RETAILMSG(1, (TEXT("Cannot found SetBacklightValue function entry in API.DLL\r\n")));
			return FALSE;
		}
		if (pfnGetBacklightValue == NULL)
		{
			RETAILMSG(1, (TEXT("Cannot found GetBacklightValue function entry in API.DLL\r\n")));
			return FALSE;
		}
	}	
	return TRUE;
}

BOOL Num2LPSTR(TCHAR* pBuff, DWORD size, DWORD val)
{
	TCHAR	pFormat[] = TEXT("%1!*d!");
	DWORD_PTR 	pArgs[] = { (DWORD_PTR)3, (DWORD_PTR) val, (DWORD_PTR)3 };
	
    if (!FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       pFormat, 
                       0,
                       0,
                       pBuff, 
                       size, 
                       (va_list*)pArgs))
	{
		RETAILMSG(1, (TEXT("Failed: FormatMessage with %d(0x%X)\r\n"), GetLastError(), GetLastError()));
		return FALSE;
	}
	
	return TRUE;
}

//----------------------------------------------------------------------
// DoInitDialogMain - Process WM_INITDIALOG message for window.
//
BOOL DoInitDialogMain(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hWndSlider;
	HWND hWndEdit;
	DWORD dwVal = 10;
	const DWORD size = 4;

	RETAILMSG( DEBUG, (TEXT("Entry InitDialogMain\r\n")));

	if (InitBLFuncs() == FALSE) {
		return FALSE;
	}
	
	hWndSlider = GetDlgItem(hWnd, IDC_BLSLIDER);
	SendMessage(hWndSlider, TBM_SETRANGE, TRUE, MAKELONG(10, 100));
	dwVal = pfnGetBacklightValue();
	SendMessage(hWndSlider, TBM_SETPOS, TRUE, dwVal);
	
	hWndEdit = GetDlgItem(hWnd, IDC_BLV);
	SendMessage(hWndEdit, EM_SETLIMITTEXT, 3, 0);
	//Num2LPSTR(pTxt, size, dwVal);
	//SetDlgItemText(hWnd, IDC_BLV, pTxt);
	SetDlgItemInt(hWnd, IDC_BLV, dwVal, FALSE);
	return TRUE;
}

//----------------------------------------------------------------------
// DoCreateDialogMain - Process WM_CREATE message for window.
//
BOOL DoCreateDialogMain(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}

BOOL DoHscrollMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam){
    HWND hWndSlider;
    DWORD dwVal;

    switch LOWORD(wParam) 
    {
        case TB_BOTTOM:
        case TB_THUMBPOSITION:
        case TB_LINEUP:
        case TB_LINEDOWN:
        case TB_PAGEUP:
        case TB_PAGEDOWN:
        case TB_TOP:
            hWndSlider = GetDlgItem (hWnd, IDC_BLSLIDER);
            if (hWndSlider == (HWND)lParam)
            {
				const DWORD size = 4;
                dwVal = SendMessage (hWndSlider, TBM_GETPOS, 0, 0);
                pfnSetBacklightValue(dwVal);
				SetDlgItemInt(hWnd, IDC_BLV, dwVal, FALSE);
            }
            break;
        default:
            break;
    }

    return 0;
}

LPARAM DoEditCommand(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	/*
	    // Parse the parameters.
    idItem = (WORD)LOWORD(wParam);
    wNotifyCode = (WORD)HIWORD(wParam);
    hwndCtl = (HWND)lParam;

	*/
	DWORD dwVal;
	BOOL	bOK = TRUE;
	HWND	hWndSlider;
	
	switch(wNotifyCode) {
	case EN_CHANGE:
			dwVal = GetDlgItemInt(hWnd, idItem, &bOK, FALSE);
			if((dwVal > 100) || (dwVal < 10))
			{
				dwVal = (dwVal > 100) ? 100 : dwVal;
				dwVal = (dwVal < 10) ? 10 : dwVal;
				SetDlgItemInt(hWnd, idItem, dwVal, FALSE);
			}
			break;
	case EN_UPDATE:
			dwVal = GetDlgItemInt(hWnd, idItem, &bOK, FALSE);
			dwVal = (dwVal > 100) ? 100 : dwVal;
			dwVal = (dwVal < 10) ? 10 : dwVal;
			pfnSetBacklightValue(dwVal);
			hWndSlider = GetDlgItem (hWnd, IDC_BLSLIDER);
			SendMessage(hWndSlider, TBM_SETPOS, TRUE, dwVal);
			break;
	}
	return 0;
}

