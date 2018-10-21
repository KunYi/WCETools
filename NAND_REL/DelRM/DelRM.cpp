// DelRM.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	DeleteFile(_T("\\ResidentFlash\\MeetingManagementSetup\\RoomManager.exe"));
    return 0;
}
