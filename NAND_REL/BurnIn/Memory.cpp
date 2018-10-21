#include "stdafx.h"
#include "HelloWorld.h"
#include <commctrl.h>
#include "api.h"
#include <pwindbas.h>


extern HWND				g_Main_hWnd;			// The main window handle
extern HINSTANCE		g_Main_hInst;			// The current instance
extern CommonControls MainCommonControls[];

HWND					g_Memory_hDlg;
HANDLE					g_hThread;
PDWORD					g_pBuffer = 0;
DWORD					g_dwTestLength = 10*1024*1024;
extern bool				g_bContinue;
bool					g_bFlag = FALSE;
HANDLE					g_Memory_hEvent;


DWORD WINAPI DoMemoryTestThread(PVOID pArg);
void GetProgramInformation(DWORD PageSize, DWORD *pRamUsed);
void GetMemoryInfo(DWORD& PageSize, DWORD& TotPages, DWORD& StoreUsed, DWORD& RamUsed, DWORD& StorePages);
void WriteMem(PDWORD start, DWORD length, DWORD pattern);
BOOL VerifyMem(PDWORD start, DWORD length, DWORD pattern);


// Mesage handler for the Memory box.
LRESULT CALLBACK DoMainCommandThreadMemoryDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
				DWORD dwThreadID;

				GetClientRect(GetParent(hDlg), &rt);

				DlgWidth	= GetSystemMetrics(SM_CXSCREEN)/2;
				DlgHeight	= GetSystemMetrics(SM_CYSCREEN)/4;

				MapWindowPoints(GetParent(hDlg), HWND_DESKTOP, (LPPOINT)&rt, 2);
				NewPosX		= rt.left;
				NewPosY		= rt.top;

				// if the dialog box is larger than the physical screen 
				if (NewPosX < 0) NewPosX = 0;
				if (NewPosY < 0) NewPosY = 0;
				SetWindowPos(hDlg, 0, DlgWidth, DlgHeight,	DlgWidth, DlgHeight, SWP_NOZORDER);
				SetWindowText(hDlg, TEXT("BurnIn Test - Memory Test"));
				//SetTimer(hDlg, ((int)lParam + 1), 200, NULL);


				GetClientRect(hDlg, &rt);
				//MapWindowPoints(GetParent(hDlg), HWND_DESKTOP, (LPPOINT)&rt, 2);
				DlgWidth		= rt.right - rt.left;
				DlgHeight		= rt.bottom - rt.top;

				//=============================================================
				//		Create Dialog Item
				//=============================================================
				CreateWindowEx(0, TEXT ("STATIC"), TEXT ("Cycle 0: Stanby"),	WS_VISIBLE | WS_CHILD | WS_TABSTOP | SS_LEFT, (int)DlgWidth * 0.0f, (int)DlgHeight * 0.0f, (int)DlgWidth * 1.0f, (int)DlgHeight * 0.2f, hDlg, (HMENU)IDC_MEMORY_STATIC_CYCLE, g_Main_hInst, NULL);
				CreateWindowEx(0, PROGRESS_CLASS, TEXT("Memory Test Progress"),						WS_VISIBLE | WS_CHILD | WS_TABSTOP | PBS_SMOOTH | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_BORDER, (int)DlgWidth * 0.0f, (int)DlgHeight * 0.2f, (int)DlgWidth * 1.0f, (int)DlgHeight * 0.1f, hDlg, (HMENU)IDC_MEMORY_PREGRESS_PREGRESSBAR, g_Main_hInst, NULL);
				CreateWindowEx(0, TEXT ("STATIC"), TEXT ("Pattern:"),			WS_VISIBLE | WS_CHILD | WS_TABSTOP | SS_LEFT, (int)DlgWidth * 0.0f, (int)DlgHeight * 0.3f, (int)DlgWidth * 1.0f, (int)DlgHeight * 0.2f, hDlg, (HMENU)IDC_MEMORY_STATIC_PATTERN, g_Main_hInst, NULL);
				CreateWindowEx(0, TEXT ("STATIC"), TEXT ("Total RAM: "),		WS_VISIBLE | WS_CHILD | WS_TABSTOP | SS_LEFT, (int)DlgWidth * 0.0f, (int)DlgHeight * 0.5f, (int)DlgWidth * 0.5f, (int)DlgHeight * 0.2f, hDlg, (HMENU)IDC_MEMORY_STATIC_TOTALMEMORY, g_Main_hInst, NULL);
				CreateWindowEx(0, TEXT ("STATIC"), TEXT ("Free RAM: "),			WS_VISIBLE | WS_CHILD | WS_TABSTOP | SS_LEFT, (int)DlgWidth * 0.5f, (int)DlgHeight * 0.5f, (int)DlgWidth * 0.5f, (int)DlgHeight * 0.2f, hDlg, (HMENU)IDC_MEMORY_STATIC_FREEMEMORY, g_Main_hInst, NULL);
				CreateWindowEx(0, TEXT ("STATIC"), TEXT ("Test RAM: "),			WS_VISIBLE | WS_CHILD | WS_TABSTOP | SS_LEFT, (int)DlgWidth * 0.0f, (int)DlgHeight * 0.7f, (int)DlgWidth * 1.0f, (int)DlgHeight * 0.2f, hDlg, (HMENU)IDC_MEMORY_STATIC_TESTMEMORY, g_Main_hInst, NULL);


				//=============================================================
				//		Create Test Thread
				//=============================================================
				g_Memory_hDlg = hDlg;
				if(!g_hThread)
				{
					if(NULL == (g_hThread = CreateThread(NULL, 0, DoMemoryTestThread, &g_Memory_hDlg, 0 , &dwThreadID)))
					{
						TCHAR	szDbg[MAX_PATH];
						wsprintf(szDbg, TEXT("Error on creating memory test thread.\r\n"));
						MessageBox (g_Main_hWnd, szDbg, TEXT("Error"), MB_OK | MB_ICONERROR);
					}
					else
					{
						if(!g_Memory_hEvent)
						{
							g_Memory_hEvent = CreateEvent(NULL, FALSE,	FALSE, MainCommonControls[(int)lParam].lpWindowName);
						}
						SetEvent(MainCommonControls[(int)lParam].hEvent);
					}
				}
				/*else
				{
					SetEvent(MainCommonControls[(int)lParam].hEvent);
				}*/
			}
			break;

		//case WM_TIMER:
			//{

				//update window
				//InvalidateRect(hDlg, NULL, TRUE);
				//UpdateWindow(hDlg);
			//}
			//break;

		case WM_LBUTTONDOWN:
			{
				g_bContinue = FALSE;
				CloseHandle(g_hThread);
				g_hThread = NULL;
				
			}
			break;
		case WM_COMMAND:
			if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
			{
				ExitThread(((int)lParam));
				EndDialog(hDlg, LOWORD(wParam));
			}
			break;
		case WM_CLOSE:
			{
				
				
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
				wsprintf(szDbg, TEXT("Click to terminate memory burnIn test."));
				DrawText(hdc, szDbg, _tcslen(szDbg), &rt, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

				EndPaint(hDlg, &ps);
			}
			break;
	}
    return DefWindowProc (hDlg, message, wParam, lParam);
}


//----------------------------------------------------------------------
//
DWORD WINAPI DoMemoryTestThread(PVOID pArg)
{
	DWORD PageSize, TotPages, StoreUsed, RamUsed, StorePages;
    DWORD i, cycle = 0, pattern[] = {1, 0xAA, 0x55, 0, 0xFF};
    TCHAR szString[25];
    TCHAR *Str[] =
    {
        TEXT("Pattern: Sequence (0,1,2,...255)"),
        TEXT("Pattern: Binary 1 (10101010)"),
        TEXT("Pattern: Binary 2 (01010101)"),
        TEXT("Pattern: Zeros (00000000)"),
        TEXT("Pattern: Ones (1111111)"),
    };


	//=========================================================================
	//		Query Memory Size
	//=========================================================================
	GetMemoryInfo(PageSize, TotPages, StoreUsed, RamUsed, StorePages);
	wsprintf(szString, TEXT("Total RAM: %d KBytes"), TotPages*PageSize);
	SetDlgItemText(g_Memory_hDlg, IDC_MEMORY_STATIC_TOTALMEMORY, szString);
	wsprintf(szString, TEXT("Free RAM: %d KBytes"), (TotPages-StorePages-RamUsed) * PageSize);
	SetDlgItemText(g_Memory_hDlg, IDC_MEMORY_STATIC_FREEMEMORY, szString);
	SendMessage (GetDlgItem (g_Memory_hDlg, IDC_MEMORY_PREGRESS_PREGRESSBAR), PBM_SETRANGE, 0, MAKELPARAM (0,100));

	//=========================================================================
	//		Allocate Test Memory
	//=========================================================================
	g_dwTestLength = ((TotPages-StorePages-RamUsed) * PageSize) / (1024);
	wsprintf(szString, TEXT("Allocate RAM: %d MBytes"), g_dwTestLength);
	SetDlgItemText(g_Memory_hDlg, IDC_MEMORY_STATIC_TESTMEMORY, szString);
	g_dwTestLength = g_dwTestLength * 1024 * 1024;

	if(!g_pBuffer)
	{
		if (!(g_pBuffer = (PDWORD)malloc(g_dwTestLength)))
		{
			MessageBox(g_Memory_hDlg, TEXT("Out of Memory!"), TEXT("Memory Test"), MB_OK);
			return 0;
		}
	}

	while (g_bContinue)
	{
		for (i = 0; i < 5; i++)
		{
			if (!g_bContinue)
                break;

			SetDlgItemText(g_Memory_hDlg, IDC_MEMORY_STATIC_PATTERN, Str[i]);

			wsprintf(szString, TEXT("Cycle %d: Writing"), cycle);
			SetDlgItemText(g_Memory_hDlg, IDC_MEMORY_STATIC_CYCLE, szString);
			WriteMem(g_pBuffer, g_dwTestLength, pattern[i]);

			wsprintf(szString, TEXT("Cycle %d: Verifying"), cycle);
			SetDlgItemText(g_Memory_hDlg, IDC_MEMORY_STATIC_CYCLE, szString);
			if (!VerifyMem(g_pBuffer, g_dwTestLength, pattern[i]))
			{
				MessageBox(g_Memory_hDlg, TEXT("Memory Verify Fail!"), TEXT("Memory Test"), MB_OK);
				return FALSE;
			}
			cycle++;
		}
	}

	g_bFlag = TRUE;


	if(g_pBuffer)
	{
		free(g_pBuffer);
		g_pBuffer = NULL;
	}

	return 0;
}
//----------------------------------------------------------------------
//
void GetProgramInformation(DWORD PageSize, DWORD *pRamUsed)
{
    MEMORYSTATUS mst;

    mst.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&mst);

    mst.dwTotalPhys /= (PageSize * 1024);
    mst.dwAvailPhys /= (PageSize * 1024);

    *pRamUsed = mst.dwTotalPhys - mst.dwAvailPhys + 1;   // '-1' because page #0 for system page ?
}
//----------------------------------------------------------------------
//
void GetMemoryInfo(DWORD& PageSize, DWORD& TotPages, DWORD& StoreUsed, DWORD& RamUsed, DWORD& StorePages)
{
    DWORD RamPages;
    STORE_INFORMATION  StoreInfo;

    GetSystemMemoryDivision(&StorePages, &RamPages, &PageSize);

    GetStoreInformation(&StoreInfo);
    StoreInfo.dwStoreSize /= PageSize;
    StoreInfo.dwFreeSize /= PageSize;

    PageSize /= 1024;
    TotPages = StorePages+RamPages;
    StoreUsed = StorePages - StoreInfo.dwFreeSize; // use the 'free' number. Becuase dwStoreSize has some compression and it is in-accurate
    GetProgramInformation(PageSize, &RamUsed);
}
//----------------------------------------------------------------------
//
void WriteMem(PDWORD start, DWORD length, DWORD pattern)
{
    DWORD i, dwPreProgress = 0, dwProgress = 0;

    for (i = 0; i < length / 4; i++)
    {
        if (pattern == 1)
        {
            start[i] = i;
        }
        else
        {
            start[i] = pattern;
        }

        dwProgress = i*400/length;
        if (dwPreProgress != dwProgress)
        {
            dwPreProgress = dwProgress;
            SendMessage(GetDlgItem(g_Memory_hDlg, IDC_MEMORY_PREGRESS_PREGRESSBAR), PBM_SETPOS, dwProgress, NULL);
        }
    }
}
//----------------------------------------------------------------------
//
BOOL VerifyMem(PDWORD start, DWORD length, DWORD pattern)
{
    DWORD i, dwPreProgress = 0, dwProgress = 0;

    for (i = 0; i < length / 4; i++)
    {
        if (pattern == 1)
        {
            if (start[i] != i)
            {
                return FALSE;
            }
        }
        else
        {
            if (start[i] != pattern)
            {
                return FALSE;
            }
        }

        dwProgress = i*400/length;
        if (dwPreProgress != dwProgress)
        {
            dwPreProgress = dwProgress;
            SendMessage(GetDlgItem(g_Memory_hDlg, IDC_MEMORY_PREGRESS_PREGRESSBAR), PBM_SETPOS, dwProgress, NULL);
        }
    }
    return TRUE;
}
