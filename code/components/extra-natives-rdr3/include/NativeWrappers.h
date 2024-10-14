#pragma once

#include "StdInc.h"

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

	inline void* GetNetObject() const
	{
		static_assert(offsetof(fwEntity, m_netObject) == 224, "wrong GetNetObject");
		return m_netObject;
	}

private:
	char m_pad[216];
	void* m_netObject;
};

class CPickup : public fwEntity
{

};

class CObject : public fwEntity
{

};

class CVehicle : public fwEntity
{

};

class CPed : public fwEntity
{

};
