#pragma once
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wldap32.lib")
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <psapi.h> 
#include <algorithm> 

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#include <gdiplus.h>
#undef max
#undef min

//using namespace std;
using namespace Gdiplus;


class Anticheat
{
public:
	Anticheat();
	~Anticheat();
	
	long lCode;
	void* pParam;
	void GetScreen();

	 std::ofstream myfile;

 private:
	bool isRunning(char *pProcessName);
	int  GetModules( DWORD processID );

	DWORD hunzPID;
	int		dlls;
	HANDLE handle;
	int startDlls;
};
