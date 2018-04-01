/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <string>

#include <Error.h>

#include <ShaderInfo.h>

#define RAGE_FORMATS_GAME ny
#define RAGE_FORMATS_GAME_NY
#include <gtaDrawable.h>

#undef RAGE_FORMATS_GAME_NY
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <gtaDrawable.h>

#include <convert/base.h>

namespace rage
{
inline std::string ConvertSpsName_NY_Five(const char* oldSps)
{
	if (!oldSps)
	{
		return "default.sps";
	}

	if (strstr(oldSps, "3lyr") || strstr(oldSps, "2lyr"))
	{
		return "terrain_cb_4lyr_lod.sps"; // regular 4lyr has bump mapping/normals
	}

	if (strstr(oldSps, "va_4lyr"))
	{
		return "terrain_cb_4lyr_lod.sps"; // regular 4lyr has bump mapping/normals
	}

	if (strstr(oldSps, "gta_wire"))
	{
		return "cable.sps";
	}

	if (strstr(oldSps, "gta_"))
	{
		std::string name = &oldSps[4];

		return name.substr(0, name.find_last_of('.')) + ".sps";
	}

	return "default.sps";
}

inline std::string ConvertShaderName_NY_Five(const char* oldSps)
{
	if (strstr(oldSps, "3lyr") || strstr(oldSps, "2lyr"))
	{
		return "terrain_cb_4lyr_lod";
	}

	if (strstr(oldSps, "gta_wire"))
	{
		return "cable";
	}

	if (strstr(oldSps, "gta_"))
	{
		return &oldSps[4];
	}

	return "default";
}

extern FORMATS_EXPORT int g_curGeom;
extern FORMATS_EXPORT std::map<int, void*> g_vbMapping;
extern FORMATS_EXPORT std::map<int, void*> g_ibMapping;

template<>
five::pgDictionary<five::grcTexturePC>* convert(ny::pgDictionary<ny::grcTexturePC>* txd)
{
	five::pgDictionary<five::grcTexturePC>* out = new(false) five::pgDictionary<five::grcTexturePC>();
	out->SetBlockMap();

	five::pgDictionary<five::grcTexturePC> newTextures;

	if (txd->GetCount()) // amazingly there's 0-sized TXDs?
	{
		for (auto& texture : *txd)
		{
			ny::grcTexturePC* nyTexture = texture.second;
			five::grcTexturePC* fiveTexture = new(false) five::grcTexturePC(
				nyTexture->GetWidth(),
				nyTexture->GetHeight(),
				nyTexture->GetPixelFormat(),
				nyTexture->GetStride(),
				nyTexture->GetLevels(),
				nyTexture->GetPixelData()
				);

			fiveTexture->SetName(nyTexture->GetName());

			trace("%s: %p\n", nyTexture->GetName(), fiveTexture->GetPixelData());

			newTextures.Add(texture.first, fiveTexture);
		}
	}

	out->SetFrom(&newTextures);

	return out;
}

template<>
five::grmShaderGroup* convert(ny::grmShaderGroup* shaderGroup)
{
	auto out = new(false) five::grmShaderGroup;

	auto texDict = shaderGroup->GetTextures();

	if (texDict)
	{
		out->SetTextures(convert<five::pgDictionary<five::grcTexturePC>*>(texDict));
	}

	five::pgPtr<five::grmShaderFx> newShaders[512];
	
	for (int i = 0; i < shaderGroup->GetNumShaders(); i++)
	{
		auto oldShader = shaderGroup->GetShader(i);
		auto oldSpsName = oldShader->GetSpsName();
		auto oldShaderName = oldShader->GetShaderName();
		auto newSpsName = ConvertSpsName_NY_Five(oldSpsName);
		auto newShaderName = ConvertShaderName_NY_Five(oldShaderName);

		auto spsFile = fxc::SpsFile::Load(MakeRelativeCitPath(fmt::sprintf(L"citizen\\shaders\\db\\%s", ToWide(newSpsName))));

		if (spsFile)
		{
			newShaderName = spsFile->GetShader();
		}

		auto newShader = new(false) five::grmShaderFx();
		newShader->DoPreset(newShaderName.c_str(), newSpsName.c_str());

		if (spsFile)
		{
			auto db = spsFile->GetParameter("__rage_drawbucket");

			if (db)
			{
				newShader->SetDrawBucket(db->GetInt());
			}
		}
		else
		{
			// no known sps drawbucket, set the original one
			newShader->SetDrawBucket(oldShader->GetDrawBucket());
		}

		auto& oldEffect = oldShader->GetEffect();

		int rescount = 0;

		auto shaderFile = fxc::ShaderFile::Load(MakeRelativeCitPath(va(L"citizen\\shaders\\win32_40_final\\%s.fxc", ToWide(newShaderName))));

		for (int j = 0; j < oldEffect.GetParameterCount(); j++)
		{
			auto hash = oldEffect.GetParameterNameHash(j);

			uint32_t newSamplerName = 0;
			uint32_t newValueName = 0;
			size_t newValueSize;

			if (hash == 0x2B5170FD) // TextureSampler
			{
				newSamplerName = HashString("DiffuseSampler");

				if (newSpsName.find("cable") != std::string::npos)
				{
					newSamplerName = HashString("TextureSamp");
				}
			}

			if (!newSamplerName)
			{
				auto newParam = newShader->GetParameter(hash);

				if (newParam)
				{
					if (newParam->IsSampler())
					{
						newSamplerName = hash;
					}
				}
			}

			if (!newSamplerName && !newValueName)
			{
				auto newParam = newShader->GetParameter(hash);

				if (newParam && !newParam->IsSampler())
				{
					for (auto& parameter : shaderFile->GetLocalParameters())
					{
						if (HashString(parameter.first.c_str()) == hash)
						{
							newValueName = hash;
							newValueSize = parameter.second->GetDefaultValue().size();

							break;
						}
					}
				}
			}

			if (newSamplerName)
			{
				ny::grcTexturePC* texture = reinterpret_cast<ny::grcTexturePC*>(oldEffect.GetParameterValue(j));

				const char* textureName = (texture) ? texture->GetName() : "none";

				// look up if we just created one of these as local texture
				bool found = false;

				if (out->GetTextures())
				{
					for (auto& outTexture : *out->GetTextures())
					{
						if (!_stricmp(outTexture.second->GetName(), textureName))
						{
							newShader->SetParameter(newSamplerName, outTexture.second);
							rescount++;
							found = true;

							break;
						}
					}
				}

				// ... it's an external reference
				if (!found)
				{
					newShader->SetParameter(newSamplerName, textureName);
					rescount++;
				}
			}
			else if (newValueName)
			{
				newShader->SetParameter(newValueName, oldEffect.GetParameterValue(j), newValueSize);
			}
			else if (hash == HashString("specularFactor"))
			{
				float multiplier[4];
				memcpy(multiplier, oldEffect.GetParameterValue(j), sizeof(multiplier));

				multiplier[0] /= 100.0f;

				if (newShader->GetParameter("specularIntensityMult"))
				{
					newShader->SetParameter("specularIntensityMult", &multiplier, sizeof(multiplier));
				}
			}
		}

		if (auto falloffMult = newShader->GetParameter("SpecularFalloffMult"); falloffMult != nullptr)
		{
			float falloffMultValue[] = { *(float*)falloffMult->GetValue() * 4.0f, 0.0f, 0.0f, 0.0f };

			if (newShaderName.find("decal") == 0)
			{
				falloffMultValue[0] = 100.f;
			}

			memcpy(falloffMult->GetValue(), falloffMultValue, sizeof(falloffMultValue));
		}

		if (auto emissiveMult = newShader->GetParameter("EmissiveMultiplier"); emissiveMult != nullptr)
		{
			float emissiveMultValue[] = { *(float*)emissiveMult->GetValue() / 4.0f, 0.0f, 0.0f, 0.0f };
			memcpy(emissiveMult->GetValue(), emissiveMultValue, sizeof(emissiveMultValue));
		}

		if (newShaderName == "trees")
		{
			float alphaValues[4] = { 0.5f, 0.0f, 0.0f, 0.0f };
			newShader->SetParameter("AlphaTest", alphaValues, sizeof(alphaValues));
		}

		if (newShader->GetResourceCount() != rescount)
		{
			// add missing layer2/3 maps
			if (newShaderName == "terrain_cb_4lyr_lod")
			{
				int missingEntries = newShader->GetResourceCount() - rescount;

				if (missingEntries >= 1)
				{
					newShader->SetParameter("TextureSampler_layer3", "none");
				}

				if (missingEntries >= 2)
				{
					newShader->SetParameter("TextureSampler_layer2", "none");
				}
			}
			else if (newShaderName == "trees")
			{
				newShader->SetParameter("SfxWindSampler3D", "none");
			}
			else
			{
				FatalError("Got %d samplers in the Five shader, but only %d matched from NY...", newShader->GetResourceCount(), rescount);
			}
		}

		// TODO: change other arguments?
		newShaders[i] = newShader;
	}

	out->SetShaders(shaderGroup->GetNumShaders(), newShaders);

	return out;
}

template<>
five::grcIndexBufferD3D* convert(ny::grcIndexBufferD3D* buffer)
{
	return new(false) five::grcIndexBufferD3D(buffer->GetIndexCount(), buffer->GetIndexData());
}

template<>
five::grcVertexFormat* convert(ny::grcVertexFormat* format)
{
	return new(false) five::grcVertexFormat(format->GetMask(), format->GetVertexSize(), format->GetFieldCount(), format->GetFVF());
}

enum class FVFType
{
	Nothing = 0,
	Float16_2,
	Float,
	Float16_4,
	Float_unk,
	Float2,
	Float3,
	Float4,
	UByte4,
	Color,
	Dec3N
};

template<>
five::grcVertexBufferD3D* convert(ny::grcVertexBufferD3D* buffer)
{
	// really hacky fix for multiple vertex buffers sharing the same array
	static std::map<void*, void*> g_vertexBufferMatches;

	void* oldBuffer = buffer->GetVertices();

	if (g_vertexBufferMatches.find(oldBuffer) != g_vertexBufferMatches.end())
	{
		oldBuffer = g_vertexBufferMatches[oldBuffer];
	}
	else
	{
		auto vertexFormat = buffer->GetVertexFormat();
		
		auto mask = vertexFormat->GetMask();

		for (int i = 0; i < buffer->GetCount(); i++)
		{
			char* thisBit = (char*)oldBuffer + (buffer->GetStride() * i);
			size_t off = 0;

			for (int j = 0; j < 32; j++)
			{
				if (vertexFormat->GetMask() & (1 << j))
				{
					uint64_t fvf = vertexFormat->GetFVF();
					auto type = (FVFType)((fvf >> (j * 4)) & 0xF);

					switch (type)
					{
					case FVFType::Float4:
						off += 4;
					case FVFType::Float3:
						off += 4;
					case FVFType::Float2:
						off += 4;
					case FVFType::Nothing:
					case FVFType::Float:
					case FVFType::Float_unk:
						off += 4;
						break;
					case FVFType::Dec3N:
						off += 4;
						break;
					case FVFType::Color:
					case FVFType::UByte4:
						if (j == 4)
						{
							uint8_t* rgba = (uint8_t*)(thisBit + off);

							rgba[1] = uint8_t(rgba[1] * 0.5f); // 50% of original green
						}

						off += 4;
						break;
					case FVFType::Float16_2:
						off += 4;
						break;
					default:
						trace("unknown vertex type?\n");
						break;
					}
				}
			}
		}
	}

	auto out = new(false) five::grcVertexBufferD3D;

	out->SetVertexFormat(convert<five::grcVertexFormat*>(buffer->GetVertexFormat()));
	out->SetVertices(buffer->GetCount(), buffer->GetStride(), oldBuffer);

	g_vertexBufferMatches[buffer->GetVertices()] = out->GetVertices();

	return out;
}

template<>
five::grmGeometryQB* convert(ny::grmGeometryQB* geometry)
{
	auto out = new(false) five::grmGeometryQB;

	out->SetIndexBuffer(convert<five::grcIndexBufferD3D*>(geometry->GetIndexBuffer(0)));
	out->SetVertexBuffer(convert<five::grcVertexBufferD3D*>(geometry->GetVertexBuffer(0)));

	return out;
}

template<>
five::grmModel* convert(ny::grmModel* model)
{
	auto out = new(false) five::grmModel;
	int ni = 0;

	{
		auto& oldGeometries = model->GetGeometries();
		five::grmGeometryQB* geometries[256];

		for (int i = 0; i < oldGeometries.GetCount(); i++)
		{
			g_curGeom = i;

			auto oldie = oldGeometries.Get(i);

			//if (oldie->GetVertexBuffer(0)->GetStride() == 0x24)
			{
				geometries[ni] = convert<five::grmGeometryQB*>(oldie);
				ni++;
			}
		}

		out->SetGeometries(ni, geometries);
	}

	out->SetShaderMappings(/*model->GetShaderMappingCount()*/ni, model->GetShaderMappings());

	return out;
}

template<>
five::gtaDrawable* convert(ny::gtaDrawable* drawable)
{
	auto out = new(false) five::gtaDrawable;

	auto& oldLodGroup = drawable->GetLodGroup();

	out->SetBlockMap();

	out->SetShaderGroup(convert<five::grmShaderGroup*>(drawable->GetShaderGroup()));

	auto& lodGroup = out->GetLodGroup();
	
	Vector3 minBounds = oldLodGroup.GetBoundsMin();
	minBounds = Vector3(minBounds.x - oldLodGroup.GetRadius(), minBounds.y - oldLodGroup.GetRadius(), minBounds.z - oldLodGroup.GetRadius());

	Vector3 maxBounds = oldLodGroup.GetBoundsMax();
	maxBounds = Vector3(maxBounds.x + oldLodGroup.GetRadius(), maxBounds.y + oldLodGroup.GetRadius(), maxBounds.z + oldLodGroup.GetRadius());

	lodGroup.SetBounds(
		minBounds,
		maxBounds,
		oldLodGroup.GetCenter(), oldLodGroup.GetRadius()
	);
	//lodGroup.SetBounds(Vector3(-66.6f, -66.6f, -10.0f), Vector3(66.6f, 66.6f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), 94.8f);

	for (int i = 0; i < 4; i++)
	{
		auto oldModel = oldLodGroup.GetModel(i);

		if (oldModel)
		{
			auto newModel = convert<five::grmModel*>(oldModel);

			lodGroup.SetModel(i, newModel);
			lodGroup.SetDrawBucketMask(i, newModel->CalcDrawBucketMask(out->GetShaderGroup())); // TODO: change this

			{
				Vector4* oldBounds = oldModel->GetGeometryBounds();

				if (oldBounds)
				{
					int extraSize = 0;

					if (newModel->GetGeometries().GetCount() > 1)
					{
						extraSize = 1;
					}

					std::vector<five::GeometryBound> geometryBounds(newModel->GetGeometries().GetCount() + extraSize);

					for (int i = 0; i < geometryBounds.size(); i++)
					{
						geometryBounds[i].aabbMin = Vector4(oldBounds[i].x - oldBounds[i].w, oldBounds[i].y - oldBounds[i].w, oldBounds[i].z - oldBounds[i].w, -oldBounds[i].w);
						geometryBounds[i].aabbMax = Vector4(oldBounds[i].x + oldBounds[i].w, oldBounds[i].y + oldBounds[i].w, oldBounds[i].z + oldBounds[i].w, oldBounds[i].w);
					}

					newModel->SetGeometryBounds(geometryBounds.size(), &geometryBounds[0]);
				}
			}
		}
	}

	out->SetPrimaryModel();
	out->SetName("lovely.#dr");

	if (drawable->GetNumLightAttrs())
	{
		std::vector<five::CLightAttr> lightAttrs(drawable->GetNumLightAttrs());

		for (int i = 0; i < lightAttrs.size(); i++)
		{
			ny::CLightAttr& inAttr = *drawable->GetLightAttr(i);
			five::CLightAttr& outAttr = lightAttrs[i];

			outAttr.position[0] = inAttr.position[0];
			outAttr.position[1] = inAttr.position[1];
			outAttr.position[2] = inAttr.position[2];
			outAttr.color[0] = inAttr.color[0];
			outAttr.color[1] = inAttr.color[1];
			outAttr.color[2] = inAttr.color[2];
			outAttr.flashiness = inAttr.flashiness;
			outAttr.intensity = inAttr.lightIntensity;
			outAttr.flags = inAttr.flags;
			outAttr.boneID = inAttr.boneID;
			outAttr.lightType = inAttr.lightType;
			outAttr.groupID = 0;
			outAttr.timeFlags = (inAttr.flags & 64) ? 0b111100'000000'000011'111111 : 0b111111'111111'111111'111111;
			outAttr.falloff = (inAttr.lightFalloff == 0.0f) ? 17.0f : inAttr.lightFalloff;
			outAttr.falloffExponent = 64.f;
			outAttr.cullingPlane = { 0.0f, 0.0f, 1.0f, 200.0f };
			outAttr.shadowBlur = 0;
			outAttr.unk1 = 0;
			outAttr.unk2 = 0;
			outAttr.unk3 = 0;
			outAttr.volumeIntensity = inAttr.volumeIntensity;
			outAttr.volumeSizeScale = inAttr.volumeSize;
			outAttr.volumeOuterColor[0] = inAttr.color[0];
			outAttr.volumeOuterColor[1] = inAttr.color[1];
			outAttr.volumeOuterColor[2] = inAttr.color[2];
			outAttr.lightHash = inAttr.lumHash & 0xFF; // ?
			outAttr.volumeOuterIntensity = inAttr.volumeIntensity;
			outAttr.coronaSize = inAttr.coronaSize;
			outAttr.volumeOuterExponent = 64.f;
			outAttr.lightFadeDistance = inAttr.lightFadeDistance;
			outAttr.shadowFadeDistance = inAttr.shadowFadeDistance;
			outAttr.specularFadeDistance = 0.0f;
			outAttr.volumetricFadeDistance = 0.0f;
			outAttr.shadowNearClip = 0.01f;
			outAttr.coronaIntensity = 0.2f;
			outAttr.coronaZBias = 0.1f;
			outAttr.direction[0] = inAttr.direction[0];
			outAttr.direction[1] = inAttr.direction[1];
			outAttr.direction[2] = inAttr.direction[2];
			outAttr.tangent[0] = inAttr.tangent[0];
			outAttr.tangent[1] = inAttr.tangent[1];
			outAttr.tangent[2] = inAttr.tangent[2];
			outAttr.coneInnerAngle = 5.f;
			outAttr.coneOuterAngle = 60.f;
			outAttr.extents[0] = 1.f;
			outAttr.extents[1] = 1.f;
			outAttr.extents[2] = 1.f;
			outAttr.projectedTextureHash = 0;
			outAttr.unk4 = 0;
		}

		out->SetLightAttrs(&lightAttrs[0], lightAttrs.size());
	}

	return out;
}

template<>
five::pgDictionary<five::gtaDrawable>* convert(ny::pgDictionary<ny::gtaDrawable>* dwd)
{
	five::pgDictionary<five::gtaDrawable>* out = new(false) five::pgDictionary<five::gtaDrawable>();
	out->SetBlockMap();

	five::pgDictionary<five::gtaDrawable> newDrawables;

	if (dwd->GetCount()) // amazingly there's 0-sized TXDs?
	{
		for (auto& drawable : *dwd)
		{
			newDrawables.Add(drawable.first, convert<five::gtaDrawable*>(drawable.second));
		}
	}

	out->SetFrom(&newDrawables);

	return out;
}

template<>
five::fragType* convert(ny::fragType* frag)
{
	__debugbreak();

	return nullptr;
}
}
