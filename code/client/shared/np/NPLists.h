// ==========================================================
// alterIWnet project
// 
// Component: xnp
// Sub-component: libnp
// Purpose: List data structure wrappers.
//
// Initial author: NTAuthority
// Started: 2013-04-15
// ==========================================================

#pragma once

#include <iterator>

struct NPKeyValuePair
{
	const char* key;
	const char* value;
};

class NPKeyValueIteratorInternal : public NPReferenceable
{
public:
	// operator implementation prototypes
	virtual NPKeyValuePair Dereference() = 0;

	virtual void Increment() = 0;

	virtual bool Equals(NPKeyValueIteratorInternal* right) = 0;
};

class NPKeyValueIterator : public std::iterator<std::forward_iterator_tag, NPKeyValuePair>
{
private:
	NPKeyValueIteratorInternal* iter;
public:
	inline NPKeyValueIterator(NPKeyValueIteratorInternal* i)
	{
		iter = i;
		iter->AddReference();
	}

	inline NPKeyValueIterator(NPKeyValueIterator& i)
	{
		iter = i.iter;
		iter->AddReference();
	}

	inline ~NPKeyValueIterator()
	{
		iter->Release();
		iter = nullptr;
	}

	inline void operator++()
	{
		iter->Increment();
	}

	inline NPKeyValuePair operator*()
	{
		return iter->Dereference();
	}

	inline NPKeyValuePair operator->()
	{
		return iter->Dereference();
	}

	inline bool operator==(NPKeyValueIterator right)
	{
		if (!iter)
		{
			return (!right.iter);
		}

		if (!right.iter)
		{
			return !iter;
		}

		return iter->Equals(right.iter);
	}

	inline bool operator!=(NPKeyValueIterator right)
	{
		return !(*this == right);
	}
};

class NPDictionaryInternal : public NPReferenceable
{
public:
	// gets an entry from the dictionary; or NULL if not found
	virtual const char* Get(const char* key) = 0;

	// sets an entry in the dictionary
	virtual void Set(const char* key, const char* value) = 0;

	// returns whether an entry exists in the dictionary
	virtual bool Has(const char* key) = 0;

	// returns a STL-style iterator for the dictionary
	virtual NPKeyValueIterator begin() = 0;
	virtual NPKeyValueIterator end() = 0;
};

// terribly hacky advance declaration
LIBNP_API NPDictionaryInternal* LIBNP_CALL NP_CreateDictionary();

class NPDictionary
{
private:
	NPDictionaryInternal* dictionary;

public:
	inline NPDictionary()
	{
		Init();
	}

	NPDictionary(NPDictionary& d)
	{
		dictionary = d.dictionary;
		dictionary->AddReference();
	}

	inline void AddRef()
	{
		dictionary->AddReference();
	}

	inline void Init()
	{
		dictionary = NP_CreateDictionary();
		dictionary->AddReference();
	}

	inline ~NPDictionary()
	{
		if (dictionary)
		{
			dictionary->Release();
			dictionary = nullptr;
		}
	}

	// gets an entry from the dictionary; or NULL if not found
	inline const char* Get(const char* key)
	{
		return dictionary->Get(key);
	}

	// sets an entry in the dictionary
	inline void Set(const char* key, const char* value)
	{
		dictionary->Set(key, value);
	}

	// returns whether an entry exists in the dictionary
	inline bool Has(const char* key)
	{
		return dictionary->Has(key);
	}

	// returns a STL-style iterator for the dictionary
	inline NPKeyValueIterator begin()
	{
		return dictionary->begin();
	}

	inline NPKeyValueIterator end()
	{
		return dictionary->end();
	}
};