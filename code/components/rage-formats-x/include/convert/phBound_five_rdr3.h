/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <string>

#define RAGE_FORMATS_GAME rdr3
#define RAGE_FORMATS_GAME_RDR3
#include <phBound.h>

#undef RAGE_FORMATS_GAME_RDR3
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <phBound.h>

#include <convert/base.h>

#include <map>
#include <vector>

inline float g_boundOffset[2];

namespace rage
{
static inline void fillBaseBound(rdr3::phBound* out, five::phBound* in)
{
	auto& centroid = in->GetCentroid();
	out->SetCentroid(rdr3::phVector3(centroid.x + g_boundOffset[0], centroid.y + g_boundOffset[1], centroid.z));

	auto& cg = in->GetCG();
	out->SetCG(rdr3::phVector3(cg.x + g_boundOffset[0], cg.y + g_boundOffset[1], cg.z));

	auto& aabbMin = in->GetAABBMin();
	out->SetAABBMin(rdr3::phVector3(aabbMin.x + g_boundOffset[0], aabbMin.y + g_boundOffset[1], aabbMin.z));

	auto& aabbMax = in->GetAABBMax();
	out->SetAABBMax(rdr3::phVector3(aabbMax.x + g_boundOffset[0], aabbMax.y + g_boundOffset[1], aabbMax.z));

	out->SetRadius(in->GetRadius());
	out->SetMargin(in->GetMargin());
}

static rdr3::phBound* convertBoundToFive(five::phBound* bound);

template<>
rdr3::phBoundComposite* convert(five::phBoundComposite* bound)
{
	auto out = new (false) rdr3::phBoundComposite;
	out->SetBlockMap();

	fillBaseBound(out, bound);

	out->SetUnkFloat(5.0f);

	// convert child bounds
	uint16_t childCount = bound->GetNumChildBounds();
	std::vector<rdr3::phBound*> children(childCount);

	for (uint16_t i = 0; i < childCount; i++)
	{
		children[i] = convertBoundToFive(bound->GetChildBound(i));
	}

	out->SetChildBounds(childCount, &children[0]);

#if 0
	// convert aux data
	std::vector<rdr3::phBoundAABB> childAABBs(childCount);
	five::phBoundAABB* inAABBs = bound->GetChildAABBs();

	for (uint16_t i = 0; i < childCount; i++)
	{
		childAABBs[i].min = rdr3::phVector3(inAABBs[i].min.x + g_boundOffset[0], inAABBs[i].min.y + g_boundOffset[1], inAABBs[i].min.z);
		childAABBs[i].intUnk = 1;
		childAABBs[i].max = rdr3::phVector3(inAABBs[i].max.x + g_boundOffset[0], inAABBs[i].max.y + g_boundOffset[1], inAABBs[i].max.z);
		childAABBs[i].floatUnk = 0.005f;
	}

	out->SetChildAABBs(childCount, &childAABBs[0]);
#endif

	// convert matrices
	std::vector<rdr3::Matrix3x4> childMatrices(childCount);
	five::Matrix3x4* inMatrices = bound->GetChildMatrices();

	memcpy(&childMatrices[0], inMatrices, sizeof(rdr3::Matrix3x4) * childCount);

	out->SetChildMatrices(childCount, &childMatrices[0]);

	// add bound flags
	std::vector<rdr3::phBoundFlagEntry> boundFlags(childCount);

	for (uint16_t i = 0; i < childCount; i++)
	{
		boundFlags[i].m_0 = 0x3E;
		boundFlags[i].pad = 0;
		boundFlags[i].m_4 = 0x7F3BEC0;
		boundFlags[i].pad2 = 0;
	}

	out->SetBoundFlags(childCount, &boundFlags[0]);

	return out;
}

static inline void fillPolyhedronBound(rdr3::phBoundPolyhedron* out, five::phBoundPolyhedron* in)
{
	auto& quantum = in->GetQuantum();
	out->SetQuantum(rdr3::Vector4(quantum.x, quantum.y, quantum.z, /*quantum.w*/ 7.62962742e-008));

	auto& offset = in->GetVertexOffset();
	out->SetVertexOffset(rdr3::Vector4(offset.x + g_boundOffset[0], offset.y + g_boundOffset[1], offset.z, /*offset.w*/ 0.0025f));

	// vertices
	five::phBoundVertex* vertices = in->GetVertices();
	std::vector<rdr3::phBoundVertex> outVertices(in->GetNumVertices());

	memcpy(&outVertices[0], vertices, outVertices.size() * sizeof(*vertices));

	out->SetVertices(outVertices.size(), &outVertices[0]);

	// polys
	five::phBoundPoly* polys = in->GetPolygons();
	uint32_t numPolys = in->GetNumPolygons();

	std::vector<rdr3::phBoundPoly> outPolys(numPolys);
	std::vector<uint8_t> outPolyMaterials(numPolys);

	//memcpy(outPolys.data(), in->GetPolygons(), sizeof(rdr3::phBoundPoly) * numPolys);

	auto vertToVector = [&](const auto& vertex)
	{
		return rage::Vector3(
		(vertex.x * quantum.x) + offset.x,
		(vertex.y * quantum.y) + offset.y,
		(vertex.z * quantum.z) + offset.z);
	};

	auto calculateArea = [&](const rdr3::phBoundPoly& poly)
	{
		rage::Vector3 v1 = vertToVector(vertices[poly.poly.v1]);
		rage::Vector3 v2 = vertToVector(vertices[poly.poly.v2]);
		rage::Vector3 v3 = vertToVector(vertices[poly.poly.v3]);

		rage::Vector3 d1 = v1 - v2;
		rage::Vector3 d2 = v3 - v2;

		rage::Vector3 cross = rage::Vector3::CrossProduct(d1, d2);

		return cross.Length() / 2.0f;
	};

	auto inPolys = in->GetPolygons();

	for (int i = 0; i < numPolys; i++)
	{
		if (inPolys[i].type == 0)
		{
			outPolys[i].poly.v1 = inPolys[i].poly.v1;
			outPolys[i].poly.v2 = inPolys[i].poly.v2;
			outPolys[i].poly.v3 = inPolys[i].poly.v3;
			outPolys[i].poly.e1 = inPolys[i].poly.e1;
			outPolys[i].poly.e2 = inPolys[i].poly.e2;
			outPolys[i].poly.e3 = inPolys[i].poly.e3;
			outPolys[i].poly.triangleArea = calculateArea(outPolys[i]);
			outPolys[i].type = 0;
		}
		else if (inPolys[i].type == 1)
		{
			outPolys[i].sphere.index = inPolys[i].sphere.index;
			outPolys[i].sphere.radius = inPolys[i].sphere.radius;
			outPolys[i].type = 1;
		}
		else if (inPolys[i].type == 2)
		{
			outPolys[i].capsule.index = inPolys[i].capsule.index;
			outPolys[i].capsule.length = inPolys[i].capsule.length;
			outPolys[i].capsule.indexB = inPolys[i].capsule.indexB;
			outPolys[i].type = 2;
		}
		else if (inPolys[i].type == 3)
		{
			memcpy(&outPolys[i].box.indices, &inPolys[i].box.indices, sizeof(inPolys[i].box.indices));
			outPolys[i].type = 3;
		}
		else if (inPolys[i].type == 4)
		{
			memcpy(&outPolys[i].box.indices, &inPolys[i].box.indices, sizeof(inPolys[i].box.indices));
			outPolys[i].type = 4;
		}
	}

	if (((five::phBoundGeometry*)in)->GetPolysToMaterials())
	{
		memcpy(outPolyMaterials.data(), ((five::phBoundGeometry*)in)->GetPolysToMaterials(), sizeof(uint8_t) * numPolys);
	}

	out->SetPolys(outPolys.size(), &outPolys[0]);

	if (out->GetType() == rdr3::phBoundType::Geometry || out->GetType() == rdr3::phBoundType::BVH)
	{
		if (!outPolyMaterials.empty())
		{
			rdr3::phBoundGeometry* geom = static_cast<rdr3::phBoundGeometry*>(out);

			geom->SetPolysToMaterials(&outPolyMaterials[0]);
		}
	}
}

static inline uint8_t ConvertMaterialIndexrdr3(uint8_t index)
{
	return index;
}

static inline void fillGeometryBound(rdr3::phBoundGeometry* out, five::phBoundGeometry* in)
{
	uint32_t materialColors[] = { 0x208DFFFF };

	out->SetMaterialColors(1, materialColors);

	std::vector<rdr3::phBoundMaterial> materials(in->GetNumMaterials());
	five::phBoundMaterial* inMaterials = in->GetMaterials();

	for (int i = 0; i < materials.size(); i++)
	{
		materials[i].mat1.materialIdx = ConvertMaterialIndexrdr3(inMaterials[i].mat1.materialIdx);
		materials[i].mat1.roomId = inMaterials[i].mat1.roomId;
		materials[i].mat2.materialColorIdx = 0x1;
	}

	out->SetMaterials(materials.size(), &materials[0]);
}

template<>
rdr3::phBoundGeometry* convert(five::phBoundGeometry* bound)
{
	auto out = new (false) rdr3::phBoundGeometry;

	fillBaseBound(out, bound);
	fillPolyhedronBound(out, bound);
	fillGeometryBound(out, bound);

	return out;
}

template<>
rdr3::phBVH* convert(five::phBVH* in)
{
	auto out = new (false) rdr3::phBVH;

	out->SetAABB(
	Vector3(in->m_aabbMin.x, in->m_aabbMin.y, in->m_aabbMin.z),
	Vector3(in->m_aabbMax.x, in->m_aabbMax.y, in->m_aabbMax.z));

	out->SetBVH(
	in->m_nodes.GetCount(),
	(rage::rdr3::phBVHNode*)&in->m_nodes.Get(0));

	return out;
}

template<>
rdr3::phBoundBVH* convert(five::phBoundBVH* bound)
{
	auto out = new (false) rdr3::phBoundBVH;

	fillBaseBound(out, bound);
	fillPolyhedronBound(out, bound);
	fillGeometryBound(out, bound);

	out->SetBVH(convert<rdr3::phBVH*>(bound->GetBVH()));
	//rdr3::CalculateBVH(out);

	return out;
}

template<>
rdr3::phBoundSphere* convert(five::phBoundSphere* bound)
{
	auto out = new (false) rdr3::phBoundSphere;

	fillBaseBound(out, bound);

	return out;
}

template<>
rdr3::phBoundBox* convert(five::phBoundBox* bound)
{
	auto out = new (false) rdr3::phBoundBox;

	fillBaseBound(out, bound);

	//rage::rdr3::phBoundMaterial material = { 0 };
	//material.mat1.materialIdx = ConvertMaterialIndex(bound->GetMaterial().mat1.materialIdx);
	//out->SetMaterial(material);

	return out;
}

template<>
rdr3::phBoundCapsule* convert(five::phBoundCapsule* bound)
{
	auto out = new (false) rdr3::phBoundCapsule;

	fillBaseBound(out, bound);

	return out;
}

static rdr3::phBound* convertBoundToFive(five::phBound* bound)
{
	switch (bound->GetType())
	{
		case five::phBoundType::Sphere:
			return convert<rdr3::phBoundSphere*>(static_cast<five::phBoundSphere*>(bound));

		case five::phBoundType::Composite:
			return convert<rdr3::phBoundComposite*>(static_cast<five::phBoundComposite*>(bound));

		case five::phBoundType::Geometry:
			return convert<rdr3::phBoundGeometry*>(static_cast<five::phBoundGeometry*>(bound));

		case five::phBoundType::BVH:
			return convert<rdr3::phBoundBVH*>(static_cast<five::phBoundBVH*>(bound));

		case five::phBoundType::Box:
			return convert<rdr3::phBoundBox*>(static_cast<five::phBoundBox*>(bound));

		case five::phBoundType::Capsule:
			return convert<rdr3::phBoundCapsule*>(static_cast<five::phBoundCapsule*>(bound));
	}

	return nullptr;
}

template<>
rdr3::phBoundComposite* convert(five::phBound* bound)
{
	if (bound->GetType() == five::phBoundType::Composite)
	{
		return (rdr3::phBoundComposite*)convertBoundToFive(bound);
	}

	auto out = new (false) rdr3::phBoundComposite;
	out->SetBlockMap();

	auto originalBound = convertBoundToFive(bound);

	fillBaseBound(out, bound);

	out->SetUnkFloat(5.0f);

	// convert child bounds
	out->SetChildBounds(1, &originalBound);

#if 0
	// convert aux data
	rdr3::phBoundAABB aabb;
	aabb.min = rdr3::phVector3(bound->GetAABBMin().x + g_boundOffset[0], bound->GetAABBMin().y + g_boundOffset[1], bound->GetAABBMin().z);
	aabb.max = rdr3::phVector3(bound->GetAABBMax().x + g_boundOffset[0], bound->GetAABBMax().y + g_boundOffset[1], bound->GetAABBMax().z);
	aabb.intUnk = 1;
	aabb.floatUnk = 0.005f;
	out->SetChildAABBs(1, &aabb);
#endif

	// convert matrices
	rdr3::Matrix3x4 childMatrix;
	childMatrix._1 = { 1.f, 0.f, 0.f };
	childMatrix._2 = { 0.f, 1.f, 0.f };
	childMatrix._3 = { 0.f, 0.f, 1.f };
	childMatrix._4 = { 0.f, 0.f, 0.f };

	out->SetChildMatrices(1, &childMatrix);

	// add bound flags
	rdr3::phBoundFlagEntry boundFlag;
	boundFlag.m_0 = 0x3E;
	boundFlag.pad = 0;
	boundFlag.m_4 = 0x7F3BEC0;
	boundFlag.pad2 = 0;

	out->SetBoundFlags(1, &boundFlag);

	return out;
}

template<>
rdr3::phBoundComposite* convert(five::datOwner<five::phBound>* bound)
{
	return convert<rdr3::phBoundComposite*>(bound->GetChild());
}

template<>
rdr3::phBound* convert(five::phBound* bound)
{
	return convertBoundToFive(bound);
}

template<>
rdr3::pgDictionary<rdr3::phBound>* convert(five::pgDictionary<five::phBound>* phd)
{
	rdr3::pgDictionary<rdr3::phBound>* out = new (false) rdr3::pgDictionary<rdr3::phBound>();
	out->SetBlockMap();

	rdr3::pgDictionary<rdr3::phBound> newDrawables;

	if (phd->GetCount())
	{
		for (auto& bound : *phd)
		{
			newDrawables.Add(bound.first, convert<rdr3::phBound*>(bound.second));
		}
	}

	out->SetFrom(&newDrawables);

	return out;
}
}
