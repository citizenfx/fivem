#include "StdInc.h"

#include <ScriptEngine.h>

#include <botan/bcrypt.h>
#include <botan/auto_rng.h>

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
});
