// DelRM.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

TCHAR logfilename[] = _T("\\ResidentFlash\\RunCounter.log");
TCHAR logFMT[] = _T("RunCounter:  %d");

int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	FILE* f  = NULL;
	int	counter = 0;
	TCHAR	tstr[512];
	
	f = _tfopen(logfilename, _T("r"));
	
	if ( f != NULL) 
	{
		_ftscanf(f, _T("%s %d"), tstr, &counter);
		fclose(f);
	}
	
	// add counter
	counter++;
	f = _tfopen(logfilename, _T("w+"));
	_ftprintf(f, logFMT, counter);
	fclose(f);	

    return 0;
}
