// HelloWorld.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "HelloWorld.h"
#include <commctrl.h>
#include "api.h"

//======================================================================
// Global Variables
//======================================================================
HWND				g_Main_hWnd;			// The main window handle
HINSTANCE			g_Main_hInst;			// The current instance
HWND				g_Main_hWndCB;			// The command bar handle
HINSTANCE			g_hLibrary;				// The API library handle

bool				g_bContinue;
volatile DWORD		g_dwFrameLEDColor, g_dwGreenFrameLEDLevel, g_dwRedFrameLEDLevel;
int					g_FrameLED_Counter;
volatile DWORD		g_dwTimeStart, g_dwTimeElapsed;
HWND				g_Main_hTimeThread;
HANDLE				g_Main_hTimeEvent;

//-----------------------------------------------------------------------------
// LED Frame Control API
//-----------------------------------------------------------------------------
PFNSetGreenLEDFrameValue pfnSetGreenLEDFrameValue;
PFNGetGreenLEDFrameValue pfnGetGreenLEDFrameValue;

PFNSetRedLEDFrameValue pfnSetRedLEDFrameValue;
PFNGetRedLEDFrameValue pfnGetRedLEDFrameValue;

PFNSetLEDFrameColor pfnSetLEDFrameColor;
PFNGetLEDFrameColor pfnGetLEDFrameColor;

PFNSetLEDStatus pfnSetLEDStatus;
PFNGetLEDFrameStatus pfnGetLEDFrameStatus;

//-----------------------------------------------------------------------------
// Text LED Control API
//-----------------------------------------------------------------------------
PFNSetTextLEDStatus pfnSetTextLEDStatus;
PFNGetTextLEDStatus pfnGetTextLEDStatus;

//-----------------------------------------------------------------------------
// Buzzer Control API
//-----------------------------------------------------------------------------
PFNSetBuzzerValue pfnSetBuzzerValue;
PFNGetBuzzerValue pfnGetBuzzerValue;
PFNBuzzerBeep pfnBuzzerBeep;

//-----------------------------------------------------------------------------
// Backlight Control API
//-----------------------------------------------------------------------------
PFNSetBacklightValue pfnSetBacklightValue;
PFNGetBacklightValue pfnGetBacklightValue;
PFNSetBacklightStatus pfnSetBacklightStatus;
PFNGetBacklightStatus pfnGetBacklightStatus;

//-----------------------------------------------------------------------------
// Updating OS Control API
//-----------------------------------------------------------------------------
PFNStartUpgrade pfnStartUpgrade;
PFNGetUpgradeProgress pfnGetUpgradeProgress;


struct CommonControls MainCommonControls[] = 
{
	{NULL, DoMainCommandThreadLCD, 0, TEXT ("BUTTON"), TEXT ("LCD"),			WS_VISIBLE | WS_CHILD  | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, (HMENU)IDC_MAIN_CHECKBOX_LCD, NULL},
	{NULL, DoMainCommandThreadBacklight, 0, TEXT ("BUTTON"), TEXT ("BACKLIGHT"),	WS_VISIBLE | WS_CHILD  | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, (HMENU)IDC_MAIN_CHECKBOX_BACKLIGHT, NULL},
	{NULL, DoMainCommandThreadFrameLED, 0, TEXT ("BUTTON"), TEXT ("FRAME LED"),			WS_VISIBLE | WS_CHILD  | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, (HMENU)IDC_MAIN_CHECKBOX_FRAME_LED, NULL},
	{NULL, DoMainCommandThreadTextLED, 0, TEXT ("BUTTON"), TEXT ("TEXT LED"),			WS_VISIBLE | WS_CHILD  | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, (HMENU)IDC_MAIN_CHECKBOX_TEXT_LED, NULL},
	{NULL, DoMainCommandThreadBuzzer, 0, TEXT ("BUTTON"), TEXT ("BUZZER"),		WS_VISIBLE | WS_CHILD  | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, (HMENU)IDC_MAIN_CHECKBOX_BUZZER, NULL},
	{NULL, DoMainCommandThreadMemory, 0, TEXT ("BUTTON"), TEXT ("MEMORY"),			WS_VISIBLE | WS_CHILD  | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, (HMENU)IDC_MAIN_CHECKBOX_Memory, NULL},

	//{NULL, DoMainCommandThreadLAN, 0, TEXT ("BUTTON"), TEXT ("LAN"),			WS_VISIBLE | WS_CHILD  | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, (HMENU)IDC_MAIN_CHECKBOX_LAN, NULL},
	//{NULL, DoMainCommandThreadVideo, 0, TEXT ("BUTTON"), TEXT ("VIDEO"), WS_VISIBLE | WS_CHILD  | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, (HMENU)IDC_MAIN_CHECKBOX_VIDEO, NULL},
};

//======================================================================
// Message dispatch table for MainWindowProc
//======================================================================
const struct decodeUINT MainMessages[] =
{
	WM_CREATE, DoCreateMain,
	WM_SIZE, DoSizeMain,
	//WM_ACTIVATE, DoActivateMain,
	WM_PAINT, DoPaintMain,
	//WM_ERASEBKGND, DoEraseBKGndMain,
	////WM_MEASUREITEM, DoMeasureItemMain, // The owner window receives a WM_MEASUREITEM message when the button is created.
	//WM_DRAWITEM, DoDrawItemMain, // The owner receives a WM_DRAWITEM message when a visual aspect of the button has changed.
	//WM_KEYDOWN, DoKeyDownMain,
    WM_HIBERNATE, DoHibernateMain,
	WM_DESTROY, DoDestroyMain,
	WM_CLOSE, DoCloseMain,
	WM_COMMAND, DoCommandMain,
	//WM_NOTIFY, DoNotifyMain,
};

//======================================================================
// Command dispatch table for 
//======================================================================
const struct decodeCMD MainCommandItems[] =
{
	IDM_HELP_ABOUT, DoMainCommandABOUT,
	IDM_FILE_EXIT, DoMainCommandEXIT,
	//IDC_MAIN_CHECKBOX_LCD, DoMainCommandLCD,
	//IDC_MAIN_CHECKBOX_LED, DoMainCommandLED,
	//IDC_MAIN_CHECKBOX_BACKLIGHT, DoMainCommandBACKLIGHT,
	//IDC_MAIN_CHECKBOX_BUZZER, DoMainCommandBUZZER,
	//IDC_MAIN_CHECKBOX_LAN, DoMainCommandLAN,
	IDC_MAIN_BUTTON_START, DoMainCommandSTART,
	IDC_MAIN_BUTTON_END, DoMainCommandEXIT,
	IDC_MAIN_BUTTON_ALL, DoMainCommandALL,
	IDC_MAIN_BUTTON_NONE, DoMainCommandNONE,
};

//======================================================================
// Notification message dispatch table for MainWindowProc
//======================================================================
/*const struct decodeNotify MainNotifyItems[] =
{
	ID_LISTV, DoMainNotifyListV,
};*/


int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	//=========================================================================
	// Detect whether another instance of our application already exists.
	//=========================================================================
	HANDLE	hMutex;
	HWND	hPrevWnd;
	TCHAR	szTitle[MAX_PATH];			// The title bar text
	TCHAR	szWindowClass[MAX_PATH];		// The window class name
	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szWindowClass, MAX_PATH);
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_PATH);
	wsprintf(szWindowClass, TEXT("MMD BurnIn test"));
	wsprintf(szTitle, TEXT("MMD BurnIn test"));

	// Create mutex
	hMutex = CreateMutex(NULL, TRUE, szTitle);

	switch(GetLastError())
	{
		case ERROR_SUCCESS:
			// Mutex created successfully. There is no instance running.
			break;
		case ERROR_ALREADY_EXISTS:
			// Mutex already exists so there is a running instance of our app.
			hMutex = NULL;

			hPrevWnd = FindWindow(szWindowClass, szTitle);
			//if(IsIconic(hPrevWnd))
			//{
				ShowWindow(hPrevWnd, SW_RESTORE);
			//}
			SetForegroundWindow(hPrevWnd);
			return false;
			break;
		default:
			break;
	}
	//=========================================================================

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_HELLOWORLD);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    It is important to call this function so that the application 
//    will get 'well formed' small icons associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS	wc;

    wc.style			= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc		= (WNDPROC) WndProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
    wc.hInstance		= hInstance;
    wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HELLOWORLD));
    wc.hCursor			= 0;
    wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName		= 0;
    wc.lpszClassName	= szWindowClass;

	return RegisterClass(&wc);
}

//
//  FUNCTION: InitInstance(HANDLE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND	hWnd;
	TCHAR	szTitle[MAX_PATH];			// The title bar text
	TCHAR	szWindowClass[MAX_PATH];		// The window class name
	int		nWidth, nHeight;

	g_Main_hInst = hInstance;		// Store instance handle in our global variable
	// Initialize global strings
	LoadString(hInstance, IDC_HELLOWORLD, szWindowClass, MAX_PATH);
	MyRegisterClass(hInstance, szWindowClass);

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_PATH);

	//hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
		//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	nWidth = GetSystemMetrics(SM_CXSCREEN);
	nHeight = GetSystemMetrics(SM_CYSCREEN);

	hWnd = CreateWindowEx(WS_EX_WINDOWEDGE | WS_EX_TOPMOST | WS_EX_ABOVESTARTUP /*| WS_EX_CAPTIONOKBTN*/, 
					  szWindowClass, szTitle,
					  WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_POPUP ,
					  (int)((float)nWidth / 4.0), (int)((float)nHeight / 4.0), (int)(((float)nWidth / 4.0) * 2.0), (int)(((float)nHeight / 4.0) * 2.0),
					  //0, 0, nWidth, nHeight,
					  //CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					  NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{	
		return FALSE;
	}
	else
	{
		g_Main_hWnd = hWnd;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	if (g_Main_hWndCB)
	{
		CommandBar_Show(g_Main_hWndCB, TRUE);
	}

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT i;
	//-------------------------------------------------------------------------
	// Search message list to see if we need to handle this
	// message.  If in list, call procedure.
	//-------------------------------------------------------------------------
	for (i = 0; i < dim(MainMessages); i++)
	{
		if (message == MainMessages[i].Code)
			return (*MainMessages[i].Fxn)(hWnd, message, wParam, lParam);
	}

	return DefWindowProc (hWnd, message, wParam, lParam);
}



//======================================================================
// DoCreateMain - Process WM_CREATE message for window.
//======================================================================
LRESULT DoCreateMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	g_Main_hWndCB = CommandBar_Create(g_Main_hInst, hWnd, 1);			
	CommandBar_InsertMenubar(g_Main_hWndCB, g_Main_hInst, IDM_MENU, 0);
	int i;
	HWND	hMainButtonStart, hMainButtonEnd, hMainButtonAll, hMainButtonNone;


	//-----------------------------------------------------------------------------
	// XXXX Common Control
	//-----------------------------------------------------------------------------
	for (i = 0; i < dim(MainCommonControls); ++i)
	{
		if( !(MainCommonControls[i].hThreadHandle = CreateWindowEx (MainCommonControls[i].dwExStyle,
												MainCommonControls[i].lpClassName,
												MainCommonControls[i].lpWindowName,
												MainCommonControls[i].dwStyle,
												MainCommonControls[i].x,
												MainCommonControls[i].y,
												MainCommonControls[i].nWidth,
												MainCommonControls[i].nHeight,
												hWnd, (HMENU)MainCommonControls[i].hMenu, g_Main_hInst, NULL)) )
		{
			TCHAR	szDbg[MAX_PATH];
			wsprintf(szDbg, TEXT("Error on Create ex-Window : %s\r\n"), MainCommonControls[i].lpWindowName);
			MessageBox (hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return  FALSE;
		}
		else
		{
			MainCommonControls[i].hThreadHandle = NULL;
		}
	}

	hMainButtonStart = CreateWindowEx(WS_EX_CLIENTEDGE,
									TEXT("BUTTON"),
									TEXT("Start"),
									WS_VISIBLE | WS_CHILD | WS_TABSTOP,
									0, 0, 0, 0,
									hWnd, (HMENU)IDC_MAIN_BUTTON_START , g_Main_hInst, NULL);
	hMainButtonEnd = CreateWindowEx(WS_EX_CLIENTEDGE,
									TEXT("BUTTON"),
									TEXT("End"),
									WS_VISIBLE | WS_CHILD | WS_TABSTOP,
									0, 0, 0, 0,
									hWnd, (HMENU)IDC_MAIN_BUTTON_END , g_Main_hInst, NULL);
	hMainButtonAll = CreateWindowEx(WS_EX_CLIENTEDGE,
									TEXT("BUTTON"),
									TEXT("Select All"),
									WS_VISIBLE | WS_CHILD | WS_TABSTOP,
									0, 0, 0, 0,
									hWnd, (HMENU)IDC_MAIN_BUTTON_ALL , g_Main_hInst, NULL);
	hMainButtonNone = CreateWindowEx(WS_EX_CLIENTEDGE,
									TEXT("BUTTON"),
									TEXT("Cancel All"),
									WS_VISIBLE | WS_CHILD | WS_TABSTOP,
									0, 0, 0, 0,
									hWnd, (HMENU)IDC_MAIN_BUTTON_NONE , g_Main_hInst, NULL);

	SetActiveWindow(hWnd);
	PostMessage(GetDlgItem(hWnd, IDC_MAIN_BUTTON_ALL), BM_CLICK, NULL, NULL);


	if(	!g_Main_hWndCB ||
		!hMainButtonStart ||
		!hMainButtonEnd
		)
	{
		MessageBox (hWnd, TEXT("Error on Create ex-Window"), TEXT("Error"), MB_OK | MB_ICONERROR);
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		return  FALSE;
	}


	return FALSE;
}


//======================================================================
// DoSizeMain - Process WM_SIZE message for window.
//======================================================================
LRESULT DoSizeMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	RECT	rectCommandBar, rect;
	LONG	lWidth, lHeight, lCommandBarHeigth;
	int		i, j;
	float	tempMargen, tempHeight;

	// Adjust the size of the client rect.
	GetWindowRect(g_Main_hWndCB, &rectCommandBar);
	GetClientRect (hWnd, &rect);

	lCommandBarHeigth = (rectCommandBar.bottom - rectCommandBar.top);
	lWidth = rect.right - rect.left;
	lHeight = (rect.bottom - (rect.top + lCommandBarHeigth));

	tempMargen = (float)lHeight / (2 * dim(MainCommonControls) + (dim(MainCommonControls) + 1));
	tempHeight = 2 * tempMargen;
	//-----------------------------------------------------------------------------
	// Main Common Control
	//-----------------------------------------------------------------------------
	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		SetWindowPos (GetDlgItem (hWnd, ((int)MainCommonControls[i].hMenu)), HWND_NOTOPMOST,
			(int)(lWidth * 0.10F), (int)((i * tempHeight) + ((i + 1) * tempMargen) + lCommandBarHeigth),
			(int)(lWidth * 0.25F), (int)(tempHeight), SWP_SHOWWINDOW);
	}

	i = 0;
	j = 4;
	tempMargen = (float)lHeight / ((2 * j) + (j + 1));
	tempHeight = 2 * tempMargen;
	SetWindowPos (GetDlgItem (hWnd, ((int)IDC_MAIN_BUTTON_START)), HWND_NOTOPMOST,
			(int)(lWidth * 0.70F), (int)((i * tempHeight) + ((i + 1) * tempMargen) + lCommandBarHeigth),
			(int)(lWidth * 0.20F), (int)(tempHeight), SWP_SHOWWINDOW);
	i = 1;
	SetWindowPos (GetDlgItem (hWnd, ((int)IDC_MAIN_BUTTON_END)), HWND_NOTOPMOST,
			(int)(lWidth * 0.70F), (int)((i * tempHeight) + ((i + 1) * tempMargen) + lCommandBarHeigth),
			(int)(lWidth * 0.20F), (int)(tempHeight), SWP_SHOWWINDOW);
	i = 2;
	SetWindowPos (GetDlgItem (hWnd, ((int)IDC_MAIN_BUTTON_ALL)), HWND_NOTOPMOST,
			(int)(lWidth * 0.70F), (int)((i * tempHeight) + ((i + 1) * tempMargen) + lCommandBarHeigth),
			(int)(lWidth * 0.20F), (int)(tempHeight), SWP_SHOWWINDOW);
	i = 3;
	SetWindowPos (GetDlgItem (hWnd, ((int)IDC_MAIN_BUTTON_NONE)), HWND_NOTOPMOST,
			(int)(lWidth * 0.70F), (int)((i * tempHeight) + ((i + 1) * tempMargen) + lCommandBarHeigth),
			(int)(lWidth * 0.20F), (int)(tempHeight), SWP_SHOWWINDOW);


	return FALSE;
}




//======================================================================
// DoPaintMain - Process WM_Paint message for window.
//======================================================================
LRESULT DoPaintMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	HDC				hdc;
	PAINTSTRUCT		ps;

	//RECT rt;
	//TCHAR szDbg[MAX_PATH];

	hdc = BeginPaint(hWnd, &ps);

	//GetClientRect(hWnd, &rt);
	//LoadString(g_Main_hInst, IDS_APP_TITLE, szDbg, MAX_PATH);
	//DrawText(hdc, szDbg, _tcslen(szDbg), &rt, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	EndPaint(hWnd, &ps);
	return FALSE;
}

//======================================================================
// DoHibernateMain - Process WM_Hibernate message for window.
//======================================================================
LRESULT DoHibernateMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	PostMessage(hWnd, WM_CLOSE, 0, 0);
	return FALSE;
}


//======================================================================
// DoDestroyMain - Process WM_DESTROY message for window.
//======================================================================
LRESULT DoDestroyMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	int i;

	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		if(MainCommonControls[i].hThreadHandle)
		{
			CloseHandle( MainCommonControls[i].hThreadHandle );
			MainCommonControls[i].hThreadHandle = NULL;
		}
	}
	PostQuitMessage (0);
	return FALSE;
}


//======================================================================
// DoCloseMain - Process WM_CLOSE message for window.
//======================================================================
LRESULT DoCloseMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	//Save any application-specific information
	CommandBar_Destroy(g_Main_hWndCB);
	DestroyWindow (hWnd);
	return FALSE;
}


//======================================================================
// DoCommandMain - Process WM_COMMAND message for window.
//======================================================================
LRESULT DoCommandMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    WORD idItem, wNotifyCode;
    HWND hwndCtl;
    INT  i;

    // Parse the parameters.
    idItem = (WORD) LOWORD (wParam);
    wNotifyCode = (WORD) HIWORD (wParam);
    hwndCtl = (HWND) lParam;

    // Call routine to handle control message.
    for (i = 0; i < dim(MainCommandItems); i++)
	{
        if (idItem == MainCommandItems[i].Code)
		{
            (*MainCommandItems[i].Fxn)(hWnd, idItem, hwndCtl, wNotifyCode);
			return DefWindowProc(hWnd, wMsg, wParam, lParam);
        }
    }
	return FALSE;
}


//----------------------------------------------------------------------
// DoMainCommandABOUT - Process WM_Command IDM_HELP_ABOUT Command for window.
//----------------------------------------------------------------------
LPARAM DoMainCommandABOUT(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	DialogBox(g_Main_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
	return FALSE;
}


//----------------------------------------------------------------------
// DoMainCommandEXIT - Process WM_Command IDM_FILE_EXIT Command for window.
//----------------------------------------------------------------------
LPARAM DoMainCommandEXIT(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	CommandBar_Destroy(g_Main_hWndCB);
	DestroyWindow(hWnd);
	return FALSE;
}


// Mesage handler for the About box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rt, rt1;
	int DlgWidth, DlgHeight;	// dialog width and height in pixel units
	int NewPosX, NewPosY;

	switch (message)
	{
		case WM_INITDIALOG:
			// trying to center the About dialog
			if (GetWindowRect(hDlg, &rt1))
			{
				GetClientRect(GetParent(hDlg), &rt);
				DlgWidth	= rt1.right - rt1.left;
				DlgHeight	= rt1.bottom - rt1.top ;
				MapWindowPoints(GetParent(hDlg), HWND_DESKTOP, (LPPOINT)&rt, 2);
				NewPosX		= rt.left + (rt.right - rt.left - DlgWidth)/2;
				NewPosY		= rt.top + (rt.bottom - rt.top - DlgHeight)/2;
				
				// if the About box is larger than the physical screen 
				if (NewPosX < 0) NewPosX = 0;
				if (NewPosY < 0) NewPosY = 0;
				SetWindowPos(hDlg, 0, NewPosX, NewPosY,
					0, 0, SWP_NOZORDER | SWP_NOSIZE);
			}
			return TRUE;

		case WM_COMMAND:
			if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}


void DoMainLibraryInitialization(void)
{
	if(g_hLibrary != INVALID_HANDLE_VALUE)
	{
		if( (g_hLibrary = LoadLibrary(TEXT("API.DLL"))) != 0)
		{
			//-----------------------------------------------------------------------------
			// LED Frame Control API
			//-----------------------------------------------------------------------------
			pfnSetGreenLEDFrameValue = (PFNSetGreenLEDFrameValue) GetProcAddress(g_hLibrary, TEXT("SetGreenLEDFrameValue"));
			if(!pfnSetGreenLEDFrameValue) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnSetGreenLEDFrameValue\r\n")));
			pfnGetGreenLEDFrameValue = (PFNGetGreenLEDFrameValue) GetProcAddress(g_hLibrary, TEXT("GetGreenLEDFrameValue"));
			if(!pfnGetGreenLEDFrameValue) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnGetGreenLEDFrameValue\r\n")));

			pfnSetRedLEDFrameValue = (PFNSetRedLEDFrameValue) GetProcAddress(g_hLibrary, TEXT("SetRedLEDFrameValue"));
			if(!pfnSetRedLEDFrameValue) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnSetRedLEDFrameValue\r\n")));
			pfnGetRedLEDFrameValue = (PFNGetRedLEDFrameValue) GetProcAddress(g_hLibrary, TEXT("GetRedLEDFrameValue"));
			if(!pfnGetRedLEDFrameValue) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnGetRedLEDFrameValue\r\n")));

			pfnSetLEDFrameColor = (PFNSetLEDFrameColor) GetProcAddress(g_hLibrary, TEXT("SetLEDFrameColor"));
			if(!pfnSetLEDFrameColor) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnSetLEDFrameColor\r\n")));
			pfnGetLEDFrameColor = (PFNGetLEDFrameColor) GetProcAddress(g_hLibrary, TEXT("GetLEDFrameColor"));
			if(!pfnGetLEDFrameColor) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnGetLEDFrameColor\r\n")));

			pfnSetLEDStatus = (PFNSetLEDStatus) GetProcAddress(g_hLibrary, TEXT("SetLEDStatus"));
			if(!pfnSetLEDStatus) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnSetLEDStatus\r\n")));
			pfnGetLEDFrameStatus = (PFNGetLEDFrameStatus) GetProcAddress(g_hLibrary, TEXT("GetLEDFrameStatus"));
			if(!pfnGetLEDFrameStatus) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnGetLEDFrameStatus\r\n")));

			//-----------------------------------------------------------------------------
			// Text LED Control API+
			//-----------------------------------------------------------------------------
			pfnSetTextLEDStatus = (PFNSetTextLEDStatus) GetProcAddress(g_hLibrary, TEXT("SetTextLEDStatus"));
			if(!pfnSetTextLEDStatus) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnSetTextLEDStatus\r\n")));
			pfnGetTextLEDStatus = (PFNGetTextLEDStatus) GetProcAddress(g_hLibrary, TEXT("GetTextLEDStatus"));
			if(!pfnGetTextLEDStatus) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnGetTextLEDStatus\r\n")));

			//-----------------------------------------------------------------------------
			// Buzzer Control API
			//-----------------------------------------------------------------------------
			pfnSetBuzzerValue = (PFNSetBuzzerValue) GetProcAddress(g_hLibrary, TEXT("SetBuzzerValue"));
			if(!pfnSetBuzzerValue) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnSetBuzzerValue\r\n")));
			pfnGetBuzzerValue = (PFNGetBuzzerValue) GetProcAddress(g_hLibrary, TEXT("GetBuzzerValue"));
			if(!pfnGetBuzzerValue) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnGetBuzzerValue\r\n")));
			pfnBuzzerBeep = (PFNBuzzerBeep) GetProcAddress(g_hLibrary, TEXT("BuzzerBeep"));
			if(!pfnBuzzerBeep) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnBuzzerBeep\r\n")));

			//-----------------------------------------------------------------------------
			// Backlight Control API
			//-----------------------------------------------------------------------------
			pfnSetBacklightValue = (PFNSetBacklightValue) GetProcAddress(g_hLibrary, TEXT("SetBacklightValue"));
			if(!pfnSetBacklightValue) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnSetBacklightValue\r\n")));
			pfnGetBacklightValue = (PFNGetBacklightValue) GetProcAddress(g_hLibrary, TEXT("GetBacklightValue"));
			if(!pfnGetBacklightValue) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnGetBacklightValue\r\n")));
			pfnSetBacklightStatus = (PFNSetBacklightStatus) GetProcAddress(g_hLibrary, TEXT("SetBacklightStatus"));
			if(!pfnSetBacklightStatus) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnSetBacklightStatus\r\n")));
			pfnGetBacklightStatus = (PFNGetBacklightStatus) GetProcAddress(g_hLibrary, TEXT("GetBacklightStatus"));
			if(!pfnGetBacklightStatus) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnGetBacklightStatus\r\n")));

			//-----------------------------------------------------------------------------
			// Updating OS Control API
			//-----------------------------------------------------------------------------
			pfnStartUpgrade = (PFNStartUpgrade) GetProcAddress(g_hLibrary, TEXT("StartUpgrade"));
			if(!pfnStartUpgrade) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnStartUpgrade\r\n")));
			pfnGetUpgradeProgress = (PFNGetUpgradeProgress) GetProcAddress(g_hLibrary, TEXT("GetUpgradeProgress"));
			if(!pfnGetUpgradeProgress) RETAILMSG(TRUE, (TEXT("Can't get library function pointer. pfnGetUpgradeProgress\r\n")));
		}
		else
		{
			MessageBox(NULL, TEXT("Unable to Load Customed Library"), TEXT("Error"), MB_OK|MB_ICONERROR);
		}
	}
}


//----------------------------------------------------------------------
// DoMainCommandSTART - Process WM_Command IDC_MAIN_BUTTON_START Command for window.
//----------------------------------------------------------------------
LPARAM DoMainCommandSTART(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	int i = 0;
	DWORD	dwTStat;

	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		if( BST_CHECKED == SendMessage( GetDlgItem(hWnd, ((int)MainCommonControls[i].hMenu)), BM_GETCHECK, NULL, NULL) )
		{
			DoMainLibraryInitialization();
			if( NULL == MainCommonControls[i].hThreadHandle )
			{
				if( NULL == (MainCommonControls[i].hThreadHandle = (HWND__ *)CreateThread(NULL, 0, MainCommonControls[i].lpThreadStartAddr, &i, 0, &dwTStat)) )
				{
					TCHAR	szDbg[MAX_PATH];
					wsprintf(szDbg, TEXT("Error on creating thread : %s\r\n"), MainCommonControls[i].lpWindowName);
					MessageBox (hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
					PostMessage(hWnd, WM_CLOSE, 0, 0);
				}
				else
				{
					if(!MainCommonControls[i].hEvent)
					{
						MainCommonControls[i].hEvent = CreateEvent(NULL, FALSE,	FALSE, MainCommonControls[i].lpWindowName);
					}
					SetEvent(MainCommonControls[i].hEvent);
				}
			}
			else
			{
					SetEvent(MainCommonControls[i].hEvent);
			}
		}
	}

	if( NULL == g_Main_hTimeThread )
	{
		if( NULL == (g_Main_hTimeThread = (HWND__ *)CreateThread(NULL, 0, DoMainCommandThreadTime, &i, 0, &dwTStat)) )
		{
			TCHAR	szDbg[MAX_PATH];
			wsprintf(szDbg, TEXT("Error on creating thread : %s\r\n"), TEXT("TIME"));
			MessageBox (hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		else
		{
			if(!g_Main_hTimeEvent)
			{
				g_Main_hTimeEvent = CreateEvent(NULL, FALSE,	FALSE, TEXT("TIME"));
			}
			SetEvent(g_Main_hTimeEvent);
		}
	}
	else
	{
			SetEvent(g_Main_hTimeEvent);
	}

	g_dwTimeStart = GetTickCount();

	return FALSE;
}


LPARAM	DoMainCommandALL(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	int i;
	
	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		PostMessage(GetDlgItem(hWnd, ((int)MainCommonControls[i].hMenu)),  BM_SETCHECK, (WPARAM) BST_CHECKED, NULL);
	}
	return FALSE;
}


LPARAM	DoMainCommandNONE(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	int i;
	
	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		PostMessage(GetDlgItem(hWnd, ((int)MainCommonControls[i].hMenu)),  BM_SETCHECK, (WPARAM) BST_UNCHECKED, NULL);
	}
	return FALSE;
}


DWORD WINAPI DoMainCommandThreadLCD (PVOID pArg)
{
	int i;
	DLGTEMPLATE	DlgTemplate = { /*DS_MODALFRAME | DS_SETFONT | DS_SETFOREGROUND |*/ WS_POPUP | WS_OVERLAPPED | WS_CHILD, NULL, 0, 0,0,0,0};

	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		if(0 == _tcscmp(MainCommonControls[i].lpWindowName, TEXT("LCD")))
		{
			break;
		}
	}

	WaitForSingleObject(MainCommonControls[i].hEvent, INFINITE);

	if( NULL == (DialogBoxIndirectParam(g_Main_hInst, &DlgTemplate, g_Main_hWnd, (DLGPROC)DoMainCommandThreadLCDDialog, i)) )
	{
		TCHAR	szDbg[MAX_PATH];
		wsprintf(szDbg, TEXT("Error on creating Dialog : %s\r\n"), MainCommonControls[i].lpWindowName);
		MessageBox (g_Main_hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	CloseHandle(MainCommonControls[i].hThreadHandle);
	MainCommonControls[i].hThreadHandle = NULL;

	return(TRUE);
}

DWORD WINAPI DoMainCommandThreadBacklight (PVOID pArg)
{
	int		i;
	DLGTEMPLATE	DlgTemplate = { /*DS_MODALFRAME | DS_SETFONT | DS_SETFOREGROUND |*/ WS_POPUP | WS_OVERLAPPED | WS_CHILD, NULL, 0, 0,0,0,0};

	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		if(0 == _tcscmp(MainCommonControls[i].lpWindowName, TEXT("BACKLIGHT")))
		{
			break;
		}
	}

	WaitForSingleObject(MainCommonControls[i].hEvent, INFINITE);

	if( NULL == (DialogBoxIndirectParam(g_Main_hInst, &DlgTemplate, g_Main_hWnd, (DLGPROC)DoMainCommandThreadBacklightDialog, i)) )
	{
		TCHAR	szDbg[MAX_PATH];
		wsprintf(szDbg, TEXT("Error on creating Dialog : %s\r\n"), MainCommonControls[i].lpWindowName);
		MessageBox (g_Main_hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	CloseHandle(MainCommonControls[i].hThreadHandle);
	MainCommonControls[i].hThreadHandle = NULL;

	return(TRUE);
}


DWORD WINAPI DoMainCommandThreadFrameLED (PVOID pArg)
{
	int		i;
	DLGTEMPLATE	DlgTemplate = {WS_POPUP | WS_OVERLAPPED | WS_CHILD, NULL, 0, 0,0,0,0};

	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		if(0 == _tcscmp(MainCommonControls[i].lpWindowName, TEXT("FRAME LED")))
		{
			break;
		}
	}

	WaitForSingleObject(MainCommonControls[i].hEvent, INFINITE);
	g_dwFrameLEDColor = GREEN;
	g_dwRedFrameLEDLevel = 0;
	g_dwGreenFrameLEDLevel = 0;
	pfnSetGreenLEDFrameValue(0);
	Sleep(200);
	pfnSetRedLEDFrameValue(0);
	Sleep(200);
	pfnSetLEDFrameColor(GREEN);
	Sleep(200);
	pfnSetLEDStatus(ON);
	Sleep(200);
	g_FrameLED_Counter = 0;

	if( NULL == (DialogBoxIndirectParam(g_Main_hInst, &DlgTemplate, g_Main_hWnd, (DLGPROC)DoMainCommandThreadFrameLEDDialog, i)) )
	{
		TCHAR	szDbg[MAX_PATH];
		wsprintf(szDbg, TEXT("Error on creating Dialog : %s\r\n"), MainCommonControls[i].lpWindowName);
		MessageBox (g_Main_hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	CloseHandle(MainCommonControls[i].hThreadHandle);
	MainCommonControls[i].hThreadHandle = NULL;

	return(TRUE);
}


DWORD WINAPI DoMainCommandThreadTextLED (PVOID pArg)
{
	int		i;
	DLGTEMPLATE	DlgTemplate = {WS_POPUP | WS_OVERLAPPED | WS_CHILD, NULL, 0, 0,0,0,0};

	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		if(0 == _tcscmp(MainCommonControls[i].lpWindowName, TEXT("TEXT LED")))
		{
			break;
		}
	}

	WaitForSingleObject(MainCommonControls[i].hEvent, INFINITE);

	if( NULL == (DialogBoxIndirectParam(g_Main_hInst, &DlgTemplate, g_Main_hWnd, (DLGPROC)DoMainCommandThreadTextLEDDialog, i)) )
	{
		TCHAR	szDbg[MAX_PATH];
		wsprintf(szDbg, TEXT("Error on creating Dialog : %s\r\n"), MainCommonControls[i].lpWindowName);
		MessageBox (g_Main_hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	CloseHandle(MainCommonControls[i].hThreadHandle);
	MainCommonControls[i].hThreadHandle = NULL;

	return(TRUE);
}


DWORD WINAPI DoMainCommandThreadBuzzer (PVOID pArg)
{
	int		i;
	DLGTEMPLATE	DlgTemplate = {WS_POPUP | WS_OVERLAPPED | WS_CHILD, NULL, 0, 0,0,0,0};

	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		if(0 == _tcscmp(MainCommonControls[i].lpWindowName, TEXT("BUZZER")))
		{
			break;
		}
	}

	WaitForSingleObject(MainCommonControls[i].hEvent, INFINITE);

	if( NULL == (DialogBoxIndirectParam(g_Main_hInst, &DlgTemplate, g_Main_hWnd, (DLGPROC)DoMainCommandThreadBuzzerDialog, i)) )
	{
		TCHAR	szDbg[MAX_PATH];
		wsprintf(szDbg, TEXT("Error on creating Dialog : %s\r\n"), MainCommonControls[i].lpWindowName);
		MessageBox (g_Main_hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	CloseHandle(MainCommonControls[i].hThreadHandle);
	MainCommonControls[i].hThreadHandle = NULL;
	
	return(TRUE);
}

/*DWORD WINAPI DoMainCommandThreadLAN (PVOID pArg)
{
	int i;
	DLGTEMPLATE	DlgTemplate = {WS_POPUP | WS_OVERLAPPED | WS_CHILD, NULL, 0, 0,0,0,0};

	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		if(0 == _tcscmp(MainCommonControls[i].lpWindowName, TEXT("BUZZER")))
		{
			break;
		}
	}

	WaitForSingleObject( MainCommonControls[i].hEvent, INFINITE);

	if( NULL == (DialogBoxIndirectParam(g_Main_hInst, &DlgTemplate, g_Main_hWnd, (DLGPROC)DoMainCommandThreadLANDialog, i)) )
	{
		TCHAR	szDbg[MAX_PATH];
		wsprintf(szDbg, TEXT("Error on creating Dialog : %s\r\n"), MainCommonControls[i].lpWindowName);
		MessageBox (g_Main_hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	CloseHandle(MainCommonControls[i].hThreadHandle);
	MainCommonControls[i].hThreadHandle = NULL;

	return(TRUE);
}*/


DWORD WINAPI DoMainCommandThreadMemory (PVOID pArg)
{
	int		i;
	DLGTEMPLATE	DlgTemplate = {WS_POPUP | WS_OVERLAPPED | WS_CHILD | WS_SYSMENU, NULL, 0, 0,0,0,0};

	for(i = 0; i < dim(MainCommonControls); ++i)
	{
		if(0 == _tcscmp(MainCommonControls[i].lpWindowName, TEXT("MEMORY")))
		{
			break;
		}
	}

	WaitForSingleObject(MainCommonControls[i].hEvent, INFINITE);
	g_bContinue = TRUE;

	/*if( NULL == (DialogBoxIndirectParam(g_Main_hInst, &DlgTemplate, g_Main_hWnd, (DLGPROC)DoMainCommandThreadMemoryDialog, i)) )
	{
			TCHAR	szDbg[MAX_PATH];
			wsprintf(szDbg, TEXT("Error on creating Dialog : %s\r\n"), MainCommonControls[i].lpWindowName);
			MessageBox (g_Main_hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
	}*/
	DialogBoxIndirectParam(g_Main_hInst, &DlgTemplate, g_Main_hWnd, (DLGPROC)DoMainCommandThreadMemoryDialog, i);
	CloseHandle(MainCommonControls[i].hThreadHandle);
	MainCommonControls[i].hThreadHandle = NULL;

	return(TRUE);
}


DWORD WINAPI DoMainCommandThreadTime (PVOID pArg)
{
	DLGTEMPLATE	DlgTemplate = { /*DS_MODALFRAME | DS_SETFONT | DS_SETFOREGROUND |*/ WS_POPUP | WS_OVERLAPPED | WS_CHILD, NULL, 0, 0,0,0,0};

	WaitForSingleObject(g_Main_hTimeEvent, INFINITE);

	if( NULL == (DialogBoxIndirectParam(g_Main_hInst, &DlgTemplate, g_Main_hWnd, (DLGPROC)DoMainCommandThreadTimeDialog, NULL)) )
	{
		TCHAR	szDbg[MAX_PATH];
		wsprintf(szDbg, TEXT("Error on creating Dialog : %s\r\n"), TEXT("Time"));
		MessageBox (g_Main_hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	CloseHandle(g_Main_hTimeThread);
	g_Main_hTimeThread = NULL;

	return(TRUE);
}