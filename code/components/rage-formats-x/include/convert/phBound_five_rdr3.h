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
	five::phBoundFlagEntry* inBoundFlags = bound->GetBoundFlags();

	std::vector<rdr3::phBoundFlagEntry> outBoundFlags(childCount);

	for (uint16_t i = 0; i < childCount; i++)
	{
		outBoundFlags[i].m_0 = inBoundFlags[i].m_0;
		outBoundFlags[i].pad = 0;
		outBoundFlags[i].m_4 = inBoundFlags[i].m_4;
		outBoundFlags[i].pad2 = 0;
	}

	out->SetBoundFlags(childCount, &outBoundFlags[0]);

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
			outPolys[i].poly.v1 = (inPolys[i].poly.v1 & 0x7FFF);
			outPolys[i].poly.v2 = (inPolys[i].poly.v2 & 0x7FFF);
			outPolys[i].poly.v3 = (inPolys[i].poly.v3 & 0x7FFF);
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
			outPolys[i].cylinder.index =  inPolys[i].cylinder.index;
			outPolys[i].cylinder.length = inPolys[i].cylinder.length;
			outPolys[i].cylinder.indexB = inPolys[i].cylinder.indexB;
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

static constexpr uint16_t GenerateMaterialIndex(const char* materialName)
{
	if (!materialName)
	{
		return 12667; // DEFAULT
	}

	uint32_t hash = 0;
	const char* next = materialName;

	for (char i = *materialName; i; i = *next)
	{
		++next;
		hash = i + 16 * hash;
		if ((hash & 0xF0000000) != 0)
		{
			hash ^= hash & 0xF0000000 ^ ((hash & 0xF0000000) >> 24);
		}
	}

	uint16_t index = (hash % 0x7FED + 18);

	return index;
}

static inline uint16_t ConvertMaterialIndexrdr3(uint8_t index)
{

#pragma region material conversion mappings
	static uint16_t conversionMap[]{
		GenerateMaterialIndex("DEFAULT"),
		GenerateMaterialIndex("CONCRETE"),
		GenerateMaterialIndex("CONCRETE"),
		GenerateMaterialIndex("CONCRETE"),
		GenerateMaterialIndex("TARMAC"),
		GenerateMaterialIndex("TARMAC"),
		GenerateMaterialIndex("TARMAC"),
		GenerateMaterialIndex("CONCRETE"),
		GenerateMaterialIndex("TARMAC"),
		GenerateMaterialIndex("ROCK"),
		GenerateMaterialIndex("ROCK_MOSSY"),
		GenerateMaterialIndex("STONE"),
		GenerateMaterialIndex("COBBLESTONE"),
		GenerateMaterialIndex("BRICK"),
		GenerateMaterialIndex("MARBLE"),
		GenerateMaterialIndex("CONCRETE_PAVEMENT"),
		GenerateMaterialIndex("SANDSTONE_SOLID"),
		GenerateMaterialIndex("SANDSTONE_BRITTLE"),
		GenerateMaterialIndex("SAND_COMPACT"),
		GenerateMaterialIndex("SAND_COMPACT"),
		GenerateMaterialIndex("SAND_WET"),
		GenerateMaterialIndex("SAND_COMPACT"),
		GenerateMaterialIndex("MUD_UNDERWATER"),
		GenerateMaterialIndex("SAND_DRY_DEEP"),
		GenerateMaterialIndex("SAND_WET_DEEP"),
		GenerateMaterialIndex("ICE"),
		GenerateMaterialIndex("ICE"),
		GenerateMaterialIndex("SNOW_COMPACT"),
		GenerateMaterialIndex("SNOW_COMPACT"),
		GenerateMaterialIndex("SNOW_DEEP"),
		GenerateMaterialIndex("SNOW_COMPACT"),
		GenerateMaterialIndex("GRAVEL_SMALL"),
		GenerateMaterialIndex("GRAVEL_LARGE"),
		GenerateMaterialIndex("GRAVEL_DEEP"),
		GenerateMaterialIndex("GRAVEL_LARGE"),
		GenerateMaterialIndex("DIRT_TRACK"),
		GenerateMaterialIndex("MUD_HARD"),
		GenerateMaterialIndex("MUD_POTHOLE"),
		GenerateMaterialIndex("MUD_SOFT"),
		GenerateMaterialIndex("MUD_SOFT"),
		GenerateMaterialIndex("MUD_DEEP"),
		GenerateMaterialIndex("MARSH"),
		GenerateMaterialIndex("MARSH_DEEP"),
		GenerateMaterialIndex("SOIL"),
		GenerateMaterialIndex("CLAY_HARD"),
		GenerateMaterialIndex("CLAY_SOFT"),
		GenerateMaterialIndex("GRASS_LONG"),
		GenerateMaterialIndex("GRASS"),
		GenerateMaterialIndex("GRASS_SHORT"),
		GenerateMaterialIndex("HAY"),
		GenerateMaterialIndex("BUSHES"),
		GenerateMaterialIndex("TWIGS"),
		GenerateMaterialIndex("LEAVES"),
		GenerateMaterialIndex("WOODCHIPS"),
		GenerateMaterialIndex("TREE_BARK"),
		GenerateMaterialIndex("METAL_SOLID_SMALL"),
		GenerateMaterialIndex("METAL_SOLID_MEDIUM"),
		GenerateMaterialIndex("METAL_SOLID_LARGE"),
		GenerateMaterialIndex("METAL_HOLLOW_SMALL"),
		GenerateMaterialIndex("METAL_HOLLOW_MEDIUM"),
		GenerateMaterialIndex("METAL_HOLLOW_LARGE"),
		GenerateMaterialIndex("METAL_CHAINLINK_SMALL"),
		GenerateMaterialIndex("METAL_CHAINLINK_SMALL"),
		GenerateMaterialIndex("METAL_CORRUGATED_IRON"),
		GenerateMaterialIndex("METAL_GRILLE"),
		GenerateMaterialIndex("METAL_RAILING"),
		GenerateMaterialIndex("METAL_DUCT"),
		GenerateMaterialIndex("CAR_METAL"),
		GenerateMaterialIndex("METAL_DUCT"),
		GenerateMaterialIndex("WOOD_SOLID_SMALL"),
		GenerateMaterialIndex("WOOD_SOLID_MEDIUM"),
		GenerateMaterialIndex("WOOD_SOLID_LARGE"),
		GenerateMaterialIndex("WOOD_SOLID_POLISHED"),
		GenerateMaterialIndex("WOOD_FLOOR_DUSTY"),
		GenerateMaterialIndex("WOOD_HOLLOW_SMALL"),
		GenerateMaterialIndex("WOOD_HOLLOW_MEDIUM"),
		GenerateMaterialIndex("WOOD_HOLLOW_LARGE"),
		GenerateMaterialIndex("WOOD_CHIPS"),
		GenerateMaterialIndex("WOOD_OLD_CREAKY"),
		GenerateMaterialIndex("WOOD_SOLID_LARGE"),
		GenerateMaterialIndex("WOOD_LATTICE"),
		GenerateMaterialIndex("CERAMIC"),
		GenerateMaterialIndex("ROOF_TILE"),
		GenerateMaterialIndex("ROOF_TILE"),
		GenerateMaterialIndex("GLASS_OPAQUE"),
		GenerateMaterialIndex("TARPAULIN"),
		GenerateMaterialIndex("PLASTIC_CLEAR"),
		GenerateMaterialIndex("PLASTIC_HOLLOW_CLEAR"),
		GenerateMaterialIndex("PLASTIC_HIGH_DENSITY_CLEAR"),
		GenerateMaterialIndex("PLASTIC_CLEAR"),
		GenerateMaterialIndex("PLASTIC_HOLLOW_CLEAR"),
		GenerateMaterialIndex("PLASTIC_HIGH_DENSITY_CLEAR"),
		GenerateMaterialIndex("GLASS_OPAQUE"),
		GenerateMaterialIndex("RUBBER"),
		GenerateMaterialIndex("RUBBER"),
		GenerateMaterialIndex("CURTAINS"),
		GenerateMaterialIndex("RUBBER"),
		GenerateMaterialIndex("CARPET_SOLID"),
		GenerateMaterialIndex("CARPET_SOLID_DUSTY"),
		GenerateMaterialIndex("CARPET_SOLID_DUSTY"),
		GenerateMaterialIndex("CLOTH"),
		GenerateMaterialIndex("PLASTER_SOLID"),
		GenerateMaterialIndex("PLASTER_BRITTLE"),
		GenerateMaterialIndex("PAPER"),
		GenerateMaterialIndex("PAPER"),
		GenerateMaterialIndex("PAPER"),
		GenerateMaterialIndex("WATER"),
		GenerateMaterialIndex("FEATHER_PILLOW"),
		GenerateMaterialIndex("CURTAINS"),
		GenerateMaterialIndex("LEATHER"),
		GenerateMaterialIndex("GLASS_OPAQUE"),
		GenerateMaterialIndex("SLATTED_BLINDS"),
		GenerateMaterialIndex("GLASS_SHOOT_THROUGH"),
		GenerateMaterialIndex("GLASS_BULLETPROOF"),
		GenerateMaterialIndex("GLASS_OPAQUE"),
		GenerateMaterialIndex("CONCRTAR_PAPERETE"),
		GenerateMaterialIndex("CAR_METAL"),
		GenerateMaterialIndex("SLATTED_BLINDS"),
		GenerateMaterialIndex("CAR_SOFTTOP"),
		GenerateMaterialIndex("CAR_SOFTTOP_CLEAR"),
		GenerateMaterialIndex("CAR_GLASS_WEAK"),
		GenerateMaterialIndex("CAR_GLASS_MEDIUM"),
		GenerateMaterialIndex("CAR_GLASS_STRONG"),
		GenerateMaterialIndex("CAR_GLASS_BULLETPROOF"),
		GenerateMaterialIndex("CAR_GLASS_OPAQUE"),
		GenerateMaterialIndex("WATER"),
		GenerateMaterialIndex("BLOOD"),
		GenerateMaterialIndex("OIL"),
		GenerateMaterialIndex("PETROL"),
		GenerateMaterialIndex("FRESH_MEAT"),
		GenerateMaterialIndex("DRIED_MEAT"),
		GenerateMaterialIndex("EMISSIVE_GLASS"),
		GenerateMaterialIndex("EMISSIVE_PLASTIC"),
		GenerateMaterialIndex("VFX_METAL_ELECTRIFIED"),
		GenerateMaterialIndex("VFX_METAL_WATER_TOWER"),
		GenerateMaterialIndex("VFX_METAL_STEAM"),
		GenerateMaterialIndex("VFX_METAL_FLAME"),
		GenerateMaterialIndex("PHYS_NO_FRICTION"),
		GenerateMaterialIndex("PHYS_NO_FRICTION"),
		GenerateMaterialIndex("PHYS_NO_FRICTION"),
		GenerateMaterialIndex("PHYS_CASTER"),
		GenerateMaterialIndex("PHYS_CASTER"),
		GenerateMaterialIndex("PHYS_CAR_VOID"),
		GenerateMaterialIndex("PHYS_PED_CAPSULE"),
		GenerateMaterialIndex("PHYS_MACHINERY"),
		GenerateMaterialIndex("PHYS_MACHINERY"),
		GenerateMaterialIndex("PHYS_BARBED_WIRE"),
		GenerateMaterialIndex("PHYS_NO_FRICTION"),
		GenerateMaterialIndex("PHYS_NO_FRICTION"),
		GenerateMaterialIndex("PHYS_NO_FRICTION"),
		GenerateMaterialIndex("PHYS_DYNAMIC_COVER_BOUND"),
		GenerateMaterialIndex("THIGH_LEFT"),
		GenerateMaterialIndex("SHIN_LEFT"),
		GenerateMaterialIndex("FOOT_LEFT"),
		GenerateMaterialIndex("THIGH_RIGHT"),
		GenerateMaterialIndex("SHIN_RIGHT"),
		GenerateMaterialIndex("FOOT_RIGHT"),
		GenerateMaterialIndex("SPINE0"),
		GenerateMaterialIndex("SPINE1"),
		GenerateMaterialIndex("SPINE2"),
		GenerateMaterialIndex("SPINE3"),
		GenerateMaterialIndex("CLAVICLE_LEFT"),
		GenerateMaterialIndex("UPPER_ARM_LEFT"),
		GenerateMaterialIndex("LOWER_ARM_LEFT"),
		GenerateMaterialIndex("HAND_LEFT"),
		GenerateMaterialIndex("CLAVICLE_RIGHT"),
		GenerateMaterialIndex("UPPER_ARM_RIGHT"),
		GenerateMaterialIndex("LOWER_ARM_RIGHT"),
		GenerateMaterialIndex("CONCRHAND_RIGHTETE"),
		GenerateMaterialIndex("NECK"),
		GenerateMaterialIndex("HEAD"),
		GenerateMaterialIndex("ANIMAL_DEFAULT"),
		GenerateMaterialIndex("CAR_ENGINE"),
		GenerateMaterialIndex("PUDDLE"),
		GenerateMaterialIndex("CONCRETE_PAVEMENT"),
		GenerateMaterialIndex("BRICK_PAVEMENT"),
		GenerateMaterialIndex("PHYS_DYNAMIC_COVER_BOUND"),
		GenerateMaterialIndex("VFX_WOOD_BEER_BARREL"),
		GenerateMaterialIndex("WOOD_LATTICE"),
		GenerateMaterialIndex("PROP_ROCK"),
		GenerateMaterialIndex("BUSHES"),
		GenerateMaterialIndex("METAL_SOLID_MEDIUM"),
		GenerateMaterialIndex("DEFAULT")
	};
#pragma endregion

	return conversionMap[index];
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

		materials[i].mat2.stairs = inMaterials[i].mat1.stairs;
		materials[i].mat2.blockClimb = inMaterials[i].mat1.blockClimb;
		materials[i].mat2.seeThrough = inMaterials[i].mat1.seeThrough;
		materials[i].mat2.shootThrough = inMaterials[i].mat1.shootThrough;
		materials[i].mat2.notCover = inMaterials[i].mat1.notCover;
		materials[i].mat2.walkablePath = inMaterials[i].mat1.walkablePath;
		materials[i].mat2.noCamCollision = inMaterials[i].mat1.noCamCollision;
		materials[i].mat2.shootThroughFx = inMaterials[i].mat1.shootThroughFx;

		materials[i].mat2.noDecal = inMaterials[i].mat2.noDecal;
		materials[i].mat2.noNavmesh = inMaterials[i].mat2.noNavmesh;
		materials[i].mat2.noRagdoll = inMaterials[i].mat2.noRagdoll;
		materials[i].mat2.vehicleWheel = inMaterials[i].mat2.vehicleWheel;
		materials[i].mat2.noPtfx = inMaterials[i].mat2.noPtfx;
		materials[i].mat2.tooSteepForPlayer = inMaterials[i].mat2.tooSteepForPlayer;
		materials[i].mat2.noNetworkSpawn = inMaterials[i].mat2.noNetworkSpawn;
		materials[i].mat2.noCamCollisionAllowClipping = inMaterials[i].mat2.noCamCollisionAllowClipping;
		materials[i].mat2.unknown = inMaterials[i].mat2.unknown;

	}

	out->SetMaterials(materials.size(), &materials[0]);
}

static inline void fillBoundPolyMaterial(rdr3::phBound* out, five::phBound* in)
{
	five::phBoundMaterial inMaterial = in->GetMaterial();
	rdr3::phBoundMaterial outMaterial = { 0 };

	outMaterial.mat1.materialIdx = ConvertMaterialIndexrdr3(inMaterial.mat1.materialIdx);
	outMaterial.mat1.roomId = inMaterial.mat1.roomId;

	outMaterial.mat2.stairs = inMaterial.mat1.stairs;
	outMaterial.mat2.blockClimb = inMaterial.mat1.blockClimb;
	outMaterial.mat2.seeThrough = inMaterial.mat1.seeThrough;
	outMaterial.mat2.shootThrough = inMaterial.mat1.shootThrough;
	outMaterial.mat2.notCover = inMaterial.mat1.notCover;
	outMaterial.mat2.walkablePath = inMaterial.mat1.walkablePath;
	outMaterial.mat2.noCamCollision = inMaterial.mat1.noCamCollision;
	outMaterial.mat2.shootThroughFx = inMaterial.mat1.shootThroughFx;

	outMaterial.mat2.noDecal = inMaterial.mat2.noDecal;
	outMaterial.mat2.noNavmesh = inMaterial.mat2.noNavmesh;
	outMaterial.mat2.noRagdoll = inMaterial.mat2.noRagdoll;
	outMaterial.mat2.vehicleWheel = inMaterial.mat2.vehicleWheel;
	outMaterial.mat2.noPtfx = inMaterial.mat2.noPtfx;
	outMaterial.mat2.tooSteepForPlayer = inMaterial.mat2.tooSteepForPlayer;
	outMaterial.mat2.noNetworkSpawn = inMaterial.mat2.noNetworkSpawn;
	outMaterial.mat2.noCamCollisionAllowClipping = inMaterial.mat2.noCamCollisionAllowClipping;
	outMaterial.mat2.unknown = inMaterial.mat2.unknown;

	out->SetMaterial(outMaterial);
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
	fillBoundPolyMaterial(out, bound);

	return out;
}

template<>
rdr3::phBoundBox* convert(five::phBoundBox* bound)
{
	auto out = new (false) rdr3::phBoundBox;

	fillBaseBound(out, bound);
	fillBoundPolyMaterial(out, bound);

	return out;
}

template<>
rdr3::phBoundCapsule* convert(five::phBoundCapsule* bound)
{
	auto out = new (false) rdr3::phBoundCapsule;

	fillBaseBound(out, bound);
	fillBoundPolyMaterial(out, bound);

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
