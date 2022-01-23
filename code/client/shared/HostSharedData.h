#pragma once

#include <CL2LaunchMode.h>

template<typename TData>
class HostSharedData
{
public:
	HostSharedData(const std::string& name)
	{
		m_data = &m_fakeData;

		bool initTime = true;
		m_fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(TData), ToWide(fmt::sprintf("CFX_%s_%s_SharedData_%s", launch::GetLaunchModeKey(), launch::GetProductKey(), name)).c_str());

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

	HostSharedData(const HostSharedData&) = delete;
	HostSharedData(HostSharedData&& right)
	{
		m_fileMapping = std::move(right.m_fileMapping);
		m_data = std::move(right.m_data);
		m_fakeData = std::move(right.m_fakeData);
	}

	~HostSharedData()
	{
		if (m_data && m_data != &m_fakeData)
		{
			UnmapViewOfFile(m_data);
			m_data = nullptr;
		}

		if (m_fileMapping)
		{
			CloseHandle(m_fileMapping);
			m_fileMapping = nullptr;
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
