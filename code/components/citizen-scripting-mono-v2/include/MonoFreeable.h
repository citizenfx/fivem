#pragma once

#include <string>

#include <mono/metadata/object.h>

namespace fx::mono
{
// wrapper to mono_free() the created resources
template<typename T>
class Freeable
{
private:
	T* value;

public:
	Freeable(T* value)
		: value(value)
	{
	}

	~Freeable()
	{
		mono_free(const_cast<T*>(value));
	}

	T* operator->() const
	{
		return value;
	}

	T* operator*() const
	{
		return value;
	}

	inline char* data() const
	{
		return value;
	}

	inline operator T*() const
	{
		return value;
	}
};

class UTF8CString
{
private:
	Freeable<char> _data;

public:
	UTF8CString(MonoString* str)
		: _data(mono_string_to_utf8(str))
	{
	}

	inline char* data() const
	{
		return _data;
	}

	inline operator char*() const
	{
		return _data;
	}

	inline operator std::string() const
	{
		return std::string(_data);
	}
};
}
