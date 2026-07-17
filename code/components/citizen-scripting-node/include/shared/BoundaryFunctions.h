/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "RuntimeHelpers.h"

namespace fx::v8shared
{
	template<class RuntimeType>
	static void V8_SubmitBoundaryStart(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		// get required entries
		auto scrt = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto scriptHost = scrt->GetScriptHost();

		auto val = args[0]->IntegerValue(scrt->GetContext());

		V8Boundary b;
		b.hint = val.ToChecked();

		scriptHost->SubmitBoundaryStart((char*)&b, sizeof(b));
	}

	template<class RuntimeType>
	static void V8_SubmitBoundaryEnd(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		// get required entries
		auto scrt = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto scriptHost = scrt->GetScriptHost();

		auto val = args[0]->IntegerValue(scrt->GetContext());

		V8Boundary b;
		b.hint = val.ToChecked();

		scriptHost->SubmitBoundaryEnd((char*)&b, sizeof(b));
	}
}
