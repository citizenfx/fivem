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
	std::vector<uint8_t> outPolyMaterials;

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
		outPolyMaterials.push_back(poly.material);

		if (poly.indices[3])
		{
			outPoly.poly.v1 = poly.indices[2];
			outPoly.poly.v2 = poly.indices[3];
			outPoly.poly.v3 = poly.indices[0];
			outPoly.poly.triangleArea = calculateArea(outPoly);
			outPoly.type = 0;

			outPolys.push_back(outPoly);
			outPolyMaterials.push_back(poly.material);
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

	if (out->GetType() == five::phBoundType::Geometry || out->GetType() == five::phBoundType::BVH)
	{
		five::phBoundGeometry* geom = static_cast<five::phBoundGeometry*>(out);

		geom->SetPolysToMaterials(&outPolyMaterials[0]);
	}
}

static inline void fillGeometryBound(five::phBoundGeometry* out, ny::phBoundGeometry* in)
{
	static std::map<int, int> conversionMap;

#pragma region material conversion mappings
	if (conversionMap.empty())
	{
		conversionMap[0] = 0; // DEFAULT
		conversionMap[1] = 137; // PHYS_NO_FRICTION
		conversionMap[2] = 142; // PHYS_CAR_VOID
		conversionMap[3] = 1; // CONCRETE
		conversionMap[4] = 1; // CONCRETE
		conversionMap[5] = 5; // TARMAC_PAINTED
		conversionMap[6] = 101; // PLASTER_SOLID
		conversionMap[7] = 7; // RUMBLE_STRIP
		conversionMap[8] = 4; // TARMAC
		conversionMap[9] = 8; // BREEZE_BLOCK
		conversionMap[10] = 10; // ROCK_MOSSY
		conversionMap[11] = 9; // ROCK
		conversionMap[12] = 11; // STONE
		conversionMap[13] = 12; // COBBLESTONE
		conversionMap[14] = 13; // BRICK
		conversionMap[15] = 14; // MARBLE
		conversionMap[16] = 15; // PAVING_SLAB
		conversionMap[17] = 81; // CERAMIC
		conversionMap[18] = 82; // ROOF_TILE
		conversionMap[19] = 70; // WOOD_SOLID_MEDIUM
		conversionMap[20] = 70; // WOOD_SOLID_MEDIUM
		conversionMap[21] = 71; // WOOD_SOLID_LARGE
		conversionMap[22] = 75; // WOOD_HOLLOW_MEDIUM
		conversionMap[23] = 69; // WOOD_SOLID_SMALL
		conversionMap[24] = 70; // WOOD_SOLID_MEDIUM
		conversionMap[25] = 78; // WOOD_OLD_CREAKY
		conversionMap[26] = 76; // WOOD_HOLLOW_LARGE
		conversionMap[27] = 96; // LAMINATE
		conversionMap[28] = 72; // WOOD_SOLID_POLISHED
		conversionMap[29] = 31; // GRAVEL_SMALL
		conversionMap[30] = 2; // CONCRETE_POTHOLE
		conversionMap[31] = 32; // GRAVEL_LARGE
		conversionMap[32] = 19; // SAND_COMPACT
		conversionMap[33] = 44; // CLAY_HARD
		conversionMap[34] = 36; // MUD_HARD
		conversionMap[35] = 51; // TWIGS
		conversionMap[36] = 38; // MUD_SOFT
		conversionMap[37] = 47; // GRASS
		conversionMap[38] = 48; // GRASS_SHORT
		conversionMap[39] = 50; // BUSHES
		conversionMap[40] = 43; // SOIL
		conversionMap[41] = 52; // LEAVES
		conversionMap[42] = 53; // WOODCHIPS
		conversionMap[43] = 54; // TREE_BARK
		conversionMap[44] = 54; // TREE_BARK
		conversionMap[45] = 54; // TREE_BARK
		conversionMap[46] = 66; // METAL_DUCT
		conversionMap[47] = 64; // METAL_GRILLE
		conversionMap[48] = 65; // METAL_RAILING
		conversionMap[49] = 63; // METAL_CORRUGATED_IRON
		conversionMap[50] = 59; // METAL_HOLLOW_MEDIUM
		conversionMap[51] = 59; // METAL_HOLLOW_MEDIUM
		conversionMap[52] = 65; // METAL_RAILING
		conversionMap[53] = 59; // METAL_HOLLOW_MEDIUM
		conversionMap[54] = 57; // METAL_SOLID_LARGE
		conversionMap[55] = 60; // METAL_HOLLOW_LARGE
		conversionMap[56] = 57; // METAL_SOLID_LARGE
		conversionMap[57] = 65; // METAL_RAILING
		conversionMap[58] = 67; // METAL_GARAGE_DOOR
		conversionMap[59] = 64; // METAL_GRILLE
		conversionMap[60] = 57; // METAL_SOLID_LARGE
		conversionMap[61] = 62; // METAL_CHAINLINK_LARGE
		conversionMap[62] = 68; // METAL_MANHOLE
		conversionMap[63] = 65; // METAL_RAILING
		conversionMap[64] = 67; // METAL_GARAGE_DOOR
		conversionMap[65] = 55; // METAL_SOLID_SMALL
		conversionMap[66] = 64; // METAL_GRILLE
		conversionMap[67] = 64; // METAL_GRILLE
		conversionMap[68] = 65; // METAL_RAILING
		conversionMap[69] = 67; // METAL_GARAGE_DOOR
		conversionMap[70] = 56; // METAL_SOLID_MEDIUM
		conversionMap[71] = 56; // METAL_SOLID_MEDIUM
		conversionMap[72] = 55; // METAL_SOLID_SMALL
		conversionMap[73] = 113; // GLASS_BULLETPROOF
		conversionMap[74] = 114; // GLASS_MEDIUM
		conversionMap[75] = 114; // GLASS_STRONG
		conversionMap[76] = 114; // GLASS_WEAK
		conversionMap[77] = 115; // PERSPEX
		conversionMap[78] = 114; // GLASS_WEAK
		conversionMap[79] = 116; // CAR_METAL
		conversionMap[80] = 117; // CAR_PLASTIC
		conversionMap[81] = 123; // CAR_GLASS_BULLETPROOF
		conversionMap[82] = 86; // PLASTIC
		conversionMap[83] = 84; // FIBREGLASS
		conversionMap[84] = 86; // PLASTIC
		conversionMap[85] = 85; // TARPAULIN
		conversionMap[86] = 95; // LINOLEUM
		conversionMap[87] = 97; // CARPET_SOLID
		conversionMap[88] = 100; // CLOTH
		conversionMap[89] = 83; // ROOF_FELT
		conversionMap[90] = 97; // CARPET_SOLID
		conversionMap[91] = 93; // RUBBER
		conversionMap[92] = 105; // PAPER
		conversionMap[93] = 103; // CARDBOARD_SHEET
		conversionMap[94] = 106; // FOAM
		conversionMap[95] = 107; // FEATHER_PILLOW
		conversionMap[96] = 125; // WATER
		conversionMap[97] = 125; // WATER
		conversionMap[98] = 125; // WATER
		conversionMap[99] = 143; // PHYS_PED_CAPSULE
		conversionMap[100] = 150; // BUTTOCKS
		conversionMap[101] = 151; // THIGH_LEFT
		conversionMap[102] = 152; // SHIN_LEFT
		conversionMap[103] = 153; // FOOT_LEFT
		conversionMap[104] = 154; // THIGH_RIGHT
		conversionMap[105] = 155; // SHIN_RIGHT
		conversionMap[106] = 156; // FOOT_RIGHT
		conversionMap[107] = 157; // SPINE0
		conversionMap[108] = 158; // SPINE1
		conversionMap[109] = 159; // SPINE2
		conversionMap[110] = 160; // SPINE3
		conversionMap[111] = 169; // NECK
		conversionMap[112] = 170; // HEAD
		conversionMap[113] = 161; // CLAVICLE_LEFT
		conversionMap[114] = 162; // UPPER_ARM_LEFT
		conversionMap[115] = 163; // LOWER_ARM_LEFT
		conversionMap[116] = 164; // HAND_LEFT
		conversionMap[117] = 165; // CLAVICLE_RIGHT
		conversionMap[118] = 166; // UPPER_ARM_RIGHT
		conversionMap[119] = 167; // LOWER_ARM_RIGHT
		conversionMap[120] = 168; // HAND_RIGHT
		conversionMap[121] = 140; // PHYS_CASTER
		conversionMap[122] = 75; // WOOD_HOLLOW_MEDIUM
		conversionMap[123] = 86; // PLASTIC
		conversionMap[124] = 103; // CARDBOARD_SHEET
		conversionMap[125] = 84; // FIBREGLASS
		conversionMap[126] = 120; // CAR_GLASS_WEAK
		conversionMap[127] = 0; // CAR_GLASS_MED
		conversionMap[128] = 122; // CAR_GLASS_STRONG
		conversionMap[129] = 123; // CAR_GLASS_BULLETPROOF
		conversionMap[130] = 110; // TVSCREEN
		conversionMap[131] = 110; // TVSCREEN
		conversionMap[132] = 0; // DEFAULT
		conversionMap[133] = 0; // DEFAULT
		conversionMap[134] = 0; // DEFAULT
		conversionMap[135] = 120; // CAR_GLASS_WEAK
		conversionMap[136] = 0; // CAR_GLASS_MED
		conversionMap[137] = 122; // CAR_GLASS_STRONG
		conversionMap[138] = 123; // CAR_GLASS_BULLETPROOF
		conversionMap[139] = 0; // DEFAULT
		conversionMap[140] = 0; // DEFAULT
		conversionMap[141] = 0; // DEFAULT
		conversionMap[142] = 0; // DEFAULT
		conversionMap[143] = 0; // DEFAULT
		conversionMap[144] = 0; // DEFAULT
		conversionMap[145] = 0; // DEFAULT
		conversionMap[146] = 0; // DEFAULT
		conversionMap[147] = 0; // DEFAULT
		conversionMap[148] = 0; // DEFAULT
		conversionMap[149] = 131; // EMISSIVE_GLASS
		conversionMap[150] = 132; // EMISSIVE_PLASTIC
		conversionMap[151] = 114; // GLASS_STRONG
		conversionMap[152] = 47; // GRASS
		conversionMap[153] = 47; // GRASS
		conversionMap[154] = 97; // CARPET_SOLID
		conversionMap[155] = 0; // DEFAULT
	}
#pragma endregion

	uint32_t materialColors[] = { 0x208DFFFF };

	out->SetMaterialColors(1, materialColors);

	std::vector<five::phBoundMaterial> materials(in->GetNumMaterials());
	ny::phBoundMaterial* inMaterials = in->GetMaterials();
	
	for (int i = 0; i < materials.size(); i++)
	{
		materials[i].materialIdx = conversionMap[inMaterials[i].materialIdx];
		materials[i].pad2 = 0x100;
	}

	out->SetMaterials(materials.size(), &materials[0]);
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