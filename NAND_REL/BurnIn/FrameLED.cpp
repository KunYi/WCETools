#include "stdafx.h"
#include "HelloWorld.h"
#include <commctrl.h>
#include "api.h"

//-----------------------------------------------------------------------------
// LED Frame Control API
//-----------------------------------------------------------------------------
extern PFNSetGreenLEDFrameValue pfnSetGreenLEDFrameValue;
extern PFNGetGreenLEDFrameValue pfnGetGreenLEDFrameValue;

extern PFNSetRedLEDFrameValue pfnSetRedLEDFrameValue;
extern PFNGetRedLEDFrameValue pfnGetRedLEDFrameValue;

extern PFNSetLEDFrameColor pfnSetLEDFrameColor;
extern PFNGetLEDFrameColor pfnGetLEDFrameColor;

extern PFNSetLEDStatus pfnSetLEDStatus;
extern PFNGetLEDFrameStatus pfnGetLEDFrameStatus;

extern volatile DWORD	g_dwFrameLEDColor, g_dwGreenFrameLEDLevel, g_dwRedFrameLEDLevel;

// Mesage handler for the Frame LED box.
LRESULT CALLBACK DoMainCommandThreadFrameLEDDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT	rt, rt1;
	int		DlgWidth, DlgHeight;	// dialog width and height in pixel units
	int		NewPosX, NewPosY;
	static int		i = 1;
	//DWORD	dwFrameLEDColor, dwGreenFrameLEDLevel, dwRedFrameLEDLevel;


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
				SetWindowPos(hDlg, 0, 0, DlgHeight*2,	DlgWidth, DlgHeight, SWP_NOZORDER);
				SetWindowText(hDlg, TEXT("BurnIn Test - Frame LED Test"));
				SetTimer(hDlg, ((int)lParam + 1), 200, NULL);
				pfnSetLEDStatus(ON);
			}
			return TRUE;

/*		case WM_TIMER:
			{
				//dwFrameLEDColor = pfnGetLEDFrameColor();
				//dwGreenFrameLEDLevel = pfnGetGreenLEDFrameValue();
				//dwRedFrameLEDLevel = pfnGetRedLEDFrameValue();

				if(i > 0)
				{
					if(( (GREEN == g_dwFrameLEDColor)?g_dwGreenFrameLEDLevel:g_dwRedFrameLEDLevel ) < 100)
					{
						(GREEN == g_dwFrameLEDColor)?++g_dwGreenFrameLEDLevel:++g_dwRedFrameLEDLevel;
					}
					else
					{
						(GREEN == g_dwFrameLEDColor)?--g_dwGreenFrameLEDLevel:--g_dwRedFrameLEDLevel;
						i = -1;
					}
				}
				else
				{
					if( ((GREEN == g_dwFrameLEDColor)?g_dwGreenFrameLEDLevel:g_dwRedFrameLEDLevel) > 1 )
					{
						(GREEN == g_dwFrameLEDColor)?--g_dwGreenFrameLEDLevel:--g_dwRedFrameLEDLevel;
					}
					else
					{
						(GREEN == g_dwFrameLEDColor)?++g_dwGreenFrameLEDLevel:++g_dwRedFrameLEDLevel;

						i = 1;
						if(GREEN == g_dwFrameLEDColor)
						{
							g_dwFrameLEDColor = RED;
							pfnSetRedLEDFrameValue(g_dwRedFrameLEDLevel);
							pfnSetLEDFrameColor(RED);
							pfnSetLEDStatus(ON);
							break;
						}
						else
						{
							g_dwFrameLEDColor = GREEN;
							pfnSetGreenLEDFrameValue(g_dwGreenFrameLEDLevel);
							pfnSetLEDFrameColor(GREEN);
							pfnSetLEDStatus(ON);
							break;
						}
					}
				}

				(GREEN == g_dwFrameLEDColor)?pfnSetGreenLEDFrameValue(g_dwGreenFrameLEDLevel):pfnSetRedLEDFrameValue(g_dwRedFrameLEDLevel);
				pfnSetLEDFrameColor(g_dwFrameLEDColor);

				//update window
				InvalidateRect(hDlg, NULL, TRUE);
				UpdateWindow(hDlg);
			}
			break;*/
		case WM_TIMER:
			{
				if(i > 0)	// 漸亮
				{
					if((GREEN == g_dwFrameLEDColor))		// 綠色
					{
						if(g_dwGreenFrameLEDLevel < 100)
						{
							++g_dwGreenFrameLEDLevel;		// 如果為綠色且小於100則綠色亮度先加一
						}
						else
						{
							--g_dwGreenFrameLEDLevel;		// 綠色亮度等於100則先減一
							i = -1;							// 反轉亮度變換方向
						}
						pfnSetGreenLEDFrameValue(g_dwGreenFrameLEDLevel);
						pfnSetLEDFrameColor(g_dwFrameLEDColor);
					}
					else									// 紅色
					{
						if(g_dwRedFrameLEDLevel < 100)
						{
							++g_dwRedFrameLEDLevel;			// 如果為紅色且小於100則紅色亮度先加一
						}
						else
						{
							--g_dwRedFrameLEDLevel;			// 紅色亮度等於100則先減一
							i = -1;							// 反轉亮度變換方向
						}
						pfnSetRedLEDFrameValue(g_dwRedFrameLEDLevel);
						pfnSetLEDFrameColor(g_dwFrameLEDColor);
					}
				}
				else	// 漸暗
				{
					if((GREEN == g_dwFrameLEDColor))		// 綠色
					{
						if(g_dwGreenFrameLEDLevel > 0)
						{
							--g_dwGreenFrameLEDLevel;		// 如果為綠色且大於0則綠色亮度先減一
						}
						else
						{
							++g_dwGreenFrameLEDLevel;		// 綠色亮度等於0則先加一
							i = 1;							// 反轉亮度變換方向
							g_dwFrameLEDColor = RED;
							pfnSetRedLEDFrameValue(g_dwRedFrameLEDLevel);
							pfnSetLEDFrameColor(RED);
							pfnSetLEDStatus(ON);
							break;
						}
						pfnSetGreenLEDFrameValue(g_dwGreenFrameLEDLevel);
						pfnSetLEDFrameColor(g_dwFrameLEDColor);
					}
					else									// 紅色
					{
						if(g_dwRedFrameLEDLevel > 0)
						{
							--g_dwRedFrameLEDLevel;			// 如果為紅色且大於0則紅色亮度先減一
						}
						else
						{
							++g_dwRedFrameLEDLevel;			// 紅色亮度等於0則先加一
							i = 1;							// 反轉亮度變換方向
							g_dwFrameLEDColor = GREEN;
							pfnSetGreenLEDFrameValue(g_dwGreenFrameLEDLevel);
							pfnSetLEDFrameColor(GREEN);
							pfnSetLEDStatus(ON);
							break;
						}
						pfnSetRedLEDFrameValue(g_dwRedFrameLEDLevel);
						pfnSetLEDFrameColor(g_dwFrameLEDColor);
					}
				}
		
				//update window
				InvalidateRect(hDlg, NULL, TRUE);
				UpdateWindow(hDlg);
			}

			break;

		case WM_LBUTTONDOWN:
			KillTimer(hDlg, ((int)lParam + 1));
			pfnSetLEDStatus(OFF);
			EndDialog(hDlg, LOWORD(wParam));
			break;
		case WM_CLOSE:
			{
				pfnSetLEDStatus(OFF);
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
				wsprintf(szDbg, TEXT("Click to terminate frame LED burnIn test."));
				DrawText(hdc, szDbg, _tcslen(szDbg), &rt, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

				EndPaint(hDlg, &ps);
			}
			break;
	}
    return DefWindowProc (hDlg, message, wParam, lParam);
}
