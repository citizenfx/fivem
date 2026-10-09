#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Patterns.h>
#include <Hooking.Stubs.h>

#include <EntitySystem.h>
#include <netObject.h>

//
// GH-4083: a malicious client is able to leave a cloned vehicle with a fwEntity::m_archetype
// that no longer holds a usable pointer, which crashes every nearby player.
//
// The reported faulting instruction (GTA5_b3258.exe+F2BC09) sits in the CVehicle method at
// vtable slot 0x498 and is an unguarded archetype dereference:
//
//     mov rax, [rbx+20h]      ; CVehicle::m_archetype
//     mov ecx, [rax+588h]     ; <- faults
//
// The reported call stack reaches it exclusively through the clone branch of the vehicle net
// object's per-frame update, i.e. only for vehicles owned by another player:
//
//     <game update> -> netObject vtable slot 0x360   (b3258: +11AB95C)
//                   -> clone update helper           (b3258: +11AC9EC, taken when IsRemote())
//                   -> CVehicle method               (b3258: +F8B354)
//                   -> CVehicle vtable slot 0x498    (b3258: +F2BA20, faults at +F2BC09)
//
// In the minidump attached to the issue m_archetype holds 0x41FF44DB1DA91950. That address is
// non-canonical, so the access raises #GP and gets reported as an access violation at
// 0xFFFFFFFFFFFFFFFF: the field contains garbage rather than null, which is why none of the
// game's own null checks catch it. Everything else about the vehicle is still intact at that
// point - the frames above successfully walked its vtable, wheel array, seat manager and audio
// entity - so a single field is being clobbered rather than the object being freed.
//
// The corrupting write itself could not be identified, because FiveM's default minidump type
// does not capture heap memory and the vehicle's own bytes are therefore absent from the
// report. Until it is found, refuse to run the network update for an entity whose archetype
// cannot possibly be dereferenced, instead of letting the game fault on it.
//

// x64 user-mode addresses live in the low canonical half of the address space. A value outside
// of it can never be a real archetype: it is either null, a small value mistaken for a pointer
// or - as in this report - non-canonical garbage. This deliberately does not try to prove that
// the pointer *is* an archetype, only that dereferencing it is defined behaviour.
static constexpr uintptr_t kMinUserAddress = 0x10000;
static constexpr uintptr_t kMaxUserAddress = 0x800000000000;

static constexpr bool IsDereferenceableAddress(uintptr_t value)
{
	return value >= kMinUserAddress && value < kMaxUserAddress;
}

// the value m_archetype held in the GH-4083 crash dump. It is specifically *not* null, so a
// plain null check - which is what this would otherwise be simplified into - does not help
static_assert(!IsDereferenceableAddress(0x41FF44DB1DA91950), "non-canonical pointers must be rejected");
static_assert(!IsDereferenceableAddress(0), "null must be rejected");
static_assert(IsDereferenceableAddress(0x1B110B66B10), "an ordinary heap address must be accepted");

static bool IsDereferenceableArchetype(rage::fwArchetype* archetype)
{
	return IsDereferenceableAddress(reinterpret_cast<uintptr_t>(archetype));
}

static bool (*g_origVehicleNetObjectUpdate)(rage::netObject*);

static bool VehicleNetObjectUpdate(rage::netObject* self)
{
	auto entity = reinterpret_cast<rage::fwEntity*>(self->GetGameObject());

	if (entity && !IsDereferenceableArchetype(entity->GetArchetype()))
	{
		// this means something corrupted the entity, so report it - but only once, as the
		// condition persists for as long as the entity does and this runs every frame
		static bool warned = false;

		if (!warned)
		{
			warned = true;

			trace("%s has an unusable archetype (%p), skipping its network update. Further occurrences will not be logged. [GH-4083]\n",
				self->ToString(), (void*)entity->GetArchetype());
		}

		// the original only ever returns true, so the caller keeps treating this object
		// exactly as it did before
		return true;
	}

	return g_origVehicleNetObjectUpdate(self);
}

static HookFunction hookFunction([]()
{
	// vehicle netObject vtable slot 0x360, anchored on its whole prologue up to and including
	// the 'mov rsi, rcx / call <clone update> / mov rdi, [rsi+50h] / test rdi, rdi' head, so
	// that a match can only ever be the start of a function. Frame offset, frame size and the
	// call displacement are wildcarded.
	//
	// Verified to match exactly once on b3258. Installed defensively because the signature
	// could not be validated against the other supported game builds.
	if (auto pattern = hook::pattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 55 48 8D 68 ? 48 81 EC ? ? ? ? 48 8B F1 E8 ? ? ? ? 48 8B 7E 50 48 85 FF 0F 84"); pattern.size() == 1)
	{
		g_origVehicleNetObjectUpdate = hook::trampoline(pattern.get(0).get<void>(), VehicleNetObjectUpdate);
	}
	else
	{
		trace("Couldn't find the vehicle net object update - the GH-4083 archetype guard is not active.\n");
	}
});
