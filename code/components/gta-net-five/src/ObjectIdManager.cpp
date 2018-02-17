#include <StdInc.h>
#include <Hooking.h>

#include <NetBuffer.h>
#include <NetLibrary.h>

#include <GameInit.h>
#include <nutsnbolts.h>

static std::set<int> g_objectIds;
static std::set<int> g_usedObjectIds;

static uint32_t AssignObjectId(void* objectIds)
{
	if (g_objectIds.empty())
	{
		trace("No free object ID!\n");
		return 0;
	}

	auto it = g_objectIds.begin();
	auto objectId = *it;

	g_objectIds.erase(it);
	g_usedObjectIds.insert(objectId);

	trace("assigned object id %d\n", objectId);

	return objectId;
}

static bool ReturnObjectId(void* objectIds, uint16_t objectId)
{
	if (g_usedObjectIds.find(objectId) != g_usedObjectIds.end())
	{
		trace("returned object id %d\n", objectId);

		g_usedObjectIds.erase(objectId);
		g_objectIds.insert(objectId);

		return true;
	}

	return false;
}

static bool HasSpaceForObjectId(void* objectIds, int num, bool unkScript)
{
	return (g_objectIds.size() >= num);
}

static NetLibrary* g_netLibrary;

static bool g_requestedIds;

void ObjectIds_BindNetLibrary(NetLibrary* netLibrary)
{
	g_netLibrary = netLibrary;

	netLibrary->AddReliableHandler("msgObjectIds", [](const char* data, size_t len)
	{
		net::Buffer buffer(reinterpret_cast<const uint8_t*>(data), len);

		auto numIds = buffer.Read<uint16_t>();

		int last = 0;

		for (int i = 0; i < numIds; i++)
		{
			auto skip = buffer.Read<uint16_t>();
			auto take = buffer.Read<uint16_t>();

			last += skip + 1;

			for (int j = 0; j <= take; j++)
			{
				int objectId = last++;

				trace("got object id %d\n", objectId);

				g_objectIds.insert(objectId);
			}
		}

		g_requestedIds = false;
	});
}

static HookFunction hookFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		if (g_objectIds.size() < 16)
		{
			if (!g_requestedIds)
			{
				net::Buffer outBuffer;
				outBuffer.Write<uint16_t>(32);

				g_netLibrary->SendReliableCommand("msgRequestObjectIds", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());

				g_requestedIds = true;
			}
		}
	});

	OnKillNetworkDone.Connect([]()
	{
		g_objectIds.clear();
		g_usedObjectIds.clear();

		g_requestedIds = false;
	});

	hook::jump(hook::get_pattern("FF 89 C4 3E 00 00 33 D2", -12), AssignObjectId);

	hook::jump(hook::get_pattern("44 8B 91 C4 3E 00 00", -0x14), ReturnObjectId);

	hook::jump(hook::get_pattern("48 83 EC 20 8B B1 C4 3E 00 00", -0xB), HasSpaceForObjectId);
});
