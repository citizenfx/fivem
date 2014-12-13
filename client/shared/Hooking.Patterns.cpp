#include "StdInc.h"
#include "Hooking.Patterns.h"
#include <cstdint>
#include <sstream>

// from boost someplace
template <std::uint64_t FnvPrime, std::uint64_t OffsetBasis>
struct basic_fnv_1
{
	std::uint64_t operator()(std::string const& text) const
	{
		std::uint64_t hash = OffsetBasis;
		for (std::string::const_iterator it = text.begin(), end = text.end();
			 it != end; ++it)
		{
			hash *= FnvPrime;
			hash ^= *it;
		}

		return hash;
	}
};

const std::uint64_t fnv_prime = 1099511628211u;
const std::uint64_t fnv_offset_basis = 14695981039346656037u;

typedef basic_fnv_1<fnv_prime, fnv_offset_basis> fnv_1;

namespace hook
{
static std::multimap<uint64_t, uintptr_t> g_hints;

static void TransformPattern(const std::string& pattern, std::string& data, std::string& mask)
{
	std::stringstream dataStr;
	std::stringstream maskStr;

	uint8_t tempDigit = 0;
	bool tempFlag = false;

	for (auto& ch : pattern)
	{
		if (ch == ' ')
		{
			continue;
		}
		else if (ch == '?')
		{
			dataStr << '\x00';
			maskStr << '?';
		}
		else if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F'))
		{
			char str[] = { ch, 0 };
			int thisDigit = strtol(str, nullptr, 16);

			if (!tempFlag)
			{
				tempDigit = (thisDigit << 4);
				tempFlag = true;
			}
			else
			{
				tempDigit |= thisDigit;
				tempFlag = false;

				dataStr << tempDigit;
				maskStr << 'x';
			}
		}
	}

	data = dataStr.str();
	mask = maskStr.str();
}

class executable_meta
{
private:
	uintptr_t m_begin;
	uintptr_t m_end;

public:
	executable_meta()
		: m_begin(0), m_end(0)
	{

	}

	void EnsureInit()
	{
		if (m_begin)
		{
			return;
		}

		m_begin = reinterpret_cast<uintptr_t>(getRVA<void>(0));

		PIMAGE_DOS_HEADER dosHeader = getRVA<IMAGE_DOS_HEADER>(0);
		PIMAGE_NT_HEADERS ntHeader = getRVA<IMAGE_NT_HEADERS>(dosHeader->e_lfanew);

		m_end = m_begin + ntHeader->OptionalHeader.SizeOfCode;
	}

	inline uintptr_t begin() { return m_begin; }
	inline uintptr_t end()   { return m_end; }
};

void pattern::Initialize(const char* pattern, size_t length)
{
	// get the hash for the base pattern
	std::string baseString(pattern, length);
	uint64_t hash = fnv_1()(baseString);

	// transform the base pattern from IDA format to canonical format
	TransformPattern(baseString, m_bytes, m_mask);

	m_size = m_mask.size();

	// if there's hints, try those first
	auto range = g_hints.equal_range(hash);

	if (range.first != range.second)
	{
		std::for_each(range.first, range.second, [&] (const std::pair<uint64_t, uintptr_t>& hint)
		{
			ConsiderMatch(hint.second);
		});

		// if the hints succeeded, we don't need to do anything more
		if (m_matches.size() > 0)
		{
			return;
		}
	}

	// scan the executable for code
	static executable_meta executable;

	executable.EnsureInit();

	for (uintptr_t i = executable.begin(); i <= executable.end(); i++)
	{
		if (ConsiderMatch(i))
		{
#if !defined(COMPILING_SHARED_LIBC)
			Citizen_PatternSaveHint(hash, i);
#endif
			g_hints.insert(std::make_pair(hash, i));
		}
	}
}

bool pattern::ConsiderMatch(uintptr_t offset)
{
	const char* pattern = m_bytes.c_str();
	const char* mask = m_mask.c_str();

	char* ptr = reinterpret_cast<char*>(offset);

	for (size_t i = 0; i < m_size; i++)
	{
		if (mask[i] == '?')
		{
			continue;
		}

		if (pattern[i] != ptr[i])
		{
			return false;
		}
	}

	m_matches.push_back(pattern_match(ptr));

	return true;
}

void pattern::hint(uint64_t hash, uintptr_t address)
{
	g_hints.insert(std::make_pair(hash, address));
}
}

static InitFunction initFunction([] ()
{
	std::wstring hintsFile = MakeRelativeCitPath(L"citizen\\hints.dat");
	FILE* hints = _wfopen(hintsFile.c_str(), L"rb");
	
	if (hints)
	{
		while (!feof(hints))
		{
			uint64_t hash;
			uintptr_t hint;

			fread(&hash, 1, sizeof(hash), hints);
			fread(&hint, 1, sizeof(hint), hints);

			hook::pattern::hint(hash, hint);
		}

		fclose(hints);
	}
});