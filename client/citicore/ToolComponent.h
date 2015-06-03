/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <boost/program_options.hpp>

class CORE_EXPORT ToolCommand : public fwRefCountable
{
public:
	virtual boost::program_options::wcommand_line_parser& SetupCommandLineParser(boost::program_options::wcommand_line_parser& parser) = 0;

	virtual void InvokeCommand(const boost::program_options::variables_map& variables) = 0;
};

class CORE_EXPORT ToolComponent
{
public:
	virtual std::vector<std::string> GetCommandNames() = 0;

	virtual fwRefContainer<ToolCommand> GetCommand(const std::string& commandName) = 0;
};