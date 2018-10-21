
#include "CE118eep.h"

BOOL SendE2pCmd(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD cmd)
{
	BOOL bRc;
	DWORD	dwOutSize;
	BYTE * p= &(queryOID->Data[0]);

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));

	*(DWORD *)p=cmd;

	queryOID->Oid = OID_NDIS_SMSC_SEND_E2P_COMMAND;
	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_SET_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		printf ( "Error in DeviceIoControl (OID_NDIS_SMSC_SEND_E2P_COMMAND) : %d\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL ReadE2p(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, WORD addr, BYTE *val)
{
	BOOL bRc;
	DWORD	dwOutSize;
	BYTE * p= &(queryOID->Data[0]);

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));

	*(WORD *)p=addr;

	queryOID->Oid = OID_NDIS_SMSC_READ_E2P;
	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_QUERY_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		printf ( "Error in DeviceIoControl (OID_NDIS_SMSC_READ_E2P) : %d\n", GetLastError());
		return FALSE;
	}

	*val = queryOID->Data[0];
	return TRUE;
}

BOOL WriteE2p(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, WORD addr, BYTE *val)
{
	BOOL bRc;
	BYTE * p= &(queryOID->Data[0]);
	DWORD	dwOutSize;

	// First Enable Eeprom Write
	if (SendE2pCmd(hAdapter,queryOID, E2P_CMD_EWEN) == FALSE)
	{
		printf ( "In WriteE2p(),Failed to send E2p Command E2P_CMD_EWEN\r\n");
		return FALSE;
	}

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));

	*(WORD *)p=addr;
	*(BYTE *)((WORD *)p+1)=*val;


	queryOID->Oid = OID_NDIS_SMSC_WRITE_E2P;
	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_SET_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		printf ( "Error in DeviceIoControl (IOCTL_LAN9118_MP_ACCESS) : %d\n", GetLastError());
		// Before exit, make sure to reset eeprom write enable
		SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWDS);
		return FALSE;
	}

	// Before exit, make sure to reset eeprom write enable
	if (SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWDS) == FALSE)
	{
		printf ( "In WriteE2p(),Failed to send E2p Command E2P_CMD_EWDS\n\n");
		return FALSE;
	}

	return TRUE;
}

BOOL ReadBlockE2p( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD length, BYTE *buf)
{
	BOOL bRc;
	BYTE * pE2pSize=&queryOID->Data[0];
	DWORD	dwOutSize;

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));
	*(DWORD *) pE2pSize=length;

	queryOID->Oid = OID_NDIS_SMSC_READ_E2P_BLOCK;
	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_QUERY_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		printf ( "Error in DeviceIoControl (OID_NDIS_SMSC_READ_E2P_BLOCK) : %d\n", GetLastError());
		return FALSE;
	}

	memcpy((void *)buf, (void *)&(queryOID->Data[0]), length);
	return TRUE;
}

BOOL WriteBlockE2p( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID,  DWORD length, BYTE *buf)
{
	BOOL bRc;
	DWORD	dwOutSize;


	// First Enable Eeprom Write
	if (SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWEN) == FALSE)
	{
		printf ( "In WriteBlockE2p, Error at SendE2pCmd(E2P_CMD_EWEN)");
		return FALSE;
	}

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));

	memcpy( (void *)&(queryOID->Data[0]), (void *)buf,length);

	queryOID->Oid = OID_NDIS_SMSC_WRITE_E2P_BLOCK;
	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_SET_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		printf ( "Error in DeviceIoControl (OID_NDIS_SMSC_WRITE_E2P_BLOCK) : %d\n", GetLastError());
		// Before exit, make sure to reset eeprom write enable
		SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWDS);
		return FALSE;
	}

	// Before exit, make sure to reset eeprom write enable
	if (SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWDS) == FALSE)
	{
		printf ( "In WriteBlockE2p, Error at SendE2pCmd(E2P_CMD_EWDS)");
		return FALSE;
	}
	return TRUE;
}

BOOL EraseE2p(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID)
{
	if ( SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWEN)==FALSE)
	{
		return FALSE;
	}
	if ( SendE2pCmd(hAdapter, queryOID,E2P_CMD_ERAL)==FALSE)
	{
		return FALSE;
	}
	if ( SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWDS)==FALSE)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL LanReadRegDW(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal)
{
	BOOL bRc;
	BYTE * p=&(queryOID->Data[0]);
	DWORD	dwOutSize;

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));
	*(DWORD *)p=dwOffset;

	queryOID->Oid = OID_NDIS_SMSC_LAN_READ_REGS;

	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_QUERY_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_READ_REGS) \n\n\n");
		return FALSE;
	}

	*dwVal = *((DWORD *)&queryOID->Data[0]);
	return TRUE;
}

BOOL LanWriteRegDW(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal)
{
	BOOL bRc;
	BYTE * p= &(queryOID->Data[0]);
	DWORD	dwOutSize;

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));
	*(DWORD *)p=dwOffset;
	*(DWORD *)((DWORD *)p+1)=*dwVal;

	queryOID->Oid = OID_NDIS_SMSC_LAN_WRITE_REGS;

	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_SET_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_WRITE_REGS) \n\n\n");
		return FALSE;
	}

   	return TRUE;
}

BOOL MacReadRegDW(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal)
{
	BOOL bRc;
	BYTE * p=&(queryOID->Data[0]);
	DWORD	dwOutSize;

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));
	*(DWORD *)p=dwOffset;

	queryOID->Oid = OID_NDIS_SMSC_MAC_READ_REGS;

	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_QUERY_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_READ_REGS) \n\n\n");
		return FALSE;
	}

	*dwVal = *((DWORD *)&queryOID->Data[0]);
	return TRUE;
}

BOOL MacWriteRegDW(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal)
{
	BOOL bRc;
	BYTE * p= &(queryOID->Data[0]);
	DWORD	dwOutSize;

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));
	*(DWORD *)p=dwOffset;
	*(DWORD *)((DWORD *)p+1)=*dwVal;

	queryOID->Oid = OID_NDIS_SMSC_MAC_WRITE_REGS;

	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_SET_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_WRITE_REGS) \n\n\n");
		return FALSE;
	}

	return TRUE;
}



BOOL PhyReadRegW( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *wVal)
{
	BOOL bRc;
	DWORD	dwOutSize;
	BYTE * p= &(queryOID->Data[0]);

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));
	*(DWORD *)p=dwOffset;

	queryOID->Oid = OID_NDIS_SMSC_PHY_READ_REGS;

	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_QUERY_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_PHY_READ_REGS) \n\n\n");
		return FALSE;
	}

	*wVal = *(DWORD *)&queryOID->Data[0];
	return TRUE;
}

BOOL PhyWriteRegW(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *wVal)
{
	BOOL bRc;
	BYTE * p= &(queryOID->Data[0]);
	DWORD	dwOutSize;

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));
	*(DWORD *)p=dwOffset;
	*(DWORD *)((DWORD *)p+1)=*wVal;

	queryOID->Oid = OID_NDIS_SMSC_LAN_WRITE_REGS;

	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_SET_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_WRITE_REGS) \n\n\n");
		return FALSE;
	}

	return TRUE;
}


BOOL GetDriverVersion(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID,void * pBuf)
{
	BOOL bRc;
	DWORD	dwOutSize;

	memset((void *)queryOID->Data,0,(sizeof(queryOID->Data)+sizeof(queryOID->buf)));

	queryOID->Oid = OID_NDIS_SMSC_GET_DRIVER_INFO;

	bRc=DeviceIoControl(hAdapter,
			IOCTL_NDISUIO_QUERY_OID_VALUE,
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			(LPVOID)queryOID,
			sizeof(NDISUIO_QUERY_OID) + sizeof(DWORD),
			&dwOutSize, NULL);

	if ( !bRc )
	{
		wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_GET_DRIVER_INFO) \n\n\n");
		return FALSE;
	}

	memcpy(pBuf, (void *)&queryOID->Data[0], sizeof(DWORD));

	return TRUE;
}
