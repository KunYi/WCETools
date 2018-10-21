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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

    api.h
    
Abstract:
    API for MMD.

Notes: 


--*/
#ifndef _API_H_
#define _API_H_

#pragma once

# ifdef     __cplusplus
extern "C" {
# endif

#define GREEN		0
#define RED			1
#define ON			0
#define OFF			1

// possible return value from StartUpgrade
#define OK			0
#define NOT_FOUND	1
#define NOT_VALID	2
#ifdef FAILED
#undef FAILED
#define FAILED		-1
#endif

// DeviceIoControl constants
#define kAPIIOCTLFunction	12400
#define IOCTL_BAK_SETLEVEL			kAPIIOCTLFunction+1
#define IOCTL_BAK_GETLEVEL			kAPIIOCTLFunction+2

// function definition

//-----------------------------------------------------------------------------
// LED Frame Control API
//-----------------------------------------------------------------------------
typedef BOOL	(* PFNSetGreenLEDFrameValue)(DWORD value);	// Value= 0-100, 0 = off , 100 = max.
typedef DWORD	(* PFNGetGreenLEDFrameValue)();
typedef BOOL	(* PFNSetRedLEDFrameValue)(DWORD value);	// Value= 0-100, 0 = off , 100 = max.
typedef DWORD	(* PFNGetRedLEDFrameValue)();
typedef BOOL	(* PFNSetLEDFrameColor)(DWORD color);		// Sets the color of LED frame to either color = RED / GREEN
typedef DWORD	(* PFNGetLEDFrameColor)();
typedef BOOL	(* PFNSetLEDStatus)(DWORD onoff);			// Turn LED frame on or off
typedef DWORD	(* PFNGetLEDFrameStatus)();


//-----------------------------------------------------------------------------
// Text LED Control API
//-----------------------------------------------------------------------------
typedef BOOL	(* PFNSetTextLEDStatus)(DWORD onoff);		// Turn text LED on or off
typedef DWORD	(* PFNGetTextLEDStatus)();


//-----------------------------------------------------------------------------
// Buzzer Control API
//-----------------------------------------------------------------------------
typedef BOOL	(* PFNSetBuzzerValue)(DWORD value);			// Value= 0-100, 0 = off , 100 = max
typedef DWORD	(* PFNGetBuzzerValue)();
typedef BOOL	(* PFNBuzzerBeep)(DWORD time);				// Start the buzzer for time ms, Value = 0 - 5000

//-----------------------------------------------------------------------------
// Backlight Control API
//-----------------------------------------------------------------------------
typedef BOOL	(* PFNSetBacklightValue)(DWORD value);		// Value= 0-100, 0 = off , 100 = max
typedef DWORD	(* PFNGetBacklightValue)();
typedef BOOL	(* PFNSetBacklightStatus)(DWORD onoff);		// Turn backlight on or off
typedef DWORD	(* PFNGetBacklightStatus)();

//-----------------------------------------------------------------------------
// Updating OS Control API
//-----------------------------------------------------------------------------
typedef DWORD	(* PFNStartUpgrade)(LPCTSTR path);
typedef DWORD	(* PFNGetUpgradeProgress)();

# ifdef     __cplusplus
}
# endif


#endif	//	_API_H_

