/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <d3d9.h>

enum
{
	D3DFMT_ATI1 = MAKEFOURCC('A', 'T', 'I', '1'),
	D3DFMT_ATI2 = MAKEFOURCC('A', 'T', 'I', '2'),
};

#include <map>
#include <optional>
#include <string>

#include <d3d9.h>

#include <Error.h>

#define RAGE_FORMATS_GAME rdr3
#define RAGE_FORMATS_GAME_RDR3
#include <gtaDrawable.h>

#undef RAGE_FORMATS_GAME_RDR3
#undef RAGE_FORMATS_GAME
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <gtaDrawable.h>

#include <convert/base.h>

namespace rage
{
enum class FVFType
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
	Dec3N
};

struct meh : public rdr3::pgStreamableBase
{
	char stuff[64];

	meh()
	{
		static unsigned char hexData[64] = {
			0x80, 0x00, 0x91, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		memcpy(stuff, hexData, 64);
	}
};

inline std::optional<rdr3::sgaInputSemantic> mapVertexSemantic(int semantic)
{
	switch (semantic)
	{
		case 0:
			return rdr3::POSITION;
		case 3:
			return rdr3::NORMAL;
		case 4:
			return rdr3::COLOR0;
		case 5:
			return rdr3::COLOR1;
		case 6:
			return rdr3::TEXCOORD0;
		case 7:
			return rdr3::TEXCOORD1;
		case 8:
			return rdr3::TEXCOORD2;
		case 9:
			return rdr3::TEXCOORD3;
		case 10:
			return rdr3::TEXCOORD4;
		case 11:
			return rdr3::TEXCOORD5;
		case 12:
			return rdr3::TEXCOORD6;
		case 13:
			return rdr3::TEXCOORD7;
		case 14:
			return rdr3::TANGENT;
		case 15:
			return rdr3::TANGENT1;
	}

	return {};
}

inline std::optional<int> mapVertexSemantic(rdr3::sgaInputSemantic semantic)
{
	switch (semantic)
	{
		case rdr3::POSITION:
			return 0;
		case rdr3::NORMAL:
			return 3;
		case rdr3::COLOR0:
			return 4;
		case rdr3::COLOR1:
			return 5;
		case rdr3::TEXCOORD0:
			return 6;
		case rdr3::TEXCOORD1:
			return 7;
		case rdr3::TEXCOORD2:
			return 8;
		case rdr3::TEXCOORD3:
			return 9;
		case rdr3::TEXCOORD4:
			return 10;
		case rdr3::TEXCOORD5:
			return 11;
		case rdr3::TEXCOORD6:
			return 12;
		case rdr3::TEXCOORD7:
			return 13;
		case rdr3::TANGENT:
			return 14;
		case rdr3::TANGENT1:
			return 15;
	}

	return {};
}

inline rdr3::sgaBufferFormat mapType(FVFType type)
{
	switch (type)
	{
		case FVFType::Float:
			return rdr3::R32_FLOAT;
		case FVFType::Float2:
			return rdr3::R32G32_FLOAT;
		case FVFType::Float3:
			return rdr3::R32G32B32_FLOAT;
		case FVFType::Float4:
			return rdr3::R32G32B32A32_FLOAT;
		case FVFType::Color:
			return rdr3::R8G8B8A8_UNORM;
		case FVFType::Dec3N:
			return rdr3::R10G10B10A2_UNORM;
	}

	return rdr3::UNKNOWN;
}

template<>
rdr3::sgaInputLayout* convert(five::grcVertexFormat* in)
{
	auto out = new (false) rdr3::sgaInputLayout;
	memset(out, 0, sizeof(*out));

	int off = 0;

	for (int i = 0; i < 16; i++)
	{
		if (in->GetMask() & (1 << i))
		{
			auto sm = mapVertexSemantic(i);

			if (sm)
			{
				auto ft = (FVFType)((in->GetFVF() >> (4 * i)) & 0xF);

				out->types[(int)*sm] = mapType(ft);
				out->offsets[(int)*sm] = off;
				out->sizes[(int)*sm] = in->GetVertexSize();

				switch (ft)
				{
					case FVFType::Float4:
						off += 4;
					case FVFType::Float3:
						off += 4;
					case FVFType::Float2:
						off += 4;
					case FVFType::Nothing:
					case FVFType::Float:
					case FVFType::Float_unk:
						off += 4;
						break;
					case FVFType::Dec3N:
						off += 4;
						break;
					case FVFType::Color:
					case FVFType::UByte4:
						off += 4;
						break;
					case FVFType::Float16_2:
						off += 4;
						break;
				}
			}
		}
	}

	return out;
}

template<>
rdr3::grcVertexBufferD3D* convert(five::grcVertexBufferD3D* buffer)
{
	auto out = new (false) rdr3::grcVertexBufferD3D;
	memset(out, 0, sizeof(*out));

	out->m_unkBuffer = new (false) meh;
	out->m_bindFlags = 0x209000;

	auto ob = (char*)buffer->GetVertices();
	auto vs = buffer->GetStride();
	auto vc = buffer->GetCount();

	auto il = new (false) rdr3::sgaInputLayout();
	auto of = buffer->GetVertexFormat();

	int goff = 0;
	
	int toffs[16];
	void* vb = NULL;

	{
		int off = 0;

		for (int i = 0; i < 16; i++)
		{
			if (of->GetMask() & (1 << i))
			{
				auto ft = (FVFType)((of->GetFVF() >> (4 * i)) & 0xF);
				toffs[i] = off;

				switch (ft)
				{
					case FVFType::Float4:
						off += 4;
					case FVFType::Float3:
						off += 4;
					case FVFType::Float2:
						off += 4;
					case FVFType::Nothing:
					case FVFType::Float:
					case FVFType::Float_unk:
						off += 4;
						break;
					case FVFType::Dec3N:
						off += 4;
						break;
					case FVFType::Color:
					case FVFType::UByte4:
						off += 4;
						break;
					case FVFType::Float16_2:
						off += 4;
						break;
				}
			}
		}

		vb = rdr3::pgStreamManager::Allocate(buffer->GetCount() * off, PHYSICAL_VERTICES, nullptr);
	}

	int outStride = 0;

	//for (int i = 0; i < 16; i++)
	for (int s = 0; s < 52; s++)
	{
		auto si = mapVertexSemantic((rdr3::sgaInputSemantic)s);

		if (si)
		{
			int i = *si;
			int* sm = &s;

			if (of->GetMask() & (1 << i))
			{
				auto ft = (FVFType)((of->GetFVF() >> (4 * i)) & 0xF);

				il->types[(int)*sm] = mapType(ft);
				il->offsets[(int)*sm] = goff;

				int off = 0;

				switch (ft)
				{
					case FVFType::Float4:
						off += 4;
					case FVFType::Float3:
						off += 4;
					case FVFType::Float2:
						off += 4;
					case FVFType::Nothing:
					case FVFType::Float:
					case FVFType::Float_unk:
						off += 4;
						break;
					case FVFType::Dec3N:
						off += 4;
						break;
					case FVFType::Color:
					case FVFType::UByte4:
						off += 4;
						break;
					case FVFType::Float16_2:
						off += 4;
						break;
				}

				il->sizes[(int)*sm] = off;

				auto ooff = off;

				if (*sm == rdr3::NORMAL || *sm == rdr3::TANGENT)
				{
					off = 4;
					il->sizes[(int)*sm] = off;
					il->types[(int)*sm] = rdr3::R10G10B10A2_UNORM;
				}

				char* outStart = (char*)vb + goff;
				int toff = toffs[i];
				
				for (int v = 0; v < vc; v++)
				{
					char* inStart = ob + (v * vs) + toff;

					// normals are abnormal now (for R10G10B10A2 format)
					if (*sm == rdr3::NORMAL || *sm == rdr3::TANGENT)
					{
						struct dec3
						{
							uint32_t r : 10;
							uint32_t g : 10;
							uint32_t b : 10;
							uint32_t a : 2;
						};

						auto n = (float*)inStart;
						auto o = (dec3*)outStart;

						int a = 0;

						if (*sm == rdr3::TANGENT)
						{
							float w = n[3];

							if (w < -0.5f)
							{
								a = 0;
							}
							else if (w > 0.5f)
							{
								a = 3;
							}
						}

						o->r = (((n[0] + 1.0f) / 2.0f) * 1023.f) + 0.5f;
						o->g = (((n[1] + 1.0f) / 2.0f) * 1023.f) + 0.5f;
						o->b = (((n[2] + 1.0f) / 2.0f) * 1023.f) + 0.5f;
						o->a = a;
					}
					else
					/*if (*sm == rdr3::NORMAL || *sm == rdr3::TANGENT)
					{
						auto n = (float*)inStart;
						auto o = (float*)outStart;

						o[0] = (n[0] + 1.0f) / 2.0f;
						o[1] = (n[1] + 1.0f) / 2.0f;
						o[2] = (n[2] + 1.0f) / 2.0f;

						if (*sm == rdr3::TANGENT)
						{
							//o[3] = (n[3] + 1.0f) / 2.0f;
							o[3] = 1.0f;
						}
					}
					else*/
					{
						memcpy(outStart, inStart, off);
					}

					outStart += off;
				}

				outStride += off;
				goff += off * vc;
			}
		}
	}

	out->m_vertexCount = buffer->GetCount();
	out->m_vertexSize = outStride;
	out->m_vertexFormat = il;
	out->m_vertexData = vb;

	il->hasSOA = true;
	il->flag = false;
	il->vertexCount = out->m_vertexCount;
	il->vertexSize = out->m_vertexSize;

	return out;
}

template<>
rdr3::grcIndexBufferD3D* convert(five::grcIndexBufferD3D* buffer)
{
	auto out = new (false) rdr3::grcIndexBufferD3D(buffer->GetIndexCount(), buffer->GetIndexData());
	out->m_unkBuffer = new (false) meh;

	return out;
}

template<>
rdr3::grmGeometry* convert(five::grmGeometryQB* geometry)
{
	auto out = new (false) rdr3::grmGeometry;

	out->SetIndexBuffer(convert<rdr3::grcIndexBufferD3D*>(geometry->GetIndexBuffer(0)));
	out->SetVertexBuffer(convert<rdr3::grcVertexBufferD3D*>(geometry->GetVertexBuffer(0)));

	return out;
}

template<>
rdr3::grmModel* convert(five::grmModel* model)
{
	auto out = new (false) rdr3::grmModel;
	int ni = 0;

	{
		auto& oldGeometries = model->GetGeometries();
		rdr3::grmGeometry* geometries[256];
		uint16_t sms[256];

		for (int i = 0; i < oldGeometries.GetCount(); i++)
		{
			auto oldie = oldGeometries.Get(i);

			//if (oldie->GetVertexBuffer(0)->GetStride() == 0x24)
			{
				geometries[ni] = convert<rdr3::grmGeometry*>(oldie);
				sms[ni] = model->GetShaderMappings()[i];
				ni++;
			}
		}

		out->SetGeometries(ni, geometries);
		out->SetShaderMappings(ni, sms);
	}

	

	return out;
}

static uint32_t MapPixelFormat(uint32_t format)
{
	static std::map<uint32_t, uint32_t> formats{
		{ D3DFMT_UNKNOWN, rdr3::sgaBufferFormat::UNKNOWN },
		//{ D3DFMT_B32G32R32F, rdr3::sgaBufferFormat::R32G32B32_FLOAT },
		{ D3DFMT_A32B32G32R32F, rdr3::sgaBufferFormat::R32G32B32A32_FLOAT },
		{ D3DFMT_A16B16G16R16F, rdr3::sgaBufferFormat::R16G16B16A16_FLOAT },
		{ D3DFMT_R32F, rdr3::sgaBufferFormat::R32_FLOAT },
		{ D3DFMT_R16F, rdr3::sgaBufferFormat::R16_FLOAT },
		{ D3DFMT_Q16W16V16U16, rdr3::sgaBufferFormat::R16G16B16A16_SNORM },
		//{ D3DFMT_D3DDECLTYPE_SHORT4, rdr3::sgaBufferFormat::R16G16B16A16_SINT },
		//{ D3DFMT_D3DDECLTYPE_UBYTE4, rdr3::sgaBufferFormat::R8G8B8A8_UINT },
		{ D3DFMT_V16U16, rdr3::sgaBufferFormat::R16G16_SNORM },
		//{ D3DFMT_D3DDECLTYPE_SHORT2, rdr3::sgaBufferFormat::R16G16_SINT },
		{ D3DFMT_R32F, rdr3::sgaBufferFormat::R32_FLOAT },
		{ D3DFMT_INDEX32, rdr3::sgaBufferFormat::R32_UINT },
		{ D3DFMT_INDEX16, rdr3::sgaBufferFormat::R16_UINT },
		{ D3DFMT_A16B16G16R16, rdr3::sgaBufferFormat::R16G16B16A16_UNORM },
		{ D3DFMT_G32R32F, rdr3::sgaBufferFormat::R32G32_FLOAT },
		//{ D3DFMT_S8D24, rdr3::sgaBufferFormat::D24_UNORM_S8_UINT },
		{ D3DFMT_D16, rdr3::sgaBufferFormat::D16_UNORM },
		{ D3DFMT_A8B8G8R8, rdr3::sgaBufferFormat::R8G8B8A8_UNORM },
		{ D3DFMT_A2B10G10R10, rdr3::sgaBufferFormat::R10G10B10A2_UNORM },
		{ D3DFMT_X8R8G8B8, rdr3::sgaBufferFormat::B8G8R8A8_UNORM },
		{ D3DFMT_Q8W8V8U8, rdr3::sgaBufferFormat::R8G8B8A8_SNORM },
		{ D3DFMT_G16R16F, rdr3::sgaBufferFormat::R16G16_FLOAT },
		{ D3DFMT_G16R16, rdr3::sgaBufferFormat::R16G16_UNORM },
		{ D3DFMT_V16U16, rdr3::sgaBufferFormat::R16G16_SNORM },
		{ D3DFMT_R32F, rdr3::sgaBufferFormat::R32_FLOAT },
		{ D3DFMT_L16, rdr3::sgaBufferFormat::R16_UNORM },
		{ D3DFMT_V8U8, rdr3::sgaBufferFormat::R8G8_SNORM },
		{ D3DFMT_L8, rdr3::sgaBufferFormat::R8_UNORM },

		{ D3DFMT_DXT1, rdr3::sgaBufferFormat::BC1_UNORM },
		{ D3DFMT_DXT2, rdr3::sgaBufferFormat::BC2_UNORM },
		{ D3DFMT_DXT3, rdr3::sgaBufferFormat::BC2_UNORM },
		{ D3DFMT_DXT4, rdr3::sgaBufferFormat::BC3_UNORM },
		{ D3DFMT_DXT5, rdr3::sgaBufferFormat::BC3_UNORM },

		{ D3DFMT_ATI1, rdr3::sgaBufferFormat::BC4_UNORM },
		{ D3DFMT_ATI2, rdr3::sgaBufferFormat::BC5_UNORM },

		{ D3DFMT_A8R8G8B8, rdr3::sgaBufferFormat::B8G8R8A8_UNORM },

		{ D3DFMT_A8, rdr3::sgaBufferFormat::A8_UNORM },
		{ D3DFMT_R5G6B5, rdr3::sgaBufferFormat::B5G6R5_UNORM },
		{ D3DFMT_A1R5G5B5, rdr3::sgaBufferFormat::B5G5R5A1_UNORM },
		{ D3DFMT_A4R4G4B4, rdr3::sgaBufferFormat::B4G4R4A4_UNORM },

		{ D3DFMT_D32, rdr3::sgaBufferFormat::D32_FLOAT },
		{ D3DFMT_D16, rdr3::sgaBufferFormat::D32_FLOAT },
		{ D3DFMT_D16_LOCKABLE, rdr3::sgaBufferFormat::D32_FLOAT },
		{ D3DFMT_D32_LOCKABLE, rdr3::sgaBufferFormat::D32_FLOAT },
	};

	auto entry = formats.find(format);
	assert(entry != formats.end());

	return entry->second;
}

template<>
rdr3::pgDictionary<rdr3::grcTexturePC>* convert(five::pgDictionary<five::grcTexturePC>* txd)
{
	rdr3::pgDictionary<rdr3::grcTexturePC>* out = new (false) rdr3::pgDictionary<rdr3::grcTexturePC>();
	out->SetBlockMap();

	rdr3::pgDictionary<rdr3::grcTexturePC> newTextures;

	if (txd->GetCount()) // amazingly there's 0-sized TXDs?
	{
		for (auto& texture : *txd)
		{
			five::grcTexturePC* oldTexture = texture.second;
			rdr3::grcTexturePC* newTexture = new (false) rdr3::grcTexturePC(
			oldTexture->GetWidth(),
			oldTexture->GetHeight(),
			MapPixelFormat(oldTexture->GetPixelFormat()),
			oldTexture->GetStride(),
			oldTexture->GetLevels(),
			oldTexture->GetPixelData());

			newTexture->SetName(oldTexture->GetName());

			newTextures.Add(texture.first, newTexture);
		}
	}

	out->SetFrom(&newTextures);

	return out;
}


template<>
rdr3::grmShaderGroup* convert(five::grmShaderGroup* shaderGroup)
{
	auto out = new (false) rdr3::grmShaderGroup;

	auto texDict = shaderGroup->GetTextures();

	if (texDict)
	{
		out->SetTextures(convert<rdr3::pgDictionary<rdr3::grcTexturePC>*>(texDict));
	}
	
	rdr3::pgPtr<rdr3::grmShaderFx> newShaders[512];
	int sh;

	for (sh = 0; sh < shaderGroup->GetNumShaders(); sh++)
	{
		auto shs = shaderGroup->GetShader(sh)->GetShaderHash();

		if (shs == HashString("vehicle_track2") || shs == HashString("vehicle_track2_emissive"))
		{
			shs = HashString("vehicle_track");
		}
		else if (shs == HashString("terrain_cb_4lyr") || shs == HashString("terrain_cb_4lyr_pxm") || shs == HashString("terrain_cb_4lyr_spec") || shs == HashString("terrain_cb_4lyr_lod") || shs == HashString("terrain_cb_4lyr_2tex") || shs == HashString("terrain_cb_4lyr_2tex_pxm") || shs == HashString("terrain_cb_4lyr_2tex_blend") || shs == HashString("terrain_cb_4lyr_2tex_blend_lod") || shs == HashString("terrain_cb_4lyr_cm") || shs == HashString("terrain_cb_4lyr_cm_tnt") || shs == HashString("terrain_cb_w_4lyr") || shs == HashString("terrain_cb_w_4lyr_pxm") || shs == HashString("terrain_cb_w_4lyr_pxm_spm") || shs == HashString("terrain_cb_w_4lyr_spec") || shs == HashString("terrain_cb_w_4lyr_spec_pxm") || shs == HashString("terrain_cb_w_4lyr_spec_int") || shs == HashString("terrain_cb_w_4lyr_spec_int_pxm") || shs == HashString("terrain_cb_w_4lyr_lod") || shs == HashString("terrain_cb_w_4lyr_2tex") || shs == HashString("terrain_cb_w_4lyr_2tex_pxm") || shs == HashString("terrain_cb_w_4lyr_2tex_blend") || shs == HashString("terrain_cb_w_4lyr_2tex_blend_tt") || shs == HashString("terrain_cb_w_4lyr_2tex_blend_ttn") || shs == HashString("terrain_cb_w_4lyr_2tex_blend_pxm") || shs == HashString("terrain_cb_w_4lyr_2tex_blend_pxm_spm") || shs == HashString("terrain_cb_w_4lyr_2tex_blend_pxm_tt_spm") || shs == HashString("terrain_cb_w_4lyr_2tex_blend_pxm_tn_spm") || shs == HashString("terrain_cb_w_4lyr_2tex_blend_lod") || shs == HashString("terrain_cb_w_4lyr_cm") || shs == HashString("terrain_cb_w_4lyr_cm_pxm") || shs == HashString("terrain_cb_w_4lyr_cm_pxm_tnt") || shs == HashString("terrain_cb_w_4lyr_cm_tnt"))
		{
			shs = HashString("default");
		}

		rdr3::grmShaderFx* shader = new (false) rdr3::grmShaderFx;
		memset(shader, 0, sizeof(*shader));
		//shader->m_shaderHash = HashString("default");
		shader->m_shaderHash = shs;
		shader->m_shaderHashPad = 'meta';
		shader->m_drawBucketMask = 0xFF01;

		// we don't even have to follow the full shader logic: the game will copy defaults *at runtime* where needed
		std::vector<std::tuple<uint32_t, void*>> textureRefs;

		auto oldShader = shaderGroup->GetShader(sh);
		auto params = oldShader->GetParameters();

		const uint32_t* parameterNames = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(params) + oldShader->GetParameterSize());

		for (int i = 0; i < oldShader->GetParameterCount(); i++)
		{
			if (params[i].IsSampler())
			{
				auto pn = parameterNames[i];

				// map sampler -> texture (for now)
				if (pn == HashString("DiffuseSampler") || pn == HashString("TextureSampler_layer0"))
				{
					pn = HashString("diffuseTex");
				}
				else if (pn == HashString("BumpSampler") || pn == HashString("BumpSampler_layer0"))
				{
					pn = HashString("bumpTexture");
				}
				else if (pn == HashString("SpecSampler") || pn == HashString("SpecSampler_layer0"))
				{
					pn = HashString("specTexture");
				}
				else if (pn == HashString("DiffuseSampler2"))
				{
					pn = HashString("diffuseTex2");
				}
				else if (pn == HashString("DetailSampler"))
				{
					pn = HashString("DetailTexture");
				}
				else if (pn == HashString("DirtSampler"))
				{
					pn = HashString("dirtTexture");
				}
				else if (pn == HashString("tintPaletteSampler"))
				{
					pn = HashString("tintPaletteTex");
				}

				auto texture = (five::grcTextureRef*)params[i].GetValue();

				const char* textureName = (texture) ? texture->GetName() : "none";

				// look up if we just created one of these as local texture
				bool found = false;

				if (out->GetTextures())
				{
					for (auto& outTexture : *out->GetTextures())
					{
						if (!_stricmp(outTexture.second->GetName(), textureName))
						{
							textureRefs.push_back({ pn, outTexture.second });
							found = true;

							break;
						}
					}
				}

				// ... it's an external reference
				if (!found)
				{
					auto tr4 = new (false) rdr3::grcTextureRef();
					tr4->SetName(textureName);

					textureRefs.push_back({ pn, tr4 });
				}
			}
		}

		auto pd = (rdr3::sgaShaderParamData*)rdr3::pgStreamManager::Allocate(sizeof(rdr3::sgaShaderParamData) + (sizeof(uint64_t) * textureRefs.size()), false, nullptr);
		shader->m_parameterData = pd;

		shader->m_parameterData->numCBuffers = 0;
		shader->m_parameterData->numParams = textureRefs.size();
		shader->m_parameterData->numSamplers = 0;
		shader->m_parameterData->numUnk1 = 0;
		shader->m_parameterData->numTextures = textureRefs.size();

		auto firstArg = (char*)pd + sizeof(*pd);

		for (int i = 0; i < textureRefs.size(); i++)
		{
			*(uint32_t*)(&firstArg[(i * 8) + 0]) = std::get<0>(textureRefs[i]);
			*(uint32_t*)(&firstArg[(i * 8) + 4]) = 0; // the game should guess this from hash as we don't match
		}

		auto trs = (rdr3::grmShaderParameter*)rdr3::pgStreamManager::Allocate(sizeof(rdr3::grmShaderParameter) * textureRefs.size(), false, nullptr);

		for (int i = 0; i < textureRefs.size(); i++)
		{
			trs[i].SetValue(std::get<1>(textureRefs[i]));
		}

		shader->m_textureRefs = trs;

		newShaders[sh] = shader;
	}

	out->SetShaders(sh, newShaders);

	return out;
}

inline void ConvertBaseDrawable(five::rmcDrawable* drawable, rdr3::gtaDrawable* out)
{
	auto& oldLodGroup = drawable->GetLodGroup();
	out->SetBlockMap();

	out->SetShaderGroup(convert<rdr3::grmShaderGroup*>(drawable->GetShaderGroup()));

	auto& lodGroup = out->GetLodGroup();
	lodGroup.SetBounds(
		oldLodGroup.GetBoundsMin(),
		oldLodGroup.GetBoundsMax(),
		oldLodGroup.GetCenter(),
		oldLodGroup.GetRadius());

	for (int i = 0; i < 4; i++)
	{
		auto oldModel = oldLodGroup.GetModel(i);

		if (oldModel)
		{
			auto newModel = convert<rdr3::grmModel*>(oldModel);

			lodGroup.SetModel(i, newModel);
			lodGroup.SetDrawBucketMask(i, 0xff01); // TODO: change this

			{
				auto oldBounds = oldModel->GetGeometryBounds();

				int extraSize = 0;

				if (newModel->GetGeometries().GetCount() > 1)
				{
					extraSize = 1;
				}

				for (int b = 0; b < newModel->GetGeometries().GetCount() + extraSize; b++)
				{
					oldBounds[b].aabbMax.w = oldBounds[b].aabbMax.x;
					oldBounds[b].aabbMin.w = oldBounds[b].aabbMin.x;
				}

				newModel->SetGeometryBounds(newModel->GetGeometries().GetCount() + extraSize, (rage::rdr3::GeometryBound*)oldBounds);
			}
		}
	}

	out->SetName("drawable_from_redm_exporter_see_redm.gg.#dr");
}

template<>
rdr3::gtaDrawable* convert(five::gtaDrawable* drawable)
{
	auto out = new (false) rdr3::gtaDrawable;

	ConvertBaseDrawable(drawable, out);

	return out;
}

template<>
rdr3::pgDictionary<rdr3::gtaDrawable>* convert(five::pgDictionary<five::gtaDrawable>* dwd)
{
	auto out = new (false) rdr3::pgDictionary<rdr3::gtaDrawable>();
	out->SetBlockMap();

	rdr3::pgDictionary<rdr3::gtaDrawable> newDrawables;

	if (dwd->GetCount()) // amazingly there's 0-sized TXDs?
	{
		for (auto& drawable : *dwd)
		{
			newDrawables.Add(drawable.first, convert<rdr3::gtaDrawable*>(drawable.second));
		}
	}

	out->SetFrom(&newDrawables);

	return out;
}

template<>
rdr3::gtaDrawable* convert(five::fragType* drawable)
{
	auto out = new (false) rdr3::gtaDrawable;

	ConvertBaseDrawable(drawable->GetPrimaryDrawable(), out);

	return out;
}
}
