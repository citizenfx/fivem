#pragma once

#include <StdInc.h>

static std::vector<std::string> g_prodCommandsWhitelist{
	"connect",
	"quit",
	"cl_drawFPS",
	"bind",
	"rbind",
	"unbind",
	"disconnect",
	"storymode",
	"loadlevel",
	"cl_drawPerf",
	"profile_reticuleSize",
	"profile_musicVolumeInMp",
	"profile_musicVolume",
	"profile_sfxVolume",
	// Crosshair commands
	"cl_customCrosshair",
	"cl_crosshairdot",
	"cl_crosshairsize",
	"cl_crosshairstyle",
	"cl_crosshairthickness",
	"cl_crosshairgap",
	"cl_crosshair_drawoutline",
	"cl_crosshair_outlinethickness",
	"cl_crosshaircolor",
	"cl_crosshaircolor_r",
	"cl_crosshaircolor_g",
	"cl_crosshaircolor_b",
	"cl_crosshairusealpha",
	"cl_crosshairalpha",
	"cl_crosshair_dynamic_splitdist",
	"cl_crosshair_dynamic_splitalpha_innermod",
	"cl_crosshair_dynamic_splitalpha_outermod",
	"cl_crosshair_dynamic_maxdist_splitratio",
	"cl_crosshair_t"
};
