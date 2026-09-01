#pragma once

#include <rlNetBuffer.h>
#include <NetworkPlayerMgr.h>
#include <XBRVirtual.h>

#include <boost/preprocessor/repeat.hpp>

namespace rage
{
#ifdef GTA_FIVE
class netArrayHandlerBase : XBR_VIRTUAL_BASE(2060, 43 * 8, 1)
#else
class netArrayHandlerBase
#endif
{
public:
#define DEFINE_VF(z, n, text) \
	XBR_VIRTUAL_METHOD(void, m_##n, ())

#ifdef IS_RDR3
	virtual ~netArrayHandlerBase() = default;

	BOOST_PP_REPEAT(42, DEFINE_VF, );
#else
	BOOST_PP_REPEAT(43, DEFINE_VF, );
#endif

#undef DEFINE_VF

	XBR_VIRTUAL_METHOD(bool, IsElementEmpty, (uint32_t element))

	XBR_VIRTUAL_METHOD(void, SetElementEmpty, (uint32_t element))

	XBR_VIRTUAL_METHOD(void, WriteElementIndex, (rage::datBitBuffer& buffer, uint32_t element))

	XBR_VIRTUAL_METHOD(void, ReadElementIndex, (rage::datBitBuffer& buffer, uint32_t& element))

	XBR_VIRTUAL_METHOD(bool, IsValidIndex, (uint32_t element))

	XBR_VIRTUAL_METHOD(void, RecalculateDirtyElements, ())

	XBR_VIRTUAL_METHOD(void, ResetElementSyncData, (uint32_t element))

	XBR_VIRTUAL_METHOD(void, DoPostReadProcessing, ())

	XBR_VIRTUAL_METHOD(void, DoPostElementReadProcessing, (uint32_t element))

	// we'll probably need to pass `force` as we don't have any *real* sender data
	XBR_VIRTUAL_METHOD(bool, CanApplyElementData, (uint32_t element, const rage::netPlayer& sender, bool force))

	XBR_VIRTUAL_METHOD(void, ExtractDataForSerialising, (uint32_t elem))

	XBR_VIRTUAL_METHOD(void, WriteElement, (rage::datBitBuffer& buffer, uint32_t elem, void* logger))

	XBR_VIRTUAL_METHOD(void, ReadElement, (rage::datBitBuffer& buffer, uint32_t elem, void* logger))

	XBR_VIRTUAL_METHOD(void, LogElement, (uint32_t elem, void* logger))

	XBR_VIRTUAL_METHOD(uint32_t, GetCurrentElementSizeInBits, (uint32_t elem))

	XBR_VIRTUAL_METHOD(void, ApplyElementData, (uint32_t element, const rage::netPlayer& sender))

	inline uint32_t GetSize()
	{
		return m_count * m_elementSize;
	}

public:
#ifdef GTA_FIVE
	uint8_t m_pad[244 - 8]; // +8
	uint16_t m_index; // 244
	uint16_t m_count; // 246
	uint8_t m_numElementsInUse; // 248
	uint8_t m_elementSize; // 249
	uint8_t m_pad2[14]; // 250
	void* m_array; // 264
#elif IS_RDR3
	uint8_t m_pad[308 - 8]; // +8
	uint16_t m_index; // 308
	uint8_t m_pad2[130]; // 310
	uint16_t m_count; // 440
	uint16_t m_numElementsInUse; // 442
	uint8_t m_pad3[4]; // 444
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
