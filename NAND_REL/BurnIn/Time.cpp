#include "stdafx.h"
#include "HelloWorld.h"
#include <commctrl.h>
#include "api.h"

extern volatile DWORD		g_dwTimeStart, g_dwTimeElapsed;

// Mesage handler for the Time box.
LRESULT CALLBACK DoMainCommandThreadTimeDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rt, rt1;//, rectCommandBar;
	int DlgWidth, DlgHeight;	// dialog width and height in pixel units
	int NewPosX, NewPosY;

	//LONG			lCommandBarHeigth;

	switch (message)
	{
		case WM_INITDIALOG:
			// trying to center the About dialog
			if (GetWindowRect(hDlg, &rt1))
			{
				GetClientRect(GetParent(hDlg), &rt);
				//GetWindowRect(g_Main_hWndCB, &rectCommandBar);

				DlgWidth	= GetSystemMetrics(SM_CXSCREEN)/2;
				DlgHeight	= GetSystemMetrics(SM_CYSCREEN)/4;
				//lCommandBarHeigth = (rectCommandBar.bottom - rectCommandBar.top);

				MapWindowPoints(GetParent(hDlg), HWND_DESKTOP, (LPPOINT)&rt, 2);
				NewPosX		= rt.left;
				NewPosY		= rt.top;

				// if the About box is larger than the physical screen 
				if (NewPosX < 0) NewPosX = 0;
				if (NewPosY < 0) NewPosY = 0;
				SetWindowPos(hDlg, 0, DlgWidth, DlgHeight*2,	DlgWidth, DlgHeight, SWP_NOZORDER);
				SetWindowText(hDlg, TEXT("BurnIn Test - Time Elapsed"));
				SetTimer(hDlg, ((int)8 + 1), 1000, NULL);
			}
			return TRUE;

		case WM_TIMER:
			{
				g_dwTimeElapsed = GetTickCount() - g_dwTimeStart;

				//update window
				InvalidateRect(hDlg, NULL, TRUE);
				UpdateWindow(hDlg);
			}
			break;

		case WM_LBUTTONDOWN:
			EndDialog(hDlg, LOWORD(wParam));
			break;

		case WM_CLOSE:
			{
				KillTimer(hDlg, ((int)lParam + 1));
				ExitThread(((int)lParam));
			}
			break;

		case WM_PAINT:
			{
				HDC				hdc;
				PAINTSTRUCT		ps;

				RECT rt;
				TCHAR szDbg[MAX_PATH];
				DWORD	dwDay, dwHour, dwMinute, dwSecond;

				hdc = BeginPaint (hDlg, &ps);

				GetClientRect(hDlg, &rt);

				dwDay = 0;
				if(g_dwTimeElapsed > (86400 * 1000))
				{
					dwDay = (DWORD)(g_dwTimeElapsed / (86400 * 1000));
					g_dwTimeElapsed -= dwDay * 86400 * 1000;
				}
				dwHour = 0;
				if(g_dwTimeElapsed > (3600 * 1000))
				{
					dwHour = (DWORD)(g_dwTimeElapsed / (3600 * 1000));
					g_dwTimeElapsed -= dwHour * 3600 * 1000;
				}
				dwMinute = 0;
				if(g_dwTimeElapsed > (60 * 1000))
				{
					dwMinute = (DWORD)(g_dwTimeElapsed / (60 * 1000));
					g_dwTimeElapsed -= dwMinute * 60 * 1000;
				}
				dwSecond = 0;
				if(g_dwTimeElapsed > (1 * 1000))
				{
					dwSecond = (DWORD)(g_dwTimeElapsed / (1 * 1000));
					g_dwTimeElapsed -= dwSecond * 1 * 1000;
				}


				wsprintf(szDbg, TEXT("%d Day %d Hour %d Minute %d Second"), dwDay, dwHour, dwMinute, dwSecond);

				DrawText(hdc, szDbg, _tcslen(szDbg), &rt, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

				EndPaint(hDlg, &ps);
			}
			break;
	}
    return DefWindowProc (hDlg, message, wParam, lParam);
}
