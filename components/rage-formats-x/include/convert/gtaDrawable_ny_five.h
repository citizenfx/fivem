/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <string>

#define RAGE_FORMATS_GAME ny
#define RAGE_FORMATS_GAME_NY
#include <gtaDrawable.h>

#undef RAGE_FORMATS_GAME_NY
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <gtaDrawable.h>

namespace rage
{
template<typename Target, typename Source>
Target convert(Source source)
{
	return (Target)0;
}
}

namespace rage
{
inline std::string ConvertSpsName_NY_Five(const char* oldSps)
{
	/*if (strstr(oldSps, "normal"))
	{
		return "normal.sps";
	}*/

	return "default.sps";
}

inline std::string ConvertShaderName_NY_Five(const char* oldSps)
{
	/*if (strstr(oldSps, "normal"))
	{
		return "normal";
	}*/

	return "default";
}

extern FORMATS_EXPORT int g_curGeom;
extern FORMATS_EXPORT std::map<int, void*> g_vbMapping;
extern FORMATS_EXPORT std::map<int, void*> g_ibMapping;

template<>
five::grmShaderGroup* convert(ny::grmShaderGroup* shaderGroup)
{
	auto out = new(false) five::grmShaderGroup;

	five::pgPtr<five::grmShaderFx> newShaders[32];
	
	for (int i = 0; i < shaderGroup->GetNumShaders(); i++)
	{
		auto oldShader = shaderGroup->GetShader(i);
		auto newSpsName = ConvertSpsName_NY_Five(oldShader->GetSpsName());
		auto newShaderName = ConvertShaderName_NY_Five(oldShader->GetShaderName());

		auto newShader = new(false) five::grmShaderFx();
		newShader->DoPreset(newShaderName.c_str(), newSpsName.c_str());

		// TODO: change other arguments?
		newShaders[i] = newShader;
	}

	out->SetShaders(shaderGroup->GetNumShaders(), newShaders);

	return out;
}

/*template<>
five::grcIndexBufferD3D* convert(ny::grcIndexBufferD3D* buffer)
{
	return new(false) five::grcIndexBufferD3D(buffer->GetIndexCount(), buffer->GetIndexData());
}*/

template<>
five::grcVertexFormat* convert(ny::grcVertexFormat* format)
{
	return new(false) five::grcVertexFormat(format->GetMask(), format->GetVertexSize(), format->GetFieldCount(), format->GetFVF());
}

template<>
five::grcVertexBufferD3D* convert(ny::grcVertexBufferD3D* buffer)
{
	auto out = new(false) five::grcVertexBufferD3D;

	out->SetVertexFormat(convert<five::grcVertexFormat*>(buffer->GetVertexFormat()));
	//out->SetVertices(buffer->GetCount(), buffer->GetStride(), buffer->GetVertices());

	return out;
}

template<>
five::grmGeometryQB* convert(ny::grmGeometryQB* geometry)
{
	auto out = new(false) five::grmGeometryQB;

	//out->SetIndexBuffer(convert<five::grcIndexBufferD3D*>(geometry->GetIndexBuffer(0)));
	out->SetIndexBuffer(new(false) five::grcIndexBufferD3D(geometry->GetIndexBuffer(0)->GetIndexCount(), (uint16_t*)g_ibMapping[g_curGeom]));

	auto inVB = geometry->GetVertexBuffer(0);
	auto vb = convert<five::grcVertexBufferD3D*>(inVB);
	vb->SetVertices(inVB->GetCount(), inVB->GetStride(), g_vbMapping[g_curGeom]);
	out->SetVertexBuffer(vb);



	return out;
}

template<>
five::grmModel* convert(ny::grmModel* model)
{
	auto out = new(false) five::grmModel;
	int ni = 0;

	{
		auto& oldGeometries = model->GetGeometries();
		five::grmGeometryQB* geometries[32];

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

	// prepare vertex/index data (Five wants it to be set *prior* to the rest?)
	auto& oldLodGroup = drawable->GetLodGroup();

	for (int i = 0; i < 4; i++)
	{
		auto oldModel = oldLodGroup.GetModel(i);

		if (oldModel)
		{
			auto& geometries = oldModel->GetGeometries();

			for (int j = 0; j < geometries.GetCount(); j++)
			{
				auto geom = geometries.Get(j);
				auto vb = geom->GetVertexBuffer(0);
				auto ib = geom->GetIndexBuffer(0);

				auto vSize = vb->GetCount() * vb->GetStride();
				auto iSize = ib->GetIndexCount() * 2;

				void* vertexDataBit = five::pgStreamManager::Allocate(vSize, false, (five::BlockMap*)1);
				memcpy(vertexDataBit, vb->GetVertices(), vSize);

				void* indexDataBit = five::pgStreamManager::Allocate(iSize, false, (five::BlockMap*)1);
				memcpy(indexDataBit, ib->GetIndexData(), ib->GetIndexCount() * 2);

				g_vbMapping[j] = vertexDataBit;
				g_ibMapping[j] = indexDataBit;
			}
		}
	}

	out->SetBlockMap();

	out->SetShaderGroup(convert<five::grmShaderGroup*>(drawable->GetShaderGroup()));

	auto& lodGroup = out->GetLodGroup();
	
	//lodGroup.SetBounds(oldLodGroup.GetBoundsMin(), oldLodGroup.GetBoundsMax(), oldLodGroup.GetCenter(), oldLodGroup.GetRadius());
	lodGroup.SetBounds(Vector3(-66.6f, -66.6f, -10.0f), Vector3(66.6f, 66.6f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), 94.8f);

	for (int i = 0; i < 4; i++)
	{
		auto oldModel = oldLodGroup.GetModel(i);

		if (oldModel)
		{
			auto newModel = convert<five::grmModel*>(oldModel);

			lodGroup.SetModel(i, newModel);
			lodGroup.SetDrawBucketMask(i, 0xFF01); // TODO: change this

			{
				Vector3 minBounds = oldLodGroup.GetBoundsMin();
				Vector3 maxBounds = oldLodGroup.GetBoundsMax();

				float radius = oldLodGroup.GetRadius();

				/*five::GeometryBound bound;
				bound.aabbMin = Vector4(minBounds.x, minBounds.y, minBounds.z, -radius);
				bound.aabbMax = Vector4(maxBounds.x, maxBounds.y, maxBounds.z, radius);

				newModel->SetGeometryBounds(bound);*/

				five::GeometryBound bounds[8];
				bounds[0].aabbMin = Vector4(-66.0f, -66.0f, -1.0f, -94.8f);
				bounds[0].aabbMax = Vector4(66.0f, 66.0f, 1.0f, 94.8f);

				bounds[1] = bounds[0];
				bounds[2] = bounds[0];
				bounds[3] = bounds[0];
				bounds[4] = bounds[0];
				bounds[5] = bounds[0];
				bounds[6] = bounds[0];
				bounds[7] = bounds[0];

				newModel->SetGeometryBounds(newModel->GetGeometries().GetCount(), bounds);
			}
		}
	}

	out->SetPrimaryModel();
	out->SetName("lovely.#dr");

	return out;
}
}