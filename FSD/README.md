File System Driver Interface
---

a file system driver need to implements the below APIs

from fsdmgr.h of Windows CE 6.0
```
/*  Interfaces imported by FSDMGR.DLL from FSDs for application support
 *
 *  Add a #define for FSDAPI before including this file, so that the following
 *  prototypes match the interface names your FSD must export.  For example,
 *  an FSD named FATFSD.DLL would want to include the following lines in its
 *  source code:
 *
 *      #define FSDAPI FATFSD
 *      #include <fsdmgr.h>
 *
 *  since FSDMGR.DLL will expect to find exports named "FATFSD_CreateDirectoryW",
 *  "FATFSD_RemoveDirectoryW", etc, if it is asked to load an FSD named FATFSD.DLL.
 */
```

1. DECLFSDAPI(BOOL,  FSDAPI, CloseVolume(PVOLUME pVolume));
1. DECLFSDAPI(BOOL,  FSDAPI, CreateDirectoryW(PVOLUME pVolume, PCWSTR pwsPathName, PSECURITY_ATTRIBUTES pSecurityAttributes));
1. DECLFSDAPI(BOOL,  FSDAPI, RemoveDirectoryW(PVOLUME pVolume, PCWSTR pwsPathName));
1. DECLFSDAPI(DWORD, FSDAPI, GetFileAttributesW(PVOLUME pVolume, PCWSTR pwsFileName));
1. DECLFSDAPI(BOOL,  FSDAPI, SetFileAttributesW(PVOLUME pVolume, PCWSTR pwsFileName, DWORD dwAttributes));
1. DECLFSDAPI(BOOL,  FSDAPI, DeleteFileW(PVOLUME pVolume, PCWSTR pwsFileName));
1. DECLFSDAPI(BOOL,  FSDAPI, MoveFileW(PVOLUME pVolume, PCWSTR pwsOldFileName, PCWSTR pwsNewFileName));
1. DECLFSDAPI(BOOL,  FSDAPI, DeleteAndRenameFileW(PVOLUME pVolume, PCWSTR pwsOldFileName, PCWSTR pwsNewFileName));
1. DECLFSDAPI(BOOL,  FSDAPI, GetDiskFreeSpaceW(PVOLUME pVolume, PCWSTR pwsPathName, PDWORD pSectorsPerCluster, PDWORD pBytesPerSector, PDWORD pFreeClusters, PDWORD pClusters));
1. DECLFSDAPI(void,  FSDAPI, Notify(PVOLUME pVolume, DWORD dwFlags));
1. DECLFSDAPI(BOOL,  FSDAPI, RegisterFileSystemFunction(PVOLUME pVolume, SHELLFILECHANGEFUNC_t pfn));
1. DECLFSDAPI(HANDLE,FSDAPI, FindFirstFileW(PVOLUME pVolume, HANDLE hProc, PCWSTR pwsFileSpec, PWIN32_FIND_DATAW pfd));
1. DECLFSDAPI(BOOL,  FSDAPI, FindNextFileW(PSEARCH pSearch, PWIN32_FIND_DATAW pfd));
1. DECLFSDAPI(BOOL,  FSDAPI, FindClose(PSEARCH pSearch));
1. DECLFSDAPI(HANDLE,FSDAPI, CreateFileW(PVOLUME pVolume, HANDLE hProc, PCWSTR pwsFileName, DWORD dwAccess, DWORD dwShareMode, PSECURITY_ATTRIBUTES pSecurityAttributes, DWORD dwCreate, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile));
1. DECLFSDAPI(BOOL,  FSDAPI, ReadFile(PFILE pFile, PVOID pBuffer, DWORD cbRead, PDWORD pcbRead, OVERLAPPED *pOverlapped));
1. DECLFSDAPI(BOOL,  FSDAPI, ReadFileWithSeek(PFILE pFile, PVOID pBuffer, DWORD cbRead, PDWORD pcbRead, OVERLAPPED *pOverlapped, DWORD dwLowOffset, DWORD dwHighOffset));
1. DECLFSDAPI(BOOL,  FSDAPI, WriteFile(PFILE pFile, PCVOID pBuffer, DWORD cbWrite, PDWORD pcbWritten, OVERLAPPED *pOverlapped));
1. DECLFSDAPI(BOOL,  FSDAPI, WriteFileWithSeek(PFILE pFile, PCVOID pBuffer, DWORD cbWrite, PDWORD pcbWritten, OVERLAPPED *pOverlapped, DWORD dwLowOffset, DWORD dwHighOffset));
1. DECLFSDAPI(DWORD, FSDAPI, SetFilePointer(PFILE pFile, LONG lDistanceToMove, PLONG pDistanceToMoveHigh, DWORD dwMoveMethod));
1. DECLFSDAPI(DWORD, FSDAPI, GetFileSize(PFILE pFile, PDWORD pFileSizeHigh));
1. DECLFSDAPI(BOOL,  FSDAPI, GetFileInformationByHandle(PFILE pFile, PBY_HANDLE_FILE_INFORMATION pFileInfo));
1. DECLFSDAPI(BOOL,  FSDAPI, FlushFileBuffers(PFILE pFile));
1. DECLFSDAPI(BOOL,  FSDAPI, GetFileTime(PFILE pFile, FILETIME *pCreation, FILETIME *pLastAccess, FILETIME *pLastWrite));
1. DECLFSDAPI(BOOL,  FSDAPI, SetFileTime(PFILE pFile, CONST FILETIME *pCreation, CONST FILETIME *pLastAccess, CONST FILETIME *pLastWrite));
1. DECLFSDAPI(BOOL,  FSDAPI, SetEndOfFile(PFILE pFile));
1. DECLFSDAPI(BOOL,  FSDAPI, DeviceIoControl(PFILE pFile, DWORD dwIoControlCode, PVOID pInBuf, DWORD nInBufSize, PVOID pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned, OVERLAPPED *pOverlapped));
1. DECLFSDAPI(BOOL,  FSDAPI, CloseFile(PFILE pFile));
1. DECLFSDAPI(BOOL,  FSDAPI, RefreshVolume(PVOLUME pVolume));
1. DECLFSDAPI(BOOL,  FSDAPI, FsIoControl(PVOLUME pVolume, DWORD dwIoControlCode, PVOID pInBuf, DWORD nInBufSize, PVOID pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned, OVERLAPPED *pOverlapped));
1. DECLFSDAPI(BOOL,  FSDAPI, LockFileEx(PFILE pFile, DWORD dwFlags, DWORD dwReserved, DWORD nNumberOfBytesToLockLow, DWORD nNumberOfBytesToLockHigh, LPOVERLAPPED lpOverlapped));
1. DECLFSDAPI(BOOL,  FSDAPI, UnlockFileEx(PFILE pFile, DWORD dwReserved, DWORD nNumberOfBytesToUnlockLow, DWORD nNumberOfBytesToUnlockHigh, LPOVERLAPPED lpOverlapped));
1. DECLFSDAPI(BOOL,  FSDAPI, GetVolumeInfo(PVOLUME pVolume, FSD_VOLUME_INFO *pInfo));