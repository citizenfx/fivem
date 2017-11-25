#include <StdInc.h>
#include <CoreConsole.h>
#include <Hooking.h>
#include <ScriptEngine.h>

#include <base64.h>

static hook::cdecl_stub<bool(void*, uint32_t, int)> _netBuffer_WriteInteger([]()
{
	return hook::get_pattern("48 8B D9 40 84 79 1C 75 6F 8B 49 10", -0x29);
});

namespace rage
{
class netObject;

class netBuffer
{
public:
	inline netBuffer(void* data, size_t size)
	{
		m_data = data;
		m_f8 = 0;
		m_maxBit = size * 8;
		m_unkBit = 0;
		m_curBit = 0;
		m_unk2Bit = 0;
		m_f1C = 0;
	}

	inline uint32_t GetPosition()
	{
		return m_unkBit;
	}

	inline bool Seek(int bits)
	{
		if (bits >= 0)
		{
			uint32_t length = (m_f1C & 1) ? m_maxBit : m_curBit;

			if (bits <= length)
			{
				m_unkBit = bits;
			}
		}

		return false;
	}

	inline bool WriteInteger(uint32_t integer, int bits)
	{
		return _netBuffer_WriteInteger(this, integer, bits);
	}

public:
	void* m_data;
	uint32_t m_f8;
	uint32_t m_maxBit;
	uint32_t m_unkBit;
	uint32_t m_curBit;
	uint32_t m_unk2Bit;
	uint8_t m_f1C;
};

class netSyncTree
{
public:
	virtual ~netSyncTree() = 0;

	virtual void WriteTree(int flags, int objFlags, netObject* object, netBuffer* buffer, uint32_t time, void* logger, uint8_t targetPlayer, void* outNull) = 0;

	virtual void ApplyToObject(netObject* object, void*) = 0;
};

class netBlender
{
public:
	virtual ~netBlender() = 0;

	virtual void m_8() = 0;

	virtual void m_10() = 0;

	virtual void m_18() = 0;

	virtual void m_20() = 0;

	virtual void m_28() = 0;

	virtual void m_30() = 0;

	virtual void m_38() = 0;

	virtual void ApplyBlend() = 0;

	virtual void m_48() = 0;

	virtual void m_50() = 0;

	virtual void m_58() = 0;
};

class netObject
{
public:
	inline netBlender* GetBlender()
	{
		return *(netBlender**)((uintptr_t)this + 88);
	}

	virtual ~netObject() = 0;

	virtual void m_8() = 0;
	virtual void m_10() = 0;
	virtual void m_18() = 0;
	virtual void* m_20() = 0;
	virtual void m_28() = 0;
	virtual netSyncTree* GetSyncTree() = 0;
	virtual void m_38() = 0;
	virtual void m_40() = 0;
	virtual void m_48() = 0;
	virtual void m_50() = 0;
	virtual void m_58() = 0;
	virtual void m_60() = 0;
	virtual void m_68() = 0;
	virtual void m_70() = 0;
	virtual void m_78() = 0;
	virtual void* GetGameObject() = 0;
	virtual void m_88() = 0;
	virtual void m_90() = 0;
	virtual void m_98() = 0;
	virtual int GetObjectFlags() = 0;
	virtual void m_A8() = 0;
	virtual void m_B0() = 0;
	virtual void m_B8() = 0;
	virtual void m_C0() = 0;
	virtual void m_C8() = 0;
	virtual void m_D0() = 0;
	virtual void m_D8() = 0;
	virtual void m_E0() = 0;
	virtual void m_E8() = 0;
	virtual void m_F0() = 0;
	virtual void m_F8() = 0;
	virtual void m_100() = 0;
	virtual void m_108() = 0;
	virtual void m_110() = 0;
	virtual void m_118() = 0;
	virtual void m_120() = 0;
	virtual void m_128() = 0;
	virtual void m_130() = 0;
	virtual void m_138() = 0;
	virtual void m_140() = 0;
	virtual void m_148() = 0;
	virtual void m_150() = 0;
	virtual void m_158() = 0;
	virtual void m_160() = 0;
	virtual void m_168() = 0;
	virtual void m_170() = 0;
	virtual void m_178() = 0;
	virtual void m_180() = 0;
	virtual void m_188() = 0;
	virtual void m_190() = 0;
	virtual void m_198() = 0;
	virtual void m_1A0() = 0;
	virtual void m_1A8() = 0;
	virtual void m_1B0() = 0;
	virtual void m_1B8() = 0;
	virtual void m_1C0() = 0;
	virtual void m_1C8() = 0;
	virtual void m_1D0() = 0;
	virtual void m_1D8() = 0;
	virtual void m_1E0() = 0;
};

class netInterface_queryFunctions
{
public:
	virtual ~netInterface_queryFunctions() = 0;

	virtual void m_8() = 0;

	virtual uint32_t GetTimestamp() = 0;
};

class netObjectMgr
{
public:
	virtual ~netObjectMgr() = 0;

	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;
	virtual void Update() = 0;

	virtual void AddEntity() = 0;
	virtual void m_28() = 0;
	virtual void m_30() = 0;
	virtual void m_38() = 0;
	virtual void m_40() = 0;
	virtual void m_48() = 0;
	virtual void m_50() = 0;
	virtual void m_58() = 0;
	virtual void RegisterObject(rage::netObject* entity) = 0;

private:
	struct atDNetObjectNode
	{
		virtual ~atDNetObjectNode();

		netObject* object;
		atDNetObjectNode* next;
	};

private:
	atDNetObjectNode* m_objects[32];

public:
	template<typename T>
	inline void ForAllNetObjects(int playerId, const T& callback)
	{
		for (auto node = m_objects[playerId]; node; node = node->next)
		{
			if (node->object)
			{
				callback(node->object);
			}
		}
	}
};
}

static hook::cdecl_stub<rage::netSyncTree*(void*, int)> getSyncTreeForType([]()
{
	return hook::get_pattern("0F B7 CA 83 F9 07 7F 5E");
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, int flags, int flags2, rage::netBuffer* buffer, void* netLogStub)> netSyncTree_ReadFromBuffer([]()
{
	return hook::get_pattern("45 89 43 18 57 48 83 EC 30 48 83 79 10 00 49", -15);
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, rage::netObject* obj)> netSyncTree_CanApplyToObject([]()
{
	return hook::get_pattern("49 8B CE FF 50 70 84 C0 74 31 33 FF", -0x2C);
});

static hook::cdecl_stub<void(rage::netBlender*, uint32_t timeStamp)> netBlender_SetTimestamp([]()
{
	return hook::get_pattern("48 8B D9 39 79 18 74 76 48", -0x13);
});

rage::netInterface_queryFunctions* g_queryFunctions;
rage::netObjectMgr** g_objectMgr;

using TCreateCloneObjFn = rage::netObject*(*)(uint16_t objectId, uint8_t, int, int);

enum class NetObjEntityType
{
	Automobile = 0,
	Bike = 1,
	Boat = 2,
	Door = 3,
	Heli = 4,
	Object = 5,
	Ped = 6,
	Pickup = 7,
	PickupPlacement = 8,
	Plane = 9,
	Submarine = 10,
	Player = 11,
	Trailer = 12,
	Train = 13
};

static std::map<NetObjEntityType, TCreateCloneObjFn> createCloneFuncs;

static hook::cdecl_stub<void*(int handle)> getScriptEntity([]()
{
	return hook::pattern("44 8B C1 49 8B 41 08 41 C1 F8 08 41 38 0C 00").count(1).get(0).get<void>(-12);
});

static hook::cdecl_stub<uint32_t(void*)> getScriptGuidForEntity([]()
{
	return hook::get_pattern("48 F7 F9 49 8B 48 08 48 63 D0 C1 E0 08 0F B6 1C 11 03 D8", -0x68);
});

struct ReturnedCallStub : public jitasm::Frontend
{
	ReturnedCallStub(int idx, uintptr_t targetFunc)
		: m_index(idx), m_targetFunc(targetFunc)
	{

	}

	static void InstrumentedTarget(void* targetFn, void* callFn, int index)
	{
		trace("called %016llx (netBlender::m_%x) from %016llx\n", (uintptr_t)targetFn, index, (uintptr_t)callFn);
	}

	virtual void InternalMain() override
	{
		push(rcx);
		push(rdx);
		push(r8);
		push(r9);

		mov(rcx, m_targetFunc);
		mov(rdx, qword_ptr[rsp + 0x20]);
		mov(r8d, m_index);

		// scratch space (+ alignment for stack)
		sub(rsp, 0x28);

		mov(rax, (uintptr_t)InstrumentedTarget);
		call(rax);

		add(rsp, 0x28);

		pop(r9);
		pop(r8);
		pop(rdx);
		pop(rcx);

		mov(rax, m_targetFunc);
		jmp(rax);
	}

private:
	uintptr_t m_originFunc;
	uintptr_t m_targetFunc;

	int m_index;
};

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_address<rage::netInterface_queryFunctions*>(hook::get_pattern("48 8D 0D ? ? ? ? 48 89 44 24 20 E8 ? ? ? ? E8", -4));

		g_queryFunctions = location;
	}

	{
		auto location = hook::get_pattern<char>("0F 8E 12 03 00 00 41 8A 57 2D", 22);
		createCloneFuncs[NetObjEntityType::Ped] = (TCreateCloneObjFn)hook::get_call(location);
		createCloneFuncs[NetObjEntityType::Object] = (TCreateCloneObjFn)hook::get_call(location + 0x39);
		createCloneFuncs[NetObjEntityType::Heli] = (TCreateCloneObjFn)hook::get_call(location + 0x72);
		createCloneFuncs[NetObjEntityType::Door] = (TCreateCloneObjFn)hook::get_call(location + 0xAB);
		createCloneFuncs[NetObjEntityType::Boat] = (TCreateCloneObjFn)hook::get_call(location + 0xE4);
		createCloneFuncs[NetObjEntityType::Bike] = (TCreateCloneObjFn)hook::get_call(location + 0x11D);
		createCloneFuncs[NetObjEntityType::Automobile] = (TCreateCloneObjFn)hook::get_call(location + 0x156);
		createCloneFuncs[NetObjEntityType::Pickup] = (TCreateCloneObjFn)hook::get_call(location + 0x18F);
		createCloneFuncs[NetObjEntityType::Train] = (TCreateCloneObjFn)hook::get_call(location + 0x1EF);
		createCloneFuncs[NetObjEntityType::Trailer] = (TCreateCloneObjFn)hook::get_call(location + 0x228);
		createCloneFuncs[NetObjEntityType::Player] = (TCreateCloneObjFn)hook::get_call(location + 0x261);
		createCloneFuncs[NetObjEntityType::Submarine] = (TCreateCloneObjFn)hook::get_call(location + 0x296);
		createCloneFuncs[NetObjEntityType::Plane] = (TCreateCloneObjFn)hook::get_call(location + 0x2C8);
		createCloneFuncs[NetObjEntityType::PickupPlacement] = (TCreateCloneObjFn)hook::get_call(location + 0x2FA);
	}

	{
		g_objectMgr = hook::get_address<rage::netObjectMgr**>(hook::get_pattern("B9 C8 7F 00 00 E8 C8 4B 04 FF 48 85 C0", 0x30));
	}

	// temp dbg
	//hook::put<uint16_t>(hook::get_pattern("0F 84 80 00 00 00 49 8B 07 49 8B CF FF 50 20"), 0xE990);

	// VERY temp dbg
	//hook::nop(0x1410987E8, 6);
	//*(uint32_t*)0x141098841 = 0x00C3C748;

	/*uintptr_t* vmt = (uintptr_t*)0x14196B3C8;

	for (int i = 0; i < 40; i++)
	{
		auto stub = new ReturnedCallStub(i * 8, *vmt);
		*vmt = (uintptr_t)stub->GetCode();

		++vmt;
	}*/
});

#include <nutsnbolts.h>
#include <GameInit.h>

static hook::cdecl_stub<int()> getPlayerId([]()
{
	return hook::get_pattern("0F B6 40 2D EB 02 33 C0 48", -0x19);
});

static char(*g_origWriteDataNode)(void* node, uint32_t flags, void* mA0, rage::netObject* object, rage::netBuffer* buffer, int time, void* playerObj, char playerId, void* unk);

static bool WriteDataNodeStub(void* node, uint32_t flags, void* mA0, rage::netObject* object, rage::netBuffer* buffer, int time, void* playerObj, char playerId, void* unk)
{
	if (playerId != 31)
	{
		return g_origWriteDataNode(node, flags, mA0, object, buffer, time, playerObj, playerId, unk);
	}
	else
	{
		// save position and write a placeholder length frame
		uint32_t position = buffer->GetPosition();
		buffer->WriteInteger(0, 11);

		bool rv = g_origWriteDataNode(node, flags, mA0, object, buffer, time, playerObj, playerId, unk);

		// write the actual length on top of the position
		uint32_t endPosition = buffer->GetPosition();
		buffer->Seek(position);
		buffer->WriteInteger(endPosition - position - 11, 11);
		buffer->Seek(endPosition);

		return rv;
	}
}

static HookFunction hookFunction2([]()
{
	// 2 matches, 1st is data, 2nd is parent
	{
		auto location = hook::get_pattern<char>("48 89 44 24 20 E8 ? ? ? ? 84 C0 0F 95 C0 48 83 C4 58", -0x3C);
		hook::set_call(&g_origWriteDataNode, location + 0x41);
		hook::jump(location, WriteDataNodeStub);
	}
});

int ObjectToEntity(int objectId)
{
	int playerIdx = (objectId >> 16) - 1;
	int objectIdx = (objectId & 0xFFFF);

	int entityIdx = -1;

	(*g_objectMgr)->ForAllNetObjects(playerIdx, [&](rage::netObject* obj)
	{
		char* objectChar = (char*)obj;
		uint16_t thisObjectId = *(uint16_t*)(objectChar + 10);

		if (objectIdx == thisObjectId)
		{
			entityIdx = getScriptGuidForEntity(obj->GetGameObject());
		}
	});

	return entityIdx;
}

#include <boost/range/adaptor/map.hpp>
#include <mmsystem.h>

#include <NetLibrary.h>
#include <NetBuffer.h>

extern NetLibrary* g_netLibrary;

struct ObjectData
{
	uint32_t lastSyncTime;
	uint32_t lastSyncAck;

	ObjectData()
	{
		lastSyncTime = 0;
		lastSyncAck = 0;
	}
};

static std::map<int, ObjectData> trackedObjects;

#include <lz4.h>

static InitFunction initFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		if (g_netLibrary == nullptr)
		{
			return;
		}

		if (g_netLibrary->GetConnectionState() != NetLibrary::CS_ACTIVE)
		{
			return;
		}

		// protocol 5 or higher are aware of this state
		if (g_netLibrary->GetServerProtocol() <= 4)
		{
			return;
		}

		auto objectMgr = *g_objectMgr;

		static uint32_t lastSend;
		static net::Buffer netBuffer;

		if (objectMgr)
		{
			std::set<int> seenObjects;

			objectMgr->ForAllNetObjects(getPlayerId(), [&](rage::netObject* object)
			{
				char* objectChar = (char*)object;
				uint16_t objectType = *(uint16_t*)(objectChar + 8);
				uint16_t objectId = *(uint16_t*)(objectChar + 10);

				auto& objectData = trackedObjects[objectId];

				static char blah[90000];

				static char bluh[1000];
				memset(bluh, 0, sizeof(bluh));
				memset(blah, 0, sizeof(blah));

				rage::netBuffer buffer(bluh, sizeof(bluh));

				// if we want to delete this object
				if (*(char*)(objectChar + 76) & 1)
				{
					if (*(uint32_t*)(objectChar + 112) & (1 << 31))
					{
						// unack the create to unburden the game
						*(uint32_t*)(objectChar + 112) &= ~(1 << 31);

						// pretend to ack the remove to process removal
						// 1103
						((void(*)(rage::netObjectMgr*, rage::netObject*))0x141595A30)(objectMgr, object);
					}

					return;
				}

				auto st = object->GetSyncTree();
				int syncType = 0;

				if (objectData.lastSyncTime == 0)
				{
					// clone create
					syncType = 1;
				}
				else if ((timeGetTime() - objectData.lastSyncTime) > 250)
				{
					if (objectData.lastSyncAck == 0)
					{
						// create resend
						syncType = 1;
					}
					else
					{
						// ack'd create on player 31
						*(uint32_t*)(objectChar + 112) |= (1 << 31);

						char* syncData = (char*)object->m_20();

						if (syncData)
						{
							*(uint32_t*)(syncData + 8) |= (1 << 31);
							*(uint32_t*)(syncData + 176 + 8) |= (1 << 31);
						}

						// clone sync
						syncType = 2;
					}
				}

				if (syncType != 0)
				{
					// write header
					netBuffer.Write<uint8_t>(syncType);

					// write tree
					st->WriteTree(syncType, 0, object, &buffer, g_queryFunctions->GetTimestamp(), nullptr, 31, nullptr);

					// instantly mark player 31 as acked
					if (object->m_20())
					{
						((void(*)(rage::netSyncTree*, rage::netObject*, uint8_t, uint16_t, uint32_t, int))0x1415A60C4)(st, object, 31, 0 /* seq? */, 0x7FFFFFFF, 0xFFFFFFFF);
					}

					// write data
					char leftoverBit = (buffer.m_curBit % 8) ? 0 : 1;

					netBuffer.Write<uint8_t>(getPlayerId()); // player ID (byte)
					netBuffer.Write<uint16_t>(objectId); // object ID (short)
					netBuffer.Write<uint8_t>(objectType);

					uint32_t len = (buffer.m_curBit / 8) + leftoverBit;
					netBuffer.Write<uint16_t>(len); // length (short)
					netBuffer.Write(buffer.m_data, len); // data

					objectData.lastSyncTime = timeGetTime();
				}

				seenObjects.insert(objectId);
			});

			// anyone gone?
			auto iter = trackedObjects | boost::adaptors::map_keys;

			std::vector<int> removedObjects;
			std::set_difference(iter.begin(), iter.end(), seenObjects.begin(), seenObjects.end(), std::back_inserter(removedObjects));

			for (auto obj : removedObjects)
			{
				auto& syncData = trackedObjects[obj];

				if ((timeGetTime() - syncData.lastSyncTime) > 100)
				{
					// write clone remove header
					netBuffer.Write<uint8_t>(3);
					netBuffer.Write<uint8_t>(getPlayerId()); // player ID (byte)
					netBuffer.Write<uint16_t>(obj); // object ID (short)

					syncData.lastSyncTime = timeGetTime();
				}
			}
		}

		if ((timeGetTime() - lastSend) > 250 || netBuffer.GetCurOffset() >= 1200)
		{
			if (netBuffer.GetCurOffset() > 0)
			{
				// compress and send data
				std::vector<char> outData(LZ4_compressBound(netBuffer.GetCurOffset()) + 4);
				int len = LZ4_compress_default(reinterpret_cast<const char*>(netBuffer.GetData().data()), outData.data() + 4, netBuffer.GetCurOffset(), outData.size() - 4);

				*(uint32_t*)(outData.data()) = HashString("netClones");
				g_netLibrary->RoutePacket(outData.data(), len + 4, 0xFFFF);

#if _DEBUG
				static int byteHunkId;

				FILE* f = _wfopen(MakeRelativeCitPath(fmt::sprintf(L"cache/byteHunk/bytehunk-%d.bin", byteHunkId++)).c_str(), L"wb");

				if (f)
				{
					fwrite(outData.data(), 1, len + 4, f);
					fclose(f);
				}
#endif
			}

			netBuffer.Reset();
			lastSend = timeGetTime();
		}

		if (g_netLibrary)
		{
			static bool g_netlibHookInited;

			if (!g_netlibHookInited)
			{
				g_netLibrary->AddReliableHandler("msgCloneAcks", [](const char* data, size_t len)
				{
					net::Buffer buf(reinterpret_cast<const uint8_t*>(data), len);
					
					while (!buf.IsAtEnd())
					{
						auto type = buf.Read<uint8_t>();

						switch (type)
						{
						case 1:
						{
							auto objId = buf.Read<uint16_t>();
							trackedObjects[objId].lastSyncAck = timeGetTime();

							break;
						}
						case 3:
						{
							auto objId = buf.Read<uint16_t>();
							trackedObjects.erase(objId);

							break;
						}
						default:
							return;
						}
					}
				});

				g_netlibHookInited = true;
			}
		}
	});

	OnKillNetworkDone.Connect([]()
	{
		trackedObjects.clear();
	});
#if 0
	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_SAVE_CLONE_CREATE", [](fx::ScriptContext& context)
	{
		char* entity = (char*)getScriptEntity(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("SAVE_CLONE_CREATE: invalid entity\n");

			context.SetResult<const char*>("");
			return;
		}

		auto netObj = *(rage::netObject**)(entity + 208);

		static char blah[90000];

		static char bluh[1000];
		memset(bluh, 0, sizeof(bluh));
		memset(blah, 0, sizeof(blah));

		rage::netBuffer buffer(bluh, sizeof(bluh));

		auto st = netObj->GetSyncTree();
		st = getSyncTreeForType(nullptr, (int)NetObjEntityType::Ped);
		st->WriteTree(1, 0, netObj, &buffer, g_queryFunctions->GetTimestamp(), nullptr, 31, nullptr);

		static char base64Buffer[2000];

		size_t outLength = sizeof(base64Buffer);
		char* txt = base64_encode((uint8_t*)buffer.m_data, (buffer.m_curBit / 8) + 1, &outLength);

		memcpy(base64Buffer, txt, outLength);
		free(txt);

		base64Buffer[outLength] = '\0';

		context.SetResult<const char*>(base64Buffer);
	});

	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_SAVE_CLONE_SYNC", [](fx::ScriptContext& context)
	{
		char* entity = (char*)getScriptEntity(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("SAVE_CLONE_SYNC: invalid entity\n");

			context.SetResult<const char*>("");
			return;
		}

		auto netObj = *(rage::netObject**)(entity + 208);

		static char blah[90000];

		static char bluh[1000];
		memset(bluh, 0, sizeof(bluh));
		memset(blah, 0, sizeof(blah));

		rage::netBuffer buffer(bluh, sizeof(bluh));

		auto st = netObj->GetSyncTree();
		st = getSyncTreeForType(nullptr, (int)NetObjEntityType::Ped);
		st->WriteTree(2, 0, netObj, &buffer, g_queryFunctions->GetTimestamp(), nullptr, 31, nullptr);

		static char base64Buffer[2000];

		size_t outLength = sizeof(base64Buffer);
		char* txt = base64_encode((uint8_t*)buffer.m_data, (buffer.m_curBit / 8) + 1, &outLength);

		memcpy(base64Buffer, txt, outLength);
		free(txt);

		trace("saving netobj %llx\n", (uintptr_t)netObj);

		base64Buffer[outLength] = '\0';

		context.SetResult<const char*>(base64Buffer);
	});

	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_LOAD_CLONE_CREATE", [](fx::ScriptContext& context)
	{
		auto data = context.GetArgument<const char*>(0);
		auto objectId = context.GetArgument<uint16_t>(1);
		auto objectType = context.GetArgument<const char*>(2);

		NetObjEntityType objType;

		if (strcmp(objectType, "automobile") == 0)
		{
			objType = NetObjEntityType::Automobile;
		}
		else if (strcmp(objectType, "player") == 0)
		{
			objType = NetObjEntityType::Ped; // until we make native players
		}
		else if (strcmp(objectType, "ped") == 0)
		{
			objType = NetObjEntityType::Ped;
		}
		else
		{
			context.SetResult(-1);
			return;
		}

		//trace("making a %d\n", (int)objType);

		size_t decLen;
		uint8_t* dec = base64_decode(data, strlen(data), &decLen);

		rage::netBuffer buf(dec, decLen);
		buf.m_f1C = 1;

		auto st = getSyncTreeForType(nullptr, (int)objType);

		netSyncTree_ReadFromBuffer(st, 1, 0, &buf, nullptr);

		free(dec);

		auto obj = createCloneFuncs[objType](objectId, 31, 0, 32);
		*((uint8_t*)obj + 75) = 1;

		if (!netSyncTree_CanApplyToObject(st, obj))
		{
			trace("Couldn't apply object.\n");

			delete obj;

			context.SetResult(-1);
			return;
		}

		st->ApplyToObject(obj, nullptr);
		(*g_objectMgr)->RegisterObject(obj);

		netBlender_SetTimestamp(obj->GetBlender(), g_queryFunctions->GetTimestamp());

		obj->m_1D0();

		obj->GetBlender()->ApplyBlend();
		obj->GetBlender()->m_38();

		obj->m_1C0();

		context.SetResult(getScriptGuidForEntity(obj->GetGameObject()));
	});

	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_LOAD_CLONE_SYNC", [](fx::ScriptContext& context)
	{
		char* entity = (char*)getScriptEntity(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("LOAD_CLONE_SYNC: invalid entity\n");
			return;
		}

		auto obj = *(rage::netObject**)(entity + 208);

		auto data = context.GetArgument<const char*>(1);
		
		size_t decLen;
		uint8_t* dec = base64_decode(data, strlen(data), &decLen);

		rage::netBuffer buf(dec, decLen);
		buf.m_f1C = 1;

		auto st = obj->GetSyncTree();

		netSyncTree_ReadFromBuffer(st, 2, 0, &buf, nullptr);

		free(dec);

		netBlender_SetTimestamp(obj->GetBlender(), g_queryFunctions->GetTimestamp());
		obj->GetBlender()->m_28();

		st->ApplyToObject(obj, nullptr);

		obj->m_1D0();

		//obj->GetBlender()->m_30();
		obj->GetBlender()->m_58();

		//obj->GetBlender()->ApplyBlend();
		//obj->GetBlender()->m_38();

		//obj->m_1C0();
	});

	fx::ScriptEngine::RegisterNativeHandler("EXPERIMENTAL_BLEND", [](fx::ScriptContext& context)
	{
		char* entity = (char*)getScriptEntity(context.GetArgument<int>(0));

		if (!entity)
		{
			trace("LOAD_CLONE_SYNC: invalid entity\n");
			return;
		}

		auto obj = *(rage::netObject**)(entity + 208);
		//obj->GetBlender()->m_30();
		obj->GetBlender()->m_58();
	});

	static ConsoleCommand saveCloneCmd("save_clone", [](const std::string& address)
	{
		uintptr_t addressPtr = _strtoui64(address.c_str(), nullptr, 16);
		auto netObj = *(rage::netObject**)(addressPtr + 208);

		static char blah[90000];

		static char bluh[1000];

		rage::netBuffer buffer(bluh, sizeof(bluh));

		auto st = netObj->GetSyncTree();
		st->WriteTree(1, 0, netObj, &buffer, g_queryFunctions->GetTimestamp(), blah, 31, nullptr);

		FILE* f = _wfopen(MakeRelativeCitPath(L"tree.bin").c_str(), L"wb");
		fwrite(buffer.m_data, 1, (buffer.m_curBit / 8) + 1, f);
		fclose(f);
	});

	static ConsoleCommand loadCloneCmd("load_clone", []()
	{
		FILE* f = _wfopen(MakeRelativeCitPath(L"tree.bin").c_str(), L"rb");
		fseek(f, 0, SEEK_END);

		int len = ftell(f);

		fseek(f, 0, SEEK_SET);

		uint8_t data[1000];
		fread(data, 1, 1000, f);

		fclose(f);

		rage::netBuffer buf(data, len);
		buf.m_f1C = 1;

		auto st = getSyncTreeForType(nullptr, 0);

		netSyncTree_ReadFromBuffer(st, 1, 0, &buf, nullptr);

		auto obj = createCloneFuncs[NetObjEntityType::Automobile](rand(), 31, 0, 32);

		if (!netSyncTree_CanApplyToObject(st, obj))
		{
			trace("Couldn't apply object.\n");

			delete obj;
			return;
		}

		st->ApplyToObject(obj, nullptr);
		(*g_objectMgr)->RegisterObject(obj);

		netBlender_SetTimestamp(obj->GetBlender(), g_queryFunctions->GetTimestamp());

		obj->m_1D0();

		obj->GetBlender()->ApplyBlend();
		obj->GetBlender()->m_38();

		obj->m_1C0();

		trace("got game object %llx\n", (uintptr_t)obj->GetGameObject());
	});
#endif
});
