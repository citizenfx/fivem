#include <StdInc.h>
#include <Hooking.h>
#include "CrossBuildRuntime.h"

#include <MinHook.h>
#include <unordered_set>

//
// Ignore 'stub' assets (exclusive content) that may crash the game.
// - dlc_mpG9EC, as of b2612
// - dlc_mpSum2_g9ec, as of b2699
// - dlc_mpChristmas3_G9EC, as of b2802
//

struct CDataFileMgr
{
	struct DataFile
	{
		char name[128];
	};
};

struct ChangeSetEntry
{
	CDataFileMgr::DataFile* dataFile;
	int type;
	// ...
};

static std::unordered_set<std::string> g_badFiles{
	"dlc_mpG9EC:/x64/levels/gta5/vehicles/mpG9EC.rpf",
	"dlc_mpG9EC:/x64/levels/mpg9ec/vehiclemods/vehlivery3_mods.rpf",
	"dlc_mpG9ECCRC:/common/data/levels/gta5/vehicles.meta",
	"dlc_mpG9ECCRC:/common/data/carcols.meta",
	"dlc_mpG9ECCRC:/common/data/carvariations.meta",
	"dlc_mpG9ECCRC:/common/data/handling.meta",
	"dlc_mpG9ECCRC:/common/data/shop_vehicle.meta",
	"dlc_mpG9EC:/x64/levels/mpg9ec/vehiclemods/arbitergt_mods.rpf",
	"dlc_mpG9EC:/x64/levels/mpg9ec/vehiclemods/astron2_mods.rpf",
	"dlc_mpG9EC:/x64/levels/mpg9ec/vehiclemods/cyclone2_mods.rpf",
	"dlc_mpG9EC:/x64/levels/mpg9ec/vehiclemods/ignus2_mods.rpf",
	"dlc_mpG9EC:/x64/levels/mpg9ec/vehiclemods/s95_mods.rpf",
	"dlc_mpG9EC:/x64/levels/gta5/props/Prop_Exc_01.rpf",
	"dlc_mpG9EC:/x64/levels/gta5/props/exc_Prop_Exc_01.ityp",
	"dlc_mpG9EC:/x64/levels/gta5/props/Prop_tr_overlay.rpf",
	"dlc_mpG9EC:/x64/levels/gta5/props/exc_Prop_tr_overlay.ityp",
	"dlc_mpG9EC:/x64/anim/creaturemetadata.rpf",
	"dlc_mpG9EC:/common/data/effects/peds/first_person_alternates.meta",
	"dlc_mpG9ECCRC:/common/data/mp_m_freemode_01_mpg9ec_shop.meta",
	"dlc_mpG9EC:/x64/models/cdimages/mpg9ec_male.rpf",
	"dlc_mpG9ECCRC:/common/data/mp_f_freemode_01_mpg9ec_shop.meta",
	"dlc_mpG9EC:/x64/models/cdimages/mpg9ec_female.rpf",

	"dlc_mpSum2_g9ec:/x64/levels/mpsum2_g9ec/vehiclemods/feltzer3hsw_mods.rpf",
	"dlc_mpSum2_g9ec:/x64/levels/mpsum2_g9ec/vehiclemods/vigero2hsw_mods.rpf",
	"dlc_mpSum2_g9ec:/x64/models/cdimages/mpSum2_g9ec_female.rpf",
	"dlc_mpSum2_g9ec:/x64/models/cdimages/mpSum2_g9ec_female_p.rpf",
	"dlc_mpSum2_g9ec:/x64/models/cdimages/mpSum2_g9ec_male.rpf",
	"dlc_mpSum2_g9ec:/x64/models/cdimages/mpSum2_g9ec_male_p.rpf",
	"dlc_mpSum2_g9ecCRC:/common/data/mp_f_freemode_01_mpSum2_g9ec_shop.meta",
	"dlc_mpSum2_g9ecCRC:/common/data/mp_m_freemode_01_mpSum2_g9ec_shop.meta",
	"dlc_mpSum2_g9ec:/x64/anim/creaturemetadata.rpf",
	"dlc_mpSum2_g9ec:/common/data/effects/peds/first_person_alternates.meta",
	"dlc_mpSum2_g9ec:/common/data/effects/peds/first_person.meta",
	"dlc_mpSum2_g9ecCRC:/common/data/pedalternatevariations.meta",

	"dlc_mpChristmas3_G9EC:/x64/levels/mpChristmas3_G9EC/vehiclemods/entity3hsw_mods.rpf",
	"dlc_mpChristmas3_G9EC:/x64/levels/mpChristmas3_G9EC/vehiclemods/issi8hsw_mods.rpf",
};

static void (*_applyChangeSetEntry)(ChangeSetEntry* entry);

static void ApplyChangeSetEntryStub(ChangeSetEntry* entry)
{
	if (entry->type != 6 && entry->type != 7 && entry->dataFile && g_badFiles.find(entry->dataFile->name) != g_badFiles.end())
	{
		return;
	}

	_applyChangeSetEntry(entry);
}

static HookFunction hookFunction([]()
{
	if (xbr::IsGameBuildOrGreater<2612>())
	{
		auto location = hook::get_call(hook::get_pattern("48 8D 0C 40 48 8D 0C CE E8 ? ? ? ? FF C3", 8));
		MH_Initialize();
		MH_CreateHook(location, ApplyChangeSetEntryStub, (void**)&_applyChangeSetEntry);
		MH_EnableHook(location);
	}
});
