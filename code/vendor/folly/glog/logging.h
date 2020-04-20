#pragma once

class GlogDummy
{
public:
	template<typename T>
	const GlogDummy& operator<<(const T& r) const
	{
		return *this;
	}
};

#define CHECK_GE(x, y)
#define CHECK(x) GlogDummy()
#define CHECK_EQ(x, y)
#define DCHECK(x)
#define DCHECK_EQ(x, y)