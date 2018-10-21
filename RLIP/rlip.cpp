
#include "stdafx.h"
#include "rlip.h"   // Adapter/configuration path setting
/*
IPCONFIG.INI EXAMPLE
; comment
[SMSC91181]  ; AdapterName
IP=192.168.1.10
MASK=255.255.255.0
GATEWAY=
DNS1=192.168.1.1
DNS2=192.168.1.2
WINS1=192.168.1.1
WINS2=192.168.1.2
*/
typedef struct
{
	DWORD mode;
	DWORD ip;
	DWORD mask;
	DWORD gateway;
	DWORD dns1;
	DWORD dns2;
	DWORD wins1;
	DWORD wins2;
	TCHAR name[256];
} configuration;

BOOL ConvIP(const char* str, DWORD* ip);
BOOL StringToAddr(TCHAR *AddressString, DWORD *AddressValue);
void IPAddrToStr (LPTSTR szStr, DWORD IPAddr);
BOOL IsValidIPAddress(DWORD IPAddr);
BOOL DoNdisIOControl( 
	DWORD	dwCommand,
	LPVOID	pInBuffer,
	DWORD	cbInBuffer,
	LPVOID	pOutBuffer,
	DWORD	*pcbOutBuffer	OPTIONAL);
BOOL AdapterReBind(const TCHAR* strAdapter);
void SetLANRegistry(configuration *adapterInfo);

#define net_long(x) (((((ulong)(x))&0xffL)<<24) | \
                     ((((ulong)(x))&0xff00L)<<8) | \
                     ((((ulong)(x))&0xff0000L)>>8) | \
                     ((((ulong)(x))&0xff000000L)>>24))
                     



static int handler(void* user, const char* section, const char* name,
				const char* value)
{
	char adapter[256];
	char *endptr = NULL;
	int ret = 1;


	configuration* pconfig = (configuration*)user;

	
	if (wcstombs(adapter, pconfig->name, 256) <= 0)
	{
		RETAILMSG(1, (TEXT("Failed! Convert adapter name %s to char adapter[]\r\n"), pconfig->name));
	}
	
	#define MATCH(s, n) _stricmp(section, s) == 0 && _stricmp(name, n) == 0
	if (MATCH(adapter, "mode")) {
		pconfig->mode = strtol(value, &endptr, 10);
	} else if (MATCH(adapter, "ip")) {
		ConvIP(value, &pconfig->ip);
	} else if (MATCH(adapter, "mask")) {
		ConvIP(value, &pconfig->mask);
	} else if (MATCH(adapter, "gateway")) {
		ConvIP(value, &pconfig->gateway);
	} else if (MATCH(adapter, "dns1")) {
		ConvIP(value, &pconfig->dns1);
	} else if (MATCH(adapter, "dns2")) {
		ConvIP(value, &pconfig->dns2);
	} else if (MATCH(adapter, "wins1")) {
		ConvIP(value, &pconfig->wins1);
	} else if (MATCH(adapter, "wins2")) {
		ConvIP(value, &pconfig->wins2);
	} else {
		/* unknown section/name, error */
		RETAILMSG(1, (TEXT("Unknown section/name\r\n")));
		ret = 0;
    }
    
	return ret;
}

BOOL ConvIP(const char* strip, DWORD* ip)
{
	TCHAR wszTmp[128];
	DWORD	dwTmp;
	size_t cnt = 0;
	
	cnt = mbstowcs(NULL, strip, 127);
	if (cnt <= 0 || cnt >= 64)
	{
		return FALSE;
	}
	
	mbstowcs(wszTmp, strip, 127);
	
    if (FALSE == StringToAddr(wszTmp, &dwTmp))
	{
		return FALSE;
	}
	
	if (TRUE == IsValidIPAddress(dwTmp))
	{
		*ip = dwTmp;
	}

	return TRUE;
}

BOOL AdapterReBind(const TCHAR* strAdapter)
{
	return DoNdisIOControl(IOCTL_NDIS_REBIND_ADAPTER, (LPVOID)strAdapter, _tcslen(strAdapter)+1, NULL, 0);
}


int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	static const TCHAR AdapterName[] = TEXT(ADAPTER_NAME);
	static const char conf_path[] = RLIP_CONF;

	configuration config = { 0 };

	memset(&config,  0, sizeof(configuration));
	_tcscpy(config.name, AdapterName);
	RETAILMSG(1, (TEXT("[RLIP]Prepare read ipconfig.ini\r\n")));
	if (ini_parse(conf_path, handler, &config) < 0) {
		// cannot read
		RETAILMSG(1, (TEXT("[RLIP]Inexist ipconfig.ini\r\n")));
		SignalStarted( _wtol(GetCommandLine()) );
		return 1;
	}
	
	SetLANRegistry(&config);
	AdapterReBind(AdapterName);
	RETAILMSG(1, (TEXT("[RLIP]Complete! reload static ip for %s\r\n"), AdapterName));
	SignalStarted( _wtol(GetCommandLine()) );
	return 0;
}

void ConvIp2Str(TCHAR* szIP, DWORD* Len, DWORD IP)
{
	if (IsValidIPAddress(IP))
	{
	  IPAddrToStr(szIP, IP);
	  *Len = _tcslen(szIP) + 1;
	}
	else
	{
		_tcscpy(szIP, TEXT(""));
		*Len = 0;
	}
}

void SetLANRegistry(configuration *adapterInfo)
{
	TCHAR	szTemp[256];
	HKEY	hKey;
	LONG	hRes;
	DWORD	dwDisp;
	DWORD	Len;
	DWORD	fUseDHCP = 0; // 1, enabled DHCP, 0 is use StaticIP
	
	// Get the current registry data.
	_tcscpy (szTemp, TEXT("Comm\\"));
	_tcscat (szTemp, adapterInfo->name);
	_tcscat (szTemp, TEXT("\\Parms\\TcpIp"));
	fUseDHCP = adapterInfo->mode;

	hRes = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szTemp, 0, NULL,
						REG_OPTION_NON_VOLATILE, 0, NULL,
						&hKey, &dwDisp);
	if (hRes != ERROR_SUCCESS) {
		RETAILMSG ( 1,(TEXT("!Unable to create reg key '%s'\r\n"),szTemp));
	}

	RegSetValueEx (hKey, TEXT("EnableDHCP"), 0, REG_DWORD,
				(BYTE*)&(fUseDHCP), sizeof(DWORD));

	if (fUseDHCP == 0)
	{
		ConvIp2Str(szTemp, &Len, adapterInfo->ip);
		szTemp[Len++] = TEXT('\0');
		RegSetValueEx (hKey, TEXT("IpAddress"), 0, REG_MULTI_SZ,
				(BYTE*)szTemp, sizeof(TCHAR)*Len);


		ConvIp2Str(szTemp, &Len, adapterInfo->mask);
		szTemp[Len++] = TEXT('\0');
		RegSetValueEx (hKey, TEXT("Subnetmask"), 0, REG_MULTI_SZ,
				(BYTE*)szTemp, sizeof(TCHAR)*Len);

		ConvIp2Str(szTemp, &Len, adapterInfo->gateway);
		szTemp[Len++] = TEXT('\0');
		RegSetValueEx (hKey, TEXT("DefaultGateway"), 0, REG_MULTI_SZ,
				(BYTE*)szTemp, sizeof(TCHAR)*Len);
	}

	if (TRUE == IsValidIPAddress(adapterInfo->dns1))
	{
		ConvIp2Str(szTemp, &Len, adapterInfo->dns1);
		if (TRUE == IsValidIPAddress(adapterInfo->dns2))
		{
			DWORD Tmp = 0;
			ConvIp2Str(szTemp+Len, &Tmp, adapterInfo->dns2);
			Len += Tmp;
			szTemp[Len++] = TEXT('\0');
		}
		RETAILMSG(1, (TEXT("\r\n")));
	} else if (TRUE == IsValidIPAddress(adapterInfo->dns2))
	{
		ConvIp2Str(szTemp, &Len, adapterInfo->dns2);
		szTemp[Len++] = TEXT('\0');
	} else {
		_tcscpy(szTemp, TEXT(""));
		szTemp[1] = TEXT('\0');
		szTemp[2] = TEXT('\0');
		Len=1;
	}
	RegSetValueEx (hKey, TEXT("DNS"), 0, REG_MULTI_SZ,
				(BYTE*)szTemp, sizeof(TCHAR)*Len);

	if (TRUE == IsValidIPAddress(adapterInfo->wins1))
	{
		ConvIp2Str(szTemp, &Len, adapterInfo->wins1);
		if (TRUE == IsValidIPAddress(adapterInfo->wins2))
		{
			DWORD Tmp = 0;
			ConvIp2Str(szTemp+Len, &Tmp, adapterInfo->wins2);
			Len += Tmp;
			szTemp[Len++] = TEXT('\0');
		}
		RETAILMSG(1, (TEXT("\r\n")));
	} else if (TRUE == IsValidIPAddress(adapterInfo->wins2))
	{
		ConvIp2Str(szTemp, &Len, adapterInfo->wins2);
		szTemp[Len++] = TEXT('\0');
	} else {
		_tcscpy(szTemp, TEXT(""));
		szTemp[1] = TEXT('\0');
		szTemp[2] = TEXT('\0');
		Len=1;
	}
	RegSetValueEx (hKey, TEXT("WINS"), 0, REG_MULTI_SZ,
				(BYTE*)szTemp, sizeof(TCHAR)*Len);

	if (hKey) {
		RegCloseKey (hKey);
	}
}


#define INADDR_ANY              0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        0xffffffff

//
// Make sure the specified static IP address is valid
//
BOOL
IsValidIPAddress(
    DWORD IPAddr
    )
{
    switch (IPAddr) {
    case INADDR_ANY:
    case INADDR_LOOPBACK:
    case INADDR_BROADCAST:
        return FALSE;
    }

    return TRUE;
}

void IPAddrToStr (LPTSTR szStr, DWORD IPAddr)
{
	wsprintf (szStr, TEXT("%d.%d.%d.%d"),
			(IPAddr >> 24) & 0xFF, (IPAddr >> 16) & 0xFF,
			(IPAddr >> 8) & 0xFF,  IPAddr & 0xFF);
}

BOOL StringToAddr(TCHAR *AddressString, DWORD *AddressValue) 
{
	TCHAR	*pStr = AddressString;
	PUCHAR	AddressPtr = (PUCHAR)AddressValue;
	int		i;
	int		Value;

	// Parse the four pieces of the address.
	for (i=0; *pStr && (i < 4); i++) {
		Value = 0;
		while (*pStr && TEXT('.') != *pStr) {
			if ((*pStr < TEXT('0')) || (*pStr > TEXT('9'))) {
				RETAILMSG (1,
						(TEXT("Unable to convert %s to address\r\n"),
						AddressString));
				return FALSE;
			}
			Value *= 10;
			Value += *pStr - TEXT('0');
			pStr++;
		}
		if (Value > 255) {
			RETAILMSG (1,
					(TEXT("Unable to convert %s to address\r\n"),
					AddressString));
			return FALSE;
		}
		AddressPtr[i] = Value;
		if (TEXT('.') == *pStr) {
			pStr++;
		}
	}

	// Did we get all of the pieces?
	if (i != 4) {
		RETAILMSG (1,
				(TEXT("Unable to convert %s to address\r\n"),
				AddressString));
		return FALSE;
	}

	*AddressValue = net_long (*AddressValue);

	return TRUE;
}	// StringToAddr()


BOOL
DoNdisIOControl(
	DWORD	dwCommand,
	LPVOID	pInBuffer,
	DWORD	cbInBuffer,
	LPVOID	pOutBuffer,
	DWORD	*pcbOutBuffer	OPTIONAL)
//
//	Execute an NDIS IO control operation.
//
{
	HANDLE	hNdis;
	BOOL	bResult = FALSE;
	DWORD	cbOutBuffer;

	hNdis = CreateFile(DD_NDIS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_ALWAYS, 0, NULL);

	if (hNdis != INVALID_HANDLE_VALUE)
	{
		cbOutBuffer = 0;
		if (pcbOutBuffer)
			cbOutBuffer = *pcbOutBuffer;

		bResult = DeviceIoControl(hNdis,
								 dwCommand,
								 pInBuffer,
								 cbInBuffer,
								 pOutBuffer,
								 cbOutBuffer,
								 &cbOutBuffer,
								 NULL);

		if (bResult == FALSE) 
		{
			RETAILMSG(1, (TEXT("IoControl result=%d"), bResult));
		}

		if (pcbOutBuffer)
			*pcbOutBuffer = cbOutBuffer;

		CloseHandle(hNdis);
	}
	else
	{
		RETAILMSG(1, (TEXT("[RLIP]CreateFile of '%s' failed, error=%d"), DD_NDIS_DEVICE_NAME, GetLastError()));
	}


	return bResult;
}