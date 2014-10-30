#include "StdInc.h"
#include "Hooking.h"
#include "Pool.h"
#include "IdeStore.h"
#include "Streaming.h"
#include <queue>
#include <mutex>

#include <strsafe.h>

void CIdeStore::Initialize()
{
	CRect worldRect;
	worldRect.fX1 = -4000.0f;
	worldRect.fY1 = 8000.0f;
	worldRect.fX2 = 4000.0f;
	worldRect.fY2 = -4000.0f;

	ms_pQuadTree = new CQuadTreeNode(worldRect, 4);
}

int CIdeStore::ms_ideNum;

int CIdeStore::RegisterIde(const char* fileName, uint32_t size)
{
	// return nothing if we're already identified
	for (auto item : ms_registeredIdes)
	{
		if (!item) break;

		if (item->GetFileName() == fileName)
		{
			return -1;
		}
	}

	int ideNum = ms_ideNum;
	ms_ideNum++;

	ms_registeredIdes[ideNum] = new IdeFile(fileName, size, ideNum);

	return ideNum;
}

static uint32_t WRAPPER NatHash(const char* str) { EAXJMP(0x7BDBF0); }

void CIdeStore::RegisterDrawable(const StreamingResource& entry)
{
	char fileNameStripped[256];
	StringCbCopyA(fileNameStripped, sizeof(fileNameStripped), entry.filename.c_str());

	strrchr(fileNameStripped, '.')[0] = '\0';

	ms_drawables[NatHash(fileNameStripped)] = entry;
}

void CIdeStore::SetIdeRelationBegin(const char* ideName)
{
	const char* ideNamePart = strrchr(ideName, '/');

	if (!ideNamePart)
	{
		return;
	}

	char iplNameLocal[64];
	StringCbCopyA(iplNameLocal, sizeof(iplNameLocal), ideNamePart + 1);

	strrchr(iplNameLocal, '.')[0] = '\0';

	StringCbCatA(iplNameLocal, sizeof(iplNameLocal), "_stream");

	for (auto item : ms_registeredIdes)
	{
		if (!item) continue;

		auto fileName = item->GetFileName();

		if (fileName.find(iplNameLocal) == 0 && fileName.find(".ide") != std::string::npos)
		{
			ms_relatedIdes.push_back(item);

			item->Request();

			CIdeStore::LoadAllRequestedArchetypes();
		}
	}

	trace("IDE relation for %s done\n", ideName);
}

void CIdeStore::SetIdeRelationEnd()
{
	trace("IDE relation end\n");

	// remove IDEs
	for (auto item : ms_relatedIdes)
	{
		CRect rect = item->GetBounds();

		rect.fX1 -= 375.0f;
		rect.fX2 += 375.0f;
		rect.fY1 += 375.0f;
		rect.fY2 -= 375.0f;

		ms_pQuadTree->AddItem(item, rect);

		item->Delete();
	}

	ms_relatedIdes.clear();
}

void CIdeStore::SetIdePoint(uint16_t curMI, const CRect& rect)
{
	for (auto item : ms_relatedIdes)
	{
		item->AddToBounds(curMI, rect);
	}
}

void CIdeStore::LoadForPosn(const CVector2D& vector)
{
	// clear required flag
	for (auto item : ms_registeredIdes)
	{
		if (!item) continue;

		item->SetRequired(false);
	}

	// scan quadtree
	ms_pQuadTree->ForAllMatching(vector, [] (const CVector2D&, void* idePtr)
	{
		IdeFile* file = (IdeFile*)idePtr;

		file->SetRequired(true);
	});

	// and load all that's required
	for (auto item : ms_registeredIdes)
	{
		if (!item) continue;

		if (item->IsRequired())
		{
			item->Request();
		}
		else
		{
			item->Delete();
		}
	}
}

static std::set<IdeFile*> g_openRequests;

bool CIdeStore::CanLoadAdditionalIde()
{
	return (g_openRequests.size() < 6);
}

static std::queue<IdeFile*> g_completedRequests;
static std::mutex g_mutex;

void CIdeStore::EnqueueRequestBegin(IdeFile* ideFile)
{
	g_mutex.lock();

	if (g_openRequests.find(ideFile) == g_openRequests.end())
	{
		g_openRequests.insert(ideFile);
	}

	g_mutex.unlock();
}

void CIdeStore::EnqueueRequestCompletion(IdeFile* ideFile)
{
	auto item = &g_streamingItems[ideFile->GetStreamItemIdx()];

	item->handle += g_streamMask;
	item->blockSize = 0;
	item->streamCounter = g_nextStreamingItem;
	g_nextStreamingItem = (g_streamMask - 1) & item->handle;//item->flags;

	g_mutex.lock();

	g_openRequests.erase(ideFile);

	g_completedRequests.push(ideFile);

	g_mutex.unlock();
}

void CIdeStore::Process()
{
	while (!g_completedRequests.empty())
	{
		g_mutex.lock();

		auto item = g_completedRequests.front();
		g_completedRequests.pop();

		g_mutex.unlock();

		item->DoLoad();
	}
}

void CIdeStore::LoadAllRequestedArchetypes()
{
	while (g_openRequests.size() > 0)
	{
		Sleep(5);

		Process();
	}
}

bool CIdeStore::CanWeEvenLoadAnyIpls()
{
	return (g_openRequests.size() == 0);
}

static void DoIplStoreAndIdeStoreInitHook()
{
	CIdeStore::Initialize();

	// CIplStore::Initialise
	((void(*)())0xB276E0)();
}

static void SetupRelatedIplsHook(const char* iplName, int a2, void** lodEntities)
{
	CIdeStore::SetIdeRelationBegin(iplName);

	// original SetupRelatedIpls
	((void(*)(const char*, int, void**))0xB26160)(iplName, a2, lodEntities);
}

static void RemoveRelatedIplsTail()
{
	CIdeStore::SetIdeRelationEnd();
}

static void CEntityAddFunc(char* entity, const CRect& rect)
{
	CIdeStore::SetIdePoint(*(uint16_t*)(entity + 0x2E), rect);
}

static void __declspec(naked) CEntityAddStub()
{
	__asm
	{
		push ecx // additional push so we can pop, as the optimizer reuses argument space to save making a stack frame

		push dword ptr [esp + 8h]
		
		push ecx

		call CEntityAddFunc

		add esp, 8h
		pop ecx

		push ebp
		mov ebp, esp
		and esp, 0FFFFFFF0h

		push 9E9E16h
		retn
	}
}

static void LoadIplIfWeCan(int ipl, int type, int prio)
{
	if (CIdeStore::CanWeEvenLoadAnyIpls())
	{
		((void(*)(int, int, int))0x832C40)(ipl, type, prio);
	}
}

void LoadSceneBitsFunc(const CVector2D& vector)
{
	if (!((bool(*)())0x832EF0)())
	{
		CIdeStore::LoadForPosn(vector);
	}
}

static void __declspec(naked) LoadSceneBitsHook()
{
	__asm
	{
		push dword ptr [esp + 4]
		call LoadSceneBitsFunc
		add esp, 4h

		push    ebp
		mov     ebp, esp
		and     esp, 0FFFFFFF0h

		push 0B26FA6h
		retn
	}
}

static HookFunction hookFunction([] ()
{
	hook::call(0x8ACC6E, DoIplStoreAndIdeStoreInitHook);

	hook::call(0x8D8080, SetupRelatedIplsHook);
	hook::call(0x8D7712, SetupRelatedIplsHook);

	hook::jump(0xB2635B, RemoveRelatedIplsTail);

	hook::jump(0x9E9E10, CEntityAddStub);

	hook::jump(0xB26FA0, LoadSceneBitsHook);

	hook::call(0xB27207, LoadIplIfWeCan);
});

CQuadTreeNode* CIdeStore::ms_pQuadTree;

IdeFile* CIdeStore::ms_registeredIdes[2048];

fwVector<IdeFile*> CIdeStore::ms_relatedIdes;

fwMap<uint32_t, StreamingResource> CIdeStore::ms_drawables;

WRAPPER CQuadTreeNode::CQuadTreeNode(const CRect& rectangle, int numSubNodes) { EAXJMP(0x8DE0F0); }
void WRAPPER CQuadTreeNode::AddItem(void* item, const CRect& rect) { EAXJMP(0x8DE330); }
void WRAPPER CQuadTreeNode::ForAllMatching(const CVector2D& vector, void(*callback)(const CVector2D&, void*)) { EAXJMP(0x8DE130); }

void* CQuadTreeNode::operator new(size_t size)
{
	return CPools::GetQuadTreePool()->Allocate();
}