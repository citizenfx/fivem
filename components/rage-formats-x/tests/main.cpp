#include "StdInc.h"
#include <gtest/gtest.h>

#include <pgBase.h>
#include <rmcDrawable.h>

using namespace rage::ny;

class testClass : public pgBase
{
private:
	char cake[32];

public:
	pgPtr<testClass> tc1;
	pgPtr<testClass> tc2;

public:
	testClass()
	{
		memset(cake, 0x32, sizeof(cake));
	}
};

int main(int argc, char **argv)
{
	//::testing::InitGoogleTest(&argc, argv);
	//return RUN_ALL_TESTS();

	/*auto blockMap = pgStreamManager::BeginPacking();

	auto dr = new(false) rmcDrawableBase();

	auto sg = new(false) grmShaderGroup();
	
	pgStreamManager::EndPacking();*/

	char* buffer = new char[4096];
	FILE* f = fopen("T:/vexp/mm/bh1_01_emissivesigns.wdr.sys", "rb");
	fread(buffer, 1, 4096, f);
	fclose(f);

	BlockMap bm;
	bm.virtualLen = 1;
	bm.physicalLen = 0;
	bm.blocks[0].data = buffer;
	bm.blocks[0].offset = 0;
	bm.blocks[0].size = 4096;

	pgPtrRepresentation ptr;
	ptr.blockType = 5;
	ptr.pointer = 0;

	pgStreamManager::SetBlockInfo(&bm);

	rmcDrawable* drawable = (rmcDrawable*)pgStreamManager::ResolveFilePointer(ptr, &bm);
	drawable->Resolve();

	return 0;
}