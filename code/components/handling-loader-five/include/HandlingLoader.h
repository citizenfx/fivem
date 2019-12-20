/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#pragma once

#include <atArray.h>
#include <boost/type_index.hpp>

namespace rage
{
	class fwEntity
	{
	public:
		virtual ~fwEntity() = 0;

		virtual bool IsOfType(uint32_t hash) = 0;

		template<typename T>
		bool IsOfType()
		{
			return reinterpret_cast<T*>(this->IsOfType(HashString(boost::typeindex::type_id<T>().pretty_name().substr(6).c_str())));
		}
	};

	enum class parMemberType : uint8_t
	{
		Int = 6,
		Float = 7,
		Lewca = 8,
		String = 0xB,
		Vector3 = 0x14,
	};

	struct parMember
	{
		uint32_t hash;
		char pad[12];
		parMemberType type;
		// some more
	};

	struct parStructure
	{
		uint32_t hash;
		char pad[12];
		void* parserData;
		parMember** members;
		uint32_t* offsets;
	};
}

class CBaseSubHandlingData
{
public:
	virtual ~CBaseSubHandlingData() = default;
	virtual void* GetParser() = 0;
	virtual int GetUnk() = 0;
	virtual void ProcessOnLoad() = 0;
};

class CHandlingData
{
private:
	uint32_t m_name;
	char m_pad[332]; // 1290, 1365, 1493, 1604
	atArray<CBaseSubHandlingData*> m_subHandlingData;
	// ^ find offset using a variant of 48 85 C9 74 13 BA 04 00 00 00 E8 (and go to the call in there)
	char m_pad2[1000];

public:
	CHandlingData(CHandlingData* orig)
	{
		memcpy(this, orig, sizeof(*this));

		CBaseSubHandlingData* shds[6] = { 0 };

		for (int i = 0; i < m_subHandlingData.GetCount(); i++)
		{
			if (m_subHandlingData.Get(i))
			{
				shds[i] = (CBaseSubHandlingData*)rage::GetAllocator()->allocate(1024, 16, 0);
				memcpy(shds[i], m_subHandlingData.Get(i), 1024);
			}
		}

		m_subHandlingData.m_offset = nullptr;
		m_subHandlingData.Clear();

		m_subHandlingData.Set(0, shds[0]);
		m_subHandlingData.Set(1, shds[1]);
		m_subHandlingData.Set(2, shds[2]);
		m_subHandlingData.Set(3, shds[3]);
		m_subHandlingData.Set(4, shds[4]);
		m_subHandlingData.Set(5, shds[5]);
	}

	virtual ~CHandlingData() = default;

	inline uint32_t GetName()
	{
		return m_name;
	}

	inline atArray<CBaseSubHandlingData*>& GetSubHandlingData()
	{
		return m_subHandlingData;
	}

	void ProcessEntry();
};

class CVehicle
{
private:
	//char m_pad[0x8C0]; // 1290, 1365, 1493
	char m_pad[0x910]; // 1604
	CHandlingData* m_handlingData;
	// find ^ with `85 C0 74 49 48 8B 86 ? ? 00 00 48 8B CE` ??s

public:
	virtual ~CVehicle() = 0;

	inline CHandlingData* GetHandlingData()
	{
		return m_handlingData;
	}

	inline void SetHandlingData(CHandlingData* ptr)
	{
		// Use an alignment byte within CHandlingDataMgr to represent the handling as hooked.
		*((char*)ptr + 28) = 1;
		m_handlingData = ptr;
	}
};
