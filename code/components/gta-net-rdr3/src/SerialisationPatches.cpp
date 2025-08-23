#include <StdInc.h>
#include <Hooking.h>

#include <netInterface.h>
#include <netPlayerManager.h>
#include <netObjectMgr.h>
#include <netObject.h>

#include <CrossBuildRuntime.h>
#include <ICoreGameInit.h>
#include <NetLibrary.h>
#include <MinHook.h>

#include <Error.h>
#include <EntitySystem.h>

static ICoreGameInit* icgi;

CNetGamePlayer* __fastcall GetPlayerByIndex(uint8_t index);
CNetGamePlayer* GetPlayerByNetId(uint16_t);

static uint32_t g_respawnPlayerPedEvent_objIdRefOffset;
static uint8_t g_respawnPlayerPedEvent_objIdOffset;

static void (*g_origRespawnPlayerPedEvent_Serialize)(void*, rage::CSyncDataReader*);
static void RespawnPlayerPedEvent_Serialize(void* self, rage::CSyncDataReader* syncData)
{
	g_origRespawnPlayerPedEvent_Serialize(self, syncData);

	if (!icgi->OneSyncEnabled)
	{
		return;
	}

	if (syncData->m_type == rage::SYNC_DATA_READER || syncData->m_type == rage::SYNC_DATA_WRITER)
	{
		auto objectIdRef = (uint16_t*)((char*)self + g_respawnPlayerPedEvent_objIdRefOffset);
		auto objectId = (uint16_t*)((char*)self + g_respawnPlayerPedEvent_objIdOffset);

		// Object ID Mapping isn't needed here.
		*objectId = *objectIdRef;
	}
}

struct CNetworkEventComponentControlBase
{
	void* vtable;
	uint16_t m_vehicleId;
	uint16_t m_entityId;
	uint8_t unk_C;
	uint8_t m_request;
	uint8_t m_isAccepted;
	uint8_t unk_F;
	uint8_t unk_10;
	char pad_11[7];
};

struct CNetworkEventVehComponentControl : CNetworkEventComponentControlBase
{
	uint16_t m_occupantObjectId;
	uint8_t m_isSeat;
	BYTE unk_1B;
	uint16_t unk_1D;
	BYTE unk_1E;
	char pad_1F[1];
};

namespace rage
{
struct scriptIdBase
{
	void* vtable;
};

struct scriptId : scriptIdBase
{
	uint32_t m_scriptHash;
};
}

struct CGameScriptId : rage::scriptId
{
	uint32_t unk_10;
	uint32_t m_positionHash;
	union
	{
		struct
		{
			uint16_t m_value;
			uint16_t unk_1A;
		} m_objectId;
		uint32_t m_identifier;
	};
	uint32_t m_uniquifier;
	BYTE m_isObjectId;
	char pad_21[3];
};

static char (*g_origGameScriptId_Read)(CGameScriptId*, rage::datBitBuffer*);
static char GameScriptId_Read(CGameScriptId* self, rage::datBitBuffer* buffer)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGameScriptId_Read(self, buffer);
	}

	buffer->ReadInteger(&self->m_scriptHash, 32);
	buffer->ReadInteger(&self->unk_10, 32);

	bool hasPositionHash = false;
	if (buffer->ReadBit(&hasPositionHash) && hasPositionHash)
	{
		buffer->ReadInteger(&self->m_positionHash, 32);
	}
	else
	{
		self->m_positionHash = 0;
	}

	uint32_t identifier = -1;

	bool hasIdentifier = false;
	if (buffer->ReadBit(&hasIdentifier) && hasIdentifier)
	{
		bool isObjectId = false;

		if (buffer->ReadBit(&isObjectId) && isObjectId)
		{
			buffer->ReadInteger((uint32_t*)&self->m_objectId.m_value, icgi->OneSyncBigIdEnabled ? 16 : 13);
			identifier = self->m_objectId.m_value;
		}
		else
		{
			bool isBigIdentifier = false;
			buffer->ReadBit(&isBigIdentifier);

			buffer->ReadInteger(&self->m_identifier, isBigIdentifier ? 32 : 9);
			identifier = self->m_identifier;
		}
	}
	else
	{
		self->m_identifier = -1;
	}

	self->m_uniquifier = (identifier == -1) ? (self->m_positionHash + self->m_scriptHash) : (identifier + self->m_scriptHash + 1);

	return true;
}

static char (*g_origGameScriptId_Write)(CGameScriptId*, rage::datBitBuffer*);
static char GameScriptId_Write(CGameScriptId* self, rage::datBitBuffer* buffer)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGameScriptId_Write(self, buffer);
	}

	buffer->WriteUns(self->m_scriptHash, 32);
	buffer->WriteUns(self->unk_10, 32);

	bool hasPositionHash = (self->m_positionHash != 0);

	buffer->WriteBit(hasPositionHash);
	if (hasPositionHash)
	{
		buffer->WriteUns(self->m_positionHash, 32);
	}

	bool hasIdentifier = (self->m_identifier != -1);

	buffer->WriteBit(hasIdentifier);
	if (hasIdentifier)
	{
		buffer->WriteBit(self->m_isObjectId);
		if (self->m_isObjectId)
		{
			buffer->WriteWord(self->m_objectId.m_value, icgi->OneSyncBigIdEnabled ? 16 : 13);
		}
		else
		{
			bool isBigIdentifier = (self->m_identifier >= 512);

			buffer->WriteBit(isBigIdentifier);
			buffer->WriteUns(self->m_identifier, isBigIdentifier ? 32 : 9);
		}
	}

	return true;
}

static bool(*g_origNetworkEventComponentControlBase_Write)(CNetworkEventComponentControlBase*, rage::datBitBuffer*);
static bool NetworkEventComponentControlBase_Write(CNetworkEventComponentControlBase* self, rage::datBitBuffer* buffer)
{
	static_assert(sizeof(CNetworkEventComponentControlBase) == 0x18);
	static_assert(sizeof(CNetworkEventVehComponentControl) == 0x20);

	trace("NetworkEventComponentControlBaseWrite | [vehicleId: %d] [entityId: %d] [unk_C %d] [m_request %d] \n", self->m_vehicleId, self->m_entityId, self->unk_C, self->m_request);

	if (!icgi->OneSyncEnabled)
	{
		return g_origNetworkEventComponentControlBase_Write(self, buffer);
	}

	static const int kObjectIdLength = icgi->OneSyncBigIdEnabled ? 16 : 13;

	buffer->WriteWord(self->m_vehicleId, kObjectIdLength);
	buffer->WriteWord(self->m_entityId, kObjectIdLength);
	buffer->WriteWord(self->unk_C, 6);
	bool result = buffer->WriteBit(self->m_request);
	return result;
}

static void(*g_origNetworkEventVehComponentControl_Write)(CNetworkEventVehComponentControl*, rage::datBitBuffer*);
static void NetworkEventVehComponentControl_Write(CNetworkEventVehComponentControl* self, rage::datBitBuffer* buffer)
{
	trace("NetworkEventVehComponentControlWrite | [occupantObjectID: %d] [m_isAccepted: %d] [m_isSeat: %d]\n", self->m_occupantObjectId, self->m_isAccepted, self->m_isSeat);

	if (!icgi->OneSyncEnabled)
	{
		return g_origNetworkEventVehComponentControl_Write(self, buffer);
	}

	static const int kObjectIdLength = icgi->OneSyncBigIdEnabled ? 16 : 13;

	buffer->WriteBit(self->m_isSeat);
	if (!self->m_request && self->m_isSeat)
	{
		buffer->WriteWord(self->m_occupantObjectId, kObjectIdLength);
		return;
	}
}

static void(*g_origNetworkEventVehComponentControl_Reply)(CNetworkEventVehComponentControl*, rage::datBitBuffer*);
static void NetworkEventVehComponentControl_Reply(CNetworkEventVehComponentControl* self, rage::datBitBuffer* buffer)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origNetworkEventVehComponentControl_Reply(self, buffer);
	}

	static const int kObjectIdLength = icgi->OneSyncBigIdEnabled ? 16 : 13;

	if (!self->m_isAccepted)
	{
		return;
	}

	buffer->WriteBit(self->m_isSeat);
	if (self->m_isSeat)
	{
		buffer->WriteWord(self->m_occupantObjectId, kObjectIdLength);
	}
}

static uint32_t (*g_origGetPhysicalMappedObjectId)(CPhysical*);
static uint32_t GetPhysicalMappedObjectId(CPhysical* physical)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetPhysicalMappedObjectId(physical);
	}

	const auto netObject = (rage::netObject*)physical->GetNetObject();

	if (!netObject)
	{
		return physical->GetUnkId();
	}

	return netObject->GetObjectId();
}

class CScriptEntityStateChangeEvent
{
public:
	class IScriptEntityStateParametersBase
	{
	public:
		virtual ~IScriptEntityStateParametersBase() = 0;
	};

	class CSetVehicleExclusiveDriver : IScriptEntityStateParametersBase
	{
	public:
		uint16_t m_objectId;
		char m_pad[2];
		uint32_t m_index;
	};

	class CSettingOfLookAtEntity : IScriptEntityStateParametersBase
	{
	public:
		uint16_t m_objectId;
		char m_pad[2];
		uint32_t m_flags;
		int unk_10;
		uint32_t unk_14;
	};
};

static void (*g_origSetVehicleExclusiveDriver_Write)(CScriptEntityStateChangeEvent::CSetVehicleExclusiveDriver*, rage::datBitBuffer*);
static void SetVehicleExclusiveDriver_Write(CScriptEntityStateChangeEvent::CSetVehicleExclusiveDriver* self, rage::datBitBuffer* buffer)
{
	static_assert(sizeof(CScriptEntityStateChangeEvent::CSetVehicleExclusiveDriver) == 0x10);
	static_assert(offsetof(CScriptEntityStateChangeEvent::CSetVehicleExclusiveDriver, m_objectId) == 8);
	static_assert(offsetof(CScriptEntityStateChangeEvent::CSetVehicleExclusiveDriver, m_index) == 12);

	if (!icgi->OneSyncEnabled)
	{
		return g_origSetVehicleExclusiveDriver_Write(self, buffer);
	}

	buffer->WriteWord(self->m_objectId, icgi->OneSyncBigIdEnabled ? 16 : 13);
	buffer->WriteWord(self->m_index, 2);
}

static void (*g_origSettingOfLookAtEntity_Write)(CScriptEntityStateChangeEvent::CSettingOfLookAtEntity*, rage::datBitBuffer*);
static void SettingOfLookAtEntity_Write(CScriptEntityStateChangeEvent::CSettingOfLookAtEntity* self, rage::datBitBuffer* buffer)
{
	static_assert(sizeof(CScriptEntityStateChangeEvent::CSettingOfLookAtEntity) == 0x18);

	if (!icgi->OneSyncEnabled)
	{
		return g_origSettingOfLookAtEntity_Write(self, buffer);
	}

	buffer->WriteWord(self->m_objectId, icgi->OneSyncBigIdEnabled ? 16 : 13);
	buffer->WriteWord(self->m_flags, 18);
	buffer->WriteWord(self->unk_14, 10);
}


static constexpr int kPlayerNetIdLength = 16;

static hook::cdecl_stub<void(rage::CSyncDataBase*, uint8_t*, char*)> _syncDataBase_SerializePlayerIndex([]()
{
	return hook::get_call(hook::get_pattern("0F B6 12 48 8B 05 ? ? ? ? 44", 0x1D));
});

static void (*g_origSyncDataReader_SerializePlayerIndex)(rage::CSyncDataReader*, uint8_t*, char*);
static void SyncDataReader_SerializePlayerIndex(rage::CSyncDataReader* syncData, uint8_t* index, char* prefix)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataReader_SerializePlayerIndex(syncData, index, prefix);
	}

	uint32_t playerNetId = -1;
	uint8_t playerIndex = -1;

	if (syncData->m_buffer->ReadInteger(&playerNetId, kPlayerNetIdLength))
	{
		if (auto player = GetPlayerByNetId(playerNetId))
		{
			playerIndex = player->physicalPlayerIndex();
		}
	}

	*index = playerIndex;

	_syncDataBase_SerializePlayerIndex(syncData, index, prefix);
}

static void (*g_origSyncDataWriter_SerializePlayerIndex)(rage::CSyncDataWriter*, uint8_t*, char*);
static void SyncDataWriter_SerializePlayerIndex(rage::CSyncDataWriter* syncData, uint8_t* index, char* prefix)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataWriter_SerializePlayerIndex(syncData, index, prefix);
	}

	uint16_t playerNetId = -1;

	if (auto player = GetPlayerByIndex(*index))
	{
		playerNetId = g_netIdsByPlayer[player];
	}

	_syncDataBase_SerializePlayerIndex(syncData, index, prefix);

	syncData->m_buffer->WriteUns(playerNetId, kPlayerNetIdLength);
}

static void (*g_origSyncDataSizeCalculator_SerializePlayerIndex)(rage::CSyncDataSizeCalculator*);
static void SyncDataSizeCalculator_SerializePlayerIndex(rage::CSyncDataSizeCalculator* syncData)
{
	static_assert(offsetof(rage::CSyncDataSizeCalculator, m_size) == 0x18);

	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataSizeCalculator_SerializePlayerIndex(syncData);
	}

	syncData->m_size += kPlayerNetIdLength;
}

static void (*g_origSyncDataSizeCalculator_SerializeObjectId)(rage::CSyncDataSizeCalculator*);
static void SyncDataSizeCalculator_SerializeObjectId(rage::CSyncDataSizeCalculator* syncData)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataSizeCalculator_SerializeObjectId(syncData);
	}

	syncData->m_size += (icgi->OneSyncBigIdEnabled ? 16 : 13);
}

static hook::cdecl_stub<void(rage::CSyncDataBase*, uint16_t*, char*)> _syncDataBaseSerializeObjectId([]()
{
	return hook::get_call(hook::get_pattern("0F B7 03 B9 3F 1F 00 00 66 FF C8 66 3B C1 76 04", -5));
});

static void (*g_origSyncDataWriter_SerializeObjectIdPrefix)(rage::CSyncDataWriter*, uint16_t*, char*);
static void SyncDataWriter_SerializeObjectIdPrefix(rage::CSyncDataWriter* syncData, uint16_t* objectId, char* prefix)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataWriter_SerializeObjectIdPrefix(syncData, objectId, prefix);
	}

	_syncDataBaseSerializeObjectId(syncData, objectId, prefix);

	syncData->m_buffer->WriteWord(*objectId, icgi->OneSyncBigIdEnabled ? 16 : 13);
}

static void (*g_origSyncDataReader_SerializeObjectIdPrefix)(rage::CSyncDataReader*, uint16_t*, char*);
static void SyncDataReader_SerializeObjectIdPrefix(rage::CSyncDataReader* syncData, uint16_t* objectId, char* prefix)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataReader_SerializeObjectIdPrefix(syncData, objectId, prefix);
	}

	uint32_t netObjectId = 0;

	if (syncData->m_buffer->ReadInteger(&netObjectId, icgi->OneSyncBigIdEnabled ? 16 : 13))
	{
		*objectId = netObjectId;
	}

	_syncDataBaseSerializeObjectId(syncData, objectId, prefix);
}

static void (*g_origSyncDataWriter_SerializeObjectIdStatic)(rage::CSyncDataWriter*, uint16_t*);
static void SyncDataWriter_SerializeObjectIdStatic(rage::CSyncDataWriter* syncData, uint16_t* objectId)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataWriter_SerializeObjectIdStatic(syncData, objectId);
	}

	syncData->m_buffer->WriteUns(*objectId, icgi->OneSyncBigIdEnabled ? 16 : 13);
}

static void (*g_origSyncDataReader_SerializeObjectIdStatic)(rage::CSyncDataReader*, uint16_t*);
static void SyncDataReader_SerializeObjectIdStatic(rage::CSyncDataReader* syncData, uint16_t* objectId)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSyncDataReader_SerializeObjectIdStatic(syncData, objectId);
	}

	uint32_t netObjectId = 0;

	if (syncData->m_buffer->ReadInteger(&netObjectId, icgi->OneSyncBigIdEnabled ? 16 : 13))
	{
		*objectId = netObjectId;
	}
}

static HookFunction hookFunction([]()
{
	// Patch `CGameScriptId::Write` and `CGameScriptId::Read` methods.
	{
		static_assert(sizeof(rage::scriptIdBase) == 0x8);
		static_assert(sizeof(rage::scriptId) == 0x10);
		static_assert(sizeof(CGameScriptId) == 0x28);

		static constexpr int kGameScriptIdReadIndex = 5;
		static constexpr int kGameScriptIdWriteIndex = 6;

		const auto gameScriptIdVtable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8B F9 41 8A D8 48 83 C1 10 48 89 01 89 69 08", -15));

		g_origGameScriptId_Read = (decltype(g_origGameScriptId_Read))gameScriptIdVtable[kGameScriptIdReadIndex];
		hook::put(&gameScriptIdVtable[kGameScriptIdReadIndex], (uintptr_t)GameScriptId_Read);

		g_origGameScriptId_Write = (decltype(g_origGameScriptId_Write))gameScriptIdVtable[kGameScriptIdWriteIndex];
		hook::put(&gameScriptIdVtable[kGameScriptIdWriteIndex], (uintptr_t)GameScriptId_Write);
	}

	MH_Initialize();

	// CSyncedDataWriter/CSyncedDataReader::SerializeObjectID (static).
	MH_CreateHook(hook::get_pattern("66 3B C1 76 ? 33 C0 EB ? 0F B7 0A", -0xE), SyncDataWriter_SerializeObjectIdStatic, (void**)&g_origSyncDataWriter_SerializeObjectIdStatic);
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 53 0A")), SyncDataReader_SerializeObjectIdStatic, (void**)&g_origSyncDataReader_SerializeObjectIdStatic);

	// CSyncedDataWriter/CSyncedDataReader::SerializeObjectID (prefixed).
	MH_CreateHook(hook::get_pattern("E8 ? ? ? ? 0F B7 03 B9", -0x10), SyncDataWriter_SerializeObjectIdPrefix, (void**)&g_origSyncDataWriter_SerializeObjectIdPrefix);
	MH_CreateHook(hook::get_pattern("B9 ? ? ? ? 49 8B 04 ? 8A 14 01", -0x36), SyncDataReader_SerializeObjectIdPrefix, (void**)&g_origSyncDataReader_SerializeObjectIdPrefix);

	// patch sync data size calculator allowing more bits
	{
		static constexpr int kSizeCalculatorSerializePlayerIndex = 25;
		static constexpr int kSizeCalculatorSerializeObjectId1 = 26;
		static constexpr int kSizeCalculatorSerializeObjectId2 = 27;
		static constexpr int kSizeCalculatorSerializeObjectId3 = 28;

		const auto sizeCalculatorVtable = hook::get_address<uintptr_t*>(hook::get_pattern("B8 BF FF 00 00 48 8B CF 66 21 87", 27));

		g_origSyncDataSizeCalculator_SerializePlayerIndex = (decltype(g_origSyncDataSizeCalculator_SerializePlayerIndex))sizeCalculatorVtable[kSizeCalculatorSerializePlayerIndex];
		hook::put(&sizeCalculatorVtable[kSizeCalculatorSerializePlayerIndex], (uintptr_t)SyncDataSizeCalculator_SerializePlayerIndex);
		g_origSyncDataSizeCalculator_SerializeObjectId = (decltype(g_origSyncDataSizeCalculator_SerializeObjectId))sizeCalculatorVtable[kSizeCalculatorSerializeObjectId1];

#if _DEBUG
		assert(g_origSyncDataSizeCalculator_SerializeObjectId == (decltype(g_origSyncDataSizeCalculator_SerializePlayerIndex))sizeCalculatorVtable[kSizeCalculatorSerializeObjectId1] && 
			g_origSyncDataSizeCalculator_SerializeObjectId == (decltype(g_origSyncDataSizeCalculator_SerializePlayerIndex))sizeCalculatorVtable[kSizeCalculatorSerializeObjectId2] && 
			g_origSyncDataSizeCalculator_SerializeObjectId == (decltype(g_origSyncDataSizeCalculator_SerializePlayerIndex))sizeCalculatorVtable[kSizeCalculatorSerializeObjectId3]
		);
#endif

		hook::put(&sizeCalculatorVtable[kSizeCalculatorSerializeObjectId1], (uintptr_t)SyncDataSizeCalculator_SerializeObjectId);
		hook::put(&sizeCalculatorVtable[kSizeCalculatorSerializeObjectId2], (uintptr_t)SyncDataSizeCalculator_SerializeObjectId);
		hook::put(&sizeCalculatorVtable[kSizeCalculatorSerializeObjectId3], (uintptr_t)SyncDataSizeCalculator_SerializeObjectId);
	}

	// patch SerializePlayerIndex methods of sync data reader/writer
	MH_CreateHook(hook::get_pattern("80 3B 20 73 ? 65 4C 8B 0C", -0x2F), SyncDataReader_SerializePlayerIndex, (void**)&g_origSyncDataReader_SerializePlayerIndex);
	MH_CreateHook(hook::get_pattern("41 B2 3F 48 8D 54 24 30 44 88", -30), SyncDataWriter_SerializePlayerIndex, (void**)&g_origSyncDataWriter_SerializePlayerIndex);

	// Patch methods of `NetworkEventComponentControlBase` and derivative classes. We don't need to patch reading methods as these are using generic SerializeObjectID method that is already patched.
	MH_CreateHook(hook::get_pattern("57 48 83 EC ? 0F B7 41 ? BD ? ? ? ? 66 FF C8", -0xF), NetworkEventComponentControlBase_Write, (void**)&g_origNetworkEventComponentControlBase_Write);
	MH_CreateHook(hook::get_pattern("38 53 1A 74 2E", -0x22), NetworkEventVehComponentControl_Write, (void**)&g_origNetworkEventVehComponentControl_Write);
	MH_CreateHook(hook::get_pattern("38 59 ? 74 ? 8A 51", -0x17), NetworkEventVehComponentControl_Reply, (void**)&g_origNetworkEventVehComponentControl_Reply);
	// `CScriptEntityStateChangeEvent` related, we don't need to patch reading methods as these are using generic SerializeObjectID method that is already patched.
	MH_CreateHook(hook::get_pattern("8B 53 0C 41 B8 02 00 00 00 48 8B CF 48 8B", -0x45), SetVehicleExclusiveDriver_Write, (void**)&g_origSetVehicleExclusiveDriver_Write);
	MH_CreateHook(hook::get_pattern("8B 53 14 41 B8 0A 00 00 00 48 8B CF 48 8B", -0x56), SettingOfLookAtEntity_Write, (void**)&g_origSettingOfLookAtEntity_Write);
	// Remove Object ID being checked against Object ID Mapping
	MH_CreateHook(hook::get_pattern("74 25 0F B7 48 42", -0xC), GetPhysicalMappedObjectId, (void**)&g_origGetPhysicalMappedObjectId);

	// RespawnPlayerPedEvent offset
	{
		g_respawnPlayerPedEvent_objIdRefOffset = *hook::get_pattern<uint8_t>("66 89 7B ? 44 89 73", 3);
		g_respawnPlayerPedEvent_objIdOffset = *hook::get_pattern<uint8_t>("66 44 89 73 ? E8 ? ? ? ? 48 85 C0", 4);
	}

	MH_CreateHook(hook::get_call(hook::get_pattern("48 89 5C 24 ? 57 48 83 EC ? 48 8B DA 48 8B F9 E8 ? ? ? ? 48 83 64 24 ? ? 48 8D 05", 57)), RespawnPlayerPedEvent_Serialize, (void**)&g_origRespawnPlayerPedEvent_Serialize);
	MH_EnableHook(MH_ALL_HOOKS);

	// removing object id mapping in CTaskClimbLadder
	{
		auto location = hook::get_pattern("0F B7 50 42 41 B9 3F 1F 00 00", 4);
		static struct : public jitasm::Frontend
		{
			intptr_t RetSuccess;
			intptr_t RetFail;

			void Init(const intptr_t retSuccess, const intptr_t retFail)
			{
				this->RetSuccess = retSuccess;
				this->RetFail = retFail;
			}

			virtual void InternalMain() override
			{
				sub(rsp, 0x20);
				mov(rcx, rax);
				mov(rdx, r8);

				mov(rax, reinterpret_cast<uintptr_t>(&CompareObjectIds));
				call(rax);

				add(rsp, 0x20);

				test(eax, eax);
				jz("fail");

				mov(rax, RetSuccess);
				jmp(rax);

				L("fail");
				mov(rax, RetFail);
				jmp(rax);
			}

			static bool CompareObjectIds(rage::netObject* leftObj, rage::netObject* rightObj)
			{
				if (!leftObj || !rightObj)
				{
					return false;
				}

				auto lId = leftObj->GetObjectId();
				auto rId = rightObj->GetObjectId();

				if (lId == 0 || rId == 0)
				{
					return false;
				}

				return rId <= lId;
			}
		} climbLadderStub;

		auto loc = hook::get_pattern("48 8B 0D ? ? ? ? 8D 42");
		auto retFail = (intptr_t)loc + 48;
		auto retSuccess = (intptr_t)loc + 52;
		climbLadderStub.Init(retSuccess, retFail);

		hook::nop(location, 0x1D);
		hook::jump(location, climbLadderStub.GetCode());
	}
});

static InitFunction initFunction([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
		icgi = Instance<ICoreGameInit>::Get();
	});
});
