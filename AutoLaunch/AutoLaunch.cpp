// AutoLaunch.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "winsock2.h"
#include "pkfuncs.h"
#include "storemgr.h"

#define MAX_APPSTART_KEYNAME 128

void WalkStartupKey();
DWORD WINAPI ProcessThread(LPVOID lpParameter);

typedef struct _ProcessStruct {
    WCHAR szName[MAX_APPSTART_KEYNAME];
    DWORD dwDelay;
} PROCESS_STRUCT,*LPPROCESS_STRUCT;


#define LENGTH_WAIT_FOR_USB_READY  500 // 1 second
#define MAX_NUM_WAITS_FOR_USB_READY 16 // 10 tries

// Local module functions:
static BOOL IsUSBReady(void);

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR lpCmdLine,
                     int nCmdShow)
{
    for (int i = 0; i < MAX_NUM_WAITS_FOR_USB_READY; i++)
    {
        if (IsUSBReady())
        {
            break;
        }
        else
        {
            Sleep(LENGTH_WAIT_FOR_USB_READY);
        }
    }

    WalkStartupKey();
    SignalStarted(_wtol(lpCmdLine));

    // RETURN - RETURN: The USB was not ready.
    return (-2);
}

BOOL IsUSBReady()
{
    HANDLE hFile;
    TCHAR szFilePath[] = TEXT("\\Hard Disk\\Diagnostic.exe");
    WIN32_FIND_DATA data;

    if ((hFile = FindFirstFile(szFilePath, &data)) != INVALID_HANDLE_VALUE)
    {
        FindClose(hFile);
        RETAILMSG(1, (TEXT("PASS : FindFirstFile('%s').\r\n"), szFilePath));
        return (TRUE);
    }
    else
    {
        RETAILMSG(1, (TEXT("FAIL : FindFirstFile('%s').\r\n"), szFilePath));
        return (FALSE);
    }
}

void WalkStartupKey()
{
    HKEY   hKey;
    WCHAR  szName[MAX_APPSTART_KEYNAME];
    WCHAR  szVal[MAX_APPSTART_KEYNAME];
    WCHAR  szDelay[MAX_APPSTART_KEYNAME];
    DWORD  dwType, dwNameSize, dwValSize, i, dwDelay, dwBytesRead;
    HANDLE hFile;
    CHAR   bBuffer[6];

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Startup"), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return;
    }

    dwNameSize = MAX_APPSTART_KEYNAME;
    dwValSize = MAX_APPSTART_KEYNAME * sizeof(WCHAR);
    i = 0;
    while (RegEnumValue(hKey, i, szName, &dwNameSize, 0, &dwType,(LPBYTE)szVal, &dwValSize) == ERROR_SUCCESS) {
        if ((dwType == REG_SZ) && !wcsncmp(szName, TEXT("Process"), 7)) { // 7 for "Process"
            hFile = CreateFile(szVal,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
				DWORD dwPtr = SetFilePointer(hFile, -6, NULL, FILE_END);
				DWORD fRun = FALSE;
				
				if (0xFFFFFFFF != dwPtr) {
					fRun = TRUE;
				} else {
					DWORD dwError = GetLastError();
					RETAILMSG(1, (TEXT("Last Error 0x%x(%d\r\n"), dwError));
					if (0 == dwError) {
						fRun = TRUE;
					}
				}
				
				if (TRUE == fRun) {
                    if (ReadFile(hFile, bBuffer, 6, &dwBytesRead, NULL)) {
                        CloseHandle(hFile);
                        if (!wcscmp(szVal, TEXT("\\ResidentFlash\\YourStartApp.exe"))) {
                            // query delay time
                            wsprintf(szDelay,L"%sDelay",szName);
                            dwValSize=sizeof(dwDelay);
                            if (ERROR_SUCCESS == RegQueryValueEx(hKey,szDelay,0,&dwType,(LPBYTE)&dwDelay,&dwValSize)) {
                                // we now have the process name and the process delay - spawn a thread to "Sleep" and then create the process.
                                LPPROCESS_STRUCT ps=new PROCESS_STRUCT;
                                ps->dwDelay=dwDelay;
                                wcscpy(ps->szName,szVal);

                                DWORD dwThreadID;
                                OutputDebugString(L"Creating Thread...\r\n");
	
                                HANDLE hThread=CreateThread(NULL,0,ProcessThread,(LPVOID)ps,0,&dwThreadID);
                                WaitForSingleObject(hThread,INFINITE);
                                CloseHandle(hThread);
                            }
                        }
						else
						{
							RETAILMSG(1, (TEXT("\r\n\r\nAutoLaunch: compare %s failed!\r\n"), szVal));
						}
                    }
					else
					{
						RETAILMSG(1, (TEXT("\r\n\r\nAutoLaunch: Failed ReadFile\r\n")));
					}
                }
				else
				{
					RETAILMSG(1, (TEXT("\r\n\r\nAutoLaunch: Failed SetFilePointer\r\n")));
				}
            } else {
				RETAILMSG(1, (TEXT("\r\n\r\n[AutoLaunch]:Open File %s Failed\r\n"), szVal));
			}
        }
        
        dwNameSize = MAX_APPSTART_KEYNAME;
        dwValSize = MAX_APPSTART_KEYNAME * sizeof(WCHAR);
        i++;
    }

    RegCloseKey(hKey);
}

DWORD WINAPI ProcessThread(LPVOID lpParameter)
{
    TCHAR tcModuleName[MAX_APPSTART_KEYNAME];

    OutputDebugString(L"Thread Created... Sleeping\r\n");
    LPPROCESS_STRUCT ps=(LPPROCESS_STRUCT)lpParameter;

    Sleep(ps->dwDelay); // Wait for delay period
    OutputDebugString(L"Done Sleeping...\r\n");

    PROCESS_INFORMATION pi;
    OutputDebugString(L"Creating Process ");
    OutputDebugString(ps->szName);
    OutputDebugString(L"\r\n");

    wcscpy(tcModuleName,ps->szName);

    //TCHAR *tcPtrSpace=wcsrchr(ps->szName,L' '); // Launch command has a space, assume command line.
    //if (NULL != tcPtrSpace) {
    //  tcModuleName[lstrlen(ps->szName)-lstrlen(tcPtrSpace)]=0x00; // overwrite the space with null, break the app and cmd line.
    //  tcPtrSpace++; // move past space character.
    //}

    if (CreateProcess( tcModuleName, // Module Name    
            //tcPtrSpace,     // Command line -- NULL or PTR to command line
            NULL,           // Command line -- NULL or PTR to command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            NULL,           // Pointer to STARTUPINFO structure
            &pi )){         // Pointer to PROCESS_INFORMATION structure
        DWORD dwExitCode = STILL_ACTIVE;
        for (;;) {
            if (GetExitCodeProcess(pi.hProcess, &dwExitCode)) {
                if (dwExitCode != STILL_ACTIVE) {
                    break;
                }
                else {
                    Sleep(1000);
                }
            }
        }

        OutputDebugString(L"Thread Exiting...\r\n");
    }

    return 0;
}

