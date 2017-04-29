/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_RAGE_FORMATS_X
#define FORMATS_EXPORT DLL_EXPORT
#else
#define FORMATS_EXPORT DLL_IMPORT
#endif

#include <memory>

#include <boost/optional.hpp>

namespace fxc
{
	typedef std::function<size_t(void*, size_t)> TReader;

	inline std::string ReadString(const TReader& reader)
	{
		uint8_t length = 0;
		reader(&length, 1);

		if (length > 0)
		{
			std::vector<char> str(length);

			reader(&str[0], str.size());

			return std::string(&str[0]);
		}
		else
		{
			return "";
		}
	}

	class ShaderEntry
	{
	private:
		std::string m_name;
		std::vector<std::string> m_arguments;
		std::map<uint16_t, std::string> m_constantBuffers;

		std::vector<uint8_t> m_shaderData;

	public:
		ShaderEntry(const TReader& reader, int);

		inline const std::string& GetName()
		{
			return m_name;
		}

		inline const std::vector<uint8_t>& GetShaderData()
		{
			return m_shaderData;
		}
	};

	enum class ShaderParameterType : uint8_t
	{

	};

	class ShaderParameter
	{
	private:
		std::string m_name;
		std::string m_description;
		ShaderParameterType m_type;
		uint8_t m_register;
		std::vector<uint8_t> m_defaultValue;

		uint32_t m_constantBufferHash;
		uint32_t m_constantBufferOffset;

		std::string m_constantBufferName;

		std::map<std::string, std::string> m_annotations;

	public:
		ShaderParameter(const TReader& reader);

		inline const std::string& GetName()
		{
			return m_name;
		}

		inline uint32_t GetNameHash()
		{
			return HashString(m_name.c_str());
		}

		inline void SetConstantBufferName(const std::string& value)
		{
			m_constantBufferName = value;
		}

		inline uint32_t GetConstantBuffer()
		{
			return m_constantBufferHash;
		}

		inline const std::string& GetDescription()
		{
			return m_description;
		}

		inline uint8_t GetRegister()
		{
			return m_register;
		}

		inline ShaderParameterType GetType()
		{
			return m_type;
		}

		inline bool IsSampler()
		{
			return (uint8_t)m_type == 6;
		}

		inline const std::vector<uint8_t>& GetDefaultValue()
		{
			return m_defaultValue;
		}
	};

	class FORMATS_EXPORT TechniqueMapping
	{
	private:
		std::string m_vertexShaderName;
		std::string m_pixelShaderName;
	
	public:
		inline TechniqueMapping()
		{

		}

		inline TechniqueMapping(const std::string& vs, const std::string& ps)
			: m_vertexShaderName(vs), m_pixelShaderName(ps)
		{

		}

		inline const std::string& GetVertexShader()
		{
			return m_vertexShaderName;
		}

		inline const std::string& GetPixelShader()
		{
			return m_pixelShaderName;
		}
	};

	class FORMATS_EXPORT ShaderFile
	{
	private:
		std::map<std::string, std::string> m_preValues;
		std::map<std::string, std::shared_ptr<ShaderEntry>> m_vertexShaders;
		std::map<std::string, std::shared_ptr<ShaderEntry>> m_pixelShaders;
		std::map<std::string, std::shared_ptr<ShaderEntry>> m_computeShaders;
		std::map<std::string, std::shared_ptr<ShaderEntry>> m_domainShaders;
		std::map<std::string, std::shared_ptr<ShaderEntry>> m_hullShaders;
		std::map<std::string, std::shared_ptr<ShaderEntry>> m_geometryShaders;

		std::map<std::string, std::shared_ptr<ShaderParameter>> m_globalParameters;
		std::map<std::string, std::shared_ptr<ShaderParameter>> m_localParameters;

		std::map<std::string, TechniqueMapping> m_techniqueMappings;

	public:
		ShaderFile(const TReader& reader);

		inline ~ShaderFile() {}

		// returns the shader names in file order
		std::vector<std::string> ReadShaders(const TReader& reader, int type, std::map<std::string, std::shared_ptr<ShaderEntry>>& list);

		static std::shared_ptr<ShaderFile> Load(const std::wstring& filename);

		inline std::map<std::string, std::string>& GetGlobalValues()
		{
			return m_preValues;
		}

		inline boost::optional<std::string> GetGlobalValue(const std::string& key)
		{
			auto it = m_preValues.find(key);
			boost::optional<std::string> retval;

			if (it != m_preValues.end())
			{
				retval = it->second;
			}

			return retval;
		}

		inline std::map<std::string, std::shared_ptr<ShaderEntry>>& GetPixelShaders()
		{
			return m_pixelShaders;
		}

		inline std::map<std::string, std::shared_ptr<ShaderParameter>>& GetLocalParameters()
		{
			return m_localParameters;
		}

		inline uint8_t MapRegister(uint8_t inReg)
		{
			uint8_t minReg = 0xFF;

			for (auto arg : m_localParameters)
			{
				if (!arg.second->IsSampler())
				{
					auto reg = arg.second->GetRegister();

					if (reg < minReg)
					{
						minReg = reg;
					}
				}
			}

			return (inReg - minReg) + 0xA0;
		}
	};
}