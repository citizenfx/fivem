#pragma once

template<typename TData>
class HostSharedData
{
public:
	HostSharedData(const std::string& name)
	{
		m_data = &m_fakeData;

		bool initTime = true;
#ifdef IS_FXSERVER
		m_fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(TData), ToWide("CFX_SV_SharedData_" + name).c_str());
#else
		if (wcsstr(GetCommandLine(), L"cl2") != nullptr)
		{
			m_fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(TData), ToWide("CFX_CL2_SharedData_" + name).c_str());
		}
		else
		{
			m_fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(TData), ToWide("CFX_SharedData_" + name).c_str());
		}
#endif

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
