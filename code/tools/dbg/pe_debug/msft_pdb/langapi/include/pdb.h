// Debug Information API
// Copyright (C) Microsoft Corporation.  All Rights Reserved.

#pragma once

#ifndef _VC_VER_INC
#include "vcver.h"
#endif

#pragma warning(push)
#pragma warning(disable:4201)

#ifndef __PDB_INCLUDED__
#define __PDB_INCLUDED__

typedef int BOOL;
typedef unsigned UINT;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned __int64 DWORDLONG;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef ULONG   INTV;       // interface version number
typedef ULONG   IMPV;       // implementation version number
typedef ULONG   SIG;        // unique (across PDB instances) signature
typedef ULONG   AGE;        // no. of times this instance has been updated
typedef const char*     SZ_CONST;   // const string
typedef void *          PV;
typedef const void *    PCV;

#if defined(__midl)

cpp_quote("#ifndef GUID_DEFINED")
cpp_quote("#define GUID_DEFINED")
typedef struct _GUID {          // size is 16
    DWORD   Data1;
    WORD    Data2;
    WORD    Data3;
    BYTE    Data4[8];
} GUID;
cpp_quote("#endif")

#else // defined(__midl)

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID {          // size is 16
    DWORD   Data1;
    WORD    Data2;
    WORD    Data3;
    BYTE    Data4[8];
} GUID;
#endif // !GUID_DEFINED

#endif // defined(__midl)

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef long HRESULT;

#endif // !_HRESULT_DEFINED


typedef GUID            SIG70;      // new to 7.0 are 16-byte guid-like signatures
typedef SIG70 *         PSIG70;
typedef const SIG70 *   PCSIG70;


#if defined(__midl)

cpp_quote("#ifndef _WCHAR_T_DEFINED")
typedef unsigned short wchar_t;
cpp_quote("#define _WCHAR_T_DEFINED")
cpp_quote("#endif")

#else

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#endif



#if !defined(__midl)

enum PDBINTV {
    PDBIntv110      = 20091201,
    PDBIntv80       = 20030901,
    PDBIntv70       = 20001102,
    PDBIntv70Dep    = 20000406,
    PDBIntv69       = 19990511,
    PDBIntv61       = 19980914,
    PDBIntv50a      = 19970116,
    PDBIntv60       = PDBIntv50a,
    PDBIntv50       = 19960502,
    PDBIntv41       = 920924,
    PDBIntv         = PDBIntv110,
};

enum PDBIMPV {
    PDBImpvVC2      = 19941610,
    PDBImpvVC4      = 19950623,
    PDBImpvVC41     = 19950814,
    PDBImpvVC50     = 19960307,
    PDBImpvVC98     = 19970604,
    PDBImpvVC70     = 20000404,
    PDBImpvVC70Dep  = 19990604,  // deprecated vc70 implementation version
    PDBImpvVC80     = 20030901,
    PDBImpvVC110    = 20091201,
    PDBImpvVC140    = 20140508,
    PDBImpv         = PDBImpvVC110,
};

enum PDBConsts {
    niNil        = 0,
    PDB_MAX_PATH = 260,
    cbErrMax     = 1024,
};

#endif


// cvinfo.h type index, intentionally typedef'ed here to check equivalence.
typedef unsigned short  CV_typ16_t;
typedef unsigned long   CV_typ_t;
typedef unsigned long   CV_pubsymflag_t;    // must be same as CV_typ_t.

typedef CV_typ_t        TI;     // PDB name for type index
typedef CV_typ16_t      TI16;   // 16-bit version
typedef unsigned long   NI;     // name index
typedef TI *            PTi;
typedef TI16 *          PTi16;

typedef BYTE            ITSM;   // type server map index
typedef ITSM*           PITSM;

typedef BOOL    (__stdcall *PFNVALIDATEDEBUGINFOFILE) (const char * szFile, ULONG * errcode );


#if !defined(__midl)

typedef struct _tagSEARCHDEBUGINFO {
    DWORD   cb;                         // doubles as version detection
    BOOL    fMainDebugFile;             // indicates "core" or "ancilliary" file
                                        // eg: main.exe has main.pdb and foo.lib->foo.pdb
    char *  szMod;                      // exe/dll
    char *  szLib;                      // lib if appropriate
    char *  szObj;                      // object file
    char * *rgszTriedThese;             // list of ones that were tried,
                                        // NULL terminated list of LSZ's
    char  szValidatedFile[PDB_MAX_PATH];// output of validated filename,
    PFNVALIDATEDEBUGINFOFILE
            pfnValidateDebugInfoFile;   // validation function
    char *  szExe;                      // exe/dll
} SEARCHDEBUGINFO, *PSEARCHDEBUGINFO;

typedef BOOL ( __stdcall * PfnFindDebugInfoFile) ( PSEARCHDEBUGINFO );

#endif

#define PdbInterface struct

PdbInterface PDB;                   // program database
PdbInterface DBI;                   // debug information within the PDB
PdbInterface Mod;                   // a module within the DBI
PdbInterface TPI;                   // type info within the DBI
PdbInterface GSI;                   // global symbol info
PdbInterface SO;                    
PdbInterface Stream;                // some named bytestream in the PDB
PdbInterface StreamImage;           // some memory mapped stream
PdbInterface NameMap;              // name mapping
PdbInterface Enum;                 // generic enumerator
PdbInterface EnumNameMap;          // enumerate names within a NameMap
PdbInterface EnumContrib;          // enumerate contributions
PdbInterface Dbg;                   // misc debug data (FPO, OMAP, etc)
PdbInterface Src;                   // Src file data
PdbInterface EnumSrc;               // Src file enumerator
PdbInterface SrcHash;               // Src file hasher
PdbInterface EnumLines;
PdbInterface EnumThunk;
PdbInterface EnumSyms;

#if !defined(__midl)
typedef PdbInterface PDB PDB;
typedef PdbInterface DBI DBI;
typedef PdbInterface Mod Mod;
typedef PdbInterface TPI TPI;
typedef PdbInterface GSI GSI;
typedef PdbInterface SO SO;
typedef PdbInterface Stream Stream;
typedef PdbInterface StreamImage StreamImage;
typedef PdbInterface NameMap NameMap;
typedef PdbInterface Enum Enum;
typedef PdbInterface EnumStreamNames EnumStreamNames;
typedef PdbInterface EnumNameMap EnumNameMap;
typedef PdbInterface EnumContrib EnumContrib;
typedef PdbInterface EnumSyms EnumSyms;
typedef PdbInterface WidenTi WidenTi;
typedef PdbInterface Dbg Dbg;
typedef PdbInterface EnumThunk EnumThunk;
typedef PdbInterface Src Src;
typedef PdbInterface EnumSrc EnumSrc;
typedef PdbInterface SrcHash SrcHash;
typedef PdbInterface EnumLines EnumLines;
typedef PdbInterface EnumThunk EnumThunk;
typedef PdbInterface EnumSyms EnumSyms;
#endif  // !__midl

typedef PdbInterface SrcHash *   PSrcHash;

typedef long EC;            // error code

#if !defined(__midl)

enum PDBErrors {
    EC_OK,                          // no problem
    EC_USAGE,                       // invalid parameter or call order
    EC_OUT_OF_MEMORY,               // out of heap
    EC_FILE_SYSTEM,                 // "pdb name", can't write file, out of disk, etc.
    EC_NOT_FOUND,                   // "pdb name", PDB file not found
    EC_INVALID_SIG,                 // "pdb name", PDB::OpenValidate() and its clients only
    EC_INVALID_AGE,                 // "pdb name", PDB::OpenValidate() and its clients only
    EC_PRECOMP_REQUIRED,            // "obj name", Mod::AddTypes() only
    EC_OUT_OF_TI,                   // "pdb name", TPI::QueryTiForCVRecord() only
    EC_NOT_IMPLEMENTED,             // -
    EC_V1_PDB,                      // "pdb name", PDB::Open* only (obsolete)
    EC_UNKNOWN_FORMAT = EC_V1_PDB,  // pdb can't be opened because it has newer versions of stuff
    EC_FORMAT,                      // accessing pdb with obsolete format
    EC_LIMIT,
    EC_CORRUPT,                     // cv info corrupt, recompile mod
    EC_TI16,                        // no 16-bit type interface present
    EC_ACCESS_DENIED,               // "pdb name", PDB file read-only
    EC_ILLEGAL_TYPE_EDIT,           // trying to edit types in read-only mode
    EC_INVALID_EXECUTABLE,          // not recogized as a valid executable
    EC_DBG_NOT_FOUND,               // A required .DBG file was not found
    EC_NO_DEBUG_INFO,               // No recognized debug info found
    EC_INVALID_EXE_TIMESTAMP,       // Invalid timestamp on Openvalidate of exe
    EC_CORRUPT_TYPEPOOL,            // A corrupted type record was found in a PDB
    EC_DEBUG_INFO_NOT_IN_PDB,       // returned by OpenValidateX
    EC_RPC,                         // Error occured during RPC
    EC_UNKNOWN,                     // Unknown error
    EC_BAD_CACHE_PATH,              // bad cache location specified with symsrv
    EC_CACHE_FULL,                  // symsrv cache is full
    EC_TOO_MANY_MOD_ADDTYPE,        // Addtype is called more then once per mod
    EC_MAX
};

#endif

#if !defined(pure)
#define  pure = 0
#endif

#ifndef PDBCALL
#define PDBCALL  __cdecl
#endif

#ifdef PDB_SERVER
#define PDB_IMPORT_EXPORT(RTYPE)    __declspec(dllexport) RTYPE PDBCALL
#elif   defined(PDB_LIBRARY)
#define PDB_IMPORT_EXPORT(RTYPE)    RTYPE PDBCALL
#else
#define PDB_IMPORT_EXPORT(RTYPE)    __declspec(dllimport) RTYPE PDBCALL
#endif

#define PDBAPI PDB_IMPORT_EXPORT

#ifndef IN
#define IN                  /* in parameter, parameters are IN by default */
#endif
#ifndef OUT
#define OUT                 /* out parameter */
#endif

// Type of callback arg to PDB::OpenValidate5

#if !defined(__midl)

enum POVC
{
    povcNotifyDebugDir,
    povcNotifyOpenDBG,
    povcNotifyOpenPDB,
    povcReserved,
    povcReadExecutableAt,
    povcReadExecutableAtRVA,
    povcRestrictRegistry,
    povcRestrictSymsrv,
    povcRestrictSystemRoot,
    povcNotifyMiscPath,
    povcReadMiscDebugData,
    povcReadCodeViewDebugData,
    povcRestrictOriginalPath,
    povcRestrictReferencePath,
    povcRestrictDBG
};

typedef int (PDBCALL *PDBCALLBACK)();

typedef PDBCALLBACK (PDBCALL *PfnPDBQueryCallback)(void *pvClient, enum POVC povc);

typedef void (PDBCALL *PfnPDBNotifyDebugDir)(void *pvClient, BOOL fExecutable, const struct _IMAGE_DEBUG_DIRECTORY *pdbgdir);
typedef void (PDBCALL *PfnPDBNotifyOpenDBG)(void *pvClient, const wchar_t *wszDbgPath, EC ec, const wchar_t *wszError);
typedef void (PDBCALL *PfnPDBNotifyOpenPDB)(void *pvClient, const wchar_t *wszPdbPath, EC ec, const wchar_t *wszError);
typedef HRESULT (PDBCALL *PfnPDBReadExecutableAt)(void *pvClient, DWORDLONG fo, DWORD cb, void *pv);
typedef HRESULT (PDBCALL *PfnPDBReadExecutableAtRVA)(void *pvClient, DWORD rva, DWORD cb, void *pv);
typedef HRESULT (PDBCALL *PfnPDBRestrictRegistry)(void *pvClient);
typedef HRESULT (PDBCALL *PfnPDBRestrictSymsrv)(void *pvClient);
typedef HRESULT (PDBCALL *PfnPDBRestrictSystemRoot)(void *pvClient);
typedef void (PDBCALL *PfnPDBNotifyMiscPath)(void *pvClient, const wchar_t *wszMiscPath);
typedef HRESULT (PDBCALL *PfnPDBReadCodeViewDebugData)(void *pvClient, DWORD *pcb, void *pv);
typedef HRESULT (PDBCALL *PfnPDBReadMiscDebugData)(void *pvClient, DWORD *pdwTimeStampExe, DWORD *pdwTimeStampDbg, DWORD *pdwSizeOfImage, DWORD *pcb, void *pv);
typedef HRESULT (PDBCALL *PfnPdbRestrictOriginalPath)(void *pvClient);
typedef HRESULT (PDBCALL *PfnPdbRestrictReferencePath)(void *pvClient);
typedef HRESULT (PDBCALL *PfnPdbRestrictDBG) (void *pvClient);

// type of callback arg to PDB::GetRawBytes
typedef BOOL (PDBCALL *PFNfReadPDBRawBytes)(const void *, long);

// type of callback arg to DBI::FSetPfn*
enum DOVC
{
    dovcNotePdbUsed,
    dovcNoteTypeMismatch,
    dovcTmdTypeFilter,
};

typedef int (PDBCALL *DBICALLBACK)();

typedef DBICALLBACK (PDBCALL *PFNDBIQUERYCALLBACK)(void *pvContext, enum DOVC dovc);

typedef void    (__cdecl *PFNNOTEPDBUSED)(
    void *          pvContext,
    const wchar_t * szFile,
    BOOL            fRead,
    BOOL            fWrite);

typedef void  (__cdecl *PFNNOTETYPEMISMATCH)(
    void *          pvContext,
    const wchar_t * szTypeName,
    const wchar_t * szInfo);

typedef BOOL  (__cdecl *PFNTMDTYPEFILTER)(
    void *          pvContext,
    const wchar_t * szUDT);

// interface for error reporting
typedef struct IPDBError
#if defined(__cplusplus)
    {
    virtual EC   QueryLastError( _Out_opt_z_cap_(cchMax) wchar_t * szError, size_t cchMax) pure;
    virtual void SetLastError(EC ec, const wchar_t * wszErr) pure;
    virtual void Destroy() pure;
    }
#endif
IPDBError;

typedef IPDBError * (PDBCALL *PfnPDBErrorCreate)(PDB *ppdb);


// WidenTi interface needs a couple of structures to communicate info back
// and forth.
struct OffMap {
    ULONG       offOld;
    ULONG       offNew;
};
typedef struct OffMap   OffMap;
typedef OffMap *        POffMap;

struct SymConvertInfo {
    ULONG       cbSyms;             // size necessary for converting a block
    ULONG       cSyms;              // count of symbols, necessary to allocate
                                    // mpoffOldoffNew array.
    BYTE *      pbSyms;             // block of symbols (output side)
    OffMap *    rgOffMap;           // OffMap rgOffMap[cSyms]
};
typedef struct SymConvertInfo   SymConvertInfo;
enum { wtiSymsNB09 = 0, wtiSymsNB10 = 1 };

// Filter values for PDBCopyTo
enum { 
    copyRemovePrivate       = 0x00000001,   // remove private debug information
    copyCreateNewSig        = 0x00000002,   // create new signature for target pdb
    copyKeepAnnotation      = 0x00000004,   // keep S_ANNOTATION symbols, filtering on the first string
    copyKeepAnnotation2     = 0x00000008,   // keep S_ANNOTATION symbols, filtering on both the first and last strings
    copyRemoveNamedStream   = 0x00000010,   // remove named stream only
};

// PDBCopy callback signatures and function pointer types for PDB::CopyTo2 and CopyToW2
//
enum PCC {
    pccFilterPublics,
    pccFilterAnnotations,
    pccFilterStreamNames,
};

#if !defined(__cplusplus)
typedef enum PCC    PCC;
#endif  // __cplusplus

typedef BOOL (PDBCALL *PDBCOPYCALLBACK)();
typedef PDBCOPYCALLBACK (PDBCALL *PfnPDBCopyQueryCallback)(void *pvClientContext, PCC pcc);

// Return (true, pszNewPublic==NULL) to keep the name as is,
// (true, pszNewPublic!=NULL) changes name to pszNewPublic,
// false to discard public entirely.
//
typedef BOOL (PDBCALL *PfnPDBCopyFilterPublics)(
    void *          pvClientContext,
    DWORD           dwFilterFlags,
    unsigned int    offPublic,
    unsigned int    sectPublic,
    unsigned int    grfPublic,      // see cvinfo.h, definition of CV_PUBSYMFLAGS_e and
                                    // CV_PUBSYMFLAGS give the format of this bitfield.
    const wchar_t * szPublic,
    wchar_t *       szNewPublic,
    unsigned int    cchNewPublic
    );

// Return true to keep the annotation, false to discard it.
//
typedef BOOL (PDBCALL *PfnPDBCopyFilterAnnotations)(
    void *          pvClientContext,
    const wchar_t * szFirstAnnotation
    );

// Return true to delete the named stream, false to keep it.
//
typedef BOOL (PDBCALL *PfnPDBCopyFilterStreamNames)(
    void *          pvClientContext,
    const wchar_t * szStream
    );

enum DBGTYPE {
    dbgtypeFPO,
    dbgtypeException,   // deprecated
    dbgtypeFixup,
    dbgtypeOmapToSrc,
    dbgtypeOmapFromSrc,
    dbgtypeSectionHdr,
    dbgtypeTokenRidMap,
    dbgtypeXdata,
    dbgtypePdata,
    dbgtypeNewFPO,
    dbgtypeSectionHdrOrig,
    dbgtypeMax          // must be last!
};

typedef enum DBGTYPE DBGTYPE;

// We add a slight bit of structure to dbg blobs so we can record extra
// relevant information there.  Generally, the blobs are lifted right out
// of an image, and need some extra info anyway.  In the case of Xdata, we
// store RVA base of the Xdata there.  This is used to interpret the
// UnwindInfoAddress RVA in the IA64 Pdata entries.
//
enum VerDataBlob {
    vdbOne = 1,
    vdbXdataCur = vdbOne,
    vdbPdataCur = vdbOne,
};

// default blob header
//
typedef struct DbgBlob {
    ULONG   ver;
    ULONG   cbHdr;
    ULONG   cbData;
    //BYTE    rgbDataBlob[];    // Data follows, but to enable simple embedding,
                                // don't use a zero-sized array here.
} DbgBlob;

// "store rva of the base and va of image base" blob header
//
typedef struct DbgRvaVaBlob {
    ULONG       ver;
    ULONG       cbHdr;
    ULONG       cbData;
    ULONG       rvaDataBase;
    DWORDLONG   vaImageBase;
    ULONG       ulReserved1;    // reserved, must be 0
    ULONG       ulReserved2;    // reserved, must be 0
    //BYTE      rgbDataBlob[];  // Data follows, but to enable simple embedding,
                                // don't use a zero-sized array here.
} DbgRvaVaBlob;

// Linker data necessary for relinking an image.  Record contains two SZ strings
// off of the end of the record with two offsets from the base 
//
enum VerLinkInfo {
    vliOne = 1,
    vliTwo = 2,
    vliCur = vliTwo,
};

struct LinkInfo {
    ULONG           cb;             // size of the whole record.  computed as
                                    //  sizeof(LinkInfo) + strlen(szCwd) + 1 +
                                    //  strlen(szCommand) + 1
    ULONG           ver;            // version of this record (VerLinkInfo)
    ULONG           offszCwd;       // offset from base of this record to szCwd
    ULONG           offszCommand;   // offset from base of this record
    ULONG           ichOutfile;     // index of start of output file in szCommand
    ULONG           offszLibs;      // offset from base of this record to szLibs

    // The command includes the full path to the linker, the -re and -out:...
    // swithches.
    // A sample might look like the following:
    // "c:\program files\msdev\bin\link.exe -re -out:debug\foo.exe"
    // with ichOutfile being 48.
    // the -out switch is guaranteed to be the last item in the command line.
#ifdef __cplusplus
    VerLinkInfo Ver() const {
        return VerLinkInfo(ver);
    }
    long Cb() const {
        return cb;
    }
    char *     SzCwd() const {
        return (char *)((char *)(this) + offszCwd);
    }
    char *    SzCommand() const {
        return (char *)((char *)(this) + offszCommand);
    }
    char *    SzOutFile() const {
        return SzCommand() + ichOutfile;
    }
    LinkInfo() :
        cb(0), ver(vliCur), offszCwd(0), offszCommand(0), ichOutfile(0)
    {
    }
    char *    SzLibs() const {
        return (char *)((char *)(this) + offszLibs);
    }

#endif
};

#ifdef __cplusplus
struct LinkInfoW : public LinkInfo
{
    wchar_t* SzCwdW() const {
        return (wchar_t *)((wchar_t *)(this) + offszCwd);
    }
    wchar_t* SzCommandW() const {
        return (wchar_t *)((wchar_t *)(this) + offszCommand);
    }
    wchar_t* SzOutFileW() const {
        return SzCommandW() + ichOutfile;
    }
    wchar_t* SzLibsW() const {
        return (wchar_t *)((wchar_t *)(this) + offszLibs);
    }
};
#else
typedef struct LinkInfo LinkInfoW;
#endif  // __cplusplus

typedef LinkInfoW * PLinkInfoW;

typedef struct LinkInfo LinkInfo;
typedef LinkInfo *      PLinkInfo;

//
// Source (Src) info
//
// This is the source file server for virtual and real source code.
// It is structured as an index on the object file name concatenated
// with 
enum SrcVer {
    srcverOne = 19980827,
};

enum SrcCompress {
    srccompressNone,
    srccompressRLE,
    srccompressHuffman,
    srccompressLZ,
};

struct SrcHeader {
    unsigned long   cb;         // record length
    unsigned long   ver;        // header version
    unsigned long   sig;        // CRC of the data for uniqueness w/o full compare
    unsigned long   cbSource;   // count of bytes of the resulting source
    unsigned char   srccompress;// compression algorithm used

    union {
        unsigned char       grFlags;
        struct {
            unsigned char   fVirtual : 1;   // file is a virtual file (injected)
            unsigned char   pad : 7;        // must be zero
        };
    };

    unsigned char szNames[1];   // file names (szFile "\0" szObj "\0" szVirtual,
                                //  as in: "f.cpp" "\0" "f.obj" "\0" "*inj:1:f.obj")
                                // in the case of non-virtual files, szVirtual is
                                // the same as szFile.
};

typedef struct SrcHeader    SrcHeader;
typedef SrcHeader *         PSrcHeader;
typedef const SrcHeader *   PCSrcHeader;

struct SrcHeaderW {
    unsigned long   cb;         // record length
    unsigned long   ver;        // header version
    unsigned long   sig;        // CRC of the data for uniqueness w/o full compare
    unsigned long   cbSource;   // count of bytes of the resulting source
    unsigned char   srccompress;// compression algorithm used
    union {
        unsigned char       grFlags;
        struct {
            unsigned char   fVirtual : 1;   // file is a virtual file (injected)
            unsigned char   pad : 7;        // must be zero
        };
    };
    wchar_t szNames[1];                  // see comment above
};

typedef struct SrcHeaderW    SrcHeaderW;
typedef SrcHeaderW *         PSrcHeaderW;
typedef const SrcHeaderW *   PCSrcHeaderW;

// cassert(offsetof(SrcHeader,szNames) == offsetof(SrcHeaderW,szNames));


// header used for storing the info and for output to clients who are reading
//
struct SrcHeaderOut {
    unsigned long   cb;         // record length
    unsigned long   ver;        // header version
    unsigned long   sig;        // CRC of the data for uniqueness w/o full compare
    unsigned long   cbSource;   // count of bytes of the resulting source
    unsigned long   niFile;
    unsigned long   niObj;
    unsigned long   niVirt;
    unsigned char   srccompress;// compression algorithm used
    union {
        unsigned char       grFlags;
        struct {
            unsigned char   fVirtual : 1;   // file is a virtual file (injected)
            unsigned char   pad : 7;        // must be zero
        };
    };
    short           sPad;
    union {
        void *      pvReserved1;
        __int64     pv64Reserved2;
    };
};

typedef struct SrcHeaderOut SrcHeaderOut;
typedef SrcHeaderOut *      PSrcHeaderOut;
typedef const SrcHeaderOut *PCSrcHeaderOut;

struct SrcHeaderBlock {
    __int32     ver;
    __int32     cb;
    struct {
        DWORD   dwLowDateTime;
        DWORD   dwHighDateTime;
    } ft;
    __int32     age;
    BYTE        rgbPad[44];
};

typedef struct SrcHeaderBlock   SrcHeaderBlock;

struct SO {
    long off;
    USHORT isect;
    unsigned short pad;
};

#endif // !defined(__midl)

#ifdef __cplusplus

struct IStream;

// C++ Binding

PdbInterface PDB {                 // program database
    enum {
        intv        = PDBIntv,
        intvVC80    = PDBIntv80,
        intvVC70    = PDBIntv70,
        intvVC70Dep = PDBIntv70Dep,     // deprecated
    };

    static PDBAPI(BOOL)
           Open2W(
               _In_z_ const wchar_t *wszPDB,
               _In_z_ const char *szMode,
               OUT EC *pec,
               _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenEx2W(
               _In_z_ const wchar_t *wszPDB,
               _In_z_ const char *szMode,
               long cbPage,
               OUT EC *pec,
               _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenValidate4(
               _In_z_ const wchar_t *wszPDB,
               _In_z_ const char *szMode,
               PCSIG70 pcsig70,
               SIG sig,
               AGE age,
               OUT EC *pec,
               _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenValidate5(
               _In_z_ const wchar_t *wszExecutable,
               _In_z_ const wchar_t *wszSearchPath,
               void *pvClient,
               PfnPDBQueryCallback pfnQueryCallback,
               OUT EC *pec,
               _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);

    static PDBAPI(BOOL) OpenInStream(
               IStream *pIStream,
               _In_z_ const char *szMode,
               OUT EC *pec,
               _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);

    static PDBAPI(BOOL)
           OpenNgenPdb(
               _In_z_ const wchar_t *wszNgenImage,
               _In_z_ const wchar_t *wszPdbPath,
               OUT EC *pec,
               _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
               size_t cchErrMax,
               OUT PDB **pppdb);

    static PDBAPI(BOOL) ExportValidateInterface(INTV intv);
    static PDBAPI(BOOL) ExportValidateImplementation(IMPV impv);

    static PDBAPI(IMPV) QueryImplementationVersionStatic();
    static PDBAPI(INTV) QueryInterfaceVersionStatic();

    static PDBAPI(BOOL) SetErrorHandlerAPI(PfnPDBErrorCreate pfn);
    static PDBAPI(BOOL) SetPDBCloseTimeout(DWORDLONG t);
    static PDBAPI(BOOL) ShutDownTimeoutManager( );
    static PDBAPI(BOOL) CloseAllTimeoutPDB();

    static PDBAPI(BOOL) RPC();

    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;
    virtual EC   QueryLastError(_Out_opt_cap_(cbErrMax) OUT char szError[cbErrMax]) pure;
    virtual char*QueryPDBName(_Out_opt_z_cap_(PDB_MAX_PATH) OUT char szPDB[PDB_MAX_PATH]) pure;
    virtual SIG  QuerySignature() pure;
    virtual AGE  QueryAge() pure;
    virtual BOOL CreateDBI(_In_z_ const char* szTarget, OUT DBI** ppdbi) pure;
    virtual BOOL OpenDBI(_In_z_ const char* szTarget, _In_z_ const char* szMode, OUT DBI** ppdbi ) pure;
    virtual BOOL OpenTpi(_In_z_ const char* szMode, OUT TPI** pptpi) pure;
    virtual BOOL OpenIpi(_In_z_ const char* szMode, OUT TPI** pptpi) pure;

    virtual BOOL Commit() pure;
    virtual BOOL Close() pure;
    virtual BOOL OpenStream(_In_z_ const char* szStream, OUT Stream** ppstream) pure;
    virtual BOOL GetEnumStreamNameMap(OUT Enum** ppenum) pure;
    virtual BOOL GetRawBytes(PFNfReadPDBRawBytes pfnfSnarfRawBytes) pure;
    virtual IMPV QueryPdbImplementationVersion() pure;

    virtual BOOL OpenDBIEx(_In_z_ const char* szTarget, _In_z_ const char* szMode, OUT DBI** ppdbi, PfnFindDebugInfoFile pfn=0) pure;

    virtual BOOL CopyTo( _Pre_notnull_ _Post_z_ const char *szDst, DWORD dwCopyFilter, DWORD dwReserved) pure;

    //
    // support for source file data
    //
    virtual BOOL OpenSrc(OUT Src** ppsrc) pure;

    virtual EC   QueryLastErrorExW(_Out_opt_cap_(cchMax) OUT wchar_t *wszError, size_t cchMax) pure;
    virtual wchar_t *QueryPDBNameExW(_Out_opt_cap_(cchMax) OUT wchar_t *wszPDB, size_t cchMax) pure;
    virtual BOOL QuerySignature2(PSIG70 psig70) pure;
    virtual BOOL CopyToW( _Pre_notnull_ _Post_z_ const wchar_t *szDst, DWORD dwCopyFilter, DWORD dwReserved) pure;
    virtual BOOL fIsSZPDB() const pure;

    // Implemented only on 7.0 and above versions.
    //
    virtual BOOL OpenStreamW(_In_z_ const wchar_t * szStream, OUT Stream** ppstream) pure;

    // Implemented in both 6.0 and 7.0 builds

    virtual BOOL CopyToW2(
        _In_z_ const wchar_t *  szDst,
        DWORD                   dwCopyFilter,
        PfnPDBCopyQueryCallback pfnCallBack,
        void *                  pvClientContext
        ) pure;


    inline BOOL ValidateInterface()
    {
        return ExportValidateInterface(intv);
    }

    virtual BOOL OpenStreamEx(_In_z_ const char * szStream, _In_z_ const char *szMode, Stream **ppStream) pure;

    // Support for PDB mapping
    virtual BOOL RegisterPDBMapping(_In_z_ const wchar_t *wszPDBFrom, _In_z_ const wchar_t *wszPDBTo) pure;

    virtual BOOL EnablePrefetching() pure;

    virtual BOOL FLazy() pure;
    virtual BOOL FMinimal() pure;

    virtual BOOL ResetGUID(BYTE *pb, DWORD cb) pure;
};


// Review: a stream directory service would be more appropriate
// than Stream::Delete, ...

PdbInterface Stream {
    virtual long QueryCb() pure;
    virtual BOOL Read(long off, void* pvBuf, long* pcbBuf) pure;
    virtual BOOL Write(long off, void* pvBuf, long cbBuf) pure;
    virtual BOOL Replace(void* pvBuf, long cbBuf) pure;
    virtual BOOL Append(void* pvBuf, long cbBuf) pure;
    virtual BOOL Delete() pure;
    virtual BOOL Release() pure;
    virtual BOOL Read2(long off, void* pvBuf, long cbBuf) pure;
    virtual BOOL Truncate(long cb) pure;
};

PdbInterface StreamImage {
    static PDBAPI(BOOL) open(Stream* pstream, long cb, OUT StreamImage** ppsi);
    virtual long size() pure;
    virtual void* base() pure;
    virtual BOOL noteRead(long off, long cb, OUT void** ppv) pure;
    virtual BOOL noteWrite(long off, long cb, OUT void** ppv) pure;
    virtual BOOL writeBack() pure;
    virtual BOOL release() pure;
};

PdbInterface DBI {             // debug information
    enum { intv = PDBIntv };
    virtual IMPV QueryImplementationVersion() pure;
    virtual INTV QueryInterfaceVersion() pure;
    virtual BOOL OpenMod(_In_z_ const char* szModule, _In_z_ const char* szFile, OUT Mod** ppmod) pure;
    virtual BOOL DeleteMod(_In_z_ const char* szModule) pure;
    virtual BOOL QueryNextMod(Mod* pmod, Mod** ppmodNext) pure;
    virtual BOOL OpenGlobals(OUT GSI **ppgsi) pure;
    virtual BOOL OpenPublics(OUT GSI **ppgsi) pure;
    virtual BOOL AddSec(USHORT isect, USHORT flags, long off, long cb) pure;
    //__declspec(deprecated)
    virtual BOOL QueryModFromAddr(USHORT isect, long off, OUT Mod** ppmod,
                    OUT USHORT* pisect, OUT long* poff, OUT long* pcb) pure;
    virtual BOOL QuerySecMap(OUT BYTE* pb, long* pcb) pure;
    virtual BOOL QueryFileInfo(OUT BYTE* pb, long* pcb) pure;
    virtual void DumpMods() pure;
    virtual void DumpSecContribs() pure;
    virtual void DumpSecMap() pure;

    virtual BOOL Close() pure;
    virtual BOOL AddThunkMap(long* poffThunkMap, unsigned nThunks, long cbSizeOfThunk,
                    struct SO* psoSectMap, unsigned nSects,
                    USHORT isectThunkTable, long offThunkTable) pure;
    virtual BOOL AddPublic(_In_z_ const char* szPublic, USHORT isect, long off) pure;
    virtual BOOL getEnumContrib(OUT Enum** ppenum) pure;
    virtual BOOL QueryTypeServer( ITSM itsm, OUT TPI** pptpi ) pure;
    virtual BOOL QueryItsmForTi( TI ti, OUT ITSM* pitsm ) pure;
    virtual BOOL QueryNextItsm( ITSM itsm, OUT ITSM *inext ) pure;
    virtual BOOL QueryLazyTypes() pure;
    virtual BOOL SetLazyTypes( BOOL fLazy ) pure;   // lazy is default and can only be turned off
    virtual BOOL FindTypeServers( OUT EC* pec, _Out_opt_cap_(cbErrMax) OUT char szError[cbErrMax] ) pure;
    virtual void DumpTypeServers() pure;
    virtual BOOL OpenDbg(DBGTYPE dbgtype, OUT Dbg **ppdbg) pure;
    virtual BOOL QueryDbgTypes(OUT DBGTYPE *pdbgtype, OUT long* pcDbgtype) pure;
    // apis to support EnC work
    virtual BOOL QueryAddrForSec(OUT USHORT* pisect, OUT long* poff, 
            USHORT imod, long cb, DWORD dwDataCrc, DWORD dwRelocCrc) pure;
    virtual BOOL QueryAddrForSecEx(OUT USHORT* pisect, OUT long* poff, USHORT imod,
            long cb, DWORD dwDataCrc, DWORD dwRelocCrc, DWORD dwCharacteristics) pure;
    virtual BOOL QuerySupportsEC() pure;
    virtual BOOL QueryPdb( OUT PDB** pppdb ) pure;
    virtual BOOL AddLinkInfo(IN PLinkInfo ) pure;
    virtual BOOL QueryLinkInfo(PLinkInfo, OUT long * pcb) pure;
    // new to vc6
    virtual AGE  QueryAge() const pure;
    virtual void * QueryHeader() const pure;
    virtual void FlushTypeServers() pure;
    virtual BOOL QueryTypeServerByPdb(_In_z_ const char* szPdb, OUT ITSM* pitsm) pure;

    // Long filename support
    virtual BOOL OpenModW(_In_z_ const wchar_t* szModule, _In_z_ const wchar_t* szFile, OUT Mod** ppmod) pure;
    virtual BOOL DeleteModW(_In_z_ const wchar_t* szModule) pure;
    virtual BOOL AddPublicW(_In_z_ const wchar_t* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf =0) pure;
    virtual BOOL QueryTypeServerByPdbW(_In_z_ const wchar_t* szPdb, OUT ITSM* pitsm ) pure;
    virtual BOOL AddLinkInfoW(IN PLinkInfoW ) pure;
    virtual BOOL AddPublic2(_In_z_ const char* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf =0) pure;
    virtual USHORT QueryMachineType() const pure;
    virtual void SetMachineType(USHORT wMachine) pure;
    virtual void RemoveDataForRva( ULONG rva, ULONG cb ) pure;
    virtual BOOL FStripped() pure;
    virtual BOOL QueryModFromAddr2(USHORT isect, long off, OUT Mod** ppmod,
                    OUT USHORT* pisect, OUT long* poff, OUT long* pcb,
                    OUT ULONG * pdwCharacteristics) pure;

    // Replacement for QueryNextMod() and QueryModFromAddr()
    virtual BOOL QueryNoOfMods(long *cMods) pure;
    virtual BOOL QueryMods(Mod **ppmodNext, long cMods) pure;
    virtual BOOL QueryImodFromAddr(USHORT isect, long off, OUT USHORT* pimod,
                    OUT USHORT* pisect, OUT long* poff, OUT long* pcb, 
                    OUT ULONG * pdwCharacteristics) pure;
    virtual BOOL OpenModFromImod( USHORT imod, OUT Mod **ppmod ) pure;

    virtual BOOL QueryHeader2(long cb, OUT BYTE * pb, OUT long * pcbOut) pure;

    virtual BOOL FAddSourceMappingItem(
        _In_z_ const wchar_t * szMapTo,
        _In_z_ const wchar_t * szMapFrom,
        ULONG           grFlags     // must be zero; no flags defn'ed as yet
        ) pure;

    typedef ::PFNNOTEPDBUSED  PFNNOTEPDBUSED;
    virtual BOOL FSetPfnNotePdbUsed(void * pvContext, PFNNOTEPDBUSED) pure;

    virtual BOOL FCTypes() pure;
    virtual BOOL QueryFileInfo2(BYTE *pb, long *pcb) pure;
    virtual BOOL FSetPfnQueryCallback(void *pvContext, PFNDBIQUERYCALLBACK) pure;

    typedef ::PFNNOTETYPEMISMATCH  PFNNOTETYPEMISMATCH;
    virtual BOOL FSetPfnNoteTypeMismatch(void * pvContext, PFNNOTETYPEMISMATCH) pure;

    typedef ::PFNTMDTYPEFILTER PFNTMDTYPEFILTER;
    virtual BOOL FSetPfnTmdTypeFilter(void *pvContext, PFNTMDTYPEFILTER) pure;

    virtual BOOL RemovePublic(_In_z_ const char* szPublic) pure;

    virtual BOOL getEnumContrib2(OUT Enum** ppenum) pure;

    virtual BOOL QueryModFromAddrEx(USHORT isect, ULONG off, OUT Mod** ppmod,
                    OUT USHORT* pisect, OUT ULONG *pisectCoff, OUT ULONG* poff, OUT ULONG* pcb,
                    OUT ULONG * pdwCharacteristics) pure;

    virtual BOOL QueryImodFromAddrEx(USHORT isect, ULONG off, OUT USHORT* pimod,
                    OUT USHORT* pisect, OUT ULONG *pisectCoff, OUT ULONG* poff, OUT ULONG* pcb, 
                    OUT ULONG * pdwCharacteristics) pure;
};

PdbInterface Mod {             // info for one module within DBI
    enum { intv = PDBIntv };
    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;
    virtual BOOL AddTypes(BYTE* pbTypes, long cb) pure;
    virtual BOOL AddSymbols(BYTE* pbSym, long cb) pure;
    virtual BOOL AddPublic(_In_z_ const char* szPublic, USHORT isect, long off) pure;
    virtual BOOL AddLines(_In_z_ const char* szSrc, USHORT isect, long offCon, long cbCon, long doff,
                          USHORT lineStart, BYTE* pbCoff, long cbCoff) pure;
    virtual BOOL AddSecContrib(USHORT isect, long off, long cb, ULONG dwCharacteristics) pure;
    virtual BOOL QueryCBName(OUT long* pcb) pure;
    virtual BOOL QueryName(_Out_z_cap_(PDB_MAX_PATH) OUT char szName[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QuerySymbols(BYTE* pbSym, long* pcb) pure;
    virtual BOOL QueryLines(BYTE* pbLines, long* pcb) pure;

    virtual BOOL SetPvClient(void *pvClient) pure;
    virtual BOOL GetPvClient(OUT void** ppvClient) pure;
    virtual BOOL QueryFirstCodeSecContrib(OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics) pure;
//
// Make all users of this api use the real one, as this is exactly what it was
// supposed to query in the first place
//
#define QuerySecContrib    QueryFirstCodeSecContrib

    virtual BOOL QueryImod(OUT USHORT* pimod) pure;
    virtual BOOL QueryDBI(OUT DBI** ppdbi) pure;
    virtual BOOL Close() pure;
    virtual BOOL QueryCBFile(OUT long* pcb) pure;
    virtual BOOL QueryFile(_Out_z_cap_(PDB_MAX_PATH) OUT char szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QueryTpi(OUT TPI** pptpi) pure; // return this Mod's Tpi
    // apis to support EnC work
    virtual BOOL AddSecContribEx(USHORT isect, long off, long cb, ULONG dwCharacteristics, DWORD dwDataCrc, DWORD dwRelocCrc) pure;
    virtual BOOL QueryItsm(OUT USHORT* pitsm) pure;
    virtual BOOL QuerySrcFile(_Out_z_cap_(PDB_MAX_PATH) OUT char szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QuerySupportsEC() pure;
    virtual BOOL QueryPdbFile(_Out_z_cap_(PDB_MAX_PATH) OUT char szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL ReplaceLines(BYTE* pbLines, long cb) pure;

    // V7 line number support
    virtual bool GetEnumLines( EnumLines** ppenum ) pure;
    virtual bool QueryLineFlags( OUT DWORD* pdwFlags ) pure;    // what data is present?
    virtual bool QueryFileNameInfo( 
                    IN DWORD        fileId,                 // source file identifier
                    _Out_opt_capcount_(*pccFilename) OUT wchar_t*    szFilename,     // file name string 
                    IN OUT DWORD*   pccFilename,            // length of string
                    OUT DWORD*      pChksumType,            // type of chksum
                    OUT BYTE*       pbChksum,               // pointer to buffer for chksum data
                    IN OUT DWORD*   pcbChksum               // number of bytes of chksum (in/out)
                    ) pure;         
    // Long filenames support
    virtual BOOL AddPublicW(_In_z_ const wchar_t* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf =0) pure;
    virtual BOOL AddLinesW(_In_z_ const wchar_t* szSrc, USHORT isect, long offCon, long cbCon, long doff,
                          ULONG lineStart, BYTE* pbCoff, long cbCoff) pure;
    virtual BOOL QueryNameW(_Out_z_cap_(PDB_MAX_PATH) OUT wchar_t szName[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QueryFileW(_Out_z_cap_(PDB_MAX_PATH) OUT wchar_t szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QuerySrcFileW(_Out_z_cap_(PDB_MAX_PATH) OUT wchar_t szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL QueryPdbFileW(_Out_z_cap_(PDB_MAX_PATH) OUT wchar_t szFile[PDB_MAX_PATH], OUT long* pcb) pure;
    virtual BOOL AddPublic2(_In_z_ const char* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf =0) pure;
    virtual BOOL InsertLines(BYTE* pbLines, long cb) pure;
    virtual BOOL QueryLines2(IN long cbLines, OUT BYTE *pbLines, OUT long *pcbLines) pure;
    virtual BOOL QueryCrossScopeExports(DWORD cb, BYTE* pb, DWORD* pcb) pure;
    virtual BOOL QueryCrossScopeImports(DWORD cb, BYTE* pb, DWORD* pcb) pure;
    virtual BOOL QueryInlineeLines(DWORD cb, BYTE* pb, DWORD* pcb) pure;
    virtual BOOL TranslateFileId(DWORD id, DWORD* pid) pure;
    virtual BOOL QueryFuncMDTokenMap(DWORD cb, BYTE* pb, DWORD* pcb) pure;
    virtual BOOL QueryTypeMDTokenMap(DWORD cb, BYTE* pb, DWORD* pcb) pure;
    virtual BOOL QueryMergedAssemblyInput(DWORD cb, BYTE* pb, DWORD* pcb) pure;
    virtual BOOL QueryILLines(DWORD cb, BYTE* pb, DWORD* pcb) pure;
    virtual bool GetEnumILLines(EnumLines** ppenum) pure;
    virtual bool QueryILLineFlags(OUT DWORD* pdwFlags) pure;
    virtual BOOL MergeTypes(BYTE *pb, DWORD cb) pure;
    virtual BOOL IsTypeServed(DWORD index, BOOL fID) pure;
    virtual BOOL QueryTypes(BYTE* pb, DWORD* pcb) pure;
    virtual BOOL QueryIDs(BYTE* pb, DWORD* pcb) pure;
    virtual BOOL QueryCVRecordForTi(DWORD index, BOOL fID, OUT BYTE* pb, IN OUT DWORD* pcb) pure;
    virtual BOOL QueryPbCVRecordForTi(DWORD index, BOOL fID, OUT BYTE** ppb) pure;
    virtual BOOL QueryTiForUDT(_In_z_ const char *sz, BOOL fCase, OUT TI *pti) pure;
    virtual BOOL QueryCoffSymRVAs(BYTE *pb, DWORD *pcb) pure;
    virtual BOOL AddSecContrib2(USHORT isect, DWORD off, DWORD isectCoff, DWORD cb, DWORD dwCharacteristics) pure;
    virtual BOOL AddSecContrib2Ex(USHORT isect, DWORD off, DWORD isecfCoff, DWORD cb, DWORD dwCharacteristics, DWORD dwDataCrc, DWORD dwRelocCrc) pure;
    virtual BOOL AddSymbols2(BYTE* pbSym, DWORD cb, DWORD isectCoff) pure;
    virtual BOOL RemoveGlobalRefs() pure;
    virtual BOOL QuerySrcLineForUDT(TI ti, _Deref_out_z_ char **pszSrc, DWORD *pLine) pure;
};

PdbInterface TPI {             // type info

    enum { intv = PDBIntv };

    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;

    virtual BOOL QueryTi16ForCVRecord(BYTE* pb, OUT TI16* pti) pure;
    virtual BOOL QueryCVRecordForTi16(TI16 ti, OUT BYTE* pb, IN OUT long* pcb) pure;
    virtual BOOL QueryPbCVRecordForTi16(TI16 ti, OUT BYTE** ppb) pure;
    virtual TI16 QueryTi16Min() pure;
    virtual TI16 QueryTi16Mac() pure;

    virtual long QueryCb() pure;
    virtual BOOL Close() pure;
    virtual BOOL Commit() pure;

    virtual BOOL QueryTi16ForUDT(_In_z_ const char *sz, BOOL fCase, OUT TI16* pti) pure;
    virtual BOOL SupportQueryTiForUDT() pure;

    // the new versions that truly take 32-bit types
    virtual BOOL fIs16bitTypePool() pure;
    virtual BOOL QueryTiForUDT(_In_z_ const char *sz, BOOL fCase, OUT TI* pti) pure;
    virtual BOOL QueryTiForCVRecord(BYTE* pb, OUT TI* pti) pure;
    virtual BOOL QueryCVRecordForTi(TI ti, OUT BYTE* pb, IN OUT long* pcb) pure;
    virtual BOOL QueryPbCVRecordForTi(TI ti, OUT BYTE** ppb) pure;
    virtual TI   QueryTiMin() pure;
    virtual TI   QueryTiMac() pure;
    virtual BOOL AreTypesEqual( TI ti1, TI ti2 ) pure;
    virtual BOOL IsTypeServed( TI ti ) pure;
    virtual BOOL QueryTiForUDTW(_In_z_ const wchar_t *wcs, BOOL fCase, OUT TI* pti) pure;
    virtual BOOL QueryModSrcLineForUDTDefn(const TI tiUdt, USHORT *pimod, OUT NI* psrcId, OUT DWORD* pline) pure;
};

PdbInterface GSI {
    enum { intv = PDBIntv };
    virtual INTV QueryInterfaceVersion() pure;
    virtual IMPV QueryImplementationVersion() pure;
    virtual BYTE* NextSym(BYTE* pbSym) pure;
    virtual BYTE* HashSym(_In_z_ const char* szName, BYTE* pbSym) pure;
    virtual BYTE* NearestSym(USHORT isect, long off, OUT long* pdisp) pure;      //currently only supported for publics
    virtual BOOL Close() pure;
    virtual BOOL getEnumThunk(USHORT isect, long off, OUT EnumThunk** ppenum) pure;
    virtual unsigned long OffForSym(BYTE *pbSym) pure;
    virtual BYTE* SymForOff(unsigned long off) pure;
    virtual BYTE* HashSymW(_In_z_ const wchar_t *wcsName, BYTE* pbSym) pure;
    virtual BOOL getEnumByAddr(EnumSyms **ppEnum) pure;
};


PdbInterface NameMap {
    static PDBAPI(BOOL) open(PDB* ppdb, BOOL fWrite, OUT NameMap** ppnm);
    virtual BOOL close() pure;
    virtual BOOL reinitialize() pure;
    virtual BOOL getNi(_In_z_ const char* sz, OUT NI* pni) pure;
    virtual BOOL getName(NI ni, _Deref_out_z_ OUT const char** psz) pure;
    virtual BOOL getEnumNameMap(OUT Enum** ppenum) pure;
    virtual BOOL contains(_In_z_ const char* sz, OUT NI* pni) pure;
    virtual BOOL commit() pure;
    virtual BOOL isValidNi(NI ni) pure;
    virtual BOOL getNiW(_In_z_ const wchar_t* sz, OUT NI* pni) pure;
    virtual BOOL getNameW(NI ni, _Out_opt_capcount_(*pcch) OUT wchar_t* szName, IN OUT size_t * pcch) pure;
    virtual BOOL containsW(_In_z_ const wchar_t *sz, OUT NI* pni) pure;
    virtual BOOL containsUTF8(_In_z_ const char* sz, OUT NI* pni) pure;
    virtual BOOL getNiUTF8(_In_z_ const char *sz, OUT NI* pni) pure;
    virtual BOOL getNameA(NI ni, _Pre_notnull_ _Post_z_ OUT const char ** psz) pure;
    virtual BOOL getNameW2(NI ni, _Pre_notnull_ _Post_z_ OUT const wchar_t ** pwsz) pure;
};

#define __ENUM_INCLUDED__
PdbInterface Enum {
    virtual void release() pure;
    virtual void reset() pure;
    virtual BOOL next() pure;
};

PdbInterface EnumNameMap : Enum {
    virtual void get(_Pre_notnull_ _Post_z_ OUT const char** psz, OUT NI* pni) pure;
};

PdbInterface EnumContrib : Enum {
    virtual void get(OUT USHORT* pimod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics) pure;
    virtual void getCrcs(OUT DWORD* pcrcData, OUT DWORD* pcrcReloc ) pure;
    virtual bool fUpdate(IN long off, IN long cb) pure;
    virtual BOOL prev() pure;
    virtual BOOL clone( OUT EnumContrib **ppEnum ) pure;
    virtual BOOL locate( IN long isect, IN long off ) pure;
    virtual void get2(OUT USHORT* pimod, OUT USHORT* pisect, OUT DWORD* poff, OUT DWORD* pisectCoff, OUT DWORD* pcb, OUT ULONG* pdwCharacteristics) pure;
};

PdbInterface EnumThunk: Enum {
    virtual void get( OUT USHORT* pisect, OUT long* poff, OUT long* pcb ) pure;
};

PdbInterface EnumSyms : Enum {
    virtual void get( BYTE** ppbSym ) pure;
    virtual BOOL prev() pure;
    virtual BOOL clone( OUT EnumSyms **ppEnum ) pure;
    virtual BOOL locate( IN long isect, IN long off ) pure;
};
struct CV_Line_t;
struct CV_Column_t;
PdbInterface EnumLines: public Enum
{
    // 
    // Blocks of lines are always in offset order, lines within blocks are also ordered by offset
    //
    virtual bool getLines(  
        OUT DWORD*      fileId,     // id for the filename
        OUT DWORD*      poffset,    // offset part of address
        OUT WORD*       pseg,       // segment part of address
        OUT DWORD*      pcb,        // count of bytes of code described by this block
        IN OUT DWORD*   pcLines,    // number of lines (in/out)
        OUT CV_Line_t*  pLines      // pointer to buffer for line info
        ) = 0;
    virtual bool getLinesColumns(   
        OUT DWORD*      fileId,     // id for the filename      
        OUT DWORD*      poffset,    // offset part of address
        OUT WORD*       pseg,       // segment part of address
        OUT DWORD*      pcb,        // count of bytes of code described by this block
        IN OUT DWORD*   pcLines,    // number of lines (in/out)
        OUT CV_Line_t*  pLines,     // pointer to buffer for line info
        OUT CV_Column_t*pColumns    // pointer to buffer for column info
        ) = 0;
    virtual bool clone( 
        OUT EnumLines **ppEnum      // return pointer to the clone
        ) = 0;
};

//
// interface to use to widen type indices from 16 to 32 bits
// and store the results in a new location.
//
PdbInterface WidenTi {
public:
    static PDBAPI(BOOL)
    fCreate (
        WidenTi *&,
        unsigned cTypeInitialCache =256,
        BOOL fNB10Syms =wtiSymsNB09
        );

    virtual void
    release() pure;

    virtual BYTE /* TYPTYPE */ *
    pTypeWidenTi ( TI ti16, BYTE /* TYPTYPE */ * ) pure;

    virtual BYTE /* SYMTYPE */ *
    pSymWidenTi ( BYTE /* SYMTYPE */ * ) pure;

    virtual BOOL
    fTypeWidenTiNoCache ( BYTE * pbTypeDst, BYTE * pbTypeSrc, long & cbDst ) pure;

    virtual BOOL
    fSymWidenTiNoCache ( BYTE * pbSymDst, BYTE * pbSymSrc, long & cbDst ) pure;

    virtual BOOL
    fTypeNeedsWidening ( BYTE * pbType ) pure;

    virtual BOOL
    fSymNeedsWidening ( BYTE * pbSym ) pure;

    virtual BOOL
    freeRecord ( void * ) pure;

    // symbol block converters/query.  symbols start at doff from pbSymIn,
    // converted symbols will go at sci.pbSyms + doff, cbSyms are all including
    // doff.
    virtual BOOL
        fQuerySymConvertInfo (
        SymConvertInfo &    sciOut,
        BYTE *              pbSym,
        long                cbSym,
        int                 doff =0
        ) pure;

    virtual BOOL
    fConvertSymbolBlock (
        SymConvertInfo &    sciOut,
        BYTE *              pbSymIn,
        long                cbSymIn,
        int                 doff =0
        ) pure;
};

// interface for managing Dbg data
PdbInterface Dbg {
   // close Dbg Interface
   virtual BOOL Close() pure;
   // return number of elements (NOT bytes)
   virtual long QuerySize() pure;
   // reset enumeration index
   virtual void Reset() pure;
   // skip next celt elements (move enumeration index)
   virtual BOOL Skip(ULONG celt) pure;
   // query next celt elements into user-supplied buffer
   virtual BOOL QueryNext(ULONG celt, OUT void *rgelt) pure;
   // search for an element and fill in the entire struct given a field.
   // Only supported for the following debug types and fields:
   // DBG_FPO              'ulOffStart' field of FPO_DATA
   // DBG_FUNC             'StartingAddress' field of IMAGE_FUNCTION_ENTRY
   // DBG_OMAP             'rva' field of OMAP
   virtual BOOL Find(IN OUT void *pelt) pure;
   // remove debug data
   virtual BOOL Clear() pure;
   // append celt elements
   virtual BOOL Append(ULONG celt, const void *rgelt) pure;
   // replace next celt elements
   virtual BOOL ReplaceNext(ULONG celt, const void *rgelt) pure;
   // create a clone of this interface
   virtual BOOL Clone( Dbg** ppDbg ) pure;

   // return size of one element
   virtual long QueryElementSize() pure;
};

PdbInterface Src {
    // close and commit the changes (when open for write)
    virtual bool
    Close() pure;

    // add a source file or file-ette
    virtual bool
    Add(IN PCSrcHeader psrcheader, IN const void * pvData) pure;

    // remove a file or file-ette or all of the injected code for
    // one particular compiland (using the object file name)
    virtual bool
    Remove(IN SZ_CONST szFile) pure;

    // query and copy the header/control data to the output buffer
    virtual bool
    QueryByName(IN SZ_CONST szFile, OUT PSrcHeaderOut psrcheaderOut) const pure;

    // copy the file data (the size of the buffer is in the SrcHeaderOut
    // structure) to the output buffer.
    virtual bool
    GetData(IN PCSrcHeaderOut pcsrcheader, OUT void * pvData) const pure;

    // create an enumerator to traverse all of the files included
    // in the mapping.
    virtual bool
    GetEnum(OUT EnumSrc ** ppenum) const pure;

    // Get the header block (master header) of the Src data.
    // Includes age, time stamp, version, and size of the master stream
    virtual bool GetHeaderBlock(SrcHeaderBlock & shb) const pure;
    virtual bool RemoveW(_In_z_ IN wchar_t *wcsFile) pure;
    virtual bool QueryByNameW(_In_z_ IN wchar_t *wcsFile, OUT PSrcHeaderOut psrcheaderOut) const pure;
    virtual bool AddW(IN PCSrcHeaderW psrcheader, IN const void * pvData) pure;
};

PdbInterface EnumSrc : Enum {
    virtual void get(OUT PCSrcHeaderOut * ppcsrcheader) pure;
};


PdbInterface SrcHash {

    // Various types we need
    //
    
    // Tri-state return type
    //
    enum TriState {
        tsYes,
        tsNo,
        tsMaybe,
    };

    // Hash identifier
    //
    enum HID {
        hidNone,
        hidMD5,
        hidSHA1,
        hidSHA256,
        hidMax,
    };

    // Define machine independent types for storage of HashID and size_t
    //
    typedef __int32 HashID_t;
    typedef unsigned __int32 CbHash_t;

    // Create a SrcHash object with the usual two-stage construction technique
    //
    static PDBAPI(bool)
    FCreateSrcHash(OUT PSrcHash &, IN HID hid);

    // Accumulate more bytes into the hash
    //
    virtual bool
    FHashBuffer(IN PCV pvBuf, IN size_t cbBuf) pure;

    // Query the hash id
    //
    virtual HashID_t
    HashID() const pure;

    // Query the size of the hash 
    //
    virtual CbHash_t
    CbHash() const pure;

    // Copy the hash bytes to the client buffer
    //
    virtual bool
    FGetHash(OUT PV pvHash, IN CbHash_t cbHash) const pure;

    // Verify the incoming hash against a target buffer of bytes
    // returning a yes it matches, no it doesn't, or indeterminate.
    //
    virtual TriState
    TsVerifyHash(
        IN HID,
        IN CbHash_t cbHash,
        IN PCV pvHash,
        IN size_t cbBuf,
        IN PCV pvBuf
        ) pure;

    // Reset this object to pristine condition
    //
    virtual bool
    FReset() pure;

    // Close off and release this object
    //
    virtual void
    Close() pure;
};

#if defined(__midl)
typedef PdbInterface PDB PDB;
typedef PdbInterface DBI DBI;
typedef PdbInterface Mod Mod;
typedef PdbInterface TPI TPI;
typedef PdbInterface GSI GSI;
typedef PdbInterface SO SO;
typedef PdbInterface Stream Stream;
typedef PdbInterface StreamImage StreamImage;
typedef PdbInterface NameMap NameMap;
typedef PdbInterface Enum Enum;
typedef PdbInterface EnumStreamNames EnumStreamNames;
typedef PdbInterface EnumNameMap EnumNameMap;
typedef PdbInterface EnumContrib EnumContrib;
typedef PdbInterface EnumSyms EnumSyms;
typedef PdbInterface WidenTi WidenTi;
typedef PdbInterface Dbg Dbg;
typedef PdbInterface EnumThunk EnumThunk;
typedef PdbInterface Src Src;
typedef PdbInterface EnumSrc EnumSrc;
typedef PdbInterface SrcHash SrcHash;
#endif  // __midl

#endif  // __cplusplus

// ANSI C Binding

#if __cplusplus
extern "C" {
#endif

#if !defined(__midl)

PDBAPI(BOOL)
PDBOpen2W(
    _In_z_ const wchar_t *wszPDB,
    _In_z_ const char *szMode,
    OUT EC *pec,
    _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
    size_t cchErrMax,
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenEx2W(
    _In_z_ const wchar_t *wszPDB,
    _In_z_ const char *szMode,
    long cbPage,
    OUT EC *pec,
    _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
    size_t cchErrMax,
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenValidate4(
    _In_z_ const wchar_t *wszPDB,
    _In_z_ const char *szMode,
    PCSIG70 pcsig70,
    SIG sig,
    AGE age,
    OUT EC *pec,
    _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
    size_t cchErrMax,
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenValidate5(
    _In_z_ const wchar_t *wszExecutable,
    _In_z_ const wchar_t *wszSearchPath,
    void *pvClient,
    PfnPDBQueryCallback pfnQueryCallback,
    OUT EC *pec,
    _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
    size_t cchErrMax,
    OUT PDB **pppdb);

PDBAPI(BOOL)
PDBOpenNgenPdb(
    _In_z_ const wchar_t *wszNgenImage,
    _In_z_ const wchar_t *wszPdbPath,
    OUT EC *pec,
    _Out_opt_cap_(cchErrMax) OUT wchar_t *wszError,
    size_t cchErrMax,
    OUT PDB **pppdb);

// a dbi client should never call PDBExportValidateInterface directly - use PDBValidateInterface
PDBAPI(BOOL)
PDBExportValidateInterface(
    INTV intv);

__inline BOOL PDBValidateInterface(void)
{
    return PDBExportValidateInterface(PDBIntv);
}

typedef BOOL (PDBCALL *PfnPDBExportValidateInterface)(INTV);

__inline BOOL PDBValidateInterfacePfn(PfnPDBExportValidateInterface pfn)
{
    return (*pfn)(PDBIntv);
}

PDBAPI(BOOL)   PDBRPC();
PDBAPI(EC)     PDBQueryLastError(PDB *ppdb, _Out_opt_cap_(cbErrMax) OUT char szError[cbErrMax]);
PDBAPI(EC)     PDBQueryLastErrorExW(PDB *ppdb, _Out_opt_cap_(cchMax) OUT wchar_t *wszError, size_t cchMax);
PDBAPI(INTV)   PDBQueryInterfaceVersion(PDB* ppdb);
PDBAPI(IMPV)   PDBQueryImplementationVersion(PDB* ppdb);
PDBAPI(char*)  PDBQueryPDBName(PDB* ppdb, _Out_z_cap_(PDB_MAX_PATH) OUT char szPDB[PDB_MAX_PATH]);
PDBAPI(SIG)    PDBQuerySignature(PDB* ppdb);
PDBAPI(BOOL)   PDBQuerySignature2(PDB* ppdb, PSIG70 psig70);
PDBAPI(AGE)    PDBQueryAge(PDB* ppdb);
PDBAPI(BOOL)   PDBCreateDBI(PDB* ppdb, _In_z_ const char* szTarget, OUT DBI** ppdbi);
PDBAPI(BOOL)   PDBOpenDBIEx(PDB* ppdb, const char* szMode, _In_z_ const char* szTarget, OUT DBI** ppdbi, PfnFindDebugInfoFile pfn);
PDBAPI(BOOL)   PDBOpenDBI(PDB* ppdb, _In_z_ const char* szMode, _In_z_ const char* szTarget, OUT DBI** ppdbi);
PDBAPI(BOOL)   PDBOpenTpi(PDB* ppdb, _In_z_ const char* szMode, OUT TPI** pptpi);
PDBAPI(BOOL)   PDBOpenIpi(PDB* ppdb, _In_z_ const char* szMode, OUT TPI** pptpi);
PDBAPI(BOOL)   PDBCommit(PDB* ppdb);
PDBAPI(BOOL)   PDBClose(PDB* ppdb);
PDBAPI(BOOL)   PDBOpenStream(PDB* ppdb, _In_z_ const char* szStream, OUT Stream** ppstream);
PDBAPI(BOOL)   PDBOpenStreamEx(PDB* ppdb, _In_z_ const char* szStream, _In_z_ const char* szMode, OUT Stream** ppstream);
PDBAPI(BOOL)   PDBCopyTo(PDB *ppdb, _In_z_ const char *szTargetPdb, DWORD dwCopyFilter, DWORD dwReserved);
PDBAPI(BOOL)   PDBCopyToW(PDB *ppdb, _In_z_ const wchar_t *szTargetPdb, DWORD dwCopyFilter, DWORD dwReserved);
PDBAPI(BOOL)   PDBfIsSZPDB(PDB *ppdb);
PDBAPI(BOOL)   PDBCopyToW2(PDB *ppdb, _In_z_ const wchar_t *szTargetPdb, DWORD dwCopyFilter, PfnPDBCopyQueryCallback pfnCallBack, void * pvClientContext);
PDBAPI(BOOL)   PDBRegisterPDBMapping(PDB *ppdb, _In_z_ const wchar_t *wszPDBFrom, _In_z_ const wchar_t *wszPDBTo);
PDBAPI(BOOL)   PDBEnablePrefetching(PDB *ppdb);
PDBAPI(BOOL)   PDBResetGUID(PDB *ppdb, BYTE *pb, DWORD cb);
PDBAPI(wchar_t*)  PDBQueryPDBNameExW(PDB *ppdb, _Out_opt_cap_(cchMax) OUT wchar_t *wszPDB, size_t cchMax);

PDBAPI(INTV)   DBIQueryInterfaceVersion(DBI* pdbi);
PDBAPI(IMPV)   DBIQueryImplementationVersion(DBI* pdbi);
PDBAPI(BOOL)   DBIOpenMod(DBI* pdbi, _In_z_ const char* szModule, _In_z_ const char* szFile, OUT Mod** ppmod);
PDBAPI(BOOL)   DBIOpenModW(DBI* pdbi, _In_z_ const wchar_t* szModule, _In_z_ const wchar_t* szFile, OUT Mod** ppmod);
PDBAPI(BOOL)   DBIDeleteMod(DBI* pdbi, _In_z_ const char* szModule);
PDBAPI(BOOL)   DBIQueryNextMod(DBI* pdbi, Mod* pmod, Mod** ppmodNext);
PDBAPI(BOOL)   DBIOpenGlobals(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI(BOOL)   DBIOpenPublics(DBI* pdbi, OUT GSI **ppgsi);
PDBAPI(BOOL)   DBIAddSec(DBI* pdbi, USHORT isect, USHORT flags, long off, long cb);
PDBAPI(BOOL)   DBIAddPublic(DBI* pdbi, _In_z_ const char* szPublic, USHORT isect, long off);
PDBAPI(BOOL)   DBIRemovePublic(DBI* pdbi, _In_z_ const char* szPublic);
#if  __cplusplus
PDBAPI(BOOL)   DBIAddPublic2(DBI* pdbi, _In_z_ const char* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf=0);
#else
PDBAPI(BOOL)   DBIAddPublic2(DBI* pdbi, _In_z_ const char* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf);
#endif
PDBAPI(BOOL)   DBIQueryModFromAddr(DBI* pdbi, USHORT isect, long off, OUT Mod** ppmod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb);
PDBAPI(BOOL)   DBIQueryModFromAddr2(DBI* pdbi, USHORT isect, long off, OUT Mod** ppmod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG *pdwCharacteristics);
PDBAPI(BOOL)   DBIQueryModFromAddrEx(DBI* pdbi, USHORT isect, ULONG off, OUT Mod** ppmod, OUT USHORT* pisect, OUT ULONG *pisectCoff, OUT ULONG* poff, OUT ULONG* pcb, OUT ULONG * pdwCharacteristics);
PDBAPI(BOOL)   DBIQueryImodFromAddrEx(USHORT isect, ULONG off, OUT USHORT* pimod, OUT USHORT* pisect, OUT ULONG *pisectCoff, OUT ULONG* poff, OUT ULONG* pcb, OUT ULONG * pdwCharacteristics);
PDBAPI(BOOL)   DBIQuerySecMap(DBI* pdbi, OUT BYTE* pb, long* pcb);
PDBAPI(BOOL)   DBIQueryFileInfo(DBI* pdbi, OUT BYTE* pb, long* pcb);
PDBAPI(BOOL)   DBIQuerySupportsEC(DBI* pdbi);
PDBAPI(void)   DBIDumpMods(DBI* pdbi);
PDBAPI(void)   DBIDumpSecContribs(DBI* pdbi);
PDBAPI(void)   DBIDumpSecMap(DBI* pdbi);
PDBAPI(BOOL)   DBIClose(DBI* pdbi);
PDBAPI(BOOL)   DBIAddThunkMap(DBI* pdbi, long* poffThunkMap, unsigned nThunks, long cbSizeOfThunk,
                              struct SO* psoSectMap, unsigned nSects, USHORT isectThunkTable, long offThunkTable);
PDBAPI(BOOL)   DBIGetEnumContrib(DBI* pdbi, OUT Enum** ppenum);
PDBAPI(BOOL)   DBIGetEnumContrib2(DBI* pdbi, OUT Enum** ppenum);
PDBAPI(BOOL)   DBIQueryTypeServer(DBI* pdbi, ITSM itsm, OUT TPI** pptpi );
PDBAPI(BOOL)   DBIQueryItsmForTi(DBI* pdbi, TI ti, OUT ITSM* pitsm );
PDBAPI(BOOL)   DBIQueryNextItsm(DBI* pdbi, ITSM itsm, OUT ITSM *inext );
PDBAPI(BOOL)   DBIQueryLazyTypes(DBI* pdbi);
PDBAPI(BOOL)   DBISetLazyTypes(DBI* pdbi, BOOL fLazy);
PDBAPI(BOOL)   DBIFindTypeServers( DBI* pdbi, OUT EC* pec, _Out_opt_cap_(cbErrMax) OUT char szError[cbErrMax] );
PDBAPI(BOOL)   DBIOpenDbg(DBI* pdbi, DBGTYPE dbgtype, OUT Dbg **ppdbg);
PDBAPI(BOOL)   DBIQueryDbgTypes(DBI* pdbi, OUT DBGTYPE *pdbgtype, OUT long* pcDbgtype);
PDBAPI(BOOL)   DBIAddLinkInfo(DBI* pdbi, IN PLinkInfo);
PDBAPI(BOOL)   DBIAddLinkInfoW(DBI* pdbi, IN PLinkInfoW);
PDBAPI(BOOL)   DBIQueryLinkInfo(DBI* pdbi, PLinkInfo, IN OUT long * pcb);
PDBAPI(BOOL)   DBIFStripped(DBI* pdbi);
PDBAPI(BOOL)   DBIFAddSourceMappingItem(DBI* pdbi, _In_z_ const wchar_t * szMapTo, _In_z_ const wchar_t * szMapFrom, ULONG grFlags);
PDBAPI(void)   DBISetMachineType(DBI* pdbi, USHORT wMachine);
PDBAPI(void)   DBIRemoveDataForRva(DBI* pdbi, ULONG rva, ULONG cb);
PDBAPI(BOOL)   DBIFSetPfnNotePdbUsed(DBI* pdbi, void * pvContext, PFNNOTEPDBUSED);
PDBAPI(BOOL)   DBIFSetPfnNoteTypeMismatch(DBI* pdbi, void * pvContext, PFNNOTETYPEMISMATCH);
PDBAPI(BOOL)   DBIFSetPfnTmdTypeFilter(DBI* pdbi, void *pvContext, PFNTMDTYPEFILTER);

PDBAPI(INTV)   ModQueryInterfaceVersion(Mod* pmod);
PDBAPI(IMPV)   ModQueryImplementationVersion(Mod* pmod);
PDBAPI(BOOL)   ModAddTypes(Mod* pmod, BYTE* pbTypes, long cb);
PDBAPI(BOOL)   ModMergeTypes(Mod* pmod, BYTE *pb, DWORD cb);
PDBAPI(BOOL)   ModAddSymbols(Mod* pmod, BYTE* pbSym, long cb);
PDBAPI(BOOL)   ModAddSymbols2(Mod* pmod, BYTE* pbSym, DWORD cb, DWORD isectCoff);
PDBAPI(BOOL)   ModAddPublic(Mod* pmod, _In_z_ const char* szPublic, USHORT isect, long off);
#if __cplusplus
PDBAPI(BOOL)   ModAddPublic2(Mod* pmod, _In_z_ const char* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf=0);
#else
PDBAPI(BOOL)   ModAddPublic2(Mod* pmod, _In_z_ const char* szPublic, USHORT isect, long off, CV_pubsymflag_t cvpsf);
#endif
PDBAPI(BOOL)   ModAddLines(Mod* pmod, _In_z_ const char* szSrc, USHORT isect, long offCon, long cbCon, long doff,
                           USHORT lineStart, BYTE* pbCoff, long cbCoff);
PDBAPI(BOOL)   ModAddLinesW(Mod* pmod, _In_z_ const wchar_t* szSrc, USHORT isect, long offCon, long cbCon, long doff,
                            ULONG lineStart, BYTE* pbCoff, long cbCoff);
PDBAPI(BOOL)   ModAddSecContrib(Mod * pmod, USHORT isect, long off, long cb, ULONG dwCharacteristics);
PDBAPI(BOOL)   ModAddSecContribEx(Mod * pmod, USHORT isect, long off, long cb, ULONG dwCharacteristics, DWORD dwDataCrc, DWORD dwRelocCrc);
PDBAPI(BOOL)   ModAddSecContrib2(Mod * pmod, USHORT isect, DWORD off, DWORD isectCoff, DWORD cb, DWORD dwCharacteristics);
PDBAPI(BOOL)   ModAddSecContrib2Ex(Mod * pmod, USHORT isect, DWORD off, DWORD isecfCoff, DWORD cb, DWORD dwCharacteristics, DWORD dwDataCrc, DWORD dwRelocCrc);
PDBAPI(BOOL)   ModQueryCBName(Mod* pmod, OUT long* pcb);
PDBAPI(BOOL)   ModQueryName(Mod* pmod, _Out_z_cap_(PDB_MAX_PATH) OUT char szName[PDB_MAX_PATH], OUT long* pcb);
PDBAPI(BOOL)   ModQuerySymbols(Mod* pmod, BYTE* pbSym, long* pcb);
PDBAPI(BOOL)   ModQueryCoffSymRVAs(Mod* pmod, BYTE* pb, DWORD* pcb);
PDBAPI(BOOL)   ModIsTypeServed(Mod* pmod, DWORD index, BOOL fID);
PDBAPI(BOOL)   ModQueryTypes(Mod* pmod, BYTE* pb, DWORD* pcb);
PDBAPI(BOOL)   ModQueryIDs(Mod* pmod, BYTE* pb, DWORD* pcb);
PDBAPI(BOOL)   ModQueryCVRecordForTi(Mod* pmod, DWORD index, BOOL fID, OUT BYTE* pb, IN OUT DWORD* pcb);
PDBAPI(BOOL)   ModQueryPbCVRecordForTi(Mod* pmod, DWORD index, BOOL fID, OUT BYTE **ppb);
PDBAPI(BOOL)   ModQueryTiForUDT(Mod* pmod, _In_z_ const char *sz, BOOL fCase, OUT TI* pti);
PDBAPI(BOOL)   ModQueryLines(Mod* pmod, BYTE* pbLines, long* pcb);
PDBAPI(BOOL)   ModQueryLines2(Mod* pmod, long cb, BYTE* pbLines, long* pcb);
PDBAPI(BOOL)   ModSetPvClient(Mod* pmod, void *pvClient);
PDBAPI(BOOL)   ModGetPvClient(Mod* pmod, OUT void** ppvClient);
PDBAPI(BOOL)   ModQuerySecContrib(Mod* pmod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics);
PDBAPI(BOOL)   ModQueryFirstCodeSecContrib(Mod* pmod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics);
PDBAPI(BOOL)   ModQueryImod(Mod* pmod, OUT USHORT* pimod);
PDBAPI(BOOL)   ModQueryDBI(Mod* pmod, OUT DBI** ppdbi);
PDBAPI(BOOL)   ModClose(Mod* pmod);
PDBAPI(BOOL)   ModQueryCBFile(Mod* pmod, OUT long* pcb);
PDBAPI(BOOL)   ModQueryFile(Mod* pmod, _Out_z_cap_(PDB_MAX_PATH) OUT char szFile[PDB_MAX_PATH], OUT long* pcb);
PDBAPI(BOOL)   ModQuerySrcFile(Mod* pmod, _Out_z_cap_(PDB_MAX_PATH) OUT char szFile[PDB_MAX_PATH], OUT long* pcb);
PDBAPI(BOOL)   ModQueryPdbFile(Mod* pmod, _Out_z_cap_(PDB_MAX_PATH) OUT char szFile[PDB_MAX_PATH], OUT long* pcb);
PDBAPI(BOOL)   ModQuerySupportsEC(Mod* pmod);
PDBAPI(BOOL)   ModQueryTpi(Mod* pmod, OUT TPI** pptpi);
PDBAPI(BOOL)   ModReplaceLines(Mod* pmod, BYTE* pbLines, long cb);
PDBAPI(BOOL)   ModQueryCrossScopeExports(Mod* pmod, DWORD cb, BYTE* pb, DWORD* pcb);
PDBAPI(BOOL)   ModQueryCrossScopeImports(Mod* pmod, DWORD cb, BYTE* pb, DWORD* pcb);    
PDBAPI(BOOL)   ModQueryInlineeLines(Mod* pmod, DWORD cb, BYTE* pb, DWORD* pcb);
PDBAPI(BOOL)   ModTranslateFileId(Mod* pmod, DWORD id, DWORD* pid);
PDBAPI(BOOL)   ModQueryFuncMDTokenMap(Mod* pmod, DWORD cb, BYTE* pb, DWORD* pcb);
PDBAPI(BOOL)   ModQueryTypeMDTokenMap(Mod* pmod, DWORD cb, BYTE* pb, DWORD* pcb);
PDBAPI(BOOL)   ModQueryMergedAssemblyInput(Mod* pmod, DWORD cb, BYTE* pb, DWORD* pcb);
PDBAPI(BOOL)   ModQueryILLines(Mod* pmod, DWORD cb, BYTE* pb, DWORD* pcb);
PDBAPI(BOOL)   ModGetEnumILLines(Mod* pmod, EnumLines** ppenum);
PDBAPI(BOOL)   ModQueryILLineFlags(Mod* pmod, OUT DWORD* pdwFlags);
PDBAPI(BOOL)   ModRemoveGlobalRefs(Mod* pmod);


PDBAPI(INTV)   TypesQueryInterfaceVersion(TPI* ptpi);
PDBAPI(IMPV)   TypesQueryImplementationVersion(TPI* ptpi);
// can't use the same api's for 32-bit TIs.
PDBAPI(BOOL)   TypesQueryTiForCVRecordEx(TPI* ptpi, BYTE* pb, OUT TI* pti);
PDBAPI(BOOL)   TypesQueryCVRecordForTiEx(TPI* ptpi, TI ti, OUT BYTE* pb, IN OUT long* pcb);
PDBAPI(BOOL)   TypesQueryPbCVRecordForTiEx(TPI* ptpi, TI ti, OUT BYTE** ppb);
PDBAPI(TI)     TypesQueryTiMinEx(TPI* ptpi);
PDBAPI(TI)     TypesQueryTiMacEx(TPI* ptpi);
PDBAPI(long)   TypesQueryCb(TPI* ptpi);
PDBAPI(BOOL)   TypesClose(TPI* ptpi);
PDBAPI(BOOL)   TypesCommit(TPI* ptpi);
PDBAPI(BOOL)   TypesQueryTiForUDTEx(TPI* ptpi, _In_z_ const char *sz, BOOL fCase, OUT TI* pti);
PDBAPI(BOOL)   TypesSupportQueryTiForUDT(TPI*);
PDBAPI(BOOL)   TypesfIs16bitTypePool(TPI*);
// Map all old ones to new ones for new compilands.
#define TypesQueryPbCVRecordForTi   TypesQueryPbCVRecordForTiEx
#define TypesQueryTiForUDT          TypesQueryTiForUDTEx
PDBAPI(BOOL)    TypesAreTypesEqual( TPI* ptpi, TI ti1, TI ti2 );
PDBAPI(BOOL)    TypesIsTypeServed( TPI* ptpi, TI ti );

PDBAPI(BYTE*)  GSINextSym (GSI* pgsi, BYTE* pbSym);
PDBAPI(BYTE*)  GSIHashSym (GSI* pgsi, _In_z_ const char* szName, BYTE* pbSym);
PDBAPI(BYTE*)  GSINearestSym (GSI* pgsi, USHORT isect, long off,OUT long* pdisp);//currently only supported for publics
PDBAPI(BOOL)   GSIClose(GSI* pgsi);
PDBAPI(unsigned long)   GSIOffForSym( GSI* pgsi, BYTE* pbSym );
PDBAPI(BYTE*)   GSISymForOff( GSI* pgsi, unsigned long off );

PDBAPI(long)   StreamQueryCb(Stream* pstream);
PDBAPI(BOOL)   StreamRead(Stream* pstream, long off, void* pvBuf, long* pcbBuf);
PDBAPI(BOOL)   StreamWrite(Stream* pstream, long off, void* pvBuf, long cbBuf);
PDBAPI(BOOL)   StreamReplace(Stream* pstream, void* pvBuf, long cbBuf);
PDBAPI(BOOL)   StreamAppend(Stream* pstream, void* pvBuf, long cbBuf);
PDBAPI(BOOL)   StreamDelete(Stream* pstream);
PDBAPI(BOOL)   StreamTruncate(Stream* pstream, long cb);
PDBAPI(BOOL)   StreamRelease(Stream* pstream);

PDBAPI(BOOL)   StreamImageOpen(Stream* pstream, long cb, OUT StreamImage** ppsi);
PDBAPI(void*)  StreamImageBase(StreamImage* psi);
PDBAPI(long)   StreamImageSize(StreamImage* psi);
PDBAPI(BOOL)   StreamImageNoteRead(StreamImage* psi, long off, long cb, OUT void** ppv);
PDBAPI(BOOL)   StreamImageNoteWrite(StreamImage* psi, long off, long cb, OUT void** ppv);
PDBAPI(BOOL)   StreamImageWriteBack(StreamImage* psi);
PDBAPI(BOOL)   StreamImageRelease(StreamImage* psi);

PDBAPI(BOOL)   NameMapOpen(PDB* ppdb, BOOL fWrite, OUT NameMap** ppnm);
PDBAPI(BOOL)   NameMapClose(NameMap* pnm);
PDBAPI(BOOL)   NameMapReinitialize(NameMap* pnm);
PDBAPI(BOOL)   NameMapGetNi(NameMap* pnm, _In_z_ const char* sz, OUT NI* pni);
PDBAPI(BOOL)   NameMapGetName(NameMap* pnm, NI ni, _Deref_out_z_ OUT const char** psz);
PDBAPI(BOOL)   NameMapGetEnumNameMap(NameMap* pnm, OUT Enum** ppenum);
PDBAPI(BOOL)   NameMapCommit(NameMap* pnm);

PDBAPI(void)   EnumNameMapRelease(EnumNameMap* penum);
PDBAPI(void)   EnumNameMapReset(EnumNameMap* penum);
PDBAPI(BOOL)   EnumNameMapNext(EnumNameMap* penum);
PDBAPI(void)   EnumNameMapGet(EnumNameMap* penum, _Deref_out_z_ OUT const char** psz, OUT NI* pni);

PDBAPI(void)   EnumContribRelease(EnumContrib* penum);
PDBAPI(void)   EnumContribReset(EnumContrib* penum);
PDBAPI(BOOL)   EnumContribNext(EnumContrib* penum);
PDBAPI(void)   EnumContribGet(EnumContrib* penum, OUT USHORT* pimod, OUT USHORT* pisect, OUT long* poff, OUT long* pcb, OUT ULONG* pdwCharacteristics);
PDBAPI(void)   EnumContribGet2(EnumContrib* penum, OUT USHORT* pimod, OUT USHORT* pisect, OUT DWORD* poff, OUT DWORD* pisectCoff, OUT DWORD* pcb, OUT ULONG* pdwCharacteristics);
PDBAPI(void)   EnumContribGetCrcs(EnumContrib* penum, OUT DWORD* pcrcData, OUT DWORD* pcrcReloc);
PDBAPI(BOOL)   EnumContribfUpdate(EnumContrib* penum, IN long off, IN long cb);

PDBAPI(SIG)    SigForPbCb(BYTE* pb, size_t cb, SIG sig);
PDBAPI(void)   TruncStFromSz(_Out_z_cap_(cbSrc) char *stDst, _In_count_(cbSrc) _Pre_z_ const char *szSrc, size_t cbSrc);

PDBAPI(BOOL)   DbgClose(Dbg *pdbg);
PDBAPI(long)   DbgQuerySize(Dbg *pdbg);
PDBAPI(void)   DbgReset(Dbg *pdbg);
PDBAPI(BOOL)   DbgSkip(Dbg *pdbg, ULONG celt);
PDBAPI(BOOL)   DbgQueryNext(Dbg *pdbg, ULONG celt, OUT void *rgelt);
PDBAPI(BOOL)   DbgFind(Dbg *pdbg, IN OUT void *pelt);
PDBAPI(BOOL)   DbgClear(Dbg *pdbg);
PDBAPI(BOOL)   DbgAppend(Dbg *pdbg, ULONG celt, const void *rgelt);
PDBAPI(BOOL)   DbgReplaceNext(Dbg *pdbg, ULONG celt, const void *rgelt);

PDBAPI(wchar_t *)   SzCanonFilename(_Inout_z_ wchar_t * szFilename);
#endif

#if __cplusplus
};
#endif

#ifndef cbNil
#define cbNil   ((long)-1)
#endif
#define tsNil   ((TPI*)0)
#define tiNil   ((TI)0)
#define imodNil ((USHORT)(-1))

#define pdbFSCompress           "C"
#define pdbVC120                "L"
#define pdbTypeAppend           "a"
#define pdbGetRecordsOnly       "c"
#define pdbFullBuild            "f"
#define pdbGetTiOnly            "i"
#define pdbNoTypeMergeLink      "l"
#define pdbTypeMismatchesLink   "m"
#define pdbNewNameMap           "n"
#define pdbMinimalLink          "o"
#define pdbRead                 "r"
#define pdbWriteShared          "s"
#define pdbCTypes               "t"
#define pdbWrite                "w"
#define pdbExclusive            "x"
#define pdbRepro                "z"

#endif // __PDB_INCLUDED__

#pragma warning(pop)
