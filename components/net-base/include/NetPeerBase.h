/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <DatagramSink.h>
#include <NetBuffer.h>

#include <SequencedInputDatagramChannel.h>
#include <SequencedOutputDatagramChannel.h>

// A network peer management base using a high-level type-framed protocol.
namespace net
{
class PeerBase;

typedef std::function<void(PeerBase*, Buffer&)> NetProcessor;
typedef std::function<void(PeerBase*, Buffer&)> NetGenerator;

class PeerHandler
{
private:
	std::map<uint32_t, NetProcessor> m_processors;
	std::map<uint32_t, NetGenerator> m_generators;
	std::vector<std::pair<const char*, fwRefContainer<fwRefCountable>>> m_components;

private:
	template<typename TContainer, typename TReceiver>
	void AddTo(const TContainer& container, const TReceiver& receiver)
	{
		for (auto&& entry : container)
		{
			receiver(entry.first, entry.second);
		}
	}

public:
	template<typename TReceiver>
	void AddProcessors(const TReceiver& receiver)
	{
		AddTo(m_processors, receiver);
	}

	template<typename TReceiver>
	void AddHandlers(const TReceiver& receiver)
	{
		AddTo(m_handlers, receiver);
	}

	template<typename TReceiver>
	void AddComponents(const TReceiver& receiver)
	{
		AddTo(m_components, receiver);
	}

	template<typename TProcess>
	uint32_t RegisterType(const char* name, const TProcess& processor)
	{
		uint32_t nameHash = HashRageString(name);

		m_processors.insert(std::make_pair(nameHash, std::function<void(PeerBase*, Buffer&)>(processor)));

		return nameHash;
	}

	template<typename TProcess, typename TGenerate>
	uint32_t RegisterType(const char* name, const TProcess& processor, const TGenerate& generator)
	{
		uint32_t nameHash = RegisterType<TProcess>(name, processor);

		m_generators.insert(std::make_pair(nameHash, std::function<void(PeerBase*, Buffer&)>(generator)));
	}

	template<typename TComponent, typename... TArgs>
	void RegisterComponent(TArgs... args)
	{
		m_components.push_back(std::make_pair(Instance<TComponent>::GetName(), new TComponent(Args...)));
	}
};

template<typename TFunc>
class FuncDatagramSinkBase : public DatagramSink
{
private:
	TFunc m_function;

public:
	FuncDatagramSinkBase(const TFunc& function)
		: m_function(function)
	{

	}

	virtual void WritePacket(const std::vector<uint8_t>& packet) override
	{
		m_function(packet);
	}
};

class FunctionDatagramSink : public FuncDatagramSinkBase<std::function<void(const std::vector<uint8_t>&)>>
{
public:
	FunctionDatagramSink(const std::function<void(const std::vector<uint8_t>&)>& function)
		: FuncDatagramSinkBase::FuncDatagramSinkBase(function)
	{
		
	}
};

class PeerBase : public fwRefCountable
{
private:
	fwRefContainer<DatagramSink> m_outSink;

	fwRefContainer<FunctionDatagramSink> m_inSink;

	fwRefContainer<SequencedInputDatagramChannel> m_inputChannel;

	fwRefContainer<SequencedOutputDatagramChannel> m_outputChannel;

	std::map<uint32_t, NetProcessor> m_processors;

	std::map<uint32_t, NetGenerator> m_generators;

	// mapping of shorthand to full packet types (local to remote)
	std::map<uint32_t, int> m_localToRemoteMapping;

	// mapping of shorthand to full packet types (remote to local)
	std::map<int, uint32_t> m_remoteToLocalMapping;

	fwRefContainer<RefInstanceRegistry> m_components;

	std::string m_peerName;

private:
	void RegisterHandlerInternal(const PeerHandler& trait);

	void ProcessEncapsulatedPacket(const std::vector<uint8_t>& buffer);

	void ProcessMappingPacket(Buffer& buffer);

	int ReadCompressedType(Buffer& buffer);

public:
	PeerBase(const fwRefContainer<DatagramSink>& outSink);

	void ProcessPacket(const std::vector<uint8_t>& buffer);

	template<typename HandlerType>
	void RegisterHandler()
	{
		RegisterHandlerInternal(HandlerType());
	}

	template<typename ComponentType>
	fwRefContainer<ComponentType> GetComponent()
	{
		return Instance<ComponentType>::Get(m_components.GetRef());
	}

	virtual const std::string& GetName()
	{
		return m_peerName;
	}

	inline void SetPeerName(const std::string& name)
	{
		m_peerName = name;
	}
};
}