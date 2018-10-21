
#if !defined(AFX_HELLOWORLD_H__EDE0247D_6670_4D60_B1B6_6FD0CFE35444__INCLUDED_)
#define AFX_HELLOWORLD_H__EDE0247D_6670_4D60_B1B6_6FD0CFE35444__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

// Returns number of elements
#define dim(x) (sizeof(x) / sizeof(x[0]))
//#define MAX_LOADSTRING 100

//----------------------------------------------------------------------
// Generic defines and data types
//----------------------------------------------------------------------
struct decodeUINT
{								// Structure associates messages with a function.
    UINT Code;
	LRESULT (*Fxn)(HWND, UINT, WPARAM, LPARAM);
};

struct decodeCMD
{								// Structure menu IDs with a function
    UINT Code;
    LRESULT (*Fxn)(HWND, WORD, HWND, WORD);
};

struct decodeNotify
{								// Structure associates control IDs with a notify handler.
	UINT Code;
	LRESULT (*Fxn)(HWND, WORD, HWND, LPNMHDR);
};

struct CommonControls
{
	HWND		hThreadHandle;
	LPTHREAD_START_ROUTINE lpThreadStartAddr;
	DWORD		dwExStyle;
	LPCTSTR		lpClassName;
	LPCTSTR		lpWindowName;
	DWORD		dwStyle;
	int			x;
	int			y;
	int			nWidth;
	int			nHeight;
	HMENU		hMenu;
	HANDLE		hEvent;
};



//----------------------------------------------------------------------
// Forward declarations of functions included in this code module:
//----------------------------------------------------------------------
ATOM				MyRegisterClass				(HINSTANCE, LPTSTR);
BOOL				InitInstance				(HINSTANCE, int);
LRESULT CALLBACK	WndProc						(HWND, UINT, WPARAM, LPARAM);
LRESULT				DoCreateMain				(HWND, UINT, WPARAM, LPARAM);
LRESULT				DoSizeMain					(HWND, UINT, WPARAM, LPARAM);
//LRESULT				DoActivateMain				(HWND, UINT, WPARAM, LPARAM);
LRESULT				DoPaintMain					(HWND, UINT, WPARAM, LPARAM);
//LRESULT				DoMeasureItemMain			(HWND, UINT, WPARAM, LPARAM);
LRESULT				DoDrawItemMain				(HWND, UINT, WPARAM, LPARAM);
LRESULT				DoKeyDownMain				(HWND, UINT, WPARAM, LPARAM);
LRESULT				DoHibernateMain				(HWND, UINT, WPARAM, LPARAM);
LRESULT				DoDestroyMain				(HWND, UINT, WPARAM, LPARAM);
LRESULT				DoCloseMain					(HWND, UINT, WPARAM, LPARAM);
//LPARAM				DoEraseBKGndMain			(HWND, UINT, WPARAM, LPARAM);
LRESULT				DoCommandMain				(HWND, UINT, WPARAM, LPARAM);
LRESULT				DoNotifyMain				(HWND, UINT, WPARAM, LPARAM);

// Extern declarations of WM_Command handling functions included in this code module:
LPARAM				DoMainCommandABOUT					(HWND, WORD, HWND, WORD);
LPARAM				DoMainCommandEXIT					(HWND, WORD, HWND, WORD);
LRESULT CALLBACK	About								(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	DoMainCommandThreadLCDDialog		(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	DoMainCommandThreadBacklightDialog	(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	DoMainCommandThreadFrameLEDDialog	(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	DoMainCommandThreadTextLEDDialog	(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	DoMainCommandThreadBuzzerDialog		(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	DoMainCommandThreadLANDialog		(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	DoMainCommandThreadMemoryDialog		(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	DoMainCommandThreadTimeDialog		(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LPARAM				DoMainCommandSTART					(HWND, WORD, HWND, WORD);
LPARAM				DoMainCommandALL					(HWND, WORD, HWND, WORD);
LPARAM				DoMainCommandNONE					(HWND, WORD, HWND, WORD);
DWORD WINAPI		DoMainCommandThreadLCD				(PVOID pArg);
DWORD WINAPI		DoMainCommandThreadBacklight		(PVOID pArg);
DWORD WINAPI		DoMainCommandThreadFrameLED			(PVOID pArg);
DWORD WINAPI		DoMainCommandThreadTextLED			(PVOID pArg);
DWORD WINAPI		DoMainCommandThreadBuzzer			(PVOID pArg);
DWORD WINAPI		DoMainCommandThreadLAN				(PVOID pArg);
DWORD WINAPI		DoMainCommandThreadMemory			(PVOID pArg);
DWORD WINAPI		DoMainCommandThreadVideo			(PVOID pArg);
DWORD WINAPI		DoMainCommandThreadTime			(PVOID pArg);
void				DoMainLibraryInitialization			(void);

#endif // !defined(AFX_HELLOWORLD_H__EDE0247D_6670_4D60_B1B6_6FD0CFE35444__INCLUDED_)
