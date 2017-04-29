/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ShaderInfo.h"

namespace fxc
{
	ShaderFile::ShaderFile(const TReader& reader)
	{
		uint32_t magic;
		reader(&magic, sizeof(magic));

		if (magic == 0x65786772) // rgxe
		{
			reader(&magic, sizeof(magic));

			uint8_t numPreStrings;
			reader(&numPreStrings, 1);

			for (int i = 0; i < numPreStrings; i++)
			{
				std::string annotationName = ReadString(reader);
				std::string annotationValue;

				uint8_t annotationType;
				reader(&annotationType, sizeof(annotationType));

				if (annotationType == 0 || annotationType == 1)
				{
					uint32_t value;
					reader(&value, sizeof(value));

					annotationValue = std::to_string(value);
				}
				else if (annotationType == 2)
				{
					annotationValue = ReadString(reader);
				}

				m_preValues[annotationName] = annotationValue;
			}

			auto vertexShaders = ReadShaders(reader, 1, m_vertexShaders);
			auto pixelShaders = ReadShaders(reader, 1, m_pixelShaders);
			ReadShaders(reader, 0, m_computeShaders);
			ReadShaders(reader, 0, m_domainShaders);
			ReadShaders(reader, 2, m_geometryShaders);
			ReadShaders(reader, 0, m_hullShaders);

			{
				auto readArguments = [&] (std::map<std::string, std::shared_ptr<ShaderParameter>>& list)
				{
					uint8_t numConstantBuffers;
					reader(&numConstantBuffers, sizeof(numConstantBuffers));

					std::map<uint32_t, std::string> constantBuffers;

					for (int i = 0; i < numConstantBuffers; i++)
					{
						uint8_t unkData[16];
						reader(unkData, sizeof(unkData));

						auto name = ReadString(reader);

						constantBuffers.insert({ HashString(name.c_str()), name });
					}

					uint8_t numArguments;
					reader(&numArguments, sizeof(numArguments));

					for (int i = 0; i < numArguments; i++)
					{
						auto argument = std::make_shared<ShaderParameter>(reader);
						argument->SetConstantBufferName(constantBuffers[argument->GetConstantBuffer()]);

						list.insert({ argument->GetName(), argument });
					}
				};

				readArguments(m_globalParameters);
				readArguments(m_localParameters);
			}

			{
				uint8_t numTechniques;

				reader(&numTechniques, sizeof(numTechniques));

				for (int i = 0; i < numTechniques; i++)
				{
					auto techniqueName = ReadString(reader);

					uint8_t techniqueIndices[8];
					reader(techniqueIndices, sizeof(techniqueIndices));

					m_techniqueMappings[techniqueName] = TechniqueMapping{ vertexShaders[techniqueIndices[1]], pixelShaders[techniqueIndices[2]] };

					if (techniqueIndices[7] > 0)
					{
						std::vector<uint8_t> unk(techniqueIndices[7] * 8);
						reader(&unk[0], unk.size());
					}
				}
			}
		}
	}

	ShaderParameter::ShaderParameter(const TReader& reader)
	{
		struct  
		{
			uint8_t type;
			uint8_t pad;
			uint8_t reg;
			uint8_t unk;
		} intro;

		reader(&intro, sizeof(intro));

		m_type = (fxc::ShaderParameterType)intro.type;
		m_register = intro.reg;

		m_name = ReadString(reader);
		m_description = ReadString(reader);

		reader(&m_constantBufferOffset, sizeof(m_constantBufferOffset));
		reader(&m_constantBufferHash, sizeof(m_constantBufferHash));

		uint8_t numAnnotations;
		reader(&numAnnotations, sizeof(numAnnotations));

		for (int i = 0; i < numAnnotations; i++)
		{
			std::string annotationName = ReadString(reader);
			std::string annotationValue;

			uint8_t annotationType;
			reader(&annotationType, sizeof(annotationType));

			if (annotationType == 0 || annotationType == 1)
			{
				uint32_t value;
				reader(&value, sizeof(value));

				annotationValue = std::to_string(value);
			}
			else if (annotationType == 2)
			{
				annotationValue = ReadString(reader);
			}

			m_annotations.insert({ annotationName, annotationValue });
		}

		uint8_t valueLength;
		reader(&valueLength, sizeof(valueLength));

		m_defaultValue.resize(max(valueLength * 4, 16));

		if (valueLength > 0)
		{
			reader(&m_defaultValue[0], valueLength * 4);
		}
	}

	std::vector<std::string> ShaderFile::ReadShaders(const TReader& reader, int type, std::map<std::string, std::shared_ptr<ShaderEntry>>& list)
	{
		std::vector<std::string> retval;

		uint8_t numShaders;
		reader(&numShaders, 1);

		for (int i = 0; i < numShaders; i++)
		{
			auto entry = std::make_shared<ShaderEntry>(reader, type);
			auto& name = entry->GetName();

			retval.push_back(name);
			list[name] = entry;
		}

		return retval;
	}

	ShaderEntry::ShaderEntry(const TReader& reader, int type)
	{
		m_name = ReadString(reader);

		uint8_t numArguments;
		reader(&numArguments, 1);

		for (int i = 0; i < numArguments; i++)
		{
			m_arguments.push_back(ReadString(reader));
		}

		uint8_t numConstantBuffers;
		reader(&numConstantBuffers, 1);

		for (int i = 0; i < numConstantBuffers; i++)
		{
			std::string name = ReadString(reader);

			uint16_t index;
			reader(&index, sizeof(index));

			m_constantBuffers[index] = name;
		}

		if (type == 2)
		{
			reader(&numConstantBuffers, 1);
		}

		uint32_t shaderSize;
		reader(&shaderSize, sizeof(shaderSize));

		if (shaderSize > 0)
		{
			m_shaderData.resize(shaderSize);
			reader(&m_shaderData[0], m_shaderData.size());

			if (type != 0)
			{
				uint16_t ui;
				reader(&ui, 2);
			}
		}
	}

	std::shared_ptr<ShaderFile> ShaderFile::Load(const std::wstring& filename)
	{
		FILE* f = _wfopen(filename.c_str(), L"rb");

		if (!f)
		{
			return nullptr;
		}

		auto shaderFile = std::make_shared<ShaderFile>([=] (void* data, size_t size)
		{
			int a = ftell(f);

			return fread(data, 1, size, f);
		});

		fclose(f);

		return shaderFile;
	}
}