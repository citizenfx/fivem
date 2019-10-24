#include "StdInc.h"
#include "ToolComponentHelpers.h"

#include <ShaderInfo.h>

#include "IteratorView.h"

#include <boost/filesystem.hpp>

static void HandleArguments(boost::program_options::wcommand_line_parser& parser, std::function<void()> cb)
{
	boost::program_options::options_description desc;

	desc.add_options()
		("filename", boost::program_options::value<std::vector<boost::filesystem::path>>()->required(), "The path of the file to convert.");

	boost::program_options::positional_options_description positional;
	positional.add("filename", -1);

	parser.options(desc).
		positional(positional);

	cb();
}

static void Run(const boost::program_options::variables_map& map)
{
	if (map.count("filename") == 0)
	{
		printf("Usage:\n\n   fivem formats:exportShaders *.fxc...\n");
		return;
	}

	auto& entries = map["filename"].as<std::vector<boost::filesystem::path>>();

	for (auto& filePath : entries)
	{
		auto shaderFile = fxc::ShaderFile::Load(filePath.wstring());

		/*
		for (auto& shaderPair : shaderFile->GetVertexShaders())
		{
			const auto& name = shaderPair->GetName();
			const auto& data = shaderPair->GetShaderData();

			if (name.find("VS_TransformDeferred") == 0)
			{
				FILE* f = fopen("replace_vs.cso", "rb");
				fseek(f, 0, SEEK_END);

				int l = ftell(f);

				fseek(f, 0, SEEK_SET);

				std::vector<uint8_t> d(l);
				fread(d.data(), 1, d.size(), f);

				fclose(f);

				shaderPair->SetShaderData(d);
			}
		}
		*/

		for (auto& shaderPair : shaderFile->GetPixelShaders())
		{
			const auto& name = shaderPair->GetName();
			const auto& data = shaderPair->GetShaderData();

			FILE* f = fopen(va("%s.cso", name), "wb");
			fwrite(data.data(), 1, data.size(), f);
			fclose(f);
		}

		//shaderFile->Save(filePath.wstring() + L"_edited");
	}
}

static FxToolCommand command("formats:exportShaders", HandleArguments, Run);
