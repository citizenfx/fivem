/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <uv.h>

#include <memory>
#include <shared_mutex>

#include "TcpServer.h"
#include <UvLoopHolder.h>

#include <array>

#include <tbb/concurrent_queue.h>

#include <uvw.hpp>

namespace net
{
class UvTcpServer;
class UvTcpChildServer;

class UvTcpServerStream : public TcpServerStream
{
private:
	UvTcpChildServer* m_server;

	std::shared_ptr<uvw::TCPHandle> m_client;

	std::shared_ptr<uvw::AsyncHandle> m_writeCallback;

	std::shared_ptr<uvw::TimerHandle> m_writeTimeout;

	std::atomic<uint64_t> m_pendingWrites;

	std::shared_mutex m_writeCallbackMutex;

	tbb::concurrent_queue<TScheduledCallback> m_pendingRequests;

	std::vector<char> m_readBuffer;

	std::thread::id m_threadId;

	volatile bool m_closingClient;

private:
	void HandleRead(ssize_t nread, const std::unique_ptr<char[]>& buf);

	void HandlePendingWrites();

	void CloseClient();

	void ResetWriteTimeout();

	inline std::vector<char>& GetReadBuffer()
	{
		return m_readBuffer;
	}

public:
	UvTcpServerStream(UvTcpChildServer* server);

	virtual ~UvTcpServerStream();

	bool Accept(std::shared_ptr<uvw::TCPHandle>&& client);

	virtual void AddRef() override
	{
		TcpServerStream::AddRef();
	}

	virtual bool Release() override
	{
		return TcpServerStream::Release();
	}

public:
	virtual PeerAddress GetPeerAddress() override;

	virtual void Write(const std::vector<uint8_t>& data, TCompleteCallback&& onComplete) override;

	virtual void Write(const std::string& data, TCompleteCallback&& onComplete) override;

	virtual void Write(std::vector<uint8_t>&&, TCompleteCallback&& onComplete) override;

	virtual void Write(std::string&&, TCompleteCallback&& onComplete) override;

	virtual void Write(std::unique_ptr<char[]> data, size_t size, TCompleteCallback&& onComplete) override;

	virtual void Close() override;

	virtual void ScheduleCallback(TScheduledCallback&& callback, bool performInline) override;

private:
	void WriteInternal(std::unique_ptr<char[]> data, size_t size, TCompleteCallback&& onComplete);
};

class TcpServerManager;

class UvTcpChildServer
{
public:
	UvTcpChildServer(UvTcpServer* parent, const std::string& pipeName, const std::array<uint8_t, 16>& pipeMessage, int idx);

	void Listen();

	void RemoveStream(UvTcpServerStream* stream);

	inline std::shared_ptr<uvw::PipeHandle> GetServer()
	{
		return m_dispatchPipe;
	}

	TcpServerManager* GetManager() const;

private:
	void OnConnection(int status);

private:
	std::shared_ptr<uvw::PipeHandle> m_dispatchPipe;

	std::string m_pipeName;

	std::array<uint8_t, 16> m_pipeMessage;

	std::string m_uvLoopName;

	fwRefContainer<net::UvLoopHolder> m_uvLoop;

	std::set<fwRefContainer<UvTcpServerStream>> m_clients;

	UvTcpServer* m_parent;
};

class UvTcpServer : public TcpServer
{
private:
	TcpServerManager* m_manager;

	std::shared_ptr<uvw::TCPHandle> m_server;

	std::shared_ptr<uvw::PipeHandle> m_listenPipe;

	std::set<std::shared_ptr<UvTcpChildServer>> m_childServers;

	std::vector<std::shared_ptr<uvw::PipeHandle>> m_dispatchPipes;
	std::set<std::shared_ptr<uvw::PipeHandle>> m_createdPipes;

	int m_dispatchIndex;

	std::string m_pipeName;
	std::array<uint8_t, 16> m_helloMessage;

	bool m_tryDetachFromIOCP;

private:
	void OnConnection(int status);

	void OnListenPipe(uvw::PipeHandle& handle);

public:
	UvTcpServer(TcpServerManager* manager);

	virtual ~UvTcpServer();

	bool Listen(std::shared_ptr<uvw::TCPHandle>&& server);

	inline std::shared_ptr<uvw::TCPHandle> GetServer()
	{
		return m_server;
	}

	inline TcpServerManager* GetManager()
	{
		return m_manager;
	}
};

// blindly copypasted from StackOverflow (to allow std::function to store the funcref types with their move semantics)
// TODO: we use this *three times* now, time for a shared header?
template<class F>
struct shared_function
{
	std::shared_ptr<F> f;
	shared_function() = default;
	shared_function(F&& f_) : f(std::make_shared<F>(std::move(f_))) {}
	shared_function(shared_function const&) = default;
	shared_function(shared_function&&) = default;
	shared_function& operator=(shared_function const&) = default;
	shared_function& operator=(shared_function&&) = default;

	template<class...As>
	auto operator()(As&&...as) const
	{
		return (*f)(std::forward<As>(as)...);
	}
};

template<class F>
shared_function<std::decay_t<F>> make_shared_function(F&& f)
{
	return { std::forward<F>(f) };
}
}

// helpful wrappers
class UvClosable
{
public:
	virtual ~UvClosable() = default;
};

class UvVirtualBase
{
public:
	virtual ~UvVirtualBase() = 0;
};

template<typename Handle, typename TFn>
auto UvCallbackWrap(Handle* handle, const TFn& fn)
{
	struct Request : public UvClosable
	{
		TFn fn;

		Request(const TFn& fn)
			: fn(fn)
		{

		}

		static void cb(Handle* handle)
		{
			Request* request = reinterpret_cast<Request*>(handle->data);

			request->fn(handle);
			delete request;
		}
	};

	auto req = new Request(fn);
	handle->data = req;

	return &Request::cb;
}

template<typename... TArgs>
struct UvCallbackArgs
{
	template<typename Handle, typename TFn>
	static auto Get(Handle* handle, TFn fn)
	{
		struct Request : public UvClosable
		{
			TFn fn;

			Request(TFn fn)
				: fn(std::move(fn))
			{

			}

			static void cb(Handle* handle, TArgs... args)
			{
				Request* request = reinterpret_cast<Request*>(handle->data);

				request->fn(handle, args...);
				delete request;
			}
		};

		auto req = new Request(std::move(fn));
		handle->data = req;

		return &Request::cb;
	}
};

template<typename Handle, typename TFn>
auto UvPersistentCallback(Handle* handle, TFn fn)
{
	struct Request : public UvClosable
	{
		TFn fn;

		Request(TFn fn)
			: fn(std::move(fn))
		{

		}

		static void cb(Handle* handle)
		{
			Request* request = reinterpret_cast<Request*>(handle->data);

			request->fn(handle);
		}
	};

	auto req = new Request(std::move(fn));
	handle->data = req;

	return &Request::cb;
}

// generic wrapper for libuv closing
template<typename Handle, typename TFn>
void UvCloseHelper(std::unique_ptr<Handle> handle, const TFn& fn)
{
	struct TempCloseData
	{
		std::unique_ptr<Handle> item;
		UvClosable* closable;
	};

	// create temporary object and give it our reference
	TempCloseData* tempCloseData = new TempCloseData;
	tempCloseData->closable = nullptr;

	try
	{
		if (handle->data)
		{
			auto closable = dynamic_cast<UvClosable*>(static_cast<UvVirtualBase*>(handle->data));

			if (closable)
			{
				tempCloseData->closable = closable;
			}
		}
	}
	catch (std::exception&)
	{

	}

	tempCloseData->item = std::move(handle);
	tempCloseData->item->data = tempCloseData;

	fn(tempCloseData->item.get(), [](auto* handle)
	{
		auto closeData = reinterpret_cast<TempCloseData*>(handle->data);

		// delete the closable, if any
		delete closeData->closable;

		// delete the close holder
		delete closeData;
	});
}

// wrapper to make sure the libuv handle only gets freed after the close completes
template<typename Handle>
void UvClose(std::unique_ptr<Handle> handle)
{
	return UvCloseHelper(std::move(handle), [](auto handle, auto cb)
	{
		uv_close((uv_handle_t*)handle, cb);
	});
}

template<typename T>
class UvHandleContainer
{
public:
	UvHandleContainer()
	{
		m_handle = std::make_unique<T>();
	}

	UvHandleContainer(UvHandleContainer&& right)
	{
		m_handle = std::move(right.m_handle);
	}

	~UvHandleContainer()
	{
		if (m_handle)
		{
			UvClose(std::move(m_handle));
		}
	}

	inline T* get()
	{
		return m_handle.get();
	}

	inline T* operator&()
	{
		return m_handle.get();
	}

private:
	std::unique_ptr<T> m_handle;
};
