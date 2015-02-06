/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "EpisodeManager.h"

class InitializerFactory
{
private:
	InitializerFactory() {}

public:
	static Episode::EpisodeInitializer GetSinglePlayerInitializer(int episodeNum, std::string episodeDat);
};