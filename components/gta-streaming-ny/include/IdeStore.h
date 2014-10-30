#pragma once

#include <ResourceCache.h>

struct CRect
{
	float fX1;
	float fY1;
	float fX2;
	float fY2;
};

struct CVector2D
{
	float x;
	float y;
};

class CQuadTreeNode
{
private:
	char m_pad[40];

public:
	CQuadTreeNode(const CRect& rectangle, int numSubNodes);

	void* operator new(size_t size);

	void ForAllMatching(const CVector2D& vector, void(*callback)(const CVector2D&, void*));

	void AddItem(void* item, const CRect& rect);
};

class IdeFile
{
private:
	CRect m_boundingBox;

	fwString m_fileName;

	fwString m_bigFileName;

	// only valid during loading, actually
	std::set<uint16_t> m_modelIndices;

	bool m_loaded;

	bool m_loading;

	bool m_required;

	int m_ideIdx;

	int m_size;

	char* m_textBuffer;

	int m_streamItemIdx;

public:
	IdeFile(fwString fileName, int size, int ideIdx);

	void AddToBounds(uint16_t mi, const CRect& rect);

	inline CRect GetBounds() { return m_boundingBox; }

	inline fwString GetFileName() { return m_fileName; }

	void Request();

	void WaitForRequest();

	void Delete();

	void DoLoad();

	inline void AddModelIndex(uint16_t mi) { m_modelIndices.insert(mi); }

	inline bool IsRequired() { return m_required; }

	inline void SetRequired(bool value) { m_required = value; }

	inline int GetStreamItemIdx() { return m_streamItemIdx; }
};

class CIdeStore
{
	friend class IdeFile;

private:
	static CQuadTreeNode* ms_pQuadTree;

	static IdeFile* ms_registeredIdes[2048];

	static int ms_ideNum;

	static fwVector<IdeFile*> ms_relatedIdes;

	static fwMap<uint32_t, StreamingResource> ms_drawables;

private:
	static void EnqueueRequestBegin(IdeFile* ideFile);

	static void EnqueueRequestCompletion(IdeFile* ideFile);

public:
	// sets up the quad tree and such
	static void Initialize();

	// called during IplStore load, will set the current IDE name list (and load them from the local cache + initialize it)
	static void SetIdeRelationBegin(const char* ideName);

	// called from CEntity::Add(const CRect&), will add the current object's bounds to the IdeFile containing said item (using the temporary model index)
	static void SetIdePoint(uint16_t curMI, const CRect& rect);

	// called to close SetIdeRelationBegin
	static void SetIdeRelationEnd();

	// waits for current pending requests to complete
	static void LoadAllRequestedArchetypes();

	// checks if any archetype request is pending
	static bool CanWeEvenLoadAnyIpls();

	// checks if any additional archetype can be requested
	static bool CanLoadAdditionalIde();

	// load IDEs for a position
	static void LoadForPosn(const CVector2D& vector);

	// process
	static void Process();

	// register an IDE
	static int RegisterIde(const char* fileName, uint32_t size);

	// register a streaming-only drawable
	static void RegisterDrawable(const StreamingResource& entry);

	// clear the IDE/drawable list
	static inline void ResetStreamList()
	{
		ms_drawables.clear();
		ms_ideNum = 0;
	}
};

// NOTE: RAGE SHARED
template<typename TValue>
class atArray
{
private:
	TValue* m_offset;
	uint16_t m_count;
	uint16_t m_size;

public:
	atArray()
	{
		m_offset = (TValue*)0xDEADC0DE;
		m_count = 0;
		m_size = 0;
	}

	atArray(int capacity)
	{
		m_offset = new TValue[capacity];
		m_count = 0;
		m_size = capacity;
	}

	inline uint16_t GetCount()
	{
		return m_count;
	}

	inline uint16_t GetSize()
	{
		return m_size;
	}

	TValue& Get(uint16_t offset)
	{
		if (offset >= m_count)
		{
			FatalError("atArray index out of bounds");
		}

		return m_offset[offset];
	}

	void Expand(uint16_t newSize)
	{
		if (m_size >= newSize)
		{
			return;
		}

		TValue* newOffset = (TValue*)rage::GetAllocator()->allocate(newSize * sizeof(TValue), 16, 0);
		std::copy(m_offset, m_offset + m_count, newOffset);

		rage::GetAllocator()->free(m_offset);

		m_offset = newOffset;
	}

	void Set(uint16_t offset, const TValue& value)
	{
		if (offset >= m_size)
		{
			Expand(offset + 1);
		}

		if (offset >= m_count)
		{
			m_count = offset + 1;
		}

		m_offset[offset] = value;
	}
};