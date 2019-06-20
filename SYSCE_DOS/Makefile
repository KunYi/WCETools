!if 0
Copyright (c) Microsoft Corporation.  All rights reserved.
!endif
!if 0
Use of this sample source code is subject to the terms of the Microsoft
license agreement under which you licensed this sample source code. If
you did not accept the terms of the license agreement, you are not
authorized to use this sample source code. For the terms of the license,
please see the license agreement between you and Microsoft or, if applicable,
see the LICENSE.RTF on your install media or the root of your tools installation.
THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
!endif

!if 0
To build this utility, type "nmake". toolchain: MSVC V1.52
!endif

all: sysce.exe

clean:
        del *.obj *.map *.exe *.com *.sbr

sysce.exe: main.obj
        $(MSVC_DIRS)\bin\link main.obj ,sysce.exe,,$(MSVC_DIRS)\lib\oldnames.lib $(MSVC_DIRS)\lib\slibce.lib,,

main.obj: main.c
        $(MSVC_DIRS)\bin\cl /c /I$(MSVC_DIRS)\include main.c
