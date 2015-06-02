#include "StdInc.h"
#include <gtest/gtest.h>

#include <pgBase.h>
#include <rmcDrawable.h>
#include <gtaDrawable.h>
#include <phBound.h>
//#include <fragType.h>

#include <ShaderInfo.h>

using namespace rage::five;

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

void ConvertDrawable(const wchar_t* from);

//#include <d3dcompiler.h>
//#pragma comment(lib, "d3dcompiler.lib")

void LoadBoundFour();

struct PolyEdge
{
	uint32_t edges[3];
};

struct PolyEdgeMap
{
	uint32_t left;
	uint32_t right;
};

#include <shellapi.h>

#pragma comment(lib, "shell32.lib")

int main(int argc, char **argv)
{
	//::testing::InitGoogleTest(&argc, argv);
	//return RUN_ALL_TESTS();

	/*auto blockMap = pgStreamManager::BeginPacking();

	auto dr = new(false) rmcDrawableBase();

	auto sg = new(false) grmShaderGroup();
	
	pgStreamManager::EndPacking();*/

	//auto shaderFile = fxc::ShaderFile::Load("Y:\\common\\shaders\\win32_40_final\\normal_spec_pxm_tnt.fxc");
	/*auto shaderFile = fxc::ShaderFile::Load("Y:\\common\\shaders\\win32_40_final\\postfx.fxc");

	_mkdir("Y:\\dev\\v\\sh");

	for (auto& ps : shaderFile->GetPixelShaders())
	{
		auto& data = ps.second->GetShaderData();

		if (data.size() > 0)
		{
			ID3D10Blob* blob = nullptr;
			HRESULT hr = D3DDisassemble(&data[0], data.size(), D3D_DISASM_ENABLE_COLOR_CODE, nullptr, &blob);

			if (blob)
			{
				FILE* f = fopen(va("Y:\\dev\\v\\sh\\exp\\%s.html", ps.first.c_str()), "wb");
				fwrite(blob->GetBufferPointer(), blob->GetBufferSize(), 1, f);
				fclose(f);

				blob->Release();
			}
		}
	}

	exit(0);*/

	//LoadBoundFour();
	//return 0;

#if 0
	ValidateSizePh<phBound, 112>();
	ValidateSizePh<phBoundComposite, 176>();
	ValidateSizePh<phBoundPolyhedron, 240>();
	ValidateSizePh<phBoundGeometry, 304>();
	ValidateSizePh<phBoundBVH, 336>();
	ValidateSizePh<phBVH, 128>();

	char* buffers2 = new char[1089536];
	//FILE* f2 = fopen("Y:/dev/ydr/bh1_07_0.ybn.seg", "rb");
	FILE* f2 = fopen("X:/dev/bh1_07_0.ybn.seg", "rb");
	size_t len = fread(buffers2, 1, 1089536, f2);
	fclose(f2);

	BlockMap bm2;
	bm2.virtualLen = 1;
	bm2.physicalLen = 0;
	bm2.baseAllocationSize[0] = 0x2000;
	bm2.baseAllocationSize[1] = 0x2000;
	bm2.blocks[0].data = buffers2;
	bm2.blocks[0].offset = 0;
	bm2.blocks[0].size = len;

	pgStreamManager::SetBlockInfo(&bm2);
	phBoundComposite* bound = (phBoundComposite*)buffers2;
	bound->Resolve(&bm2);

	for (uint16_t idx = 0; idx < bound->GetNumChildBounds(); idx++)
	{
		phBoundBVH* childBound = static_cast<phBoundBVH*>(bound->GetChildBound(idx));
		childBound->Resolve(&bm2);
	}
#endif

	wchar_t** wargv = CommandLineToArgvW(GetCommandLine(), &argc);

	if (argc == 3)
	{
		if (_wcsicmp(wargv[1], L"convert") == 0)
		{
			ConvertDrawable(wargv[2]);
		}
	}
	return 0;

	char* buffer = new char[2089536];
	FILE* f = fopen("Y:/common/lovely.ydr.seg", "rb");
	//FILE* f = fopen("Y:/dev/ydr/dt1_07_building2.ydr.seg", "rb");
	fread(buffer, 1, 2089536, f);
	fclose(f);

	char* buffers = new char[1089536];
	//FILE* f = fopen("Y:/common/lovely.ydr.seg", "rb");
	f = fopen("Y:/dev/ydr/dt1_07_building2.ydr.seg", "rb");
	fread(buffers, 1, 1089536, f);
	fclose(f);


	BlockMap bm;
	bm.virtualLen = 1;
	bm.physicalLen = 0;
	bm.blocks[0].data = buffer;
	bm.blocks[0].offset = 0;
	bm.blocks[0].size = 1089536;

	pgStreamManager::SetBlockInfo(&bm);
	gtaDrawable* ddrawable = (gtaDrawable*)buffer;
	ddrawable->Resolve(&bm);

	bm.virtualLen = 1;
	bm.physicalLen = 0;
	bm.blocks[0].data = buffers;
	bm.blocks[0].offset = 0;
	bm.blocks[0].size = 1089536;

	pgStreamManager::SetBlockInfo(&bm);
	gtaDrawable* ddrawable2 = (gtaDrawable*)buffers;
	ddrawable2->Resolve(&bm);

	return 0;

	rage::five::ValidateSize<rage::five::grmShaderGroup, 64>();

	auto blockMap = pgStreamManager::BeginPacking();

	auto drawable = new(false) gtaDrawable();

	auto shaderGroup = new(false) grmShaderGroup();

	auto shaderFile = fxc::ShaderFile::Load("Y:\\common\\shaders\\win32_40_final\\default.fxc");

	//
	grmShaderFx* shader = new(false) grmShaderFx();
	shader->SetSpsName("default.sps");
	shader->SetShaderName("default");
	shader->SetDrawBucket(0);

	auto& localParameters = shaderFile->GetLocalParameters();

	std::vector<grmShaderParameterMeta> parameters(localParameters.size());
	std::vector<uint32_t> parameterNames(localParameters.size());
	std::vector<std::vector<uint8_t>> parameterValues(localParameters.size());

	// count the samplers in the parameter list
	auto numSamplers = std::count_if(localParameters.begin(), localParameters.end(), [] (const decltype(*localParameters.begin())& value) // weird workaround to the lack of auto arguments in VS12
	{
		return value.second->IsSampler();
	});

	int samplerIdx = 0;
	int nonSamplerIdx = numSamplers;

	for (auto& pair : localParameters)
	{
		std::shared_ptr<fxc::ShaderParameter> parameter = pair.second;

		int idx = (parameter->IsSampler()) ? samplerIdx++ : nonSamplerIdx++;

		parameterNames[idx] = parameter->GetNameHash();

		if (!parameter->IsSampler())
		{
			parameterValues[idx] = parameter->GetDefaultValue();
			parameters[idx].registerIdx = shaderFile->MapRegister(parameter->GetRegister());
		}
		else
		{
			// TODO: set as sampler reference
			parameters[idx].registerIdx = parameter->GetRegister();
		}

		parameters[idx].isSampler = parameter->IsSampler();
	}

	shader->SetParameters(parameters, parameterNames, parameterValues);

	pgPtr<grmShaderFx> shaders[1];
	shaders[0] = shader;

	shaderGroup->SetShaders(1, shaders);
	drawable->SetShaderGroup(shaderGroup);



	auto& lodGroup = drawable->GetLodGroup();
	lodGroup.SetDrawBucketMask(0, 0xFF01);
	lodGroup.SetBounds(Vector3(-66.6f, -66.6f, -10.0f), Vector3(66.6f, 66.6f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), 94.8f);

	// model
	GeometryBound bounds;
	bounds.aabbMin = Vector4(-1.0f, 0.0f, -1.0f, -94.8f);
	bounds.aabbMax = Vector4(1.0f, 0.0f, 1.0f, 94.8f);

	grmModel* model = new(false) grmModel();
	model->SetGeometryBounds(1, &bounds);

	uint16_t shaderMappings[1] = {
		0
	};

	model->SetShaderMappings(1, shaderMappings);

	// file buffers
	f = fopen("X:/temp/indbuf.bin", "rb");
	static uint16_t indices[40000];
	fread(indices, 1, sizeof(indices), f);
	fclose(f);

	f = fopen("X:/temp/vertbuf.bin", "rb");
	static char vertices[160000];
	fread(vertices, 1, sizeof(vertices), f);
	fclose(f);

	int maxVerts = 4436 / 4;
	int maxIndices = 0;

	for (int i = 0; i < 6597; i += 3)
	{
		if (indices[i] >= maxVerts || indices[i + 1] >= maxVerts || indices[i + 2] >= maxVerts)
		{
			maxIndices = i;
			break;
		}
	}

	// index buffer
	grcIndexBufferD3D* indexBuffer = new(false) grcIndexBufferD3D(maxIndices, indices);

	// vertex format
	grcVertexFormat* vertexFormat = new(false) grcVertexFormat(89, 36, 4, 0x6755555555996996);

	// vertex buffer
	grcVertexBufferD3D* vertexBuffer = new(false) grcVertexBufferD3D();
	vertexBuffer->SetVertexFormat(vertexFormat);
	vertexBuffer->SetVertices(maxVerts, 36, vertices);

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

	drawable->SetPrimaryModel();
	drawable->SetName("lovely.#dr");

	pgStreamManager::EndPacking();

	f = fopen("Y:\\common\\lovely.ydr", "wb");

	blockMap->Save(165, [&] (const void* d, size_t s)
	{
		fwrite(d, 1, s, f);
	});

	fclose(f);

	/*char* buffer2 = new char[4784128];
	f = fopen("Y:/dls/liefzt/sultanrs.wft.gfx", "rb");
	fread(buffer2, 1, 4784128, f);
	fclose(f);*/

	__debugbreak();

	/*bm.blocks[1].data = buffer2;
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

	return 0;*/

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