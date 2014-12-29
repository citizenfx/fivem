/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "scrBind.h"

// setup gtaDrawable format
#if defined(GTA_NY)
#define RAGE_FORMATS_GAME ny
#endif

#include <gtaDrawable.h>

using namespace rage::RAGE_FORMATS_GAME;

class ScriptLodGroup
{
private:
	Vector3 m_minBound;
	Vector3 m_maxBound;
	Vector3 m_center;
	float m_radius;

	uint32_t m_setFlags;

public:
	ScriptLodGroup();

	inline void SetMinBound(float x, float y, float z)
	{
		m_minBound = Vector3(x, y, z);
		m_setFlags |= 1;
	}

	inline void SetMaxBound(float x, float y, float z)
	{
		m_maxBound = Vector3(x, y, z);
		m_setFlags |= 2;
	}

	inline void SetCenter(float x, float y, float z)
	{
		m_center = Vector3(x, y, z);
		m_setFlags |= 4;
	}

	inline void SetRadius(float radius)
	{
		m_radius = radius;
		m_setFlags |= 8;
	}

	inline bool IsValid()
	{
		if ((m_setFlags & 15) != 15)
		{
			return false;
		}

		return true;
	}
};

ScriptLodGroup::ScriptLodGroup()
	: m_radius(0.0f), m_setFlags(0)
{

}

class ScriptDrawable
{
private:
	ScriptLodGroup m_lodGroup;

public:
	ScriptDrawable();
	~ScriptDrawable();

	inline ScriptLodGroup* GetLodGroup()
	{
		return &m_lodGroup;
	}
};

ScriptDrawable::ScriptDrawable()
{
	scrBindAddSafePointer(&m_lodGroup);
}

ScriptDrawable::~ScriptDrawable()
{
	// TODO: delete safe pointer
}

static InitFunction initFunction([] ()
{
	using namespace rage;

	scrBindClass<ScriptDrawable>()
		.AddConstructor<void(*)()>("CREATE_DRAWABLE")
		.AddMethod("GET_DRAWABLE_LODGROUP", &ScriptDrawable::GetLodGroup);

	scrBindClass<ScriptLodGroup>()
		.AddMethod("SET_LODGROUP_BOUNDS_MIN", &ScriptLodGroup::SetMinBound)
		.AddMethod("SET_LODGROUP_BOUNDS_MAX", &ScriptLodGroup::SetMaxBound)
		.AddMethod("SET_LODGROUP_CENTER", &ScriptLodGroup::SetCenter)
		.AddMethod("SET_LODGROUP_RADIUS", &ScriptLodGroup::SetRadius);

	/*scrEngine::OnScriptInit.Connect([] ()
	{
		void* drawable = NativeInvoke::Invoke<0x33486661, void*>(1, 2, "lovely");

		NativeInvoke::Invoke<0x7DFC57A4, int>(drawable, 1, 2, 3, 4, 5, 6);

		__debugbreak();
	}, 100);*/
});