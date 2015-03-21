#include "StdInc.h"
#include <gtest/gtest.h>

#include <pgBase.h>
#include <rmcDrawable.h>
#include <gtaDrawable.h>
#include <fragType.h>

using namespace rage::ny;

void FatalError(const char* a, ...)
{
	__debugbreak();
}

class testClass : public pgBase
{
private:
	char cake[32];

public:
	pgPtr<testClass> tc1;
	pgPtr<testClass> tc2;

public:
	testClass()
	{
		memset(cake, 0x32, sizeof(cake));
	}
};

Vector4* MakeVector4(float x, float y, float z, float w)
{
	Vector4* ret = (Vector4*)pgStreamManager::Allocate(sizeof(Vector4), false, nullptr);

	ret->x = x;
	ret->y = y;
	ret->z = z;
	ret->w = w;

	return ret;
}

void* MakeMatrix(float a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, float a9, float a10, float a11, float a12, float a13, float a14, float a15, float a16)
{
	float* ret = (float*)pgStreamManager::Allocate(sizeof(float) * 16, false, nullptr);

	ret[0] = a1;
	ret[1] = a2;
	ret[2] = a3;
	ret[3] = a4;
	ret[4] = a5;
	ret[5] = a6;
	ret[6] = a7;
	ret[7] = a8;
	ret[8] = a9;
	ret[9] = a10;
	ret[10] = a11;
	ret[11] = a12;
	ret[12] = a13;
	ret[13] = a14;
	ret[14] = a15;
	ret[15] = a16;

	return ret;
}

int main(int argc, char **argv)
{
	//::testing::InitGoogleTest(&argc, argv);
	//return RUN_ALL_TESTS();

	/*auto blockMap = pgStreamManager::BeginPacking();

	auto dr = new(false) rmcDrawableBase();

	auto sg = new(false) grmShaderGroup();
	
	pgStreamManager::EndPacking();*/

	char* buffer = new char[163840];
	FILE* f = fopen("Y:/dls/liefzt/sultanrs.wft.sys", "rb");
	fread(buffer, 1, 163840, f);
	fclose(f);

	char* buffer2 = new char[4784128];
	f = fopen("Y:/dls/liefzt/sultanrs.wft.gfx", "rb");
	fread(buffer2, 1, 4784128, f);
	fclose(f);

	BlockMap bm;
	bm.virtualLen = 1;
	bm.physicalLen = 1;
	bm.blocks[0].data = buffer;
	bm.blocks[0].offset = 0;
	bm.blocks[0].size = 163840;
	bm.blocks[1].data = buffer2;
	bm.blocks[1].offset = 0;
	bm.blocks[1].size = 4784128;

	pgPtrRepresentation ptr;
	ptr.blockType = 5;
	ptr.pointer = 0;

	pgStreamManager::SetBlockInfo(&bm);
	pgStreamManager::BeginPacking(&bm);

	fragType* frag = (fragType*)pgStreamManager::ResolveFilePointer(ptr, &bm);
	frag->Resolve();

	auto processDrawable = [] (rmcDrawable* drawable)
	{
		if (!drawable)
		{
			return;
		}

		auto processModel = [] (grmModel* model)
		{
			auto& geometries = model->GetGeometries();

			for (int i = 0; i < geometries.GetCount(); i++)
			{
				grmGeometryQB* geom = geometries.Get(i);

				geom->FixUpBrokenVertexCounts();
			}
		};

		auto& lodGroup = drawable->GetLodGroup();

		for (int i = 0; i < 3; i++)
		{
			grmModel* model = lodGroup.GetModel(i);

			if (model)
			{
				processModel(model);
			}
		}
	};

	processDrawable(frag->GetDrawable());

	for (int i = 0; i < frag->GetNumChildren(); i++)
	{
		processDrawable(frag->GetChild(i)->GetDrawable());
		processDrawable(frag->GetChild(i)->GetDrawable2());
	}

	pgStreamManager::EndPacking();

	f = fopen("T:\\mydocs\\rockstar games\\tlad\\avvy.wft", "wb");

	bm.Save(112, [&] (const void* d, size_t s)
	{
		fwrite(d, 1, s, f);
	});

	fclose(f);

	return 0;

	/*auto blockMap = pgStreamManager::BeginPacking();

	auto drawable = new(false) gtaDrawable();

	auto shaderGroup = new(false) grmShaderGroup();

	//
	uint32_t eightyNine = 89;

	shaderGroup->SetVertexFormats(1, &eightyNine);

	//
	uint32_t one = 1;

	shaderGroup->SetShaderIndices(1, &one);

	//
	grmShaderFx* shader = new(false) grmShaderFx();
	shader->SetPreset(0);
	shader->SetSpsName("gta_alpha.sps");
	shader->SetShaderName("gta_default");
	shader->SetDrawBucket(1);
	shader->SetUsageCount(1);
	shader->SetVersion(2);
	shader->SetIndex(1);
	
	uint32_t parameterNames[6] = {
		3617324062,
		3126116752,
		726757629,
		4230725802,
		4034105764,
		2215984599
	};

	grmShaderEffectParamType parameterTypes[6] = {
		grmShaderEffectParamType::Vector4,
		grmShaderEffectParamType::Vector4,
		grmShaderEffectParamType::Texture,
		grmShaderEffectParamType::Vector4,
		grmShaderEffectParamType::Matrix,
		grmShaderEffectParamType::Vector4
	};

	void* parameterValues[6];

	grcTexturePC* texture = new(false) grcTexturePC();
	texture->SetName("waterclear");

	parameterValues[0] = MakeVector4(1.0f, 0.0f, 0.0f, 0.0f);
	parameterValues[1] = MakeVector4(0.0f, 1.0f, 0.0f, 0.0f);
	parameterValues[2] = texture;
	parameterValues[3] = MakeVector4(1280.0f, 0.0f, 0.0f, 0.0f);
	parameterValues[4] = MakeMatrix(-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	parameterValues[5] = MakeVector4(0.2125f, 0.7154f, 0.0721f, 0.0f);

	auto& effect = shader->GetEffect();
	effect.SetShaderHash(0x7B2B443A);
	effect.SetParameterDataSize(208);
	effect.SetParameters(6, parameterNames, parameterValues, parameterTypes);

	pgPtr<grmShaderFx> shaders[1];
	shaders[0] = shader;

	shaderGroup->SetShaders(1, shaders);

	drawable->SetShaderGroup(shaderGroup);

	//
	auto& lodGroup = drawable->GetLodGroup();
	lodGroup.SetDrawBucketMask(0, 2);
	lodGroup.SetBounds(Vector3(-1.0f, 0.0f, -1.0f), Vector3(1.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), 1.41421f);

	// model
	grmModel* model = new(false) grmModel();
	model->SetGeometryBounds(Vector4(0.0f, 0.0f, 0.0f, 1.41421f));

	uint16_t shaderMappings[1] = {
		0
	};

	model->SetShaderMappings(1, shaderMappings);

	// file buffers
	FILE* f = fopen("X:/temp/indbuf.bin", "rb");
	static uint16_t indices[512];
	fread(indices, 1, sizeof(indices), f);
	fclose(f);

	f = fopen("X:/temp/vertbuf.bin", "rb");
	static char vertices[512];
	fread(vertices, 1, sizeof(vertices), f);
	fclose(f);

	// index buffer
	grcIndexBufferD3D* indexBuffer = new(false) grcIndexBufferD3D(24, indices);

	// vertex format
	grcVertexFormat* vertexFormat = new(false) grcVertexFormat(89, 36, 4, 0x6755555555996996);

	// vertex buffer
	grcVertexBufferD3D* vertexBuffer = new(false) grcVertexBufferD3D();
	vertexBuffer->SetVertexFormat(vertexFormat);
	vertexBuffer->SetVertices(9, 36, vertices);

	// geometry
	grmGeometryQB* geometry = new(false) grmGeometryQB();
	geometry->SetIndexBuffer(indexBuffer);
	geometry->SetVertexBuffer(vertexBuffer);

	grmGeometryQB* geometries[] = {
		geometry
	};

	model->SetGeometries(1, geometries);

	// set lod group geom
	lodGroup.SetModel(0, model);

	pgStreamManager::EndPacking();

	f = fopen("T:\\mydocs\\rockstar games\\tlad\\blah.wdr", "wb");

	blockMap->Save(110, [&] (const void* d, size_t s)
	{
		fwrite(d, 1, s, f);
	});

	fclose(f);*/
}