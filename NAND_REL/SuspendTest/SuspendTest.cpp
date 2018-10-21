#include <windows.h>                 // For all that Windows stuff
#include <Pkfuncs.h>
#include <pm.h>
#include "SuspendTest.h"                 // Program-specific stuff
#include "resource.h"

#define QUEUE_ENTRIES   3
#define MAX_NAMELEN     200
#define QUEUE_SIZE      (QUEUE_ENTRIES * (sizeof(POWER_BROADCAST) + MAX_NAMELEN))


//----------------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = TEXT ("SuspendTest");

// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_INITDIALOG, DoInitDlgMain,
    WM_COMMAND, DoCommandMain,
	WM_TIMER, DoTimer
};
// Command Message dispatch for MainWindowProc
const struct decodeCMD MainCommandItems[] = {
	IDC_BUTTON1, DoStartStop,
    IDOK, DoMainCommandExit,
    IDCANCEL, DoMainCommandExit,
};

#if 0
HANDLE ghPowerNotifications;
HANDLE ghNotifications;
#endif

static HANDLE ghFile;
static BOOL	gbStart;
static PFNSetAlarmTime pfnSetAlarmTime;
static const TCHAR logfilename[] = _T("\\ResidentFlash\\SuspendTest.log");
static int gCounter = 0;

#if 0
static int CreateNotifications(void)
{
	MSGQUEUEOPTIONS msgOptions = {0};
	
    // create a message queue for Power Manager notifications
    msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    msgOptions.dwFlags = 0;
    msgOptions.dwMaxMessages = QUEUE_ENTRIES;
    msgOptions.cbMaxMessage = sizeof(POWER_BROADCAST) + MAX_NAMELEN;
    msgOptions.bReadAccess = TRUE;
	
	ghNotifications = CreateMsgQueue(NULL, &msgOptions);
	
	if (NULL == ghNotifications) {
		DWORD errcode = GetLastError();
		RETAILMSG(1, (_T("Failed: CreateMsgQueue() Error 0x%X(%d)\r\n"), errcode, errcode));
		return -1;
	}
	
	ghPowerNotifications = RequestPowerNotifications(ghNotifications, POWER_NOTIFY_ALL);
	if (NULL == ghPowerNotifications) {
		DWORD errcode = GetLastError();
		RETAILMSG(1, (TEXT("Failed: RequestPowerNotifications() Error 0x%X(%d)\r\rn"), errcode, errcode));
		return -2;
	}
	return 0;
}

static int StopNotifications(void) {
	if (NULL != ghPowerNotifications) {
		StopPowerNotifications(ghPowerNotifications);
		ghPowerNotifications = NULL;
	}
	
	if (NULL != ghNotifications) {
		CloseMsgQueue(ghNotifications);
		ghNotifications = NULL;
	}
	
	return 0;
}
#endif

//======================================================================
// Program entry point
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	HMODULE hApiDll;
	#if 0
	ghNotifications = NULL;
	ghPowerNotifications = NULL;
	#endif
	gbStart = FALSE;
	gCounter = 0;
	
	hApiDll = LoadLibrary(TEXT("API.DLL"));
    
	if (NULL == hApiDll) {
		RETAILMSG(TRUE, (TEXT("Failed: Load API.DLL Error\r\n")));
		MessageBox(NULL, TEXT("Failed: Load API.DLL Error\r\n"), TEXT("ERROR!"), MB_OK | MB_ICONERROR);
		return -1;
    }
    pfnSetAlarmTime = (PFNSetAlarmTime) GetProcAddress(hApiDll, TEXT("SetAlarmTime"));	
	
	if (NULL == pfnSetAlarmTime) {
		RETAILMSG(TRUE, (TEXT("Failed: Can't find SetAlarmTime() in API.DLL\r\n")));
		MessageBox(NULL, TEXT("Failed: Can't find SetAlarmTime() in API.DLL\r\n"), TEXT("ERROR!"), MB_OK | MB_ICONERROR);
		return -2;
	}
	ghFile = CreateFile(logfilename, GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (ghFile == INVALID_HANDLE_VALUE) {
		RETAILMSG(TRUE, (TEXT("Failed: Create log file Error\r\n")));
		MessageBox(NULL, TEXT("Failed: Create log file Error\r\n"), TEXT("ERROR!"), MB_OK | MB_ICONERROR);
		return -3;
	}
	
    // Display dialog box as main window.
    DialogBoxParam (hInstance, _T("MAINDLG"), NULL, MainDlgProc, 0);
	
	FreeLibrary(hApiDll);
	if (INVALID_HANDLE_VALUE != ghFile) {
		CloseHandle(ghFile);
		ghFile = INVALID_HANDLE_VALUE;
	}
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
    //SetDlgItemText(hWnd, IDC_STATIC1, szUUID);
	SetDlgItemText(hWnd, IDC_STATIC2, TEXT("Press \"Start\" Button"));
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

BOOL DoTimer(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) 
{
	static int slptime = 0;
	TCHAR buff[128];

	if (gCounter == 0 && 0 != slptime ) {
		SYSTEMTIME st;
		char obuff[128];
		DWORD dwBytesWriten;
		
		GetLocalTime(&st);
		sprintf(obuff, "Resume at: %02d/%02d, %04d, %02d:%02d:%02d\r\n",
					st.wMonth, st.wDay, st.wYear, 
					st.wHour, st.wMinute, st.wSecond);
		WriteFile(ghFile, obuff, strlen(obuff), &dwBytesWriten, NULL);
	}

	wsprintf(buff, TEXT("Sleep in: %ds\r\n"), 28-gCounter);	
	SetDlgItemText(hWnd, IDC_STATIC2, buff);
	RETAILMSG(TRUE, (buff));

	gCounter++;

	if ( gCounter >= 29) {
		SYSTEMTIME st;
		FILETIME ft;
		ULARGE_INTEGER	t;
		char obuff[128];
		DWORD dwBytesWriten;
		
		// Get current system
		GetLocalTime(&st);
		SystemTimeToFileTime(&st, &ft);
		memcpy(&t, &ft, sizeof(FILETIME));
		t.QuadPart += 30I64*10000000I64; // add 30sec
		memcpy(&ft, &t, sizeof(FILETIME));
		FileTimeToSystemTime(&ft, &st);
		pfnSetAlarmTime(&st);
		sprintf(obuff, "SetAlarmTime at: %02d/%02d, %04d, %02d:%02d:%02d\r\n",
					st.wMonth, st.wDay, st.wYear, 
					st.wHour, st.wMinute, st.wSecond);
		WriteFile(ghFile, obuff, strlen(obuff), &dwBytesWriten, NULL);
		
		gCounter = 0;

		slptime++;
		
		GetLocalTime(&st);
		sprintf(obuff, "Sleep at: %02d/%02d, %04d, %02d:%02d:%02d\r\n",
					st.wMonth, st.wDay, st.wYear, 
					st.wHour, st.wMinute, st.wSecond);
		WriteFile(ghFile, obuff, strlen(obuff), &dwBytesWriten, NULL);
		sprintf(obuff, "*** Sleep counter: %d ***\r\n", slptime);
		WriteFile(ghFile, obuff, strlen(obuff), &dwBytesWriten, NULL);
		wsprintf(buff, TEXT("Sleep counter: %d\r\n"), slptime);
		// FlushFileBuffers(ghFile);
		
		RETAILMSG(TRUE, (buff));
		Sleep(200);		// wait Debug/Log message
		SetDlgItemText(hWnd, IDC_STATIC1, buff);

		GwesPowerOffSystem();		// System into suspend mode
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
	
	if (TRUE == gbStart) {
		KillTimer(hWnd, 1);
	}
	
    EndDialog (hWnd, 0);
    return 0;
}

LPARAM DoStartStop (HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	static BOOL  bFirst = FALSE;
	
	gbStart = (gbStart == FALSE) ? TRUE:FALSE;
	
	if (TRUE == gbStart)
	{
		SYSTEMTIME st;
		char obuff[128];
		DWORD dwBytesWriten;
		
		SetDlgItemText(hWnd, IDC_BUTTON1, TEXT("Stop"));
		SetTimer(hWnd, 1, 1000, NULL);
		SetDlgItemText(hWnd, IDC_STATIC2, TEXT("Sleep in: 29s\r\n"));
		SetDlgItemText(hWnd, IDC_STATIC1, TEXT("Sleep counter: 0"));
		sprintf(obuff, "==========================================\r\n");
		WriteFile(ghFile, obuff, strlen(obuff), &dwBytesWriten, NULL);
		GetLocalTime(&st);
		sprintf(obuff, "Start Time: %02d/%02d, %04d, %02d:%02d:%02d\r\n",
					st.wMonth, st.wDay, st.wYear, 
					st.wHour, st.wMinute, st.wSecond);
		WriteFile(ghFile, obuff, strlen(obuff), &dwBytesWriten, NULL);
		sprintf(obuff, "==========================================\r\n");
		WriteFile(ghFile, obuff, strlen(obuff), &dwBytesWriten, NULL);
 	}
	else
	{
		SYSTEMTIME st;
		char obuff[128];
		DWORD dwBytesWriten;

		SetDlgItemText(hWnd, IDC_BUTTON1, TEXT("Start"));
		KillTimer(hWnd, 1);
		gCounter = 0;

		sprintf(obuff, "==========================================\r\n");
		WriteFile(ghFile, obuff, strlen(obuff), &dwBytesWriten, NULL);
		GetLocalTime(&st);
		sprintf(obuff, "Stop Time: %02d/%02d, %04d, %02d:%02d:%02d\r\n",
					st.wMonth, st.wDay, st.wYear, 
					st.wHour, st.wMinute, st.wSecond);
		WriteFile(ghFile, obuff, strlen(obuff), &dwBytesWriten, NULL);
		sprintf(obuff, "==========================================\r\n");
		WriteFile(ghFile, obuff, strlen(obuff), &dwBytesWriten, NULL);
	}
	return FALSE;
}
