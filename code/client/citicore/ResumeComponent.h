#pragma once

class LifeCycleComponent
{
public:
	virtual void PreResumeGame() = 0;

	virtual void PreInitGame() = 0;

	virtual void HandleAbnormalTermination(void* reason) = 0;
};

__declspec(selectany) fwEvent<> OnResumeGame;

__declspec(selectany) fwEvent<void*> OnAbnormalTermination;

template<typename TBaseComponent>
class LifeCycleComponentBase : public TBaseComponent, public LifeCycleComponent
{
public:
	virtual void PreResumeGame() override
	{
		OnResumeGame();
	}
	
	virtual void PreInitGame() override
	{
		// empty
	}

	virtual void HandleAbnormalTermination(void* reason) override
	{
		OnAbnormalTermination(reason);
	}

	virtual bool IsA(uint32_t type) override
	{
		if (type == HashString("LifeCycleComponent"))
		{
			return true;
		}

		return TBaseComponent::IsA(type);
	}

	virtual void* As(uint32_t type) override
	{
		if (type == HashString("LifeCycleComponent"))
		{
			return static_cast<LifeCycleComponent*>(this);
		}

		return TBaseComponent::As(type);
	}
};

void Component_RunPreInit();

template<typename TBaseComponent>
class LifeCyclePreInitComponentBase : public LifeCycleComponentBase<TBaseComponent>
{
public:
	virtual void PreInitGame() override
	{
		Component_RunPreInit();
	}
};
