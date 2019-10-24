/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <stdint.h>

#include <rmcDrawable.h>
#include <phBound.h>
#include <pgContainers.h>
//#include <grcTexture.h>

#define RAGE_FORMATS_FILE gtaDrawable
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK
#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_gtaDrawable 1
#elif defined(RAGE_FORMATS_GAME_PAYNE)
#define RAGE_FORMATS_payne_gtaDrawable 1
#elif defined(RAGE_FORMATS_GAME_RDR3)
#define RAGE_FORMATS_rdr3_gtaDrawable 1
#endif

#if defined(RAGE_FORMATS_GAME_FIVE)
#define RAGE_FORMATS_five_gtaDrawable 1
#endif

class CLightAttr : public datBase
{
public:
#ifdef RAGE_FORMATS_GAME_NY
	float position[3];
	float direction[3];
	float tangent[3];
	uint8_t color[4];
	float lodDistance;
	float volumeIntensity;
	float volumeSize;
	float lightAttenuationEnd;
	float lightIntensity;
	float coronaSize;
	float lightHotspot;
	float lightFalloff;
	uint32_t flags;
	uint32_t coronaHash;
	uint32_t lumHash;
	uint8_t flashiness;
	uint8_t pad[2];
	uint8_t lightType;
	float coronaHDRMultiplier;
	float lightFadeDistance;
	float shadowFadeDistance;
	uint16_t boneID;
	uint16_t pad2;
#elif defined(RAGE_FORMATS_GAME_FIVE)
	float position[3];
	uint32_t pad;
	uint8_t color[3];
	uint8_t flashiness;
	float intensity;
	uint32_t flags;
	uint16_t boneID;
	uint8_t lightType;
	uint8_t groupID;
	uint32_t timeFlags;
	float falloff;
	float falloffExponent;
	Vector4 cullingPlane; // normal xyz, offset
	uint8_t shadowBlur;
	uint8_t unk1;
	uint16_t unk2;
	uint32_t unk3;
	float volumeIntensity;
	float volumeSizeScale;
	uint8_t volumeOuterColor[3];
	uint8_t lightHash;
	float volumeOuterIntensity;
	float coronaSize;
	float volumeOuterExponent;
	uint8_t lightFadeDistance;
	uint8_t shadowFadeDistance;
	uint8_t specularFadeDistance;
	uint8_t volumetricFadeDistance;
	float shadowNearClip;
	float coronaIntensity;
	float coronaZBias;
	float direction[3];
	float tangent[3];
	float coneInnerAngle;
	float coneOuterAngle;
	float extents[3];
	uint32_t projectedTextureHash;
	uint32_t unk4;
#endif

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{

	}
};

class gtaDrawable : public rmcDrawable
{
private:
#if defined(RAGE_FORMATS_GAME_RDR3)
	uint8_t padGtaDrawable1[24];
#endif

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	pgPtr<const char> m_name;
#endif

	pgArray<CLightAttr> m_lightAttrs;

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	pgPtr<void> m_unk1;
	pgPtr<phBound> m_bound;
#endif

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		rmcDrawable::Resolve(blockMap);

		m_lightAttrs.Resolve(blockMap);

#ifdef RAGE_FORMATS_GAME_FIVE
		m_name.Resolve(blockMap);

		m_unk1.Resolve(blockMap);
		m_bound.Resolve(blockMap);

		if (!m_bound.IsNull())
		{
			m_bound->Resolve(blockMap);
		}
#endif
	}

#if defined(RAGE_FORMATS_GAME_FIVE) || defined(RAGE_FORMATS_GAME_RDR3)
	inline const char* GetName()
	{
		return *m_name;
	}
#endif

#ifdef RAGE_FORMATS_GAME_FIVE
	inline void SetName(const char* name)
	{
		m_name = pgStreamManager::StringDup(name);
	}

	inline void SetBound(phBound* bound)
	{
		m_bound = bound;
	}
#endif

	inline uint16_t GetNumLightAttrs()
	{
		return m_lightAttrs.GetCount();
	}

	inline CLightAttr* GetLightAttr(uint16_t index)
	{
		return &m_lightAttrs.Get(index);
	}

	inline void SetLightAttrs(CLightAttr* attrs, uint32_t count)
	{
		m_lightAttrs.SetFrom(attrs, count);
	}
};

#endif

#include <formats-footer.h>
