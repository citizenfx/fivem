#pragma once

class GAMESPEC_EXPORT BoundStreaming
{
public:
	static uint32_t RegisterBound(const char* filename, uint32_t size, uint32_t rscFlags, uint32_t rscVersion);

	static void Process();

	static void LoadCollision(int id, int priority);

	static void ReleaseCollision(int id);

	static void __stdcall LoadAllObjectsTail(int);
};