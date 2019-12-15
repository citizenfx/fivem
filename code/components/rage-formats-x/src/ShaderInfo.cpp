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
				std::variant<std::string, int, float> annotationValue;

				uint8_t annotationType;
				reader(&annotationType, sizeof(annotationType));

				if (annotationType == 0 || annotationType == 1)
				{
					int value;
					reader(&value, sizeof(value));

					annotationValue = value;
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
				auto readArguments = [&] (std::vector<std::shared_ptr<ShaderConstantBuffer>>& constantBufferList, std::map<std::string, std::shared_ptr<ShaderParameter>>& list)
				{
					uint8_t numConstantBuffers;
					reader(&numConstantBuffers, sizeof(numConstantBuffers));

					std::map<uint32_t, std::string> constantBuffers;

					for (int i = 0; i < numConstantBuffers; i++)
					{
						std::array<uint8_t, 16> unkData;
						reader(unkData.data(), unkData.size());

						auto name = ReadString(reader);

						constantBuffers.insert({ HashString(name.c_str()), name });

						constantBufferList.push_back(std::make_shared<ShaderConstantBuffer>(name, unkData));
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

				readArguments(m_globalConstantBuffers, m_globalParameters);
				readArguments(m_localConstantBuffers, m_localParameters);
			}

			{
				uint8_t numTechniques;

				reader(&numTechniques, sizeof(numTechniques));

				for (int i = 0; i < numTechniques; i++)
				{
					auto techniqueName = ReadString(reader);

					uint8_t numPasses;
					reader(&numPasses, 1);

					std::vector<TechniqueMapping> passes;

					for (int j = 0; j < numPasses; j++)
					{
						std::vector<uint8_t> techniqueIndices(7);
						reader(techniqueIndices.data(), techniqueIndices.size());

						if (techniqueIndices[6] > 0)
						{
							techniqueIndices.resize(7 + (techniqueIndices[6] * 8));
							reader(&techniqueIndices[7], techniqueIndices.size() - 7);
						}

						passes.emplace_back(vertexShaders[techniqueIndices[0]], pixelShaders[techniqueIndices[1]], techniqueIndices);
					}

					m_techniqueMappings[techniqueName] = std::move(passes);
				}
			}
		}
	}

	ShaderParameter::ShaderParameter(const TReader& reader)
	{
		struct  
		{
			uint8_t type;
			uint8_t size;
			uint8_t reg;
			uint8_t unk;
		} intro;

		reader(&intro, sizeof(intro));

		m_type = (fxc::ShaderParameterType)intro.type;
		m_size = intro.size;
		m_register = intro.reg;
		m_unk = intro.unk;

		m_name = ReadString(reader);
		m_description = ReadString(reader);

		reader(&m_constantBufferOffset, sizeof(m_constantBufferOffset));
		reader(&m_constantBufferHash, sizeof(m_constantBufferHash));

		uint8_t numAnnotations;
		reader(&numAnnotations, sizeof(numAnnotations));

		for (int i = 0; i < numAnnotations; i++)
		{
			std::string annotationName = ReadString(reader);
			std::variant<std::string, int, float> annotationValue;

			uint8_t annotationType;
			reader(&annotationType, sizeof(annotationType));

			if (annotationType == 0 || annotationType == 1)
			{
				int value;
				reader(&value, sizeof(value));

				annotationValue = value;
			}
			else if (annotationType == 2)
			{
				annotationValue = ReadString(reader);
			}

			m_annotations.insert({ annotationName, annotationValue });
		}

		uint8_t valueLength;
		reader(&valueLength, sizeof(valueLength));

		m_defaultValue.resize(valueLength * 4);

		if (valueLength > 0)
		{
			reader(&m_defaultValue[0], valueLength * 4);
		}
	}

	void ShaderParameter::Write(const TWriter& writer)
	{
		struct
		{
			uint8_t type;
			uint8_t size;
			uint8_t reg;
			uint8_t unk;
		} intro;

		intro.type = (uint8_t)m_type;
		intro.size = m_size;
		intro.reg = m_register;
		intro.unk = m_unk;

		writer(&intro, sizeof(intro));

		WriteString(writer, m_name);
		WriteString(writer, m_description);

		writer(&m_constantBufferOffset, sizeof(m_constantBufferOffset));
		writer(&m_constantBufferHash, sizeof(m_constantBufferHash));

		uint8_t numAnnotations = m_annotations.size();
		writer(&numAnnotations, sizeof(numAnnotations));

		for (auto& preValue : m_annotations)
		{
			WriteString(writer, preValue.first);

			// string?
			if (preValue.second.index() == 0)
			{
				uint8_t annotationType = 2;
				writer(&annotationType, 1);

				WriteString(writer, std::get<std::string>(preValue.second));
			}
			else if (preValue.second.index() == 1)
			{
				uint8_t annotationType = 0;
				writer(&annotationType, 1);

				int value = std::get<int>(preValue.second);
				writer(&value, sizeof(value));
			}
		}

		uint8_t valueLength = m_defaultValue.size() / 4;
		writer(&valueLength, sizeof(valueLength));

		if (valueLength > 0)
		{
			writer(&m_defaultValue[0], valueLength * 4);
		}
	}

	std::vector<std::string> ShaderFile::ReadShaders(const TReader& reader, int type, std::vector<std::shared_ptr<ShaderEntry>>& list)
	{
		std::vector<std::string> retval;

		uint8_t numShaders;
		reader(&numShaders, 1);

		for (int i = 0; i < numShaders; i++)
		{
			auto entry = std::make_shared<ShaderEntry>(reader, type);
			auto& name = entry->GetName();

			retval.push_back(name);
			list.push_back(entry);
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

	void ShaderEntry::Write(const TWriter& writer, int type)
	{
		WriteString(writer, m_name);

		uint8_t numArguments = m_arguments.size();
		writer(&numArguments, 1);

		for (int i = 0; i < numArguments; i++)
		{
			WriteString(writer, m_arguments[i]);
		}

		uint8_t numConstantBuffers = m_constantBuffers.size();
		writer(&numConstantBuffers, 1);

		for (auto& cbuf : m_constantBuffers)
		{
			WriteString(writer, cbuf.second);
			writer(&cbuf.first, 2);
		}

		if (type == 2)
		{
			// TODO: is this not a temp var?
			numConstantBuffers = 0;

			writer(&numConstantBuffers, 1);
		}

		uint32_t shaderSize = m_shaderData.size();
		writer(&shaderSize, sizeof(shaderSize));

		if (shaderSize > 0)
		{
			writer(&m_shaderData[0], m_shaderData.size());

			if (type != 0)
			{
				uint16_t ui = 0;
				writer(&ui, 2);
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
			return fread(data, 1, size, f);
		});

		fclose(f);

		return shaderFile;
	}

	void ShaderFile::Save(const std::wstring& filename)
	{
		FILE* f = _wfopen(filename.c_str(), L"wb");

		if (!f)
		{
			return;
		}

		Save([=](const void* data, size_t size)
		{
			fwrite(data, 1, size, f);
		});

		fclose(f);
	}

	void ShaderFile::Save(const TWriter& writer)
	{
		uint32_t magic = 0x65786772;
		writer(&magic, sizeof(magic));

		magic = 1;
		writer(&magic, sizeof(magic));

		uint8_t numPreStrings = m_preValues.size();
		writer(&numPreStrings, 1);

		for (auto& preValue : m_preValues)
		{
			WriteString(writer, preValue.first);

			// string?
			if (preValue.second.index() == 0)
			{
				uint8_t annotationType = 2;
				writer(&annotationType, 1);

				WriteString(writer, std::get<std::string>(preValue.second));
			}
			else if (preValue.second.index() == 1)
			{
				uint8_t annotationType = 0;
				writer(&annotationType, 1);

				int value = std::get<int>(preValue.second);
				writer(&value, sizeof(value));
			}
		}

		WriteShaders(writer, 1, m_vertexShaders);
		WriteShaders(writer, 1, m_pixelShaders);
		WriteShaders(writer, 0, m_computeShaders);
		WriteShaders(writer, 0, m_domainShaders);
		WriteShaders(writer, 2, m_geometryShaders);
		WriteShaders(writer, 0, m_hullShaders);

		{
			auto writeArguments = [&](const decltype(m_localConstantBuffers)& cbList, const std::map<std::string, std::shared_ptr<ShaderParameter>>& list)
			{
				uint8_t numConstantBuffers = cbList.size();
				writer(&numConstantBuffers, 1);

				for (auto& cb : cbList)
				{
					writer(cb->GetData().data(), cb->GetData().size());

					WriteString(writer, cb->GetName());
				}

				uint8_t numArguments = list.size();
				writer(&numArguments, sizeof(numArguments));

				for (const auto& arg : list)
				{
					arg.second->Write(writer);
				}
			};

			writeArguments(m_globalConstantBuffers, m_globalParameters);
			writeArguments(m_localConstantBuffers, m_localParameters);
		}

		{
			uint8_t numTechniques = m_techniqueMappings.size();

			writer(&numTechniques, sizeof(numTechniques));

			for (auto& tech : m_techniqueMappings)
			{
				WriteString(writer, tech.first);

				uint8_t numPasses = tech.second.size();
				writer(&numPasses, 1);

				for (auto& pass : tech.second)
				{
					auto rd = pass.GetRawData();
					writer(rd.data(), rd.size());
				}
			}
		}
	}

	void ShaderFile::WriteShaders(const TWriter& writer, int type, const std::vector<std::shared_ptr<ShaderEntry>>& list)
	{
		uint8_t numShaders = list.size();
		writer(&numShaders, 1);

		for (auto& shaderPair : list)
		{
			shaderPair->Write(writer, type);
		}
	}

	SpsFile::SpsFile(const TReader& reader)
	{
		char c;

		enum
		{
			// whitespace before a key
			PsInitial,
			// a key
			PsKey,
			// whitespace before a value
			PsPreValue,
			// a plain value
			PsValue,
			// whitespace before a type
			PsPreType,
			// a type
			PsType,
			// whitespace after a type
			PsPreTypeValue,
			// a typed value
			PsTypeValue,
			// whitespace after a type value
			PsPostTypeValue,
			// a closing brace
			PsCloseBrace

		} parsingState = PsInitial;

		std::string curKey;
		std::string curValue;
		std::string curType;

		bool eof = false;

		do
		{
			eof = (reader(&c, 1) == 0);

			if (eof)
			{
				c = '\n';
			}

			switch (parsingState)
			{
			case PsInitial:
				if (!isspace(c))
				{
					parsingState = PsKey;
					curKey = "";
				}
				else
				{
					break;
				}
			case PsKey:
				if (isspace(c))
				{
					parsingState = PsPreValue;
				}
				else
				{
					curKey += c;
				}
				break;
			case PsPreValue:
				if (!isspace(c))
				{
					if (c == '{')
					{
						parsingState = PsPreType;
						break;
					}
					else
					{
						parsingState = PsValue;
						curValue = "";
					}
				}
				else
				{
					// next char
					break;
				}
			case PsValue:
				if (isspace(c))
				{
					m_parameters[curKey] = curValue;

					parsingState = PsInitial;
				}
				else
				{
					curValue += c;
				}
				break;
			case PsPreType:
				if (!isspace(c))
				{
					parsingState = PsType;
					curType = "";
				}
				else
				{
					// next char
					break;
				}
			case PsType:
				if (isspace(c))
				{
					parsingState = PsPreTypeValue;
				}
				else
				{
					curType += c;
				}

				break;
			case PsPreTypeValue:
				if (!isspace(c))
				{
					parsingState = PsTypeValue;
					curValue = "";
				}
				else
				{
					// next char
					break;
				}
			case PsTypeValue:
				if (isspace(c))
				{
					parsingState = PsPostTypeValue;
				}
				else
				{
					curValue += c;
				}

				break;
			case PsPostTypeValue:
				if (c == '}')
				{
					if (curType == "int")
					{
						m_parameters[curKey] = atoi(curValue.c_str());
					}
					else if (curType == "float")
					{
						m_parameters[curKey] = float(atof(curValue.c_str()));
					}

					parsingState = PsInitial;
				}

				break;
			}
		} while (!eof);
	}

	std::shared_ptr<SpsFile> SpsFile::Load(const std::wstring& filename)
	{
		FILE* f = _wfopen(filename.c_str(), L"rb");

		if (!f)
		{
			return nullptr;
		}

		auto spsFile = std::make_shared<SpsFile>([=](void* data, size_t size)
		{
			return fread(data, 1, size, f);
		});

		fclose(f);

		return spsFile;
	}
}
