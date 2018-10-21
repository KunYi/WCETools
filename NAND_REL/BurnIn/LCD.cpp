#include "stdafx.h"
#include "HelloWorld.h"
#include <commctrl.h>
#include "api.h"


// Mesage handler for the LCD box.
LRESULT CALLBACK DoMainCommandThreadLCDDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rt, rt1;//, rectCommandBar;
	int DlgWidth, DlgHeight;	// dialog width and height in pixel units
	int NewPosX, NewPosY;

	//LONG			lCommandBarHeigth;
	static DWORD	g_dwLCDTestLoop = 0;

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
				SetWindowPos(hDlg, 0, 0, 0,	DlgWidth, DlgHeight, SWP_NOZORDER);
				SetWindowText(hDlg, TEXT("BurnIn Test - Video Memory Test"));
				SetTimer(hDlg, ((int)lParam + 1), 200, NULL);
			}
			return TRUE;

		case WM_TIMER:
			{
				if (g_dwLCDTestLoop++ == 33)
			        g_dwLCDTestLoop = 0;

				//update window
				InvalidateRect(hDlg, NULL, TRUE);
				UpdateWindow(hDlg);
			}
			break;

		case WM_LBUTTONDOWN:
			EndDialog(hDlg, LOWORD(wParam));
			break;

		case WM_PAINT:
			{
				HDC				hdc;
				PAINTSTRUCT		ps;

				RECT rectCli;
				HBRUSH hBr, hOldBr;

				DWORD ColorTable[] =
				{
					RGB(255,255,255),
					RGB(255,0,0),
					RGB(0,255,0),
					RGB(0,0,255),
					RGB(0,255,255),
					RGB(255,0,255),
					RGB(255,255,0),
					RGB(0,0,0),
				};

				COLOR16 RGBTable[][8] =
				{
					{0xff00,0xff00,0xff00,0x0000,0x0000,0x0000,0x0000,0x0000},
					{0x0000,0x0000,0x0000,0x0000,0xff00,0xff00,0xff00,0x0000},
					//{0xff00,0xff00,0xff00,0x0000,0xff00,0xff00,0xff00,0x0000},
					//{0xff00,0xff00,0xff00,0x0000,0xff00,0xff00,0xff00,0x0000},

					{0xff00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
					{0x0000,0x0000,0x0000,0x0000,0xff00,0x0000,0x0000,0x0000},
					{0xff00,0x0000,0x0000,0x0000,0xff00,0xff00,0xff00,0x0000},
					{0xff00,0xff00,0xff00,0x0000,0xff00,0x0000,0x0000,0x0000},

					{0x0000,0xff00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
					{0x0000,0x0000,0x0000,0x0000,0x0000,0xff00,0x0000,0x0000},
					{0x0000,0xff00,0x0000,0x0000,0xff00,0xff00,0xff00,0x0000},
					{0xff00,0xff00,0xff00,0x0000,0x0000,0xff00,0x0000,0x0000},

					{0x0000,0x0000,0xff00,0x0000,0x0000,0x0000,0x0000,0x0000},
					{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0xff00,0x0000},
					{0x0000,0x0000,0xff00,0x0000,0xff00,0xff00,0xff00,0x0000},
					{0xff00,0xff00,0xff00,0x0000,0x0000,0x0000,0xff00,0x0000},

					{0x0000,0xff00,0xff00,0x0000,0x0000,0x0000,0x0000,0x0000},
					{0x0000,0x0000,0x0000,0x0000,0x0000,0xff00,0xff00,0x0000},
					{0x0000,0xff00,0xff00,0x0000,0xff00,0xff00,0xff00,0x0000},
					{0xff00,0xff00,0xff00,0x0000,0x0000,0xff00,0xff00,0x0000},

					{0xff00,0x0000,0xff00,0x0000,0x0000,0x0000,0x0000,0x0000},
					{0x0000,0x0000,0x0000,0x0000,0xff00,0x0000,0xff00,0x0000},
					{0xff00,0x0000,0xff00,0x0000,0xff00,0xff00,0xff00,0x0000},
					{0xff00,0xff00,0xff00,0x0000,0xff00,0x0000,0xff00,0x0000},
					
					{0xff00,0xff00,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000},
					{0x0000,0x0000,0x0000,0x0000,0xff00,0xff00,0x0000,0x0000},
					{0xff00,0xff00,0x0000,0x0000,0xff00,0xff00,0xff00,0x0000},
					{0xff00,0xff00,0xff00,0x0000,0xff00,0xff00,0x0000,0x0000},
				};

				//TCHAR *Str[] =
				//{
					//TEXT("White"),
					//TEXT("Red"),
					//TEXT("Green"),
					//TEXT("Blue"),
					//TEXT("Cyan"),
					//TEXT("Magenta"),
					//TEXT("Yellow"),
					//TEXT("Black"),
				//};

				GetClientRect(hDlg, &rectCli);
				hdc = BeginPaint (hDlg, &ps); 


				if (g_dwLCDTestLoop <= 7)
				{
					hBr = CreateSolidBrush(ColorTable[g_dwLCDTestLoop]);
					hOldBr = (HBRUSH)SelectObject(hdc, hBr);
					Rectangle(hdc, rectCli.left, rectCli.top, rectCli.right, rectCli.bottom);
					SelectObject(hdc, hOldBr);
					//DrawText(hdc, Str[g_dwLCDTestLoop], -1, &rectCli, DT_SINGLELINE);
				}
				else
				{
					TRIVERTEX vert[2];
					GRADIENT_RECT gRect;

					vert[0].x = rectCli.left;
					vert[0].y = rectCli.top;
					vert[0].Red = RGBTable[g_dwLCDTestLoop-8][0];
					vert[0].Green = RGBTable[g_dwLCDTestLoop-8][1];
					vert[0].Blue = RGBTable[g_dwLCDTestLoop-8][2];
					vert[0].Alpha = RGBTable[g_dwLCDTestLoop-8][3];
					vert[1].x = rectCli.right;
					vert[1].y = rectCli.bottom;
					vert[1].Red = RGBTable[g_dwLCDTestLoop-8][4];
					vert[1].Green = RGBTable[g_dwLCDTestLoop-8][5];
					vert[1].Blue = RGBTable[g_dwLCDTestLoop-8][6];
					vert[1].Alpha = RGBTable[g_dwLCDTestLoop-8][7];
					gRect.UpperLeft = 0;
					gRect.LowerRight = 1;
					GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
					//DrawText(hdc, Str[g_dwLCDTestLoop-8], -1, &rectCli, DT_SINGLELINE);
				}
				//DrawText(hdc, TEXT("Tap screen to continue."), -1, &rectCli, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

				DeleteObject(hBr);

				EndPaint(hDlg, &ps);
			}
			break;
		case WM_CLOSE:
			{
				KillTimer(hDlg, ((int)lParam + 1));
				ExitThread(((int)lParam));
			}
			break;
	}
    return DefWindowProc (hDlg, message, wParam, lParam);
}
