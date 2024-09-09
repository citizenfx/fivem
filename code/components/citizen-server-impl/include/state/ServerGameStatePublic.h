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

#include <NetGameEventV2.h>

#include "ArrayUpdate.h"

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

class StateBag;

class ServerGameStatePublic : public fwRefCountable
{
public:
	virtual void HandleArrayUpdate(const fx::ClientSharedPtr& client, net::packet::ClientArrayUpdate& buffer) = 0;

	virtual void GetFreeObjectIds(const fx::ClientSharedPtr& client, uint8_t numIds, std::vector<uint16_t>& freeIds) = 0;

	virtual SyncStyle GetSyncStyle() = 0;

	virtual EntityLockdownMode GetEntityLockdownMode(const fx::ClientSharedPtr& client) = 0;

	virtual void ParseGameStatePacket(const fx::ClientSharedPtr& client, const std::vector<uint8_t>& packetData) = 0;

	virtual void ForAllEntities(const std::function<void(sync::Entity*)>& cb) = 0;

	virtual bool SetEntityStateBag(uint8_t playerId, uint16_t objectId, std::function<std::shared_ptr<StateBag>()> createStateBag) = 0;

	virtual uint32_t GetClientRoutingBucket(const fx::ClientSharedPtr& client) = 0;

	virtual std::function<bool()> GetGameEventHandlerWithEvent(const fx::ClientSharedPtr& client, const std::vector<uint16_t>& targetPlayers, net::packet::ClientNetGameEventV2& netGameEvent) = 0;

	virtual bool IsClientRelevantEntity(const fx::ClientSharedPtr& client, uint32_t objectId) = 0;
};
}

DECLARE_INSTANCE_TYPE(fx::ServerGameStatePublic);
