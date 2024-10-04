#pragma once

#include "EntitySystem.h"
#include "NetworkPlayerMgr.h"
#include "NetGameEventPacket.h"

namespace rage
{
	class datBitBuffer;

	class netGameEvent
	{
	public:
		virtual ~netGameEvent() = 0;

#ifdef GTA_FIVE
		virtual const char* GetName() = 0;
#endif

		virtual bool IsInScope(CNetGamePlayer* player) = 0;

		virtual bool TimeToResend(uint32_t) = 0;

		virtual bool CanChangeScope() = 0;

		virtual void Prepare(rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual void Handle(rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual bool Decide(CNetGamePlayer* sourcePlayer, void* connUnk) = 0;

		virtual void PrepareReply(rage::datBitBuffer* buffer, CNetGamePlayer* replyPlayer) = 0;

		virtual void HandleReply(rage::datBitBuffer* buffer, CNetGamePlayer* sourcePlayer) = 0;

#ifdef GTA_FIVE
		virtual void PrepareExtraData(rage::datBitBuffer* buffer, bool isReply, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual void HandleExtraData(rage::datBitBuffer* buffer, bool isReply, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;
#elif IS_RDR3
		virtual void PrepareExtraData(rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;

		virtual void HandleExtraData(rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkPlayer) = 0;
#endif

#ifdef GTA_FIVE
		virtual void m_60() = 0;

		virtual void m_68() = 0;

		virtual void m_70() = 0;
#endif

		virtual void m_78() = 0;

		virtual bool Equals(const netGameEvent* event) = 0;

		virtual bool NotEquals(const netGameEvent* event) = 0;

		virtual bool MustPersist() = 0;

		virtual bool MustPersistWhenOutOfScope() = 0;

		virtual bool HasTimedOut() = 0;

	public:
		uint16_t eventType; // +0x8

		uint8_t requiresReply : 1; // +0xA

		uint8_t pad_0Bh; // +0xB

#ifdef GTA_FIVE
		char pad_0Ch[24]; // +0xC
#elif IS_RDR3
		char pad_0Ch[36]; // +0xC
#endif

		uint16_t eventId; // +0x24

		uint8_t hasEventId : 1; // +0x26

#ifdef IS_RDR3
	public:
		const char* GetName();
#endif
	};

#ifdef GTA_FIVE
	// #TODO2802: impossible now, use base game's!
	class datBase
	{
	public:
		virtual ~datBase() = default;
	};

	template<typename TNode>
	class atDLList
	{
	public:
		void Add(TNode* node)
		{
			if (tail)
			{
				node->prev = tail;
				node->next = tail->next;

				if (tail->next)
				{
					tail->next->prev = node;
				}

				tail->next = node;
			}
			else
			{
				head = node;
			}

			tail = node;
		}

		void Remove(TNode* node)
		{
			if (node == head)
			{
				head = (decltype(head))node->next;
			}

			if (node == tail)
			{
				tail = (decltype(tail))node->prev;
			}

			node->Unlink();

			delete node;
		}

		template<typename Data>
		void RemoveData(Data* data)
		{
			for (auto node = head; node->next; node = (decltype(node))node->next)
			{
				if (node->data == data)
				{
					Remove(node);
					break;
				}
			}
		}

		template<typename Data>
		inline bool Has(Data* data) const
		{
			for (auto node = head; node->next; node = (decltype(node))node->next)
			{
				if (node->data == data)
				{
					return true;
				}
			}

			return false;
		}

		void Clear()
		{
			auto node = head;

			while (node)
			{
				auto next = node->next;
				delete node;

				node = (decltype(node))next;
			}

			head = tail = nullptr;
		}

	private:
		TNode* head;
		TNode* tail;
	};

	template<typename Data, typename Base>
	class atDNode : public Base
	{
	public:
		inline atDNode()
		{
			data = nullptr;
			next = nullptr;
			prev = nullptr;
		}

		inline Data* GetData() const
		{
			return data;
		}

		inline void SetData(Data* data)
		{
			this->data = data;
		}

		void Unlink()
		{
			if (next)
			{
				next->prev = prev;
			}

			if (prev)
			{
				prev->next = next;
			}

			next = nullptr;
			prev = nullptr;
		}

	public:
		Data* data;
		atDNode* next;
		atDNode* prev;
	};

	class netEventMgr
	{
	public:
		class atDNetEventNode : public atDNode<netGameEvent, datBase>, public PoolAllocated<atDNetEventNode>
		{
		public:
			static constexpr const uint32_t kHash = HashString("atDNetEventNode");
		};

		void AddEvent(netGameEvent* event);

		void RemoveEvent(netGameEvent* event);

		bool HasEvent(netGameEvent* event);

		void ClearEvents();

	private:
		char pad[40];
		atDLList<atDNetEventNode> eventList;
	};
#endif

	void EventManager_Update();
	bool EnsurePlayer31();
	CNetGamePlayer* GetPlayer31();

	static void HandleNetGameEvent(const char* idata, size_t len);
	static void HandleNetGameEventV2(net::packet::ServerNetGameEventV2& serverNetGameEventV2);
}
