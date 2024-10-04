#pragma once

#include "ComponentExport.h"
#include <atArray.h>

#ifdef GTA_FIVE
#define GTA_CORE_TARGET GTA_CORE_FIVE
#elif IS_RDR3
#define GTA_CORE_TARGET GTA_CORE_RDR3
#endif

namespace rage
{
	enum class parMemberType : uint8_t
	{
		Bool = 0,
		Int8 = 1,
		UInt8 = 2,
		Int16 = 3,
		UInt16 = 4,
		Int32 = 5,
		UInt32 = 6,
		Float = 7,
		Vector2 = 8,
		Vector3 = 9,
		Vector4 = 10,
		String = 11,
		Struct = 12,
		Array = 13,
		Enum = 14,
		Bitset = 15,
		Map = 16,
		Matrix4x3 = 17,
		Matrix4x4 = 18,
		Vector2_Padded = 19,
		Vector3_Padded = 20,
		Vector4_Padded = 21,
		Matrix3x4 = 22,
		Matrix4x3_2 = 23,
		Matrix4x4_2 = 24,
		Vector1_Padded = 25,
		Flag1_Padded = 26, // for shaders?
		Flag4_Padded = 27,
		Int32_64 = 28,
		Int32_U64 = 29,
		Half = 30,
		Int64 = 31,
		UInt64 = 32,
		Double = 33,
#ifdef IS_RDR3
		Guid = 34,
		Vec2f = 35,
		QuatV = 36,
#endif
	};

	struct parEnumField
	{
		uint32_t hash;
		uint32_t index; // 0xFFFFFFFF for last
	};

	struct parEnumDefinition
	{
		parEnumField* fields;
		const char** names; // NULL if there are no names
	};

	enum class parArrayType : uint8_t
	{
		// type is atArray
		atArray = 0,

		// count*size, trailing integer (after alignment)
		FixedTrailingCount = 1,

		// count*size, fixed
		Fixed = 2,

		// pointer to count*size
		FixedPointer = 3,

		// unknown difference from 2
		Fixed_2 = 4,

		// atArray but with uint32_t index
		atArray32 = 5,

		// pointer with a trailing count at +size
		TrailerInt = 6,

		TrailerByte = 7,

		TrailerShort = 8
	};

	enum class parStructType : uint8_t
	{
		// not a pointer, just the offset
		Inline = 0,

		// other types
		Struct_1 = 1,
		Struct_2 = 2,
		Struct_3 = 3,
		Struct_4 = 4,
	};

	class parStructure;

	struct parMemberDefinition
	{
		uint32_t hash; // +0
		uint32_t pad; // +4
		uint64_t offset; // +8
		parMemberType type; // +16
		union
		{
			parArrayType arrayType; // +17
			parStructType structType;
		};
		uint8_t pad2[2]; // +18
		uint8_t pad3[12]; // +20
		union // +32
		{
			uint32_t arrayElemSize;
			uint64_t __pad;
			parStructure* structure;
		};
		union // +40
		{
			parEnumDefinition* enumData;
			uint32_t arrayElemCount;
		};
		uint16_t enumElemCount; // +48 for enum defs
	};

	class parMember
	{
	public:
		virtual ~parMember() = default;

		parMemberDefinition* m_definition;
		parMember* m_arrayDefinition; // in case of array
	};

	class parStructure
	{
	public:
		virtual ~parStructure() = default;

#if IS_RDR3
		uint8_t m_critSection[0x28];
#endif

		uint32_t m_nameHash; // +8

		char m_pad[4]; // +12

		parStructure* m_baseClass; // +16

#if IS_RDR3
		char m_pad2[32]; // +24
#else
		char m_pad2[24]; // +24
#endif

		atArray<rage::parMember*> m_members; // +48

		char m_pad3[8];

		void* m_newPtr; // +72
		void* (*m_new)();

		void* m_placementNewPtr;
		void* (*m_placementNew)(void*);

		void* m_getTypePtr;
		parStructure* (*m_getType)(void*);

		void* m_deletePtr;
		void(*m_delete)(void*);
	};

	COMPONENT_EXPORT(GTA_CORE_TARGET) rage::parStructure* GetStructureDefinition(const char* structType);

	COMPONENT_EXPORT(GTA_CORE_TARGET) rage::parStructure* GetStructureDefinition(uint32_t structHash);
}

#undef GTA_CORE_TARGET
