/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <stdint.h>

#include <numeric>

#include <pgBase.h>
#include <pgContainers.h>
//#include <grcTexture.h>

#define RAGE_FORMATS_FILE rmcDrawable
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK
#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_rmcDrawable 1
#endif

#if defined(RAGE_FORMATS_GAME_FIVE)
#define RAGE_FORMATS_five_rmcDrawable 1
#endif

template<typename T, size_t Size, size_t ActualSize = sizeof(T)>
inline void ValidateSize()
{
	static_assert(Size == ActualSize, "Invalid size.");
}

class grcTexture : public pgBase
{
protected:
#ifdef RAGE_FORMATS_GAME_NY
	uint8_t m_objectType;
	uint8_t m_depth;
	uint16_t m_usageCount;
	uint32_t m_pad;
	uint32_t m_pad2;
	pgPtr<char> m_name;
	uint32_t m_nativeHandle;
	uint16_t m_width;
	uint16_t m_height;
	uint32_t m_pixelFormat;
	uint16_t m_stride;
	uint8_t m_textureType;
	uint8_t m_levels;
#elif defined(RAGE_FORMATS_GAME_FIVE)
	uint8_t m_pad[24];
	pgPtr<char> m_name;
	uint8_t m_pad2[16];
#endif

public:
	inline grcTexture()
		: pgBase()
	{
#ifdef RAGE_FORMATS_GAME_NY
		m_objectType = 2;
		m_depth = 0;
		m_usageCount = 1;
		m_pad = 0;
		m_pad2 = 0;
		m_nativeHandle = 0;
		m_width = 0;
		m_height = 0;
		m_pixelFormat = 0;
		m_stride = 0;
		m_textureType = 0;
		m_levels = 0;
#elif defined(RAGE_FORMATS_GAME_FIVE)
		memset(m_pad, 0, sizeof(m_pad));
		memset(m_pad2, 0, sizeof(m_pad));

		m_pad2[0] = 1;
		m_pad2[2] = 0x80;
#endif
	}

	inline const char* GetName()
	{
		return *m_name;
	}

	inline void SetName(const char* name)
	{
		m_name = pgStreamManager::StringDup(name);
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_name.Resolve(blockMap);
	}
};

class grcTexturePC : public grcTexture
{
private:
#ifdef RAGE_FORMATS_GAME_NY
	float m_28[3];
	float m_34[3];
	uint32_t m_next;
	uint32_t m_prev;
	pgPtr<void> m_pixelData;
	uint8_t pad[4];
#elif defined(RAGE_FORMATS_GAME_FIVE)
	uint8_t m_pad3[16];
	uint16_t m_width;
	uint16_t m_height;
	uint16_t m_depth; // likely?
	uint16_t m_stride;

	uint32_t m_pixelFormat;

	uint8_t m_textureType;
	uint8_t m_levels;

	uint8_t m_pad4[18];

	pgPtr<void, true> m_pixelData;

	uint8_t m_pad5[24];
#endif

public:
	inline grcTexturePC()
		: grcTexture()
	{
#ifdef RAGE_FORMATS_GAME_NY
		memset(m_28, 0, sizeof(m_28));
		memset(m_34, 0, sizeof(m_34));
		m_next = 0;
		m_prev = 0;
		memset(pad, 0, sizeof(pad));
#elif defined(RAGE_FORMATS_GAME_FIVE)
		memset(m_pad3, 0, sizeof(m_pad3));

		m_depth = 1;
		m_textureType = 0;
		m_levels = 0;

		memset(m_pad4, 0, sizeof(m_pad4));
		memset(m_pad5, 0, sizeof(m_pad5));
#endif
	}

	inline size_t GetDataSize()
	{
		size_t levelSize = m_stride * m_height;
		size_t size = 0;

		for (int i = 0; i < m_levels; i++)
		{
			size += levelSize;
			levelSize /= 4;
		}

		return size;
	}

	inline uint16_t GetWidth()
	{
		return m_width;
	}

	inline uint16_t GetHeight()
	{
		return m_height;
	}

	inline uint16_t GetStride()
	{
		return m_stride;
	}

	inline uint8_t GetLevels()
	{
		return m_levels;
	}

	inline uint32_t GetPixelFormat()
	{
		return m_pixelFormat;
	}

	inline const void* GetPixelData()
	{
		return *m_pixelData;
	}

	inline grcTexturePC(uint16_t width, uint16_t height, uint32_t pixelFormat, uint16_t stride, uint8_t levels, const void* data)
		: grcTexturePC()
	{
		m_width = width;
		m_height = height;
		m_stride = stride;

		m_pixelFormat = pixelFormat;

		m_levels = levels;

		size_t dataSize = GetDataSize();
		
		void* outData = pgStreamManager::Allocate(dataSize, true, nullptr);
		memcpy(outData, data, dataSize);

		m_pixelData = outData;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		grcTexture::Resolve(blockMap);
	}
};

class crSkeletonData
{
public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{

	}
};

enum class grmShaderEffectParamType : uint8_t
{
	Texture = 0,
	Vector4 = 1,
	Matrix = 4,
	Vector4x6 = 8,
	Vector4x14 = 14,
	Vector4x15 = 15,
	Vector4x16 = 16
};

class grmShaderEffect
{
private:
	pgPtr<pgPtr<void>> m_parameters;
	pgPtr<int> m_pNull;
	uint32_t m_parameterCount;
	uint32_t m_parameterDataSize;
	pgPtr<grmShaderEffectParamType> m_parameterTypes;
	uint32_t m_shaderHash;
	uint32_t _f2C;
	uint32_t _f30;
	pgPtr<uint32_t> m_parameterNameHashes;
	uint32_t _f38;
	uint32_t _f3C;
	uint32_t _f40;

public:
	inline grmShaderEffect()
	{
		m_parameterCount = 0;
		m_parameterDataSize = 0;
		m_shaderHash = 0;
		_f2C = 0;
		_f30 = 0;
		_f38 = 0;
		_f3C = 0;
		_f40 = 0;
	}

	inline uint32_t GetParameterDataSize()
	{
		return SwapLongRead(m_parameterDataSize);
	}

	inline void SetParameterDataSize(uint32_t dataSize)
	{
		m_parameterDataSize = SwapLongWrite(dataSize);
	}

	inline uint32_t GetShaderHash()
	{
		return SwapLongRead(m_shaderHash);
	}

	inline void SetShaderHash(uint32_t shaderHash)
	{
		m_shaderHash = SwapLongWrite(shaderHash);
	}

	inline uint32_t GetParameterCount()
	{
		return SwapLongRead(m_parameterCount);
	}

	inline uint32_t GetParameterNameHash(int idx)
	{
		return SwapLongRead((*m_parameterNameHashes)[idx]);
	}

	inline void* GetParameterValue(int idx)
	{
		return *((*m_parameters)[idx]);
	}

	inline void SetParameters(uint32_t count, uint32_t* names, void** values, grmShaderEffectParamType* types)
	{
		pgPtr<void>* valuesInt = new(false) pgPtr<void>[count];

		for (int i = 0; i < count; i++)
		{
			valuesInt[i] = values[i];
		}

		m_parameterNameHashes = (uint32_t*)pgStreamManager::Allocate(sizeof(uint32_t) * count, false, nullptr);

		for (int i = 0; i < count; i++)
		{
			(*m_parameterNameHashes)[i] = names[i];
		}

		m_parameterTypes = (grmShaderEffectParamType*)pgStreamManager::Allocate(sizeof(grmShaderEffectParamType) * count, false, nullptr);

		for (int i = 0; i < count; i++)
		{
			(*m_parameterTypes)[i] = types[i];
		}

		m_parameters = valuesInt;

		m_parameterCount = SwapLongWrite(count);
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_parameters.Resolve(blockMap);
		m_parameterNameHashes.Resolve(blockMap);
		m_parameterTypes.Resolve(blockMap);

		auto count = GetParameterCount();

		for (int i = 0; i < count; i++)
		{
			(*m_parameters)[i].Resolve(blockMap);
		}
	}
};

#ifdef RAGE_FORMATS_GAME_FIVE
class grmShaderParameter
{
private:
	uint8_t m_pad;
	uint8_t m_type;
	uint16_t m_pad2;
	uint32_t m_pad3;
	pgPtr<void> m_value;

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_value.Resolve(blockMap);
	}

	inline bool IsSampler()
	{
		return !m_pad;
	}

	inline void SetSampler(bool isSampler)
	{
		m_pad = !isSampler;
		m_pad2 = 0;
		m_pad3 = 0;
	}

	inline void SetRegister(uint8_t reg)
	{
		m_type = reg;
	}

	inline void* GetValue()
	{
		return *m_value;
	}

	inline void SetValue(void* value)
	{
		m_value = value;
	}
};

struct grmShaderParameterMeta
{
	bool isSampler;
	uint8_t registerIdx;
};

class grcTextureRef : public datBase
{
private:
	uint8_t m_pad[32];

	pgPtr<char> m_name;

	uint16_t m_const1;
	uint16_t m_const2;

	uint8_t m_pad2[28];

public:
	inline grcTextureRef()
	{
		memset(m_pad, 0, sizeof(m_pad));
		memset(m_pad2, 0, sizeof(m_pad2));

		m_const1 = 1;
		m_const2 = 2;
	}

	inline void SetName(const char* name)
	{
		m_name = pgStreamManager::StringDup(name);
	}
};
#endif

class grmShader
#ifndef RAGE_FORMATS_GAME_FIVE
	: public pgBase
#else
	: public pgStreamableBase
#endif
{
protected:
#ifdef RAGE_FORMATS_GAME_NY
	uint8_t m_version;
	uint8_t m_drawBucket;
	uint8_t m_usageCount;
	uint8_t m_pad;
	uint16_t _fC;
	uint16_t m_shaderIndex;
	uint32_t _f10;
	grmShaderEffect m_effect;
#endif

#ifdef RAGE_FORMATS_GAME_FIVE
	pgPtr<grmShaderParameter> m_parameters;
	uintptr_t m_shaderHash;
	uint8_t m_parameterCount;
	uint8_t m_drawBucket;
	uint8_t m_pad;
	uint8_t m_hasComment;
	uint16_t m_parameterSize;
	uint16_t m_parameterDataSize;
	uintptr_t m_spsHash; // replaced with sps at runtime?
	uint32_t m_drawBucketMask; // 1 << (bucket) | 0xFF00
	uint8_t m_instanced;
	uint8_t m_pad2[2];
	uint8_t m_resourceCount;
	uint32_t m_pad3;
#endif

public:
#ifdef RAGE_FORMATS_GAME_FIVE
	inline grmShader()
	{
		m_resourceCount = 0;
		m_instanced = 0;
		memset(m_pad2, 0, sizeof(m_pad2));
		m_pad3 = 0;
		m_hasComment = 0;
		m_pad = 0;
	}
#endif

#ifdef RAGE_FORMATS_GAME_NY
	inline grmShader()
	{
		m_version = 0;
		m_drawBucket = 0;
		m_usageCount = 1;
		_fC = 0;
		m_shaderIndex = 0;
		_f10 = 0;
	}

	inline uint8_t GetVersion() { return m_version; }
	inline uint16_t GetShaderIndex() { return SwapShortRead(m_shaderIndex); }
	inline uint8_t GetUsageCount() { return m_usageCount; }

	inline void SetIndex(uint16_t shaderIndex)
	{
		m_shaderIndex = shaderIndex;
	}

	inline void SetVersion(uint8_t version) { m_version = version; }
	inline void SetUsageCount(uint8_t usageCount) { m_usageCount = SwapShortWrite(usageCount); }

	inline grmShaderEffect& GetEffect() { return m_effect; }

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_effect.Resolve(blockMap);
	}
#endif

	inline uint8_t GetDrawBucket() { return m_drawBucket; }

	inline void SetDrawBucket(uint8_t drawBucket)
	{
		m_drawBucket = drawBucket;

#ifdef RAGE_FORMATS_GAME_FIVE
		m_drawBucketMask = (1 << drawBucket) | 0xFF00;
#endif
	}

#ifdef RAGE_FORMATS_GAME_FIVE
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_parameters.Resolve(blockMap);

		for (int i = 0; i < m_parameterCount; i++)
		{
			(*m_parameters)[i].Resolve(blockMap);
		}
	}

	inline uint8_t GetResourceCount()
	{
		return m_resourceCount;
	}

	inline void SetParameters(const std::vector<grmShaderParameterMeta>& parameters, const std::vector<uint32_t>& parameterNames, const std::vector<std::vector<uint8_t>>& parameterValues)
	{
		assert(parameters.size() == parameterNames.size());
		assert(parameters.size() == parameterValues.size());

		m_parameterCount = parameters.size();
		
		uint32_t parameterSize = std::accumulate(parameterValues.begin(), parameterValues.end(), 0, [] (int left, const std::vector<uint8_t>& right)
		{
			return left + right.size();
		});

		parameterSize += sizeof(grmShaderParameter) * m_parameterCount;

		uint32_t parameterDataSize = parameterSize + sizeof(uint32_t) * m_parameterCount;

		grmShaderParameter* parameterData = (grmShaderParameter*)pgStreamManager::Allocate(parameterDataSize, false, nullptr);
		
		for (int i = 0; i < m_parameterCount; i++)
		{
			parameterData[i].SetSampler(parameters[i].isSampler);
			parameterData[i].SetRegister(parameters[i].registerIdx);
		}

		char* parameterValueData = (char*)(parameterData + m_parameterCount);

		for (int i = 0; i < m_parameterCount; i++)
		{
			if (!parameters[i].isSampler)
			{
				memcpy(parameterValueData, &parameterValues[i][0], parameterValues[i].size());

				parameterData[i].SetValue(parameterValueData);
				parameterValueData += parameterValues[i].size();
			}
			else
			{
				m_resourceCount++;
			}
		}

		uint32_t* parameterNameData = (uint32_t*)parameterValueData;
		std::copy(parameterNames.begin(), parameterNames.end(), parameterNameData);

		m_parameters = parameterData;
		m_parameterSize = parameterSize;
		m_parameterDataSize = parameterDataSize;
	}

	inline grmShaderParameter* GetParameter(uint32_t paramHash)
	{
		// get the parameter name list and find the parameter
		const uint32_t* parameterNames = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(*m_parameters) + m_parameterSize);

		for (int i = 0; i < m_parameterCount; i++)
		{
			if (parameterNames[i] == paramHash)
			{
				return (*m_parameters) + i;
			}
		}

		return nullptr;
	}

	inline grmShaderParameter* GetParameter(const char* parameterName)
	{
		// get the parameter name list and find the parameter
		return GetParameter(HashString(parameterName));
	}

	// assumes parameters have been set from a preset already
	inline void SetParameter(uint32_t parameterName, grcTexturePC* texture)
	{
		// get the parameter index
		grmShaderParameter* parameter = GetParameter(parameterName);

		// set the value
		parameter->SetValue(texture);
	}

	inline void SetParameter(uint32_t parameterName, const char* extTextureName)
	{
		// get the parameter index
		grmShaderParameter* parameter = GetParameter(parameterName);

		// set the value
		grcTextureRef* textureRef = new(false) grcTextureRef();
		textureRef->SetName(extTextureName);

		parameter->SetValue(textureRef);
	}

	inline void SetParameter(uint32_t parameterName, const void* data, size_t dataSize)
	{
		// get the parameter index
		grmShaderParameter* parameter = GetParameter(parameterName);

		// set the value (note: unsafe)
		memcpy(parameter->GetValue(), data, dataSize);
	}

	inline void SetParameter(const char* parameterName, grcTexturePC* texture)
	{
		// get the parameter index
		grmShaderParameter* parameter = GetParameter(parameterName);

		// set the value
		parameter->SetValue(texture);
	}

	inline void SetParameter(const char* parameterName, const char* extTextureName)
	{
		// get the parameter index
		grmShaderParameter* parameter = GetParameter(parameterName);

		// set the value
		grcTextureRef* textureRef = new(false) grcTextureRef();
		textureRef->SetName(extTextureName);

		parameter->SetValue(textureRef);
	}

	inline void SetParameter(const char* parameterName, const void* data, size_t dataSize)
	{
		// get the parameter index
		grmShaderParameter* parameter = GetParameter(parameterName);

		// set the value (note: unsafe)
		memcpy(parameter->GetValue(), data, dataSize);
	}
#endif
};

#if defined(RAGE_FORMATS_GAME_FIVE) && defined(_M_AMD64)
static_assert(sizeof(grmShader) == 48, "grmShader size is incorrect");
#endif

class grmShaderFx : public grmShader
{
private:
#ifndef RAGE_FORMATS_GAME_FIVE
	pgPtr<char> m_shaderName;
	pgPtr<char> m_spsName;
	uint32_t _f4C;
	uint32_t _f50;
	uint32_t m_preset;
	uint32_t _f58;
#endif

public:
#ifndef RAGE_FORMATS_GAME_FIVE
	inline grmShaderFx()
		: grmShader()
	{
		_f4C = 0;
		_f50 = 0;
		m_preset = 0;
		_f58 = 0;
	}

	inline const char* GetShaderName() { return *(m_shaderName); }

	inline const char* GetSpsName() { return *(m_shaderName); }

	inline void SetShaderName(char* value) { m_shaderName = pgStreamManager::StringDup(value); }

	inline void SetSpsName(char* value) { m_spsName = pgStreamManager::StringDup(value); }

	inline uint32_t GetPreset() { return SwapLongRead(m_preset); }

	inline void SetPreset(uint32_t value) { m_preset = SwapLongWrite(value); }
#else
	void FORMATS_EXPORT DoPreset(const char* shader, const char* sps);

	inline void SetShaderName(const char* value)
	{
		m_shaderHash = HashString(value);
	}

	inline void SetSpsName(const char* value)
	{
		m_spsHash = HashString(value);
	}
#endif

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		grmShader::Resolve(blockMap);

#ifndef RAGE_FORMATS_GAME_FIVE
		m_shaderName.Resolve(blockMap);
		m_spsName.Resolve(blockMap);
#endif
	}
};

class grmShaderGroup : public datBase
{
private:
	pgPtr<pgDictionary<grcTexturePC>> m_textures;
	pgObjectArray<grmShaderFx> m_shaders;
#ifdef RAGE_FORMATS_GAME_NY
	pgObjectArray<int> _f10;
	pgObjectArray<int> _f18;
	pgObjectArray<int> _f20;
	pgArray<int> _f28;
	pgArray<int> _f30;
	pgObjectArray<int> _f38;
	pgArray<uint32_t> m_vertexFormats;
	pgArray<uint32_t> m_shaderIndices;
#elif defined(RAGE_FORMATS_GAME_FIVE)
	pgObjectArray<int> _f20;
	uint32_t _f30;
	uintptr_t _f38;
#endif

public:
	inline pgDictionary<grcTexturePC>* GetTextures() { return *m_textures; }

	inline void SetTextures(pgDictionary<grcTexturePC>* textureDict)
	{
		m_textures = textureDict;
	}

	inline uint16_t GetNumShaders()
	{
		return m_shaders.GetCount();
	}

	inline grmShaderFx* GetShader(uint16_t offset) { return m_shaders.Get(offset); }

	inline void SetShaders(uint16_t count, pgPtr<grmShaderFx>* shaders)
	{
		m_shaders.SetFrom(shaders, count);

#if defined(RAGE_FORMATS_GAME_FIVE)
		_f30 = 0x1D;
#endif
	}

#if defined(RAGE_FORMATS_GAME_NY)
	inline uint32_t GetVertexFormat(uint16_t offset) { return SwapLongRead(m_vertexFormats.Get(offset)); }

	inline void SetVertexFormats(uint16_t count, uint32_t* formats)
	{
		for (int i = 0; i < count; i++)
		{
			formats[i] = SwapLongWrite(formats[i]);
		}

		m_vertexFormats.SetFrom(formats, count);
	}

	inline uint32_t GetShaderIndex(uint16_t offset)
	{
		return SwapLongRead(m_shaderIndices.Get(offset));
	}

	inline void SetShaderIndices(uint16_t count, uint32_t* formats)
	{
		for (int i = 0; i < count; i++)
		{
			formats[i] = SwapLongWrite(formats[i]);
		}

		m_shaderIndices.SetFrom(formats, count);
	}
#endif

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_textures.Resolve(blockMap);

		if (!m_textures.IsNull())
		{
			m_textures->Resolve(blockMap);
		}

		m_shaders.Resolve(blockMap);

#ifdef RAGE_FORMATS_GAME_NY
		m_vertexFormats.Resolve(blockMap);
		m_shaderIndices.Resolve(blockMap);
#endif
	}
};

class rmcDrawableBase : public pgBase
{
private:
	pgPtr<grmShaderGroup> m_shaderGroup;

public:
	inline grmShaderGroup* GetShaderGroup() { return *m_shaderGroup; }

	inline void SetShaderGroup(grmShaderGroup* shaderGroup) { m_shaderGroup = shaderGroup; }

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_shaderGroup.Resolve(blockMap);

		if (!m_shaderGroup.IsNull())
		{
			m_shaderGroup->Resolve(blockMap);
		}
	}
};

#ifdef RAGE_FORMATS_GAME_FIVE
#define PHYSICAL_VERTICES 0
#else
#define PHYSICAL_VERTICES 1
#endif

class grcIndexBuffer : public datBase
{
private:
	uint32_t m_indexCount;
	pgPtr<uint16_t, PHYSICAL_VERTICES> m_indexData;

public:
	grcIndexBuffer(uint32_t indexCount, uint16_t* indexData)
	{
		m_indexCount = indexCount;

		if (pgStreamManager::IsInBlockMap(indexData, nullptr, PHYSICAL_VERTICES))
		{
			m_indexData = indexData;
		}
		else
		{
			m_indexData = (uint16_t*)pgStreamManager::Allocate(indexCount * sizeof(uint16_t), PHYSICAL_VERTICES, nullptr);
			memcpy(*m_indexData, indexData, indexCount * sizeof(uint16_t));
		}
	}

	inline uint16_t* GetIndexData()
	{
		return *m_indexData;
	}

	inline uint32_t GetIndexCount()
	{
		return m_indexCount;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_indexData.Resolve(blockMap);
	}
};

class grcIndexBufferD3D : public grcIndexBuffer
{
private:
	void* m_pIIndexBuffer;
	uintptr_t m_unk[8];

public:
	grcIndexBufferD3D(uint32_t indexCount, uint16_t* indexData)
		: grcIndexBuffer(indexCount, indexData)
	{
		m_pIIndexBuffer = nullptr;
		memset(m_unk, 0, sizeof(m_unk));
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		grcIndexBuffer::Resolve(blockMap);
	}
};

#if defined(RAGE_FORMATS_GAME_FIVE) && defined(_M_AMD64)
static_assert(sizeof(grcIndexBufferD3D) >= 96, "grcIndexBufferD3D is too small!");
#endif

class grcVertexFormat : public pgStreamableBase
{
private:
#ifdef RAGE_FORMATS_GAME_NY
	uint32_t m_mask;
	uint8_t m_vertexSize;
	uint8_t _f5;
	uint8_t _f6;
	uint8_t m_vertexFieldCount;
	uint64_t m_vertexFields;
#endif

#ifdef RAGE_FORMATS_GAME_FIVE
	uint16_t m_mask;
	uint16_t _pad;
	uint8_t m_vertexSize;
	uint8_t _f5;
	uint8_t _f6;
	uint8_t m_vertexFieldCount; // maybe still 2 uint8s?
	uint64_t m_vertexFields;
#endif

public:
	grcVertexFormat(uint16_t mask, uint16_t vertexSize, uint8_t fieldCount, uint64_t fvf)
	{
#ifndef RAGE_FORMATS_GAME_NY
		_pad = 0;
#endif
		_f5 = 0;
		_f6 = 0;

		m_mask = mask;
		m_vertexSize = vertexSize;
		m_vertexFieldCount = fieldCount;
		m_vertexFields = fvf;
	}

	inline decltype(m_mask) GetMask()
	{
		return m_mask;
	}

	inline uint16_t GetVertexSize()
	{
		return m_vertexSize;
	}

	inline uint8_t GetFieldCount()
	{
		return m_vertexFieldCount;
	}

	inline uint64_t GetFVF()
	{
		return m_vertexFields;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{

	}
};

class grcVertexBuffer : public datBase
{
private:
#ifdef RAGE_FORMATS_GAME_NY
	uint16_t m_vertexCount;
	uint8_t m_locked;
	uint8_t m_pad;
	pgPtr<void> m_lockedData;
	uint32_t m_vertexSize;
	pgPtr<grcVertexFormat> m_vertexFormat;
	uint32_t m_lockThreadId;
	pgPtr<void, true> m_vertexData;
#endif

#ifdef RAGE_FORMATS_GAME_FIVE
	uint16_t m_vertexSize;
	uint8_t m_locked;
	uint8_t m_flags;
	pgPtr<void> m_lockedData;
	uint32_t m_vertexCount;
	pgPtr<void> m_vertexData;
	uintptr_t m_pad;
	pgPtr<grcVertexFormat> m_vertexFormat;
	pgPtr<void> m_unkData;
#endif

public:
	grcVertexBuffer()
	{
		m_locked = 0;

#ifdef RAGE_FORMATS_GAME_FIVE
		m_pad = 0;
		m_flags = 0; // sets movement flags for 'data'
#endif

#ifdef RAGE_FORMATS_GAME_NY
		m_pad = 0;
		m_lockThreadId = 0;
#endif
	}

	inline void SetVertexFormat(grcVertexFormat* vertexFormat)
	{
		m_vertexFormat = vertexFormat;
	}

	inline grcVertexFormat* GetVertexFormat()
	{
		return *m_vertexFormat;
	}

	inline void* GetVertices()
	{
		return *m_vertexData;
	}

	inline void SetVertices(uint32_t vertexCount, uint32_t vertexStride, void* vertexData)
	{
		m_vertexCount = vertexCount;
		m_vertexSize = vertexStride;

		if (pgStreamManager::IsInBlockMap(vertexData, nullptr, PHYSICAL_VERTICES))
		{
			m_vertexData = vertexData;

#ifdef RAGE_FORMATS_GAME_FIVE
			// Five resources have this set, although resource loading appears to unset this anyway.
			m_lockedData = vertexData;
#endif
		}
		else
		{
			void* vertexDataBit = pgStreamManager::Allocate(vertexCount * vertexStride, PHYSICAL_VERTICES, nullptr);

			m_vertexData = vertexDataBit;

#ifdef RAGE_FORMATS_GAME_FIVE
			m_lockedData = vertexDataBit;
#endif
			memcpy(*m_vertexData, vertexData, vertexCount * vertexStride);
		}
	}

	inline uint32_t GetStride()
	{
		return m_vertexSize;
	}

	inline uint32_t GetCount()
	{
		return m_vertexCount;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_lockedData.Resolve(blockMap);
		m_vertexData.Resolve(blockMap);

		m_vertexFormat.Resolve(blockMap);

		if (!m_vertexFormat.IsNull())
		{
			m_vertexFormat->Resolve(blockMap);
		}
	}
};

class grcVertexBufferD3D : public grcVertexBuffer
{
private:
	void* m_pIVertexBuffer;
	uintptr_t m_unk[8];

public:
	inline grcVertexBufferD3D()
	{
		memset(m_unk, 0, sizeof(m_unk));

		m_pIVertexBuffer = nullptr;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		grcVertexBuffer::Resolve(blockMap);
	}
};

#if defined(RAGE_FORMATS_GAME_FIVE) && defined(_M_AMD64)
static_assert(sizeof(grcVertexBufferD3D) >= 128, "grcIndexBufferD3D is too small!");
#endif

class grmGeometryQB : public datBase
{
private:
	pgPtr<void> m_vertexDeclaration; // actually IDirect3DVertexDeclaration9* at runtime
	uint32_t m_objectType;
	pgPtr<grcVertexBufferD3D> m_vertexBuffers[4];
	pgPtr<grcIndexBufferD3D> m_indexBuffers[4];
	uint32_t m_dwIndexCount;
	uint32_t m_dwFaceCount;
	uint16_t m_wVertexCount;
	uint16_t m_wIndicesPerFace;
	pgPtr<uint16_t> m_boneMapping;
	uint16_t m_vertexStride;
	uint16_t m_boneCount;

#ifdef RAGE_FORMATS_GAME_NY
	pgPtr<void> m_vertexDeclarationInstance;
	pgPtr<void> m_vertexBufferInstance;
	uint32_t m_useGlobalStreamIndex;
#elif defined(RAGE_FORMATS_GAME_FIVE)
	pgPtr<void> m_vertexData;
	uint8_t m_pad[32];
#endif

public:
	inline grmGeometryQB()
	{
		m_vertexDeclaration = nullptr;
		m_objectType = 0;
		m_dwIndexCount = 0;
		m_dwFaceCount = 0;
		m_wVertexCount = 0;
		m_wIndicesPerFace = 3;
		m_boneCount = 0;

#ifdef RAGE_FORMATS_GAME_NY
		m_useGlobalStreamIndex = 0;
#elif defined(RAGE_FORMATS_GAME_FIVE)
		memset(m_pad, 0, sizeof(m_pad));
#endif
	}

	inline grcVertexBufferD3D* GetVertexBuffer(int idx)
	{
		return *(m_vertexBuffers[idx]);
	}

	inline grcIndexBufferD3D* GetIndexBuffer(int idx)
	{
		return *(m_indexBuffers[idx]);
	}

	inline void FixUpBrokenVertexCounts()
	{
		m_vertexStride = m_vertexBuffers[0]->GetStride();
		m_wVertexCount = m_vertexBuffers[0]->GetCount();

		m_dwIndexCount = m_indexBuffers[0]->GetIndexCount();
		m_dwFaceCount = m_indexBuffers[0]->GetIndexCount() / m_wIndicesPerFace;
	}

	inline void SetVertexBuffer(grcVertexBufferD3D* vertexBuffer)
	{
		m_vertexBuffers[0] = vertexBuffer;
		m_vertexStride = vertexBuffer->GetStride();
		m_wVertexCount = vertexBuffer->GetCount();

#ifdef RAGE_FORMATS_GAME_FIVE
		m_vertexData = vertexBuffer->GetVertices();
#endif
	}

	inline void SetIndexBuffer(grcIndexBufferD3D* indexBuffer)
	{
		m_indexBuffers[0] = indexBuffer;
		m_dwIndexCount = indexBuffer->GetIndexCount();
		m_dwFaceCount = indexBuffer->GetIndexCount() / m_wIndicesPerFace;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		for (int i = 0; i < 4; i++)
		{
			m_vertexBuffers[i].Resolve(blockMap);

			if (!m_vertexBuffers[i].IsNull())
			{
				m_vertexBuffers[i]->Resolve(blockMap);
			}

			m_indexBuffers[i].Resolve(blockMap);

			if (!m_indexBuffers[i].IsNull())
			{
				m_indexBuffers[i]->Resolve(blockMap);
			}
		}

#ifdef RAGE_FORMATS_GAME_NY
		m_boneMapping.Resolve(blockMap);
#endif
	}
};

#if defined(RAGE_FORMATS_GAME_FIVE)
struct GeometryBound
{
	Vector4 aabbMin;
	Vector4 aabbMax;
};
#endif

class grmModel : public datBase
{
private:
	pgObjectArray<grmGeometryQB> m_geometries;

#if defined(RAGE_FORMATS_GAME_NY)
	pgPtr<Vector4> m_geometryBounds;
#elif defined(RAGE_FORMATS_GAME_FIVE)
	pgPtr<GeometryBound> m_geometryBounds;
#endif
	pgPtr<uint16_t> m_shaderMappings;

	uint8_t m_boneCount;
	uint8_t m_skinned;
	uint8_t m_pad;
	uint8_t m_zero;
	uint8_t m_zero2;
	uint8_t m_hasBoneMapping; // bitfield: bit 0 - has bone mapping, bit 1-7: number of geometries with bone mapping
	uint16_t m_shaderMappingCount; // may be geometry count by proxy?

public:
	inline grmModel()
	{
		m_boneCount = 0;
		m_skinned = 0;
		m_zero = 0;
#if defined(RAGE_FORMATS_GAME_NY)
		m_zero2 = 0;
#elif defined(RAGE_FORMATS_GAME_FIVE)
		m_zero2 = 0xFF; // gets tested against the upper byte of the draw bucket mask
#endif
		m_hasBoneMapping = 0;
		m_shaderMappingCount = 0;
	}

	inline pgObjectArray<grmGeometryQB>& GetGeometries()
	{
		return m_geometries;
	}

	inline void SetGeometries(int count, grmGeometryQB** geometries)
	{
		static pgPtr<grmGeometryQB> geometriesInd[64];

		for (int i = 0; i < count; i++)
		{
			geometriesInd[i] = geometries[i];
		}

		m_geometries.SetFrom(geometriesInd, count);
	}

#ifdef RAGE_FORMATS_GAME_NY
	inline Vector4* GetGeometryBounds()
	{
		return (*m_geometryBounds);
	}

	inline void SetGeometryBounds(int count, const Vector4* vectors)
	{
		m_geometryBounds = (Vector4*)pgStreamManager::Allocate(sizeof(Vector4) * count, false, nullptr);

		for (int i = 0; i < count; i++)
		{
			(*m_geometryBounds)[i] = vectors[i];
		}
	}
#else
	inline GeometryBound* GetGeometryBounds()
	{
		return (*m_geometryBounds);
	}

	inline void SetGeometryBounds(int count, const GeometryBound* vectors)
	{
		m_geometryBounds = (GeometryBound*)pgStreamManager::Allocate(sizeof(GeometryBound) * count, false, nullptr);

		for (int i = 0; i < count; i++)
		{
			(*m_geometryBounds)[i] = vectors[i];
		}
	}
#endif

	inline void SetBoneCount(uint8_t count)
	{
		m_boneCount = count;
	}

	inline uint16_t* GetShaderMappings()
	{
		return *m_shaderMappings;
	}

	inline uint16_t GetShaderMappingCount()
	{
		return m_shaderMappingCount;
	}

	inline void SetShaderMappings(uint16_t count, const uint16_t* shaderMappings)
	{
		m_shaderMappings = (uint16_t*)pgStreamManager::Allocate(sizeof(uint16_t) * count, false, nullptr);

		for (int i = 0; i < count; i++)
		{
			(*m_shaderMappings)[i] = shaderMappings[i];
		}

		m_shaderMappingCount = count;
	}

	inline uint16_t CalcDrawBucketMask(grmShaderGroup* shaderGroup)
	{
		uint16_t drawBucketMask = 0xFF00;

		for (int i = 0; i < m_shaderMappingCount; i++)
		{
			uint16_t shaderIdx = (*m_shaderMappings)[i];
			grmShaderFx* shader = shaderGroup->GetShader(shaderIdx);

			drawBucketMask |= (1 << shader->GetDrawBucket());
		}

		return drawBucketMask;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_geometries.Resolve(blockMap);
		m_geometryBounds.Resolve(blockMap);
		m_shaderMappings.Resolve(blockMap);
	}
};

class grmLodGroup
{
private:
	Vector4 m_center;
	Vector3 m_boundsMin;
	Vector3 m_boundsMax;
	pgPtr<pgObjectArray<grmModel>> m_models[4];

#ifdef RAGE_FORMATS_GAME_NY
	float m_9999[4];
#endif
#ifdef RAGE_FORMATS_GAME_FIVE
	Vector4 m_maxPoint;
#endif

	int m_drawBucketMask[4]; // does this apply to five?

#ifndef RAGE_FORMATS_GAME_FIVE
	float m_radius;
	float m_zeroes[3];
#endif

public:
	inline grmLodGroup()
	{
#ifdef RAGE_FORMATS_GAME_NY
		m_9999[0] = 9999.f;
		m_9999[1] = 9999.f;
		m_9999[2] = 9999.f;
		m_9999[3] = 9999.f;
#else
		m_maxPoint = Vector4(9998.0f, 9998.0f, 9998.0f, 9998.0f);

		m_drawBucketMask[0] = 0;
		m_drawBucketMask[1] = 0;
		m_drawBucketMask[2] = 0;
		m_drawBucketMask[3] = 0;
#endif

#ifndef RAGE_FORMATS_GAME_FIVE
		m_drawBucketMask[0] = -1;
		m_drawBucketMask[1] = -1;
		m_drawBucketMask[2] = -1;
		m_drawBucketMask[3] = -1;

		m_zeroes[0] = 0;
		m_zeroes[1] = 0;
		m_zeroes[2] = 0;

		m_radius = 0.0f;
#endif
	}

	inline Vector3 GetBoundsMin()
	{
		return m_boundsMin;
	}

	inline Vector3 GetBoundsMax()
	{
		return m_boundsMax;
	}

	inline Vector3 GetCenter()
	{
		return { m_center.x, m_center.y, m_center.z };
	}

#ifndef RAGE_FORMATS_GAME_FIVE
	inline float GetRadius()
	{
		return m_radius;
	}

	inline void SetBounds(const Vector3& min, const Vector3& max, const Vector3& center, float radius)
	{
		m_boundsMin = min;
		m_boundsMax = max;
		m_center = Vector4(center.x, center.y, center.z, radius);
		m_radius = radius;
	}
#else
	inline float GetRadius()
	{
		return m_center.w;
	}

	inline void SetBounds(const Vector3& min, const Vector3& max, const Vector3& center, float radius)
	{
		m_boundsMin = min;
		m_boundsMax = max;
		m_center = { center.x, center.y, center.z, radius };
	}
#endif

	inline grmModel* GetModel(int idx)
	{
		if (idx < 0 || idx >= _countof(m_models))
		{
			abort();
		}

		if (m_models[idx].IsNull())
		{
			return nullptr;
		}

		return m_models[idx]->Get(0);
	}

	inline pgObjectArray<grmModel>* GetPrimaryModel()
	{
		return *m_models[0];
	}

#ifdef RAGE_FORMATS_GAME_FIVE
	inline const Vector4& GetMaxPoint()
	{
		return m_maxPoint;
	}

	inline void SetMaxPoint(const Vector4& point)
	{
		m_maxPoint = point;
	}
#endif

	inline void SetModel(int idx, grmModel* model)
	{
		if (idx < 0 || idx >= _countof(m_models))
		{
			abort();
		}

		if (model == nullptr)
		{
			m_models[idx] = nullptr;
			return;
		}

		pgPtr<grmModel> models[1];
		models[0] = model;

		m_models[idx] = new(false) pgObjectArray<grmModel>(models, 1);
	}

	inline void SetDrawBucketMask(int idx, int mask)
	{
		if (idx < 0 || idx >= _countof(m_models))
		{
			abort();
		}

		m_drawBucketMask[idx] = mask;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		for (int i = 0; i < 4; i++)
		{
			m_models[i].Resolve(blockMap);

			if (!m_models[i].IsNull())
			{
				m_models[i]->Resolve(blockMap);
			}
		}
	}
};

class rmcDrawable : public rmcDrawableBase
{
private:
	pgPtr<crSkeletonData> m_skeletonData;

	grmLodGroup m_lodGroup;

#ifdef RAGE_FORMATS_GAME_FIVE
	uintptr_t m_unk1;
	uint16_t m_unk2;
	uint16_t m_unk3;
	uint32_t m_unk4;
	
	pgPtr<pgObjectArray<grmModel>> m_primaryModel;
#endif

public:
#ifdef RAGE_FORMATS_GAME_FIVE
	inline rmcDrawable()
	{
		m_unk1 = 0;
		m_unk2 = 0;
		m_unk3 = 0x60;
		m_unk4 = 0;
	}
#endif

	inline grmLodGroup& GetLodGroup()
	{
		return m_lodGroup;
	}

#ifdef RAGE_FORMATS_GAME_FIVE
	inline void SetPrimaryModel()
	{
		m_primaryModel = m_lodGroup.GetPrimaryModel();
	}
#endif

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		rmcDrawableBase::Resolve(blockMap);

		m_skeletonData.Resolve(blockMap);

		if (!m_skeletonData.IsNull())
		{
			m_skeletonData->Resolve(blockMap);
		}

		m_lodGroup.Resolve(blockMap);

#ifdef RAGE_FORMATS_GAME_FIVE
		m_primaryModel.Resolve(blockMap);
#endif
	}
};
#endif

#include <formats-footer.h>