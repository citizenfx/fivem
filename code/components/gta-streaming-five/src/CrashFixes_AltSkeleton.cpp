#include <StdInc.h>
#include <Hooking.h>
#include <EntitySystem.h>

struct fwAltSkeletonExtension
{
	void* vtbl;
	rage::fwEntity* entity;
	float m1[4];
	float m2[4];
	float m3[4];
	float m4[4];

	void Init(rage::fwEntity* entity)
	{
		this->entity = entity;
		memset(m1, 0, sizeof(m1));
		memset(m2, 0, sizeof(m2));
		memset(m3, 0, sizeof(m3));
		memset(m4, 0, sizeof(m4));
		m1[0] = m2[1] = m3[2] = 1.0f;

		entity->AddExtension((rage::fwExtension*)this);
	}
};

static uint32_t* fwAltSkeletonExtension_id;
static uint32_t* fwAltSkeletonExtension_ct;
static fwAltSkeletonExtension* fwAltSkeletonExtension_pl;

static fwAltSkeletonExtension* fwAltSkeletonExtension__Add(rage::fwEntity* entity)
{
	fwAltSkeletonExtension* extension;

	if (extension = (fwAltSkeletonExtension*)entity->GetExtension(*fwAltSkeletonExtension_id); !extension)
	{
		if (*fwAltSkeletonExtension_ct < 16)
		{
			auto selfExt = &fwAltSkeletonExtension_pl[*fwAltSkeletonExtension_ct];
			(*fwAltSkeletonExtension_ct)++;

			selfExt->Init(entity);
			extension = selfExt;
		}
	}

	return extension;
}

static HookFunction hookFunction([]()
{
	auto location = hook::get_call(hook::get_pattern<char>("48 8B CB E8 ? ? ? ? 48 89 44 24 60 48 85 C0", 3));
	fwAltSkeletonExtension_id = hook::get_address<decltype(fwAltSkeletonExtension_id)>(location + 0x11);
	fwAltSkeletonExtension_ct = hook::get_address<decltype(fwAltSkeletonExtension_ct)>(location + 0x2C);
	fwAltSkeletonExtension_pl = hook::get_address<decltype(fwAltSkeletonExtension_pl)>(location + 0x40);
	hook::jump(location, fwAltSkeletonExtension__Add);
});
