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

namespace rage
{
	static inline void fillBaseBound(five::phBound* out, rdr3::phBound* in)
	{
		out->SetBlockMap();

		auto& centroid = in->GetCentroid();
		out->SetCentroid(five::phVector3(centroid.x + g_boundOffset[0], centroid.y + g_boundOffset[1], centroid.z));

		auto& cg = in->GetCG();
		out->SetCG(five::phVector3(cg.x + g_boundOffset[0], cg.y + g_boundOffset[1], cg.z));

		auto& aabbMin = in->GetAABBMin();
		out->SetAABBMin(five::phVector3(aabbMin.x + g_boundOffset[0], aabbMin.y + g_boundOffset[1], aabbMin.z));

		auto& aabbMax = in->GetAABBMax();
		out->SetAABBMax(five::phVector3(aabbMax.x + g_boundOffset[0], aabbMax.y + g_boundOffset[1], aabbMax.z));

		out->SetRadius(in->GetRadius());
		out->SetMargin(in->GetMargin());
	}

	static five::phBound* convertBoundToFive(rdr3::phBound* bound);

	template<>
	five::phBoundComposite* convert(rdr3::phBoundComposite* bound)
	{
		auto out = new(false) five::phBoundComposite;
		out->SetBlockMap();

		fillBaseBound(out, bound);

		out->SetUnkFloat(5.0f);

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
		rdr3::phBoundAABB* inAABBs = bound->GetChildAABBs();

		for (uint16_t i = 0; i < childCount; i++)
		{
			childAABBs[i].min = five::phVector3(inAABBs[i].min.x + g_boundOffset[0], inAABBs[i].min.y + g_boundOffset[1], inAABBs[i].min.z);
			childAABBs[i].intUnk = 1;
			childAABBs[i].max = five::phVector3(inAABBs[i].max.x + g_boundOffset[0], inAABBs[i].max.y + g_boundOffset[1], inAABBs[i].max.z);
			childAABBs[i].floatUnk = 0.005f;
		}

		out->SetChildAABBs(childCount, &childAABBs[0]);

		// convert matrices
		std::vector<five::Matrix3x4> childMatrices(childCount);
		rdr3::Matrix3x4* inMatrices = bound->GetChildMatrices();

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

	static inline void fillPolyhedronBound(five::phBoundPolyhedron* out, rdr3::phBoundPolyhedron* in)
	{
		auto& quantum = in->GetQuantum();
		out->SetQuantum(five::Vector4(quantum.x, quantum.y, quantum.z, /*quantum.w*/7.62962742e-008));

		auto& offset = in->GetVertexOffset();
		out->SetVertexOffset(five::Vector4(offset.x + g_boundOffset[0], offset.y + g_boundOffset[1], offset.z, /*offset.w*/0.0025f));

		// vertices
		rdr3::phBoundVertex* vertices = in->GetVertices();
		std::vector<five::phBoundVertex> outVertices(in->GetNumVertices());

		memcpy(&outVertices[0], vertices, outVertices.size() * sizeof(*vertices));

		out->SetVertices(outVertices.size(), &outVertices[0]);

		// polys
		rdr3::phBoundPoly* polys = in->GetPolygons();
		uint32_t numPolys = in->GetNumPolygons();

		std::vector<five::phBoundPoly> outPolys(numPolys);
		std::vector<uint8_t> outPolyMaterials(numPolys);

		//memcpy(outPolys.data(), in->GetPolygons(), sizeof(five::phBoundPoly) * numPolys);

		auto vertToVector = [&](const auto& vertex)
		{
			return rage::Vector3(
				(vertex.x * quantum.x) + offset.x,
				(vertex.y * quantum.y) + offset.y,
				(vertex.z * quantum.z) + offset.z
			);
		};

		auto calculateArea = [&](const five::phBoundPoly& poly)
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
			else
			{
				printf("unhandled primitive type %d\n", inPolys[i].type);
				__debugbreak();
			}
		}

		if (((rdr3::phBoundGeometry*)in)->GetPolysToMaterials())
		{
			memcpy(outPolyMaterials.data(), ((rdr3::phBoundGeometry*)in)->GetPolysToMaterials(), sizeof(uint8_t) * numPolys);
		}

		out->SetPolys(outPolys.size(), &outPolys[0]);

		if (out->GetType() == five::phBoundType::Geometry || out->GetType() == five::phBoundType::BVH)
		{
			if (!outPolyMaterials.empty())
			{
				five::phBoundGeometry* geom = static_cast<five::phBoundGeometry*>(out);

				geom->SetPolysToMaterials(&outPolyMaterials[0]);
			}
		}
	}

	static inline uint8_t ConvertMaterialIndexrdr3(uint8_t index)
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

		return conversionMap[index];
	}

	static inline void fillGeometryBound(five::phBoundGeometry* out, rdr3::phBoundGeometry* in)
	{
		uint32_t materialColors[] = { 0x208DFFFF };

		out->SetMaterialColors(1, materialColors);

		std::vector<five::phBoundMaterial> materials(in->GetNumMaterials());
		rdr3::phBoundMaterial* inMaterials = in->GetMaterials();

		for (int i = 0; i < materials.size(); i++)
		{
			materials[i].mat1.materialIdx = ConvertMaterialIndexrdr3(inMaterials[i].mat1.materialIdx);
			materials[i].mat1.roomId = inMaterials[i].mat1.roomId;
			materials[i].mat2.materialColorIdx = 0x1;
		}

		out->SetMaterials(materials.size(), &materials[0]);
	}

	template<>
	five::phBoundGeometry* convert(rdr3::phBoundGeometry* bound)
	{
		auto out = new(false) five::phBoundGeometry;

		fillBaseBound(out, bound);
		fillPolyhedronBound(out, bound);
		fillGeometryBound(out, bound);

		return out;
	}

	template<>
	five::phBVH* convert(rdr3::phBVH* in)
	{
		auto out = new(false) five::phBVH;

		out->SetAABB(
			Vector3(in->m_aabbMin.x, in->m_aabbMin.y, in->m_aabbMin.z),
			Vector3(in->m_aabbMax.x, in->m_aabbMax.y, in->m_aabbMax.z));

		out->SetBVH(
			in->m_nodes.GetCount(),
			(rage::five::phBVHNode*)&in->m_nodes.Get(0),
			0,
			NULL
		);

		return out;
	}

	template<>
	five::phBoundBVH* convert(rdr3::phBoundBVH* bound)
	{
		auto out = new(false) five::phBoundBVH;

		fillBaseBound(out, bound);
		fillPolyhedronBound(out, bound);
		fillGeometryBound(out, bound);

		//out->SetBVH(convert<five::phBVH*>(bound->GetBVH()));
		//five::CalculateBVH(out);

		return out;
	}

	template<>
	five::phBoundSphere* convert(rdr3::phBoundSphere* bound)
	{
		auto out = new(false) five::phBoundSphere;

		fillBaseBound(out, bound);

		return out;
	}

	template<>
	five::phBoundBox* convert(rdr3::phBoundBox* bound)
	{
		auto out = new(false) five::phBoundBox;

		fillBaseBound(out, bound);

		//rage::five::phBoundMaterial material = { 0 };
		//material.mat1.materialIdx = ConvertMaterialIndex(bound->GetMaterial().mat1.materialIdx);
		//out->SetMaterial(material);

		return out;
	}

	template<>
	five::phBoundCapsule* convert(rdr3::phBoundCapsule* bound)
	{
		auto out = new(false) five::phBoundCapsule;

		fillBaseBound(out, bound);

		return out;
	}

	static five::phBound* convertBoundToFive(rdr3::phBound* bound)
	{
		switch (bound->GetType())
		{
		case rdr3::phBoundType::Sphere:
			return convert<five::phBoundSphere*>(static_cast<rdr3::phBoundSphere*>(bound));

		case rdr3::phBoundType::Composite:
			return convert<five::phBoundComposite*>(static_cast<rdr3::phBoundComposite*>(bound));

		case rdr3::phBoundType::Geometry:
			return convert<five::phBoundGeometry*>(static_cast<rdr3::phBoundGeometry*>(bound));

		case rdr3::phBoundType::BVH:
			return convert<five::phBoundBVH*>(static_cast<rdr3::phBoundBVH*>(bound));

		case rdr3::phBoundType::Box:
			return convert<five::phBoundBox*>(static_cast<rdr3::phBoundBox*>(bound));

		case rdr3::phBoundType::Capsule:
			return convert<five::phBoundCapsule*>(static_cast<rdr3::phBoundCapsule*>(bound));
		}

		return nullptr;
	}

	template<>
	five::phBoundComposite* convert(rdr3::phBound* bound)
	{
		if (bound->GetType() == rdr3::phBoundType::Composite)
		{
			return (five::phBoundComposite*)convertBoundToFive(bound);
		}

		auto out = new(false) five::phBoundComposite;
		out->SetBlockMap();

		auto originalBound = convertBoundToFive(bound);

		fillBaseBound(out, bound);

		out->SetUnkFloat(5.0f);

		// convert child bounds
		out->SetChildBounds(1, &originalBound);

		// convert aux data
		five::phBoundAABB aabb;
		aabb.min = five::phVector3(bound->GetAABBMin().x + g_boundOffset[0], bound->GetAABBMin().y + g_boundOffset[1], bound->GetAABBMin().z);
		aabb.max = five::phVector3(bound->GetAABBMax().x + g_boundOffset[0], bound->GetAABBMax().y + g_boundOffset[1], bound->GetAABBMax().z);
		aabb.intUnk = 1;
		aabb.floatUnk = 0.005f;
		out->SetChildAABBs(1, &aabb);

		// convert matrices
		five::Matrix3x4 childMatrix;
		childMatrix._1 = { 1.f, 0.f, 0.f };
		childMatrix._2 = { 0.f, 1.f, 0.f };
		childMatrix._3 = { 0.f, 0.f, 1.f };
		childMatrix._4 = { 0.f, 0.f, 0.f };

		out->SetChildMatrices(1, &childMatrix);

		// add bound flags
		five::phBoundFlagEntry boundFlag;
		boundFlag.m_0 = 0x3E;
		boundFlag.m_4 = 0x7F3BEC0;

		out->SetBoundFlags(1, &boundFlag);

		return out;
	}

	template<>
	five::phBoundComposite* convert(rdr3::datOwner<rdr3::phBound>* bound)
	{
		return convert<five::phBoundComposite*>(bound->GetChild());
	}

	template<>
	five::phBound* convert(rdr3::phBound* bound)
	{
		return convertBoundToFive(bound);
	}

	template<>
	five::pgDictionary<five::phBound>* convert(rdr3::pgDictionary<rdr3::phBound>* phd)
	{
		five::pgDictionary<five::phBound>* out = new(false) five::pgDictionary<five::phBound>();
		out->SetBlockMap();

		five::pgDictionary<five::phBound> newDrawables;

		if (phd->GetCount())
		{
			for (auto& bound : *phd)
			{
				newDrawables.Add(bound.first, convert<five::phBound*>(bound.second));
			}
		}

		out->SetFrom(&newDrawables);

		return out;
	}
}
