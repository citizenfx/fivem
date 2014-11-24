#pragma once

inline int GetScreenResolutionX()
{
	return *(int*)0xFDCEAC;
}

inline int GetScreenResolutionY()
{
	return *(int*)0xFDCEB0;
}