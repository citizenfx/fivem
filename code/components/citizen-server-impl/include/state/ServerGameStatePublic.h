#pragma once

#include <ClientRegistry.h>
#include <OneSyncVars.h>

#define GLM_ENABLE_EXPERIMENTAL

#if defined(_M_IX86) || defined(_M_AMD64) || defined(__x86_64__) || defined(__i386__)
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SSE2
#define GLM_FORCE_SSE3
#endif

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

namespace fx
{
enum class SyncStyle
{
	NAK = 0,
	ARQ = 1
};

namespace sync
{
class Entity
{
public:
	virtual bool IsPlayer() = 0;

	virtual fx::ClientSharedPtr GetOwner() = 0;

	virtual glm::vec3 GetPosition() = 0;

	virtual uint32_t GetId() = 0;

	virtual std::string GetPopType() = 0;

	virtual uint32_t GetModel() = 0;

	virtual std::string GetType() = 0;
};
}

class ServerGameStatePublic : public fwRefCountable
{
public:
	virtual void SendObjectIds(const fx::ClientSharedPtr& client, int numIds) = 0;

	virtual SyncStyle GetSyncStyle() = 0;

	virtual EntityLockdownMode GetEntityLockdownMode(const fx::ClientSharedPtr& client) = 0;

	virtual void ParseGameStatePacket(const fx::ClientSharedPtr& client, const std::vector<uint8_t>& packetData) = 0;

	virtual void ForAllEntities(const std::function<void(sync::Entity*)>& cb) = 0;
};
}

DECLARE_INSTANCE_TYPE(fx::ServerGameStatePublic);
