#pragma once

class ProgramArguments
{
public:
#ifdef _MSC_VER
	using TCharType = char32_t;
#else
	// on Linux, wchar_t is 32 bit, and char32_t isn't supported
	using TCharType = wchar_t;
#endif

private:
	std::vector<std::string> m_arguments;

public:
	ProgramArguments(int argc, char** argv);

	inline explicit ProgramArguments(const std::vector<std::string>& arguments)
		: m_arguments(arguments)
	{
		
	}

	inline explicit ProgramArguments(const std::vector<std::basic_string<TCharType>>& arguments)
	{
		// see comments about this workaround in Console.cpp
#ifndef _MSC_VER
		static std::wstring_convert<std::codecvt_utf8<TCharType>, TCharType> converter;
#else
		static std::wstring_convert<std::codecvt_utf8<uint32_t>, uint32_t> converter;
#endif

		m_arguments.resize(arguments.size());

		for (size_t i = 0; i < arguments.size(); i++)
		{
			try
			{
				typename decltype(converter)::wide_string tempString(arguments[i].begin(), arguments[i].end());

				m_arguments[i] = converter.to_bytes(tempString);
			}
			catch (std::range_error& e)
			{

			}
		}
	}

	template<typename... Args>
	inline explicit ProgramArguments(Args... args)
	{
		m_arguments = std::vector<std::string>{args...};
	}

	inline const std::string& Get(int i) const
	{
		assert(i >= 0 && i < m_arguments.size());

		return m_arguments[i];
	}

	inline const std::string& operator[](int i) const
	{
		return Get(i);
	}

    inline const std::string& operator[](size_t i) const
    {
        assert(i < m_arguments.size());

        return m_arguments[i];
    }

	inline size_t Count() const
	{
		return m_arguments.size();
	}

	inline std::string Shift()
	{
		std::string value = *(m_arguments.begin());
		m_arguments.erase(m_arguments.begin());

		return value;
	}

	inline const std::vector<std::string>& GetArguments() const
	{
		return m_arguments;
	}
};
