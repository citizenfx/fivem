import os, sys

outf = sys.argv[1]
pn = sys.argv[2]

pid = os.getenv('CI_PIPELINE_ID', '1')

f = open(outf, 'w')
f.write("""
#include <windows.h>

LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US

VS_VERSION_INFO VERSIONINFO
 FILEVERSION 1,0,0,{0}
 PRODUCTVERSION 1,0,0,{0}
 FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
 FILEFLAGS 0x1L
#else
 FILEFLAGS 0x0L
#endif
 FILEOS 0x40004L
 FILETYPE 0x1L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName", "Cfx.re"
#if defined(GTA_FIVE)
			VALUE "FileDescription", "{1} for FiveM"
#elif defined(IS_RDR3)
			VALUE "FileDescription", "{1} for RedM"
#elif defined(IS_FXSERVER)
			VALUE "FileDescription", "{1} for FXServer"
#else
			VALUE "FileDescription", "{1} for CitizenFX"
#endif
            VALUE "FileVersion", "1.0.0.{0}"
            VALUE "InternalName", "{1}"
            VALUE "LegalCopyright", "(C) 2015- CitizenFX Collective"
            VALUE "OriginalFilename", "{1}.dll"
            VALUE "ProductName", "CitizenFX"
            VALUE "ProductVersion", "1.0.0.{0}"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END
""".format(pid, pn))
f.close()