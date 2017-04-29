/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <boost/property_tree/ptree.hpp>

namespace fx
{
	class
#ifdef COMPILING_CITIZEN_SERVER_INSTANCE
		DLL_EXPORT
#else
		DLL_IMPORT
#endif
		ServerInstanceBase : public fwRefCountable
	{
	public:
		virtual InstanceRegistry* GetInstanceRegistry() = 0;

		virtual const std::string& GetRootPath() = 0;

	public:
		fwEvent<const boost::property_tree::ptree&> OnReadConfiguration;

	public:
		static fwEvent<ServerInstanceBase*> OnServerCreate;
	};
}