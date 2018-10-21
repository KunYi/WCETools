
#include <windows.h>                 // For all that Windows stuff
#include <CommCtrl.h>
#include <bootpart.h>
#include <fmd.h>
//#include <bppriv.h>
#include <storemgr.h>
#include <fatutil.h>

FlashInfo g_FlashInfo;
#define NUM_PARTS		(4)
#define SIZE_END_SIG	(2)
#define PARTTABLE_OFFSET        (SECTOR_SIZE - SIZE_END_SIG - (sizeof(PARTENTRY) * NUM_PARTS))
#define BADBLOCKMARK	(0)

DWORD nMBR = 0;
BYTE pMBuf[2048];
BYTE pSBuf[64];

WCHAR storeName[] = L"SMFLASH";
WCHAR partName[] = L"Part01";
HINSTANCE	hFatUtil = NULL;
PFN_FORMATVOLUMEEX pFormatVolumeEx = NULL;
PPARTENTRY pPartition;

BOOL IsValidMBR(void)
{
	unsigned int i;
	DWORD blkStatus;

	FMD_Init(NULL, NULL, NULL);
	FMD_GetInfo(&g_FlashInfo);
	
	nMBR = 0;
	
	for (i = 0; i < g_FlashInfo.dwNumBlocks; i++)
	{
		blkStatus = FMD_GetBlockStatus(i);
		if ((blkStatus & (BLOCK_STATUS_BAD | BLOCK_STATUS_RESERVED)) == 0)
			break;
	}
	
	if (!FMD_ReadSector(g_FlashInfo.wSectorsPerBlock*i, pMBuf, NULL, 1))
		return FALSE;
	
	if ((pMBuf[0] ==0xE9) && (pMBuf[1] == 0xfd) && (pMBuf[2] == 0xff) &&
		(pMBuf[SECTOR_SIZE-2] == 0x55) && (pMBuf[SECTOR_SIZE-1] == 0xAA))
	{
		nMBR = i;
		RETAILMSG(1, (TEXT("find MBR in Block %d\r\n"), nMBR));
		return TRUE;
	}
	
	return FALSE;
}

DWORD Log2Phys (DWORD dwLogSector)
{
	DWORD LogBlk = dwLogSector / g_FlashInfo.wSectorsPerBlock;
	DWORD phyBlk = 0;

	for (phyBlk = (nMBR+1); phyBlk < g_FlashInfo.dwNumBlocks; phyBlk++) {
		if ((FMD_GetBlockStatus(phyBlk) & (BLOCK_STATUS_BAD | BLOCK_STATUS_RESERVED)) == 0)
			LogBlk--;

		if (LogBlk == 0)
			break;
	}
	
	if (phyBlk >= g_FlashInfo.dwNumBlocks-1)
		return 0xFFFFFFFF;

	return phyBlk*g_FlashInfo.wSectorsPerBlock;
}

void Dismount(void) 
{
	STOREINFO si = {0};
    HANDLE hFind = INVALID_HANDLE_VALUE;
	HANDLE hStore = INVALID_HANDLE_VALUE;
	HANDLE hPartition = INVALID_HANDLE_VALUE;

    si.cbSize = sizeof(STOREINFO);
	hFind = FindFirstStore(&si);
	
	if (INVALID_HANDLE_VALUE != hFind)
	{
		do {
			RETAILMSG(1, (TEXT("DeviceName:%s, storeName:%s\r\n"), si.szDeviceName, si.szStoreName));
			
		} while(FindNextStore(hFind, &si));
		FindClose(hFind);
	}
	
	hStore = OpenStore(storeName);
	if (INVALID_HANDLE_VALUE != hStore)
	{
		RETAILMSG(1, (TEXT("Success!! OpenStore()\r\n")));
		hPartition = OpenPartition(hStore, partName);
		if (INVALID_HANDLE_VALUE != hPartition)
		{
			RETAILMSG(1, (TEXT("Success!! OpenPartition()\r\n")));
			if( TRUE == DismountPartition(hPartition))
				RETAILMSG(1, (TEXT("Success!! Dismount Partition()\r\n")));
			CloseHandle(hPartition);
		}
		CloseHandle(hStore);
	}
	
}


VOID fmtProgress(DWORD dwPercent)
{
	RETAILMSG(1, (TEXT("Format %3d%%\r"), dwPercent));
	
}

void myformat(void)
{
	HANDLE hStore = INVALID_HANDLE_VALUE;
	HANDLE hPartition = INVALID_HANDLE_VALUE;
	FORMAT_PARAMS fps = { 0 };
	
	fps.cbSize = sizeof(fps);
	fps.fo.dwClusSize = 4*1024; // 4K
	fps.fo.dwRootEntries = 512;
	fps.fo.dwFatVersion = 16; // FAT16
	fps.fo.dwNumFats = 2;
	fps.fo.dwFlags = FATUTIL_FORMAT_TFAT;
	fps.fo.dwFlags |= FATUTIL_FULL_FORMAT;
	fps.pfnProgress = fmtProgress;
	fps.pfnMessage = NULL;

	hStore = OpenStore(storeName);
	if (INVALID_HANDLE_VALUE != hStore)
	{
		hPartition = OpenPartition(hStore, partName);
		if (INVALID_HANDLE_VALUE != hPartition)
		{
			if (ERROR_SUCCESS != pFormatVolumeEx(hPartition, &fps))
				RETAILMSG(1, (TEXT("Failed!, exception in format\r\n")));
			CloseHandle(hPartition);
		}
		else
		{
			RETAILMSG(1, (TEXT("Failed!! OpenPartition()\r\n")));
		}
		CloseHandle(hStore);
	} 
	else
	{
		RETAILMSG(1, (TEXT("Failed!! OpenStore()\r\n")));
	}
}

void DumpImageFile(void)
{
	HANDLE hFile;
	DWORD dwTotalPages = 64*1024;
	DWORD dwBytesWriten;
	unsigned int nSpareSize = 64;
	unsigned int i = 0;

	hFile = CreateFile(TEXT("\\Hard Disk\\FLASHIMG.img"), GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, TEXT("Create File Failed"), TEXT("Error"), MB_OK|MB_ICONQUESTION);
        return;
    }

    for (i = 0; i < dwTotalPages; i++)
    {
		RETAILMSG(1, (TEXT("Read Block:%4d:Page:%2d\r\n"), i/64, i%64));
		RAW_LB_ReadPage(i, pMBuf, pSBuf);
		
        if(!WriteFile(hFile, pMBuf, g_FlashInfo.wDataBytesPerSector, &dwBytesWriten, NULL))
        {
            MessageBox(NULL, TEXT("File write failed"), TEXT("Dump Image"), MB_OK|MB_ICONQUESTION);
            goto Exit;
        }

        if(!WriteFile(hFile, pSBuf, nSpareSize, &dwBytesWriten, NULL))
        {
            MessageBox(NULL, TEXT("File write failed"), TEXT("Dump Image"), MB_OK|MB_ICONQUESTION);
            goto Exit;
        }
	}
Exit:
    CloseHandle(hFile);	
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX icex;
	DWORD i = 0;
	DWORD phyB = 0;
	DWORD ttlB = 0;
	SectorInfo si = { 0 };

	RETAILMSG(1, (TEXT("LowFormat Startup\r\n")));
    // Load the command bar common control class.
    icex.dwSize = sizeof (INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx (&icex);

	if (::MessageBox(NULL, L"PLEASE CONFIRM YOU WANT TO LOW-FORMAT THE DEVICE\r\n"
						L"  The program just support ERM2/ERM2A device\r\n"
						L"  Will lost all data under ResidenFlash if you click Yes\r\n",
						L"LOW-FORMAT",
						MB_YESNO | MB_ICONWARNING | MB_TOPMOST) != IDYES) {
		RETAILMSG(1, (TEXT("Exit program\r\n")));
		return -3;		
	}
						
	if (TRUE == IsValidMBR()) {
		RETAILMSG(1, (TEXT("Find a ValidMBR\r\n")));
#if 0
		for ( i = 0; i < 512; i+=16) {
			RETAILMSG(1, (TEXT("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n"),
				pMBuf[i], pMBuf[i+1], pMBuf[i+2], pMBuf[i+3], pMBuf[i+4], pMBuf[i+5], pMBuf[i+6], pMBuf[i+7],
				pMBuf[i+8], pMBuf[i+9], pMBuf[i+10], pMBuf[i+11], pMBuf[i+12], pMBuf[i+13], pMBuf[i+14], pMBuf[i+15]));
		}
#endif
	}
	else
	{
		::MessageBox(NULL, L"We cannot found a MBR sector on NAND flash",
							L"LOW-FORMAT",
							MB_OK | MB_TOPMOST);
		RETAILMSG(1, (TEXT("Missing MBR\r\n")));
		return -2;
	}
	
	pPartition = (PPARTENTRY)(pMBuf+PARTTABLE_OFFSET);
#if 0
	for (i = 0; i < 2; i++) {
		RETAILMSG(1, (TEXT("Dump Partition[i]\r\n"), i));
		RETAILMSG(1, (TEXT("  Part_BootInd:0x%02X\r\n"), (pPartition+i)->Part_BootInd));
		RETAILMSG(1, (TEXT("  Part_FirstHead:0x%02X\r\n"), (pPartition+i)->Part_FirstHead));
		RETAILMSG(1, (TEXT("  Part_FirstSector:0x%02X\r\n"), (pPartition+i)->Part_FirstSector));
		RETAILMSG(1, (TEXT("  Part_FirstTrack:0x%02X\r\n"), (pPartition+i)->Part_FirstTrack));
		RETAILMSG(1, (TEXT("  Part_FileSystem:0x%02X\r\n"), (pPartition+i)->Part_FileSystem));
		RETAILMSG(1, (TEXT("  Part_LastHead:0x%02X\r\n"), (pPartition+i)->Part_LastHead));
		RETAILMSG(1, (TEXT("  Part_LastSector:0x%02X\r\n"), (pPartition+i)->Part_LastSector));
		RETAILMSG(1, (TEXT("  Part_LastTrack:0x%02X\r\n"), (pPartition+i)->Part_LastTrack));
		
		RETAILMSG(1, (TEXT("  Part_StartSector:0x%08X\r\n"), (pPartition+i)->Part_StartSector));
		phyB = Log2Phys((pPartition+i)->Part_StartSector)/g_FlashInfo.wSectorsPerBlock;
		RETAILMSG(1, (TEXT("    PhysBlocks:0x%08X\r\n"), phyB));
		
		RETAILMSG(1, (TEXT("  Part_TotalSectors:0x%08X\r\n"), (pPartition+i)->Part_TotalSectors));
		ttlB = (pPartition+i)->Part_TotalSectors/g_FlashInfo.wSectorsPerBlock;
		RETAILMSG(1, (TEXT("    TotalPhysSectors:0x%08X, LastBlock:0x%08X\r\n\r\n"), ttlB, (ttlB+phyB)));
	}
#endif
	
	// Display dialog box as main window.
	// DialogBoxParam (hInstance, szAppName, NULL, MainDlgProc, 0);
	RETAILMSG(1, (TEXT("Dismount partition\r\n")));
	Dismount();
	
	phyB = Log2Phys(pPartition->Part_StartSector)/g_FlashInfo.wSectorsPerBlock;
	
	::MessageBox(NULL, L"Now we will low-format, Please waiting a minute until device reboot\r\n", 
						L"LOW-FORMAT", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
	RETAILMSG(1, (TEXT("Now LowFormat from Physical Block:%4d\r\n"), nMBR+1));
	for ( i = (nMBR+1); i < phyB; i++)
	{
		if ((FMD_GetBlockStatus(i) & (BLOCK_STATUS_BAD | BLOCK_STATUS_RESERVED | BLOCK_STATUS_READONLY)) == 0)
		{
			RETAILMSG(1, (TEXT("Erase Block: %d\r\n"),i));
			if (FMD_EraseBlock(i) != TRUE) 
			{
				RETAILMSG(1, (TEXT("Block: %d is bad, mark it\r\n"), i));
				// Mark Bad
				si.bOEMReserved = 0xff;
				si.bBadBlock    = BADBLOCKMARK;
				si.dwReserved1  = 0xffffffff;
				si.wReserved2   = 0xffff;
				FMD_WriteSector(i*g_FlashInfo.wSectorsPerBlock, NULL, &si, 1);
				FMD_WriteSector((i*g_FlashInfo.wSectorsPerBlock)+1, NULL, &si, 1);
			}
		} 
		else
		{
			RETAILMSG(1, (TEXT("Block %d is Bad or Reserved/Readonly\r\n"), i));
		}
	}

	phyB = Log2Phys((pPartition+1)->Part_StartSector)/g_FlashInfo.wSectorsPerBlock;
	RETAILMSG(1, (TEXT(" now Lowformat from Physical Block:%4d\r\n"), phyB));
	for ( i = phyB; i < g_FlashInfo.dwNumBlocks; i++)
	{
		if ((FMD_GetBlockStatus(i) & (BLOCK_STATUS_BAD | BLOCK_STATUS_RESERVED | BLOCK_STATUS_READONLY)) == 0)
		{
			RETAILMSG(1, (TEXT("Erase Block: %d\r\n"),i));
			if (FMD_EraseBlock(i) != TRUE) 
			{
				RETAILMSG(1, (TEXT("Block: %d is bad, mark it\r\n"), i));
				// Mark Bad
				si.bOEMReserved = 0xff;
				si.bBadBlock    = BADBLOCKMARK;
				si.dwReserved1  = 0xffffffff;
				si.wReserved2   = 0xffff;
				FMD_WriteSector(i*g_FlashInfo.wSectorsPerBlock, NULL, &si, 1);
				FMD_WriteSector((i*g_FlashInfo.wSectorsPerBlock)+1, NULL, &si, 1);
			}
		}
		else
		{
			RETAILMSG(1, (TEXT("Block %d is Bad or Reserved/Readonly\r\n"), i));
		}
	}

    hFatUtil = LoadLibrary (TEXT("FATUTIL.DLL"));
	if (hFatUtil != NULL)
		pFormatVolumeEx = (PFN_FORMATVOLUMEEX)GetProcAddress(hFatUtil, TEXT("FormatVolumeEx"));
	
	if (pFormatVolumeEx != NULL) {
		RETAILMSG(1, (TEXT("FATFS format\r\n")));
		myformat();
	}
	RETAILMSG(1, (TEXT("LowFormat finish\r\n")));
	
	::MessageBox(NULL, L"Complete Low-format, Device will reboot now\r\n",
						L"LOW-FORMAT",
						MB_ICONINFORMATION | MB_OK | MB_TOPMOST);

	RETAILMSG(1, (TEXT("System will reboot\r\n")));
	KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, NULL);

	//RETAILMSG(1, (TEXT("Dump Flash Start\r\n")));
	//DumpImageFile();
	//RETAILMSG(1, (TEXT("Dump Flash finish\r\n")));

    return 0;
}