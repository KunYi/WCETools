// ForReboot.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

HINSTANCE hInst;

void SystemReset(void)
{
	//KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);
	SetSystemPowerState(NULL, POWER_STATE_RESET, POWER_FORCE);
}

#define KEY_DEFVALUE      (0)
#define COUNTDOWN_SEC	  (3)

TCHAR gKEY_AAEON_REBOOT[] = TEXT("Software\\SYSTESTER\\Reboot");
TCHAR gKEY_VALUENAME[] = TEXT("Counter");

void setDefaultValue(HKEY hKey)
{
	DWORD val = KEY_DEFVALUE;
	LONG rc;
	rc = RegSetValueEx(
		hKey,
		gKEY_VALUENAME,
		0,
		REG_DWORD,
		(LPBYTE)&val,
		sizeof(DWORD));
	if (ERROR_SUCCESS != rc)
		MessageBox(NULL, TEXT("Failed: setDefaultValue"), TEXT("WARRING"), MB_OK);
}

ULONG GetCounter(void)
{
	LONG rc;
	HKEY hKey;
	DWORD dwDisposition;
	ULONG val;
	DWORD valSize;

	rc = RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			gKEY_AAEON_REBOOT,
			0,
			NULL,
			0,
			0,
			NULL,
			&hKey,
			&dwDisposition);
	
	if (ERROR_SUCCESS == rc)
	{
		if (REG_CREATED_NEW_KEY == dwDisposition)
			setDefaultValue(hKey);

		rc = RegQueryValueEx(
			hKey,
			gKEY_VALUENAME,
			0,
			NULL,
			(LPBYTE)&val,
			&valSize
			);
		
		RegFlushKey(hKey);
		RegCloseKey(hKey);

		if (ERROR_SUCCESS == rc)
			return val;
		else
			MessageBox(NULL, TEXT("Failed: RegQueryValue"), TEXT("WARRING"), MB_OK);
	}
    else {
		MessageBox(NULL, TEXT("Failed: RegCreateKeyEx() in GetCounter()"), TEXT("WARRING"), MB_OK);
	}
	return 0xFFFFFFFF;
}

void SetCounter(ULONG val)
{
	LONG rc;
	HKEY hKey;
	DWORD dwDisposition;

	rc = RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			gKEY_AAEON_REBOOT,
			0,
			NULL,
			0,
			0,
			NULL,
			&hKey,
			&dwDisposition);

	if (ERROR_SUCCESS == rc)
	{
		RegSetValueEx(
			hKey,
			gKEY_VALUENAME,
			0,
			REG_DWORD,
			(const BYTE*)&val,
			sizeof(val));
		RegFlushKey(hKey);
		RegCloseKey(hKey);
	} 
	else {
		MessageBox(NULL, TEXT("Failed: RegCreateKeyEx() in SetCounter()"), TEXT("WARRING"), MB_OK);
	}
}

BOOL CALLBACK InitDlgProc(HWND hDlgWnd,
					   UINT Message,
					   WPARAM wParam,
					   LPARAM lParam)
{
	static ULONG Count = 0;
	switch(Message)
	{
		case WM_INITDIALOG:
		{
			RECT Desk;
			RECT Dialog;
			GetWindowRect(GetDesktopWindow(), &Desk);
			GetWindowRect(hDlgWnd, &Dialog);
			SetWindowPos(hDlgWnd, HWND_TOP, 
				(Desk.right - Dialog.right) / 2,
				(Desk.bottom - Dialog.bottom) / 2,
				0,
				0,
				SWP_NOSIZE);

			ULONG time = GetCounter();
			TCHAR BUF[256];
			_stprintf(BUF, TEXT("reboot time:%d\n"), time);
			SetDlgItemText(hDlgWnd, IDC_STATIC1, BUF);
			SetCounter(time+1);
			Count = 0;
			SetTimer(hDlgWnd, 100, 1000, NULL); // Timer ID equ 100
			_stprintf(BUF, TEXT("Countdown:%d sec, to reboot\n"), (COUNTDOWN_SEC-Count));
			SetDlgItemText(hDlgWnd, IDC_STATIC2, BUF);
		}
		break;
		case WM_COMMAND:
		{
			if(LOWORD(wParam) == IDOK) {
				KillTimer(hDlgWnd, 100);
				DestroyWindow(hDlgWnd);
			}
		}
		break;
		case WM_TIMER:
		{
			if(wParam == 100) // Timer ID
			{
				TCHAR BUF[256];
				Count++;
				_stprintf(BUF, TEXT("Countdown:%d sec, to reboot\n"), COUNTDOWN_SEC-Count);
				SetDlgItemText(hDlgWnd, IDC_STATIC2, BUF);
				if (Count >= COUNTDOWN_SEC)
					SystemReset();

			}
		}
		break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;
	}

	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
	HWND hWnd = NULL;
	MSG  Msg;

	hInst = hInstance;
	
	WaitForAPIReady( SH_SHELL, 5000);

	hWnd = CreateDialog( hInstance,
						(LPCTSTR)MAKEINTRESOURCE(IDD_DIALOG1),
						NULL,
						(DLGPROC)InitDlgProc);
	ShowWindow(hWnd, SW_SHOW);
	SignalStarted(_ttol(lpCmdLine));

	while(GetMessage(&Msg, NULL, 0, 0) == TRUE)
	{
		if (!IsDialogMessage(hWnd, &Msg))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}
	return Msg.wParam;
}
