#pragma once

class LifeCycleComponent
{
public:
	virtual void PreResumeGame() = 0;

	virtual void PreInitGame() = 0;
};

__declspec(selectany) fwEvent<> OnResumeGame;

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
};

void Component_RunPreInit();

template<typename TBaseComponent>
class LifeCyclePreInitComponentBase : public LifeCycleComponentBase<TBaseComponent>
{
public:
	virtual void PreResumeGame() override
	{
		OnResumeGame();
	}

	virtual void PreInitGame() override
	{
		Component_RunPreInit();
	}
};