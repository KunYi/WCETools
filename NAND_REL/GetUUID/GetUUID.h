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

//----------------------------------------------------------------------
// Generic defines used by application

#define  ID_ICON             1
#define  ID_TIMER            2

//----------------------------------------------------------------------
// Function prototypes
//

// Window procedures
BOOL CALLBACK MainDlgProc (HWND, UINT, WPARAM, LPARAM);

// Message handlers
BOOL DoInitDlgMain (HWND, UINT, WPARAM, LPARAM);
BOOL DoCommandMain (HWND, UINT, WPARAM, LPARAM);

// Command functions
LPARAM DoMainCommandOk (HWND, WORD, HWND, WORD);
LPARAM DoMainCommandExit (HWND, WORD, HWND, WORD);
