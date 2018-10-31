!if 0
Copyright (c) Microsoft Corporation.  All rights reserved.
!endif
!if 0
Use of this source code is subject to the terms of the Microsoft end-user
license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
If you did not accept the terms of the EULA, you are not authorized to use
this source code. For a copy of the EULA, please see the LICENSE.RTF on your
install media.
!endif

!if 0
To build this utility, type "nmake".
!endif

all: RLDR.exe

clean:
	del *.obj *.map *.exe *.com *.sbr

RLDR.exe: main.obj
	c:\msvc\bin\link main.obj ,RLDR.exe,,c:\msvc\lib\oldnames.lib c:\msvc\lib\slibce.lib,,

main.obj: main.c
	c:\msvc\bin\cl /c /Ic:\msvc\include main.c
