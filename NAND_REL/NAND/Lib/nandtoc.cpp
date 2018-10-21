#include <windows.h>
#include "nand.h"

static BOOL s_IsOldbOEMReserved = FALSE;

//----------------------------------------------------------------------
//
BOOL TOC_Read(PTOC pTOC, int szTOC)
{
    SectorInfo si;
	FlashInfo  fi;
	

    FMD_Init(NULL, NULL, NULL);
	
	FMD_GetInfo(&fi);
	
	if (szTOC < fi.wDataBytesPerSector) {
		RETAILMSG(TRUE, (TEXT("TOC_Read ERROR: TOC buffer size is too small\r\n")));
		return FALSE;
	}

    if ( !FMD_ReadSector(TOC_SECTOR, (PUCHAR)pTOC, &si, 1) )
    {
        RETAILMSG(TRUE, (TEXT("TOC_Read ERROR: Unable to read TOC\r\n")));
        return FALSE;
    }

    // is it a valid TOC?
    if ( !VALID_TOC(pTOC) )
    {
        RETAILMSG(TRUE, (TEXT("TOC_Read ERROR: INVALID_TOC Signature: 0x%x\r\n"), pTOC->dwSignature));
        return FALSE;
    }

    // is it an OEM block?
    if ( (si.bBadBlock != 0xFF))
    {
        RETAILMSG(TRUE, (TEXT("TOC_Read ERROR: The sector is BAD: %x %x %x %x, Bad Block\r\n"),
            si.dwReserved1, si.bOEMReserved, si.bBadBlock, si.wReserved2));
        return FALSE;
    }
	
	if (((BYTE)(~si.bOEMReserved) == (BYTE)(OEM_BLOCK_RESERVED | OEM_BLOCK_READONLY)) || 
		 (si.bOEMReserved == (OEM_BLOCK_RESERVED | OEM_BLOCK_READONLY))) {
		if (si.bOEMReserved == (OEM_BLOCK_RESERVED | OEM_BLOCK_READONLY))
		{
			RETAILMSG(TRUE, (TEXT("Old bOEMReserved for TOC Sector\r\n")));
			s_IsOldbOEMReserved = TRUE;
		} else {
			s_IsOldbOEMReserved	= FALSE;
		}
		return TRUE;
	}

	RETAILMSG(TRUE, (TEXT("TOC_Read ERROR: SectorInfo verify failed %x %x %x %x\r\n"),
		si.dwReserved1, si.bOEMReserved, si.bBadBlock, si.wReserved2));
	return FALSE;
}

//----------------------------------------------------------------------
//
BOOL TOC_Write(PTOC pTOC, int szTOC)
{
    SectorInfo si, si2;
	FlashInfo  fi;
	static unsigned char toc[2048];

    FMD_Init(NULL, NULL, NULL);	
	FMD_GetInfo(&fi);

	if (szTOC < fi.wDataBytesPerSector) {
		RETAILMSG(TRUE, (TEXT("TOC_Read ERROR: TOC buffer size is too small\r\n")));
		return FALSE;
	}

    // is it a valid TOC?
    if ( !VALID_TOC(pTOC) )
    {
        RETAILMSG(TRUE, (TEXT("TOC_Write ERROR: INVALID_TOC Signature: 0x%x\r\n"), pTOC->dwSignature));
        return FALSE;
    }

    // in order to write a sector we must erase the entire block first
    // !! BUGBUG: must cache the TOC first so we don't trash other image descriptors !!
    if ( !FMD_EraseBlock(TOC_BLOCK) )
    {
        RETAILMSG(TRUE, (TEXT("TOC_Write ERROR: EraseBlock[%d] \r\n"), TOC_BLOCK));
        return FALSE;
    }

    // setup our metadata so filesys won't stomp us
    si.dwReserved1 = 0xFFFFFFFF;
	if (s_IsOldbOEMReserved == TRUE) {
		si.bOEMReserved = (OEM_BLOCK_RESERVED | OEM_BLOCK_READONLY);		
	} else {
		si.bOEMReserved = ~((BYTE)OEM_BLOCK_RESERVED | OEM_BLOCK_READONLY);
	}
    si.bBadBlock = 0xFF;
    si.wReserved2 = 0xFFFF;

    // write the sector & metadata
    if ( !FMD_WriteSector(TOC_SECTOR, (PUCHAR)pTOC, &si, 1) )
    {
        RETAILMSG(TRUE, (TEXT("TOC_Write ERROR: Unable to save TOC\r\n")));
        return FALSE;
    }

    // read it back & verify both data & metadata
    if ( !FMD_ReadSector(TOC_SECTOR, (PUCHAR)&toc, &si2, 1) )
    {
        RETAILMSG(TRUE, (TEXT("TOC_Write ERROR: Unable to read/verify TOC\r\n")));
        return FALSE;
    }

    if ( 0 != memcmp(&pTOC, &toc, SECTOR_SIZE) )
    {
        RETAILMSG(TRUE, (TEXT("TOC_Write ERROR: TOC verify failed\r\n")));
        return FALSE;
    }

    if ( 0 != memcmp(&si, &si2, sizeof(si)) )
    {
        RETAILMSG(TRUE, (TEXT("TOC_Write ERROR: SectorInfo verify failed: %x %x %x %x\r\n"),
            si.dwReserved1, si.bOEMReserved, si.bBadBlock, si.wReserved2));
        return FALSE;
    }

    return TRUE;
}
