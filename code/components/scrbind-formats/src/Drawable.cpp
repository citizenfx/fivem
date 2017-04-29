/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "scrBind.h"
#include <memory>

// setup gtaDrawable format
#if defined(GTA_NY)
#define RAGE_FORMATS_GAME ny
#endif

#include <gtaDrawable.h>

using namespace rage::RAGE_FORMATS_GAME;

class ScriptShader : public fwRefCountable
{

};

class ScriptGeometry : public fwRefCountable
{
private:
	fwRefContainer<ScriptShader> m_shader;

	uint16_t* m_indices;

	uint32_t m_numIndices;

	void* m_vertices;

	uint32_t m_vertexStride;

	uint32_t m_vertexCount;

	uint32_t m_vertexUsageMask;

	uint64_t m_vertexTypeMask;

public:
	ScriptGeometry();

	~ScriptGeometry();

	inline void SetShader(ScriptShader* shader)
	{
		if (!scrBindIsSafePointer(shader))
		{
			return;
		}

		m_shader = shader;
	}

	inline void* AllocateIndices(uint32_t size)
	{
		if (m_indices)
		{
			delete[] m_indices;
		}

		m_indices = new uint16_t[size];

		scrBindAddSafePointer(m_indices);

		return m_indices;
	}

	inline void* AllocateVertices(uint32_t count, uint32_t stride)
	{
		if (m_vertices)
		{
			delete[] m_vertices;
		}

		m_vertices = new char[count * stride];

		scrBindAddSafePointer(m_vertices);

		return m_vertices;
	}

	inline void SetVertexFormat(uint32_t usageMask, uint32_t typeMaskLow, uint32_t typeMaskHigh)
	{
		m_vertexTypeMask = ((uint64_t)typeMaskHigh << 32) | typeMaskLow;
		m_vertexUsageMask = usageMask;
	}

	inline bool IsValid()
	{
		if (!m_shader.GetRef())
		{
			return false;
		}

		if (!m_indices)
		{
			return false;
		}

		if (!m_vertices)
		{
			return false;
		}

		return true;
	}
};

class ScriptModel : public fwRefCountable
{
private:
	Vector4 m_geometryBounds;

	std::vector<fwRefContainer<ScriptGeometry>> m_geometries;

	uint32_t m_setFlags;

#if 0
	Vector4 m_aabbMin;
	Vector4 m_aabbMax;
#endif

public:
	ScriptModel();

	inline void SetBounds(float x, float y, float z, float radius)
	{
		m_geometryBounds = Vector4(x, y, z, radius);
		m_setFlags |= 1;
	}

	inline ScriptGeometry* AddGeometry()
	{
		fwRefContainer<ScriptGeometry> geometry = new ScriptGeometry();
		m_geometries.push_back(geometry);

		// return pointer
		scrBindAddSafePointer(geometry.GetRef());

		return geometry.GetRef();
	}

	inline bool IsValid()
	{
		if ((m_setFlags & 1) != 1)
		{
			return false;
		}

		if (m_geometries.size() == 0)
		{
			return false;
		}

		return true;
	}
};

ScriptModel::ScriptModel()
	: m_setFlags(0)
{

}

class ScriptLodGroup
{
private:
	Vector3 m_minBound;
	Vector3 m_maxBound;
	Vector3 m_center;
	float m_radius;

	fwRefContainer<ScriptModel> m_models[4];

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

	inline ScriptModel* CreateModel(int lod)
	{
		if (lod < 0 || lod >= _countof(m_models))
		{
			return nullptr;
		}

		m_models[lod] = new ScriptModel();

		// return the pointer
		ScriptModel* model = m_models[lod].GetRef();

		scrBindAddSafePointer(model);

		return model;
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
		.AddMethod("SET_LODGROUP_RADIUS", &ScriptLodGroup::SetRadius)
		.AddMethod("CREATE_LODGROUP_MODEL", &ScriptLodGroup::CreateModel);

	/*scrEngine::OnScriptInit.Connect([] ()
	{
		void* drawable = NativeInvoke::Invoke<0x33486661, void*>(1, 2, "lovely");

		NativeInvoke::Invoke<0x7DFC57A4, int>(drawable, 1, 2, 3, 4, 5, 6);

		__debugbreak();
	}, 100);*/
});