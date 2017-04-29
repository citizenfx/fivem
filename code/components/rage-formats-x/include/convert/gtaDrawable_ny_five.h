/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <string>

#include <Error.h>

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
	if (strstr(oldSps, "3lyr") || strstr(oldSps, "2lyr"))
	{
		return "terrain_cb_4lyr_lod.sps"; // regular 4lyr has bump mapping/normals
	}

	if (strstr(oldSps, "gta_"))
	{
		return &oldSps[4] + std::string(".sps");
	}

	return "default.sps";
}

inline std::string ConvertShaderName_NY_Five(const char* oldSps)
{
	if (strstr(oldSps, "3lyr") || strstr(oldSps, "2lyr"))
	{
		return "terrain_cb_4lyr_lod";
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

	five::pgPtr<five::grmShaderFx> newShaders[128];
	
	for (int i = 0; i < shaderGroup->GetNumShaders(); i++)
	{
		auto oldShader = shaderGroup->GetShader(i);
		auto newSpsName = ConvertSpsName_NY_Five(oldShader->GetSpsName());
		auto newShaderName = ConvertShaderName_NY_Five(oldShader->GetShaderName());

		auto newShader = new(false) five::grmShaderFx();
		newShader->DoPreset(newShaderName.c_str(), newSpsName.c_str());
		newShader->SetDrawBucket(oldShader->GetDrawBucket()); // in case the lack of .sps reading doesn't override it, yet

		auto& oldEffect = oldShader->GetEffect();

		int rescount = 0;

		for (int j = 0; j < oldEffect.GetParameterCount(); j++)
		{
			auto hash = oldEffect.GetParameterNameHash(j);

			uint32_t newSamplerName = 0;
			uint32_t newValueName = 0;
			size_t newValueSize;

			if (hash == 0x2B5170FD) // TextureSampler
			{
				newSamplerName = HashString("DiffuseSampler");
			}
			else if (hash == 0xF6712B81)
			{
				newValueName = HashString("bumpiness"); // as, well, it's bumpiness?
				newValueSize = 4 * sizeof(float);
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
		oldLodGroup.GetCenter(), oldLodGroup.GetRadius() * 2
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
					std::vector<five::GeometryBound> geometryBounds(newModel->GetGeometries().GetCount() + 1);

					for (int i = 1; i < geometryBounds.size(); i++)
					{
						oldBounds[i - 1].w *= 2;

						geometryBounds[i].aabbMin = Vector4(oldBounds[i - 1].x - oldBounds[i - 1].w, oldBounds[i - 1].y - oldBounds[i - 1].w, oldBounds[i - 1].z - oldBounds[i - 1].w, -oldBounds[i - 1].w);
						geometryBounds[i].aabbMax = Vector4(oldBounds[i - 1].x + oldBounds[i - 1].w, oldBounds[i - 1].y + oldBounds[i - 1].w, oldBounds[i - 1].z + oldBounds[i - 1].w, oldBounds[i - 1].w);
					}

					geometryBounds[0].aabbMin = Vector4(minBounds.x, minBounds.y, minBounds.z, oldLodGroup.GetRadius() * -2.0f);
					geometryBounds[0].aabbMax = Vector4(maxBounds.x, maxBounds.y, maxBounds.z, oldLodGroup.GetRadius() * 2.0f);

					newModel->SetGeometryBounds(geometryBounds.size(), &geometryBounds[0]);
				}
			}
		}
	}

	out->SetPrimaryModel();
	out->SetName("lovely.#dr");

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
}