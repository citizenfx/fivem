/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

struct CEpisode
{
	char path[260];
	char pad[64];
	char device[20]; // +324
	int deviceType; // +344
	char pad2[8];
	char pad3[2];
	char useThis; // +358; checked before calling the 'scan and process episode setup' func
	char unknown1; // +359
};

struct CEpisodes
{
	char pad[324];
	void* contents; // actually atArray?
	short numContents;
	short numAllocatedContents;
	CEpisode* episodes; // +332
	short numEpisodes; // +336
	short numAllocatedEpisodes; // +338
	char pad2[18]; // +340
	char unknownScanFlag; // +358

	void addEpisode(const CEpisode* episode);

	void ScanEpisodes();
};

extern CEpisodes*& g_episodes;
extern int g_useNorthEpisodes;
extern bool g_preventSaveLoading;