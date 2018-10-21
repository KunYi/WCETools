#include <windows.h>
#include "resource.h"

//----------------------------------------------------------------------
// Global data
//
DWORD g_dwLCDTestLoop = 0;
extern HINSTANCE g_hInst;
extern HANDLE g_hFile;

//======================================================================
// LCD Test Dialog procedure
//
BOOL CALLBACK LCDTestDlgProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
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

    COLOR16 RGBTable[][3] =
    {
        {0xff00,0xff00,0xff00},
        {0xff00,0,0},
        {0,0xff00,0},
        {0,0,0xff00},
        {0,0xff00,0xff00},
        {0xff00,0,0xff00},
        {0xff00,0xff00,0},
        {0,0,0},
    };

    TCHAR *Str[] =
    {
        TEXT("White"),
        TEXT("Red"),
        TEXT("Green"),
        TEXT("Blue"),
        TEXT("Cyan"),
        TEXT("Magenta"),
        TEXT("Yellow"),
        TEXT("Black"),
    };

    switch (wMsg)
    {
        case WM_INITDIALOG:
        {
            SetWindowPos (hWnd, HWND_TOPMOST, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 0);
            break;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT rectCli;
            HBRUSH hBr, hOldBr;
            HDC hdc;

            GetClientRect(hWnd, &rectCli);
            hdc = BeginPaint(hWnd, &ps); 
            if (g_dwLCDTestLoop < 8)
            {
                hBr = CreateSolidBrush(ColorTable[g_dwLCDTestLoop]);
                hOldBr = (HBRUSH)SelectObject(hdc, hBr);
                Rectangle(hdc, rectCli.left, rectCli.top, rectCli.right, rectCli.bottom);
                SelectObject(hdc, hOldBr);
                DeleteObject(hBr);
                //DrawText(hdc, Str[g_dwLCDTestLoop], -1, &rectCli, DT_SINGLELINE);
            }
            else
            {
                TRIVERTEX vert[2];
                GRADIENT_RECT gRect;
                DWORD i;

                for (i = 0; i < 8; i++)
                {
                    vert[0].x = rectCli.left;
                    vert[0].y = rectCli.top + i * 60;
                    vert[0].Red = 0x0000;
                    vert[0].Green = 0x0000;
                    vert[0].Blue = 0x0000;
                    vert[0].Alpha = 0x0000;
                    vert[1].x = rectCli.right;
                    vert[1].y = rectCli.top + (i + 1) * 60;
                    vert[1].Red = RGBTable[i][0];
                    vert[1].Green = RGBTable[i][1];
                    vert[1].Blue = RGBTable[i][2];
                    vert[1].Alpha = 0x0000;
                    gRect.UpperLeft = 0;
                    gRect.LowerRight = 1;
                    GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
                }
            }
            DrawText(hdc, TEXT("Tap screen to continue."), -1, &rectCli, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            EndPaint(hWnd, &ps); 
            break;
        }
        
        case WM_LBUTTONDOWN:
        {
            if (g_dwLCDTestLoop++ == 8)
                EndDialog(hWnd, 0);
        
            //update window
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
            break;
        }
    }
    return FALSE;
}
//----------------------------------------------------------------------
// LCD_Test
//
BOOL LCD_Test(HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    CHAR szText[32];
    DWORD dwBytesWriten;

    g_dwLCDTestLoop = 0;
    DialogBoxParam(g_hInst, TEXT("LCDTestDlg"), NULL, LCDTestDlgProc, 0);
    if (MessageBox(hWnd, TEXT("Are the RGB patterns correct?"), TEXT("LCD Test"), MB_YESNO|MB_ICONQUESTION) == IDYES)
    {
        SetDlgItemText(hWnd, IDC_EDIT2, TEXT("PASS"));
        sprintf(szText, "LCD Test: PASS\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }
    else
    {
        SetDlgItemText(hWnd, IDC_EDIT2, TEXT("FAIL"));
        sprintf(szText, "LCD Test: FAIL\r\n");
        if(!WriteFile(g_hFile, szText, strlen(szText), &dwBytesWriten, NULL))
        {
            MessageBox(hWnd, TEXT("Write File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        }
    }
    return 0;
}