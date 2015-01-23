/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

//
// Basic platform includes.
//

// TODO: non-portable; need to replace when making cross-platform
#include <ppltasks.h>

#include <stdint.h>

// CitizenFX-specific: export
#ifdef COMPILING_TERMINAL_CLIENT
#define TERMINAL_EXPORT __declspec(dllexport)
#else
#define TERMINAL_EXPORT __declspec(dllimport)
#endif

namespace terminal
{
//
// Enumeration of error codes.
//
enum class ErrorCode
{
	// The operation completed successfully.
	Success = 0,

	// An unknown error occurred.
	Unknown,

	// The specified interface is unknown.
	UnknownInterface,

	// The client is not connected.
	NotConnected,

	// Starting point for connection-related errors.
	ConnectionBase = 100,

	// The connection to the remote peer timed out.
	ConnectionTimedOut,

	// The remote peer refused the connection.
	ConnectionRefused,

	// An invalid host name was passed, or the name could not be resolved due to a transient error.
	InvalidHostname,

	// The end of the stream was reached.
	EndOfStream,

	// An unknown connection error occurred.
	ConnectionError,

	// An invalid URI scheme was passed.
	InvalidScheme,

	// An invalid URI was passed.
	InvalidUri,

	// Starting point for authentication-related errors.
	AuthenticateBase = 200,

	// Invalid details were passed to the service.
	InvalidAuthDetails = 201,

	// The authentication service is temporarily unavailable.
	AuthenticationServiceUnavailable = 202,

	// The passed details are blacklisted.
	AuthDetailsBanned = 203
};

//
// Base type for a result.
//
class ResultBase
{
private:
	ErrorCode m_error;

public:
	//
	// Create a ResultBase indicating success.
	//
	inline ResultBase()
		: m_error(ErrorCode::Success)
	{

	}

	//
	// Create a ResultBase containing an error code.
	//
	inline ResultBase(ErrorCode error)
		: m_error(error)
	{

	}

	//
	// Gets the error code belonging to the ResultBase.
	//
	inline ErrorCode GetError() const
	{
		return m_error;
	}

	//
	// Returns whether the error code indicates success.
	//
	inline bool HasSucceeded() const
	{
		return (m_error == ErrorCode::Success);
	}
};

//
// Asynchronous result type containing a message and value.
//
template<typename T>
class Result : public ResultBase
{
private:
	T m_detail;

public:
	inline Result()
		: ResultBase()
	{

	}

	inline Result(const T& detail)
		: m_detail(detail)
	{

	}

	template<typename TOther>
	inline Result(const Result<TOther>& codeFrom)
		: ResultBase(codeFrom.GetError())
	{

	}

	template<typename TOther>
	inline Result(const Result<TOther>& codeFrom, const T& detail)
		: Result(codeFrom.GetError(), detail)
	{

	}

	inline Result(ErrorCode error, const T& detail)
		: Result(detail)
	{
		ResultBase::ResultBase(error);
	}

	inline const T& GetDetail() const
	{
		return m_detail;
	}
};

//
// Asynchronous result type containing no value.
//
template<>
class Result<void> : public ResultBase
{
public:
	inline Result()
		: ResultBase()
	{

	}

	inline Result(ErrorCode error)
		: ResultBase(error)
	{

	}
};

//
// Function to create an instance of an interface.
//
TERMINAL_EXPORT fwRefContainer<fwRefCountable> CreateInterface(uint64_t interfaceID);

//
// Convenience function to create an instance of an interface given only a type.
//
template<class T>
inline fwRefContainer<T> Create()
{
	return CreateInterface(T::InterfaceID);
}
}