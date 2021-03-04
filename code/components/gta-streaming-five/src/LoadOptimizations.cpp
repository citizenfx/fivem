#include <StdInc.h>
#include <Hooking.h>

#include <atArray.h>
#include <atPool.h>
#include <Streaming.h>
#include <CrossBuildRuntime.h>
#include <CL2LaunchMode.h>

#include <tbb/combinable.h>
#include <tbb/parallel_for.h>

#include <unordered_set>

struct strStreamingInfoManager
{
	char pad[408];
	atArray<atArray<int>> dependentsArray;
};

template<typename T1, typename T2>
struct atPair
{
	T1 first;
	T2 second;
};

template<typename T>
struct atSet
{
	bool needsSorting;
	atArray<atPair<T, bool>> entries;

	atSet()
		: needsSorting(true)
	{
		
	}

	void EnsureSorted()
	{
		if (needsSorting)
		{
			if (entries.GetCount())
			{
				std::sort(entries.begin(), entries.end(), [](const auto& left, const auto& right)
				{
					return left.first < right.first;
				});
			}

			needsSorting = false;
		}
	}
};

static hook::cdecl_stub<void(strStreamingInfoManager*, int, atSet<uint32_t>*)> _getStrIndicesForArchive([]()
{
	return hook::get_call(hook::get_pattern("74 27 4C 8D 44 24 28 49 8B CD E8", 10));
});

static hook::cdecl_stub<void(void*, const uint32_t&, atArray<int>*& const)> _hashMapAdd([]()
{
	return hook::get_call(hook::get_pattern("74 27 4C 8D 44 24 28 49 8B CD E8", 36));
});

static hook::cdecl_stub<bool(void*, uint32_t)> _IsObjectInImage([]()
{
	return hook::get_pattern("74 20 8B C2 48 8B", -7);
});

static void CreateDependentsGraph(strStreamingInfoManager* self, atArray<int>& packfiles)
{
	if (packfiles.GetCount() == 0)
	{
		return;
	}

	self->dependentsArray.Expand(16);

	{
		atArray<int> dummyArray;
		self->dependentsArray.Set(self->dependentsArray.GetCount(), dummyArray);
	}

	auto& thisArray = self->dependentsArray[self->dependentsArray.GetCount() - 1];
	atSet<uint32_t> set;

	for (int packfile : packfiles)
	{
		if (packfile != -1)
		{
			// GetStrIndicesForArchive
			_getStrIndicesForArchive(self, packfile, &set);

			auto arrRef = &thisArray;

			// atHashMap setting
			_hashMapAdd((char*)self + 424, packfile, arrRef);
		}
	}

	// sort the set into a real one, please
	std::set<uint32_t> realSet;
	auto str = (streaming::Manager*)self;

	for (auto& entry : set.entries)
	{
		auto idx = entry.first;
		realSet.insert(idx);

		// is this even needed for *all of them*?! most are nullsub..
		auto strModule = str->moduleMgr.GetStreamingModule(idx);
		strModule->GetModelMapTypeIndex(idx - strModule->baseIdx, idx);

		thisArray.Set(thisArray.GetCount(), idx);
	}

	while (true)
	{
		tbb::combinable<std::set<uint32_t>> dependents;

		// for time
		// R* did a similar division in RDR3, each thread gets a chunk of streaming entries
		tbb::parallel_for(tbb::blocked_range<size_t>(0, str->numEntries), [self, str, &dependents, &realSet](const tbb::blocked_range<size_t>& r)
		{
			atSet<uint32_t> thisIter;

			for (size_t idx = r.begin(); idx != r.end(); idx++)
			{
				// IsObjectInImage
				if (_IsObjectInImage(self, idx))
				{
					auto module = str->moduleMgr.GetStreamingModule(idx);
					auto lidx = idx - module->baseIdx;
					
					uint32_t depList[48];
					// In FxDK this results in access violation error somehow
					int numDeps = module->GetDependencies(lidx, depList, std::size(depList));

					for (int di = 0; di < numDeps; di++)
					{
						uint32_t idx2 = idx;
						module->GetModelMapTypeIndex(lidx, idx2);

						// set contains
						if (realSet.find(depList[di]) != realSet.end())
						{
							// set insert
							dependents.local().insert(idx2);
						}
					}
				}
			}
		});

		// collect both sets
		std::set<uint32_t> bigSet;
		dependents.combine_each([&bigSet](const std::set<uint32_t>& right)
		{
			// let's violate const safety
			bigSet.merge(const_cast<std::set<uint32_t>&>(right));
		});

		dependents.clear();

		// if empty, break
		if (bigSet.empty())
		{
			break;
		}

		// add entries we found
		for (auto& idx : bigSet)
		{
			thisArray.Set(thisArray.GetCount(), idx);
		}

		// recurse! with all the entries from before
		realSet = std::move(bigSet);
	}
}

static void (*g_origInitAnim)();

static void AnimDirector_InitAfterMapLoaded(int why)
{
	if (why == 4)
	{
		// we want to load and add-ref a few streaming indices
		// original game code is horrible regarding this, does a *lot* of sequential LoadObjectsNow
		std::vector<std::tuple<streaming::strStreamingModule*, int>> indicesToReference;
		indicesToReference.reserve(512);

		// full stores we want to handle
		auto str = streaming::Manager::GetInstance();

		auto addOne = [str, &indicesToReference](streaming::strStreamingModule* module, int idx)
		{
			auto gidx = module->baseIdx + idx;

			if (_IsObjectInImage(str, gidx))
			{
				str->RequestObject(gidx, 7);
				indicesToReference.emplace_back(module, idx);
			}
		};

		auto handleFullStore = [str, &addOne](const char* storeExt)
		{
			auto module = str->moduleMgr.GetStreamingModule(storeExt);
			auto pool = (atPoolBase*)((char*)module + 56);

			for (int idx = 0; idx < pool->GetCountDirect(); idx++)
			{
				addOne(module, idx);
			}
		};

		handleFullStore("mrf");
		handleFullStore("ypdb");

		{
			auto module = str->moduleMgr.GetStreamingModule("yed");
			uint32_t idx = -1;
			module->FindSlot(&idx, "default");

			if (idx != -1)
			{
				addOne(module, idx);
			}
		}

		handleFullStore("yfd");

		streaming::LoadObjectsNow(false);

		for (auto [ module, lid ] : indicesToReference)
		{
			module->AddRef(lid);
			str->ReleaseObject(lid + module->baseIdx, 7);
		}

		// orig call
		g_origInitAnim();
	}
}

static HookFunction hookFunction([]()
{
	//hook::jump(hook::get_pattern("45 8D 70 01 66 39 B1 A2 01 00 00 74 41", -0x38), CreateDependentsGraph);

	if (!xbr::IsGameBuildOrGreater<2060>())
	{
		auto location = hook::get_pattern<char>("33 FF 44 8D 71 7C C1 E0 02", -0x1F);
		hook::set_call(&g_origInitAnim, location + 0x2D0);
		hook::jump(location, AnimDirector_InitAfterMapLoaded);
	}
});
