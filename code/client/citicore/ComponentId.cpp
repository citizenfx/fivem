/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"
#include <sstream>

ComponentId::ComponentId()
{
	memset(m_versions, 0, sizeof(m_versions));
}

const std::string& ComponentId::GetCategory(size_t idx) const
{
	static std::string emptyString = "";

	if (idx < 0 || idx >= m_categories.size())
	{
		return emptyString;
	}

	return m_categories[idx];
}

int ComponentId::CompareVersion(const ComponentId& secondId) const
{
	for (int i = 0; i < _countof(m_versions); i++)
	{
		if (m_versions[i] < secondId.m_versions[i])
		{
			return -1;
		}
		else if (m_versions[i] > secondId.m_versions[i])
		{
			return 1;
		}
	}

	return 0;
}

bool ComponentId::IsMatchedBy(const ComponentId& provider) const
{
	// check categories
	for (size_t i = 0; i < m_categories.size(); i++)
	{
		if (provider.GetCategory(i) != GetCategory(i))
		{
			return false;
		}
	}

	// check version equal/higher
	return CompareVersion(provider) <= 0;
}

std::string ComponentId::GetString() const
{
	std::stringstream ss;

	ss << GetCategory(0);

	for (size_t i = 1; i < m_categories.size(); i++)
	{
		ss << ":" << m_categories[i];
	}

	ss << "[" << m_versions[0];

	for (int i = 1; i < _countof(m_versions); i++)
	{
		ss << "." << m_versions[i];
	}

	ss << "]";

	return ss.str();
}

ComponentId ComponentId::Parse(const char* str)
{
	enum
	{
		PARSE_ID,
		PARSE_VERSION
	} parseState = PARSE_ID;

	const char* p = str;
	char tempBuf[256];
	char* tempP = tempBuf;
	int vI = 0;

	ComponentId id;

	while (true)
	{
		switch (parseState)
		{
			case PARSE_ID:
				if (*p == ':' || *p == '[' || *p == '\0')
				{
					*tempP = '\0';

					id.m_categories.push_back(tempBuf);

					if (*p == '[')
					{
						parseState = PARSE_VERSION;
					}

					tempP = tempBuf;
				}
				else
				{
					*tempP = *p;
					tempP++;
				}

				break;

			case PARSE_VERSION:
				if (*p == '.' || *p == ']')
				{
					*tempP = '\0';

					if (vI < _countof(id.m_versions))
					{
						id.m_versions[vI] = atoi(tempBuf);

						vI++;
					}

					tempP = tempBuf;
				}
				else
				{
					*tempP = *p;
					tempP++;
				}
				break;
		}

		if (*p == '\0')
		{
			break;
		}

		p++;
	}

	return id;
}