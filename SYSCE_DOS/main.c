/*
   Abstract: Utility to update the active partition boot sector and update the first root directory
          entry to support finding and loading the Windows CE BIOS bootloader.  It also transfers
          the bootloader image.

Functions:

Notes:

    The following assumptions have been made - these are areas that might be revisited in a future release:
    
    * The boot sector and bootloader designs assume the loader image will be a contiguous grouping of sectors
      on the storage device.  To help ensure this is true, the storage device should first be formatted and
      before copying other files, this utility should be run to transfer the loader.  Note that bad sectors
      are not accounted for in this utility and could cause problems.
      
    * It's assumed that the storage device is fully prepared before running this utility.  Prepared means that
      there is an configured Active partition on the storage device and that it has been formatted (not to
      include DOS system files).
      
    * The bootloader is assumed to lie in the root directory of the storage device.  If it needs to be placed
      somewhere else, this utility and the boot sector will need to be updated. 

--*/

#include <stdio.h>
#include <malloc.h>      
#include <memory.h>

//
// FAT filesystem constants.
//
#define SECTOR_SIZE      0x200
#define PARTTBL_SIZE        64
#define PARTSIG_SIZE         2
#define PARTACTV_FLAG     0x80
#define MAX_PARTITION        4

//
// Disk constants.
//
#define FIXED_DISK_ID     0x80
#define FLOPPY_DISK_ID       0
#define FLOPPY_IO_RETRIES    2
#define FIXDISK_IO_RETRIES   0

#define TRUE                 1
#define FALSE                0

//
// Helper macros.
//
#define TOLOWER(a) ((a >= 0x41 && a <= 0x5a) ? (a + 0x20) : a)

//
// FAT directory entry structure.
//
#pragma pack(1)
typedef struct
{
    unsigned char  FileName[8];
    unsigned char  FileExt[3];
    unsigned char  FATTr;
    unsigned char  FOO[10];
    unsigned short LModTime;
    unsigned short LModDate;
    unsigned short FirstClust;
    unsigned long  FileSize;
} DIRENTRY, *PDIRENTRY;

//
// BIOS parameter block structure.
//
#pragma pack(1)
typedef struct BIOSPB_TAG
{                                   //  Offset   Size
    unsigned char  VersionId[8];    //  3        8
    unsigned short BytesPerSect;    //  11       2
    unsigned char  SectsPerClust;   //  13       1
    unsigned short RsvdSects;       //  14       2
    unsigned char  NumFATs;         //  16       1 
    unsigned short NumRootEntries;  //  17       2
    unsigned short SectsPerPart;    //  19       2
    unsigned char  MediaDesc;       //  21       1
    unsigned short SectsPerFAT;     //  22       2
    unsigned short SectsPerTrack;   //  24       2
    unsigned short NumHeads;        //  26       2
    unsigned short NumHiddenSectL;  //  28       2
    unsigned short NumHiddenSectH;  //  30       2
    unsigned short TotalSectorsL;   //  32       2
    unsigned short TotalSectorsH;   //  34       2
    unsigned char  DriveId;         //  36       1
    unsigned char  TempVal;         //  37       1
    unsigned char  ExtRecordSig;    //  38       1
    unsigned short VolSerNumL;      //  39       2
    unsigned short VolSerNumH;      //  41       2
    unsigned char  VolLabel[11];    //  43       11
    unsigned char  TypeFAT[8];      //  54       8
} BIOSPB, *PBIOSPB;

#pragma pack(1)
typedef struct BIOSPB_TAG32
{                            //  Offset    Size
    unsigned char   VersionId[8];     //  3         8
    unsigned short  BytesPerSect;     //  11        2
    unsigned char   SectsPerClust;    //  13        1
    unsigned short  RsvdSects;        //  14        2
    unsigned char   NumFATs;          //  16        1
    unsigned short  NumRootEntries;   //  17        2
    unsigned short  SectsPerPart;     //  19        2
    unsigned char   MediaDesc;        //  21        1
    unsigned short  SectsPerFAT;      //  22        2
    unsigned short  SectsPerTrack;    //  24        2
    unsigned short  NumHeads;         //  26        2
    unsigned short  NumHiddenSectL;   //  28        4
    unsigned short  NumHiddenSectH;
    unsigned short  TotalSectorsL;    //  32        4
    unsigned short  TotalSectorsH;
    unsigned short  SectsPerFATL;     //  36        4
    unsigned short  SectsPerFATH;
    unsigned short  Flags;            //  40        2
    unsigned short  FSVersion;        //  42        2
    unsigned short  RootClusterL;     //  44        4
    unsigned short  RootClusterH;
    unsigned short  FSInfo;           //  48        2
    unsigned short  BackupBootSector; //  50        2
    unsigned char   Reserved[12];     //  52        12
    unsigned char   DriveId;          //  64        1
    unsigned char   Reserved1;        //  65        1
    unsigned char   BootSig;          //  66        1
    unsigned short  VolumeIdL;        //  67        4
    unsigned short  VolumeIdH;
    unsigned char   Label[11];        //  71        11
    unsigned char   TypeFAT[8];       //  82        8
} BIOSPB32, *PBIOSPB32;

//
// MBR partition table entry structure.
//
#pragma pack(1)
typedef struct
{
    unsigned char ActiveFlag;
    unsigned char SHead;
    unsigned char SSector;
    unsigned char SCylinder;
    unsigned char Type;
    unsigned char EHead;
    unsigned char ESector;
    unsigned char ECylinder;
    unsigned long SLBA;
    unsigned long Size;
} PARTTE, *PPARTTE;

//
// User parameters structure.
//
typedef struct
{
    unsigned short Offset;
    unsigned char *pSectorFileName;
    unsigned char *pLoaderFileName;
    unsigned char  DriveNum;
    unsigned char  DriveLetter;
} PARAMS, *PPARAMS;

//
// Globals.
//
unsigned long gActivePartLBA;           // LBA address of active partition.
unsigned char gTempSector[SECTOR_SIZE]; // Buffer for sector reads.
unsigned int  gIsFAT32 = 0;

//
// Function prototypes.
//
static int ReadSector(unsigned char Drive, unsigned short Cylinder, unsigned char Head, unsigned char Sector, unsigned char *pBuffer);
static int WriteSector(unsigned char Drive, unsigned short Cylinder, unsigned char Head, unsigned char Sector, unsigned char *pBuffer);
static int WriteBootSector(unsigned char *pSector, unsigned char DriveNum, unsigned short Offset, PBIOSPB32 pBPB);
static int ParseArguments(int ArgC, char **ArgV, PPARAMS pUserParams);
static int TransferLoader(PPARAMS pParams, PBIOSPB32 pBPB);
static void LBA2PCHS(unsigned long LBA, unsigned short *pCylinder, unsigned char  *pHead, unsigned char  *pSector, PBIOSPB32 pBPB);


//
// Main routine.
//
int main(int ArgC, char **ArgV)
{
    PARAMS UserParams;
    FILE *FP = NULL;
    BIOSPB32 BPB;
    unsigned short DataSize = 0;
    unsigned char Sector[SECTOR_SIZE];

    // If no parameters provided, user is looking for help.
    //
    if (ArgC == 1)
        goto UsageMessage;

    // Parse user arguments.
    //
    if (ParseArguments(ArgC, ArgV, &UserParams))
    {
        printf("ERROR: Invalid command line.\r\n");
        goto UsageMessage;
    }

    // Open the boot sector file for reading.
    //
    FP = fopen(UserParams.pSectorFileName, "rb");
    if (FP == NULL)
    {
        printf("ERROR: Unable to open boot sector image (%s) for reading.\r\n", UserParams.pSectorFileName);
        goto Done;
    }

    // Read file into sector buffer (terminate with EOF encountered or we reach the
    // end of the sector buffer).
    //
    memset(Sector, 0, sizeof(SECTOR_SIZE));
    for (DataSize = 0 ; DataSize < SECTOR_SIZE && !feof(FP) ; DataSize++)
    {
        *(unsigned char *)(Sector + DataSize) = fgetc(FP);
    }

	if (DataSize != (SECTOR_SIZE - 1))
	{
		printf("ERROR: size of boot sector need match 512Bytes\r\n");
		goto Done;
	}
    // Update the boot sector.
    //
    if (WriteBootSector(Sector, UserParams.DriveNum, 0, &BPB))
    {
        printf("ERROR: Unable to write to boot sector.\r\n");
        goto Done;
    }

    // Transfer bootloader (copy it to the storage device, updating the root directory).
    //
    if (TransferLoader(&UserParams, &BPB))
    {
        printf("ERROR: Unable to copy to bootloader.\r\n");
        goto Done;
    }
    
Done:
    if (FP != NULL)
        fclose(FP);

    return(0);

UsageMessage:
    printf("Prepares the storage device to run the Windows CE BIOS bootloader.\r\n\r\n");
    printf("%s bsectfile bldrfile drive\r\n\r\n", ArgV[0]);
    printf("bsectfile        - name of file that contains sector data\r\n");
    printf("bldrfile         - name of bootloader executable file\r\n");
    printf("drive            - driver letter (a:, c:, etc.)\r\n");
    printf("\r\n\r\n Tips: should be with offset, need skip FAT BPB info\r\n");
    printf("    example -b:90 for FAT32 bootsector (512-422), due to bootsector size is 422\r\n");

    return(0);
}

static int isExFAT(unsigned char *pSector)
{
	unsigned char i;
	unsigned char *pT = (pSector);
	for (i = 0x0B; i < 0x40; i++) // normal zero
    {
		if (pT[i])
			return 0; // not exFAT
	}
    return 1;	
}

static int isFAT32(unsigned char *pSector)
{
	PBIOSPB32 pBPB32 = (pSector+3);

	if (!memcmp(pBPB32->TypeFAT, "FAT32   ", 8))
		return 1;

	if((*((unsigned short*)pSector+13) == 0) && 
	   (*((unsigned short*)pSector + 20) == 0))
	{
		return 1;
	}

    return 0;
}

static int isFAT(unsigned char *pSector)
{
	PBIOSPB pBPB = (pSector+3);

	if ((memcmp(pBPB->TypeFAT, "FAT16   ", 8) == 0) ||
        (memcmp(pBPB->TypeFAT, "FAT12   ", 8) == 0) ||
        (memcmp(pBPB->TypeFAT, "FAT     ", 8) == 0))
		return 1;

    return 0;
}

// 
// Write the specified sector buffer, with offset applied, to the boot sector and return the BIOS parameter
// block (BPB) contents to the caller.
//
static int WriteBootSector(unsigned char *pSector, unsigned char DriveNum, unsigned short Offset, PBIOSPB32 pBPB)
{
    int Status = 0;
    unsigned short Cylinder;
    unsigned char Head, Sector;
    unsigned char Count;
    PPARTTE pPartTable = NULL;

    // Check parameters.
    //
    if (!pSector || !pBPB)
        return(-1);

    // To find the active boot partition on a fixed disk, we need to look at the MBR.
    // At the end of the MBR is the partition table information.  An active flag
    // denotes the active partition entry and from this we can find the location 
    // of the boot sector.
    //
    // Note - we can skip much of this if we're writing to a floppy because the boot
    // sector is the first sector on the floppy.
    //

    if (DriveNum & FIXED_DISK_ID)   // Fixed disk.
    {
        // Read MBR.
        //
        if ((Status = ReadSector(DriveNum, 0, 0, 1, gTempSector)) != 0)
        {
            printf("ERROR: Unable to read the MBR (status=0x%x).\r\n", Status);
            return(-1);
        }

        // Search through partition table in the MBR for the active partition.
        //
        pPartTable = (PPARTTE)(gTempSector + SECTOR_SIZE - PARTSIG_SIZE - PARTTBL_SIZE);

        for(Count = 0 ; (Count < MAX_PARTITION) && (pPartTable->ActiveFlag != PARTACTV_FLAG); ++Count)
            ++pPartTable;

        // Find active partition?
        //
        if (Count == MAX_PARTITION)
        {
            printf("ERROR: Unable to find active partition on drive 0x%x.\r\n", DriveNum);
            return(-1);
        }

        // Record starting cylinder, header, and sector values of active partition - this is the 
        // location of the partition's boot sector.
        //
		Head     = pPartTable->SHead;
		Sector   = pPartTable->SSector & 0x3F;  // 6bits
		Cylinder = pPartTable->SCylinder + ((pPartTable->SSector & 0xC0) << 2); // 10bits

        gActivePartLBA = pPartTable->SLBA;
    }
    else            // Floppy.
    {
        Cylinder = 0;
        Head     = 0;
        Sector   = 1;   

        gActivePartLBA = 0;
    }

    if ((Status = ReadSector(DriveNum, Cylinder, Head, Sector, gTempSector)) != 0)
    {
        printf("ERROR: Unable to read boot sector (status=0x%x).\r\n", Status);
        return(-1);
    }

	/* identify disk original FAT type */
	if(isExFAT(gTempSector))
		Offset = 120;
	else if (isFAT32(gTempSector))
		Offset = 90;
	else if (isFAT(gTempSector))
		Offset = 62;

	/* check filesystem on disk */
	if ((Offset == 120) && !isExFAT(pSector))
    {
       printf("bootsector file not match exFAT on disk\r\n");
	   return (-1);
	}

	if ((Offset == 90) && !isFAT32(pSector))
	{
       printf("bootsector file not match FAT32 on disk\r\n");
	   return (-1);
	}

	if ((Offset == 62) && !isFAT(pSector))
	{
	   printf("bootsector file not match FAT12/FAT16 on disk\r\n");
	   return (-1);
	}
    // Overlay data.
    memcpy((gTempSector + Offset), pSector, (SECTOR_SIZE - Offset));

    // Copy the BIOS parameter block for the caller.
    //
    memcpy(pBPB, (gTempSector + 3), sizeof(BIOSPB32));

    // Write the boot sector data back to disk.
    //
    Status = WriteSector(DriveNum, Cylinder, Head, Sector, gTempSector);
    if (Status)
    {
        printf("ERROR: Unable to write boot sector (status=0x%x).\r\n", Status);
        return(-1);
    }

    return(0);
}

//
// Parse and validate user's arguments.
//
static int ParseArguments(int ArgC, char **ArgV, PPARAMS pUserParams)
{
    char DriveLetter;

    // Check for valid arguments.
    //
    if (ArgC < 4 && ArgC != 5)
        return(-1);

    if (!pUserParams)
        return(-1);

    memset(pUserParams, 0, sizeof(PARAMS));

    // "argv[0] -b offset sectfile bldrfile drive".

    // Get drive letter.
    //
    DriveLetter = TOLOWER(ArgV[ArgC - 1][0]);

    // Make sure user is passing a valid driver letter.
    //
    if (DriveLetter < 'a' || DriveLetter > 'z')
    {
        printf("ERROR: Invalid drive letter (%c:).\r\n", DriveLetter);
        return(-1);
    }
    if (DriveLetter < 'c')
    {
        pUserParams->DriveNum = (FLOPPY_DISK_ID + (DriveLetter - 'a'));
    }
    else
    {
        pUserParams->DriveNum = (FIXED_DISK_ID + (DriveLetter - 'c'));
    }
    pUserParams->DriveLetter = DriveLetter;

    // Get file name.
    //
    pUserParams->pSectorFileName = ArgV[ArgC - 3];  
    pUserParams->pLoaderFileName = ArgV[ArgC - 2];  

    // Check for optional byte offset
    //
    if (ArgC > 4)
    {
        if (!memicmp(ArgV[1], "-b:", 3))
        {
            pUserParams->Offset = (unsigned char)atoi(&ArgV[1][3]);     // Not a space since the above bounds check would have caught it.

            // Make sure offset value fits within a sector.
            //
            if (pUserParams->Offset >= SECTOR_SIZE)
            {
                printf("ERROR: Offset (0x%x)should be less than sector size (0x%x).\r\n", pUserParams->Offset, SECTOR_SIZE);
                return(-1);
            }
        }
    }

    return(0);
}


//
// Read a disk sector using the BIOS INT 13h command.
//
static int ReadSector(unsigned char Drive, unsigned short Cylinder, unsigned char Head, unsigned char Sector, unsigned char *pBuffer)
{
    int Status = 0;
    unsigned char Retries = 0;

    // If we're reading from a floppy, retry the operation a couple of times to allow for the floppy to spin up.
    //
    if (Drive < FIXED_DISK_ID)
        Retries = FLOPPY_IO_RETRIES;
    else
        Retries = FIXDISK_IO_RETRIES;

    do
    {
        _asm
        {
            push   ax
            push   bx
            push   cx
            push   dx
    
            mov    ah, 02h          ; BIOS read sector command.
            mov    al, 1            ; Read 1 sector.
            mov    dx, Cylinder     ; Cylinder number.
            mov    cl, 06H
            shl    dh, cl
            or     dh, Sector       ; Sector number.
            mov    cx, dx
            xchg   ch, cl
            mov    dl, Drive        ; Drive number.
            mov    dh, Head         ; Head number.
            mov    bx, pBuffer      ; Buffer address.
            int    13h
            jnc    RS_Done
            lea    bx, Status
            mov    [bx], ah        
RS_Done:
            pop    dx
            pop    cx
            pop    bx
            pop    ax
        }
    }
    while(Status && Retries--);

    return(Status);
}


//
// Write a disk sector using the BIOS INT 13h command.
//
static int WriteSector(unsigned char Drive,  unsigned short Cylinder, unsigned char Head, unsigned char Sector, unsigned char *pBuffer)
{
    int Status = 0;
    unsigned char Retries = 0;

    // If we're writing to a floppy, retry the operation a couple of times to allow for the floppy to spin up.
    //
    if (Drive < FIXED_DISK_ID)
        Retries = FLOPPY_IO_RETRIES;
    else
        Retries = FIXDISK_IO_RETRIES;

    do
    {
        _asm
        {
            push   ax
            push   bx
            push   cx
            push   dx
    
            mov    ah, 03h          ; BIOS write sector command.
            mov    al, 1            ; Read 1 sector.
            mov    dx, Cylinder     ; Cylinder number.
            mov    cl, 06H
            shl    dh, cl
            or     dh, Sector       ; Sector number.
            mov    cx, dx
            xchg   ch, cl
            mov    dl, Drive        ; Drive number.
            mov    dh, Head         ; Head number.
            mov    bx, pBuffer      ; Buffer address.
            int    13h
            jnc    WS_Done
            lea    bx, Status
            mov    [bx], ah        
WS_Done:
            pop    dx
            pop    cx
            pop    bx
            pop    ax
        }
    }
    while(Status && Retries--);

    return(Status);
}


//
// Transfer the bootloader to the storage device and update the root directory.
//
static int TransferLoader(PPARAMS pParams, PBIOSPB32 pBPB32)
{
    unsigned long RootDirLBA = 0;
    int Status = 0;
    unsigned short Cylinder;
    unsigned char Head, Sector;
    PDIRENTRY pDirEntry = NULL;
    unsigned char FATTYPE[8];
#define MAX_COMMAND_STRING  50
    unsigned char SystemCommand[MAX_COMMAND_STRING];

    //
    // This function transfers the bootloader image to the target drive.  The
    // loader image must be stored in a contiguous grouping of sectors and the
    // loader file name must be first in the FAT directory.  Both constraints
    // are required for the boot sector to locate and load the bootloader.
    //

    // Determine the active partitions root directory LBA and convert it to a physical CHS value
    // which is needed by the BIOS.
    //
    memcpy(FATTYPE, pBPB32->TypeFAT, 8);
    if (memcmp(FATTYPE, "FAT32   ", 8) == 0)
    {
        gIsFAT32 = 1;
        RootDirLBA = pBPB32->SectsPerFATL + (pBPB32->SectsPerFATH << 16);
        RootDirLBA = RootDirLBA * pBPB32->NumFATs;
        RootDirLBA += pBPB32->RsvdSects + gActivePartLBA;
    }
    else
    {
        PBIOSPB pBPB = (PBIOSPB)(pBPB32);
        memcpy(FATTYPE, pBPB->TypeFAT, 8);
        if ((memcmp(FATTYPE, "FAT12   ", 8) == 0) ||
            (memcmp(FATTYPE, "FAT16   ", 8) == 0))
        {
            RootDirLBA = gActivePartLBA + (pBPB->NumFATs * pBPB->SectsPerFAT) + pBPB->RsvdSects;
        }
        else
		{
            printf("Unkwon filesystem, only support FAT12/FAT16/FAT32\r\n");
            return (-1);
        }
    }
    LBA2PCHS(RootDirLBA, &Cylinder, &Head, &Sector, pBPB32);

    // Read the first sector of the root directory.
    //
    if ((Status = ReadSector(pParams->DriveNum, Cylinder, Head, Sector, gTempSector)) != 0)
    {
        printf("ERROR: Unable to read a root directory sector (status=0x%x).\r\n", Status);
        return(-1);
    }

    // Update the first entry of the root directory so a later copy will use it.
    // MS-DOS's IO.SYS might be here if this is a formatted system disk.
    //
    pDirEntry = (PDIRENTRY)gTempSector;
	if (gIsFAT32)
		pDirEntry++;

    // Initialize first directory entry in order for copy to make use of it (it's typically updated by the DOS sys tool).
    //
	memset(pDirEntry, 0, sizeof(DIRENTRY));
    memset(pDirEntry->FileName, ' ', 11);
    memset(pDirEntry->FOO, ' ', 10);
    pDirEntry->FileName[0] = 0xE5; // Deleted file tag.

    // Write the updated boot sector back to disk.
    // 
    if ((Status = WriteSector(pParams->DriveNum, Cylinder, Head, Sector, gTempSector)) != 0)
    {
        printf("ERROR: Unable to write root a directory sector (status=0x%x).\r\n", Status);
        return(-1);
    }

    // Copy the bootloader image.  It's assumed that the copy will be to a
    // contiguous group of sectors.  This might not be true if the disk
    // already contains files (documented setup requirement) or if the disk
    // contains bad sector(s).
    //
    // TODO - this area should be revisited.
    //
    sprintf(SystemCommand, "copy %s %c:", pParams->pLoaderFileName, pParams->DriveLetter);
    SystemCommand[MAX_COMMAND_STRING - 1] = '\0';

    if (system(SystemCommand) == -1)
    {
        printf("ERROR: Unable to transfer bootloader.\r\n");
        return(-1);
    }

    return(0);
}


//
// Convert a LBA address to a physical CHS address that's used by the BIOS.
//
static void LBA2PCHS(unsigned long LBA, unsigned short *pCylinder, unsigned char  *pHead, unsigned char  *pSector, PBIOSPB32 pBPB)
{
    unsigned short Temp = 0;
    Temp = (unsigned short)(LBA / (unsigned long)pBPB->SectsPerTrack);

    // Do the math...
    *pCylinder = (unsigned short)(Temp / pBPB->NumHeads);
    *pHead     = (unsigned char)(Temp % pBPB->NumHeads);
    *pSector   = (unsigned char)(LBA % pBPB->SectsPerTrack) + 1;
}

