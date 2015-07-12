/*
* fileform.h
*
* This file is a part of NSIS.
*
* Copyright (C) 1999-2015 Nullsoft and Contributors
*
* Licensed under the zlib/libpng license (the "License");
* you may not use this file except in compliance with the License.
*
* Licence details can be found in the file COPYING.
*
* This software is provided 'as-is', without any express or implied
* warranty.
*
* Unicode support by Jim Park -- 08/13/2007
*/

#ifndef _FILEFORM_H_
#define _FILEFORM_H_


// * the installer is composed of the following parts:
// exehead (~34kb)
// firstheader (struct firstheader)
// * headers (compressed together):
//   header (struct header - contains pointers to all blocks)
//   * nsis blocks (described in header->blocks)
//     pages (struct page)
//     section headers (struct section)
//     entries/instructions (struct entry)
//     strings (null seperated)
//     language tables (language id, dialog offset, language strings)
//     colors (struct color)
// data block (compressed files and uninstaller data)
// CRC (optional - 4 bytes)
//
// headers + datablock is at least 512 bytes if CRC enabled


#define MAX_ENTRY_OFFSETS 6


// if you want people to not be able to decompile your installers as easily,
// reorder the lines following EW_INVALID_OPCODE randomly.

enum
{
	EW_INVALID_OPCODE,    // zero is invalid. useful for catching errors. (otherwise an all zeroes instruction
						  // does nothing, which is easily ignored but means something is wrong.
	EW_RET,               // return from function call
	EW_NOP,               // Nop/Jump, do nothing: 1, [?new address+1:advance one]
	EW_ABORT,             // Abort: 1 [status]
	EW_QUIT,              // Quit: 0
	EW_CALL,              // Call: 1 [new address+1]
	EW_UPDATETEXT,        // Update status text: 2 [update str, ui_st_updateflag=?ui_st_updateflag:this]
	EW_SLEEP,             // Sleep: 1 [sleep time in milliseconds]
	EW_BRINGTOFRONT,      // BringToFront: 0
	EW_CHDETAILSVIEW,     // SetDetailsView: 2 [listaction,buttonaction]
	EW_SETFILEATTRIBUTES, // SetFileAttributes: 2 [filename, attributes]
	EW_CREATEDIR,         // Create directory: 2, [path, ?update$INSTDIR]
	EW_IFFILEEXISTS,      // IfFileExists: 3, [file name, jump amount if exists, jump amount if not exists]
	EW_SETFLAG,           // Sets a flag: 2 [id, data]
	EW_IFFLAG,            // If a flag: 4 [on, off, id, new value mask]
	EW_GETFLAG,           // Gets a flag: 2 [output, id]
#ifdef NSIS_SUPPORT_RENAME
	EW_RENAME,            // Rename: 3 [old, new, rebootok]
#endif
#ifdef NSIS_SUPPORT_FNUTIL
	EW_GETFULLPATHNAME,   // GetFullPathName: 2 [output, input, ?lfn:sfn]
	EW_SEARCHPATH,        // SearchPath: 2 [output, filename]
	EW_GETTEMPFILENAME,   // GetTempFileName: 2 [output, base_dir]
#endif
#ifdef NSIS_SUPPORT_FILE
	EW_EXTRACTFILE,       // File to extract: 6 [overwriteflag, output filename, compressed filedata, filedatetimelow, filedatetimehigh, allow ignore]
						  //  overwriteflag: 0x1 = no. 0x0=force, 0x2=try, 0x3=if date is newer
#endif
#ifdef NSIS_SUPPORT_DELETE
	EW_DELETEFILE,        // Delete File: 2, [filename, rebootok]
#endif
#ifdef NSIS_SUPPORT_MESSAGEBOX
	EW_MESSAGEBOX,        // MessageBox: 5,[MB_flags,text,retv1:retv2,moveonretv1:moveonretv2]
#endif
#ifdef NSIS_SUPPORT_RMDIR
	EW_RMDIR,             // RMDir: 2 [path, recursiveflag]
#endif
#ifdef NSIS_SUPPORT_STROPTS
	EW_STRLEN,            // StrLen: 2 [output, input]
	EW_ASSIGNVAR,         // Assign: 4 [variable (0-9) to assign, string to assign, maxlen, startpos]
	EW_STRCMP,            // StrCmp: 5 [str1, str2, jump_if_equal, jump_if_not_equal, case-sensitive?]
#endif
#ifdef NSIS_SUPPORT_ENVIRONMENT
	EW_READENVSTR,        // ReadEnvStr/ExpandEnvStrings: 3 [output, string_with_env_variables, IsRead]
#endif
#ifdef NSIS_SUPPORT_INTOPTS
	EW_INTCMP,            // IntCmp: 6 [val1, val2, equal, val1<val2, val1>val2, unsigned?]
	EW_INTOP,             // IntOp: 4 [output, input1, input2, op] where op: 0=add, 1=sub, 2=mul, 3=div, 4=bor, 5=band, 6=bxor, 7=bnot input1, 8=lnot input1, 9=lor, 10=land], 11=1%2
	EW_INTFMT,            // IntFmt: [output, format, input]
#endif
#ifdef NSIS_SUPPORT_STACK
	EW_PUSHPOP,           // Push/Pop/Exchange: 3 [variable/string, ?pop:push, ?exch]
#endif
#ifdef NSIS_SUPPORT_HWNDS
	EW_FINDWINDOW,        // FindWindow: 5, [outputvar, window class,window name, window_parent, window_after]
	EW_SENDMESSAGE,       // SendMessage: 6 [output, hwnd, msg, wparam, lparam, [wparamstring?1:0 | lparamstring?2:0 | timeout<<2]
	EW_ISWINDOW,          // IsWindow: 3 [hwnd, jump_if_window, jump_if_notwindow]
#endif

#ifdef NSIS_CONFIG_ENHANCEDUI_SUPPORT
	EW_GETDLGITEM,        // GetDlgItem:        3: [outputvar, dialog, item_id]
	EW_SETCTLCOLORS,      // SerCtlColors:      3: [hwnd, pointer to struct colors]
	EW_SETBRANDINGIMAGE,  // SetBrandingImage:  1: [Bitmap file]
	EW_CREATEFONT,        // CreateFont:        5: [handle output, face name, height, weight, flags]
	EW_SHOWWINDOW,        // ShowWindow:        2: [hwnd, show state]
#endif

#ifdef NSIS_SUPPORT_SHELLEXECUTE
	EW_SHELLEXEC,         // ShellExecute program: 4, [shell action, complete commandline, parameters, showwindow]
#endif

#ifdef NSIS_SUPPORT_EXECUTE
	EW_EXECUTE,           // Execute program: 3,[complete command line,waitflag,>=0?output errorcode]
#endif

#ifdef NSIS_SUPPORT_GETFILETIME
	EW_GETFILETIME,       // GetFileTime; 3 [file highout lowout]
#endif

#ifdef NSIS_SUPPORT_GETDLLVERSION
	EW_GETDLLVERSION,     // GetDLLVersion: 3 [file highout lowout]
#endif

#ifdef NSIS_SUPPORT_ACTIVEXREG
	EW_REGISTERDLL,       // Register DLL: 3,[DLL file name, string ptr of function to call, text to put in display (<0 if none/pass parms), 1 - no unload, 0 - unload]
#endif

#ifdef NSIS_SUPPORT_CREATESHORTCUT
	EW_CREATESHORTCUT,    // Make Shortcut: 5, [link file, target file, parameters, icon file, iconindex|show mode<<8|hotkey<<16|noworkingdir]
#endif

#ifdef NSIS_SUPPORT_COPYFILES
	EW_COPYFILES,         // CopyFiles: 3 [source mask, destination location, flags]
#endif

#ifdef NSIS_SUPPORT_REBOOT
	EW_REBOOT,            // Reboot: 0
#endif

#ifdef NSIS_SUPPORT_INIFILES
	EW_WRITEINI,          // Write INI String: 4, [Section, Name, Value, INI File]
	EW_READINISTR,        // ReadINIStr: 4 [output, section, name, ini_file]
#endif

#ifdef NSIS_SUPPORT_REGISTRYFUNCTIONS
	EW_DELREG,            // DeleteRegValue/DeleteRegKey: 4, [root key(int), KeyName, ValueName, delkeyonlyifempty]. ValueName is -1 if delete key
	EW_WRITEREG,          // Write Registry value: 5, [RootKey(int),KeyName,ItemName,ItemData,typelen]
						  //  typelen=1 for str, 2 for dword, 3 for binary, 0 for expanded str
	EW_READREGSTR,        // ReadRegStr: 5 [output, rootkey(int), keyname, itemname, ==1?int::str]
	EW_REGENUM,           // RegEnum: 5 [output, rootkey, keyname, index, ?key:value]
#endif

#ifdef NSIS_SUPPORT_FILEFUNCTIONS
	EW_FCLOSE,            // FileClose: 1 [handle]
	EW_FOPEN,             // FileOpen: 4  [name, openmode, createmode, outputhandle]
	EW_FPUTS,             // FileWrite: 3 [handle, string, ?int:string]
	EW_FGETS,             // FileRead: 4  [handle, output, maxlen, ?getchar:gets]
	EW_FSEEK,             // FileSeek: 4  [handle, offset, mode, >=0?positionoutput]
#endif//NSIS_SUPPORT_FILEFUNCTIONS

#ifdef NSIS_SUPPORT_FINDFIRST
	EW_FINDCLOSE,         // FindClose: 1 [handle]
	EW_FINDNEXT,          // FindNext: 2  [output, handle]
	EW_FINDFIRST,         // FindFirst: 2 [filespec, output, handleoutput]
#endif

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
	EW_WRITEUNINSTALLER,  // WriteUninstaller: 3 [name, offset, icon_size]
#endif

#ifdef NSIS_CONFIG_LOG
	EW_LOG,               // LogText: 2 [0, text] / LogSet: [1, logstate]
#endif

#ifdef NSIS_CONFIG_COMPONENTPAGE
	EW_SECTIONSET,        // SectionSetText:    3: [idx, 0, text]
						  // SectionGetText:    3: [idx, 1, output]
						  // SectionSetFlags:   3: [idx, 2, flags]
						  // SectionGetFlags:   3: [idx, 3, output]
	EW_INSTTYPESET,       // InstTypeSetFlags:  3: [idx, 0, flags]
						  // InstTypeGetFlags:  3: [idx, 1, output]
#endif

						  // instructions not actually implemented in exehead, but used in compiler.
	EW_GETLABELADDR,      // both of these get converted to EW_ASSIGNVAR
	EW_GETFUNCTIONADDR,

#ifdef NSIS_LOCKWINDOW_SUPPORT
	EW_LOCKWINDOW,
#endif

#ifdef _UNICODE     // opcodes available only in Unicode installers must be at the end of the enumeration
#ifdef NSIS_SUPPORT_FILEFUNCTIONS
	EW_FPUTWS,            // FileWriteUTF16LE: 3 [handle, string, ?int:string]
	EW_FGETWS,            // FileReadUTF16LE: 4 [handle, output, maxlen, ?getchar:gets]
#endif//NSIS_SUPPORT_FILEFUNCTIONS
#endif
};

#define FH_FLAGS_MASK 15
#define FH_FLAGS_UNINSTALL 1
#ifdef NSIS_CONFIG_SILENT_SUPPORT
#  define FH_FLAGS_SILENT 2
#endif
#ifdef NSIS_CONFIG_CRC_SUPPORT
#  define FH_FLAGS_NO_CRC 4
#  define FH_FLAGS_FORCE_CRC 8
#endif

#define FH_SIG 0xDEADBEEF

// neato surprise signature that goes in firstheader. :)
#define FH_INT1 0x6C6C754E
#define FH_INT2 0x74666F73
#define FH_INT3 0x74736E49

typedef struct
{
	int flags; // FH_FLAGS_*
	int siginfo;  // FH_SIG

	int nsinst[3]; // FH_INT1,FH_INT2,FH_INT3

				   // these point to the header+sections+entries+stringtable in the datablock
	int length_of_header;

	// this specifies the length of all the data (including the firstheader and CRC)
	int length_of_all_following_data;
} firstheader;

// Flags for common_header.flags
#define CH_FLAGS_DETAILS_SHOWDETAILS 1
#define CH_FLAGS_DETAILS_NEVERSHOW 2
#define CH_FLAGS_PROGRESS_COLORED 4
#ifdef NSIS_CONFIG_SILENT_SUPPORT
#define CH_FLAGS_SILENT 8
#define CH_FLAGS_SILENT_LOG 16
#endif
#define CH_FLAGS_AUTO_CLOSE 32
#define CH_FLAGS_DIR_NO_SHOW 64
#define CH_FLAGS_NO_ROOT_DIR 128
#ifdef NSIS_CONFIG_COMPONENTPAGE
#define CH_FLAGS_COMP_ONLY_ON_CUSTOM 256
#define CH_FLAGS_NO_CUSTOM 512
#endif

// nsis blocks
struct block_header
{
	int offset;
	int num;
};

enum
{
#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
	NB_PAGES,
#endif
	NB_SECTIONS,
	NB_ENTRIES,
	NB_STRINGS,
	NB_LANGTABLES,
	NB_CTLCOLORS,
#ifdef NSIS_SUPPORT_BGBG
	NB_BGFONT,
#endif
	NB_DATA,

	BLOCKS_NUM
};

// nsis strings
typedef TCHAR NSIS_STRING[NSIS_MAX_STRLEN];

// Settings common to both installers and uninstallers
typedef struct
{
	int flags; // CH_FLAGS_*
	struct block_header blocks[BLOCKS_NUM];

	// InstallDirRegKey stuff
	int install_reg_rootkey;
	// these two are not processed!
	int install_reg_key_ptr, install_reg_value_ptr;

#ifdef NSIS_SUPPORT_BGBG
	int bg_color1, bg_color2, bg_textcolor;
#endif

#ifdef NSIS_CONFIG_VISIBLE_SUPPORT
	// installation log window colors
	int lb_bg, lb_fg;
#endif

	// langtable size
	int langtable_size;

#ifdef NSIS_CONFIG_LICENSEPAGE
	// license background color
	int license_bg;
#endif//NSIS_CONFIG_LICENSEPAGE

#ifdef NSIS_SUPPORT_CODECALLBACKS
	// .on* calls
	int code_onInit;
	int code_onInstSuccess;
	int code_onInstFailed;
	int code_onUserAbort;
#ifdef NSIS_CONFIG_ENHANCEDUI_SUPPORT
	int code_onGUIInit;
	int code_onGUIEnd;
	int code_onMouseOverSection;
#endif//NSIS_CONFIG_ENHANCEDUI_SUPPORT
	int code_onVerifyInstDir;
#ifdef NSIS_CONFIG_COMPONENTPAGE
	int code_onSelChange;
#endif//NSIS_CONFIG_COMPONENTPAGE
#ifdef NSIS_SUPPORT_REBOOT
	int code_onRebootFailed;
#endif//NSIS_SUPPORT_REBOOT
#endif//NSIS_SUPPORT_CODECALLBACKS

#ifdef NSIS_CONFIG_COMPONENTPAGE
	int install_types[NSIS_MAX_INST_TYPES + 1];
#endif

	int install_directory_ptr; // default install dir.
	int install_directory_auto_append; // auto append part

#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
	int str_uninstchild;
	int str_uninstcmd;
#endif//NSIS_CONFIG_UNINSTALL_SUPPORT
#ifdef NSIS_SUPPORT_MOVEONREBOOT
	int str_wininit;   // Points to the path of wininit.ini
#endif//NSIS_SUPPORT_MOVEONREBOOT
} header;

#ifdef NSIS_SUPPORT_CODECALLBACKS
// callback indices
enum
{
	CB_ONINIT,
	CB_ONINSTSUCCESS,
	CB_ONINSTFAILED,
	CB_ONUSERABORT,
#ifdef NSIS_CONFIG_ENHANCEDUI_SUPPORT
	CB_ONGUIINIT,
	CB_ONGUIEND,
	CB_ONMOUSEOVERSECTION,
#endif//NSIS_CONFIG_ENHANCEDUI_SUPPORT
	CB_ONVERIFYINSTDIR,
#ifdef NSIS_CONFIG_COMPONENTPAGE
	CB_ONSELCHANGE,
#endif//NSIS_CONFIG_COMPONENTPAGE
#ifdef NSIS_SUPPORT_REBOOT
	CB_ONREBOOTFAILED
#endif//NSIS_SUPPORT_REBOOT
};
#endif//NSIS_SUPPORT_CODECALLBACKS

// used for section->flags
#define SF_SELECTED   1
#define SF_SECGRP     2
#define SF_SECGRPEND  4
#define SF_BOLD       8
#define SF_RO         16
#define SF_EXPAND     32
#define SF_PSELECTED  64
#define SF_TOGGLED    128
#define SF_NAMECHG    256

typedef struct
{
	int name_ptr; // initial name pointer
	int install_types; // bits set for each of the different install_types, if any.
	int flags; // SF_* - defined above
			   // for labels, it looks like it's only used to track how often it is used.
	int code;       // The "address" of the start of the code in count of struct entries.
	int code_size;  // The size of the code in num of entries?
	int size_kb;
	TCHAR name[NSIS_MAX_STRLEN]; // '' for invisible sections
} section;

#define SECTION_OFFSET(field) (FIELD_OFFSET(section, field)/sizeof(int))

typedef struct
{
	int which;   // EW_* enum.  Look at the enum values to see what offsets mean.
	int offsets[MAX_ENTRY_OFFSETS]; // count and meaning of offsets depend on 'which'
									// sometimes they are just straight int values or bool
									// values and sometimes they are indices into string
									// tables.
} entry;

// page window proc
enum
{
#ifdef NSIS_CONFIG_LICENSEPAGE
	PWP_LICENSE,
#endif
#ifdef NSIS_CONFIG_COMPONENTPAGE
	PWP_SELCOM,
#endif
	PWP_DIR,
	PWP_INSTFILES,
#ifdef NSIS_CONFIG_UNINSTALL_SUPPORT
	PWP_UNINST,
#endif
	PWP_COMPLETED,
	PWP_CUSTOM
};

// page flags
#define PF_BACK_ENABLE 256
#define PF_NEXT_ENABLE 2
#define PF_CANCEL_ENABLE 4
#define PF_BACK_SHOW 8 // must be SW_SHOWNA, don't change
#define PF_LICENSE_STREAM 16
#define PF_LICENSE_FORCE_SELECTION 32
#define PF_LICENSE_NO_FORCE_SELECTION 64
#define PF_LICENSE_SELECTED 1 // must be 1
#define PF_NO_NEXT_FOCUS 128
#define PF_PAGE_EX 512
#define PF_DIR_NO_BTN_DISABLE 1024

typedef struct
{
	int dlg_id; // dialog resource id
	int wndproc_id;

#ifdef NSIS_SUPPORT_CODECALLBACKS
	// called before the page is created, or if custom to show the page
	// use Abort to skip the page
	int prefunc;
	// called right before page is shown
	int showfunc;
	// called when the user leaves to the next page
	// use Abort to force the user to stay on this page
	int leavefunc;
#endif //NSIS_SUPPORT_CODECALLBACKS

	int flags;

	int caption;
	int back;
	int next;
	int clicknext;
	int cancel;

	int parms[5];
} page;

// text/bg color
#define CC_TEXT 1
#define CC_TEXT_SYS 2
#define CC_BK 4
#define CC_BK_SYS 8
#define CC_BKB 16

#pragma pack(push, 4)
typedef struct
{
	COLORREF text;
	COLORREF bkc;
	UINT lbStyle;
#ifndef MAKENSIS
	HBRUSH bkb;
#else
	INT32 bkb;
#endif
	int bkmode;
	int flags;
} ctlcolors32;
typedef struct
{
	COLORREF text;
	COLORREF bkc;
#ifndef MAKENSIS
	HBRUSH bkb; // NOTE: Placed above lbStyle for better alignment
#else
	INT64 bkb;
#endif
	UINT lbStyle;
	int bkmode;
	int flags;
} ctlcolors64;
#if defined(_WIN64) && !defined(MAKENSIS)
#  define ctlcolors ctlcolors64
#else
#  define ctlcolors ctlcolors32
#endif
#pragma pack(pop)

// constants for myDelete (util.c)
#define DEL_DIR 1
#define DEL_RECURSE 2
#define DEL_REBOOT 4
#define DEL_SIMPLE 8

// special escape characters used in strings: (we use control codes in order to minimize conflicts with normal characters)
#define NS_LANG_CODE  _T('\x01')    // for a langstring
#define NS_SHELL_CODE _T('\x02')    // for a shell folder path
#define NS_VAR_CODE   _T('\x03')    // for a variable
#define NS_SKIP_CODE  _T('\x04')    // to consider next character as a normal character
#define NS_IS_CODE(x) ((x) <= NS_SKIP_CODE) // NS_SKIP_CODE must always be the higher code

// We are doing this to store an integer value into a char string and we
// don't want false end of string values
#define CODE_SHORT(x) (WORD)((((WORD)(x) & 0x7F) | (((WORD)(x) & 0x3F80) << 1) | 0x8080))
#define MAX_CODED 0x3FFF
// This macro takes a pointer to CHAR
#define DECODE_SHORT(c) (((((char*)c)[1] & 0x7F) << 7) | (((char*)c)[0] & 0x7F))

#define NSIS_INSTDIR_INVALID 1
#define NSIS_INSTDIR_NOT_ENOUGH_SPACE 2

#define FIELDN(x, y) (((int *)&x)[y])

#ifdef EXEHEAD

// the following are only used/implemented in exehead, not makensis.

int NSISCALL isheader(firstheader *h); // returns 0 on not header, length_of_datablock on success

									   // returns nonzero on error
									   // returns 0 on success
									   // on success, m_header will be set to a pointer that should eventually be GlobalFree()'d.
									   // (or m_uninstheader)
const TCHAR * NSISCALL loadHeaders(int cl_flags);

int NSISCALL _dodecomp(int offset, HANDLE hFileOut, unsigned char *outbuf, int outbuflen);

#define GetCompressedDataFromDataBlock(offset, hFileOut) _dodecomp(offset,hFileOut,NULL,0)
#define GetCompressedDataFromDataBlockToMemory(offset, out, out_len) _dodecomp(offset,NULL,out,out_len)

extern HANDLE g_db_hFile;
extern int g_quit_flag;

BOOL NSISCALL ReadSelfFile(LPVOID lpBuffer, DWORD nNumberOfBytesToRead);
DWORD NSISCALL SetSelfFilePointer(LONG lDistanceToMove);

extern struct block_header g_blocks[BLOCKS_NUM];
extern header *g_header;
extern int g_flags;
extern int g_filehdrsize;
extern int g_is_uninstaller;

#define g_pages ((page*)g_blocks[NB_PAGES].offset)
#define g_sections ((section*)g_blocks[NB_SECTIONS].offset)
#define num_sections (g_blocks[NB_SECTIONS].num)
#define g_entries ((entry*)g_blocks[NB_ENTRIES].offset)
#endif

#endif //_FILEFORM_H_
