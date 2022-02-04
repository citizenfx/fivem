#include <StdInc.h>

#include <Pool.h>
#include <Hooking.h>
#include <MinHook.h>
#include <Error.h>

class RageHashList
{
public:
	template<int Size>
	RageHashList(const char* (&list)[Size])
	{
		for (int i = 0; i < Size; i++)
		{
			m_lookupList.insert({ HashString(list[i]), list[i] });
		}
	}

	inline std::string LookupHash(uint32_t hash)
	{
		auto it = m_lookupList.find(hash);

		if (it != m_lookupList.end())
		{
			return std::string(it->second);
		}

		return fmt::sprintf("0x%08x", hash);
	}

private:
	std::map<uint32_t, std::string_view> m_lookupList;
};

static std::map<uint32_t, atPoolBase*> g_pools;
static std::map<atPoolBase*, uint32_t> g_inversePools;

static const char* poolEntriesTable[] = {
"animatedbuilding",
"attachmentextension",
"audioheap",
"audscene",
"auddynmixpatch",
"auddynmixpatchsettings"
"avoidancevolumes",
"cbirdcurvecontainer",
"blendshapestore",
"building",
"ccarryconfigtargetclipsetrequester",
"ctimedcarryconfigtargetclipsetrequester",
"cactionconfiginfo",
"caimsolver",
"caimsolver::internalstate",
"cambientmaskvolume",
"cambientmaskvolumeentity",
"cambientmaskvolumedoor",
"carmsolver",
"clegposturesolver",
"carmposturesolver",
"cimpulsereactionsolver",
"cmountedlegsolver",
"cmountedlegsolverproxy",
"carmiksolver",
"cbalancesolver",
"cclimbsolver",
"cbodylookiksolver",
"cbodylookiksolver::internalstate",
"cbodylookiksolverproxy",
"cbodyrecoiliksolver",
"clegiksolver",
"clegiksolverstate",
"clegiksolverproxy",
"cquadlegiksolver",
"cquadlegiksolverproxy",
"crootslopefixupiksolver",
"cstirrupsolver",
"ccontoursolver",
"ccontoursolverproxy",
"ctorsoreactiksolver",
"ctorsovehicleiksolver",
"ctorsovehicleiksolverproxy",
"cupperbodyblend",
"cupperbodyblend::cbodyblendbonecache",
"cbodyaimsolver",
"cbodyaimsolver::internalstate",
"ctwobonesolver",
"cposefixupsolver",
"cvehicleturretsolver",
"cquadrupedinclinesolver",
"cquadrupedinclinesolverproxy",
"cfullbodysolver",
"cgamescripthandler",
"chighheelsolver",
"cbodydampingsolver",
"cbodyreachsolver",
"canimalattackgroup",
"canimalflock",
"canimaltargeting",
"canimaltuning",
"canimalunalertedgroup",
"canimalgroup",
"canimalgroupmember",
"cflocktuning",
"cpopzonespawner",
"cmodelsetspawner",
"animscenestore",
"carrec",
"cvehiclecombatavoidancearea",
"ccargen",
"ccombatdirector",
"cpedsharedtargeting",
"ccombatinfo",
"ccombatsituation",
"cbarbrawler",
"ccoverfinder",
"caimhelper",
"cinventoryitem",
"cweaponcomponentitem",
"cweaponitem",
"cammoitem",
"csatchelitem",
"cclothingitem",
"chorseinventoryitem",
"ccharacteritem",
"chorseequipmentinventoryitem",
"ccoachinventoryitem",
"ccampitem",
"cdogitem",
"ccrimeobserver",
"cscenariorequest",
"cscenariorequesthandler",
"cscenariorequestresults",
"cscenarioinfo",
"ctacticalanalysis",
"ctaskusescenarioentityextension",
"cterrainadaptationhelper",
"canimscenehelper",
"animstore",
"clipstore",
"cgamescriptresource",
"clothstore",
"ccombatmeleegroup",
"combatmountedmanager_attacks",
"compentity",
"cpedinventory",
"persistentlootabledata",
"lootactionfinderfsm",
"managedlootableentitydata",
"carryactionfinderfsm",
"motionstore",
"compositelootableentitydefinst",
"carriableextension",
"centitygameinfocomponent",
"cemotionallocohelper",
"cgameownership",
"fwanimationcomponent",
"canimationcomponent",
"cdynamicentityanimationcomponent",
"cobjectanimationcomponent",
"cpedanimationcomponent",
"cvehicleanimationcomponent",
"canimatedbuildinganimationcomponent",
"curvelib::curve",
"fwcreaturecomponent",
"fwanimdirectorcomponentcreature",
"fwanimdirectorcomponentmotiontree",
"fwanimdirectorcomponentmove",
"fwanimdirectorcomponentfacialrig",
"fwanimdirectorcomponentcharactercreator",
"fwanimdirectorcomponentextraoutputs",
"fwanimdirectorcomponentpose",
"fwanimdirectorcomponentexpressions",
"fwanimdirectorcomponentragdoll",
"fwanimdirectorcomponentparent",
"fwanimdirectorcomponentparent_parent",
"fwanimdirectorcomponentreplay",
"canimdirectorcomponentik",
"fwanimdirector",
"crcreaturecomponentskeleton",
"crcreaturecomponentextradofs",
"crcreaturecomponentexternaldofs",
"crcreaturecomponentcloth",
"crcreaturecomponentshadervars",
"crcreaturecomponentphysical",
"crcreaturecomponenthistory",
"crcreaturecomponentparticleeffect",
"crexpressionplayer",
"crrelocatableheapsize",
"crrelocatableasynccompactsize",
"crrelocatablemapslots",
"crframeacceleratorheapsize",
"crframeacceleratormapslots",
"crframeacceleratorentryheaders",
"crweightsetacceleratorheapsize",
"crweightsetacceleratormapslots",
"crweightsetacceleratorentryheaders",
"mvpagebuffersize",
"crmtnodefactorypool",
"crikheap",
"fwmtupdatescheduleroperation",
"cpedcreaturecomponent",
"cpedanimalcomponent",
"cpedanimalearscomponent",
"cpedanimaltailcomponent",
"cpedanimalaudiocomponent",
"cpedavoidancecomponent",
"cpedbreathecomponent",
"cpedclothcomponent",
"cpedcorecomponent",
"cpeddamagemodifiercomponent",
"cpedmeleemodifiercomponent",
"cpeddrivingcomponent",
"cpedfacialcomponent",
"cpedfootstepcomponent",
"cpedgameplaycomponent",
"cpedattributecomponent",
"cpeddistractioncomponent",
"cpeddummycomponent",
"cpedgraphicscomponent",
"cpedvfxcomponent",
"cpedhealthcomponent",
"cpedhorsecomponent",
"cpedhumanaudiocomponent",
"cpedeventcomponent",
"cpedintelligencecomponent",
"cpedinventorycomponent",
"cpedlookatcomponent",
"cpedmotioncomponent",
"cpedmotivationcomponent",
"cpedphysicscomponent",
"cpedprojdecalcomponent",
"cpedragdollcomponent",
"cpedscriptdatacomponent",
"cpedstaminacomponent",
"cpedtargetingcomponent",
"cpedthreatresponsecomponent",
"cpedtransportcomponent",
"cpedtransportusercomponent",
"cpedweaponcomponent",
"cpedweaponmanagercomponent",
"cpedvisibilitycomponent",
"cobjectautostartanimcomponent",
"cobjectbreakableglasscomponent",
"cobjectbuoyancymodecomponent",
"cobjectcollisiondetectedcomponent",
"cobjectcollisioneffectscomponent",
"cobjectdoorcomponent",
"cobjectdraftvehiclewheelcomponent",
"cobjectintelligencecomponent",
"cobjectnetworkcomponent",
"cobjectphysicscomponent",
"cobjectriverprobesubmissioncomponent",
"maxriverphysicsinsts",
"cobjectvehicleparentdeletedcomponent",
"cobjectweaponscomponent",
"cpairedanimationreservationcomponent",
"cvehiclecorecomponent",
"cvehicledrivingcomponent",
"cvehicleintelligencecomponent",
"cvehiclephysicscomponent",
"cvehicleweaponscomponent",
"cpickupdata",
"cprioritizedsetrequest",
"cprioritizeddictionaryrequest",
"cquadrupedreactsolver",
"croadblock",
"cstuntjump",
"csimulatedroutemanager::route",
"csquad",
"cutscenestore",
"cscriptentityextension",
"cscriptentityidextension",
"cvehiclechasedirector",
"cvehiclecliprequesthelper",
"cvolumelocationextension",
"cvolumeownerextension",
"cmeleecliprequesthelper",
"cgrapplecliprequesthelper",
"cactioncache",
"cgrabhelper",
"cfleedecision",
"cpointgunhelper",
"cthreatenedhelper",
"cstunthelper",
"cgpsnumnodesstored",
"cclimbhandholddetected",
"cambientflockspawncontainer",
"customshadereffectbatchtype",
"customshadereffectbatchslodtype",
"customshadereffectcommontype",
"customshadereffectgrasstype",
"customshadereffecttreetype",
"cwildlifespawnrequest",
"decorator",
"decoratorextension",
"drawablestore",
"dummy object",
"cpropsetobjectextension",
"cmapentityrequest",
"cmapdatareference",
"cpinmapdataextension",
"dwdstore",
"entitybatch",
"grassbatch",
"entitybatchbitset",
"tcbox",
"tcvolume",
"exprdictstore",
"flocksperpopulationzone",
"framefilterstore",
"fragmentstore",
"fwscriptguid",
"fwuianimationopbase",
"fwuianimationopinstancedatabase",
"fwuianimationtargetbase",
"fwuianimationvaluebase",
"fwuiblip",
"layoutnode",
"fwuivisualpromptdata",
"fwuiiconhandle",
"fwcontainerlod",
"gameplayerbroadcastdatahandler_remote",
"ccontainedobjectid",
"ccontainedobjectidsinfo",
"interiorinst",
"instancebuffer",
"interiorproxy",
"iplstore",
"itemset",
"itemsetbuffer",
"jointlimitdictstore",
"lastinstmatrices",
"bodydatadictstore",
"behaviordatadictstore",
"cpersistentcharacterinfo",
"cpersistentcharactergroupinfo",
"cpersistentcharacter",
"cperschargroup",
"cperscharhorse",
"cperscharvehicle",
"canimalattractor",
"canimaldispatch",
"cnavobstructionpath",
"volcylinder",
"volbox",
"volsphere",
"volaggregate",
"volnetdatastateprimitive",
"volnetdatastateaggregate",
"maxloadedinfo",
"maxloadrequestedinfo",
"activeloadedinfo",
"activepersistentloadedinfo",
"maxmanagedrequests",
"maxunguardedrequests",
"maxreleaserefs",
"known refs",
"clightentity",
"mapdataloadednode",
"mapdatastore",
"maptypesstore",
"metadatastore",
"navmeshes",
"netscriptserialisationplan_small",
"netscriptserialisationplan_large",
"netscriptserialisationplan_extralarge",
"networkdefstore",
"networkcrewdatamgr",
"networkscriptstatusmanager",
"object",
"naspeechinst",
"navocalization",
"fwactivemanagedwaveslotinterface",
"nafoliageentity",
"nafoliagecontactevent",
"objectdependencies",
"occlusioninteriorinfo",
"occlusionpathnode",
"occlusionportalentity",
"occlusionportalinfo",
"peds",
"cpedequippedweapon",
"pedroute",
"cweapon",
"cweaponcomponent",
"cweaponcomponentinfo",
"phinstgta",
"fragcacheentriesprops",
"fragcacheheadroom",
"maxclothcount",
"maxcachedropecount",
"maxropecount",
"maxvisibleclothcount",
"maxpresimdependency",
"maxsinglethreadedphysicscallbacks",
"maxsinglethreadedphysicscallbacks",
"worldupdateentities",
"maxfoliagecollisions",
"fraginstgta",
"physicsbounds",
"maxbroadphasepairs",
"cpickup",
"cpickupplacement",
"cpickupplacementcustomscriptdata",
"cregenerationinfo",
"portalinst",
"posematcherstore",
"pmstore",
"ptfxsortedentity",
"ptfxassetstore",
"quadtreenodes",
"scaleformstore",
"scaleformmgrarray",
"scriptbrains",
"scriptstore",
"srequest",
"staticbounds",
"cremotetaskdata",
"textstore",
"txdstore",
"vehicles",
"vehiclestreamrequest",
"vehiclestreamrender",
"vehiclestruct",
"handlingdata",
"wptrec",
"fwlodnode",
"ctask",
"ctasknetworkcomponent",
"cevent",
"cmoveobject",
"cmoveanimatedbuilding",
"atdscriptobjectnode",
"cremotescriptargs",
"fwdynamicarchetypecomponent",
"fwdynamicentitycomponent",
"fwentitycontainer",
"fwmatrixtransform",
"fwquaterniontransform",
"fwsimpletransform",
"scenariopointsandedgesperregion",
"scenariopointentity",
"scenariopointworld",
"maxnonregionscenariopointspatialobjects",
"maxtrainscenariopoints",
"maxscenarioprompts",
"maxscenariointeriornames",
"objectintelligence",
"vehiclescenarioattractors",
"aircraftflames",
"crelationshipgroup",
"cscenariopoint",
"cscenariopointchainuseinfo",
"cscenarioclusterspawnedtrackingdata",
"cspclusterfsmwrapper",
"cgroupscenario",
"fwarchetypepooledmap",
"ctaskconversationhelper",
"syncedscenes",
"animscenes",
"cpropmanagementhelper",
"cpropinstancehelper",
"cscenariopropmanager::pendingpropinfo",
"cscenariopropmanager::loadedpropinfo",
"cscenariopropmanager::uprootedpropinfo",
"cscenariopropmanager::activeschedule",
"cgamescripthandlernetwork",
"navmeshroute",
"ccustommodelboundsmappings::cmapping",
"ladderentities",
"stairsentities",
"cqueriabletaskinfo",
"cguidcomponent",
"chealthcomponent",
"cavoidancecomponent",
"cprojdecalcomponent",
"clightcomponent",
"clightshaftcomponent",
"ctransportcomponent",
"ckinematiccomponent",
"csubscriberentitycomponent",
"cportablecomponent",
"cunlock",
"clightgroupextensioncomponent",
"clightshaftextensioncomponent",
"cladderinfo",
"cladderinfoextensioncomponent",
"cfakedoorinfo",
"cfakedoorextension::fakedoorinfo",
"cfakedoorgroupextensioncomponent",
"cfakedoorextension",
"caudiocollisionextensioncomponent",
"caudioemitter",
"caudioeffectextensioncomponent",
"cfragobjectanimextensioncomponent",
"cprocobjattr",
"cprocobjectextensioncomponent",
"cswayableattr",
"cswayableextensioncomponent",
"cbuoyancyextensioncomponent",
"cexpressionextensioncomponent",
"cwinddisturbanceextensioncomponent",
"cstairsextension",
"cstairsextensioncomponent",
"cdecalattr",
"cdecalextensioncomponent",
"cobjectautostartanimextensioncomponent",
"cexplosionattr",
"cexplosionextensioncomponent",
"cobjectlinksextensioncomponent",
"cparticleattr",
"cparticleextensioncomponent",
"camvolumeextensioncomponent",
"cfogvolumeextensioncomponent",
"clothmanagerheapsize",
"scrglobals",
"orders",
"incidents",
"caicurvepoint",
"fraginstgta",
"ctasksequencelist",
"cvehiclestreamrequestgfx",
"chandlingobject",
"cpatrollink",
"cpatrolnode",
"pointroute",
"cspawnpointoverrideextension",
"ccollectioninfo",
"wheels",
"vehicleglasscomponententity",
"naenvironmentgroup",
"cbullet",
"cbullet::sbulletinstance",
"itemsetbuffer",
"itemset",
"tasksequenceinfo",
"ceventnetwork",
"scriptshapetestresult",
"cnamedpatrolroute",
"explosiontype",
"camsplinenode",
"clandinggear_qkdfqq",
"collision_5plvhjd",
"shapetesttaskdata",
"scenariopoint",
"ceventdecisionmaker",
"musicevent",
"musicaction",
"ccargenforscenarios",
"vehicleaudioentity",
#include "RDR3VtableList.h"
};

static RageHashList poolEntries(poolEntriesTable);

GTA_CORE_EXPORT atPoolBase* rage::GetPoolBase(uint32_t hash)
{
	auto it = g_pools.find(hash);

	if (it == g_pools.end())
	{
		return nullptr;
	}

	return it->second;
}

static atPoolBase* SetPoolFn(atPoolBase* pool, uint32_t hash)
{
	g_pools[hash] = pool;
	g_inversePools.insert({ pool, hash });

	return pool;
}

static void(*g_origPoolDtor)(atPoolBase*);

static void PoolDtorWrap(atPoolBase* pool)
{
	auto hashIt = g_inversePools.find(pool);

	if (hashIt != g_inversePools.end())
	{
		auto hash = hashIt->second;

		g_pools.erase(hash);
		g_inversePools.erase(pool);
	}

	return g_origPoolDtor(pool);
}

static void* (*g_origPoolAllocate)(atPoolBase*, uint64_t);

static void* PoolAllocateWrap(atPoolBase* pool, uint64_t unk)
{
	void* value = g_origPoolAllocate(pool, unk);

	if (!value)
	{
		auto it = g_inversePools.find(pool);
		std::string poolName = "<<unknown pool>>";

		if (it != g_inversePools.end())
		{
			uint32_t poolHash = it->second;

			poolName = poolEntries.LookupHash(poolHash);
		}

		AddCrashometry("pool_error", "%s (%d)", poolName, pool->GetSize());

		std::string extraWarning = (poolName.find("0x") == std::string::npos)
			? fmt::sprintf(" (you need to raise %s PoolSize in common/data/gameconfig.xml)", poolName)
			: "";

		FatalErrorNoExcept("%s Pool Full, Size == %d%s", poolName, pool->GetSize(), extraWarning);
	}

	return value;
}

static hook::cdecl_stub<void(atPoolBase*)> poolRelease([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? BA 88 01 00 00 48 8B CB E8", 13));
});

namespace rage
{
	GTA_CORE_EXPORT void* PoolAllocate(atPoolBase* pool)
	{
		return PoolAllocateWrap(pool, 0);
	}

	GTA_CORE_EXPORT void PoolRelease(atPoolBase* pool)
	{
		return poolRelease(pool);
	}
}

static hook::cdecl_stub<void()> _loadStreamingFiles([]()
{
	return hook::get_pattern("C7 85 78 02 00 00 61 00 00 00 41 BE", -0x28);
});

void (*g_origLevelLoad)(const char* r);

void WrapLevelLoad(const char* r)
{
	_loadStreamingFiles();

	g_origLevelLoad(r);
}

static HookFunction hookFunction([]()
{
	auto registerPools = [](hook::pattern& patternMatch, int callOffset, int hashOffset)
	{
		for (size_t i = 0; i < patternMatch.size(); i++)
		{
			auto match = patternMatch.get(i);
			auto hash = *match.get<uint32_t>(hashOffset);

			struct : jitasm::Frontend
			{
				uint32_t hash;
				uint64_t origFn;

				void InternalMain() override
				{
					sub(rsp, 0x38);

					mov(rax, qword_ptr[rsp + 0x38 + 0x28]);
					mov(qword_ptr[rsp + 0x20], rax);

					mov(rax, qword_ptr[rsp + 0x38 + 0x30]);
					mov(qword_ptr[rsp + 0x28], rax);

					mov(rax, origFn);
					call(rax);

					mov(rcx, rax);
					mov(edx, hash);

					mov(rax, (uint64_t)&SetPoolFn);
					call(rax);

					add(rsp, 0x38);

					ret();
				}
			}*stub = new std::remove_pointer_t<decltype(stub)>();

			stub->hash = hash;

			auto call = match.get<void>(callOffset);
			hook::set_call(&stub->origFn, call);
			hook::call(call, stub->GetCode());
		}
	};

	// find initial pools
	registerPools(hook::pattern("BA ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 8B D8 E8"), 51, 1);
	registerPools(hook::pattern("BA ? ? ? ? E8 ? ? ? ? 8B D8 E8 ? ? ? ? 48 89 44 24 28 4C 8D 05 ? ? ? ? 44 8B CD"), 41, 1);
	registerPools(hook::pattern("BA ? ? ? ? E8 ? ? ? ? 8B D8 E8 ? ? ? ? 48 89 44 24 28 4C 8D 05 ? ? ? ? 44 8B CE"), 45, 1);

	// no-op assertation to ensure our pool crash reporting is used instead
	hook::nop(hook::get_pattern("83 C9 FF BA EF 4F 91 02 E8", 8), 5);

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("4C 63 41 1C 4C 8B D1 49 3B D0 76", -4), PoolAllocateWrap, (void**)&g_origPoolAllocate);
	MH_CreateHook(hook::get_pattern("8B 41 28 A9 00 00 00 C0 74", -15), PoolDtorWrap, (void**)&g_origPoolDtor);
	MH_EnableHook(MH_ALL_HOOKS);

	// raw sfe reg from non-startup
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("48 8B D8 48 85 C0 75 26 8D 50 5C", -0x38), WrapLevelLoad, (void**)&g_origLevelLoad);
	MH_EnableHook(MH_ALL_HOOKS);
});
