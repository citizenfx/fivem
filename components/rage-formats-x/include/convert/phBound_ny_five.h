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
#include <phBound.h>

#undef RAGE_FORMATS_GAME_NY
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <phBound.h>

#include <convert/base.h>

#include <map>
#include <vector>

namespace rage
{
static inline void fillBaseBound(five::phBound* out, ny::phBound* in)
{
	auto& centroid = in->GetCentroid();
	out->SetCentroid(five::phVector3(centroid.x, centroid.y, centroid.z));

	auto& cg = in->GetCG();
	out->SetCG(five::phVector3(cg.x, cg.y, cg.z));

	auto& aabbMin = in->GetAABBMin();
	out->SetAABBMin(five::phVector3(aabbMin.x, aabbMin.y, aabbMin.z));

	auto& aabbMax = in->GetAABBMax();
	out->SetAABBMax(five::phVector3(aabbMax.x, aabbMax.y, aabbMax.z));

	out->SetRadius(in->GetRadius());
	out->SetMargin(in->GetMargin());
}

static five::phBound* convertBoundToFive(ny::phBound* bound);

template<>
five::phBoundComposite* convert(ny::phBoundComposite* bound)
{
	auto out = new(false) five::phBoundComposite;
	out->SetBlockMap();

	fillBaseBound(out, bound);

	out->SetUnkFloat(5.0f);

	//out->SetUnkVector(five::phVector3(256.436523f, 413.156311f, 451.682312f));

	// convert child bounds
	uint16_t childCount = bound->GetNumChildBounds();
	std::vector<five::phBound*> children(childCount);

	for (uint16_t i = 0; i < childCount; i++)
	{
		children[i] = convertBoundToFive(bound->GetChildBound(i));
	}

	out->SetChildBounds(childCount, &children[0]);

	// convert aux data
	std::vector<five::phBoundAABB> childAABBs(childCount);
	ny::phBoundAABB* inAABBs = bound->GetChildAABBs();

	for (uint16_t i = 0; i < childCount; i++)
	{
		childAABBs[i].min = five::phVector3(inAABBs[i].min.x, inAABBs[i].min.y, inAABBs[i].min.z);
		childAABBs[i].intUnk = 1;
		childAABBs[i].max = five::phVector3(inAABBs[i].max.x, inAABBs[i].max.y, inAABBs[i].max.z);
		childAABBs[i].floatUnk = 0.005f;
	}

	out->SetChildAABBs(childCount, &childAABBs[0]);

	// convert matrices
	std::vector<five::Matrix3x4> childMatrices(childCount);
	ny::Matrix3x4* inMatrices = bound->GetChildMatrices();

	memcpy(&childMatrices[0], inMatrices, sizeof(five::Matrix3x4) * childCount);

	out->SetChildMatrices(childCount, &childMatrices[0]);

	// add bound flags
	std::vector<five::phBoundFlagEntry> boundFlags(childCount);

	for (uint16_t i = 0; i < childCount; i++)
	{
		boundFlags[i].m_0 = 0x3E;
		boundFlags[i].m_4 = 0x7F3BEC0;
	}

	out->SetBoundFlags(childCount, &boundFlags[0]);

	return out;
}

static inline void fillPolyhedronBound(five::phBoundPolyhedron* out, ny::phBoundPolyhedron* in)
{
	auto& quantum = in->GetQuantum();
	out->SetQuantum(five::Vector4(quantum.x, quantum.y, quantum.z, /*quantum.w*/7.62962742e-008));

	auto& offset = in->GetVertexOffset();
	out->SetVertexOffset(five::Vector4(offset.x, offset.y, offset.z, /*offset.w*/0.0025f));

	// vertices
	ny::phBoundVertex* vertices = in->GetVertices();
	std::vector<five::phBoundVertex> outVertices(in->GetNumVertices());

	memcpy(&outVertices[0], vertices, outVertices.size() * sizeof(*vertices));

	out->SetVertices(outVertices.size(), &outVertices[0]);

	// polys
	ny::phBoundPoly* polys = in->GetPolygons();
	uint32_t numPolys = in->GetNumPolygons();

	// TODO: save polygon mapping in a temporary variable (for BVH purposes)
	auto vertToVector = [&] (const auto& vertex)
	{
		return rage::Vector3(
			(vertex.x * quantum.x) + offset.x,
			(vertex.y * quantum.y) + offset.y,
			(vertex.z * quantum.z) + offset.z
		);
	};

	auto calculateArea = [&] (const five::phBoundPoly& poly)
	{
		rage::Vector3 v1 = vertToVector(vertices[poly.poly.v1]);
		rage::Vector3 v2 = vertToVector(vertices[poly.poly.v2]);
		rage::Vector3 v3 = vertToVector(vertices[poly.poly.v3]);

		rage::Vector3 d1 = v1 - v2;
		rage::Vector3 d2 = v3 - v2;

		rage::Vector3 cross = rage::Vector3::CrossProduct(d1, d2);

		return cross.Length() / 2.0f;
	};

	struct PolyEdge
	{
		uint32_t edges[3];
	};

	std::vector<five::phBoundPoly> outPolys;
	std::vector<PolyEdge> outPolyEdges;

	for (uint16_t i = 0; i < numPolys; i++)
	{
		auto& poly = polys[i];

		five::phBoundPoly outPoly;
		outPoly.poly.v1 = poly.indices[0];
		outPoly.poly.v2 = poly.indices[1];
		outPoly.poly.v3 = poly.indices[2];
		outPoly.poly.triangleArea = calculateArea(outPoly);
		outPoly.type = 0;

		outPolys.push_back(outPoly);

		if (poly.indices[3])
		{
			outPoly.poly.v1 = poly.indices[2];
			outPoly.poly.v2 = poly.indices[3];
			outPoly.poly.v3 = poly.indices[0];
			outPoly.poly.triangleArea = calculateArea(outPoly);
			outPoly.type = 0;

			outPolys.push_back(outPoly);
		}
	}

	auto makeEdge = [] (uint16_t a, uint16_t b)
	{
		if (a < b)
		{
			return (a << 16) | b;
		}
		else
		{
			return (b << 16) | a;
		}
	};

	struct PolyEdgeMap
	{
		uint32_t left;
		uint32_t right;
	};

	std::map<uint32_t, PolyEdgeMap> edgeMapping;

	for (int i = 0; i < outPolys.size(); i++)
	{
		auto& outPoly = outPolys[i];

		PolyEdge edge;
		edge.edges[0] = makeEdge(outPoly.poly.v1, outPoly.poly.v2);
		edge.edges[1] = makeEdge(outPoly.poly.v2, outPoly.poly.v3);
		edge.edges[2] = makeEdge(outPoly.poly.v3, outPoly.poly.v1);
		outPolyEdges.push_back(edge);

		for (int j = 0; j < 3; j++)
		{
			auto it = edgeMapping.find(edge.edges[j]);

			if (it == edgeMapping.end())
			{
				PolyEdgeMap map;
				map.left = i;
				map.right = -1;

				edgeMapping[edge.edges[j]] = map;
			}
			else
			{
				auto& edgeMap = it->second;

				if (edgeMap.right == -1)
				{
					edgeMap.right = i;
				}
			}
		}
	}

	auto findEdge = [&] (int i, int edgeIdx) -> int16_t
	{
		auto& edge = outPolyEdges[i].edges[edgeIdx];
		auto& map = edgeMapping[edge];

		if (map.right == -1)
		{
			return -1;
		}
		else
		{
			if (map.left == i)
			{
				return map.right;
			}
			else
			{
				return map.left;
			}
		}
	};

	for (int i = 0; i < outPolys.size(); i++)
	{
		auto& outPoly = outPolys[i];

		outPoly.poly.e1 = findEdge(i, 0);
		outPoly.poly.e2 = findEdge(i, 1);
		outPoly.poly.e3 = findEdge(i, 2);
	}

	out->SetPolys(outPolys.size(), &outPolys[0]);
}

static inline void fillGeometryBound(five::phBoundGeometry* out, ny::phBoundGeometry* in)
{
	uint32_t materialColors[] = { 0x208Dffff };

	out->SetMaterialColors(1, materialColors);

	uint32_t materials[] = { 1, 0x100, 0, 0, 0, 0 };

	out->SetMaterials(1, materials, sizeof(materials));

	std::vector<uint8_t> materialMappings(out->GetNumPolygons());
	out->SetPolysToMaterials(&materialMappings[0]);
}

template<>
five::phBoundGeometry* convert(ny::phBoundGeometry* bound)
{
	auto out = new(false) five::phBoundGeometry;

	fillBaseBound(out, bound);
	fillPolyhedronBound(out, bound);
	fillGeometryBound(out, bound);

	return out;
}

template<>
five::phBoundBVH* convert(ny::phBoundBVH* bound)
{
	auto out = new(false) five::phBoundBVH;

	fillBaseBound(out, bound);
	fillPolyhedronBound(out, bound);
	fillGeometryBound(out, bound);

	return out;
}

static five::phBound* convertBoundToFive(ny::phBound* bound)
{
	switch (bound->GetType())
	{
		//case ny::phBoundType::BVH:
		//	return convert<five::phBoundBVH*>(static_cast<ny::phBoundBVH*>(bound));

		case ny::phBoundType::Composite:
			return convert<five::phBoundComposite*>(static_cast<ny::phBoundComposite*>(bound));

		case ny::phBoundType::Geometry:
			return convert<five::phBoundGeometry*>(static_cast<ny::phBoundGeometry*>(bound));

		case ny::phBoundType::BVH:
			return convert<five::phBoundBVH*>(static_cast<ny::phBoundBVH*>(bound));
	}

	return nullptr;
}

template<>
five::phBound* convert(ny::phBound* bound)
{
	return convertBoundToFive(bound);
}
}