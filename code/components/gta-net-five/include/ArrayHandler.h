#pragma once

#include <rlNetBuffer.h>
#include <NetworkPlayerMgr.h>

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
#ifdef GTA_FIVE
	uint8_t m_pad[244 - 8]; // +8
	uint16_t m_index; // 244
	uint16_t m_count; // 246
	uint8_t m_unk; // 248
	uint8_t m_elementSize; // 249
	uint8_t m_pad2[14]; // 250
	void* m_array; // 264
#elif IS_RDR3
	uint8_t m_pad[308 - 8]; // +8
	uint16_t m_index; // 308
	uint8_t m_pad2[130]; // 310
	uint16_t m_count; // 440
	uint8_t m_pad3[6]; // 442
	uint8_t m_elementSize; // 448
	uint8_t m_pad4[23]; // 449
	void* m_array; // 472
#endif
};

class netArrayManager
{
public:
	netArrayHandlerBase* GetArrayHandler(int index, void* identifier);
};
}

struct ArrayHandlerInfo
{
	std::vector<size_t> hashes;
};
