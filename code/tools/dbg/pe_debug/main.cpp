#include <memory>
#include <codecvt>
#include <regex>

#include <assert.h>

#include <peframework.h>

#include "mangle.h"

// Get PDB headers.
#include "msft_pdb/include/cvinfo.h"
#include "msft_pdb/langapi/include/pdb.h"
#include "msft_pdb/langapi/shared/crc32.h"

#include <Shellapi.h>

// From other compilation modules (for a reason).
void tryGenerateSamplePDB( PEFile& peFile, const char* exeFile, const char* symFile );

static void printHeader( void )
{
    printf(
        "PEframework PE file debug extender written by The_GTA\n" \
        "Made to advance the professionality of the GTA community hacking experience\n" \
        "wordwhirl@outlook.de\n\n"
    );
}

#include <fstream>

int main( int argc, char *argv[] )
{
	if (argc < 3)
	{
		printf("invalid args\n");
		return 0;
	}

	PEFile filedata;

	{
		std::filebuf fbuf;
		fbuf.open(argv[1], std::ios::binary | std::ios_base::in);

		std::iostream stlStream(&fbuf);

		PEStreamSTL peStream(&stlStream);

		printf("found input file, processing...\n");

		filedata.LoadFromDisk(&peStream);
	}

	tryGenerateSamplePDB(filedata, argv[1], argv[2]);

	{
		std::filebuf fbuf;
		fbuf.open(argv[1], std::ios::binary | std::ios_base::out);
		std::iostream stlStream(&fbuf);

		PEStreamSTL peStream(&stlStream);

		filedata.WriteToStream(&peStream);
	}

	return 0;
}
