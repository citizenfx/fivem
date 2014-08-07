#include "StdInc.h"
#include "BoundStreaming.h"
#include "sysAllocator.h"
#include "DrawCommands.h"
#include <unordered_set>
#include <mutex>

#pragma comment(lib, "d3d9.lib")

uint32_t WRAPPER NatHash(const char* str) { EAXJMP(0x7BDBF0); }

class CBaseModelInfo : public rage::sysUseAllocator
{
protected:
	int _f4;
	int _f8;
	char pad[48];
	uint32_t m_modelHash; // 60
	//char pad2[32]; // 64
	//char pad2[20]; // 64
	uint32_t m_usageFlags;
	uint32_t m_refCount;
	uint16_t m_txd; // 72
	uint16_t m_padW;
	char pad3[8];
	uint16_t m_drawblDict;
	uint16_t m_padW2;
	char pad4[8];

public:
	CBaseModelInfo();

	virtual ~CBaseModelInfo(); // 0

	virtual void Initialize(); // 4

	virtual void m_8(); // 8

	virtual void m_C(); // 12

	virtual void m_10(); // 16

	virtual void m_14(); // 20

	virtual void InstantiateInst(int instance);

	virtual void m_1C(); // 28

	virtual void RemoveInstance();

	void DoLodStuff(const char* lodName);

	inline bool HasField8() { return _f8 != 0; }

	inline void AddRef() { m_refCount++; }

	inline void ReleaseRef() { m_refCount--; }

	inline bool ShouldRelease() { return m_refCount == 0; }

	inline void SetModelName(const char* name) { m_modelHash = NatHash(name); }

	inline uint32_t GetModelHash() { return m_modelHash; }

	inline uint16_t GetTxd() { return m_txd; }

	inline uint16_t GetDrawblDict() { return m_drawblDict; }

	inline uint32_t GetRefCount() { return m_refCount; }

	inline bool HasInstance() { return (m_usageFlags & 8) != 0; }
};

class CTimeModelInfo : public CBaseModelInfo
{
private:
	void* m_timeDayModel;
	void* m_timeNightModel;
	char m_padTime[8];

public:
	CTimeModelInfo();
};

CBaseModelInfo::~CBaseModelInfo() { }
void CBaseModelInfo::Initialize() { __asm int 3 }
void CBaseModelInfo::m_8() { __asm int 3 }
void CBaseModelInfo::m_C() { __asm int 3 }
void CBaseModelInfo::m_10() { __asm int 3 }
void CBaseModelInfo::m_14() { __asm int 3 }
void CBaseModelInfo::InstantiateInst(int instance) { __asm int 3 }
void CBaseModelInfo::m_1C() { __asm int 3 }
void CBaseModelInfo::RemoveInstance() { __asm int 3 }

CBaseModelInfo::CBaseModelInfo()
{
	*(uint32_t*)this = 0xD7CD04;
	_f8 = 0;
}

CTimeModelInfo::CTimeModelInfo()
{
	*(uint32_t*)this = 0xD7D444;
	_f8 = 0;

	m_timeDayModel = this;
	m_timeNightModel = (void*)-1;
}

void WRAPPER CBaseModelInfo::DoLodStuff(const char*) { EAXJMP(0x98EE80); }

static std::unordered_map<uint32_t, std::shared_ptr<CBaseModelInfo>> g_modelInfos;
static std::unordered_map<uint32_t, std::shared_ptr<CBaseModelInfo>> g_modelInfoIdxTable;

static int miInt = 0;

void PostProcessCustomMIs()
{
	for (auto& pair : g_modelInfos)
	{
		auto& mi = pair.second;
		auto miIdx = (char*)mi.get();

		if (*(uint16_t*)(miIdx + 84) != 0xFFFF)
		{
			// not noted: finding a .lod file; these aren't used in retail IV I'd assume
			// ^ i'm wrong, these are some metatype used to allow streaming to request these MIs and get something sensible called in return

			//mi->DoLodStuff("NULL");
		}

		// not shown: type == 4 check (weapon)
		//}
	}

	/*if (((int(*)(const char*, int, int))0x832EA0)("unknown", 30999, *(int*)0x15F73AC) == 65535)
	{
		//__asm int 3
	}*/
}

static int g_modelInfosSaved;

CBaseModelInfo* AllocModelInfoForModel(const char* name)
{
	// hack: get the lod name from the name address because we know the caller
	const char* lodName = name + 48;

	((CBaseModelInfo**)0x15F73B0)[30999] = (CBaseModelInfo*)0xCA3EBA3E;

	// if it's not null...
	if (_stricmp(lodName, "null"))
	{
		std::shared_ptr<CBaseModelInfo> mi(new CBaseModelInfo());
		g_modelInfos[NatHash(name)] = mi;

		mi->Initialize();
		mi->SetModelName(name);

		g_modelInfosSaved++;

		return mi.get();
	}

	return ((CBaseModelInfo*(*)(const char*))0x98AC60)(name);
}

CBaseModelInfo* AllocTimeModelInfoForModel(const char* name)
{
	// hack: get the lod name from the name address because we know the caller
	const char* lodName = name + 48;

	// if it's not null...
	if (_stricmp(lodName, "null"))
	{
		std::shared_ptr<CBaseModelInfo> mi(new CTimeModelInfo());
		g_modelInfos[NatHash(name)] = mi;

		mi->Initialize();
		mi->SetModelName(name);

		g_modelInfosSaved++;

		return mi.get();
	}

	return ((CBaseModelInfo*(*)(const char*))0x98AD40)(name);
}

static std::unordered_multimap<int, CBaseModelInfo*> m_dependencyDrawableDicts;

CBaseModelInfo* GetModelInfo(uint32_t nameHash, int* mIdx)
{
	auto it = g_modelInfos.find(nameHash);

	if (it == g_modelInfos.end())
	{
		CBaseModelInfo* info = ((CBaseModelInfo*(*)(uint32_t, int*))0x98AAE0)(nameHash, mIdx);

		if (nameHash == HashRageString("parkgates_mh06"))
		{
//			__asm int 3
		}

		return info;
	}

	*mIdx = nameHash | 0xF0000000;

	auto indIt = g_modelInfoIdxTable.find(*mIdx);

	if (indIt == g_modelInfoIdxTable.end())
	{
		m_dependencyDrawableDicts.insert(std::make_pair(it->second->GetDrawblDict(), it->second.get()));
		g_modelInfoIdxTable.insert(std::make_pair(*mIdx, it->second));
	}

	return it->second.get();
}

class CEntity
{
private:
public:
	char pad[40];
	uint16_t RandomSeed; // ? // +40
	uint16_t m_nModelIndex; // +42
	char pad2[8]; // +44
	void* m_pInstance; // +52

	virtual ~CEntity() = 0;

	virtual void m4() = 0;
	virtual void m8() = 0;
	virtual void mC() = 0;
	virtual void m10() = 0;
	virtual void m14() = 0;
	virtual void m18() = 0;
	virtual void m1C() = 0;
	virtual void m20() = 0;
	virtual void m24() = 0;
	virtual void m28() = 0;
	virtual void m2C() = 0;
	virtual void m30() = 0;
	virtual void m34() = 0;
	virtual void m38() = 0;
	virtual void m3C() = 0;
	virtual void m40() = 0;
	virtual void DestroyModel() = 0;
};

struct CEntityExt
{
	CBaseModelInfo* modelInfo;
};

// TODO: allow attaching arbitrary components to entities
static std::unordered_map<CEntity*, CEntityExt> g_entityExtensions;

CBaseModelInfo* SetModelIndexHook(CEntity* entity, int modelIdx)
{
	if (modelIdx & 0xF0000000)
	{
		auto modelInfo = g_modelInfoIdxTable[modelIdx];

		if (modelInfo->GetModelHash() == 0x1b179f94)
		{
			//__asm int 3
		}

		entity->m_nModelIndex = 30999;

		g_entityExtensions[entity].modelInfo = modelInfo.get();

		return modelInfo.get();
	}
	else
	{
		CBaseModelInfo* modelInfo = ((CBaseModelInfo**)0x15F73B0)[modelIdx];

		entity->m_nModelIndex = modelIdx;

		return modelInfo;
	}
}

struct EntityRequest
{
	uint16_t txd;
	uint16_t drawblDict;
	CBaseModelInfo* modelInfo;
};

static std::vector<EntityRequest> m_requestList;
static std::unordered_set<CBaseModelInfo*> m_requestExistenceSet;
static std::unordered_multimap<int, CEntity*> m_dependencyDictEnts;
static std::unordered_set<CBaseModelInfo*> g_primedMIs;

static void WRAPPER RequestModel(int mIdx, int fileType, int priority) { EAXJMP(0x832C40); }
static bool WRAPPER HasModelLoaded(int mIdx, int fileType) { EAXJMP(0x832DD0); }

std::string GetStreamName(int idx, int ftype);

static std::vector<std::function<void()>> g_deferFuncs;

static void DeferToNextFrame(std::function<void()> func)
{
	g_deferFuncs.push_back(func);
}

int RequestEntityModel(CEntity* entity)
{
	auto entityExt = g_entityExtensions[entity];

	uint16_t txd = entityExt.modelInfo->GetTxd();
	uint16_t drawblDict = entityExt.modelInfo->GetDrawblDict();

	assert(drawblDict != 0xFFFF);

	if (txd != 0xFFFF)
	{
		RequestModel(txd, *(int*)0xF1CD84, 0);
	}

	if (drawblDict != 0xFFFF)
	{
		RequestModel(drawblDict, *(int*)0xF272E4, 0);
	}

	if (m_requestExistenceSet.find(entityExt.modelInfo) == m_requestExistenceSet.end())
	{
		//trace("request 0x%08x (drawable dict %s)\n", entityExt.modelInfo->GetModelHash(), GetStreamName(drawblDict, *(int*)0xF272E4).c_str());

		EntityRequest req;
		req.txd = txd;
		req.drawblDict = drawblDict;
		req.modelInfo = entityExt.modelInfo;
		m_requestList.push_back(req);
		m_requestExistenceSet.insert(entityExt.modelInfo);

		m_dependencyDictEnts.insert(std::make_pair(drawblDict, entity));
	}

	return 1;
}

void ProcessEntityRequests()
{
	// deferred stuff
	for (auto& defer : g_deferFuncs)
	{
		defer();
	}

	g_deferFuncs.clear();

	// process deletion (this is before the other list as MIs may not have been AddRef'd)
	for (auto& pair : g_modelInfos)
	{
		auto mi = pair.second;

		if (mi->ShouldRelease())
		{
			if (g_primedMIs.find(mi.get()) != g_primedMIs.end())
			{
				//trace("removing reference to drawable dict %s as model info 0x%08x is unused\n", GetStreamName(mi->GetDrawblDict(), *(int*)0xF272E4).c_str(), mi->GetModelHash());
				((void(*)(int))0x907940)(mi->GetDrawblDict());

				g_primedMIs.erase(mi.get());
			}
		}
	}

	// and so on
	for (auto it = m_requestList.begin(); it != m_requestList.end(); )
	{
		auto req = *it;

		bool done = false;

		if (req.txd == -1 || HasModelLoaded(req.txd, *(int*)0xF1CD84))
		{
			if (req.drawblDict == -1 || HasModelLoaded(req.drawblDict, *(int*)0xF272E4))
			{
				done = true;
			}
		}

		if (done)
		{
			void* drawblDict = ((void*(*)(uint32_t))0x422070)(req.drawblDict);
			int inst = ((int(__thiscall*)(void*, uint32_t))0x908F80)(drawblDict, req.modelInfo->GetModelHash());

			// add reference to drawable dict
			((void(*)(int))0x907910)(req.drawblDict);

			g_primedMIs.insert(req.modelInfo);

			if (inst)
			{
				req.modelInfo->InstantiateInst(inst);

				//trace("load 0x%08x (drawable dict %s)\n", req.modelInfo->GetModelHash(), GetStreamName(req.drawblDict, *(int*)0xF272E4).c_str());

				m_requestExistenceSet.erase(req.modelInfo);
				it = m_requestList.erase(it);
			}
			else
			{
				//trace("load 0x%08x failed? (drawable dict %s)\n", req.modelInfo->GetModelHash(), GetStreamName(req.drawblDict, *(int*)0xF272E4).c_str());

				it++;
			}
		}
		else
		{
			if (req.txd != 0xFFFF)
			{
				RequestModel(req.txd, *(int*)0xF1CD84, 0);
			}

			if (req.drawblDict != 0xFFFF)
			{
				RequestModel(req.drawblDict, *(int*)0xF272E4, 0);
			}

			it++;
		}
	}

	BoundStreaming::Process();
}

int WRAPPER DrawblDictGetUsage(int dict) { EAXJMP(0x4220A0); }

auto ReleaseStreamingObjectNow = (void (__thiscall*)(void* streaming, int objectNum))0xBCCB20;

int GetTypeStart(int type);
int GetTypeEnd(int type);

int DrawableDictStoreGetUsageWrap(int dict)
{
	int usage = DrawblDictGetUsage(dict);

	if (usage == 0 && _ReturnAddress() == (void*)0xBCC153)
	{
		bool deferred = false;

		auto pair = m_dependencyDrawableDicts.equal_range(dict);

		for (auto it = pair.first; it != pair.second; it++)
		{
			if (it->second->HasInstance())
			{
				if (!it->second->ShouldRelease())
				{
					//trace("model info for %s (0x%08x) still has %i references!\n", GetStreamName(dict, *(int*)0xF272E4).c_str(), it->second->GetModelHash(), it->second->GetRefCount());

					return 1;
				}

				it->second->RemoveInstance();

				deferred = true;
			}
		}

		//m_dependencyDrawableDicts.erase(dict);

		auto entPair = m_dependencyDictEnts.equal_range(dict);

		for (auto it = entPair.first; it != entPair.second; it++)
		{
			it->second->DestroyModel();

			deferred = true;
		}

		m_dependencyDictEnts.erase(dict);

		if (deferred)
		{
			//trace("pre-deferring drawable destruction for %s\n", GetStreamName(dict, *(int*)0xF272E4).c_str());

			DeferToNextFrame([=] ()
			{
				//trace("executing pre-deferred drawable destruction for %s\n", GetStreamName(dict, *(int*)0xF272E4).c_str());

				//ReleaseDrawblDict(dict);
				ReleaseStreamingObjectNow((void*)0xF21C60, dict + GetTypeStart(*(int*)0xF272E4));
			});

			return 1;
		}
	}

	return usage;
}

void PreFullReleaseModel(int streamingIdx)
{
	int drawblDict = *(int*)0xF272E4;

	if (streamingIdx >= GetTypeStart(drawblDict) && streamingIdx <= GetTypeEnd(drawblDict))
	{
		int dict = streamingIdx - GetTypeStart(drawblDict);

		auto pair = m_dependencyDrawableDicts.equal_range(dict);

		for (auto it = pair.first; it != pair.second; it++)
		{
			if (it->second->HasInstance())
			{
				//trace("releasing dict %s (0x%08x) for a full release because *reasons*\n", GetStreamName(dict, *(int*)0xF272E4).c_str(), it->second->GetModelHash());

				it->second->RemoveInstance();
			}
		}

		//m_dependencyDrawableDicts.erase(dict);

		auto entPair = m_dependencyDictEnts.equal_range(dict);

		for (auto it = entPair.first; it != entPair.second; it++)
		{
			//trace("also destroying model for dict %s\n", GetStreamName(dict, *(int*)0xF272E4).c_str());

			it->second->DestroyModel();
		}

		m_dependencyDictEnts.erase(dict);
	}
}

void WRAPPER ReleaseDrawblDict(int dict) { EAXJMP(0x907750); }

void ReleaseDrawblDictWrap(int dict)
{
	//if (!deferred)
	//{
		//trace("not deferring drawable destruction for %s\n", GetStreamName(dict, *(int*)0xF272E4).c_str());

		ReleaseDrawblDict(dict);
	/*}
	else
	{
		trace("deferring drawable destruction for %s\n", GetStreamName(dict, *(int*)0xF272E4).c_str());

		DeferToNextFrame([=] ()
		{
			trace("executing deferred drawable destruction for %s\n", GetStreamName(dict, *(int*)0xF272E4).c_str());

			ReleaseDrawblDict(dict);
		});
	}*/
}

void __declspec(naked) SetModelIndexStub()
{
	__asm
	{
		push ebx
		push esi
		call SetModelIndexHook
		add esp, 8h

		mov edi, eax

		retn
	}
}

CBaseModelInfo* GetModelInfoForEntity(CEntity* entity)
{
	if (entity->m_nModelIndex == 0xFFFF)
	{
		trace("RETURNED FAKE MODELINFO\n");

		return ((CBaseModelInfo**)0x15F73B0)[1];
	}

	if (entity->m_nModelIndex >= 30999)
	{
		return g_entityExtensions[entity].modelInfo;
	}

	return ((CBaseModelInfo**)0x15F73B0)[entity->m_nModelIndex];
}

void __declspec(naked) RetModelInfoEaxEax()
{
	__asm
	{
		push ecx
		call GetModelInfoForEntity
		pop ecx

		retn
	}
}

void __declspec(naked) RetModelInfoEaxEcx()
{
	__asm
	{
		push ecx
		call GetModelInfoForEntity
		add esp, 4h

		mov ecx, eax

		retn
	}
}

void __declspec(naked) RetModelInfoEcxEax()
{
	__asm
	{
		push ecx
		call GetModelInfoForEntity
		pop ecx

		retn
	}
}

void __declspec(naked) RetModelInfoEdiEsi()
{
	__asm
	{
		push edi
		call GetModelInfoForEntity
		add esp, 4h

		mov esi, eax

		retn
	}
}

void __declspec(naked) RetModelInfoEsiEdi()
{
	__asm
	{
		push eax
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		mov edi, eax
		pop eax

		retn
	}
}

void __declspec(naked) RetModelInfoEbxEsi()
{
	__asm
	{
		push eax
		push ebx
		call GetModelInfoForEntity
		add esp, 4h

		mov esi, eax
		pop eax

		retn
	}
}

void IsThisMyEnemiend(CBaseModelInfo* enemy)
{
	if (enemy->GetModelHash() == 0x1b179f94)
	{
		//__asm int 3
	}
}

void __declspec(naked) RetModelInfoEsiEdiAddCmp()
{
	__asm
	{
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		push eax
		call IsThisMyEnemiend
		pop eax

		mov edi, eax

		cmp dword ptr [ebx + 10h], 0

		retn
	}
}

void __declspec(naked) RetModelInfoEsiEbx()
{
	__asm
	{
		push eax
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		mov ebx, eax
		pop eax

		retn
	}
}

void __declspec(naked) RetModelInfoEdiEbx()
{
	__asm
	{
		push eax
		push edi
		call GetModelInfoForEntity
		add esp, 4h

		mov ebx, eax
		pop eax

		retn
	}
}

void __declspec(naked) RetModelInfoEsiEcx()
{
	__asm
	{
		push eax
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		mov ecx, eax
		pop eax

		retn
	}
}

void __declspec(naked) RetModelInfoEdiEdx()
{
	__asm
	{
		push eax
		push edi
		call GetModelInfoForEntity
		add esp, 4h

		mov edx, eax
		pop eax

		retn
	}
}

void __declspec(naked) RetModelInfoEsiEbp()
{
	__asm
	{
		push edx
		push eax
		push ecx
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		mov ebp, eax
		pop ecx
		pop eax
		pop edx

		retn
	}
}

void __declspec(naked) RetModelInfoEsiEbpCmpEbx()
{
	__asm
	{
		push edx
		push eax
		push ecx
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		mov ebp, eax
		pop ecx
		pop eax
		pop edx

		cmp ebx, 0EFh

		retn
	}
}

void __declspec(naked) RetModelInfoEsiEdx()
{
	__asm
	{
		push eax
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		mov edx, eax
		pop eax

		retn
	}
}

void __declspec(naked) RetModelInfoEsiEdxAddCmp1()
{
	__asm
	{
		push eax
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		mov edx, eax
		pop eax

		cmp byte ptr ds:[1144648h], 0
		retn
	}
}

void __declspec(naked) RetModelInfoEsiEsi()
{
	__asm
	{
		push edx
		push ecx
		push eax
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		mov esi, eax
		pop eax
		pop ecx
		pop edx

		retn
	}
}

void __declspec(naked) RetModelInfoEdiEdi()
{
	__asm
	{
		push edx
		push ecx
		push eax
		push edi
		call GetModelInfoForEntity
		add esp, 4h

		mov edi, eax
		pop eax
		pop ecx // very demanding function as far as register saving goes
		pop edx // since when is edx a scratch register?

		retn
	}
}

void __declspec(naked) RetModelInfoEcxEsi()
{
	__asm
	{
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		mov ecx, eax

		retn
	}
}

void __declspec(naked) RetModelInfoEcxEsiAddCmp1()
{
	__asm
	{
		push edx
		push esi
		call GetModelInfoForEntity
		add esp, 4h

		mov ecx, eax
		pop edx

		cmp byte ptr [esi + 61h], 0

		retn
	}
}

void __declspec(naked) RetModelInfoEaxEdi()
{
	__asm
	{
		push edi
		call GetModelInfoForEntity
		add esp, 4h

		retn
	}
}

void __declspec(naked) RetModelInfoEsiEax()
{
	__asm
	{
		push ecx
		push esi
		call GetModelInfoForEntity
		add esp, 4h
		pop ecx

		retn
	}
}

void __declspec(naked) RetModelInfoEdiEcx()
{
	__asm
	{
		push ecx
		call GetModelInfoForEntity
		pop ecx

		mov edi, eax

		retn
	}
}

void __declspec(naked) RetModelInfoEcxEdi()
{
	__asm
	{
		push eax
		push edi
		call GetModelInfoForEntity
		add esp, 4h

		mov ecx, eax
		pop eax

		retn
	}
}

void __declspec(naked) RetModelInfoEbpEbx()
{
	__asm
	{
		push ebp
		call GetModelInfoForEntity
		add esp, 4h

		mov ebx, eax

		retn
	}
}

static CEntity* curEnt;

void __declspec(naked) DrawEntityDC_ctorHook()
{
	__asm
	{
		mov curEnt, esi

		push 7BFAA0h
		retn
	}
}

struct CDrawEntityDC
{
	char pad[48];
	uint16_t modelIdx;
	uint16_t pad2;
	char pad3[12];
	CBaseModelInfo* modelInfo;
};

void AddEntityRef(CEntity* entity);

void SetModelInfoAndSuch(CDrawEntityDC* dc)
{
	dc->modelInfo = g_entityExtensions[curEnt].modelInfo;

	AddEntityRef(curEnt);
}

void __declspec(naked) DrawEntityDC_dontCallIfInstance()
{
	__asm
	{
		cmp word ptr [esp + 4h], 07917h
		jl justGoAlong

		push ecx
		push esi
		call SetModelInfoAndSuch
		add esp, 4h

		//retn 4
		/*push curEnt
		call AddEntityRef
		add esp, 4h*/
		pop ecx

		/*push esi
		push edi
		mov edi, [esp + 8 + 4]
		mov esi, ecx

		push 7B7144h
		retn*/

		retn 4

	justGoAlong:
		push 7B7130h
		retn
	}
}

static bool inEvent;

CBaseModelInfo* GetModelInfoForDraw(CDrawEntityDC* dc)
{
	if (dc->modelIdx >= 0x7917)
	{
		inEvent = true;
		D3DPERF_BeginEvent(D3DCOLOR_ARGB(0, 0, 0, 0), va(L"draw mi %x", dc->modelInfo->GetModelHash()));

		return dc->modelInfo;
	}

	return ((CBaseModelInfo**)0x15F73B0)[dc->modelIdx];
}

void DrawEntityDCTail()
{
	if (inEvent)
	{
		D3DPERF_EndEvent();
		inEvent = false;
	}
}

static void __declspec(naked) RetModelInfoDraw()
{
	__asm
	{
		push esi
		call GetModelInfoForDraw
		add esp, 4h

		retn
	}
}

template<int Value>
int ReturnInt()
{
	return Value;
}

void DebugRequest(int mi)
{
	//if (mi == 30999)
	if (mi == 0x3001)
	{
		//__asm int 3
	}
}

void DebugRequest2(int mi, int type)
{
	//if (mi == 30999)
	if (type == *(int*)0x15F73A0)
	{
		auto minf = ((CBaseModelInfo**)0x15F73B0)[mi];

		if (minf->GetDrawblDict() != 0xFFFF)
		{
			trace("requesting model instance for drawbldict %s (mi %p - 0x%08x)\n", GetStreamName(minf->GetDrawblDict(), *(int*)0xF272E4).c_str(), minf, minf->GetModelHash());
		}
	}
}

void DebugLoaded(int mi)
{
	if (mi == 30999)
	//if (mi == 0x3001)
	{
		__asm int 3
	}
}

void __declspec(naked) RequestModelStub()
{
	__asm
	{
		push dword ptr [esp + 8]
		push dword ptr [esp + 4 + 4]
		call DebugRequest2
		add esp, 8h

		mov eax, [esp + 8]
		mov ecx, [esp + 0Ch]

		push 832C48h
		retn
	}
}

void __declspec(naked) ReleaseModelStub()
{
	__asm
	{
		push dword ptr [esp + 4]
		call DebugRequest
		add esp, 4h

		mov eax, [esp + 8h]
		imul eax, 64h

		push 832C77h
		retn
	}
}

void __declspec(naked) IsModelLoadedStub()
{
	__asm
	{
		push dword ptr[esp + 4]
		call DebugLoaded
		add esp, 4h

		mov eax, [esp + 8h]
		mov edx, dword ptr ds:[0F21C60h]

		push 832DDAh
		retn
	}
}

void __declspec(naked) RequestEntityModelEsi()
{
	__asm
	{
		cmp word ptr [esi + 2Eh], 7917h
		jl justDo

		push esi
		call RequestEntityModel
		add esp, 4h

		retn

	justDo:
		push 832C40h
		retn
	}
}

static CBaseModelInfo* g_removeTheseRefs[4096];
static int g_removeRefIdx;
static std::mutex g_removeRefMutex;

void AddEntityRef(CEntity* entity)
{
	// TODO: cleanup (0x7B7240)
	auto modelInfo = g_entityExtensions[entity].modelInfo;

	if (modelInfo)
	{
		modelInfo->AddRef();

		g_removeRefMutex.lock();
		g_removeTheseRefs[g_removeRefIdx++] = modelInfo;
		g_removeRefMutex.unlock();
	}
	else
	{
		// how the fuck did this get here?!
		__asm int 3
	}
}

void ReleaseRenderModels()
{
	g_removeRefMutex.lock();

	for (int i = 0; i < g_removeRefIdx; i++)
	{
		g_removeTheseRefs[i]->ReleaseRef();
	}
	
	g_removeRefIdx = 0;

	g_removeRefMutex.unlock();
}

void __declspec(naked) AddModelRefEsi()
{
	__asm
	{
		cmp word ptr [esi + 2Eh], 7917h
		jl justDo

		push ecx
		push esi
		call AddEntityRef
		add esp, 4h
		pop ecx

		/*
		push esi
		push edi
		mov edi, [esp + 8 + 4]
		mov esi, ecx

		push 7B7144h
		retn
		*/

		retn 4

	justDo:
		push 7B7130h
		retn
	}
}

static void NativeDrawblDictLog(int modelIdx)
{
	auto mi = ((CBaseModelInfo**)0x15F73B0)[modelIdx];

	//trace("load 0x%08x - %s\n", mi->GetModelHash(), GetStreamName(mi->GetDrawblDict(), *(int*)0xF272E4).c_str());
}

static void ReleaseDrawblDictTailDbg(int dict)
{
	__asm int 3
}

static void ReleaseModelWrapDbg(int modelIdx)
{
	auto mi = ((CBaseModelInfo**)0x15F73B0)[modelIdx];

	if (mi->GetDrawblDict())
	{
		__asm int 3
	}

	return ((void(*)(int))0x988440)(modelIdx);
}

void __declspec(naked) PreFullReleaseModelStub()
{
	__asm
	{
		push ecx
		push dword ptr [esp + 4 + 8]
		call PreFullReleaseModel
		add esp, 4h
		pop ecx

		sub esp, 1Ch
		push ebx
		push ebp

		push 0BCCB25h
		retn
	}
}

int __fastcall CreateBuildingInstance(CEntity* entity)
{
	auto info = ((CBaseModelInfo**)0x15F73B0)[entity->m_nModelIndex];

	if (info->GetDrawblDict() != 0xFFFF && !entity->m_pInstance)
	{
		trace("creating entity instance for drawbldict %s (%p - 0x%08x)\n", GetStreamName(info->GetDrawblDict(), *(int*)0xF272E4).c_str(), info, info->GetModelHash());
	}

	return ((int(__thiscall*)(CEntity*))0x9E7CF0)(entity);
}

int __fastcall DeleteBuildingInstance(CEntity* entity)
{
	auto info = ((CBaseModelInfo**)0x15F73B0)[entity->m_nModelIndex];

	if (info->GetDrawblDict() != 0xFFFF && entity->m_pInstance)
	{
		trace("removing entity instance for drawbldict %s (%p - 0x%08x)\n", GetStreamName(info->GetDrawblDict(), *(int*)0xF272E4).c_str(), info, info->GetModelHash());
	}

	return ((int(__thiscall*)(CEntity*))0x9E6A80)(entity);
}

void __fastcall RemoveModelInstanceLog(CBaseModelInfo* info)
{
	if (info->GetDrawblDict() != 0xFFFF)
	{
		trace("removing model instance for drawbldict %s (%p - 0x%08x)\n", GetStreamName(info->GetDrawblDict(), *(int*)0xF272E4).c_str(), info, info->GetModelHash());
	}

	return ((void(__thiscall*)(CBaseModelInfo*))0x98E6B0)(info);
}


static HookFunction hookModelInfoParents([] ()
//static RuntimeHookFunction hookModelInfoParents("ignore_lod_modelinfos", [] ()
{
	/*hook::put(0xD7CD24, RemoveModelInstanceLog);
	hook::jump(0x832C40, RequestModelStub);

	// temp dbg: find out where entity instance creation is called from
	hook::put(0xDA3A7C, CreateBuildingInstance);

	// and deletion
	hook::put(0xDA3A80, DeleteBuildingInstance);
	return;*/

	/*hook::jump(0x98A919, NativeDrawblDictLog);

	hook::jump(0x832C40, RequestModelStub);

	hook::jump(0x907786, ReleaseDrawblDictTailDbg);
	hook::put(0x98B0B9, ReleaseModelWrapDbg);
	return;*/

	hook::jump(0xBCCB20, PreFullReleaseModelStub);

	hook::put(0x90800F, DrawableDictStoreGetUsageWrap);

	// temp dbg: annoying read of some memory that is annoying the hell out of me but i should fix
	//hook::put<uint8_t>(0x7D7F8A, 0xEB);

	// temp dbg: fuck over this fatal error function as it's an annoying fuck
	hook::put<uint8_t>(0x5A8CB0, 0xCC);

	hook::call(0x8D22E2, AllocModelInfoForModel);
	hook::call(0x8D2845, AllocTimeModelInfoForModel);
	hook::call(0x8D63B2, GetModelInfo);

	//hook::jump(0x907786, ReleaseDrawblDictTail);
	hook::put(0x908028, ReleaseDrawblDictWrap);

	hook::jump(0x7B73D8, ReleaseRenderModels);

	// SetModelIndex
	hook::nop(0x9E7C2D, 11);
	hook::call(0x9E7C2D, SetModelIndexStub);

	// m6C on entity
	hook::nop(0x9E95E5, 10);
	hook::call(0x9E95E5, RetModelInfoEaxEax);

	// m68 on entity
	hook::nop(0x9E7ED8, 7);
	hook::call(0x9E7ED8, RetModelInfoEaxEax);

	// m64 on entity
	hook::nop(0x9E6B04, 7);
	hook::call(0x9E6B04, RetModelInfoEaxEax);

	// m60 on entity
	hook::nop(0x9E6AF4, 7);
	hook::call(0x9E6AF4, RetModelInfoEaxEax);

	// m5C on entity
	hook::nop(0x9E84A0, 7);
	hook::call(0x9E84A0, RetModelInfoEaxEax);

	// m58 on entity
	hook::nop(0x9E6AE4, 7);
	hook::call(0x9E6AE4, RetModelInfoEaxEcx);

	// m54 on entity
	hook::nop(0x9E828D, 7);
	hook::call(0x9E828D, RetModelInfoEaxEax);

	// m50 on entity
	hook::nop(0x9E839D, 7);
	hook::call(0x9E839D, RetModelInfoEaxEax);

	// entity func
	hook::nop(0x9E68AA, 7);
	hook::call(0x9E68AA, RetModelInfoEdiEsi);

	hook::nop(0x8D62C6, 7);
	hook::call(0x8D62C6, RetModelInfoEcxEsiAddCmp1);

	hook::nop(0x9E6DDC, 11);
	hook::call(0x9E6DDC, RetModelInfoEaxEax);

	// another buttbuddy function that addresses a modelinfo in a different way
	// and sadly I can't just use template functions for these AS THE DAMN RETARDED COMPILER WILL GENERATE RANDOM XOR [REGISTER I'M JUST ABOUT TO USE], [SAME REGISTER] OPCODES REMOVING MY DATA
	hook::nop(0x9EA116, 7);
	hook::call(0x9EA116, RetModelInfoEaxEdi);

	// and more stuff in the IPL tail sequence...
	hook::nop(0x8D4843, 11);
	hook::call(0x8D4843, RetModelInfoEdiEdi);

	hook::nop(0xB2B274, 7);
	hook::call(0xB2B274, RetModelInfoEsiEdiAddCmp);

	hook::nop(0x9E7D0C, 7);
	hook::call(0x9E7D0C, RetModelInfoEsiEdi); // ucrash?!

	// lol defsched
	hook::nop(0x7D7840, 7);
	hook::call(0x7D7840, RetModelInfoEsiEbx);

	// more defsched
	hook::nop(0x969733, 7);
	hook::call(0x969733, RetModelInfoEsiEax); // botmark

	// back on main track
	hook::nop(0x7D9C85, 7);
	hook::call(0x7D9C85, RetModelInfoEsiEdi);

	// and centity i missed above
	hook::nop(0x9E858F, 7);
	hook::call(0x9E858F, RetModelInfoEsiEdx);

	// somewhere in rendering, still not effin' done
	hook::nop(0x7E0085, 7);
	hook::call(0x7E0085, RetModelInfoEsiEdx);

	// same func
	hook::nop(0x7E01ED, 7);
	hook::call(0x7E01ED, RetModelInfoEsiEdxAddCmp1);

	// more
	hook::nop(0x9E9BAB, 7);
	hook::call(0x9E9BAB, RetModelInfoEsiEbp);

	// and even more
	hook::nop(0x9E7AFA, 7);
	hook::call(0x9E7AFA, RetModelInfoEcxEax);

	// new one
	hook::nop(0x7D8607, 7);
	hook::call(0x7D8607, RetModelInfoEaxEdi);

	// and another
	hook::nop(0x9E6B15, 7);
	hook::call(0x9E6B15, RetModelInfoEdiEcx);

	// yay
	hook::nop(0x7DF991, 7);
	hook::call(0x7DF991, RetModelInfoEdiEbx); // edi to ebx

	// more of the same
	hook::nop(0x7DFF10, 7);
	hook::call(0x7DFF10, RetModelInfoEsiEbp); // esi to ebp, persisting eax/edx

	// wow
	hook::nop(0x7DFB60, 7);
	hook::call(0x7DFB60, RetModelInfoEsiEcx); // esi to ecx

	// this is going to taek a whlie
	hook::nop(0x7DFF9F, 7);
	hook::call(0x7DFF9F, RetModelInfoEsiEax);

	// even more...
	hook::nop(0x7DF7B4, 7);
	hook::call(0x7DF7B4, RetModelInfoEsiEbpCmpEbx);

	// and it goes on
	hook::nop(0x7D99C2, 7);
	hook::call(0x7D99C2, RetModelInfoEdiEdx); // edi to edx, preserve eax

	// wpl load, amaaaaaaaazing
	hook::nop(0xB26E1C, 7);
	hook::call(0xB26E1C, RetModelInfoEcxEsi);

	// ebp to ebx? really?
	hook::nop(0x9E71BB, 7);
	hook::call(0x9E71BB, RetModelInfoEbpEbx);

	// AND ANOTHER ONE
	hook::nop(0x9E9DA6, 7);
	hook::call(0x9E9DA6, RetModelInfoEsiEdx);

	// MOAR MOAR MOAR
	hook::nop(0x7E26D5, 7);
	hook::call(0x7E26D5, RetModelInfoEdiEsi);

	// called by the last one
	hook::nop(0xB1E79A, 7);
	hook::call(0xB1E79A, RetModelInfoEsiEcx);

	// removeref function
	hook::nop(0x9E6AA0, 7);
	hook::call(0x9E6AA0, RetModelInfoEsiEsi);

	// check if refs == 0 function; fairly important if we want to clean up our drawable dicts
	hook::nop(0xAC1CFE, 7);
	hook::call(0xAC1CFE, RetModelInfoEsiEcx);

	// NEW
	hook::nop(0x7D7E8C, 7);
	hook::call(0x7D7E8C, RetModelInfoEsiEcx);

	// it never ends
	hook::nop(0xADB8D6, 7);
	hook::call(0xADB8D6, RetModelInfoEsiEdx);

	// and even when the rest works, there's more
	hook::nop(0x7D9BC6, 7);
	hook::call(0x7D9BC6, RetModelInfoEsiEcx);

	// this is new
	hook::nop(0xCAEB92, 7);
	hook::call(0xCAEB92, RetModelInfoEbxEsi);

	// load scene?
	hook::nop(0xB2B436, 7);
	hook::call(0xB2B436, RetModelInfoEcxEdi);

	// add reference to model index
	//hook::nop(0x8C8F46, 7);
	hook::call(0x8C8F46, AddModelRefEsi);

	// DrawEntityDC expansion
	hook::put<uint8_t>(0x9EA61A, 68);
	hook::put(0xD55930, ReturnInt<68>);

	hook::call(0x9EA655, DrawEntityDC_ctorHook);
	hook::call(0x7BFB26, DrawEntityDC_dontCallIfInstance);

	//hook::put<uint16_t>(0x7BFB30, 0x20);

	// CDrawEntityDC process call
	hook::nop(0x7BFB51, 7);
	hook::call(0x7BFB51, RetModelInfoDraw);

	// temp dbg: make gtaDefSched single-threaded
	//hook::put<uint8_t>(0x79706F, 0);
	//hook::put<uint8_t>(0x797072, 1);

	// temp dbg: ignore 30999
	hook::put(0x98E5B4, 0x7917);
	hook::put(0x98E5BC, 0x7917);
	hook::put(0x989753, 0x7917);

	// as shown above
	hook::jump(0x98975B, PostProcessCustomMIs);

	// temp dbg: check where LOD WDD proxies get requested
	//hook::jump(0x832C40, RequestModelStub);
	//hook::jump(0x832C70, ReleaseModelStub);
	//hook::jump(0x832DD0, IsModelLoadedStub);

	// meta object call #1
	// 7D9F76 calls request
	hook::call(0x7D9F76, RequestEntityModelEsi);
	hook::call(0xB2B3FB, RequestEntityModelEsi);
	hook::call(0x7DA076, RequestEntityModelEsi);

	// process loader tail
	hook::jump(0xC57786, ProcessEntityRequests);

	// draw entity dc tail
	hook::jump(0x7BFCF7, DrawEntityDCTail);

	// temp dbg: relocate datBase vtable
	/*char* newDatBase = new char[1024];
	memset(newDatBase, 0, 1024);

	for (char* ptrr = (char*)0x400100; ptrr < (char*)0xD40000; ptrr++)
	{
		if (!memcmp(ptrr, "\x50\x29\xD5\x00", 4))
		{
			*(char**)ptrr = newDatBase;
		}
	}*/
});