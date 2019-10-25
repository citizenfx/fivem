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

#include <array>
#include <memory>
#include <variant>

#include <boost/optional.hpp>

#include <variant>

namespace fxc
{
	typedef std::function<size_t(void*, size_t)> TReader;
	typedef std::function<void(const void*, size_t)> TWriter;

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

	inline void WriteString(const TWriter& writer, const std::string& string)
	{
		uint8_t length = string.length() + 1;
		writer(&length, 1);

		writer(string.c_str(), length);
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

		void Write(const TWriter& writer, int);

		inline const std::string& GetName()
		{
			return m_name;
		}

		inline const std::vector<uint8_t>& GetShaderData()
		{
			return m_shaderData;
		}

		inline void SetShaderData(const std::vector<uint8_t>& data)
		{
			m_shaderData = data;
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
		uint8_t m_unk;
		uint8_t m_size;
		std::vector<uint8_t> m_defaultValue;

		uint32_t m_constantBufferHash;
		uint32_t m_constantBufferOffset;

		std::string m_constantBufferName;

		std::map<std::string, std::variant<std::string, int, float>> m_annotations;

	public:
		ShaderParameter(const TReader& reader);

		void Write(const TWriter& writer);

		inline const std::string& GetName()
		{
			return m_name;
		}

		inline uint32_t GetNameHash()
		{
			return HashString(m_name.c_str());
		}

		inline const std::string& GetConstantBufferName()
		{
			return m_constantBufferName;
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

		std::vector<uint8_t> m_rawData;
	
	public:
		inline TechniqueMapping()
		{

		}

		inline TechniqueMapping(const std::string& vs, const std::string& ps, const std::vector<uint8_t>& rawData)
			: m_vertexShaderName(vs), m_pixelShaderName(ps), m_rawData(rawData)
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

		inline const std::vector<uint8_t>& GetRawData()
		{
			return m_rawData;
		}
	};

	class FORMATS_EXPORT ShaderConstantBuffer
	{
	private:
		std::string m_name;
		std::array<uint8_t, 16> m_data;

	public:
		inline ShaderConstantBuffer()
		{

		}

		inline ShaderConstantBuffer(const std::string& name, const std::array<uint8_t, 16>& data)
			: m_name(name), m_data(data)
		{

		}

		inline const std::string& GetName() const
		{
			return m_name;
		}

		inline const std::array<uint8_t, 16>& GetData() const
		{
			return m_data;
		}
	};

	class FORMATS_EXPORT ShaderFile
	{
	private:
		std::map<std::string, std::variant<std::string, int, float>> m_preValues;
		std::vector<std::shared_ptr<ShaderEntry>> m_vertexShaders;
		std::vector<std::shared_ptr<ShaderEntry>> m_pixelShaders;
		std::vector<std::shared_ptr<ShaderEntry>> m_computeShaders;
		std::vector<std::shared_ptr<ShaderEntry>> m_domainShaders;
		std::vector<std::shared_ptr<ShaderEntry>> m_hullShaders;
		std::vector<std::shared_ptr<ShaderEntry>> m_geometryShaders;

		std::map<std::string, std::shared_ptr<ShaderParameter>> m_globalParameters;
		std::map<std::string, std::shared_ptr<ShaderParameter>> m_localParameters;

		std::vector<std::shared_ptr<ShaderConstantBuffer>> m_globalConstantBuffers;
		std::vector<std::shared_ptr<ShaderConstantBuffer>> m_localConstantBuffers;

		std::map<std::string, std::vector<TechniqueMapping>> m_techniqueMappings;

	public:
		ShaderFile(const TReader& reader);

		inline ~ShaderFile() {}

		void Save(const TWriter& writer);

		void Save(const std::wstring& filename);

		// returns the shader names in file order
		std::vector<std::string> ReadShaders(const TReader& reader, int type, std::vector<std::shared_ptr<ShaderEntry>>& list);

		void WriteShaders(const TWriter& writer, int type, const std::vector<std::shared_ptr<ShaderEntry>>& list);

		static std::shared_ptr<ShaderFile> Load(const std::wstring& filename);

		inline std::map<std::string, std::variant<std::string, int, float>>& GetGlobalValues()
		{
			return m_preValues;
		}

		inline boost::optional<std::variant<std::string, int, float>> GetGlobalValue(const std::string& key)
		{
			auto it = m_preValues.find(key);
			boost::optional<std::variant<std::string, int, float>> retval;

			if (it != m_preValues.end())
			{
				retval = it->second;
			}

			return retval;
		}

		inline std::vector<std::shared_ptr<ShaderEntry>>& GetPixelShaders()
		{
			return m_pixelShaders;
		}

		inline std::vector<std::shared_ptr<ShaderEntry>>& GetVertexShaders()
		{
			return m_vertexShaders;
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

	enum class SpsParameterType
	{
		Int,
		Float,
		String
	};

	class SpsParameter
	{
	private:
		SpsParameterType m_type;
		std::variant<int, float, std::string> m_value;

	public:
		inline SpsParameter()
		{
			m_value = "";
			m_type = SpsParameterType::String;
		}

		inline SpsParameter(int value)
		{
			m_value = value;
			m_type = SpsParameterType::Int;
		}

		inline SpsParameter(float value)
		{
			m_value = value;
			m_type = SpsParameterType::Float;
		}

		inline SpsParameter(const std::string& value)
		{
			m_value = value;
			m_type = SpsParameterType::String;
		}

		inline float GetFloat()
		{
			assert(m_type == SpsParameterType::Float);

			return std::get<float>(m_value);
		}

		inline int GetInt()
		{
			assert(m_type == SpsParameterType::Int);

			return std::get<int>(m_value);
		}

		inline std::string GetString()
		{
			assert(m_type == SpsParameterType::String);

			return std::get<std::string>(m_value);
		}
	};

	class FORMATS_EXPORT SpsFile
	{
	private:
		std::map<std::string, SpsParameter> m_parameters;

	public:
		SpsFile(const TReader& reader);

		inline ~SpsFile() {}

		inline boost::optional<SpsParameter> GetParameter(const std::string& name)
		{
			auto it = m_parameters.find(name);

			if (it == m_parameters.end())
			{
				return {};
			}

			return it->second;
		}

		inline std::string GetShader()
		{
			return m_parameters["shader"].GetString();
		}

		static std::shared_ptr<SpsFile> Load(const std::wstring& filename);
	};
}
