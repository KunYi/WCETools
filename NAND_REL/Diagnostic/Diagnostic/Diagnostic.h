//======================================================================
// Header file
//
// Written for the book Programming Windows CE
// Copyright (C) 2003 Douglas Boling
//======================================================================
// Returns number of elements
#define dim(x) (sizeof(x) / sizeof(x[0])) 

//----------------------------------------------------------------------
// Generic defines and data types
//
struct decodeUINT {                            // Structure associates
    UINT Code;                                 // messages 
                                               // with a function. 
    BOOL (*Fxn)(HWND, UINT, WPARAM, LPARAM);
}; 
struct decodeCMD {                             // Structure associates
    UINT Code;                                 // menu IDs with a 
    LRESULT (*Fxn)(HWND, WORD, HWND, WORD);    // function.
};
struct decodePROC {
    BOOL Result;
    BOOL (*Fxn)(HWND, WORD, HWND, WORD);
};
//----------------------------------------------------------------------
// Program defines used by application
//

//----------------------------------------------------------------------
// Generic defines used by application

#define  ID_ICON             1   

//----------------------------------------------------------------------
// Function prototypes
//

// Window procedures
BOOL CALLBACK MainDlgProc (HWND, UINT, WPARAM, LPARAM);

// Message handlers
BOOL DoCreateDialogMain (HWND, UINT, WPARAM, LPARAM);
BOOL DoInitDialogMain (HWND, UINT, WPARAM, LPARAM);
BOOL DoCommandMain (HWND, UINT, WPARAM, LPARAM);

// Command functions
LPARAM DoMainCommandExit (HWND, WORD, HWND, WORD);
LPARAM DoMainCommandAddTestItems (HWND, WORD, HWND, WORD);
LPARAM DoMainCommandStartTest (HWND, WORD, HWND, WORD);
BOOL Memory_Test (HWND, WORD, HWND, WORD);
BOOL LCD_Test (HWND, WORD, HWND, WORD);
BOOL Touch_Test (HWND, WORD, HWND, WORD);
BOOL Backlight_Test (HWND, WORD, HWND, WORD);
BOOL LAN_Test (HWND, WORD, HWND, WORD);
BOOL Frame_LED_Test (HWND, WORD, HWND, WORD);
BOOL Text_LED_Test (HWND, WORD, HWND, WORD);
BOOL Buzzer_Test (HWND, WORD, HWND, WORD);
BOOL Suspend_Test (HWND, WORD, HWND, WORD);