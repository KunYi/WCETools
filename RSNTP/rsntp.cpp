
#include "stdafx.h"
#include "rsntp.h"   // Adapter/configuration path setting
#define B_DBG  (FALSE)
#define MAX_SERVER_LEN	(2048)
/*
RSNTP.INI EXAMPLE
; comment
[RSNTP]
SERVER=tock.usno.navy.mil, time.windows.com
;
AUTOUPDATE= 1 ; DWORD, 1 is enabled, 0 is disable
ServerRole= 1 ; DWORD,
REFRESH=1209600000 ; DWORD two week
RecoveryRefresh=86400000 ; DWORD default 1 day
THRESHOLD=86400000 ; DWORD, default = 1day = 24(hour)*60(min)*60(sec)*1000 = 86400000
TRUSTLOCALCLOCK=0 ; 
*/

WCHAR	gFullPath[MAX_PATH];
WCHAR	gINIFileName[MAX_PATH];

#define NO_IMPLEMENT  (0)
typedef struct {
	WCHAR	server[MAX_SERVER_LEN];
	DWORD	serverRole;
	DWORD	autoUpdate;
	DWORD	refresh;
	DWORD	recoveryRefresh;
	DWORD	threshold;
	DWORD	trustlocalclock;
} configuration;

void SetRegistry(configuration *config);
static int handler(void* user, const char* section, const char* name,
				const char* value);

static BOOL GetFullPath(LPWSTR path, DWORD len)
{
	TCHAR	szTemp[256];
	HKEY	hKey;
	LONG	hRes;
	DWORD	dwDisp;
	DWORD	Len;
	DWORD	dwType;
	BOOL	ret = FALSE;

	// Get the current registry data.
	_tcscpy (szTemp, TEXT("SOFTWARE\\RSNTP"));	
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

static BOOL GetINIFileName(LPWSTR path, DWORD len)
{
	TCHAR	szTemp[256];
	HKEY	hKey;
	LONG	hRes;
	DWORD	dwDisp;
	DWORD	Len;
	DWORD	dwType;
	BOOL	ret = FALSE;

	// Get the current registry data.
	_tcscpy (szTemp, TEXT("SOFTWARE\\RSNTP"));	
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

static void initConfig(configuration* config)
{
	const WCHAR def_server[] = L"tock.usno.navy.mil,time.windows.com";
	wcscpy(config->server, def_server);
	config->serverRole = 1;
	config->autoUpdate = 1;
	config->refresh = 7*24*60*60*1000;			// 1 weekly
	config->recoveryRefresh = 24*60*60*1000;	// 1day
	config->threshold = 24*60*60*1000;			// 1day
	config->trustlocalclock = 0;
}

static void dumpConfig(configuration* config)
{
	RETAILMSG(1, (TEXT("[RSNTP] dump configuration\r\n")));
	RETAILMSG(1, (TEXT("  server:%s\r\n"), config->server));
	RETAILMSG(1, (TEXT("  serverRole:%d, autoUpdate:%d\r\n"), 
		config->serverRole, config->autoUpdate));
	RETAILMSG(1, (TEXT("  Refresh:0x%x(%d), RecoveryRefresh:0x%x(%d)\r\n"),
		config->refresh, config->refresh,
		config->recoveryRefresh, config->recoveryRefresh));
	RETAILMSG(1, (TEXT("  Threshold:0x%x(%d)\r\n"), config->threshold, config->threshold));
	RETAILMSG(1, (TEXT("  TrustLocalClock:%d\r\n"), config->trustlocalclock));
}

static void dumpMultiSZ(WCHAR* szTemp, DWORD len) 
{
	DWORD i = 0;
	DWORD j = 1;

	while(i < len) {
		if ((szTemp[i] == L'\0') && (szTemp[i+1] == L'\0')) {
			RETAILMSG(1, (TEXT("\r\nend of string n:%d, start next\r\n"), j++));
			i++;
		}
		RETAILMSG(1, (TEXT("%c"), szTemp[i]));
		i++;
	}
	RETAILMSG(1, (TEXT("\r\n")));
}

void RefreshSNTP(void)
{
	HANDLE hFile=CreateFile(L"NTP0:",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(INVALID_HANDLE_VALUE==hFile)
		return;
	DeviceIoControl(hFile, IOCTL_SERVICE_REFRESH, NULL, 0, NULL, 0, NULL, NULL);
	CloseHandle(hFile);
}

#if 0
void SyncServer(void)
{
	HANDLE hFile=CreateFile(L"NTP0:",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	if(INVALID_HANDLE_VALUE==hFile)
		return;

	WCHAR szControlString[]=L"sync";
	
	DWORD dwLenIn=sizeof(szControlString);
	DWORD dwBytesReturned;

	DeviceIoControl(hFile, IOCTL_SERVICE_CONTROL, szControlString, dwLenIn, NULL, 0, &dwBytesReturned, NULL);
	CloseHandle(hFile);
}
#endif

int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	//static const char conf_path[] = RSNTP_CONF;
	WCHAR	wszINIFileName[MAX_PATH];
	WCHAR	wszPath[MAX_PATH];
	char	inifile[MAX_PATH];

	configuration config = { 0 };

	initConfig(&config);
    // clear globals
	if (TRUE == GetFullPath(wszPath, MAX_PATH)) {
		wcscpy(gFullPath, wszPath);
	}
	if (TRUE == GetINIFileName(wszINIFileName, MAX_PATH)) {
		wcscpy(gINIFileName, wszPath);
		wcscat(gINIFileName, L"\\");
		wcscat(gINIFileName, wszINIFileName);
	}

	RETAILMSG(1, (TEXT("[RSNTP]Prepare read rsntp.ini, %s\r\n"), gINIFileName));
	wcstombs(inifile, gINIFileName, wcslen(gINIFileName));
	if (B_DBG) dumpConfig(&config);	
	if (ini_parse(inifile, handler, &config) < 0) {
		// cannot read
		RETAILMSG(1, (TEXT("[RSNTP]Inexist rsntp.ini\r\n")));
		SignalStarted( _wtol(GetCommandLine()) );
		return 1;
	}
	if (B_DBG) dumpConfig(&config);
	SetRegistry(&config);
	RefreshSNTP();
	RETAILMSG(1, (TEXT("[RSNTP]Complete! reload SNTP Server setting\r\n")));
	SignalStarted( _wtol(GetCommandLine()) );
	return 0;
}


void SetRegistry(configuration *config)
{
	TCHAR	szTemp[4096];
	WCHAR*	pT;
	HKEY	hKey;
	LONG	hRes;
	DWORD	dwDisp;
	DWORD	Len;
	BOOL	bMULTI_SZ = FALSE;

	if (B_DBG) RETAILMSG(1, (TEXT("+SetRegistry() \r\n")));
	// Get the current registry data.
	_tcscpy (szTemp, TEXT(REG_PATH_SNTP_SERVICE));
	hRes = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, NULL,
						REG_OPTION_NON_VOLATILE, 0, NULL,
						&hKey, &dwDisp);
	if (hRes != ERROR_SUCCESS) {
		RETAILMSG ( 1,(TEXT("!Unable to create reg key '%s'\r\n"),szTemp));
	}

	Len = 0;
	pT = config->server;
	while (*pT != L'\0') {
		if (*pT == L',') {
			szTemp[Len++] = L'\0';
			bMULTI_SZ = TRUE;
		}
		else if (*pT == L' ') {
			// do nothing, just skip
		} else {
			szTemp[Len++] = *pT;
		}
		pT++;
	}
	szTemp[Len] = L'\0';
	szTemp[Len+1] = L'\0';

	if (bMULTI_SZ == TRUE) {
		RegSetValueEx (hKey, TEXT("server"), 0, REG_MULTI_SZ,
				(BYTE*)szTemp, sizeof(TCHAR)*Len);
		if (B_DBG) dumpMultiSZ(szTemp, Len);
	} else {
		RegSetValueEx (hKey, TEXT("server"), 0, REG_SZ, 
			(BYTE*)szTemp, sizeof(TCHAR)*Len);
		if (B_DBG) RETAILMSG(1, (TEXT("Server setting is String(%s)\r\n"), szTemp));
	}

	RegSetValueEx (hKey, TEXT("ServerRole"), 0, REG_DWORD,
				(BYTE*)&(config->serverRole), sizeof(DWORD));
	RegSetValueEx (hKey, TEXT("AutoUpdate"), 0, REG_DWORD,
				(BYTE*)&(config->autoUpdate), sizeof(DWORD));
	RegSetValueEx (hKey, TEXT("refresh"), 0, REG_DWORD,
				(BYTE*)&(config->refresh), sizeof(DWORD));
	RegSetValueEx (hKey, TEXT("recoveryrefresh"), 0, REG_DWORD,
				(BYTE*)&(config->recoveryRefresh), sizeof(DWORD));
	RegSetValueEx (hKey, TEXT("threshold"), 0, REG_DWORD,
				(BYTE*)&(config->threshold), sizeof(DWORD));
	RegSetValueEx (hKey, TEXT("trustlocalclock"), 0, REG_DWORD,
				(BYTE*)&(config->trustlocalclock), sizeof(DWORD));

	if (hKey) {
		RegCloseKey (hKey);
	}
	if (B_DBG) RETAILMSG(1, (TEXT("-SetRegistry()\r\n")));
}


static int handler(void* user, const char* section, const char* name,
				const char* value)
{
	const char msection[] = "RSNTP";
	char *endptr = NULL;
	int ret = 1;

	configuration* pconfig = (configuration*)user;
	
	#define MATCH(s, n) _stricmp(section, s) == 0 && _stricmp(name, n) == 0
	if (MATCH(msection, "server")) {
		int len = mbstowcs(NULL, value, MAX_SERVER_LEN);
		if ( len <= 0) {
			RETAILMSG(1, (TEXT("Can't conver server string to wchar_t\r\n")));
			ret = 0;
		} else if (len >= 2048) {
			RETAILMSG(1, (TEXT("Server string too long, must less than %d bytes\r\n"), MAX_SERVER_LEN));
			ret = 0;
		} else {
			mbstowcs(pconfig->server, value, MAX_SERVER_LEN);
			if (B_DBG) RETAILMSG(1, (TEXT("Server string is %s\r\n"), 
				pconfig->server));
		}
	} else if (MATCH(msection, "refresh")) {
		pconfig->refresh = strtol(value, &endptr, 10);
		if (B_DBG) RETAILMSG(1, (TEXT("get refresh 0x%x(%d)\r\n"), 
			pconfig->refresh, pconfig->refresh));
	} else if (MATCH(msection, "recoveryrefresh")) {
		pconfig->recoveryRefresh = strtol(value, &endptr, 10);
		if (B_DBG) RETAILMSG(1, (TEXT("get RecoveryRefresh 0x%x(%d)\r\n"), 
			pconfig->recoveryRefresh, pconfig->recoveryRefresh));
	} else if (MATCH(msection, "threshold")) {
		pconfig->threshold = strtol(value, &endptr, 10);
		if (B_DBG) RETAILMSG(1, (TEXT("get threshold 0x%x(%d)\r\n"), 
			pconfig->threshold, pconfig->threshold));
	} else if (MATCH(msection, "trustlocalclock")) {
		pconfig->trustlocalclock = strtol(value, &endptr, 10);
		if (B_DBG) RETAILMSG(1, (TEXT("get trustlocalclock 0x%x(%d)\r\n"), 
			pconfig->trustlocalclock, pconfig->trustlocalclock));
	} else if (MATCH(msection, "autoupdate")) {
		pconfig->autoUpdate = (strtol(value, &endptr, 10) != 0) ? 1:0;
		if (B_DBG) RETAILMSG(1, (TEXT("get autoupdate 0x%x(%d)\r\n"),
			pconfig->autoUpdate, pconfig->autoUpdate));
	} else if (MATCH(msection, "serverrole")) {
		pconfig->serverRole = (strtol(value, &endptr, 10) != 0) ? 1:0;
		if (B_DBG) RETAILMSG(1, (TEXT("get serverRole 0x%x(%d)\r\n"), 
			pconfig->serverRole, pconfig->serverRole));
	} else {
		/* unknown section/name, error */
		RETAILMSG(1, (TEXT("Unknown section/name\r\n")));
		ret = 0;
    }
	return ret;
}
