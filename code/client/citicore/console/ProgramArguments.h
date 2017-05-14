#pragma once

class ProgramArguments
{
private:
	std::vector<std::string> m_arguments;

public:
	ProgramArguments(int argc, char** argv);

	inline explicit ProgramArguments(const std::vector<std::string>&& arguments)
		: m_arguments(arguments)
	{
		
	}

	template<typename... Args>
	explicit ProgramArguments(Args... args)
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

	std::string Shift()
	{
		std::string value = *(m_arguments.begin());
		m_arguments.erase(m_arguments.begin());

		return value;
	}
};
