#include <windows.h>
#include "resource.h"

#define TS_CROSSSIZE 15

typedef struct {
    DWORD x, y;
} touchPointT;

//----------------------------------------------------------------------
// Global data
//
BOOL g_fShowCrosshair = FALSE;
DWORD g_dwCrosshairX = 0;
DWORD g_dwCrosshairY = 0;
DWORD g_dwTouchTestLoop = 0;
touchPointT touchPoint[5];
extern HWND g_hWnd;
extern HINSTANCE g_hInst;
extern HANDLE g_hFile;

//======================================================================
// SetCrosshair
//  move the crosshair to the desired location on the screen
//
void SetCrosshair(HWND hWnd, DWORD x, DWORD y, BOOL fVisible )
{
    g_dwCrosshairX = x;
    g_dwCrosshairY = y;
    g_fShowCrosshair = fVisible;

    // invalidate, erase, and refresh the entire window
    InvalidateRect( hWnd, NULL, TRUE );
    UpdateWindow( hWnd );
}

//----------------------------------------------------------------------
// DrawCrosshair
//  draws a crosshair at the specified X, Y coordinate given device context
//  and radius in pixels. Returns TRUE on success, FALSE otherwise.
//
BOOL DrawCrosshair(HDC hdc, DWORD dwCrosshairX, DWORD dwCrosshairY, DWORD dwRadius)
{
    if( NULL == hdc )
    {
        return FALSE;
    }

    POINT lineHoriz[2] =
    {
        dwCrosshairX - dwRadius,
        dwCrosshairY,
        dwCrosshairX + dwRadius,
        dwCrosshairY
    };

    POINT lineVert[2] =
    {
        dwCrosshairX,
        dwCrosshairY - dwRadius,
        dwCrosshairX,
        dwCrosshairY + dwRadius
    };

    Polyline( hdc, lineHoriz, 2 );
    Polyline( hdc, lineVert, 2 );

    return TRUE;
}
//----------------------------------------------------------------------
// Touch Test Dialog procedure
//
BOOL CALLBACK TouchTestDlgProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    DWORD dwDisplayWidth= 0;
    DWORD dwDisplayHeight = 0;

    switch (wMsg)
    {
        case WM_INITDIALOG:
        {
            dwDisplayWidth = GetSystemMetrics(SM_CXSCREEN);
            dwDisplayHeight = GetSystemMetrics(SM_CYSCREEN);

            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, dwDisplayWidth, dwDisplayHeight, 0);

            touchPoint[0].x = dwDisplayWidth/2;
            touchPoint[0].y = dwDisplayHeight/2;
            touchPoint[1].x = TS_CROSSSIZE*2+1;
            touchPoint[1].y = TS_CROSSSIZE*2+1;
            touchPoint[2].x = TS_CROSSSIZE*2+1;
            touchPoint[2].y = dwDisplayHeight-TS_CROSSSIZE*2+1;
            touchPoint[3].x = dwDisplayWidth-TS_CROSSSIZE*2+1;
            touchPoint[3].y = dwDisplayHeight-TS_CROSSSIZE*2+1;
            touchPoint[4].x = dwDisplayWidth-TS_CROSSSIZE*2+1;
            touchPoint[4].y = TS_CROSSSIZE*2+1;

            SetCrosshair(hWnd, touchPoint[0].x, touchPoint[0].y, TRUE);
            break;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc;
            HPEN hOldPen;

            hdc = BeginPaint(hWnd, &ps);

            // draw crosshair
            hOldPen = (HPEN) SelectObject( hdc, GetStockObject(BLACK_PEN) );
            if( g_fShowCrosshair )
            {
                DrawCrosshair( hdc, g_dwCrosshairX, g_dwCrosshairY, TS_CROSSSIZE );
            }

            EndPaint(hWnd, &ps);
            break;
        }

        case WM_LBUTTONDOWN:
        {
            POINT ptM;
            CHAR szText[32];
            DWORD dwBytesWriten;

            ptM.x = LOWORD (lParam);
            ptM.y = HIWORD (lParam);
            RETAILMSG(1, (TEXT("now x = %d, y = %d, should x = %d, y = %d\r\n"),ptM.x, ptM.y, touchPoint[g_dwTouchTestLoop].x, touchPoint[g_dwTouchTestLoop].y));

            if ((abs(ptM.x - touchPoint[g_dwTouchTestLoop].x) < 10) && (abs(ptM.y - touchPoint[g_dwTouchTestLoop].y) < 10))
            {
                g_dwTouchTestLoop++;
                if (g_dwTouchTestLoop == 5)
                {
                    SetDlgItemText(g_hWnd, IDC_EDIT3, TEXT("PASS"));
                    sprintf(szText, "Touch Test: PASS\r\n");
                    if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
                    {
                        MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
                    }
                    EndDialog(hWnd, 0);
                }
                else
                {
                    SetCrosshair(hWnd, touchPoint[g_dwTouchTestLoop].x, touchPoint[g_dwTouchTestLoop].y, TRUE);
                }
            }
            else
            {
                SetDlgItemText(g_hWnd, IDC_EDIT3, TEXT("FAIL"));
                sprintf(szText, "Touch Test: FAIL\r\n");
                if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
                {
                    MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
                }
                EndDialog(hWnd, 0);
            }
            break;
        }
    }
    return FALSE;
}
//----------------------------------------------------------------------
// Touch_Test
//
BOOL Touch_Test(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    Sleep(1000);
    g_hWnd = hWnd;
    g_dwTouchTestLoop = 0;
    DialogBoxParam(g_hInst, TEXT("TouchTestDlg"), NULL, TouchTestDlgProc, 0);

    return 0;
}
