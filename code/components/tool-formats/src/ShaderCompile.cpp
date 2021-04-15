#include <StdInc.h>

#ifdef GTA_FIVE
#include "ToolComponentHelpers.h"

#include <ShaderInfo.h>

#include "IteratorView.h"
#include <d3dx11effect.h>

#include <d3dcompiler.h>
#include <boost/filesystem.hpp>

#include <wrl.h>

namespace WRL = Microsoft::WRL;

static void HandleArguments(boost::program_options::wcommand_line_parser& parser, std::function<void()> cb)
{
	boost::program_options::options_description desc;

	desc.add_options()("filename", boost::program_options::value<std::vector<boost::filesystem::path>>()->required(), "The path of the file to convert.");

	boost::program_options::positional_options_description positional;
	positional.add("filename", -1);

	parser.options(desc).positional(positional);

	cb();
}

#pragma comment(lib, "d3d11.lib")

static void Run(const boost::program_options::variables_map& map)
{
	if (map.count("filename") == 0)
	{
		printf("Usage:\n\n   fivem formats:compileShaders *.fxl...\n");
		return;
	}

	auto& entries = map["filename"].as<std::vector<boost::filesystem::path>>();

	WRL::ComPtr<ID3D11Device> device;
	D3D_FEATURE_LEVEL fl;
	WRL::ComPtr<ID3D11DeviceContext> ic;
	auto d3dDevice = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &device, &fl, &ic);

	auto compiler = LoadLibraryW(L"d3dcompiler_47.dll");
	static auto _D3DReflect = (decltype(&D3DReflect))GetProcAddress(compiler, "D3DReflect");

	for (auto& filePath : entries)
	{
		WRL::ComPtr<ID3DX11Effect> effect;
		if (FAILED(D3DX11CreateEffectFromFile(filePath.wstring().c_str(), 0, device.Get(), &effect)))
		{
			printf("failed\n");
			continue;
		}

		auto shaderFile = std::make_shared<fxc::ShaderFile>();

		D3DX11_EFFECT_DESC effectDesc;
		effect->GetDesc(&effectDesc);

		std::map<std::string, int> bindPoints;

		auto NullShader = std::make_shared<fxc::ShaderEntry>();
		NullShader->m_name = "NULL";

		shaderFile->GetVertexShaders().push_back(NullShader);
		shaderFile->GetPixelShaders().push_back(NullShader);
		shaderFile->m_geometryShaders.push_back(NullShader);
		shaderFile->m_hullShaders.push_back(NullShader);
		shaderFile->m_computeShaders.push_back(NullShader);
		shaderFile->m_domainShaders.push_back(NullShader);

		for (size_t tech = 0; tech < effectDesc.Techniques; tech++)
		{
			auto techD = effect->GetTechniqueByIndex(tech);
			D3DX11_TECHNIQUE_DESC techDesc;
			techD->GetDesc(&techDesc);

			auto techName = techDesc.Name ? techDesc.Name : "a";
			std::vector<fxc::TechniqueMapping> techData(techDesc.Passes);

			for (size_t passI = 0; passI < techDesc.Passes; passI++)
			{
				auto pass = techD->GetPassByIndex(passI);

				D3DX11_PASS_SHADER_DESC psd;
				D3DX11_PASS_SHADER_DESC vsd;
				D3DX11_PASS_SHADER_DESC gsd;
				pass->GetPixelShaderDesc(&psd);
				pass->GetVertexShaderDesc(&vsd);
				pass->GetGeometryShaderDesc(&gsd);

				auto processShader = [&bindPoints](auto& list, D3DX11_PASS_SHADER_DESC desc)
				{
					D3DX11_EFFECT_SHADER_DESC shaderD;
					desc.pShaderVariable->GetShaderDesc(0, &shaderD);

					D3DX11_EFFECT_VARIABLE_DESC d;
					desc.pShaderVariable->GetDesc(&d);

					int idx = list.size();
					auto se = std::make_shared<fxc::ShaderEntry>();
					se->m_name = d.Name;
					
					WRL::ComPtr<ID3D11ShaderReflection> refl;
					_D3DReflect(shaderD.pBytecode, shaderD.BytecodeLength, IID_PPV_ARGS(&refl));

					D3D11_SHADER_DESC rd;
					refl->GetDesc(&rd);

					for (size_t cb = 0; cb < rd.ConstantBuffers; cb++)
					{
						auto c = refl->GetConstantBufferByIndex(cb);
						D3D11_SHADER_BUFFER_DESC bd;
						c->GetDesc(&bd);

						D3D11_SHADER_INPUT_BIND_DESC b;
						refl->GetResourceBindingDescByName(bd.Name, &b);

						se->m_constantBuffers[b.BindPoint] = bd.Name;
						bindPoints[bd.Name] = b.BindPoint;
					}

					for (size_t rb = 0; rb < rd.BoundResources; rb++)
					{
						D3D11_SHADER_INPUT_BIND_DESC r;
						refl->GetResourceBindingDesc(rb, &r);

						if (r.Type == D3D_SIT_CBUFFER)
						{
							auto c = refl->GetConstantBufferByName(r.Name);
							D3D11_SHADER_BUFFER_DESC bd;
							c->GetDesc(&bd);

							for (size_t pi = 0; pi < bd.Variables; pi++)
							{
								auto v = c->GetVariableByIndex(pi);
								D3D11_SHADER_VARIABLE_DESC vd;
								v->GetDesc(&vd);

								se->m_arguments.push_back(vd.Name);
							}
						}
						else
						{
							bindPoints[r.Name] = r.BindPoint;
							se->m_arguments.push_back(r.Name);
						}
					}

					se->m_shaderData = { shaderD.pBytecode, shaderD.pBytecode + shaderD.BytecodeLength };
					list.push_back(se);

					return idx;
				};

				int vertexShader = processShader(shaderFile->GetVertexShaders(), vsd);
				int pixelShader = processShader(shaderFile->GetPixelShaders(), psd);
				int geometryShader = (gsd.pShaderVariable->IsValid()) ? processShader(shaderFile->m_geometryShaders, gsd) : 0;

				std::vector<uint8_t> d(7);
				d[0] = vertexShader;
				d[1] = pixelShader;
				// compute
				// domain
				d[4] = geometryShader;
				// hull

				fxc::TechniqueMapping tmap;
				tmap.SetRawData(d);
				techData[passI] = tmap;
			}

			shaderFile->GetTechniques()[techName] = techData;
		}

		std::map<std::string, int> cbs;
		
		for (size_t cb = 0; cb < effectDesc.ConstantBuffers; cb++)
		{
			auto c = effect->GetConstantBufferByIndex(cb);
			D3DX11_EFFECT_VARIABLE_DESC evDesc;
			c->GetDesc(&evDesc);

			D3DX11_EFFECT_TYPE_DESC td;
			c->GetType()->GetDesc(&td);

			auto cbuf = std::make_shared<fxc::ShaderConstantBuffer>();
			cbuf->m_name = evDesc.Name;

			*(uint32_t*)&cbuf->m_data[0] = td.UnpackedSize;

			// bind point per shader type
			// this assumes cb* register is the same across all of them!
			*(uint16_t*)&cbuf->m_data[4] = bindPoints[evDesc.Name];
			*(uint16_t*)&cbuf->m_data[6] = bindPoints[evDesc.Name];
			*(uint16_t*)&cbuf->m_data[8] = bindPoints[evDesc.Name];
			*(uint16_t*)&cbuf->m_data[10] = bindPoints[evDesc.Name];
			*(uint16_t*)&cbuf->m_data[12] = bindPoints[evDesc.Name];
			*(uint16_t*)&cbuf->m_data[14] = bindPoints[evDesc.Name];

			cbs[evDesc.Name] = shaderFile->GetLocalConstantBuffers().size();
			shaderFile->GetLocalConstantBuffers().push_back(cbuf);
		}

		size_t idx = 0xD6;

		for (size_t p = 0; p < effectDesc.GlobalVariables; p++)
		{
			auto var = effect->GetVariableByIndex(p);
			D3DX11_EFFECT_VARIABLE_DESC evd;
			var->GetDesc(&evd);

			auto param = std::make_shared<fxc::ShaderParameter>();
			int cbi = 0;

			if (auto cb = var->GetParentConstantBuffer(); cb)
			{
				D3DX11_EFFECT_VARIABLE_DESC evDesc;
				cb->GetDesc(&evDesc);

				param->m_constantBufferHash = HashString(evDesc.Name);
				cbi = bindPoints[evDesc.Name];
			}

			param->m_name = evd.Name;
			param->m_description = evd.Name;
			param->m_constantBufferOffset = evd.BufferOffset;
			param->m_register = idx;
			
			auto ty = var->GetType();
			D3DX11_EFFECT_TYPE_DESC tyd;
			ty->GetDesc(&tyd);

			// type:
			// 2: float
			// 3: float2
			// 4: float3
			// 5: float4
			// 6: sampler
			// 9: float4x4
			// 11: int

			if (tyd.Type == D3D_SVT_FLOAT)
			{
				if (tyd.Columns == 4 && tyd.Rows == 4)
				{
					param->m_type = (decltype(param->m_type))9;
				}
				else
				{
					param->m_type = (decltype(param->m_type))(2 + (tyd.Columns - 1));
				}
			}
			else if (tyd.Type == D3D_SVT_INT)
			{
				param->m_type = (decltype(param->m_type))11;
			}
			else if (tyd.Type == D3D_SVT_SAMPLER)
			{
				unsigned char a[40] = {
					0x05, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
					0x06, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
					0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00
				};

				param->m_defaultValue = std::vector<uint8_t>{
					(uint8_t*)a,
					(uint8_t*)a + sizeof(a)
				};
				param->m_type = (decltype(param->m_type))6;
				param->m_register = bindPoints[evd.Name];
				param->m_constantBufferOffset = bindPoints[evd.Name];
				param->m_constantBufferHash = 0;
			}
			else
			{
				continue;
			}

			param->m_unk = 2;
			param->m_size = tyd.Elements;
			
			for (size_t ann = 0; ann < evd.Annotations; ann++)
			{
				auto a = var->GetAnnotationByIndex(ann);

				D3DX11_EFFECT_VARIABLE_DESC evd;
				a->GetDesc(&evd);

				auto ty = a->GetType();
				D3DX11_EFFECT_TYPE_DESC tyd;
				ty->GetDesc(&tyd);

				if (tyd.Type == D3D_SVT_STRING)
				{
					LPCSTR s;
					a->AsString()->GetString(&s);

					param->m_annotations[evd.Name] = std::string{
						s
					};
				}
			}

			if (tyd.Type != D3D_SVT_SAMPLER)
			{
				param->m_defaultValue = std::vector<uint8_t>(tyd.PackedSize);
				param->m_unk = cbi;
				idx += std::max(tyd.PackedSize / 16, uint32_t(1));
			}

			shaderFile->GetLocalParameters()[evd.Name] = param;
		}

		auto outPath = filePath;
		outPath.replace_extension(".fxc");
		shaderFile->Save(outPath.wstring());
	}
}

static FxToolCommand command("formats:compileShaders", HandleArguments, Run);

#endif
