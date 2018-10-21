//======================================================================
// TBIcons - Taskbar icon demonstration for Windows CE
//
// Written for the book Programming Windows CE
// Copyright (C) 2003 Douglas Boling
//======================================================================
#include <windows.h>                 // For all that Windows stuff
#include <CommCtrl.h>
#include "UpdateMACAddress.h"        // Program-specific stuff
#include "nand.h"
#include "resource.h"

//=============================================================================
//	for MAC address to SMSC9118's EEP Rom
//=============================================================================
//#include "CE118eep.h"
#include "windev.h"
#include "ntddndis.h"
#include "nuiouser.h"

#define MAJOR_VER 0
#define MINOR_VER 1
#define DATECODE "12/02/2009"

// SCSRs

#define ID_REV							(0x50UL)
#define INT_CFG							(0x54UL)
#define INT_STS							(0x58UL)
#define INT_EN							(0x5CUL)
#define DMA_CFG							(0x60UL)
#define BYTE_TEST						(0x64UL)	// RO default 0x87654321
#define FIFO_INT						(0x68UL)
#define RX_CFG							(0x6CUL)
#define TX_CFG							(0x70UL)
#define HW_CFG							(0x74UL)
#define RX_DP_CTL						(0x78UL)
#define RX_FIFO_INF						(0x7CUL)
#define TX_FIFO_INF						(0x80UL)
#define PMT_CTRL						(0x84UL)
#define GPIO_CFG						(0x88UL)
#define GPT_CFG							(0x8CUL)
#define GPT_CNT							(0x90UL)
#define WORD_SWAP						(0x98UL)	// R/W Not Affected by SW Reset
#define FREE_RUN						(0x9CUL)	// RO
#define RX_DROP							(0xA0UL)	// R/WC
#define MAC_CSR_CMD						(0xA4UL)
#define MAC_CSR_DATA					(0xA8UL)	// R/W
#define DEFAULT_AFC_CFG					(0x006E374FUL)
#define AFC_CFG							(0xACUL)
#define E2P_CMD							(0xB0UL)
#define E2P_DATA						(0xB4UL)
#define TEST_REG_A						(0xC0UL)
#define LAN_REGISTER_EXTENT				(0x00000100UL)



// MAC CSRs
#define MAC_CR							(0x01UL)	// R/W
#define ADDRH							(0x02UL)	// R/W mask 0x0000FFFFUL
#define ADDRL							(0x03UL)	// R/W mask 0xFFFFFFFFUL
#define HASHH							(0x04UL)	// R/W
#define HASHL							(0x05UL)	// R/W
#define MII_ACC							(0x06UL)	// R/W
#define MII_DATA						(0x07UL)	// R/W mask 0x0000FFFFUL
#define FLOW							(0x08UL)	// R/W
#define VLAN1							(0x09UL)	// R/W mask 0x0000FFFFUL
#define VLAN2							(0x0AUL)	// R/W mask 0x0000FFFFUL
#define WUFF							(0x0BUL)	// WO
#define WUCSR							(0x0CUL)	// R/W




////////////////////////////////////
// PHY Definitions
////////////////////////////////////
#define LAN9118_PHY_ID					(0x00C0001CUL)
#define PHY_BSR							((DWORD)1U)
#define PHY_ID_1						((DWORD)2U)
#define PHY_ID_2						((DWORD)3U)
#define PHY_ANEG_ADV					((DWORD)4U)
#define PHY_ANEG_LPA					((DWORD)5U)
#define PHY_ANEG_EXP					((DWORD)6U)
#define PHY_MODE_CTRL_STS				((DWORD)17)		// Mode Control/Status Register
#define SPECIAL_CTRL_STS				((DWORD)27)
#define PHY_INT_SRC						((DWORD)29)
#define PHY_INT_MASK					((DWORD)30)
#define PHY_SPECIAL						((DWORD)31)


typedef struct _SCSR_Disp
{
	char name[50];
	DWORD offset;
}SCSR_Disp;


SCSR_Disp LanRegSet[] =	{"ID_REV      ", ID_REV,
						"INT_CFG    ", INT_CFG,
						"INT_STS     ", INT_STS,
						"INT_EN       ", INT_EN,
						"DMA_CFG      ", DMA_CFG,
						"BYTE_TEST      ", BYTE_TEST,
						"FIFO_INT ", FIFO_INT,
						"RX_CFG ", RX_CFG,
						"TX_CFG      ", TX_CFG,
						"HW_CFG ", HW_CFG,
						"RX_DP_CTL     ", RX_DP_CTL,
						"RX_FIFO_INF     ", RX_FIFO_INF,
						"TX_FIFO_INF     ", TX_FIFO_INF,
						"PMT_CTRL  ", PMT_CTRL,
						"GPIO_CFG ", GPIO_CFG,
						"GPT_CFG     ", GPT_CFG,
						"GPT_CNT     ", GPT_CNT,
						"WORD_SWAP     ", WORD_SWAP,
						"FREE_RUN    ", FREE_RUN,
						"RX_DROP    ", RX_DROP,
						"MAC_CSR_CMD   ", MAC_CSR_CMD,
						"MAC_CSR_DATA  ", MAC_CSR_DATA,
						"DEFAULT_AFC_CFG ", DEFAULT_AFC_CFG,
						"AFC_CFG",            AFC_CFG,
						"E2P_CMD",             	E2P_CMD,
						"E2P_DATA",            	E2P_DATA,
						"TEST_REG_A",          TEST_REG_A,
						"LAN_REGISTER_EXTENT",  LAN_REGISTER_EXTENT};


SCSR_Disp MacRegSet[] = {"MAC_CR    ", MAC_CR,
						"ADDRH     ", ADDRH,
						"ADDRL     ", ADDRL,
						"HASHH     ", HASHH,
						"HASHL     ", HASHL,
						"MII_ACC  ", MII_ACC,
						"MII_DATA  ", MII_DATA,
						"FLOW      ", FLOW,
						"VLAN1     ", VLAN1,
						"VLAN2     ", VLAN2,
						"WUFF      ", WUFF,
						"WUCSR     ", WUCSR};


SCSR_Disp PhyRegSet[] = {"LAN9118_PHY_ID           ", LAN9118_PHY_ID,
						"PHY_BSR            ", PHY_BSR,
						"PHY_ID_1            ", PHY_ID_1,
						"PHY_ID_2            ", PHY_ID_2,
						"PHY_ANEG_ADV            ", PHY_ANEG_ADV,
						"PHY_ANEG_LPA          ", PHY_ANEG_LPA,
						"PHY_ANEG_EXP            ", PHY_ANEG_EXP,
						"PHY_MODE_CTRL_STS ", PHY_MODE_CTRL_STS,
						"SPECIAL_CTRL_STS", SPECIAL_CTRL_STS,
						"PHY_INT_SRC  ", PHY_INT_SRC,
						"PHY_INT_MASK     ", PHY_INT_MASK,
						"PHY_SPECIAL  ", PHY_SPECIAL};


BOOL SendE2pCmd(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD cmd);
BOOL ReadE2p(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, WORD addr, BYTE *val);
BOOL WriteE2p( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, WORD addr, BYTE *val);
//BOOL ReadBlockE2p( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD addr, DWORD length, BYTE *buf);
BOOL ReadBlockE2p( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD length, BYTE *buf);
//BOOL WriteBlockE2p( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD addr, DWORD length, BYTE *buf);
BOOL WriteBlockE2p( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD length, BYTE *buf);
BOOL EraseE2p(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID);
BOOL LanReadRegDW(HANDLE	hAdapter,  PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal);
BOOL LanWriteRegDW( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal);
BOOL MacReadRegDW(HANDLE	hAdapter,  PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal);
BOOL MacWriteRegDW( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *dwVal);
BOOL PhyReadRegW( HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *wVal);
BOOL PhyWriteRegW(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, DWORD dwOffset, DWORD *wVal);
BOOL GetDriverVersion(HANDLE	hAdapter, PNDISUIO_QUERY_OID queryOID, void *Buf);


/* Basic type definitions */
//typedef unsigned char   BYTE, *PBYTE;
//typedef unsigned short  WORD, *PWORD;
//typedef unsigned long   DWORD, *PDWORD;
//typedef unsigned __int64 DWORD64, *PDWORD64;


#define OID_NDIS_SMSC_LAN_READ_REGS					0xFFFF0001U
#define OID_NDIS_SMSC_MAC_READ_REGS					0xFFFF0002U
#define OID_NDIS_SMSC_PHY_READ_REGS					0xFFFF0003U
#define OID_NDIS_SMSC_LAN_WRITE_REGS				0xFFFF0004U
#define OID_NDIS_SMSC_MAC_WRITE_REGS				0xFFFF0005U
#define OID_NDIS_SMSC_PHY_WRITE_REGS				0xFFFF0006U
#define OID_NDIS_SMSC_DUMP_LAN_REGS					0xFFFF0007U
#define OID_NDIS_SMSC_DUMP_MAC_REGS					0xFFFF0008U
#define OID_NDIS_SMSC_DUMP_PHY_REGS					0xFFFF0009U
#define OID_NDIS_SMSC_DUMP_TX_STATS					0xFFFF000AU
#define OID_NDIS_SMSC_DUMP_RX_STATS					0xFFFF000BU
#define OID_NDIS_SMSC_SAVE_E2P_TO_FILE				0xFFFF000CU
#define OID_NDIS_SMSC_ERASE_E2P						0xFFFF000DU
#define OID_NDIS_SMSC_WRITE_FILE_TO_E2P				0xFFFF000EU
#define OID_NDIS_SMSC_SEND_E2P_COMMAND				0xFFFF000FU
#define OID_NDIS_SMSC_READ_E2P						0xFFFF0010U
#define OID_NDIS_SMSC_WRITE_E2P						0xFFFF0011U
#define OID_NDIS_SMSC_READ_E2P_BLOCK				0xFFFF0012U
#define OID_NDIS_SMSC_WRITE_E2P_BLOCK				0xFFFF0013U
#define OID_NDIS_SMSC_GET_E2P_SIZE					0xFFFF0014U
#define OID_NDIS_SMSC_GET_DRIVER_INFO				0xFFFF0015U
#define OID_NDIS_SMSC_ENABLE_DEBUG					0xFFFF0016U
#define OID_NDIS_SMSC_ENABLE_RESUME					0xFFFF0017U


// E2pCmdType:
#define	E2P_CMD_EWDS								0x10000000UL // Erase/Write Disable
#define	E2P_CMD_EWEN								0x20000000UL // Erase/Write Enable
#define	E2P_CMD_ERAL								0x60000000UL // Erase All




//----------------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = TEXT ("UpdateMACAddress");
PBOOT_CFG g_pBootCfg;
UCHAR g_TOC[LB_PAGE_SIZE];
const PTOC g_pTOC = (PTOC)g_TOC;

// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_INITDIALOG, DoInitDlgMain,
    WM_COMMAND, DoCommandMain,
};
// Command Message dispatch for MainWindowProc
const struct decodeCMD MainCommandItems[] = {
    IDOK, DoUpdateMACAddress,
    IDCANCEL, DoMainCommandExit,
};

//======================================================================
// Program entry point
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX icex;

    // Load the command bar common control class.
    icex.dwSize = sizeof (INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx (&icex);

    // Display dialog box as main window.
    DialogBoxParam (hInstance, szAppName, NULL, MainDlgProc, 0);
    return 0;
}
//======================================================================
// Message handling procedures for main window
//----------------------------------------------------------------------
// MainDlgProc - Callback function for application window
//
BOOL CALLBACK MainDlgProc (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    INT i;
    //
    // Search message list to see if we need to handle this
    // message. If in list, call procedure.
    //
    for (i = 0; i < dim(MainMessages); i++) {
        if (wMsg == MainMessages[i].Code)
            return (*MainMessages[i].Fxn)(hWnd, wMsg, wParam, lParam);
    }
    return FALSE;
}
//----------------------------------------------------------------------
// DoInitDlgMain - Process WM_INITDIALOG message for window.
//
BOOL DoInitDlgMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    SetFocus(GetDlgItem(hWnd, IDC_EDIT1));
    SendDlgItemMessage (hWnd, IDC_EDIT1, EM_SETLIMITTEXT, 12, 0);

	SendMessage(GetDlgItem(hWnd, IDC_RADIO_NAND_FLASH), BM_SETCHECK, BST_CHECKED, 0);
	SendMessage(GetDlgItem(hWnd, IDC_RADIO_EEP_ROM), BM_SETCHECK, BST_UNCHECKED, 0);

    return 0;
}
//----------------------------------------------------------------------
// DoCommandMain - Process WM_COMMAND message for window.
//
BOOL DoCommandMain (HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    WORD idItem, wNotifyCode;
    HWND hwndCtl;
    INT  i;

    // Parse the parameters.
    idItem = (WORD) LOWORD (wParam);
    wNotifyCode = (WORD) HIWORD (wParam);
    hwndCtl = (HWND) lParam;

    // Call routine to handle control message.
    for (i = 0; i < dim(MainCommandItems); i++) {
        if (idItem == MainCommandItems[i].Code) {
            (*MainCommandItems[i].Fxn)(hWnd, idItem, hwndCtl, 
                                       wNotifyCode);
            return TRUE;
        }
    }
    return FALSE;
}
//======================================================================
// Command handler routines
//----------------------------------------------------------------------
// DoMainCommandExit - Process Program Exit command.
//
LPARAM DoMainCommandExit (HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
    EndDialog (hWnd, 0);
    return 0;
}
//----------------------------------------------------------------------
//
LPARAM DoUpdateMACAddress (HWND hWnd, WORD idItem, HWND hwndCtl, WORD wNotifyCode)
{
	TCHAR	szString[64];
	UCHAR	szLANID[13], bMac[6];
    DWORD	i;

    GetDlgItemText (hWnd, IDC_EDIT1, szString, sizeof(szString));
    WideCharToMultiByte(CP_ACP, 0, szString, -1, (LPSTR)szLANID, 12, NULL, NULL);

    for (i = 0; i < 6; i++)
    {
        bMac[i] = Hex2Dec(&szLANID[i << 1]);
    }


	if(BST_CHECKED == SendMessage(GetDlgItem(hWnd, IDC_RADIO_NAND_FLASH), BM_GETCHECK, 0, 0))
	{
		//=========================================================================
		// MAC Address to Nand Flash
		//=========================================================================

		if (!TOC_Read(g_pTOC, sizeof(g_TOC)))
		{
			MessageBox(hWnd, TEXT("MAC Update Failed"), TEXT("Update MAC Address"), MB_OK);
			return 0;
		}

		g_pBootCfg->EdbgAddr.wMAC[0] = (bMac[1] << 8) | bMac[0];
		g_pBootCfg->EdbgAddr.wMAC[1] = (bMac[3] << 8) | bMac[2];
		g_pBootCfg->EdbgAddr.wMAC[2] = (bMac[5] << 8) | bMac[4];

		if (!TOC_Write(g_pTOC, sizeof(g_TOC)))
		{
			MessageBox(hWnd, TEXT("MAC update failed"), TEXT("Update MAC Address"), MB_OK|MB_ICONERROR);
		}
		else
		{
			MessageBox(hWnd, TEXT("MAC update succeeded"), TEXT("Update MAC Address"), MB_OK);
		}
	}
	else if(BST_CHECKED == SendMessage(GetDlgItem(hWnd, IDC_RADIO_EEP_ROM), BM_GETCHECK, 0, 0))
	{
		//=========================================================================
		// MAC Address to EEP Rom
		//=========================================================================

		HANDLE	hAdapter;
		char	Buffer[1024];
		PTCHAR	pAdapterName;
		DWORD	dwOutSize,ChipId;
		PNDISUIO_QUERY_BINDING	pQueryBinding;
		PNDISUIO_QUERY_OID	queryOID;
		UCHAR	QueryBuffer[sizeof(NDISUIO_QUERY_OID)+sizeof(DWORD)];
		BOOL LoopEndFlag = FALSE;
		DWORD NofBytes = 0, E2pSize=128;
		int i=0,key=0;
		DWORD DrvVer;
		BYTE readBuf[516],writeBuf[516];
		BYTE *pBuf=writeBuf;

		hAdapter = CreateFile(TEXT("UIO1:"), 0x00, 0x00, NULL, OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
					INVALID_HANDLE_VALUE);

		if (hAdapter == INVALID_HANDLE_VALUE)
		{
			//wprintf (L"\nCE118eep: Invalid hAdapter\n\n");
			RETAILMSG(TRUE, (TEXT("CE118eep: Invalid hAdapter\r\n")));
			MessageBox(hWnd, TEXT("Invalid hAdapter"), TEXT("Update MAC Address"), MB_OK);
			return 0;
		}

		pQueryBinding = (PNDISUIO_QUERY_BINDING)&Buffer[0];

		memset(pQueryBinding, 0x00, sizeof(NDISUIO_QUERY_BINDING));

		pQueryBinding->BindingIndex = 0;

		while(TRUE)
		{
			if (!DeviceIoControl(hAdapter,
					IOCTL_NDISUIO_QUERY_BINDING, pQueryBinding,
					sizeof(NDISUIO_QUERY_BINDING),
					pQueryBinding, 1024, &dwOutSize, NULL))
			{
				//wprintf (L"\nError at IOCTL_NDISUIO_QUERY_BINDING, GetLastError()= %d\n",GetLastError());
				RETAILMSG(TRUE, (TEXT("\r\nError at IOCTL_NDISUIO_QUERY_BINDING, GetLastError()= %d\r\n"), GetLastError()));
				MessageBox(hWnd, TEXT("Error at IOCTL_NDISUIO_QUERY_BINDING"), TEXT("Update MAC Address"), MB_OK|MB_ICONERROR);
				goto Exit_WinMain;
			}

			pAdapterName = (PTCHAR)&Buffer[pQueryBinding->DeviceNameOffset];
			pAdapterName[(pQueryBinding->DeviceNameLength / sizeof(TCHAR)) - 1] = 0x00;
			if (!wcscmp((const wchar_t *)pAdapterName, (const wchar_t *)L"SMSC91181"))
				break;
			pQueryBinding->BindingIndex++;
		}

		//wprintf (L"NDISPWR:: Found adapter [%s]\r\n",pAdapterName);
		RETAILMSG(TRUE, (TEXT("NDISPWR:: Found adapter [%s]\r\n"), pAdapterName));

		queryOID = (PNDISUIO_QUERY_OID)&QueryBuffer[0];
		queryOID->ptcDeviceName = pAdapterName;


		if (!GetDriverVersion(hAdapter,queryOID,(void *)&DrvVer))
		{
			MessageBox(hWnd, TEXT("Error on GetDriverVersion"), TEXT("Update MAC Address"), MB_OK|MB_ICONERROR);
			goto Exit_WinMain;
		}

		if (!LanReadRegDW(hAdapter,queryOID, ID_REV, &ChipId))
		{
			MessageBox(hWnd, TEXT("Error on LanReadRegDW"), TEXT("Update MAC Address"), MB_OK|MB_ICONERROR);
			goto Exit_WinMain;
		}

		//printf ("Driver Version V%x\n", DrvVer);
		//printf ("Chip ID: 0x%x\n", ChipId);
		//printf ("Application Version V%d.%d (%s)\n", MAJOR_VER, MINOR_VER, DATECODE);
		//printf ("Built at %s on %s)\n\n\n",  __TIME__, __DATE__);

		RETAILMSG(TRUE, (TEXT("Driver Version V%x\r\n"), DrvVer));
		RETAILMSG(TRUE, (TEXT("Chip ID: 0x%x\r\n"), ChipId));
		RETAILMSG(TRUE, (TEXT("Application Version V%d.%d (%s)\r\n"), MAJOR_VER, MINOR_VER, DATECODE));
		RETAILMSG(TRUE, (TEXT("Built at %s on %s)\r\n\r\n\r\n"), __TIME__, __DATE__));

		memset(writeBuf, 0x00, sizeof(writeBuf));

		writeBuf[0] = 0x80;
		writeBuf[4] = 0xA5;			// SMSC Magic Number
		writeBuf[5] = bMac[0];		// MAC Address First Byte
		writeBuf[6] = bMac[1];
		writeBuf[7] = bMac[2];
		writeBuf[8] = bMac[3];
		writeBuf[9] = bMac[4];
		writeBuf[10] = bMac[5];

		memset(pBuf+11, 0xFF, (127-11+1));
		//for(i = 11; i <= 127; i++)
		//{
			//writeBuf[i] = 0xFF;
		//}

		if (WriteBlockE2p(hAdapter, queryOID, E2pSize, writeBuf) == FALSE)
		{
			MessageBox(hWnd, TEXT("Error on WriteBlockE2p"), TEXT("Update MAC Address"), MB_OK|MB_ICONERROR);
			goto Exit_WinMain;
		}

		//wprintf(L"Verify..................\n");
		RETAILMSG(TRUE, (TEXT("Verify..................\r\n")));

		memset(readBuf, 0x00, sizeof(readBuf));

		if (ReadBlockE2p(hAdapter, queryOID, E2pSize, readBuf) == FALSE)
		{
			goto Exit_WinMain;
		}

		for (i=0; i<(int)E2pSize; i++)
		{
			if  (readBuf[i+4] !=  writeBuf[i+4])
			{
				//wprintf(L"Verify failed at %d\n\n\n", i);
				RETAILMSG(TRUE, (TEXT("Verify failed at %d\r\n"), i));
				MessageBox(hWnd, TEXT("Verify failed"), TEXT("Update MAC Address"), MB_OK|MB_ICONERROR);
				break;
			}
		}

		//wprintf(L"Successfully Program & Verify MAC address to EEPROM, E2p size is %d\n\n\n",E2pSize);
		RETAILMSG(TRUE, (TEXT("Successfully Program & Verify MAC address to EEPROM, E2p size is %d\r\n\r\n\r\n"), E2pSize));
		MessageBox(hWnd, TEXT("MAC update succeeded"), TEXT("Update MAC Address"), MB_OK|MB_ICONINFORMATION);


Exit_WinMain:

		CloseHandle(hAdapter);
	}

    return 0;
}
//----------------------------------------------------------------------
//
UCHAR Hex2Dec(PUCHAR pStr)
{
    UCHAR i, c, nVal = 0;

    for (i = 0; i < 2; i++)
    {
        c = pStr[i];

        if (c >= '0' && c <= '9')
        {
            c -= '0';
        }
        else if (c >= 'A' && c <= 'F')
        {
            c -= 'A';
            c  = (0xA + c);
        }
        else if (c >= 'a' && c <= 'f')
        {
            c -= 'a';
            c  = (0xA + c);
        }

        if (!i)
        {
            nVal += c * 16;
        }
        else
        {
            nVal += c;
        }
    }

    return(nVal);
}


//=============================================================================
// fot MAC address to SMSC9118's EEP Rom
//=============================================================================
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
		//printf ( "Error in DeviceIoControl (OID_NDIS_SMSC_SEND_E2P_COMMAND) : %d\n", GetLastError());
		RETAILMSG(TRUE, (TEXT("Error in DeviceIoControl (OID_NDIS_SMSC_SEND_E2P_COMMAND) : %d\r\n"), GetLastError()));
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
		//printf ( "Error in DeviceIoControl (OID_NDIS_SMSC_READ_E2P) : %d\n", GetLastError());
		RETAILMSG(TRUE, (TEXT("Error in DeviceIoControl (OID_NDIS_SMSC_READ_E2P) : %d\r\n"), GetLastError()));
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
		//printf ( "In WriteE2p(),Failed to send E2p Command E2P_CMD_EWEN\r\n");
		RETAILMSG(TRUE, (TEXT("In WriteE2p(),Failed to send E2p Command E2P_CMD_EWEN\r\n")));
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
		//printf ( "Error in DeviceIoControl (IOCTL_LAN9118_MP_ACCESS) : %d\n", GetLastError());
		RETAILMSG(TRUE, (TEXT("Error in DeviceIoControl (IOCTL_LAN9118_MP_ACCESS) : %d\r\n"), GetLastError()));
		// Before exit, make sure to reset eeprom write enable
		SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWDS);
		return FALSE;
	}

	// Before exit, make sure to reset eeprom write enable
	if (SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWDS) == FALSE)
	{
		//printf ( "In WriteE2p(),Failed to send E2p Command E2P_CMD_EWDS\n\n");
		RETAILMSG(TRUE, (TEXT("In WriteE2p(),Failed to send E2p Command E2P_CMD_EWDS\r\n")));
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
		//printf ( "Error in DeviceIoControl (OID_NDIS_SMSC_READ_E2P_BLOCK) : %d\n", GetLastError());
		RETAILMSG(TRUE, (TEXT("Error in DeviceIoControl (OID_NDIS_SMSC_READ_E2P_BLOCK) : %d\r\n"), GetLastError()));
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
		//printf ( "In WriteBlockE2p, Error at SendE2pCmd(E2P_CMD_EWEN)");
		RETAILMSG(TRUE, (TEXT("In WriteBlockE2p, Error at SendE2pCmd(E2P_CMD_EWEN)\r\n")));
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
		//printf ( "Error in DeviceIoControl (OID_NDIS_SMSC_WRITE_E2P_BLOCK) : %d\n", GetLastError());
		RETAILMSG(TRUE, (TEXT("Error in DeviceIoControl (OID_NDIS_SMSC_WRITE_E2P_BLOCK) : %d\r\n"), GetLastError()));
		// Before exit, make sure to reset eeprom write enable
		SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWDS);
		return FALSE;
	}

	// Before exit, make sure to reset eeprom write enable
	if (SendE2pCmd(hAdapter, queryOID,E2P_CMD_EWDS) == FALSE)
	{
		//printf ( "In WriteBlockE2p, Error at SendE2pCmd(E2P_CMD_EWDS)");
		RETAILMSG(TRUE, (TEXT("In WriteBlockE2p, Error at SendE2pCmd(E2P_CMD_EWDS)\r\n")));
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
		//wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_READ_REGS) \n\n\n");
		RETAILMSG(TRUE, (TEXT("\r\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_READ_REGS)\r\n\r\n\r\n")));
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
		//wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_WRITE_REGS) \n\n\n");
		RETAILMSG(TRUE, (TEXT("\r\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_WRITE_REGS)\r\n\r\n\r\n")));
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
		//wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_READ_REGS) \n\n\n");
		RETAILMSG(TRUE, (TEXT("\r\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_READ_REGS)\r\n\r\n\r\n")));
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
		//wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_WRITE_REGS) \n\n\n");
		RETAILMSG(TRUE, (TEXT("\r\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_WRITE_REGS)\r\n\r\n\r\n")));
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
		//wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_PHY_READ_REGS) \n\n\n");
		RETAILMSG(TRUE, (TEXT("\r\nError in DeviceIoControl(),  OID_NDIS_SMSC_PHY_READ_REGS)\r\n\r\n\r\n")));
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
		//wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_WRITE_REGS) \n\n\n");
		RETAILMSG(TRUE, (TEXT("\r\nError in DeviceIoControl(),  OID_NDIS_SMSC_LAN_WRITE_REGS)\r\n\r\n\r\n")));
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
		//wprintf(L"\nError in DeviceIoControl(),  OID_NDIS_SMSC_GET_DRIVER_INFO) \n\n\n");
		RETAILMSG(TRUE, (TEXT("\r\nError in DeviceIoControl(),  OID_NDIS_SMSC_GET_DRIVER_INFO)\r\n\r\n\r\n")));
		return FALSE;
	}

	memcpy(pBuf, (void *)&queryOID->Data[0], sizeof(DWORD));

	return TRUE;
}
