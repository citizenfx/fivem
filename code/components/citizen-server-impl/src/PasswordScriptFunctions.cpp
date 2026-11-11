#include "StdInc.h"

#include <ScriptEngine.h>

#include <GameServerComms.h>
#include <ResourceCallbackComponent.h>
#include <ResourceManager.h>

#include <thread_pool.hpp>

#include <botan/bcrypt.h>
#include <botan/auto_rng.h>

static tp::ThreadPool& GetPasswordThreadPool()
{
	// created on first use, so servers not using the async natives don't spawn its threads
	static tp::ThreadPool threadPool;
	return threadPool;
}

template<typename TTask>
static void RunPasswordTaskAsync(const char* callbackRef, TTask&& task)
{
	auto callback = std::make_shared<fx::FunctionRef>(callbackRef);
	auto resourceManager = fx::ResourceManager::GetCurrent();

	GetPasswordThreadPool().post([callback, resourceManager, task = std::forward<TTask>(task)]() mutable
	{
		decltype(task()) result{};

		try
		{
			result = task();
		}
		catch (std::exception& e)
		{
			trace("Password hash task failed: %s\n", e.what());
		}

		// moved out, as the pool keeps a finished task alive until the worker picks up its next one
		gscomms_execute_callback_on_main_thread([callback = std::move(callback), resourceManager, result]()
		{
			resourceManager->CallReference<void>(callback->GetRef(), result);
		});
	});
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_PASSWORD_HASH", [](fx::ScriptContext& context)
	{
		const char* str = context.CheckArgument<const char*>(0);

		std::string password = (str != nullptr) ? str : "";

		Botan::AutoSeeded_RNG rng;
		static thread_local std::string hashed;
		hashed = Botan::generate_bcrypt(password, rng, 11);

		context.SetResult(hashed.c_str());
	});

	fx::ScriptEngine::RegisterNativeHandler("VERIFY_PASSWORD_HASH", [](fx::ScriptContext& context)
	{
		std::string password = context.CheckArgument<const char*>(0);
		std::string hash = context.CheckArgument<const char*>(1);

		context.SetResult(Botan::check_bcrypt(password, hash));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PASSWORD_HASH_ASYNC", [](fx::ScriptContext& context)
	{
		std::string password = context.CheckArgument<const char*>(0);

		RunPasswordTaskAsync(context.CheckArgument<const char*>(1), [password]()
		{
			Botan::AutoSeeded_RNG rng;
			return Botan::generate_bcrypt(password, rng, 11);
		});
	});

	fx::ScriptEngine::RegisterNativeHandler("VERIFY_PASSWORD_HASH_ASYNC", [](fx::ScriptContext& context)
	{
		std::string password = context.CheckArgument<const char*>(0);
		std::string hash = context.CheckArgument<const char*>(1);

		RunPasswordTaskAsync(context.CheckArgument<const char*>(2), [password, hash]()
		{
			return Botan::check_bcrypt(password, hash);
		});
	});
});
