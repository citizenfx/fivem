/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <optional>
#include <string>

#include <Error.h>

#include <ShaderInfo.h>

#define RAGE_FORMATS_GAME rdr3
#define RAGE_FORMATS_GAME_RDR3
#include <gtaDrawable.h>

#undef RAGE_FORMATS_GAME_RDR3
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <gtaDrawable.h>

#include <convert/base.h>

namespace rage
{
extern FORMATS_EXPORT int g_curGeom;
extern FORMATS_EXPORT std::map<int, void*> g_vbMapping;
extern FORMATS_EXPORT std::map<int, void*> g_ibMapping;


enum pf
{
	D3DFMT_A8R8G8B8 = 21,
	D3DFMT_A1R5G5B5 = 25,
	D3DFMT_A8 = 28,
	D3DFMT_A8B8G8R8 = 32,
	D3DFMT_L8 = 50,

	// fourCC
	D3DFMT_DXT1 = 0x31545844,
	D3DFMT_DXT3 = 0x33545844,
	D3DFMT_DXT5 = 0x35545844,
	D3DFMT_ATI1 = 0x31495441,
	D3DFMT_ATI2 = 0x32495441,
	D3DFMT_BC7 = 0x20374342,
};

inline __declspec(noinline) uint32_t mapPixelFormat(rdr3::sgaBufferFormat format)
{
	switch (format)
	{
	case rdr3::B8G8R8A8_UNORM:
	case rdr3::B8G8R8A8_UNORM_SRGB:
		return D3DFMT_A8R8G8B8;
	case rdr3::R8G8B8A8_UNORM:
	case rdr3::R8G8B8A8_UNORM_SRGB:
		return D3DFMT_A8B8G8R8;
	case rdr3::A8_UNORM:
		return D3DFMT_A8;
	case rdr3::B5G5R5A1_UNORM:
		return D3DFMT_A1R5G5B5;
	case rdr3::R8_UNORM:
		return D3DFMT_L8;
	case rdr3::BC1_UNORM:
	case rdr3::BC1_UNORM_SRGB:
		return D3DFMT_DXT1;
	case rdr3::BC2_UNORM_SRGB:
		return D3DFMT_DXT3;
	case rdr3::BC3_UNORM:
	case rdr3::BC3_UNORM_SRGB:
		return D3DFMT_DXT5;
	case rdr3::BC4_UNORM:
		return D3DFMT_ATI1;
	case rdr3::BC5_UNORM:
		return D3DFMT_ATI2;
	case rdr3::BC7_UNORM:
	case rdr3::BC7_UNORM_SRGB:
		return D3DFMT_BC7;
	}

	return 0;
}


template<>
five::pgDictionary<five::grcTexturePC>* convert(rdr3::pgDictionary<rdr3::grcTexturePC>* txd)
{
	five::pgDictionary<five::grcTexturePC>* out = new(false) five::pgDictionary<five::grcTexturePC>();
	out->SetBlockMap();

	five::pgDictionary<five::grcTexturePC> newTextures;

	if (txd->GetCount()) // amazingly there's 0-sized TXDs?
	{
		for (auto& texture : *txd)
		{
			rdr3::grcTexturePC* nyTexture = texture.second;

			if (!nyTexture)
			{
				continue;
			}

			int ps = 1;
			int pf = nyTexture->GetPixelFormat();

			if (pf >= 70 && pf <= 84)
			{
				ps = 4;
			}
			else if (pf >= 94 && pf <= 99)
			{
				ps = 4;
			}

			auto smethod_0 = [](int int_2, int int_3, int int_4)
			{
				int num = 1;
				int num2 = 1;
				int num3 = num;
				int num4 = int_2;
				int num5 = int_3;
				int num6 = int_4;
				int num7 = 0;
				int num8 = 0;
				while (num5 > 1 || num6 > 1)
				{
					if (num5 > 1)
					{
						num7 += num3 * (num4 & 1);
						num4 >>= 1;
						num3 *= 2;
						num5 >>= 1;
					}
					if (num6 > 1)
					{
						num8 += num2 * (num4 & 1);
						num4 >>= 1;
						num2 *= 2;
						num6 >>= 1;
					}
				}
				return num8 * int_3 + num7;
			};

			static int bits[] = {
						0,
		128,
		128,
		128,
		128,
		96,
		96,
		96,
		96,
		64,
		64,
		64,
		64,
		64,
		64,
		64,
		64,
		64,
		64,
		64,
		64,
		64,
		64,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		16,
		16,
		16,
		16,
		16,
		16,
		16,
		16,
		16,
		16,
		16,
		16,
		8,
		8,
		8,
		8,
		8,
		8,
		1,
		32,
		32,
		32,
		4,
		4,
		4,
		8,
		8,
		8,
		8,
		8,
		8,
		4,
		4,
		4,
		8,
		8,
		8,
		16,
		16,
		32,
		32,
		32,
		32,
		32,
		32,
		32,
		8,
		8,
		8,
		8,
		8,
		8,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		16
			};

			uint8_t tmp[64];
			int wt = nyTexture->GetWidth() / ps;
			int ht = nyTexture->GetHeight() / ps;
			int s = 0;
			int num8 = bits[pf] * 2;

			if (ps == 1)
			{
				num8 /= 8;
			}

			std::vector<uint8_t> newPixelData(nyTexture->GetWidth() * nyTexture->GetHeight() * ps * 2);//nyTexture->GetDataSize() * 2);

			for (int i = 0; i < (ht + 7) / 8; i++)
			{
				for (int j = 0; j < (wt + 7) / 8; j++)
				{
					for (int k = 0; k < 64; k++)
					{
						int num12 = smethod_0(k, 8, 8);
						int num13 = num12 / 8;
						int num14 = num12 % 8;

						memcpy(tmp, &((uint8_t*)nyTexture->GetPixelData())[s], num8);
						s += num8;

						if (j * 8 + num14 < wt && i * 8 + num13 < ht)
						{
							int num15 = num8 * ((i * 8 + num13) * ht + j * 8 + num14);
							if (num15 + num8 <= newPixelData.size())
							{
								memcpy(&newPixelData[num15], tmp, num8);
							}
						}
					}
				}
			}

			five::grcTexturePC* fiveTexture = new(false) five::grcTexturePC(
				nyTexture->GetWidth(),
				nyTexture->GetHeight(),
				mapPixelFormat(nyTexture->GetPixelFormat()),
				(num8 / 8) * nyTexture->GetWidth(),
				nyTexture->GetLevels(),
				newPixelData.data()
			);

			fiveTexture->SetName(nyTexture->GetName());

			trace("%s: %p\n", nyTexture->GetName(), fiveTexture->GetPixelData());

			newTextures.Add(texture.first, fiveTexture);
		}
	}

	out->SetFrom(&newTextures);

	return out;
}



template<>
five::grcIndexBufferD3D* convert(rdr3::grcIndexBufferD3D* buffer)
{
	std::vector<uint16_t> indices(buffer->GetIndexCount());

	// assert index size?

	uint16_t* old = buffer->GetIndexData();
	for (int i = 0; i < buffer->GetIndexCount(); i++)
	{
		indices[i] = old[i];
	}

	return new(false) five::grcIndexBufferD3D(buffer->GetIndexCount(), &indices[0]);
}

inline std::optional<uint32_t> mapVertexSemantic(rdr3::sgaInputSemantic semantic)
{
	switch (semantic)
	{
	case rdr3::POSITION:
		return 1;
	case rdr3::NORMAL:
		return 8;
	case rdr3::COLOR2:
		return 16;
	case rdr3::COLOR1:
		return 32;
	case rdr3::TEXCOORD0:
		return 64;
	case rdr3::TEXCOORD1:
		return 128;
	case rdr3::TEXCOORD2:
		return 256;
	case rdr3::TEXCOORD3:
		return 512;
	case rdr3::TEXCOORD4:
		return 1024;
	case rdr3::TEXCOORD5:
		return 2048;
	case rdr3::TEXCOORD6:
		return 4096;
	case rdr3::TEXCOORD7:
		return 8192;
	case rdr3::TANGENT:
		return 16384;
	case rdr3::TANGENT1:
		return 32768;
	}

	return {};
}

enum class FVFTypeRdr
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

inline FVFTypeRdr mapType(rdr3::sgaBufferFormat type)
{
	switch (type)
	{
	case rdr3::R32_FLOAT:
		return FVFTypeRdr::Float;
	case rdr3::R32G32_FLOAT:
		return FVFTypeRdr::Float2;
	case rdr3::R32G32B32_FLOAT:
		return FVFTypeRdr::Float3;
	case rdr3::R32G32B32A32_FLOAT:
		return FVFTypeRdr::Float4;
	case rdr3::R8G8B8A8_UNORM:
		return FVFTypeRdr::Color;
	case rdr3::R10G10B10A2_UNORM:
		return FVFTypeRdr::Dec3N;
	}

	return FVFTypeRdr::Nothing;
}

template<>
five::grcVertexFormat* convert(rdr3::sgaInputLayout* format)
{
	/*int adjustment = 0;
	int sizeAdjustment = 0;

	// if it has specular
	if (format->GetMask() & (1 << 5))
	{
		adjustment -= 1;
		sizeAdjustment -= 4;
	}

	// if it has dec3n normal
	uint64_t fvf = format->GetFVF();

	if (format->GetMask() & (1 << 3))
	{
		FVFTypeRdr type = (FVFTypeRdr)((fvf >> (3 * 4)) & 0xF);

		if (type == FVFTypeRdr::Dec3N)
		{
			sizeAdjustment += 8;
		}
	}
	
	fvf = (fvf & ~(0xF << 12)) | (0x6 << 12);

	return new(false) five::grcVertexFormat(format->GetMask() & ~(1 << 5), format->GetVertexSize() + sizeAdjustment, format->GetFieldCount() + adjustment, fvf);*/

	uint64_t fvf = 0;
	uint16_t msk = 0;
	int numFields = 0;
	int totalSize = 0;

	for (int i = 0; i < std::size(format->sizes); i++)
	{
		if (format->sizes[i] != 0)
		{
			auto mapped = mapVertexSemantic((rdr3::sgaInputSemantic)i);

			if (mapped)
			{
				msk |= *mapped;

				DWORD firstBit;
				_BitScanForward(&firstBit, *mapped);

				auto t = mapType(format->types[i]);

				if (t == FVFTypeRdr::Dec3N)
				{
					t = FVFTypeRdr::Float3;
				}

				fvf |= ((uint64_t)t << (firstBit * 4));
				//totalSize += format->sizes[i];

				switch (t)
				{
				case FVFTypeRdr::Float4:
					totalSize += 4;
				case FVFTypeRdr::Float3:
					totalSize += 4;
				case FVFTypeRdr::Float2:
					totalSize += 4;
				case FVFTypeRdr::Nothing:
				case FVFTypeRdr::Float:
				case FVFTypeRdr::Float_unk:
					totalSize += 4;
					break;
				case FVFTypeRdr::Dec3N:
				{
					totalSize += 12;
					break;
				}
				case FVFTypeRdr::Color:
				case FVFTypeRdr::UByte4:
					totalSize += 4;
					break;
				case FVFTypeRdr::Float16_2:
					totalSize += 4;
					break;
				}

				numFields++;
			}
		}
	}

	// force for cw viewer, default
	/*msk = 0x00000059;
	totalSize = 0x24;
	numFields = 0x04;
	fvf = 0x7755555555996996;*/

	msk = 0x000042F9;
	totalSize = 0x48;
	numFields = 0x08;
	fvf = 0x7755555555996996;

	return new(false) five::grcVertexFormat(msk, totalSize, numFields, fvf);
}

template<>
five::grcVertexBufferD3D* convert(rdr3::grcVertexBufferD3D* buffer)
{
	// really hacky fix for multiple vertex buffers sharing the same array
	static std::map<void*, void*> g_vertexBufferMatches;

	void* oldBuffer = buffer->GetVertices();

	if (g_vertexBufferMatches.find(oldBuffer) != g_vertexBufferMatches.end())
	{
		oldBuffer = g_vertexBufferMatches[oldBuffer];
	}

	auto out = new(false) five::grcVertexBufferD3D;

	auto newVF = convert<five::grcVertexFormat*>(buffer->GetVertexFormat());
	out->SetVertexFormat(newVF);

	std::vector<char> vertexBuffer(2 * buffer->GetStride() * buffer->GetCount());

	auto oldVF = buffer->GetVertexFormat();
	char* oldData = (char*)oldBuffer;

	int newStride = newVF->GetVertexSize();
	int newMask = newVF->GetMask();

	std::map<int, std::optional<int>> fvfToSemantic;

	for (int s = 0; s < 52; s++)
	{
		auto mapped = mapVertexSemantic((rdr3::sgaInputSemantic)s);

		if (!mapped || !oldVF->sizes[s])
		{
			continue;
		}

		DWORD firstBit;
		_BitScanForward(&firstBit, *mapped);

		fvfToSemantic[firstBit] = s;
	}

	for (int i = 0; i < buffer->GetCount(); i++)
	{
		char* thisBit = &vertexBuffer[i * newStride];
		//char* oldBit = &oldData[i * buffer->GetStride()];
		//memcpy(thisBit, &oldData[i * buffer->GetStride()], buffer->GetStride());

		int off = 0;
		int newOff = 0;

		//for (int s = 0; s < 52; s++)
		for (int f = 0; f < 16; f++)
		{
			//if (!mapVertexSemantic((rdr3::sgaInputSemantic)s) || !oldVF->sizes[s])
			if (!fvfToSemantic[f] || (newVF->GetMask() & (1 << f)) == 0)
			{
				continue;
			}

			auto s = *fvfToSemantic[f];

			char* oldBit = &oldData[oldVF->offsets[s] + oldVF->sizes[s] * i];

			FVFTypeRdr type = mapType(oldVF->types[s]);

			off = 0;

			float fScale = 1.0f;

			if (f == 6)
			{
				fScale = 128.0f;
			}

			switch (type)
			{
			case FVFTypeRdr::Float4:
				*(float*)&thisBit[newOff] = *(float*)&oldBit[off];
				off += 4;
				newOff += 4;
			case FVFTypeRdr::Float3:
				*(float*)&thisBit[newOff] = *(float*)&oldBit[off];
				off += 4;
				newOff += 4;
			case FVFTypeRdr::Float2:
				*(float*)&thisBit[newOff] = *(float*)&oldBit[off] * fScale;
				off += 4;
				newOff += 4;
			case FVFTypeRdr::Nothing:
			case FVFTypeRdr::Float:
			case FVFTypeRdr::Float_unk:
				*(float*)&thisBit[newOff] = *(float*)&oldBit[off] * fScale;
				off += 4;
				newOff += 4;
				break;
			case FVFTypeRdr::Dec3N:
			{
				uint32_t N = *(uint32_t*)& oldBit[off];

				auto convertInt10 = [](int int10)
				{
					float value = 1.0f;

					if (int10 & 0x200)
					{
						int10 = ~((int10 & 0x1FF) - 1);
						int10 &= 0x1FF; // to remove the random top bytes that are set now

						value = -1.0f;
					}

					return value * (int10 / (511.0f));
				};

				int64_t nSafe = N;

				//float x = convertInt10((N >> 20) & 0x3FF);
				//float y = convertInt10((N >> 10) & 0x3FF);
				//float z = convertInt10((N >> 0) & 0x3FF);
				float x = float(((nSafe >> uint32_t(20)) & 0x3FF) - 512) / 511.0f;
				float y = float(((nSafe >> uint32_t(10)) & 0x3FF) - 512) / 511.0f;
				float z = float(((nSafe >> uint32_t( 0)) & 0x3FF) - 512) / 511.0f;

				// Y-UP AGAIN AAAA
				/**(float*)& thisBit[newOff] = z;
				*(float*)& thisBit[newOff + 4] = -x;
				*(float*)& thisBit[newOff + 8] = y; // swap hands*/

				*(float*)&thisBit[newOff] = z;
				*(float*)&thisBit[newOff + 4] = y;
				*(float*)&thisBit[newOff + 8] = x;

				newOff += 12;
				break;
			}
			case FVFTypeRdr::Color:
			case FVFTypeRdr::UByte4:
				*(uint32_t*)&thisBit[newOff] = *(uint32_t*)&oldBit[off];
				off += 4;
				newOff += 4;
				break;
			case FVFTypeRdr::Float16_2:
				*(uint16_t*)&thisBit[newOff] = *(uint16_t*)&oldBit[off];
				*(uint16_t*)&thisBit[newOff + 2] = *(uint16_t*)&oldBit[off + 2];

				off += 4;
				newOff += 4;
				break;
			default:
				FatalError("wat?");
			}

			if (f == 5) // color1
			{
				uint8_t* color0 = (uint8_t*)&thisBit[newOff - 8];
				uint8_t* color1 = (uint8_t*)&thisBit[newOff - 4];

				/*
				  // direct light
  r0     = v2;                           // temporal variable
  v2     = 0;                            // reset all setttings
  v2.a   = 1;                            // must be 1
  v2.r   = v1.g;                         // baked ssao?
  v2.gb  = 1 - (r0.r + r0.g + r0.b) / 3; // black to cyan
  v2.g   = saturate(v2.g + r0.b);        // blue to green
  v2.b   = saturate(v2.b + r0.g);        // green to blue
  v2.b   = saturate(v2.b - r0.g);        // reduce blue
  v2.g   = saturate(v2.g - r0.b);        // reduce green
  v2.rgb = saturate(v2.rgb - r0.r);      // red to black

  // indirect light
  r0   = v1;   // temporal variable
  v1   = 0;    // reset all setttings
  v1.a = 1;    // must be 1
  v1.r = r0.b; // ssao
  v1.g = 0;    // artificial lights
  v1.b = 0;    // dunno
  */

				uint8_t nc0[4] = { 0 };
				uint8_t nc1[4] = { 0 };

				auto clampColor = [](auto color) -> uint8_t
				{
					if (color > 255)
					{
						return 255;
					}

					if (color < 0)
					{
						return 0;
					}

					return (uint8_t)color;
				};

				nc1[3] = 255;
				nc1[0] = color0[1];
				nc1[1] = nc1[2] = uint8_t(255 - ((float)color1[0] + (float)color1[1] + (float)color1[2]) / 3.0f);
				nc1[1] = clampColor((int)nc1[1] + (int)color1[2]);
				nc1[2] = clampColor((int)nc1[2] + (int)color1[1]);
				nc1[2] = clampColor((int)nc1[2] - (int)color1[1]);
				nc1[1] = clampColor((int)nc1[1] - (int)color1[2]);
				nc1[0] = clampColor((int)nc1[0] - (int)color1[0]);
				nc1[1] = clampColor((int)nc1[1] - (int)color1[0]);
				nc1[2] = clampColor((int)nc1[2] - (int)color1[0]);

				nc0[3] = 255;
				nc0[0] = color0[2];
				nc0[1] = 0;
				nc0[2] = 0;

				*(uint32_t*)color0 = *(uint32_t*)nc0;
				*(uint32_t*)color1 = *(uint32_t*)nc1;

				//std::swap(color0[3], color1[0]);
				//color0[0] = 0xa0;
				//color0[1] = 0xa0;
				//color0[2] = 0xa0;

				//color1[0] = 0x0;
				//color1[3] = 0xFF;
				//color0[3] = color1[3];
				//color1[3] = 0;
			}

			if (f == 7)
			{
				float* tc0 = (float*)& thisBit[newOff - 16];
				float* tc1 = (float*)& thisBit[newOff - 8];

				//tc1[0] = tc0[0] / 128.0f;
				//tc1[1] = tc0[1] / 128.0f;
			}

			/*if (f == 0)
			{
				float* position = (float*)&thisBit[newOff - 12];

				std::swap(position[1], position[2]);
				position[1] = -position[1];
			}*/
		}

		if (off != buffer->GetStride())
		{
			//FatalError("wat..");
		}
	}

	out->SetVertices(buffer->GetCount(), newStride, &vertexBuffer[0]);

	g_vertexBufferMatches[buffer->GetVertices()] = out->GetVertices();

	return out;
}

template<>
five::grmGeometryQB* convert(rdr3::grmGeometry* geometry)
{
	auto out = new(false) five::grmGeometryQB;

	out->SetIndexBuffer(convert<five::grcIndexBufferD3D*>(geometry->GetIndexBuffer()));
	out->SetVertexBuffer(convert<five::grcVertexBufferD3D*>(geometry->GetVertexBuffer()));

	return out;
}

template<>
five::grmModel* convert(rdr3::grmModel* model)
{
	auto out = new(false) five::grmModel;
	int ni = 0;

	{
		auto& oldGeometries = model->GetGeometries();
		five::grmGeometryQB* geometries[256];

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

	//out->SetShaderMappings(/*model->GetShaderMappingCount()*/ni, model->GetShaderMappings());
	std::vector<uint16_t> fakeShaderMappings(ni); // defaults to be filled with 0s

	for (int i = 0; i < ni; i++)
	{
		fakeShaderMappings[i] = model->GetShaderMappings()[i];
	}

	out->SetShaderMappings(/*model->GetShaderMappingCount()*/ni, &fakeShaderMappings[0]);

	return out;
}

template<>
five::grmShaderGroup* convert(rdr3::grmShaderGroup* shaderGroup)
{
	auto out = new(false) five::grmShaderGroup;

	auto texDict = shaderGroup->GetTextures();

	if (texDict)
	{
		out->SetTextures(convert<five::pgDictionary<five::grcTexturePC>*>(texDict));
	}

	five::pgPtr<five::grmShaderFx> newShaders[512];

	for (int i = 0; i < shaderGroup->GetNumShaders(); i++)
	{
		auto newShader = new(false) five::grmShaderFx();
		//newShader->DoPreset("default", "default.sps");
		//newShader->DoPreset("terrain_cb_4lyr", "terrain_cb_4lyr.sps");
		newShader->DoPreset("terrain_cb_4lyr_2tex_blend", "terrain_cb_4lyr_2tex_blend.sps");
		//newShader->DoPreset("rdr3_terrain", "rdr3_terrain.sps");
		newShader->SetDrawBucket(0); // in case the lack of .sps reading doesn't override it, yet

		auto oldShader = shaderGroup->GetShader(i);

		if (!oldShader->m_textureRefs.IsNull())
		{
			auto texRef = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[1].GetValue();
			auto texRef2 = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[1 + 3].GetValue();
			auto texRef3 = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[1 + 3 + 3].GetValue();
			auto texRef4 = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[1 + 3 + 3 + 3].GetValue();
			//newShader->SetParameter("DiffuseSampler", texRef->GetName());

			if (texRef && texRef2 && texRef3 && texRef4)
			{
				newShader->SetParameter("TextureSampler_layer0", texRef->GetName());
				newShader->SetParameter("TextureSampler_layer1", texRef2->GetName());
				newShader->SetParameter("TextureSampler_layer2", texRef3->GetName());
				newShader->SetParameter("TextureSampler_layer3", texRef4->GetName());

				texRef = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[2].GetValue();
				texRef2 = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[2 + 3].GetValue();
				texRef3 = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[2 + 3 + 3].GetValue();
				texRef4 = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[2 + 3 + 3 + 3].GetValue();

				newShader->SetParameter("BumpSampler_layer0", texRef->GetName());
				newShader->SetParameter("BumpSampler_layer1", texRef2->GetName());
				newShader->SetParameter("BumpSampler_layer2", texRef3->GetName());
				newShader->SetParameter("BumpSampler_layer3", texRef4->GetName());

				/*texRef = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[3].GetValue();
				texRef2 = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[3 + 3].GetValue();
				texRef3 = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[3 + 3 + 3].GetValue();
				texRef4 = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[3 + 3 + 3 + 3].GetValue();

				newShader->SetParameter("heightMapSamplerLayer0", texRef->GetName());
				newShader->SetParameter("heightMapSamplerLayer1", texRef2->GetName());
				newShader->SetParameter("heightMapSamplerLayer2", texRef3->GetName());
				newShader->SetParameter("heightMapSamplerLayer3", texRef4->GetName());*/

				texRef = (rage::rdr3::grcTextureRef*)(*oldShader->m_textureRefs)[0].GetValue();
				newShader->SetParameter("lookupSampler", texRef->GetName());
			}
			else
			{
				newShader->SetParameter("TextureSampler_layer0", "nope");
				newShader->SetParameter("TextureSampler_layer1", "nope");
				newShader->SetParameter("TextureSampler_layer2", "nope");
				newShader->SetParameter("TextureSampler_layer3", "nope");

				newShader->SetParameter("BumpSampler_layer0", "nope");
				newShader->SetParameter("BumpSampler_layer1", "nope");
				newShader->SetParameter("BumpSampler_layer2", "nope");
				newShader->SetParameter("BumpSampler_layer3", "nope");
			}
		}
		else
		{
			//newShader->SetParameter("DiffuseSampler", "nope");
			newShader->SetParameter("TextureSampler_layer0", "nope");
			newShader->SetParameter("TextureSampler_layer1", "nope");
			newShader->SetParameter("TextureSampler_layer2", "nope");
			newShader->SetParameter("TextureSampler_layer3", "nope");

			newShader->SetParameter("BumpSampler_layer0", "nope");
			newShader->SetParameter("BumpSampler_layer1", "nope");
			newShader->SetParameter("BumpSampler_layer2", "nope");
			newShader->SetParameter("BumpSampler_layer3", "nope");
		}

		// TODO: change other arguments?
		newShaders[i] = newShader;
	}

	out->SetShaders(shaderGroup->GetNumShaders(), newShaders);

	return out;
}


five::grmShaderGroup* shaderGroup()
{
	auto out = new(false) five::grmShaderGroup;

	five::pgPtr<five::grmShaderFx> newShaders[1];

	auto newShader = new(false) five::grmShaderFx();
	newShader->DoPreset("default", "default.sps");
	newShader->SetDrawBucket(0); // in case the lack of .sps reading doesn't override it, yet

	newShader->SetParameter("DiffuseSampler", "nope");

	// TODO: change other arguments?
	newShaders[0] = newShader;

	out->SetShaders(1, newShaders);

	return out;
}
template<>
five::gtaDrawable* convert(rdr3::gtaDrawable* drawable)
{
	auto out = new(false) five::gtaDrawable;
	
	auto& oldLodGroup = drawable->GetLodGroup();

	out->SetBlockMap();

	out->SetShaderGroup(convert<five::grmShaderGroup*>(drawable->GetShaderGroup()));
	//out->SetShaderGroup(shaderGroup());

	auto& lodGroup = out->GetLodGroup();

	lodGroup.SetBounds(oldLodGroup.GetBoundsMin(), oldLodGroup.GetBoundsMax(), oldLodGroup.GetCenter(), oldLodGroup.GetRadius());

	for (int i = 0; i < 4; i++)
	{
		auto oldModel = oldLodGroup.GetModel(i);

		if (oldModel)
		{
			auto newModel = convert<five::grmModel*>(oldModel);

			lodGroup.SetModel(i, newModel);
			lodGroup.SetDrawBucketMask(i, newModel->CalcDrawBucketMask(out->GetShaderGroup())); // TODO: change this

			{
				auto oldBounds = oldModel->GetGeometryBounds();
				
				if (oldBounds)
				{
					int extraSize = 0;

					if (newModel->GetGeometries().GetCount() > 1)
					{
						extraSize = 1;
					}

					std::vector<five::GeometryBound> geometryBounds(newModel->GetGeometries().GetCount() + extraSize);
					memcpy(geometryBounds.data(), oldBounds, geometryBounds.size() * sizeof(five::GeometryBound));

					/*for (int i = 0; i < geometryBounds.size(); i++)
					{
						geometryBounds[i].aabbMin = Vector4(oldBounds[i].aabbMin.x, oldBounds[i].aabbMin.z, -oldBounds[i].aabbMin.y, oldBounds[i].aabbMin.w);
						geometryBounds[i].aabbMax = Vector4(oldBounds[i].aabbMax.x, oldBounds[i].aabbMax.z, -oldBounds[i].aabbMax.y, oldBounds[i].aabbMax.w);
					}*/

					newModel->SetGeometryBounds(geometryBounds.size(), geometryBounds.data());
				}
			}
		}
	}

	out->SetPrimaryModel();
	out->SetName(drawable->GetName());

	return out;
}
}
