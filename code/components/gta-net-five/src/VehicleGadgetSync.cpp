#include <StdInc.h>

#include <ICoreGameInit.h>

#include <Hooking.h>
#include <Hooking.FlexStruct.h>
#include <Hooking.Stubs.h>

#include <netObject.h>

#include <algorithm>
#include <array>
#include <cmath>

// The game syncs CVehicleGadgetForks as the slide's position (capped at 1.7m) and the acceleration.
// With game_enableVehicleGadgetSyncFix, it syncs each joint's 0-1 target between its limits (slide and tilt) and the acceleration instead.

extern ICoreGameInit* icgi;

extern rage::netObject* GetNetObjectFromEntity(void* entity);

// JointInfo
struct VehicleGadgetJoint
{
	struct PrismaticInfo
	{
		float position;
		float desiredPosition;
		float openPosition;
		float closedPosition;
		bool onlyControlSpeed;
	};

	struct RevoluteInfo
	{
		int32_t rotationAxis;
		float angle;
		float desiredAngle;
		float openAngle;
		float closedAngle;
		bool onlyControlSpeed;
		bool snapToAngle;
	};

	int32_t fragChild;
	int32_t bone;
	float desiredAcceleration;
	float speed;
	float prevPosition;
	bool prismatic;
	bool continuousInput;
	uint8_t pad_16[0x1A];
	bool swingFreely;
	uint8_t pad_31[3];

	union
	{
		PrismaticInfo prismaticJoint;
		RevoluteInfo revoluteJoint;
	};

	uint8_t pad_4C[4];
};

static_assert(sizeof(VehicleGadgetJoint) == 0x50);
static_assert(offsetof(VehicleGadgetJoint, prismatic) == 0x14);
static_assert(offsetof(VehicleGadgetJoint, prismaticJoint) == 0x34);

// CVehicleGadgetWithJointsBase
class VehicleGadgetWithJoints : public hook::FlexStruct
{
public:
	inline VehicleGadgetJoint* GetJoint(int16_t index)
	{
		if (index >= Get<int16_t>(0x10))
		{
			return nullptr;
		}

		auto joints = Get<VehicleGadgetJoint*>(0x18);
		return joints ? &joints[index] : nullptr;
	}
};

// rage::CSyncDataBase
class SyncDataSerialiser : public hook::FlexStruct
{
public:
	inline bool IsReader()
	{
		return Get<uint8_t>(8) == 1;
	}

	inline void SerialiseUnsigned(uint32_t& value, int bits)
	{
		CallVirtual<void>(0x68, &value, bits, (const char*)nullptr);
	}
};

static constexpr int kAccelerationBits = 8;
static constexpr int kRatioBits = 12;
static constexpr uint32_t kRatioMax = (1u << kRatioBits) - 1;
static constexpr float kAccelerationScale = 127.0f;
static constexpr float kJointSleepThreshold = 0.05f;

// CVehicleGadgetForks::WantsToBeAwake
static constexpr float kForksSleepAccelerationThreshold = 0.001f;

static bool IsVehicleGadgetSyncFixActive()
{
	return icgi->OneSyncEnabled && icgi->HasVariable("vehicleGadgetSyncFix");
}

static bool GetJointLimits(VehicleGadgetJoint* joint, float& closed, float& open)
{
	if (!joint || joint->bone < 0)
	{
		return false;
	}

	closed = joint->prismatic ? joint->prismaticJoint.closedPosition : joint->revoluteJoint.closedAngle;
	open = joint->prismatic ? joint->prismaticJoint.openPosition : joint->revoluteJoint.openAngle;

	return std::abs(open - closed) > 1e-4f;
}

static uint32_t PackJointTarget(VehicleGadgetJoint* joint, bool atRest)
{
	float closed, open;

	if (!GetJointLimits(joint, closed, open))
	{
		return 0;
	}

	// the owner's vehicle can go to sleep before its joints reach their desired values once the input stops, so send where they stopped
	const float target = joint->prismatic ? (atRest ? joint->prismaticJoint.position : joint->prismaticJoint.desiredPosition) : (atRest ? joint->revoluteJoint.angle : joint->revoluteJoint.desiredAngle);
	const float ratio = std::clamp((target - closed) / (open - closed), 0.0f, 1.0f);

	return static_cast<uint32_t>(ratio * kRatioMax + 0.5f);
}

static void UnpackJointTarget(VehicleGadgetJoint* joint, uint32_t packed)
{
	float closed, open;

	if (!GetJointLimits(joint, closed, open))
	{
		return;
	}

	const float target = closed + (open - closed) * (float(std::min(packed, kRatioMax)) / kRatioMax);

	if (joint->prismatic)
	{
		joint->prismaticJoint.desiredPosition = target;
	}
	else
	{
		joint->revoluteJoint.desiredAngle = target;
		joint->revoluteJoint.onlyControlSpeed = false;
	}
}

static bool IsJointAwayFromTarget(VehicleGadgetJoint* joint)
{
	if (!joint || joint->bone < 0 || joint->swingFreely)
	{
		return false;
	}

	const float delta = joint->prismatic ? (joint->prismaticJoint.position - joint->prismaticJoint.desiredPosition) : (joint->revoluteJoint.angle - joint->revoluteJoint.desiredAngle);
	return std::abs(delta) > kJointSleepThreshold;
}

static void (*g_origForksSerialise)(VehicleGadgetWithJoints* gadget, SyncDataSerialiser* serialiser);

//
// acceleration:8 slideTarget:12 tiltTarget:12, both targets as a 0..1 ratio between the joint's closed and open limits.
// The size of the gadget data block is measured by serialising a dummy gadget, so it follows this format too.
//
static void CVehicleGadgetForks_Serialise(VehicleGadgetWithJoints* gadget, SyncDataSerialiser* serialiser)
{
	if (!IsVehicleGadgetSyncFixActive())
	{
		g_origForksSerialise(gadget, serialiser);
		return;
	}

	auto slideJoint = gadget->GetJoint(0);
	auto tiltJoint = gadget->GetJoint(1);

	const bool atRest = !slideJoint || std::abs(slideJoint->desiredAcceleration) <= kForksSleepAccelerationThreshold;

	uint32_t acceleration = static_cast<uint32_t>(kAccelerationScale);
	uint32_t slideTarget = PackJointTarget(slideJoint, atRest);
	uint32_t tiltTarget = PackJointTarget(tiltJoint, atRest);

	if (slideJoint)
	{
		acceleration = static_cast<uint32_t>(std::lround(std::clamp(slideJoint->desiredAcceleration, -1.0f, 1.0f) * kAccelerationScale) + kAccelerationScale);
	}

	serialiser->SerialiseUnsigned(acceleration, kAccelerationBits);
	serialiser->SerialiseUnsigned(slideTarget, kRatioBits);
	serialiser->SerialiseUnsigned(tiltTarget, kRatioBits);

	if (!serialiser->IsReader())
	{
		return;
	}

	const float desiredAcceleration = std::clamp((float(acceleration) - kAccelerationScale) / kAccelerationScale, -1.0f, 1.0f);

	if (slideJoint)
	{
		slideJoint->desiredAcceleration = desiredAcceleration;
		UnpackJointTarget(slideJoint, slideTarget);
	}

	if (tiltJoint)
	{
		tiltJoint->desiredAcceleration = desiredAcceleration;
		UnpackJointTarget(tiltJoint, tiltTarget);
	}
}

static bool (*g_origForksWantsToBeAwake)(VehicleGadgetWithJoints* gadget, void* vehicle);

// clones only drive their joints to the synced targets while awake, the forks gadget only stays awake while accelerating
static bool CVehicleGadgetForks_WantsToBeAwake(VehicleGadgetWithJoints* gadget, void* vehicle)
{
	if (g_origForksWantsToBeAwake(gadget, vehicle))
	{
		return true;
	}

	if (!vehicle || !IsVehicleGadgetSyncFixActive())
	{
		return false;
	}

	auto netObj = GetNetObjectFromEntity(vehicle);

	if (!netObj || !netObj->syncData.isRemote)
	{
		return false;
	}

	return IsJointAwayFromTarget(gadget->GetJoint(0)) || IsJointAwayFromTarget(gadget->GetJoint(1));
}

static HookFunction hookFunction([]
{
	g_origForksSerialise = hook::trampoline(hook::get_pattern("83 64 24 48 00 83 64 24 40 00 48 8B FA 48 8B D9 48 85 C0 74 ? F3 0F 10 40 38", -0xE), CVehicleGadgetForks_Serialise);
	g_origForksWantsToBeAwake = hook::trampoline(hook::get_pattern("44 0F BF 41 10 33 D2 45 85 C0 7E ? 4C 8B 49 18 48 0F BF C2 48 8D 0C 80 48 03 C9 F3 41 0F 10 44 C9 08"), CVehicleGadgetForks_WantsToBeAwake);
});
