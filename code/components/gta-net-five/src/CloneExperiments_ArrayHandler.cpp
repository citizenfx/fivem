#include <StdInc.h>
#include <Hooking.h>

#include <rlNetBuffer.h>
#include <NetworkPlayerMgr.h>

#include <NetLibrary.h>

#include <boost/preprocessor/repeat.hpp>

namespace rage
{
class netArrayHandlerBase
{
public:
	virtual ~netArrayHandlerBase() = default;

#define DEFINE_VF(z, n, text) \
	virtual void m_##n() = 0;

	BOOST_PP_REPEAT(42, DEFINE_VF, );

#undef DEFINE_VF

	virtual bool IsElementEmpty(uint32_t element) = 0;

	virtual void SetElementEmpty(uint32_t element) = 0;

	virtual void WriteElementIndex(const rage::netPlayer& player, rage::datBitBuffer& buffer, uint32_t) = 0;

	virtual void ReadElementIndex(const rage::netPlayer& player, rage::datBitBuffer& buffer, uint32_t&) = 0;

	virtual bool IsValidIndex(uint32_t) = 0;

	virtual void RecalculateDirtyElements() = 0;

	virtual void ResetElementSyncData(uint32_t element) = 0;

	virtual void DoPostReadProcessing() = 0;

	virtual void DoPostElementReadProcessing(uint32_t element) = 0;

	// we'll probably need to pass `force` as we don't have any *real* sender data
	virtual bool CanApplyElementData(uint32_t element, const rage::netPlayer& sender, bool force) = 0;

	virtual void ExtractDataForSerialising(uint32_t elem) = 0;

	virtual void WriteElement(rage::datBitBuffer& buffer, uint32_t elem, void* logger) = 0;

	virtual void ReadElement(rage::datBitBuffer& buffer, uint32_t elem, void* logger) = 0;

	virtual void LogElement(uint32_t elem, void* logger) = 0;

	virtual uint32_t GetCurrentElementSizeInBits(uint32_t elem) = 0;

	virtual void ApplyElementData(uint32_t element, const rage::netPlayer& sender) = 0;

	inline uint32_t GetSize()
	{
		return m_count * m_elementSize;
	}

public:
	uint8_t m_pad[244 - 8]; // +8
	uint16_t m_index; // 244
	uint16_t m_count; // 246
	uint8_t m_unk; // 248
	uint8_t m_elementSize; // 249
	uint8_t m_pad2[14]; // 250
	void* m_array; // 264
};

class netArrayManager
{
public:
	netArrayHandlerBase* GetArrayHandler(int index, void* identifier);
};
}

static hook::cdecl_stub<rage::netArrayHandlerBase*(rage::netArrayManager*, int, void*)> _getArrayHandler([]()
{
	return hook::get_call(hook::get_pattern("48 8B 0D ? ? ? ? BA 06 00 00 00 45 33 C0", 0xF));
});

static rage::netArrayManager** g_arrayManager;

namespace rage
{
netArrayHandlerBase* netArrayManager::GetArrayHandler(int index, void* identifier)
{
	return _getArrayHandler(this, index, identifier);
}
}

struct ArrayHandlerInfo
{
	std::vector<size_t> hashes;
};

static std::array<std::unique_ptr<ArrayHandlerInfo>, 16> arrayHandlers;

static auto GetArrayHandlerInfo(int index, rage::netArrayHandlerBase* handler)
{
	if (!arrayHandlers[index])
	{
		arrayHandlers[index] = std::make_unique<ArrayHandlerInfo>();
		arrayHandlers[index]->hashes.resize(handler->m_count);
	}

	return arrayHandlers[index].get();
}

extern NetLibrary* g_netLibrary;

void ArrayManager_Update()
{
	if (!*g_arrayManager)
	{
		return;
	}

	static const int arrayHandlers[] = {
		4, // incidents
		7, // sticky bombs
	};

	for (int arrayIndex : arrayHandlers)
	{
		static net::Buffer outBuffer;
		auto arrayHandler = (*g_arrayManager)->GetArrayHandler(arrayIndex, nullptr);

		if (arrayHandler)
		{
			static thread_local std::vector<uint8_t> data(1024);

			auto info = GetArrayHandlerInfo(arrayIndex, arrayHandler);

			for (int elem = 0; elem < arrayHandler->m_count; elem++)
			{
				memset(data.data(), 0, data.size());

				std::string sv;

				if (!arrayHandler->IsElementEmpty(elem))
				{
					// write element
					rage::datBitBuffer buffer(data.data(), data.size());
					arrayHandler->WriteElement(buffer, elem, nullptr);

					sv = { reinterpret_cast<char*>(data.data()), (buffer.m_curBit / 8) + 1 };
				}

				// check hash for equality
				auto thisHash = std::hash<std::string>()(sv);

				if (thisHash != info->hashes[elem])
				{
					// write the array update
					outBuffer.Reset();

					outBuffer.Write<uint8_t>(arrayIndex);
					outBuffer.Write<uint16_t>(elem);
					outBuffer.Write<uint16_t>(sv.size());
					outBuffer.Write(sv.data(), sv.size());

					g_netLibrary->SendReliableCommand("msgArrayUpdate", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());

					info->hashes[elem] = thisHash;
				}
			}
		}
	}
}

static HookFunction hookFunctionArray([]()
{
	g_arrayManager = hook::get_address<rage::netArrayManager**>(hook::get_pattern("48 8B 0D ? ? ? ? BA 06 00 00 00 45 33 C0", 3));
});

extern std::unordered_map<uint16_t, CNetGamePlayer*> g_playersByNetId;

static InitFunction initFunctionArray([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* lib)
	{
		lib->AddReliableHandler("msgArrayUpdate", [](const char* data, size_t len)
		{
			if (!*g_arrayManager)
			{
				return;
			}

			net::Buffer buf(reinterpret_cast<const uint8_t*>(data), len);

			auto arrayIndex = buf.Read<uint8_t>();
			auto player = buf.Read<uint16_t>();
			auto element = buf.Read<uint32_t>();
			auto length = buf.Read<uint32_t>();

			auto arrayHandler = (*g_arrayManager)->GetArrayHandler(arrayIndex, nullptr);

			if (!arrayHandler)
			{
				return;
			}

			auto playerData = g_playersByNetId[player];

			if (!playerData)
			{
				return;
			}

			std::vector<uint8_t> bufData(length);
			buf.Read(bufData.data(), bufData.size());

			if (!bufData.empty())
			{
				rage::datBitBuffer buffer(bufData.data(), bufData.size());
				buffer.m_f1C = 1;

				arrayHandler->ReadElement(buffer, element, nullptr);

				// apply
				if (arrayHandler->CanApplyElementData(element, *playerData, true))
				{
					arrayHandler->ApplyElementData(element, *playerData);
				}
			}
			else
			{
				arrayHandler->SetElementEmpty(element);
			}

			arrayHandler->DoPostElementReadProcessing(element);
			arrayHandler->DoPostReadProcessing();

			// update hashes
			auto info = GetArrayHandlerInfo(arrayIndex, arrayHandler);

			{
				// write element
				static thread_local std::vector<uint8_t> data(1024);
				memset(data.data(), 0, data.size());

				rage::datBitBuffer buffer(data.data(), data.size());
				arrayHandler->WriteElement(buffer, element, nullptr);

				std::string sv{ reinterpret_cast<char*>(data.data()), (buffer.m_curBit / 8) + 1 };

				auto thisHash = std::hash<std::string>()(sv);
				info->hashes[element] = thisHash;
			}
		});
	});
});
