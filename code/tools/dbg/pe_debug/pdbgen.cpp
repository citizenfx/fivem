#include <memory>
#include <codecvt>
#include <regex>

#include <assert.h>

#include <peframework.h>
#include <filesystem>

#include "mangle.h"

// Get PDB headers.
#include "msft_pdb/include/cvinfo.h"
#include "msft_pdb/langapi/include/pdb.h"
#include "msft_pdb/langapi/shared/crc32.h"

#undef VOID
#undef CDECL

// Utility to parse debug information from a text file, created by an IDC script...
// https://www.hex-rays.com/products/ida/support/freefiles/dumpinfo.idc
struct nameInfo
{
    std::string name;
    std::uint64_t absolute_va;
};
typedef std::vector <nameInfo> symbolNames_t;

static const std::regex patternMatchItem(R"(([0123456789aAbBcCdDeEfF]+)[\s\t]+([^)]+)[\s\t]+(.+))");

// Thanks to https://www.snip2code.com/Snippet/735099/Dump-PDB-information-from-a-PE-file/
const DWORD CV_SIGNATURE_RSDS = 0x53445352; // 'SDSR'

struct CV_INFO_PDB70
{
    DWORD      CvSignature;
    SIG70      Signature;
    DWORD      Age;
    //BYTE       PdbFileName[1];
};

void tryGenerateSamplePDB( PEFile& peFile, const char* exeFile, const char* symFile )
{
    // Prepare symbol names from an input file.
    symbolNames_t symbols;
    {
        try
        {
			symbols = {};

			FILE* f = fopen(symFile, "r");

			while (!feof(f))
			{
				char buf[4096] = { 0 };
				fgets(buf, sizeof(buf), f);

				std::smatch results;
				std::string l = buf;
				l = l.substr(0, l.length() - 1);

				bool gotMatch = std::regex_match(l, results, patternMatchItem);

				if (gotMatch && results.size() == 4)
				{
					std::string offset = std::move(results[1]);
					std::string typeName = std::move(results[2]);
					std::string valueString = std::move(results[3]);

					if (typeName == "UserName")
					{
						try
						{
							nameInfo newInfo;
							newInfo.name = std::move(valueString);
							newInfo.absolute_va = std::stoull(offset, NULL, 16);

							symbols.push_back(std::move(newInfo));
						}
						catch (...)
						{
							// Ignore cast error.
						}
					}
				}
			}

			fclose(f);

            printf( "finished reading symbols file.\n" );
        }
        catch( ... )
        {
            throw;
        }
    }
    
    // Establish a file location.
	std::string narrowPath = exeFile;
	narrowPath = narrowPath.substr(0, narrowPath.find_last_of('.'));

	std::wstring widePath(narrowPath.begin(), narrowPath.end());

	std::filesystem::path p(widePath);
	std::wstring widePDBFileLocation = (p.parent_path() / L"dbg" / p.filename()).wstring() + L".pdb";

    wprintf( L"generating PDB file (%s)\n", widePDBFileLocation.c_str() );

    EC error_code_out;
    wchar_t errorBuf[ 4096 ];

    PDB *pdbHandle;

    BOOL openSuccess =
        PDB::Open2W(
            widePDBFileLocation.c_str(), "wb", &error_code_out, errorBuf, _countof(errorBuf),
            &pdbHandle
        );

    if ( openSuccess == FALSE )
    {
        // We fail in life.
        printf( "failed to create PDB file\n" );
        return;
    }

    // Yes!
    DBI *dbiHandle;

    BOOL dbiOpenSuccess =  pdbHandle->OpenDBI( NULL, "rwb", &dbiHandle );

    if ( dbiOpenSuccess == TRUE )
    {
        // One step closer.

		Mod* mod;

		long m;
		dbiHandle->QueryNoOfMods(&m);

		std::vector<Mod*> ms(m);
		dbiHandle->QueryMods(&ms[0], m);
		
		std::string nn;

		for (auto& mmod : ms)
		{
			char n[260];
			long ns = sizeof(n);
			mmod->QueryName(n, &ns);

			mmod->Close();

			if (strstr(n, "DummyVariables"))
			{
				nn = n;
				break;
			}
		}

		dbiHandle->DeleteMod(nn.c_str());

		dbiHandle->OpenMod("fxtm", "fxtm.o", &mod);

		std::vector<uint8_t> symList;
		symList.reserve(2048);
		size_t l = 2048;

		auto append = [&symList, &l](size_t len)
		{
			auto oldLen = symList.size();

			if ((oldLen + len) > l)
			{
				l *= 2;
				symList.reserve(l);
			}

			symList.resize(oldLen + len);

			return &symList[oldLen];
		};

		*(ULONG*)(append(sizeof(ULONG))) = CV_SIGNATURE_C13;
		auto h = (CV_DebugSSubsectionHeader_t*)append(sizeof(CV_DebugSSubsectionHeader_t));
		h->type = DEBUG_S_SYMBOLS;

        // Embed parsed symbols as publics.
        if ( symbols.empty() == false )
        {
            printf( "embedding %d symbols into PDB\n", symbols.size() );

            CV_PUBSYMFLAGS pubflags_func;
            pubflags_func.grfFlags = 0;
            pubflags_func.fFunction = true;

            CV_PUBSYMFLAGS pubflags_data;
            pubflags_data.grfFlags = 0;

            std::uint64_t imageBase = peFile.GetImageBase();

			int si = 0;

            for ( nameInfo& infoItem : symbols )
            {
                // Convert the VA into a RVA.
                std::uint32_t rva = (std::uint32_t)( infoItem.absolute_va - imageBase );

                // Find the section associated with this item.
                // If we found it, add it as public symbol.
                std::uint32_t sectIndex = 0;

                PEFile::PESection *symbSect = peFile.FindSectionByRVA( rva, &sectIndex );

                if ( symbSect )
                {
                    // Get the offset into the section.
                    std::uint64_t native_off = ( rva - symbSect->GetVirtualAddress() );
                    
                    // If this item is in the executable section, we put a function symbol.
                    // Otherwise we put a data symbol.
                    CV_pubsymflag_t useFlags;

                    if ( symbSect->chars.sect_mem_execute || true )
                    {
                        useFlags = pubflags_func.grfFlags;
                    }
                    else
                    {
                        useFlags = pubflags_data.grfFlags;
                    }

                    // Try to transform the symbol name into a C++ representation if we can.
                    std::string symbName = std::move( infoItem.name );
                    {
                        ProgFunctionSymbol symbCodec;
                        bool gotSymbol = symbCodec.ParseMangled( symbName.c_str() );

                        if ( gotSymbol )
                        {
#ifdef PEDEBUG_ENABLE_FAKE_SYMBOL_INFORMATION
                            // Because IDA does not support the entirety of the Visual C++ mangle, we
                            // need to patch some things up for it so it will still support the mangled names.
                            if ( symbCodec.returnType == eSymbolValueType::UNKNOWN )
                            {
                                symbCodec.returnType = eSymbolValueType::VOID;
                            }

                            if ( symbCodec.callingConv == eSymbolCallConv::UNKNOWN )
                            {
                                symbCodec.callingConv = eSymbolCallConv::CDECL;
                            }
#endif //PEDEBUG_ENABLE_FAKE_SYMBOL_INFORMATION

                            // Remangle the name in Visual C++ format, if possible.
                            symbCodec.OutputMangled(
                                ProgFunctionSymbol::eManglingType::VISC,
                                symbName
                            );
                        }
                    }

                    // Remove previous definition of this public.
                    //dbiHandle->RemovePublic( symbName.c_str() );
					PROCSYM32* procsym = (PROCSYM32*)append(sizeof(PROCSYM32) + symbName.length());
					memset(procsym, 0, sizeof(PROCSYM32));

					procsym->reclen = sizeof(*procsym) + symbName.length() - 2;
					procsym->rectyp = S_GPROC32;

					if (si < (symbols.size() - 1))
					{
						procsym->len = symbols[si + 1].absolute_va - symbols[si].absolute_va;
					}

					procsym->typind = 0; // ?
					procsym->seg = sectIndex + 1; // ??
					procsym->off = native_off;
					procsym->flags.bAll = 0;
					strcpy((char*)procsym->name, symbName.c_str());

					SYMTYPE* endsym = (SYMTYPE*)append(sizeof(SYMTYPE));
					endsym->reclen = 2;
					endsym->rectyp = S_END;

                    dbiHandle->AddPublic2( symbName.c_str(), sectIndex + 1, native_off, useFlags );
                }
                else
                {
                    printf( "failed to map symbol '%s' (invalid RVA)\n", infoItem.name.c_str() );
                }

				si++;
            }
        }

		((CV_DebugSSubsectionHeader_t*)&symList[4])->cbLen = symList.size() - sizeof(ULONG) - sizeof(CV_DebugSSubsectionHeader_t);

		BOOL r = mod->AddSymbols((BYTE*)symList.data(), symList.size());

		if (!r)
		{
			char g[1024];
			DWORD e = pdbHandle->QueryLastError(g);
			printf("f? %d\n", e);
		}

		mod->Close();

		dbiHandle->RemovePublic("dummy_seg");
		dbiHandle->RemovePublic("stub_seg");
		dbiHandle->RemovePublic("zdata");
		dbiHandle->RemovePublic("?dummy_seg@@3PADA");
		dbiHandle->RemovePublic("?stub_seg@@3PADA");
		dbiHandle->RemovePublic("?zdata@@3PADA");

        // Write information about all sections.
        Dbg *dbgSectHeader;

        BOOL gotSectStream = dbiHandle->OpenDbg( dbgtypeSectionHdr, &dbgSectHeader );

        if ( gotSectStream == TRUE )
        {
            // We do not want any previous data.
            dbgSectHeader->Clear();

            // Write new things.
            peFile.ForAllSections(
                [&]( PEFile::PESection *sect )
            {
                IMAGE_SECTION_HEADER header;
                strncpy( (char*)header.Name, sect->shortName.c_str(), _countof(header.Name) );
                header.Misc.VirtualSize = sect->GetVirtualSize();
                header.VirtualAddress = sect->GetVirtualAddress();
                header.SizeOfRawData = (DWORD)sect->stream.Size();
                header.PointerToRawData = 0;
                header.PointerToRelocations = 0;
                header.PointerToLinenumbers = 0;
                header.NumberOfRelocations = 0;
                header.NumberOfLinenumbers = 0;
                header.Characteristics = sect->GetPENativeFlags();

                dbgSectHeader->Append( 1, &header );
            });

            dbgSectHeader->Close();
        }

        // Remember to close our stuff.
        dbiHandle->Close();
    }

    // Make sure everything is written?
    pdbHandle->Commit();

    printf( "finished writing to PDB file!\n" );

    // Inject PDB information into the EXE file.
    {
		PEFile::PEDebugDesc& cvDebug = peFile.debugDescs[0];

        PEFile::fileSpaceStream_t stream = cvDebug.dataStore.OpenStream();

        // First write the header.
        CV_INFO_PDB70 pdbDebugEntry;

		stream.Read(&pdbDebugEntry, sizeof(pdbDebugEntry));

        pdbDebugEntry.CvSignature = CV_SIGNATURE_RSDS;
        BOOL gotSig = pdbHandle->QuerySignature2( &pdbDebugEntry.Signature );
        pdbDebugEntry.Age = pdbHandle->QueryAge();

        assert( gotSig == TRUE );

		stream.Seek(0);

        stream.Write( &pdbDebugEntry, sizeof(pdbDebugEntry) );

        // Done!
    }

    printf( "injected debug information into PE file\n" );

    // Remember to close our PDB again for sanity!
    pdbHandle->Close();
}
