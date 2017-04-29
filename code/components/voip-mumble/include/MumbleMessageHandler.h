/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <MumbleTypes.h>

#include <list>

class MumbleMessageHandlerBase
{
private:
	static MumbleMessageHandlerBase* ms_messageHandlerRegistry[MAX_MUMBLE_MESSAGE + 1];

protected:
	MumbleMessageType m_messageType;

public:
	static MumbleMessageHandlerBase* GetHandlerFor(MumbleMessageType type);

public:
	MumbleMessageHandlerBase(MumbleMessageType type);

	virtual void HandleMessage(const uint8_t* message, size_t length) = 0;
};

class MumbleMessageHandler : public MumbleMessageHandlerBase
{
public:
	typedef std::function<void(const uint8_t*, size_t)> HandlerType;
	
private:
	HandlerType m_handler;

public:
	MumbleMessageHandler(MumbleMessageType messageType, HandlerType handler)
		: MumbleMessageHandlerBase(messageType), m_handler(handler)
	{

	}

	virtual void HandleMessage(const uint8_t* message, size_t length);
};

template<typename T>
class MumbleProtoMessageHandler : public MumbleMessageHandlerBase
{
public:
	typedef std::function<void(T&)> HandlerType;

private:
	HandlerType m_handler;

public:
	MumbleProtoMessageHandler(MumbleMessageType type, HandlerType handler)
		: MumbleMessageHandlerBase(type), m_handler(handler)
	{
	}

	virtual void HandleMessage(const uint8_t* message, size_t length)
	{
		T data;
		data.ParseFromArray(message, length);

		m_handler(data);
	}
};

#define DEFINE_HANDLER(msg) static MumbleProtoMessageHandler<MumbleProto::msg> handler_##msg(MumbleMessageType::msg, [] (MumbleProto::msg& data)

inline std::string ConvertToUTF8(std::wstring str)
{
	char buffer[8192];
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, buffer, sizeof(buffer), nullptr, nullptr);

	return buffer;
}

inline std::wstring ConvertFromUTF8(std::string str)
{
	wchar_t buffer[8192];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, _countof(buffer));

	return buffer;
}