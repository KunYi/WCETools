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
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Module Name:    FMD.H

Abstract:       FLASH Media Driver Interface for Windows CE 
  
Notes:          The following module defines the required entry points for
                creating a new FLASH Media Driver (FMD) for Windows CE.  The 
                FMD is called by the FLASH Abstraction Layer (FAL) and is 
                responsible for directly writing to the underlying FLASH hardware
                (i.e. NAND/NOR chip).  In turn, the FAL handles all of the necessary
                wear-leveling of the device.  Practically, this means that the FMD
                is NEVER asked to read/write an invalid page or block.

                The FMD is responsible for:
                    * reading from the FLASH media
                    * writing to the FLASH media
                    * erasing FLASH media blocks
                    * performing error correcting-codes (ECC) on data stored on the 
                      FLASH media (if necessary)

Environment:    As noted, this media driver works on behalf of the FAL to directly
                access the underlying FLASH hardware.  Consquently, this module 
                needs to be linked with FAL.LIB to produce the device driver 
                named FLASHDRV.DLL.

-----------------------------------------------------------------------------*/

#ifndef _FMD_H_
#define _FMD_H_ 

#include <windows.h>
#include <diskio.h>
#include <bldver.h>
#include <pcireg.h>

// FMD IOCTL definitions.
//
#define IOCTL_FMD_SET_XIPMODE	IOCTL_DISK_USER(0)
#define IOCTL_FMD_LOCK_BLOCKS	IOCTL_DISK_USER(1)
#define IOCTL_FMD_UNLOCK_BLOCKS	IOCTL_DISK_USER(2)
#define IOCTL_FMD_GET_INTERFACE IOCTL_DISK_USER(3)
#define IOCTL_FMD_GET_XIPMODE	IOCTL_DISK_USER(4)

// FMD block status definitions.
#define BLOCK_STATUS_UNKNOWN	0x01
#define BLOCK_STATUS_BAD		0x02
#define BLOCK_STATUS_READONLY	0x04
#define BLOCK_STATUS_RESERVED   0x08
#define BLOCK_STATUS_XIP        0x10
// FMD OEM reserved area bitfield.
#define OEM_BLOCK_RESERVED		0x01
#define OEM_BLOCK_READONLY		0x02

#ifdef __cplusplus
extern "C" {
#endif 

#define INVALID_BLOCK_ID        0xFFFFFFFF
#define INVALID_SECTOR_ADDR     0xFFFFFFFF


//--------------------------- Structure Definitions -----------------------------

typedef enum  _FLASH_TYPE { NAND, NOR } FLASH_TYPE;

typedef DWORD  SECTOR_ADDR;
typedef PDWORD PSECTOR_ADDR;

typedef DWORD  BLOCK_ID;
typedef PDWORD PBLOCK_ID;

typedef struct _FlashInfo
{
    FLASH_TYPE  flashType;
    DWORD       dwNumBlocks;
    DWORD       dwBytesPerBlock;
    WORD        wSectorsPerBlock;
    WORD        wDataBytesPerSector;

}FlashInfo, *PFlashInfo;


typedef struct _SectorInfo
{
    DWORD dwReserved1;              // Reserved - used by FAL
    BYTE  bOEMReserved;             // For use by OEM
    BYTE  bBadBlock;	            // Indicates if block is BAD
    WORD  wReserved2;               // Reserved - used by FAL
    
}SectorInfo, *PSectorInfo;

typedef struct _BlockLockInfo
{
    BLOCK_ID StartBlock;
    ULONG    NumBlocks;
}BlockLockInfo, *PBlockLockInfo;


typedef PVOID  (*PFN_INIT)(LPCTSTR lpActiveReg, PPCI_REG_INFO pRegIn, PPCI_REG_INFO pRegOut);
typedef BOOL  (*PFN_DEINIT)(PVOID);
typedef BOOL  (*PFN_GETINFO)(PFlashInfo pFlashInfo);
typedef DWORD (*PFN_GETBLOCKSTATUS)(BLOCK_ID blockID);
typedef BOOL (*PFN_SETBLOCKSTATUS)(BLOCK_ID blockID, DWORD dwStatus);
typedef BOOL  (*PFN_READSECTOR)(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors);
typedef BOOL  (*PFN_WRITESECTOR)(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors);
typedef BOOL  (*PFN_ERASEBLOCK)(BLOCK_ID blockID);
typedef VOID  (*PFN_POWERUP)(VOID);
typedef VOID  (*PFN_POWERDOWN)(VOID);
typedef VOID (*PFN_GETPHYSSECTORADDR)(DWORD dwSector, PSECTOR_ADDR pStartSectorAddr);


typedef struct _FMDInterface
{
    DWORD cbSize;
    PFN_INIT pInit;
    PFN_DEINIT pDeInit;
    PFN_GETINFO pGetInfo;
    PFN_GETBLOCKSTATUS pGetBlockStatus;
    PFN_SETBLOCKSTATUS pSetBlockStatus;
    PFN_READSECTOR pReadSector;
    PFN_WRITESECTOR pWriteSector;
    PFN_ERASEBLOCK pEraseBlock;
    PFN_POWERUP pPowerUp;
    PFN_POWERDOWN pPowerDown;
    PFN_GETPHYSSECTORADDR pGetPhysSectorAddr;
} FMDInterface, *PFMDInterface;


//------------------------------- Public Interface (used by the FAL) ------------------------------
PVOID  FMD_Init(LPCTSTR lpActiveReg, PPCI_REG_INFO pRegIn, PPCI_REG_INFO pRegOut);
BOOL  FMD_Deinit(PVOID);
BOOL  FMD_GetInfo(PFlashInfo pFlashInfo);
DWORD FMD_GetBlockStatus(BLOCK_ID blockID);
BOOL FMD_SetBlockStatus(BLOCK_ID blockID, DWORD dwStatus);

BOOL  FMD_ReadSector (SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors);
BOOL  FMD_WriteSector(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, PSectorInfo pSectorInfoBuff, DWORD dwNumSectors);

BOOL  FMD_EraseBlock(BLOCK_ID blockID);

VOID  FMD_PowerUp(VOID);
VOID  FMD_PowerDown(VOID);

BOOL  FMD_OEMIoControl(DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, 
                       PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned);
				   
DWORD FMD_GetChipID(VOID);

BOOL RAW_LB_ReadPage(SECTOR_ADDR startSectorAddr, LPBYTE pSectorBuff, LPBYTE pSectorInfoBuff);

//---------------------------------------- Helper Functions ----------------------------------------

#ifdef __cplusplus
}
#endif

#endif _FMD_H_
