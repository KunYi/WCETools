#include "stdafx.h"
#include "HelloWorld.h"
#include <commctrl.h>
#include "api.h"

//-----------------------------------------------------------------------------
// Backlight Control API
//-----------------------------------------------------------------------------
extern PFNSetBacklightValue pfnSetBacklightValue;
extern PFNGetBacklightValue pfnGetBacklightValue;
extern PFNSetBacklightStatus pfnSetBacklightStatus;
extern PFNGetBacklightStatus pfnGetBacklightStatus;

// Mesage handler for the Backlight box.
LRESULT CALLBACK DoMainCommandThreadBacklightDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT	rt, rt1;
	int		DlgWidth, DlgHeight;	// dialog width and height in pixel units
	int		NewPosX, NewPosY;
	static int		i = 1;
	DWORD	dwBacklightLevel;

	dwBacklightLevel = 0;

	switch (message)
	{
		case WM_INITDIALOG:
			// trying to center the dialog
			if (GetWindowRect(hDlg, &rt1))
			{
				GetClientRect(GetParent(hDlg), &rt);

				DlgWidth	= GetSystemMetrics(SM_CXSCREEN)/2;
				DlgHeight	= GetSystemMetrics(SM_CYSCREEN)/4;

				MapWindowPoints(GetParent(hDlg), HWND_DESKTOP, (LPPOINT)&rt, 2);
				NewPosX		= rt.left;
				NewPosY		= rt.top;

				// if the dialog box is larger than the physical screen 
				if (NewPosX < 0) NewPosX = 0;
				if (NewPosY < 0) NewPosY = 0;
				SetWindowPos(hDlg, 0, 0, DlgHeight,	DlgWidth, DlgHeight, SWP_NOZORDER);
				SetWindowText(hDlg, TEXT("BurnIn Test - Backlight Test"));
				SetTimer(hDlg, ((int)lParam + 1), 200, NULL);
			}
			return TRUE;

		case WM_TIMER:
			{
				dwBacklightLevel = pfnGetBacklightValue();
				if(i > 0)
				{
					if(dwBacklightLevel < 100)
					{
						pfnSetBacklightValue(++dwBacklightLevel);
					}
					else
					{
						pfnSetBacklightValue(--dwBacklightLevel);
						i = -1;
					}
				}
				else
				{
					if(dwBacklightLevel > 0)
					{
						pfnSetBacklightValue(--dwBacklightLevel);
					}
					else
					{
						pfnSetBacklightValue(++dwBacklightLevel);
						i = 1;
					}
				}

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

				hdc = BeginPaint (hDlg, &ps);

				GetClientRect(hDlg, &rt);
				wsprintf(szDbg, TEXT("Click to terminate backlight burnIn test."));
				DrawText(hdc, szDbg, _tcslen(szDbg), &rt, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

				EndPaint(hDlg, &ps);
			}
			break;
	}
    return DefWindowProc (hDlg, message, wParam, lParam);
}
