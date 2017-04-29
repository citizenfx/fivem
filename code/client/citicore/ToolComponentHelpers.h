/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "ToolComponent.h"

class FxToolCommand;

class ToolComponentBaseImpl : public fwRefCountable
{
private:
	static ToolComponentBaseImpl* ms_instance;

	FxToolCommand* m_baseCommand;

public:
	inline ToolComponentBaseImpl()
	{
		m_baseCommand = nullptr;
	}

	static inline ToolComponentBaseImpl* Get()
	{
		if (!ms_instance)
		{
			ms_instance = new ToolComponentBaseImpl();
		}

		return ms_instance;
	}

	inline FxToolCommand*& GetCurrentToolCommand()
	{
		return m_baseCommand;
	}
};

class FxToolCommand : public ToolCommand
{
private:
	FxToolCommand* m_next;

	const char* m_name;

	void(*m_parseOptionFn)(boost::program_options::wcommand_line_parser&, std::function<void()> cb);

	void(*m_runFn)(const boost::program_options::variables_map&);

public:
	inline FxToolCommand(const char* name, void(*parseOptionFn)(boost::program_options::wcommand_line_parser&, std::function<void()> cb), void(*runFn)(const boost::program_options::variables_map&))
		: m_name(name), m_parseOptionFn(parseOptionFn), m_runFn(runFn)
	{
		AddRef();

		auto& toolCommand = ToolComponentBaseImpl::Get()->GetCurrentToolCommand();

		if (toolCommand)
		{
			m_next = toolCommand->m_next;
			toolCommand->m_next = this;
		}
		else
		{
			toolCommand = this;
		}
	}

	inline const char* GetName()
	{
		return m_name;
	}

	inline FxToolCommand* GetNext()
	{
		return m_next;
	}

	inline virtual void SetupCommandLineParser(boost::program_options::wcommand_line_parser& parser, std::function<void(boost::program_options::wcommand_line_parser&)> cb) override
	{
		m_parseOptionFn(parser, [&] ()
		{
			cb(parser);
		});
	}

	inline virtual void InvokeCommand(const boost::program_options::variables_map& variables) override
	{
		m_runFn(variables);
	}
};

template<typename TBaseComponent>
class ToolComponentBase : public TBaseComponent, public ToolComponent
{
private:
	ToolComponentBaseImpl* m_impl;

public:
	ToolComponentBase()
		: TBaseComponent()
	{
		m_impl = ToolComponentBaseImpl::Get();
	}

	virtual std::vector<std::string> GetCommandNames() override
	{
		auto cmd = m_impl->GetCurrentToolCommand();
		std::vector<std::string> retval;

		while (cmd)
		{
			retval.push_back(cmd->GetName());

			cmd = cmd->GetNext();
		}

		return retval;
	}

	virtual fwRefContainer<ToolCommand> GetCommand(const std::string& commandName) override
	{
		auto cmd = m_impl->GetCurrentToolCommand();

		while (cmd)
		{
			if (_stricmp(commandName.c_str(), cmd->GetName()) == 0)
			{
				return cmd;
			}

			cmd = cmd->GetNext();
		}

		return nullptr;
	}
};

