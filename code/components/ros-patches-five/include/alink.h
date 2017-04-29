// ==++==
//
//   Copyright (c) Microsoft Corporation.  All rights reserved.
//
// ==--==
// ===========================================================================
// File: alink.h
//
// main ALink interface
// ===========================================================================
#ifndef __alink_h__
#define __alink_h__

#include <winapifamily.h>

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

EXTERN_GUID(CLSID_AssemblyLinker, 0xf7e02368, 0xa7f4, 0x471f, 0x8c, 0x5e, 0x98, 0x39, 0xed, 0x57, 0xcb, 0x5e);

typedef enum _AssemblyOptions {
    optAssemTitle   = 0,    // String
    optAssemDescription,    // String
    optAssemConfig,         // String
    optAssemOS,             // String encoded as: "dwOSPlatformId.dwOSMajorVersion.dwOSMinorVersion"
    optAssemProcessor,      // ULONG
    optAssemLocale,         // String
    optAssemVersion,        // String encoded as: "Major.Minor.Build.Revision"
    optAssemCompany,        // String
    optAssemProduct,        // String
    optAssemProductVersion, // String (aka InformationalVersion)
    optAssemCopyright,      // String
    optAssemTrademark,      // String
    optAssemKeyFile,        // String (filename)
    optAssemKeyName,        // String
    optAssemAlgID,          // ULONG
    optAssemFlags,          // ULONG
    optAssemHalfSign,       // Bool   (aka DelaySign)
    optAssemFileVersion,    // String encoded as: "Major.Minor.Build.Revision" - same as ProductVersion
    optAssemSatelliteVer,   // String encoded as: "Major.Minor.Build.Revision"
    optAssemSignaturePublicKey, // Byte array encoded as hex String

    optLastAssemOption
}   AssemblyOptions;


typedef enum _AssemblyFlags {
    afNone              = 0x00000000, // Normal case
    afInMemory          = 0x00000001, // An InMemory single-file assembly the filename == AssemblyName
    afCleanModules      = 0x00000002, // Use DeleteToken and Merging to remove the AssemblyAttributesGoHere
    afNoRefHash         = 0x00000004, // Do not generate hashes for AssemblyRefs
    afNoDupTypeCheck    = 0x00000008, // Do not check for duplicate types (ExportedType table + manifest file's TypeDef table)
    afDupeCheckTypeFwds = 0x00000010, // Do dupe checking for type forwarders.  This is so you can specify afNoDupTypeCheck for regular typedefs + afDupeCheckTypeFwds.
}   AssemblyFlags;

//-------------------------------------
//--- IALink
//-------------------------------------
//---
// {C8E77F39-3604-4fd4-85CF-38BDEB233AD4}
// {C8E77F39-3604-4fd4-85CF-38BDEB233AD5}
EXTERN_GUID(IID_IALink, 0xc8e77f39, 0x3604, 0x4fd4, 0x85, 0xcf, 0x38, 0xbd, 0xeb, 0x23, 0x3a, 0xd4);
EXTERN_GUID(IID_IALink2, 0xc8e77f39, 0x3604, 0x4fd4, 0x85, 0xcf, 0x38, 0xbd, 0xeb, 0x23, 0x3a, 0xd5);
// {22D4F7A0-0065-43dd-8EAF-B9FB901D8223}
EXTERN_GUID(IID_IALink3, 0x22d4f7a0, 0x65, 0x43dd, 0x8e, 0xaf, 0xb9, 0xfb, 0x90, 0x1d, 0x82, 0x23);

#define AssemblyIsUBM ((mdAssembly)mdAssemblyNil)
#define MAX_IDENT_LEN 2048

#ifndef HALINKENUM
#define HALINKENUM  void*
#endif

#undef  INTERFACE
#define INTERFACE IALink

interface DECLSPEC_UUID("C8E77F39-3604-4fd4-85CF-38BDEB233AD4") IALink : IUnknown
{
    // Call to initialize everything
    STDMETHOD(Init)(
        _In_     IMetaDataDispenserEx*
                        pDispenser,         // IN - MetaData Dispenser
        _In_opt_ IMetaDataError* pErrorHandler) PURE;// IN - optional Error handling interface

    // Call this to import UBMs and Assemblies.
    // Now you can use the same code to import Metadata from both!
    STDMETHOD(ImportFile)(
        _In_z_     LPCWSTR         pszFilename,        // IN - filename of the file to add (must be fully qualified)
        _In_opt_z_ LPCWSTR         pszTargetName,      // IN - (optional) filename to copy bound modules to.
                   BOOL            fSmartImport,       // IN - Set to TRUE if you want to use ImportTypes, etc.  FALSE means you'll do your own importing
        _Out_      mdToken*        pImportToken,       // OUT- unique ID for the file (used to uniquely identify it) - it could be and assembly or file
        _Outptr_opt_ IMetaDataAssemblyImport**
                        ppAssemblyScope,    // OUT- assembly import scope (NULL if the file is not an assembly)
        _Out_      DWORD*          pdwCountOfScopes) PURE;
                                            // OUT- Count of files/scopes imported due to this file
    
    // If you call this function, the linker assumes you are building an assembly
    // Do NOT call this if you are producing UBMs
    STDMETHOD(SetAssemblyFile)(
        _In_z_     LPCWSTR         pszFilename,        // IN - filename of the manifest file (fully-qualified)
        _In_       IMetaDataEmit*  pEmitter,           // IN - Emitter interface for this file
                   AssemblyFlags   afFlags,            // IN - flags for ALink to generate a proper Assembly
        _Out_      mdAssembly*     pAssemblyID) PURE;  // OUT- Unique ID for this assembly


    // Call this to add files to the assembly (or just to create UBMs)
    STDMETHOD(AddFile)(
                   mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to this file to
        _In_z_     LPCWSTR         pszFilename,        // IN - filename of the file to add (must be fully qualified)
                   DWORD           dwFlags,            // IN - COM+ FileDef flags (ffContainsNoMetaData, ffWriteable, etc.)
        _In_       IMetaDataEmit*  pEmitter,           // IN - Emitter interface for file, so we can emit stuff if needed
        _Out_      mdFile*         pFileToken) PURE;   // OUT- unique ID for the file (used to uniquely identify it)

    // Call this to add imports to the assembly
    STDMETHOD(AddImport)(
                   mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to this file to
                   mdToken         ImportToken,        // IN - imported file to add
                   DWORD           dwFlags,            // IN - COM+ FileDef flags (ffContainsNoMetaData, ffWriteable, etc.)
        _Out_      mdFile*         pFileToken) PURE;   // OUT- unique ID for the file (used to uniquely identify it)

    // Get an import scope for
    STDMETHOD(GetScope)(
                   mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to import to
                   mdToken         FileToken,          // IN - unique ID for the file to import from
                   DWORD           dwScope,            // IN - Which scope to import (must be from 0 to dwCountOfScopes - 1)
        _Outptr_   IMetaDataImport**
                        ppImportScope) PURE;// OUT- Import interface for scope

    STDMETHOD(GetAssemblyRefHash)(
                   mdToken         FileToken,          // IN - assembly to Reference (hash)
        _Outptr_result_bytebuffer_(*pcbHash)   const void**    ppvHash,            // OUT- pointer to blob
        _Out_      DWORD*          pcbHash) PURE;      // OUT- size (in bytes) of hash blob

    // Call this to begin importing types from each scope imported via ImportFile()
    STDMETHOD(ImportTypes)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to import to
        mdToken         FileToken,          // IN - unique ID for the file to import from
        DWORD           dwScope,            // IN - Which scope to import (must be from 0 to dwCountOfScopes - 1)
        _Out_           HALINKENUM*     phEnum,             // OUT- Enumerator handle for the types in this scope
        _Outptr_opt_    IMetaDataImport**  ppImportScope,      // OUT- optional Import interface to use
        _Out_ DWORD*    pdwCountOfTypes) PURE;  // OUT- optional count of types in this scope

    // Use this to get Assembly-level custom attributes
    STDMETHOD(EnumCustomAttributes)(
        HALINKENUM      hEnum,              // IN - Enumerator handle
        mdToken         tkType,             // IN - Type of interest, mdTokenNil for all
        _Out_writes_to_(cMax, *pcCustomValues)       mdCustomAttribute
                        rCustomValues[],    // OUT- Put custom attribute tokens here
        ULONG           cMax,               // IN - Size of rCustomValues
        _Out_ ULONG*    pcCustomValues) PURE;
                                            // OUT- optional count of token values here.

    // Call this to enumerate each type in each scope
    STDMETHOD(EnumImportTypes)(
        HALINKENUM      hEnum,              // IN - Enumerator handle
        DWORD           dwMax,              // IN - Max count of types to return
        _Out_writes_bytes_to_(dwMax, *pdwCount)  mdTypeDef       aTypeDefs[],        // OUT- Array of TypeDef tokens, maxlength = dwMax
        _Out_ DWORD*          pdwCount) PURE;     // OUT- Actual count of types in aTypeDefs returned

    // Call this to close the enum, and free up it's resources
    STDMETHOD(CloseEnum)(HALINKENUM hEnum) PURE;

    // Call this to specify a type as being exportable
    STDMETHOD(ExportType)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to export from
        mdToken         FileToken,          // IN - FileToken or AssemblyID of file that defines the type
        mdTypeDef       TypeToken,          // IN - TypeDef token for the type
        _In_z_ LPCWSTR  pszTypename,        // IN - Fully-qualified type name to export
        DWORD           dwFlags,            // IN - Flags for ComType (tdPublic, tdNested*, etc.)
        _Out_ mdExportedType* pType) PURE;  // OUT- Actual ExportedType token emitted (keep this only for emitting nested types)

    // Call this for nested types - prefered, because it's faster
    STDMETHOD(ExportNestedType)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to export from
        mdToken         FileToken,          // IN - FileToken or AssemblyID of file that defines the type
        mdTypeDef       TypeToken,          // IN - TypeDef token for the type
        mdExportedType  ParentType,         // IN - ExportedType token of parent type (outer class)
        _In_z_ LPCWSTR  pszTypename,        // IN - Fully-qualified type name to export
        DWORD           dwFlags,            // IN - Flags for ComType (tdNested*, etc.)
        _Out_ mdExportedType* pType) PURE;  // OUT- Actual ExportedType token emitted (keep this only for emitting nested types)

    // Call this to declare an embedded resource
    // the compiler must actually embed the resource itself
    STDMETHOD(EmbedResource)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly
        mdToken         FileToken,          // IN - FileToken or AssemblyID of file that has the resource
        _In_z_ LPCWSTR  pszResourceName,    // IN - name of resource
        DWORD           dwOffset,           // IN - Offset of resource from RVA
        DWORD           dwFlags) PURE;      // IN - Flags, mrPublic, mrPrivate, etc.

    // Call this to link in a resource
    // This internally calls AddFile(), It is an error if AssemblyID == AssemblyIsUBM
    STDMETHOD(LinkResource)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly
        _In_z_ LPCWSTR  pszFileName,        // IN - name of file
        _In_opt_z_ LPCWSTR pszNewLocation,     // IN - (optional) new filename (if non-NULL, pszFileName will be copied to pszNewLocation)
        _In_z_ LPCWSTR  pszResourceName,    // IN - name of resource
        DWORD           dwFlags) PURE;      // IN - Flags, mrPublic, mrPrivate, etc.

    // Call this to get the resultion scope for a TypeRef
    // Do NOT call this for nested types
    // You do not need to call this if FileToken == TargetFile
    STDMETHOD(GetResolutionScope)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly
        mdToken         FileToken,          // IN - file that is needing a reference
        mdToken         TargetFile,         // IN - token from ImportFile that the type is defined in (not ImportScope)
        _Out_ mdToken*  pScope) PURE;       // OUT- AssemblyRef or ModuleRef

    // Call to set assembly level settings
    // Returns S_FALSE if this overrides a previous setting
    STDMETHOD(SetAssemblyProps)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly
        mdToken         FileToken,          // IN - file that is defining the property (may be NULL if AssemblyID != AssemblyIsUBM)
        AssemblyOptions Option,             // IN - Option to set
        VARIANT         Value) PURE;        // IN - New value for option

    // Call to set assembly level CAs
    // Returns S_FALSE if this overrides a previous setting or CA
    STDMETHOD(EmitAssemblyCustomAttribute)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly
        mdToken         FileToken,          // IN - file that is defining the property (may be NULL if AssemblyID != AssemblyIsUBM)
        mdToken         tkType,             // IN - Type of the CustomAttribute.
        _In_ void const* pCustomValue,       // IN - The custom value data.
        DWORD           cbCustomValue,      // IN - The custom value data length.
        BOOL            bSecurity,          // IN - True if the CA is a security attribute
        BOOL            bAllowMulti) PURE;  // IN - True if AllowMultiple=true

    // Call this after setting all the useful assembly options
    // to get a RES blob
    STDMETHOD(GetWin32ResBlob)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly
        mdToken         FileToken,          // IN - file that is using the RES file (so we can generate a correct internal name)
        BOOL            fDll,               // IN - Is the file a Dll or EXE?
        LPCWSTR         pszIconFile,        // IN - (optional) icon to insert into the RES blob
        _Outptr_result_buffer_(*pcbResBlob) const void**    ppResBlob,          // OUT- Pointer to RES blob
        _Out_ DWORD*          pcbResBlob) PURE;   // OUT- Size of RES blob

    // Call this when you're done with the blob, so it's memory can be freed
    STDMETHOD(FreeWin32ResBlob)(
        _Inout_ const void**    ppResBlob) PURE;    // IN/OUT- We'll free it and set it to NULL

    // Call this after importing all other files and setting all options
    // Never call this for UBMs
    // This will create the actual manifest
    // After calling this if (*pdwReserveSize > 0) allocate space in the
    // PE image for the signature.  Pass the size and resulting RVA to ICeeFileGen::SetStrongNameEntry()
    // or set the appropriate entry in the CLR header directly.
    STDMETHOD(EmitManifest)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly
        _Out_ DWORD*    pdwReserveSize,     // OUT- Size to reserve in assembly file
        _Out_ mdAssembly* ptkManifest) PURE;  // OUT- (optional) Actualy assembly manifest token

    // Call this after closing all other files, but before closing assembly file
    // Never call this for UBMs
    STDMETHOD(PreCloseAssembly)(
        mdAssembly      AssemblyID) PURE;  // IN - Unique ID for the assembly

    // Call this after closing the assembly file
    // Call this to finish off a set of files and begin a new assembly or set of UBMs
    STDMETHOD(CloseAssembly)(
        mdAssembly      AssemblyID) PURE;   // IN - Unique ID for the assembly

    // New Custom Attributes have been merged into the emit scope
    // call this to have ALink 'read' them
    STDMETHOD(EndMerge)(
        mdAssembly      AssemblyID) PURE;   // IN - Unique ID for the assembly

    // Call this to pass flags into ALink when not creating an Assembly
    // (instead of calling SetAssemblyFile)
    STDMETHOD(SetNonAssemblyFlags)(
        AssemblyFlags   afFlags) PURE;      // IN - flags for ALink to generate a proper Assembly

    // CAVEAT: Using pAssemblyScopeIn only works correctly for single file assemblies;
    //         for multi-file assemblies, pszFilename must exist on disk and be current.
    // Same as ImportFile but it is useful if you have an IMetaDataAssemblyImport but
    // pszFilename doesn't exist on disk.
    STDMETHOD(ImportFile2)(
        _In_z_          LPCWSTR         pszFilename,        // IN - filename of the file to add (must be fully qualified)
        _In_opt_z_      LPCWSTR         pszTargetName,      // IN - (optional) filename to copy bound modules to.
        _In_opt_        IMetaDataAssemblyImport*
                                        pAssemblyScopeIn,   // IN - (optional) import scope for pszFilename
                        BOOL            fSmartImport,       // IN - Set to TRUE if you want to use ImportTypes, etc.  FALSE means you'll do your own importing
        _Out_           mdToken*        pImportToken,       // OUT- unique ID for the file (used to uniquely identify it) - it could be and assembly or file
        _Outptr_        IMetaDataAssemblyImport**
                                        ppAssemblyScope,    // OUT- assembly import scope (NULL if the file is not an assembly)
        _Out_           DWORD*          pdwCountOfScopes) PURE;
                                            // OUT- Count of files/scopes imported due to this file

    // Call this to add a type forwarder to the exported type table for this assembly.
    STDMETHOD(ExportTypeForwarder)(
        mdAssemblyRef   tkAssemblyRef,      // IN - AssemblyRef token for the target assembly of this forwarder
        _In_z_ LPCWSTR  pszTypename,        // IN - Fully-qualified type name to export
        DWORD           dwFlags,            // IN - Flags for ComType (tdPublic, tdNested*, etc.)
        _Out_ mdExportedType* pType) PURE;  // OUT- Actual ExportedType token emitted (keep this only for emitting nested types)

    // Call this for nested types 
    STDMETHOD(ExportNestedTypeForwarder)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to export from
        mdToken         FileToken,          // IN - FileToken or AssemblyID of file that defines the type
        mdTypeDef       TypeToken,          // IN - TypeDef token for the type
        mdExportedType  ParentType,         // IN - ExportedType token of parent type (outer class)
        _In_z_ LPCWSTR  pszTypename,        // IN - Fully-qualified type name to export
        DWORD           dwFlags,            // IN - Flags for ComType (tdNested*, etc.)
        _Out_ mdExportedType* pType) PURE;  // OUT- Actual ExportedType token emitted (keep this only for emitting nested types)


};

#undef  INTERFACE
#define INTERFACE IALink2

interface DECLSPEC_UUID("C8E77F39-3604-4fd4-85CF-38BDEB233AD5") IALink2 : IALink
{
    // If you call this function, the linker assumes you are building an assembly
    // Do NOT call this if you are producing UBMs
    STDMETHOD(SetAssemblyFile2)(
        _In_z_ LPCWSTR         pszFilename,        // IN - filename of the manifest file (fully-qualified)
        _In_   IMetaDataEmit2* pEmitter,           // IN - Emitter interface for this file
               AssemblyFlags   afFlags,            // IN - flags for ALink to generate a proper Assembly
        _Out_  mdAssembly*     pAssemblyID) PURE;  // OUT- Unique ID for this assembly


    // Call this to add files to the assembly (or just to create UBMs)
    STDMETHOD(AddFile2)(
                mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to this file to
        _In_z_  LPCWSTR         pszFilename,        // IN - filename of the file to add (must be fully qualified)
                DWORD           dwFlags,            // IN - COM+ FileDef flags (ffContainsNoMetaData, ffWriteable, etc.)
        _In_    IMetaDataEmit2* pEmitter,           // IN - Emitter interface for file, so we can emit stuff if needed
        _Out_   mdFile*         pFileToken) PURE;   // OUT- unique ID for the file (used to uniquely identify it)

    // Get an import scope for
    STDMETHOD(GetScope2)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to import to
        mdToken         FileToken,          // IN - unique ID for the file to import from
        DWORD           dwScope,            // IN - Which scope to import (must be from 0 to dwCountOfScopes - 1)
        _Outptr_ IMetaDataImport2**
                        ppImportScope) PURE;// OUT- Import interface for scope

    // Call this to begin importing types from each scope imported via ImportFile()
    STDMETHOD(ImportTypes2)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to import to
        mdToken         FileToken,          // IN - unique ID for the file to import from
        DWORD           dwScope,            // IN - Which scope to import (must be from 0 to dwCountOfScopes - 1)
        _Out_    HALINKENUM*     phEnum,    // OUT- Enumerator handle for the types in this scope
        _Outptr_ IMetaDataImport2**
                        ppImportScope,      // OUT- optional Import interface to use
        _Out_    DWORD*          pdwCountOfTypes) PURE;
                                            // OUT- optional count of types in this scope

    // Call this to get the actual FileDef token used in metadata (rather than ALink's token)
    STDMETHOD(GetFileDef)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly
        mdFile          TargetFile,         // IN - token for the added file (from AddFile or AddImport)
        _Out_ mdFile*   pScope) PURE;       // OUT- actual mdFile token

    // Call this to extract the public key token for a given keyfile or key container
    STDMETHOD(GetPublicKeyToken)(
        LPCWSTR         pszKeyFile,         // IN- filename of key
        LPCWSTR         pszKeyContainer,    // IN- key container name
        _In_reads_bytes_(*pcbPublicKeyToken) void*           pvPublicKeyToken,   // IN- pointer to blob
        _Inout_ DWORD*          pcbPublicKeyToken) PURE;
                                            // IN, OUT- size (in bytes) of public key token

    // Call this after adding all modules (via AddImport) if known in advance
    // That internal types should be added to the ExportedType table
    // Otherwise these types will not get added until after all attributes have been
    // compiled (inside IALink::EmitManifest)
    STDMETHOD(EmitInternalExportedTypes) (
        mdAssembly      AssemblyID) PURE;   // IN - Unique ID for the assembly

    // Call this to import UBMs and Assemblies.
    // Now you can use the same code to import Metadata from both!
    STDMETHOD(ImportFileEx)(
        _In_z_   LPCWSTR         pszFilename,        // IN - filename of the file to add (must be fully qualified)
        _In_z_   LPCWSTR         pszTargetName,      // IN - (optional) filename to copy bound modules to.
                 BOOL            fSmartImport,       // IN - Set to TRUE if you want to use ImportTypes, etc.  FALSE means you'll do your own importing
                 DWORD           dwOpenFlags,        // IN - Flags to pass into IMetaDataDispenser::OpenScope
        _Out_    mdToken*        pImportToken,       // OUT- unique ID for the file (used to uniquely identify it) - it could be and assembly or file
        _Outptr_ IMetaDataAssemblyImport**
                        ppAssemblyScope,    // OUT- assembly import scope (NULL if the file is not an assembly)
        _Out_    DWORD*          pdwCountOfScopes) PURE;
                                            // OUT- Count of files/scopes imported due to this file
    // CAVEAT: Using pAssemblyScopeIn only works correctly for single file assemblies;
    //         for multi-file assemblies, pszFilename must exist on disk and be current.
    // Same as ImportFile but it is useful if you have an IMetaDataAssemblyImport but
    // pszFilename doesn't exist on disk.
    STDMETHOD(ImportFileEx2)(
        _In_z_   LPCWSTR         pszFilename,        // IN - filename of the file to add (must be fully qualified)
        _In_z_   LPCWSTR         pszTargetName,      // IN - (optional) filename to copy bound modules to.
        _In_opt_ IMetaDataAssemblyImport* 
                        pAssemblyScopeIn,   // IN - (optional) import scope for pszFilename
                 BOOL            fSmartImport,       // IN - Set to TRUE if you want to use ImportTypes, etc.  FALSE means you'll do your own importing
                 DWORD           dwOpenFlags,        // IN - Flags to pass into IMetaDataDispenser::OpenScope
        _Out_    mdToken*        pImportToken,       // OUT- unique ID for the file (used to uniquely identify it) - it could be and assembly or file
        _Outptr_ IMetaDataAssemblyImport**
                                 ppAssemblyScope,    // OUT- assembly import scope (NULL if the file is not an assembly)
        _Out_    DWORD*          pdwCountOfScopes) PURE;
                                                     // OUT- Count of files/scopes imported due to this file

    // Call this to set the PE kind so that ALink can check your added modules and references
    // For conflicts
    // Uses the same enums as IMetaDataImport2::GetPEKind
    // All files are either not PEs, Agnostic (peILonly, IMAGE_FILE_MACHINE_I386), or machine specific
    // Non-PEs are never checked
    // For added modules it is an error to mix and match (except an agnostic module with a machine specific manifest file)
    // For references it is a warning for a machine specific assembly to reference different machines
    STDMETHOD(SetPEKind)(
        mdAssembly      AssemblyID,         // IN - Unique ID for the assembly to this file to
        mdToken         FileToken,          // IN - file to be set (may be NULL if AssemblyID != AssemblyIsUBM)
        DWORD           dwPEKind,           // IN - The kind of PE (see CorPEKind enum in corhdr.h)
        DWORD           dwMachine) PURE;    // IN - Machine as defined in NT header

    // Can call this after closing all other files, but before closing assembly file
    // Never call this for UBMs
    STDMETHOD(EmitAssembly)(
        mdAssembly      AssemblyID) PURE;  // IN - Unique ID for the assembly
};


#undef  INTERFACE
#define INTERFACE IALink3

interface DECLSPEC_UUID("22D4F7A0-0065-43dd-8EAF-B9FB901D8223") IALink3 : IALink2
{
    // Call this prior to asking for the Win32ResBlob.
    // 'pszFile' is the name of the manifest file whose contents will be read
    // and placed in the Win32 resources with id of RT_MANIFEST.
    // When called with a parameter of NULL, any previously read manifest
    // will be cleared. This allows one to reset the state of the linker
    // to that of initialization time.
    STDMETHOD(SetManifestFile)(_In_opt_z_ LPCWSTR pszFile) PURE;
};


#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI CreateALink(REFIID riid, _Outptr_ IUnknown** ppInterface);
HINSTANCE WINAPI GetALinkMessageDll();

#ifdef __cplusplus
}
#endif

#endif

#endif


