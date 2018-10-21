//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++


Module Name:

    mon.c

Abstract:

    Power Manager Monitor.
    Run in the background to display power state changes in debugger.

Notes:

Revision History:

--*/

#include <windows.h>
#include <shlwapi.h>
#include <msgqueue.h>
#include <pnp.h>
#include <guiddef.h>
#include <pm.h>

#include <tchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ini.h"

#define QUEUE_ENTRIES   3
#define MAX_NAMELEN     200
#define QUEUE_SIZE      (QUEUE_ENTRIES * (sizeof(POWER_BROADCAST) + MAX_NAMELEN))

// global variables
HANDLE ghPowerNotifications = NULL;
HANDLE ghevTerminate = NULL;

WCHAR	gFullPath[MAX_PATH];
WCHAR	gINIFileName[MAX_PATH];
WCHAR	gLogFileName[MAX_PATH];
BOOL	gbLog = FALSE;
const WCHAR PRE_DIR[] = TEXT("\\ResidentFlash");

static int SystemTime2String(const SYSTEMTIME* st, char* str, const int len);

SYSTEMTIME	lastRecordDateTime;

typedef struct {
	BOOL	activate;
} configuration;

static int handler(void* user, const char* section, const char* name,
				const char* value)
{
	int ret = 1;

	configuration* pconfig = (configuration*)user;
	
	#define MATCH(s, n) _stricmp(section, s) == 0 && _stricmp(name, n) == 0
	if (MATCH("PMEVENT", "ACTIVATE")) {
		pconfig->activate = (atoi(value) != 0) ? TRUE : FALSE;
	} else {
		/* unknown section/name, error */
		RETAILMSG(1, (TEXT("Unknown section/name\r\n")));
		ret = 0;
    }
    
	return ret;
}


BOOL compareSystemTimeIn3sec(const SYSTEMTIME* pP1,const SYSTEMTIME* pP2)
{
	FILETIME          ft1, ft2;
	ULARGE_INTEGER    uInt1, uInt2;
	
	SystemTimeToFileTime(pP1, &ft1);
	SystemTimeToFileTime(pP2, &ft2);
	
    uInt1.LowPart = ft1.dwLowDateTime;
    uInt1.HighPart = ft1.dwHighDateTime;

    uInt2.LowPart = ft2.dwLowDateTime;
    uInt2.HighPart = ft2.dwHighDateTime;
	
	if (uInt1.QuadPart<uInt2.QuadPart) 
	   return ((uInt2.QuadPart-uInt1.QuadPart) < 3*10000000) ? TRUE : FALSE;
	else
	   return ((uInt1.QuadPart-uInt2.QuadPart) < 3*10000000) ? TRUE : FALSE;
}

static void WriteLogToFile(const char* str, const int len)
{
	HANDLE hFile;
	DWORD dwBytesWriten;

	
	hFile = CreateFile(gLogFileName, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);

	if (hFile == INVALID_HANDLE_VALUE) {
		RETAILMSG(1, (TEXT("FAILED: Cannot open file %s\r\n"), gLogFileName));
		return;
	}
	
	// move to file end
	SetFilePointer(hFile, 0, NULL, FILE_END);
	WriteFile(hFile, str, len, &dwBytesWriten, NULL);
	
	CloseHandle(hFile);
}

static void WriteLog(const BOOL flag, const SYSTEMTIME* pST,const char* msg)
{
	char buff[512];
	WCHAR dMsg[512];
	SYSTEMTIME st;
	size_t	len;

	if (flag == FALSE) 
		return;

	if (NULL == pST) {
		GetLocalTime(&st);
	} else {			
		st.wYear = pST->wYear;
		st.wMonth = pST->wMonth;
		st.wDay = pST->wDay;
		st.wHour = pST->wHour;
		st.wMinute = pST->wMinute;
		st.wSecond = pST->wSecond;
	}
	
	len = SystemTime2String(&st, buff, sizeof(buff));

	strcat(buff, ",<");
	strcat(buff, msg);
	strcat(buff, ">\r\n\0");
	len = strlen(buff);
	memset(dMsg, 0, sizeof(dMsg));
	mbstowcs(dMsg, buff, len);
    RETAILMSG(TRUE, (TEXT("WriteLog:%s"), dMsg));
	WriteLogToFile(buff, len);
}

static void GetLastRecordDateTime(void)
{
	HANDLE hFile;
	char buff[512];
	DWORD dwBytesRead;
	DWORD FileSizeLow;
	DWORD FileSizeHigh;
	
	hFile = CreateFile(gLogFileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

	if (hFile == INVALID_HANDLE_VALUE) {
		//RETAILMSG(1, (TEXT("FAILED: Cannot open file CreateFile()\r\n"));
		hFile = CreateFile(gLogFileName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
		if (hFile == INVALID_HANDLE_VALUE) {
			RETAILMSG(1, (TEXT("FAILED: Cannot create file CreateFile()\r\n")));
		}
		CloseHandle(hFile);
		GetLocalTime(&lastRecordDateTime);
		return;
	}

	FileSizeLow = GetFileSize(hFile, &FileSizeHigh);
	if (!((FileSizeHigh == 0) && (FileSizeLow <= 512))) {
		// find last line, to read date and time
		SetFilePointer(hFile, -512, NULL, FILE_END); 
	}

	if (ReadFile(hFile, buff, 512, &dwBytesRead, NULL))	{
		BOOL  fTime = FALSE;
		BOOL  fDate = FALSE;
		DWORD i = 0;
		DWORD last = 0;
		DWORD plast = 0;
		char  t[6];
		int  month;
		int  day;
		int  year;
		int	 hour;
		int  minute;
		int  second;
		

		do {
			if (buff[i] == '\n') {
				plast = last;
				last = i;
			}
		} while( i++ < dwBytesRead);
		
		if (plast != last) {
		   i = plast+1;
		} 
		else
		{
			// No any line
			GetLocalTime(&lastRecordDateTime);
			return;
		}
		
		#if 0
		{
			WCHAR wD[512];
			mbstowcs(wD, (buff+i), strlen(buff+i));
			RETAILMSG(TRUE, (TEXT("GetLastRecordTime():%s\r\n"), wD));
		}
		#endif

		if (buff[i+2] == '/' && buff[i+5] == '/' && buff[i+10] == ',')
		{
			t[0] = buff[i++];
			t[1] = buff[i++];
			t[2] = '\0'; i++;
			month = atoi(t);
			t[0] = buff[i++];
			t[1] = buff[i++];
			t[2] = '\0'; i++;
			day = atoi(t);
			t[0] = buff[i++];
			t[1] = buff[i++];
			t[2] = buff[i++];
			t[3] = buff[i++];
			t[4] = '\0'; i++;
			year = atoi(t);
			fDate = TRUE;
			
			if (buff[i+2] == ':' && buff[i+5] == ':') {
				t[0] = buff[i++];
				t[1] = buff[i++];
				t[2] = '\0'; i++;
				hour = atoi(t);
				t[0] = buff[i++];
				t[1] = buff[i++];
				t[2] = '\0'; i++;
				minute = atoi(t);
				t[0] = buff[i++];
				t[1] = buff[i++];
				t[2] = '\0';
				second = atoi(t);
				fTime = TRUE;
			}
		}
		
		if ((FALSE == fTime) || (FALSE == fDate))
		{
			RETAILMSG(1, (TEXT("Last line is incorrect format, fDate: %d, fTime: %d\r\n"), fDate, fTime));
			GetLocalTime(&lastRecordDateTime);
		}
		else {
			lastRecordDateTime.wYear = year;
			lastRecordDateTime.wMonth = month;
			lastRecordDateTime.wDay = day;
			lastRecordDateTime.wHour = hour;
			lastRecordDateTime.wMinute = minute;
			lastRecordDateTime.wSecond = second;
		}
	}
	CloseHandle(hFile);
}

static int SystemTime2String(const SYSTEMTIME* st, char* str, const int len)
{
	char obuf[256];
	int  tlen;

	sprintf(obuf, "%02d/%02d/%04d,%02d:%02d:%02d",
			st->wMonth, st->wDay, st->wYear, 
			st->wHour, st->wMinute, st->wSecond);

	tlen = strlen(obuf);

	if (len < tlen)
		return -1;
	
	if (NULL == str)
		return tlen;
	
	strcpy(str, obuf);
	return tlen;
}

static int getDateTimeString(char* str, const int len)
{
	SYSTEMTIME st;

	GetLocalTime(&st);
	return SystemTime2String(&st, str, len);
}

BOOL GetFullPath(LPWSTR path, DWORD len)
{
	TCHAR	szTemp[256];
	HKEY	hKey;
	LONG	hRes;
	DWORD	dwDisp;
	DWORD	Len;
	DWORD	dwType;
	BOOL	ret = FALSE;

	// Get the current registry data.
	_tcscpy (szTemp, TEXT("SOFTWARE\\PMEvent"));	
	hRes = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, NULL,
			REG_OPTION_NON_VOLATILE, 0, NULL,
			&hKey, &dwDisp);

	if (hRes != ERROR_SUCCESS) {
		RETAILMSG ( 1,(TEXT("!Unable to create reg key '%s'\r\n"),szTemp));
		return FALSE;
	}

	Len = len;
	if (ERROR_SUCCESS == RegQueryValueEx (hKey, TEXT("PATH"), NULL, &dwType, (LPBYTE)path, &Len))
	{
		ret = TRUE;
	}
	
	RegCloseKey(hKey);
	return ret;
}

BOOL GetINIFileName(LPWSTR path, DWORD len)
{
	TCHAR	szTemp[256];
	HKEY	hKey;
	LONG	hRes;
	DWORD	dwDisp;
	DWORD	Len;
	DWORD	dwType;
	BOOL	ret = FALSE;

	// Get the current registry data.
	_tcscpy (szTemp, TEXT("SOFTWARE\\PMEvent"));	
	hRes = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, NULL,
			REG_OPTION_NON_VOLATILE, 0, NULL,
			&hKey, &dwDisp);

	if (hRes != ERROR_SUCCESS) {
		RETAILMSG ( 1,(TEXT("!Unable to create reg key '%s'\r\n"),szTemp));
		return FALSE;
	}

	Len = len;
	if (ERROR_SUCCESS == RegQueryValueEx (hKey, TEXT("INI"), NULL, &dwType, (LPBYTE)path, &Len))
	{
		ret = TRUE;
	}
	
	RegCloseKey(hKey);
	return ret;
}

BOOL GetLogFileName(LPWSTR path, DWORD len)
{
	TCHAR	szTemp[256];
	HKEY	hKey;
	LONG	hRes;
	DWORD	dwDisp;
	DWORD	Len;
	DWORD	dwType;
	BOOL	ret = FALSE;

	// Get the current registry data.
	_tcscpy (szTemp, TEXT("SOFTWARE\\PMEvent"));
	hRes = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, NULL,
			REG_OPTION_NON_VOLATILE, 0, NULL,
			&hKey, &dwDisp);

	if (hRes != ERROR_SUCCESS) {
		RETAILMSG ( 1,(TEXT("!Unable to create reg key '%s'\r\n"),szTemp));
		return FALSE;
	}

	Len = len;
	if (ERROR_SUCCESS == RegQueryValueEx (hKey, TEXT("LOG"), NULL, &dwType, (LPBYTE)path, &Len))
	{
		ret = TRUE;
	}
	
	RegCloseKey(hKey);
	return ret;
}

void ProcessPowerNotification(HANDLE hMsgQ)
{
    UCHAR buf[QUEUE_SIZE];
    int iBytesInQueue;
    DWORD dwFlags = 0;
    static int dwCount = 0;
    LPTSTR pszFname = _T("PMEvent!PowerNotification");

    iBytesInQueue = 0;
    memset(buf, 0, QUEUE_SIZE);

    if ( !ReadMsgQueue(hMsgQ,
                       buf,
                       QUEUE_SIZE,
                       &iBytesInQueue,
                       INFINITE,    // Timeout
                       &dwFlags))
    {
        DWORD dwErr = GetLastError();
        RETAILMSG(1, (TEXT("%s: ReadMsgQueue: ERROR:%d\r\n"), pszFname, dwErr));
        ASSERT(0);
    } else if(iBytesInQueue >= sizeof(POWER_BROADCAST)) {
		char b[256];
		WCHAR wb[256];
        //
        // process power notifications
        //
        PPOWER_BROADCAST pB = (PPOWER_BROADCAST) buf;
        
		RETAILMSG(1, (TEXT("%s: *** PowerNotification @ Tick:%u, Count:%d***\r\n"), pszFname,
            GetTickCount(), dwCount++));

		getDateTimeString(b, sizeof(b));
		memset(wb, 0, sizeof(wb));
		mbstowcs(wb, b, strlen(b));

        switch (pB->Message) 
        {
        case PBT_TRANSITION:
			RETAILMSG(1, (TEXT("%s:\tPBT_TRANSITION to system power state: '%s'\r\n"), pszFname, pB->SystemPowerState));
            switch (POWER_STATE(pB->Flags)) {
            case POWER_STATE_ON:
				WriteLog(gbLog, NULL, "POWER_STATE_ON");
				RETAILMSG(1, (TEXT("%s:\tPOWER_STATE_ON at %s\r\n"), pszFname, wb));
				break;
            case POWER_STATE_OFF:
				WriteLog(gbLog, NULL, "POWER_STATE_OFF");
				RETAILMSG(1, (TEXT("%s:\tPOWER_STATE_OFF at %s\r\n"), pszFname, wb));
                break;
			case POWER_STATE_CRITICAL:
				WriteLog(gbLog, NULL, "POWER_STATE_CRITICAL");
				RETAILMSG(1, (TEXT("%s:\tPOWER_STATE_CRITICAL at %s\r\n"), pszFname, wb));
                break;
			case POWER_STATE_BOOT:
				WriteLog(gbLog, NULL, "POWER_STATE_BOOT");
				RETAILMSG(1, (TEXT("%s:\tPOWER_STATE_BOOT at %s\r\n"), pszFname, wb));
                break;
			case POWER_STATE_IDLE:
				WriteLog(gbLog, NULL, "POWER_STATE_IDLE");
				RETAILMSG(1, (TEXT("%s:\tPOWER_STATE_IDLE at %s\r\n"), pszFname, wb));
                break;
			case POWER_STATE_SUSPEND:
				WriteLog(gbLog, NULL, "POWER_STATE_SUSPEND");
				RETAILMSG(1, (TEXT("%s:\tPOWER_STATE_SUSPEND at %s\r\n"), pszFname, wb));
                break;
			case POWER_STATE_RESET:
				WriteLog(gbLog, NULL, "POWER_STATE_RESET");
				RETAILMSG(1, (TEXT("%s:\tPOWER_STATE_RESET at %s\r\n"), pszFname, wb));
                break;
			case 0:
				WriteLog(gbLog, NULL, "POWER_STATE_IDLE");
				RETAILMSG(1, (TEXT("%s:\tPOWER_STATI_IDLE at %s\r\n"), pszFname, wb)); // should to IDLE
				break;
            default:
                RETAILMSG(1,(TEXT("%s:\tUnknown Power State Flags:0x%x\r\n"), pszFname, pB->Flags));
                ASSERT(0);
                break;
            }
            break;
            
        case PBT_RESUME:
			WriteLog(gbLog, NULL, "POWER_STATE_RESUME");
            RETAILMSG(1, (TEXT("%s:\tPBT_RESUME at %s\r\n"), pszFname, wb));
            break;
                
        case PBT_POWERSTATUSCHANGE:
            RETAILMSG(1, (TEXT("%s:\tPBT_POWERSTATUSCHANGE at %s\r\n"), pszFname, wb));
            break;
                
        case PBT_POWERINFOCHANGE:  {
			PPOWER_BROADCAST_POWER_INFO ppbpi = (PPOWER_BROADCAST_POWER_INFO) pB->SystemPowerState;
			RETAILMSG(1, (TEXT("%s:\tPBT_POWERINFOCHANGE\r\n"), pszFname));
			#if 0
			RETAILMSG(1, (TEXT("%s:\t\tAC line status %u, battery flag %u, backup flag %u, %d levels\r\n"), pszFname,
				ppbpi->bACLineStatus, ppbpi->bBatteryFlag, ppbpi->bBackupBatteryFlag, ppbpi->dwNumLevels));
			RETAILMSG(1, (TEXT("%s:\t\tbattery life %d, backup life %d\r\n"), pszFname,
				ppbpi->bBatteryLifePercent, ppbpi->bBackupBatteryLifePercent));
			RETAILMSG(1, (TEXT("%s:\t\tlifetime 0x%08x, full lifetime 0x%08x\r\n"), pszFname,
				ppbpi->dwBatteryLifeTime, ppbpi->dwBatteryFullLifeTime));
			RETAILMSG(1, (TEXT("%s:\t\tbackup lifetime 0x%08x, backup full lifetime 0x%08x\r\n"), pszFname, 
				ppbpi->dwBackupBatteryLifeTime, ppbpi->dwBackupBatteryFullLifeTime));
			#endif
			}
			break;
		default:
			RETAILMSG(1, (TEXT("%s:\tUnknown Message:%d\r\n"), pszFname, pB->Message));
			ASSERT(0);
			break;
        }
#if 1
        RETAILMSG(1, (TEXT("%s:\tMessage: 0x%x\r\n"), pszFname, pB->Message));
        RETAILMSG(1, (TEXT("%s:\tFlags: 0x%x\r\n"), pszFname, pB->Flags));
        RETAILMSG(1, (TEXT("%s:\tdwLen: %d\r\n"), pszFname, pB->Length));
#endif
        RETAILMSG(1, (TEXT("%s:***********************\r\n"), pszFname));
    } else {
        RETAILMSG(1, (TEXT("%s:\tReceived short message: %d bytes\r\n"), pszFname, iBytesInQueue));
        ASSERT(0);
    }
}

int WINAPI MonThreadProc(LPVOID pvParam)
{
    HANDLE hEvents[2];
    LPTSTR pszFname = _T("PMMON!MonThreadProc");

    UNREFERENCED_PARAMETER(pvParam);

    ASSERT(ghPowerNotifications != NULL);
    ASSERT(ghevTerminate != NULL);

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    hEvents[0] = ghPowerNotifications;
    hEvents[1] = ghevTerminate;

    while (1) 
    {
        DWORD dwStatus;

        // Block on our message queue.
        // This thread runs when the power manager writes a notification into the queue.
        dwStatus = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
        if(dwStatus == WAIT_OBJECT_0) {
            ProcessPowerNotification(ghPowerNotifications);
        } else if(dwStatus == (WAIT_OBJECT_0 + 1)) {
            RETAILMSG(1, (TEXT("%s: termination event signaled\r\n"), pszFname));
        } else {
            RETAILMSG(1, (TEXT("%s: WaitForMultipleObjects returned %d (error %d)\r\n"), pszFname,
                dwStatus, GetLastError()));
            break;
        }
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmShow)
{
    HANDLE hNotifications = NULL;
    HANDLE ht;	
    configuration config;
    LPTSTR pszFname = _T("PMEvent:WinMain");
	WCHAR  wszINIFileName[MAX_PATH];
	WCHAR  wszLogFileName[MAX_PATH];
	WCHAR  wszPath[MAX_PATH];
	char inifile[MAX_PATH];
	SYSTEMTIME Now;
	
    
    MSGQUEUEOPTIONS msgOptions = {0};   

    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(hPrevInst);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmShow);

    // clear globals
	if (TRUE == GetFullPath(wszPath, MAX_PATH)) {
		wcscpy(gFullPath, wszPath);
	}
	if (TRUE == GetINIFileName(wszINIFileName, MAX_PATH)) {
		wcscpy(gINIFileName, wszPath);
		wcscat(gINIFileName, L"\\");
		wcscat(gINIFileName, wszINIFileName);
	}
	if (TRUE == GetLogFileName(wszLogFileName, MAX_PATH)) {
		wcscpy(gLogFileName, wszPath);
		wcscat(gLogFileName, L"\\");
		wcscat(gLogFileName, wszLogFileName);
	}

	if (FALSE == PathFileExists(wszPath)) {
		if (FALSE == CreateDirectory(wszPath, NULL)) {
			RETAILMSG(TRUE, (TEXT("Failed: Create %s directory\r\n"), wszPath));
		}
		else {
			RETAILMSG(TRUE, (TEXT("Success: Create %s directory\r\n"), wszPath));
		}
	}

	wcstombs(inifile, gINIFileName, wcslen(gINIFileName));
    if (ini_parse(inifile, handler, &config) < 0) {
		RETAILMSG(TRUE, (TEXT("Cannot Open %s\r\n"), gINIFileName));
		gbLog=FALSE;
    } else {
		gbLog = config.activate;
	}

	GetLocalTime(&Now);
	GetLastRecordDateTime();
	if (compareSystemTimeIn3sec(&Now, &lastRecordDateTime)) {
		const char msg[] = "First BootUp";
		WriteLog(gbLog, &lastRecordDateTime, msg);
	}
	else {
		const char msg[] = "System BootUp";
		WriteLog(gbLog, &lastRecordDateTime, msg);
	}
	
    // create a termination event
    ghevTerminate = CreateEvent(NULL, FALSE, FALSE, _T("SOFTWARE/PMTestPrograms/PMMON/Terminate"));
    if(ghevTerminate == NULL) {
        RETAILMSG(TRUE, (_T("%s: CreateEvent() failed %d for termination event\r\n"),
            pszFname, GetLastError()));
        goto _Exit;
    }

    // did the event already exist?
    if(GetLastError() == ERROR_ALREADY_EXISTS) {
        // yes, kill the existing process
        RETAILMSG(TRUE, (_T("%s: Signaling termination event\r\n"), pszFname));
        //SetEvent(ghevTerminate);
        goto _Exit;
    }
    
    // create a message queue for Power Manager notifications
    msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
    msgOptions.dwFlags = 0;
    msgOptions.dwMaxMessages = QUEUE_ENTRIES;
    msgOptions.cbMaxMessage = sizeof(POWER_BROADCAST) + MAX_NAMELEN;
    msgOptions.bReadAccess = TRUE;

    ghPowerNotifications= CreateMsgQueue(NULL, &msgOptions);
    if (!ghPowerNotifications) {
        DWORD dwErr = GetLastError();
        RETAILMSG(1, (TEXT("%s:CreateMessageQueue ERROR:%d\r\n"), pszFname, dwErr));
        goto _Exit;
    }

    // request Power notifications
    hNotifications = RequestPowerNotifications(ghPowerNotifications, POWER_NOTIFY_ALL); // Flags
    if (!hNotifications) {
        DWORD dwErr = GetLastError();
        RETAILMSG(TRUE, (TEXT("%s:RequestPowerNotifications ERROR:%d\r\n"), pszFname, dwErr));
        goto _Exit;
    }

    // create the monitoring thread
    ht = CreateThread(NULL, 0, MonThreadProc, NULL, 0, NULL);
    if(ht) {
        // wait for the thread to exit
        WaitForSingleObject(ht, INFINITE);
        CloseHandle(ht);
    }
_Exit:
    if(hNotifications) StopPowerNotifications(hNotifications);
    if(ghPowerNotifications) CloseMsgQueue(ghPowerNotifications);
    if(ghevTerminate) CloseHandle(ghevTerminate);
    RETAILMSG(TRUE, (_T("%s: exiting\r\n"), pszFname));

    return 0;
}

// EOF
