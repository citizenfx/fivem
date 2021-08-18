#pragma once

#include <EntitySystem.h>

// fwRegdRef
namespace rage
{
template<typename T>
class fwRegdRef
{
public:
	inline fwRegdRef()
	{
	}

	inline fwRegdRef(T* value)
	{
		Reset(value);
	}

	inline fwRegdRef(const fwRegdRef& right)
	{
		Reset(right.ref);
	}

	inline fwRegdRef(fwRegdRef&& right)
	{
		auto ref = right.ref;
		right.Reset(nullptr);
		Reset(ref);
	}

	~fwRegdRef()
	{
		Reset(nullptr);
	}

	void Reset(T* ref)
	{
		if (ref != this->ref)
		{
			if (this->ref)
			{
				this->ref->RemoveKnownRef((void**)&this->ref);
			}

			this->ref = ref;

			if (ref)
			{
				ref->AddKnownRef((void**)&this->ref);
			}
		}
	}

	inline auto& operator=(const fwRegdRef& right)
	{
		Reset(right.ref);
		return *this;
	}

	inline auto& operator=(T* right)
	{
		Reset(right);
		return *this;
	}

	inline bool operator<(const fwRegdRef& right) const
	{
		return ref < right.ref;
	}

	inline bool operator==(const fwRegdRef& right) const
	{
		return ref == right.ref;
	}

	inline bool operator==(const fwEntity* right) const
	{
		return ref == right;
	}

	inline bool operator!=(const fwRegdRef& right) const
	{
		return ref != right.ref;
	}

	inline T* operator->() const
	{
		return ref;
	}

	inline T& operator*() const
	{
		return *ref;
	}

	inline operator bool() const
	{
		return (ref != nullptr);
	}

	template<typename TOther>
	inline operator TOther*() const
	{
		return (TOther*)ref;
	}

private:
	T* ref = nullptr;
};
}
// end

enum class EntityExtensionClassId : int
{
	MapDataOwner = 64,
	InstantiatedObjectRef = 65,
};

class InstantiatedObjectRefExtension : public rage::fwExtension
{
public:
	InstantiatedObjectRefExtension()
	{
	}
	virtual ~InstantiatedObjectRefExtension() = default;

	virtual int GetExtensionId() const override
	{
		return GetClassId();
	}

	static int GetClassId()
	{
		return (int)EntityExtensionClassId::InstantiatedObjectRef;
	}

	void SetObjectRef(fwEntity* object)
	{
		objectRef = object;
	}

	fwEntity* GetObjectRef()
	{
		return objectRef;
	}

	void RemoveObjectRef()
	{
		objectRef = nullptr;
	}

private:
	rage::fwRegdRef<fwEntity> objectRef = nullptr;
};
