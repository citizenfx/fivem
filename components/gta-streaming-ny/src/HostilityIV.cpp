#include "StdInc.h"
#include "Hooking.h"

static HookFunction hookFunction([] ()
{
	// define global divisor
	// can only be a single divisor for both axes without having to do more complicated code patching in some funcs :(
	static float divisor = 1.0f / (16000 / 120.0f);
	static float idleDivisor = (16000 / 120.0f);
	static float lodDivisor = 1.0f / (16000 / 30.0f);
	static float idleLodDivisor = (16000 / 30.0f);
	static float posExtent = 8000.0f;
	static float negExtent = -4000.0f;

	// 7D7700
	hook::put(0x7D771A, &divisor);

	hook::put(0x7DDDD6, &divisor);

	hook::put(0x7DF166, &divisor);
	hook::put(0x7DF221, &divisor);

	hook::put(0x7DF166, &divisor);

	hook::put(0x8B8E75, &divisor);

	// v is for navmeshes, not relevant here
	//hook::put(0x938F9D, &divisor);

	hook::put(0x9E00AA, &divisor);

	// |
	// v navmeeeeesh
	//hook::put(0xAA5A7E, &divisor);
	//hook::put(0xAA5AFE, &divisor);
	//hook::put(0xAA5B6A, &divisor);
	//hook::put(0xAA5BE2, &divisor);

	// |
	// v more navmesh stuff
	//hook::put(0xAA5F8A, &divisor);
	//hook::put(0xAA6005, &divisor);
	//hook::put(0xAA6079, &divisor);
	//hook::put(0xAA60F4, &divisor);

	hook::put(0xADE6C9, &divisor);

	hook::put(0xAE3B0A, &divisor);
	hook::put(0xAE3AFC, &idleDivisor);

	hook::put(0xB257F1, &divisor);

	hook::put(0xAA5A7E, &divisor);

	hook::put(0xBF5F08, &divisor);
	hook::put(0xBF5F75, &divisor);
	hook::put(0xBF5FB4, &divisor);

	// 818C40
	hook::put(0x818C4B, &divisor);

	// 81B4A0
	hook::put(0x81B4AB, &divisor);
	hook::put(0x81B571, &divisor);
	hook::put(0x81B5A3, &divisor);

	// 81BB90
	hook::put(0x81BBCD, &divisor);
	hook::put(0x81BBE7, &divisor);
	hook::put(0x81BC03, &divisor);
	hook::put(0x81BC59, &divisor);

	// 81BFD0
	hook::put(0x81C00D, &divisor);
	hook::put(0x81C027, &divisor);
	hook::put(0x81C043, &divisor);
	hook::put(0x81C099, &divisor);

	// 81C3D0
	hook::put(0x81C40D, &divisor);
	hook::put(0x81C427, &divisor);
	hook::put(0x81C443, &divisor);
	hook::put(0x81C499, &divisor);

	// 8FC7D0
	hook::put(0x8FC80D, &divisor);
	hook::put(0x8FC827, &divisor);
	hook::put(0x8FC843, &divisor);
	hook::put(0x8FC899, &divisor);

	// CEntity::Add
	hook::put(0x9E9FA6, &divisor);
	hook::put(0x9E9FC6, &divisor);
	hook::put(0x9EA001, &divisor);
	hook::put(0x9EA050, &divisor);

	hook::put(0x9E9E24, &negExtent);
	hook::put(0x9E9E5A, &posExtent);

	// CEntity::something
	hook::put(0x9EA826, &divisor);
	hook::put(0x9EA849, &divisor);
	hook::put(0x9EA892, &divisor);
	hook::put(0x9EA8A2, &divisor);

	hook::put(0x9EA6CF, &negExtent);
	hook::put(0x9EA701, &posExtent);

	// 9FA130
	hook::put(0x9FA16D, &divisor);
	hook::put(0x9FA185, &divisor);
	hook::put(0x9FA1D0, &divisor);
	hook::put(0x9FA1E5, &divisor);

	// A739C0
	hook::put(0xA739ED, &divisor);
	hook::put(0xA73A06, &divisor);
	hook::put(0xA73A50, &divisor);
	hook::put(0xA73A66, &divisor);

	// B257E0
	hook::put(0xB257F1, &divisor);

	// B2B660
	hook::put(0xB2B6A0, &divisor);

	// BBE1A0
	hook::put(0xBBE214, &divisor);

	// BBE440
	hook::put(0xBBE479, &divisor);
	hook::put(0xBBE4BB, &divisor);
	hook::put(0xBBE547, &divisor);

	// repeatsectors only caller
	hook::put(0x9E00AA, &divisor);

	// repeatsectors only, BF7B90
	hook::put(0xBF7C27, &divisor);
	hook::put(0xBF7C8E, &divisor);
	hook::put(0xBF7CCF, &divisor);

	// BF81E0
	hook::put(0xBF82F3, &divisor);

	// FIN... for now

	// lod sectors
	hook::put(0x819000, &lodDivisor);

	hook::put(0x9E9E9E, &lodDivisor);

	hook::put(0x9EA745, &lodDivisor);

	hook::put(0xB2BAAD, &lodDivisor);

	// worldscan
	hook::put(0x7DE008, &lodDivisor);

	hook::put(0x7DF452, &lodDivisor);
	hook::put(0x7DF505, &lodDivisor);

	// iplstore quadtree bounds
	static float minCoord = -8000.0f;
	static float maxCoord = 8000.0f;

	hook::put(0xB2731B, &minCoord);
	hook::put(0xB27335, &maxCoord);

	// iplstore subnodes
	//hook::put<uint8_t>(0xB276EB, 4);

	// boundsstore
	hook::put(0xC0A22B, &minCoord);
	hook::put(0xC0A245, &maxCoord);

	// boundsstore subnodes
	//hook::put<uint8_t>(0xC0A6AB, 4);
});