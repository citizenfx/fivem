/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NodeScopeHandler.h"
#include <stack>
#include <functional>

// Defining it here to avoid forking nodejs repo for just adding one line of code
namespace v8
{
void SetScopeHandler(const std::function<void(v8::Isolate*)>& enter,
const std::function<void(v8::Isolate*)>& exit);
}

namespace fx::nodejs
{
	class BaseScope{
	public:
		inline BaseScope(v8::Isolate* isolate)
			: m_isolate(isolate)
		{
		}
		v8::Isolate* m_isolate;
	};

	class LockerScope : public BaseScope
	{
		v8::Locker m_locker;
		v8::Isolate::Scope m_scope;

	public:
		inline LockerScope(v8::Isolate* isolate) : BaseScope(isolate), m_locker(isolate), m_scope(isolate) {}
	};

	static thread_local std::stack<std::unique_ptr<BaseScope>> g_scopeStack;

	void ScopeHandler::Initialize()
	{
		v8::SetScopeHandler([](v8::Isolate* isolate) {
			// don't push the same isolate if its already on top of the stack
			if (g_scopeStack.size() > 0 && g_scopeStack.top()->m_isolate == isolate)
			{
				g_scopeStack.push(std::make_unique<BaseScope>(isolate));
				return;
			}
			g_scopeStack.push(std::make_unique<LockerScope>(isolate));
		},
		[](v8::Isolate* isolate) {
			g_scopeStack.pop();
		});

		m_initialized = true;
	}

	void ScopeHandler::Shutdown()
	{
		
	}
}
