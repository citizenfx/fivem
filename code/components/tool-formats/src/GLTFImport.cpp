#include <StdInc.h>

// TODOs:
// - texture compression and repacking R/B channels
// - draco compression enablement
// - animations
// - skeletons
// - material improvements for PBR
//   - spec conversion
// - RAGE materials
// - #map/#typ exporting (text at first)

#ifdef GTA_FIVE
#include <d3d9.h>

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <gtaDrawable.h>
#include <phBound.h>
#include <fragType.h>

#include "ToolComponentHelpers.h"

#include <json.hpp>
#include <tiny_gltf.h>

#include <boost/filesystem.hpp>
#include <DirectXMath.h>

#include <ShaderInfo.h>

static DirectX::XMFLOAT3 ConvertTranslation(const std::vector<double>& pos)
{
	if (pos.empty())
	{
		return {
			0.0f, 0.0f, 0.0f
		};
	}

	return {
		-static_cast<float>(pos[0]),
		static_cast<float>(pos[2]),
		static_cast<float>(pos[1])
	};
}

static DirectX::XMFLOAT4 ConvertQuaternion(const std::vector<double>& quat)
{
	if (quat.empty())
	{
		return {
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}

	return {
		-static_cast<float>(quat[0]),
		static_cast<float>(quat[2]), 
		static_cast<float>(quat[1]), 
		static_cast<float>(quat[3])
	};
}

static DirectX::XMFLOAT3 ConvertScale(const std::vector<double>& scale)
{
	if (scale.empty())
	{
		return {
			1.0f, 1.0f, 1.0f
		};
	}

	return {
		static_cast<float>(scale[0]),
		static_cast<float>(scale[2]),
		static_cast<float>(scale[1])
	};
}

static DirectX::XMMATRIX ConvertNodeMatrix(const tinygltf::Node& node)
{
	auto translation = ConvertTranslation(node.translation);
	auto rotation = ConvertQuaternion(node.rotation);
	auto scale = ConvertScale(node.scale);

	if (!node.matrix.empty())
	{
		DirectX::XMFLOAT4X4 mat;
		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				mat.m[x][y] = static_cast<float>(node.matrix[(x * 4) + y]);
			}
		}

		// decompose and run conversion ops
		DirectX::XMVECTOR scaleVec;
		DirectX::XMVECTOR rotationVec;
		DirectX::XMVECTOR translationVec;
		DirectX::XMMatrixDecompose(&scaleVec, &rotationVec, &translationVec, DirectX::XMLoadFloat4x4(&mat));

		std::vector<double> scaleD(3);
		std::vector<double> rotationD(4);
		std::vector<double> translationD(3);

		scaleD[0] = DirectX::XMVectorGetX(scaleVec);
		scaleD[1] = DirectX::XMVectorGetY(scaleVec);
		scaleD[2] = DirectX::XMVectorGetZ(scaleVec);

		rotationD[0] = DirectX::XMVectorGetX(rotationVec);
		rotationD[1] = DirectX::XMVectorGetY(rotationVec);
		rotationD[2] = DirectX::XMVectorGetZ(rotationVec);
		rotationD[3] = DirectX::XMVectorGetW(rotationVec);

		translationD[0] = DirectX::XMVectorGetX(translationVec);
		translationD[1] = DirectX::XMVectorGetY(translationVec);
		translationD[2] = DirectX::XMVectorGetZ(translationVec);

		translation = ConvertTranslation(translationD);
		rotation = ConvertQuaternion(rotationD);
		scale = ConvertScale(scaleD);
	}

	return DirectX::XMMatrixAffineTransformation(DirectX::XMLoadFloat3(&scale), DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), DirectX::XMLoadFloat4(&rotation), DirectX::XMLoadFloat3(&translation));
}

template<typename T>
static bool OutputFile(T&& callback, int fileVersion, const std::wstring& fileName)
{
	auto bm = rage::five::pgStreamManager::BeginPacking();

	callback();

	rage::five::pgStreamManager::EndPacking();

	FILE* f = _wfopen(fileName.c_str(), L"wb");

	if (!f)
	{
		printf("... couldn't open output file for writing.\n");
		return false;
	}

	size_t outputSize = 0;

	bm->Save(fileVersion, [&](const void* d, size_t s)
	{
		fwrite(d, 1, s, f);

		outputSize += s;
	});

	wprintf(L"written %s successfully - compressed size %d\n", boost::filesystem::path(fileName).filename().c_str(), outputSize);

	fclose(f);

	return true;
}

enum class grcDataSize
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
	Dec3N,
	UShort4 = 15
};

static rage::five::grmGeometryQB* ConvertGeometry(const tinygltf::Model& model, const tinygltf::Primitive& geometry, const DirectX::XMMATRIX& scaleMatrix, int boneIdx = -1)
{
	DirectX::XMVECTOR scale;
	DirectX::XMVECTOR rotQuat;
	DirectX::XMVECTOR trans;
	DirectX::XMMatrixDecompose(&scale, &rotQuat, &trans, scaleMatrix);
	auto rotMatrix = DirectX::XMMatrixAffineTransformation(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMVectorZero(), rotQuat, DirectX::XMVectorZero());

	rage::five::grmGeometryQB* gameGeom = new (false) rage::five::grmGeometryQB();
	
	// make index buffer
	{
		const tinygltf::Accessor& accessor = model.accessors[geometry.indices];
		uint32_t numIndices = accessor.count;

		const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

		std::vector<uint16_t> indices(numIndices);
		if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
		{
			memcpy(&indices[0], (uint16_t*)&buffer.data[bufferView.byteOffset], numIndices * 2);
		}
		else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
		{
			uint8_t* start = (uint8_t*)&buffer.data[bufferView.byteOffset];

			for (int i = 0; i < numIndices; i++)
			{
				indices[i] = start[i];
			}
		}
		
		// flip?
		/*for (int i = 0; i < numIndices; i += 3)
		{
			std::swap(indices[i + 1], indices[i + 2]);
		}*/

		rage::five::grcIndexBufferD3D* indexBuffer = new (false) rage::five::grcIndexBufferD3D(numIndices, &indices[0]);
		gameGeom->SetIndexBuffer(indexBuffer);
	}

	{
		rage::five::grcVertexBufferD3D* vertexBuffer = new (false) rage::five::grcVertexBufferD3D();

		// build a fitting (interleaved) FVF
		uint64_t dataSizes = 0;
		uint32_t bits = 0;
		uint32_t vertexSize = 0;
		uint8_t numFields = 0;

		size_t count = 0;

		static const std::map<std::string, int> attributeRefs = {
			{ "POSITION", 0	},
			{ "WEIGHTS_0", 1 },
			{ "JOINTS_0", 2 },
			{ "NORMAL", 3 },
			{ "COLOR_0", 4 },
			{ "TEXCOORD_0", 6 },
			{ "TEXCOORD_1", 7 },
			{ "TANGENT", 14 },
		};

		static const std::map<std::tuple<int, int>, grcDataSize> typeRefs = { 
			{ { TINYGLTF_TYPE_VEC4, TINYGLTF_COMPONENT_TYPE_BYTE }, grcDataSize::UByte4 },
			{ { TINYGLTF_TYPE_VEC4, TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE }, grcDataSize::UByte4 },
			{ { TINYGLTF_TYPE_VEC4, TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT }, grcDataSize::UShort4 },
			{ { TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_FLOAT }, grcDataSize::Float },
			{ { TINYGLTF_TYPE_VEC2, TINYGLTF_COMPONENT_TYPE_FLOAT }, grcDataSize::Float2 },
			{ { TINYGLTF_TYPE_VEC3, TINYGLTF_COMPONENT_TYPE_FLOAT }, grcDataSize::Float3 },
			{ { TINYGLTF_TYPE_VEC4, TINYGLTF_COMPONENT_TYPE_FLOAT }, grcDataSize::Float4 },
		};

		auto increaseSize = [](grcDataSize dataSize, uint32_t& vertexSize)
		{
			switch (dataSize)
			{
				case grcDataSize::Float16_2:
					vertexSize += 4;
					break;
				case grcDataSize::Float:
					vertexSize += 4;
					break;
				case grcDataSize::Float16_4:
					vertexSize += 8;
					break;
				case grcDataSize::Float_unk:
					vertexSize += 4;
					break;
				case grcDataSize::Float2:
					vertexSize += 8;
					break;
				case grcDataSize::Float3:
					vertexSize += 12;
					break;
				case grcDataSize::Float4:
					vertexSize += 16;
					break;
				case grcDataSize::UByte4:
					vertexSize += 4;
					break;
				case grcDataSize::Color:
					vertexSize += 4;
					break;
				case grcDataSize::Dec3N:
					vertexSize += 4;
					break;
			}
		};

		const tinygltf::Accessor* accessors[16] = { 0 };

		for (const auto& entry : geometry.attributes)
		{
			const tinygltf::Accessor& accessor = model.accessors[entry.second];
			
			auto attrRefIt = attributeRefs.find(entry.first);

			if (attrRefIt == attributeRefs.end())
			{
				continue;
			}

			auto typeRefIt = typeRefs.find({ accessor.type, accessor.componentType });

			if (typeRefIt == typeRefs.end())
			{
				continue;
			}

			int fvfIndex = attrRefIt->second;
			accessors[fvfIndex] = &accessor;

			grcDataSize dataSize = typeRefIt->second;
			bits |= (1 << fvfIndex);
			dataSizes |= uint64_t(dataSize) << uint64_t(fvfIndex * 4);

			increaseSize(dataSize, vertexSize);

			count = accessor.count;
			numFields++;
		}

		// no color?
		if (!(bits & (1 << 4)))
		{
			bits |= (1 << 4);
			dataSizes |= uint64_t(grcDataSize::Color) << uint64_t(4 * 4);
			vertexSize += 4;
		}

		// no texcoord0?
		if (!(bits & (1 << 6)))
		{
			bits |= (1 << 6);
			dataSizes |= uint64_t(grcDataSize::Float2) << uint64_t(6 * 4);
			vertexSize += 8;
		}

		// no blend weights (for complex hierarchy)?
		if (!bits & (1 << 1))
		{
			bits |= (1 << 1);
			dataSizes |= uint64_t(grcDataSize::UByte4) << uint64_t(1 * 4);
			vertexSize += 4;
		}

		if (!bits & (1 << 2))
		{
			bits |= (1 << 2);
			dataSizes |= uint64_t(grcDataSize::UByte4) << uint64_t(2 * 4);
			vertexSize += 4;
		}

		// build vertex buffer
		std::vector<uint8_t> vertices(vertexSize * count);
		uint32_t outOffset = 0;
		
		for (int fvfIndex = 0; fvfIndex < 16; fvfIndex++)
		{
			if (!(bits & (1 << fvfIndex)))
			{
				continue;
			}

			if (!accessors[fvfIndex])
			{
				// make up colors
				if (fvfIndex == 4)
				{
					auto outData = &vertices[outOffset];

					for (uint32_t vert = 0; vert < count; vert++)
					{
						*(uint32_t*)outData = 0xffffffff;
						outData += vertexSize;
					}

					outOffset += 4;
				}
				else if (fvfIndex == 6)
				{
					auto outData = &vertices[outOffset];

					for (uint32_t vert = 0; vert < count; vert++)
					{
						*(float*)outData = 0.0f;
						*((float*)outData + 1) = 0.0f;
						outData += vertexSize;
					}

					outOffset += 8;
				}
				else if (fvfIndex == 1)
				{
					auto outData = &vertices[outOffset];

					for (uint32_t vert = 0; vert < count; vert++)
					{
						*(uint32_t*)outData = 0x000000FF;
						outData += vertexSize;
					}

					outOffset += 4;
				}
				else if (fvfIndex == 2)
				{
					auto outData = &vertices[outOffset];

					for (uint32_t vert = 0; vert < count; vert++)
					{
						*(uint32_t*)outData = 0x00000000 | (uint8_t)boneIdx;
						outData += vertexSize;
					}

					outOffset += 4;
				}

				continue;
			}

			const auto& accessor = *accessors[fvfIndex];

			grcDataSize dataSize = (grcDataSize)((dataSizes >> uint64_t(fvfIndex * 4)) & 0xF);

			const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

			auto inStride = accessor.ByteStride(bufferView);
			const uint8_t* inData = &buffer.data[bufferView.byteOffset];
			uint8_t* outData = &vertices[outOffset];

			uint32_t compSize = 0;
			increaseSize(dataSize, compSize);

			for (uint32_t vert = 0; vert < accessor.count; vert++)
			{
				memcpy(outData, inData, compSize);

				inData += inStride;
				outData += vertexSize;
			}

			// convert coordsys
			if (fvfIndex == 0 || fvfIndex == 3 || fvfIndex == 14)
			{
				outData = &vertices[outOffset];

				for (uint32_t vert = 0; vert < accessor.count; vert++)
				{
					float* dataRef = (float*)outData;
					std::swap(dataRef[1], dataRef[2]);
					dataRef[0] = -dataRef[0];

					outData += vertexSize;
				}

				// scale positions
				if (fvfIndex == 0)
				{
					outData = &vertices[outOffset];

					for (uint32_t vert = 0; vert < accessor.count; vert++)
					{
						float* dataRef = (float*)outData;
						DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)dataRef, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)dataRef), scaleMatrix));
						
						outData += vertexSize;
					}
				}
				else if (fvfIndex == 3 || fvfIndex == 14)
				{
					outData = &vertices[outOffset];

					for (uint32_t vert = 0; vert < accessor.count; vert++)
					{
						float* dataRef = (float*)outData;
						DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)dataRef, DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)dataRef), rotMatrix));

						outData += vertexSize;
					}
				}
			}

			outOffset += compSize;
		}

		rage::five::grcVertexFormat* fvf = new (false) rage::five::grcVertexFormat(bits, vertexSize, numFields, dataSizes);
		vertexBuffer->SetVertices(count, vertexSize, &vertices[0]);
		vertexBuffer->SetVertexFormat(fvf);

		gameGeom->SetVertexBuffer(vertexBuffer);
	}

	return gameGeom;
}

static rage::five::grcTexturePC* MakeTexture(const std::vector<double>& factor, rage::five::pgDictionary<rage::five::grcTexturePC>* txd)
{
	uint8_t a, r, g, b;
	r = uint8_t(factor[0] * 255.0);
	g = uint8_t(factor[1] * 255.0);
	b = uint8_t(factor[2] * 255.0);
	a = uint8_t(factor[3] * 255.0);

	auto imageName = fmt::sprintf("c_%02X%02X%02X%02X", r, g, b, a);

	if (txd->Get(imageName.c_str()))
	{
		return txd->Get(imageName.c_str());
	}

	uint32_t color = (a << 24) | (r << 16) | (g << 8) | (b << 0);

	uint16_t width = 1;
	uint16_t height = 1;

	int format = D3DFMT_A8R8G8B8;

	auto tex = new (false) rage::five::grcTexturePC(
	width,
	height,
	format,
	4,
	1,
	&color);

	tex->SetName(imageName.c_str());
	txd->Add(imageName.c_str(), tex);

	return tex;
}

static rage::five::grcTexturePC* ConvertTexture(const tinygltf::Model& model, int textureIdx, rage::five::pgDictionary<rage::five::grcTexturePC>* txd)
{
	const tinygltf::Texture& texture = model.textures[textureIdx];
	const tinygltf::Image& image = model.images[texture.source];

	auto imageName = image.name;

	if (imageName.empty())
	{
		imageName = image.uri;
	}

	if (imageName.empty())
	{
		imageName = fmt::sprintf("t%d", rand());
	}

	if (txd->Get(imageName.c_str()))
	{
		return txd->Get(imageName.c_str());
	}

	assert(!image.as_is);

	uint16_t width = image.width;
	uint16_t height = image.height;

	int format = D3DFMT_UNKNOWN;

	if (image.bits == 8)
	{
		if (image.component == 4)
		{
			format = D3DFMT_A8B8G8R8;
		}
		else if (image.component == 3)
		{
			format = D3DFMT_R8G8B8;
		}
		else if (image.component == 2)
		{
			format = D3DFMT_A8L8;
		}
		else if (image.component == 1)
		{
			format = D3DFMT_L8;
		}
	}
	else if (image.bits == 16)
	{
		if (image.component == 4)
		{
			format = D3DFMT_A16B16G16R16;
		}
		else if (image.component == 3)
		{
			//??
		}
		else if (image.component == 2)
		{
			format = D3DFMT_G16R16;
		}
		else if (image.component == 1)
		{
			format = D3DFMT_L16;
		}
	}

	auto tex = new (false) rage::five::grcTexturePC(
	width,
	height,
	format,
	(image.bits * image.component * image.width) / 8,
	1, // TODO: gen mips!
	&image.image[0]);

	tex->SetName(imageName.c_str());
	txd->Add(imageName.c_str(), tex);
	
	return tex;
}

static rage::five::grmShaderFx* ConvertShader(const tinygltf::Model& model, int materialIdx, rage::five::pgDictionary<rage::five::grcTexturePC>* txd)
{
	const tinygltf::Material& material = model.materials[materialIdx];
	
	bool hasNormal = material.normalTexture.index >= 0;
	bool hasEmissive = material.emissiveTexture.index >= 0;
	bool hasAlbedo = material.pbrMetallicRoughness.baseColorTexture.index >= 0;
	// we don't do specular *yet*

	std::string shaderPreset = "default.sps";

	if (hasNormal)
	{
		shaderPreset = "normal.sps";
	}

	if (hasEmissive)
	{
		shaderPreset = "emissive.sps";
	}

	auto spsFile = fxc::SpsFile::Load(MakeRelativeCitPath(fmt::sprintf(L"citizen\\shaders\\db\\%s", ToWide(shaderPreset))));
	std::string shaderName;

	if (spsFile)
	{
		shaderName = spsFile->GetShader();
	}

	rage::five::grmShaderFx* shader = new (false) rage::five::grmShaderFx();
	shader->DoPreset(shaderName.c_str(), shaderPreset.c_str());

	// emissive trumps all
	if (hasEmissive)
	{
		shader->SetParameter("DiffuseSampler", ConvertTexture(model, material.emissiveTexture.index, txd));

		float emissiveMult = float(material.emissiveFactor[0] + material.emissiveFactor[1] + material.emissiveFactor[2]) / 3.0f;
		shader->SetParameter("emissiveMultiplier", &emissiveMult, sizeof(emissiveMult));
	}
	else
	{
		if (hasNormal)
		{
			shader->SetParameter("BumpSampler", ConvertTexture(model, material.normalTexture.index, txd));

			float bumpiness = (float)material.normalTexture.scale;
			shader->SetParameter("bumpiness", &bumpiness, sizeof(bumpiness));
		}

		if (!hasAlbedo)
		{
			shader->SetParameter("DiffuseSampler", MakeTexture(material.pbrMetallicRoughness.baseColorFactor, txd));
		}
		else
		{
			shader->SetParameter("DiffuseSampler", ConvertTexture(model, material.pbrMetallicRoughness.baseColorTexture.index, txd));
		}
	}

	return shader;

	//shaders[0] = new (false) rage::five::grmShaderFx();
	//shaders[0]->DoPreset("default", "default.sps");
	//shaders[0]->SetParameter("DiffuseSampler", "dreamy");
}

struct GeomSetupContext
{
	bool hadSkin = false;
	int geomIndex = 0;
	int boneIndex = 0;

	rage::five::grmGeometryQB* geometries[512];
	uint16_t shaderMappings[512];
	rage::five::GeometryBound gbs[512];

	rage::five::crBone bones[256];
	std::map<int, int> boneMapping;
};

#if 0
static void ProcessMesh(const tinygltf::Model& model, const std::string& nodeName, int meshIdx, const DirectX::XMMATRIX& matrix)
{
	const tinygltf::Mesh& mesh = model.meshes[meshIdx];

	std::string meshName = mesh.name;

	if (!meshName.empty())
	{
		meshName = nodeName;
	}

	OutputFile([&]()
	{
		auto out = new (false) rage::five::gtaDrawable;
		out->SetBlockMap();

		out->SetName(fmt::format("{}.#dr", nodeName).c_str());
		auto& lodGroup = out->GetLodGroup();

		rage::five::grmModel* gameModel = new (false) rage::five::grmModel;

		int i = 0;

		for (auto& geom : mesh.primitives)
		{
			geometries[i] = ConvertGeometry(model, geom, matrix);
			shaderMappings[i] = 0;

			i++;
		}

		gameModel->SetGeometries(i, geometries);
		gameModel->SetGeometryBounds(i, gbs);
		
		lodGroup.SetModel(0, gameModel);
		lodGroup.SetDrawBucketMask(0, 0xff01);
		lodGroup.SetMaxPoint({ 9999.0f, 9999.0f, 9999.0f, 9999.0f });

		// set up shaders
		rage::five::pgPtr<rage::five::grmShaderFx> shaders[512];
		//shaders[0] = new (false) rage::five::grmShaderFx();
		//shaders[0]->DoPreset("default", "default.sps");
		//shaders[0]->SetParameter("DiffuseSampler", "dreamy");

		i = 0;
		int shaderIdx = 0;

		std::map<int, int> materialsToShaders;
		rage::five::pgDictionary<rage::five::grcTexturePC> txd;

		for (auto& geom : mesh.primitives)
		{
			if (materialsToShaders.find(geom.material) != materialsToShaders.end())
			{
				shaderMappings[i] = materialsToShaders[geom.material];

				i++;
				continue;
			}

			auto materialIdx = geom.material;
			shaders[shaderIdx] = ConvertShader(model, geom.material, &txd);

			materialsToShaders[materialIdx] = shaderIdx;
			shaderMappings[i] = shaderIdx;

			shaderIdx++;
			i++;
		}

		rage::five::grmShaderGroup* shaderGroup = new (false) rage::five::grmShaderGroup();
		shaderGroup->SetShaders(shaderIdx, shaders);

		if (txd.GetCount())
		{
			rage::five::pgDictionary<rage::five::grcTexturePC>* rtxd = new (false) rage::five::pgDictionary<rage::five::grcTexturePC>();
			rtxd->SetBlockMap();

			rtxd->SetFrom(&txd);

			shaderGroup->SetTextures(rtxd);
		}

		gameModel->SetShaderMappings(i, shaderMappings);

		out->SetPrimaryModel();
		out->SetShaderGroup(shaderGroup);
	}, 165, ToWide(nodeName + ".ydr"));
}
#endif

static void AddMesh(GeomSetupContext& context, const tinygltf::Model& model, int meshIdx, const DirectX::XMMATRIX& matrix, int boneIdx = -1)
{
	const tinygltf::Mesh& mesh = model.meshes[meshIdx];

	for (auto& geom : mesh.primitives)
	{
		context.geometries[context.geomIndex] = ConvertGeometry(model, geom, matrix, boneIdx);
		context.shaderMappings[context.geomIndex] = geom.material;

		context.geomIndex++;
	}
}

static int ProcessNodesForSkeleton(GeomSetupContext& ctx, const tinygltf::Model& model, int nodeIdx, int parentIdx = -1, DirectX::XMMATRIX parentMatrix = DirectX::XMMatrixIdentity())
{
	using namespace DirectX;

	const tinygltf::Node& node = model.nodes[nodeIdx];

	std::string nodeName = node.name;

	if (nodeName.empty())
	{
		nodeName = fmt::format("node_{}", nodeIdx);
	}
	
	auto matrix = ConvertNodeMatrix(node);
	auto fullMatrix = parentMatrix * matrix;

	DirectX::XMVECTOR scale;
	DirectX::XMVECTOR rotQuat;
	DirectX::XMVECTOR trans;
	DirectX::XMMatrixDecompose(&scale, &rotQuat, &trans, matrix);

	rage::Vector4 rotationV;
	rage::Vector3 transV;
	rage::Vector3 scaleV;

	scaleV.x = DirectX::XMVectorGetX(scale);
	scaleV.y = DirectX::XMVectorGetY(scale);
	scaleV.z = DirectX::XMVectorGetZ(scale);

	rotationV.x = DirectX::XMVectorGetX(rotQuat);
	rotationV.y = DirectX::XMVectorGetY(rotQuat);
	rotationV.z = DirectX::XMVectorGetZ(rotQuat);
	rotationV.w = DirectX::XMVectorGetW(rotQuat);

	transV.x = DirectX::XMVectorGetX(trans);
	transV.y = DirectX::XMVectorGetY(trans);
	transV.z = DirectX::XMVectorGetZ(trans);

	using rage::five::crBoneFlags;

	int siblingIdx = -1;

	int bi = ctx.boneIndex++;
	ctx.bones[bi].Init(nodeName.c_str(), bi, (crBoneFlags)(crBoneFlags::RotX | crBoneFlags::RotY | crBoneFlags::RotZ | crBoneFlags::TransX | crBoneFlags::TransY | crBoneFlags::TransZ), rotationV, scaleV, transV, parentIdx, siblingIdx);

	ctx.boneMapping[nodeIdx] = bi;

	int lastIdx = -1;

	for (int c = 0; c < node.children.size(); c++)
	{
		int childIdx = node.children[c];
		int idx = ProcessNodesForSkeleton(ctx, model, childIdx, nodeIdx, fullMatrix);
		
		if (lastIdx >= 0)
		{
			ctx.bones[lastIdx].SetSiblingIndex(idx);
		}

		lastIdx = idx;
	}

	return bi;
}

static void ProcessNode(const tinygltf::Model& model, int nodeIdx, const std::string& sceneName, int depth = 0, DirectX::XMMATRIX parentMatrix = DirectX::XMMatrixIdentity(), DirectX::XMMATRIX invRootMatrix = DirectX::XMMatrixIdentity())
{
	using namespace DirectX;

	const tinygltf::Node& node = model.nodes[nodeIdx];

	auto matrix = ConvertNodeMatrix(node);
	auto fullMatrix = parentMatrix * matrix;

	if (depth == 0)
	{
		DirectX::XMVECTOR scale;
		DirectX::XMVECTOR rotQuat;
		DirectX::XMVECTOR trans;
		DirectX::XMMatrixDecompose(&scale, &rotQuat, &trans, fullMatrix);

		auto noScaleMatrix = DirectX::XMMatrixAffineTransformation(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMVectorZero(), rotQuat, trans);

		invRootMatrix = DirectX::XMMatrixInverse(NULL, noScaleMatrix);
	}

	std::string nodeName = node.name;

	if (nodeName.empty())
	{
		nodeName = fmt::format("{}_{}", sceneName, nodeIdx);
	}

	static thread_local GeomSetupContext* geomCtx;

	auto processEntry = [&]()
	{
		if (node.mesh >= 0)
		{
			AddMesh(*geomCtx, model, node.mesh, invRootMatrix * fullMatrix, geomCtx->boneMapping[nodeIdx]);
		}

		for (int nodeIdx : node.children)
		{
			ProcessNode(model, nodeIdx, sceneName, depth + 1, fullMatrix, invRootMatrix);
		}
	};

	if (depth == 0)
	{
		geomCtx = new GeomSetupContext();

		OutputFile([&]()
		{
			auto out = new (false) rage::five::gtaDrawable;
			out->SetBlockMap();

			out->SetName(fmt::format("{}.#dr", nodeName).c_str());

			ProcessNodesForSkeleton(*geomCtx, model, nodeIdx);

			auto& lodGroup = out->GetLodGroup();

			rage::five::grmModel* gameModel = new (false) rage::five::grmModel;

			processEntry();

			gameModel->SetGeometries(geomCtx->geomIndex, geomCtx->geometries);
			gameModel->SetGeometryBounds(geomCtx->geomIndex, geomCtx->gbs);

			lodGroup.SetModel(0, gameModel);
			lodGroup.SetDrawBucketMask(0, 0xff01);
			lodGroup.SetMaxPoint({ 9999.0f, 9999.0f, 9999.0f, 9999.0f });

			// set up shaders
			rage::five::pgPtr<rage::five::grmShaderFx> shaders[512];

			int shaderIdx = 0;

			std::map<int, int> materialsToShaders;
			rage::five::pgDictionary<rage::five::grcTexturePC> txd;

			for (int i = 0; i < geomCtx->geomIndex;)
			{
				if (materialsToShaders.find(geomCtx->shaderMappings[i]) != materialsToShaders.end())
				{
					geomCtx->shaderMappings[i] = materialsToShaders[geomCtx->shaderMappings[i]];

					i++;
					continue;
				}

				auto materialIdx = geomCtx->shaderMappings[i];
				shaders[shaderIdx] = ConvertShader(model, materialIdx, &txd);

				materialsToShaders[materialIdx] = shaderIdx;
				geomCtx->shaderMappings[i] = shaderIdx;

				shaderIdx++;
				i++;
			}

			rage::five::grmShaderGroup* shaderGroup = new (false) rage::five::grmShaderGroup();
			shaderGroup->SetShaders(shaderIdx, shaders);

			if (txd.GetCount())
			{
				rage::five::pgDictionary<rage::five::grcTexturePC>* rtxd = new (false) rage::five::pgDictionary<rage::five::grcTexturePC>();
				rtxd->SetBlockMap();

				rtxd->SetFrom(&txd);

				shaderGroup->SetTextures(rtxd);
			}

			gameModel->SetShaderMappings(geomCtx->geomIndex, geomCtx->shaderMappings);

			auto skeleton = new (false) rage::five::crSkeletonData();
			skeleton->SetBones(geomCtx->boneIndex, geomCtx->bones);
			//out->SetSkeleton(skeleton);

			out->SetPrimaryModel();
			out->SetShaderGroup(shaderGroup);

			delete geomCtx;
		},
		165, ToWide(nodeName + ".ydr"));
	}
	else
	{
		processEntry();
	}
}

static void ConvertAsset(const boost::filesystem::path& path)
{
	tinygltf::Model model;
	std::string err, warn;

	// load the asset
	tinygltf::TinyGLTF loader;

	bool success = false;
	if (path.extension() == ".glb")
	{
		success = loader.LoadBinaryFromFile(&model, &err, &warn, ToNarrow(path.wstring()));
	}
	else
	{
		success = loader.LoadASCIIFromFile(&model, &err, &warn, ToNarrow(path.wstring()));
	}

	// check for success
	if (!warn.empty())
	{
		fmt::printf("WARN: %s\n", warn);
	}

	if (!err.empty())
	{
		fmt::printf("ERR: %s\n", err);
	}

	if (!success)
	{
		return;
	}

	if (model.defaultScene < 0)
	{
		fmt::printf("%s: Assets without default scene are not supported!\n", path.filename().string());
		return;
	}

	const tinygltf::Scene& scene = model.scenes[model.defaultScene];
	auto fn = path.filename();
	fn.replace_extension();

	std::string sceneName = scene.name.empty() ? fn.string() : scene.name;

	for (int nodeIdx : scene.nodes)
	{
		ProcessNode(model, nodeIdx, sceneName);
	}
}

static void HandleArguments(boost::program_options::wcommand_line_parser& parser, std::function<void()> cb)
{
	boost::program_options::options_description desc;

	desc.add_options()("filename", boost::program_options::value<std::vector<boost::filesystem::path>>()->required(), "The path of the file to convert.");

	boost::program_options::positional_options_description positional;
	positional.add("filename", -1);

	parser.options(desc).positional(positional);

	cb();
}

static void Run(const boost::program_options::variables_map& map)
{
	if (map.count("filename") == 0)
	{
		printf("Usage:\n\n   fivem formats:gltfImport *.gltf/*.glb...\n");
		return;
	}

	auto& entries = map["filename"].as<std::vector<boost::filesystem::path>>();

	for (auto& filePath : entries)
	{
		ConvertAsset(filePath);
	}
}

static FxToolCommand command("formats:gltfImport", HandleArguments, Run);
#endif
