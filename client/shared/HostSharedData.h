#pragma once

template<typename TData>
class HostSharedData
{
public:
	HostSharedData(const std::string& name)
	{
		m_data = &m_fakeData;

		bool initTime = true;
		m_fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(TData), ToWide("CFX_SharedData_" + name).c_str());

		if (m_fileMapping != nullptr)
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				initTime = false;
			}

			m_data = (TData*)MapViewOfFile(m_fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TData));

			if (initTime)
			{
				m_data = new(m_data) TData();
			}
		}
	}

	inline TData& operator*()
	{
		return *m_data;
	}

	inline TData* operator->()
	{
		return m_data;
	}

private:
	HANDLE m_fileMapping;
	TData* m_data;

	TData m_fakeData;
};
