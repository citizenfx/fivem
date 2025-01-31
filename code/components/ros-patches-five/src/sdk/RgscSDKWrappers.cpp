#include <StdInc.h>
#include <Hooking.h>

#include <botan/base64.h>

#include <ICoreGameInit.h>

#include <MinHook.h>
#include <Hooking.Aux.h>

#include <CoreConsole.h>
#include <Error.h>

HRESULT(__stdcall*g_origInitializeGraphics)(void*, void*, void*);

static HANDLE g_uiEvent;

LONG_PTR DontSetWindowLongPtrW(HWND hWnd, int nIndex, LONG_PTR dwNewLong)
{
	return GetWindowLongPtrW(hWnd, nIndex);
}

LONG(WINAPI* g_origSetWindowLongW)(HWND hWnd, int nIndex, LONG dwNewLong);

LONG WINAPI DontSetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong)
{
	static thread_local bool in = false;

	if (in)
	{
		return g_origSetWindowLongW(hWnd, nIndex, dwNewLong);
	}

	in = true;
	auto v = GetWindowLongW(hWnd, nIndex);
	in = false;

	return v;
}

HRESULT __stdcall NullInitializeGraphics(void* rgscUI, void* d3dDevice, void* hWndStruct)
{
	DisableToolHelpScope ts;

	trace("NullInitializeGraphics\n");

#ifdef _M_AMD64
	auto fn = GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowLongPtrW");
	MH_CreateHook(fn, DontSetWindowLongPtrW, nullptr);
#else
	auto fn = GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowLongW");
	MH_CreateHook(fn, DontSetWindowLongW, (void**)&g_origSetWindowLongW);
#endif

	MH_EnableHook(fn);

	g_origInitializeGraphics(rgscUI, d3dDevice, hWndStruct);

	MH_DisableHook(fn);

	return S_OK;
}

HRESULT __stdcall NullRender(void* rgscUI)
{
	return S_OK;
}

HRESULT __stdcall NullCopyBuffer(void* rgscUI, void* a1)
{
	return S_OK;
}

#define LOG_CALL() console::DPrintf("ros-patches", "%s\n", __FUNCTION__)

class IRgscUi
{

};

class IProfileV3
{
public:
	virtual HRESULT QueryInterface(GUID* iid, void** out) = 0;
	virtual void SetAccountId(uint64_t accountId) = 0; // 0x000000000FABDF22
	virtual void SetUsername(const char* userName) = 0; // R0fabdf22
	virtual void SetKey(const uint8_t* key) = 0; // 32 bytes
	virtual void SetBool1(bool value) = 0; // false
	virtual void SetTicket(const char* ticket) = 0; // YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh
	virtual void SetEmail(const char* email) = 0; // onlineservices@fivem.net
	virtual void SetUnkString(const char* str) = 0; // 0 byte string, ""
	virtual void SetScAuthToken(const char* str) = 0; // base64 blob, 120 bytes
};

class IRgscProfileManager
{
public:
	virtual HRESULT QueryInterface(GUID* iid, void** out) = 0;
	virtual bool isX() = 0; // bool isX()
	virtual bool isOnline() = 0; // bool isY()
	virtual int getProfileData(IProfileV3*) = 0; // int getProfileData(rgsc::ProfileV3*)
	virtual void* m_04(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual bool isZ() = 0; // bool isZ() - only called after /cfx/login call?
	virtual bool is1() = 0; // bool is1() - called from a background thread
	virtual void* m_07(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_08(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_09(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_10(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_11(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_12(void*, void*, void*, void*, void*, void*, void*) = 0;
};

class IRgscCommerceManager
{
public:
	virtual HRESULT QueryInterface(GUID* iid, void** out) = 0;

	virtual void* m_1(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_2(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_3(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_4(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_5(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_6(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_7(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_8(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_9(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_10(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_11(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_12(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_13(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_14(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_15(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_16(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_17(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_18(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_19(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_20(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_21(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_22(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_23(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_24(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_25(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_26(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_27(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_28(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_29(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_30(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_31(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_32(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_33(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_34(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_35(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_36(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_37(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_38(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_39(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_40(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_41(void*, void*, void*, void*, void*, void*, void*) = 0;
};

class IRgscFileSystem
{
public:
	virtual HRESULT QueryInterface(GUID* iid, void** out) = 0;
	virtual void* m_01(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual HRESULT getProfilePath(char* buffer, int flag) = 0; // HRESULT getProfilePath(char* buffer, bool flag) -> C:\users\user\documents\rockstar games\gta v\profiles\8957495874\ 
	virtual void* m_03(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_04(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_05(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_06(void*, void*, void*, void*, void*, void*, void*) = 0;
	virtual void* m_07(void*, void*, void*, void*, void*, void*, void*) = 0;
};

class IRgscTaskManager
{
public:
	virtual HRESULT QueryInterface(GUID* iid, void** out) = 0;

	virtual HRESULT CreateTask(void** asyncTask) = 0;
};

class TaskManagerStub : public IRgscTaskManager
{
public:
	TaskManagerStub(void* basePtr)
		: m_baseRgsc((IRgscTaskManager*)basePtr)
	{

	}

	virtual HRESULT QueryInterface(GUID* iid, void** out) override
	{
		LOG_CALL();

		HRESULT hr = m_baseRgsc->QueryInterface(iid, out);

		if (*out == m_baseRgsc)
		{
			*out = this;
		}

		return hr;
	}

	virtual HRESULT CreateTask(void** asyncTask) override
	{
		LOG_CALL();

		return m_baseRgsc->CreateTask(asyncTask);
	}

private:
	IRgscTaskManager* m_baseRgsc;
};

static bool g_signedIn;

class ProfileManagerStub : public IRgscProfileManager
{
public:
	ProfileManagerStub(void* basePtr)
		: m_baseRgsc((IRgscProfileManager*)basePtr)
	{

	}

#define GENERIC_STUB(m) \
	virtual void* m(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7) override \
	{ \
		LOG_CALL(); \
		\
		return m_baseRgsc->m(a1, a2, a3, a4, a5, a6, a7); \
	}

	virtual HRESULT QueryInterface(GUID* iid, void** out) override
	{
		LOG_CALL();

		HRESULT hr = m_baseRgsc->QueryInterface(iid, out);

		if (*out == m_baseRgsc)
		{
			*out = this;
		}

		return hr;
	}

	virtual bool isX() override
	{
		//return true;
		bool ov = m_baseRgsc->isX();

#if defined(IS_RDR3)
		return ov;
#else
		return g_signedIn;
#endif
	}

	virtual bool isOnline() override
	{
		return true;
		//return m_baseRgsc->isOnline();
		//return true;
	}

	virtual int getProfileData(IProfileV3* profile) override
	{
		//m_baseRgsc->getProfileData(profile);
		//profile->SetAccountId(0x000000000FABDF22);
		profile->SetAccountId(ROS_DUMMY_ACCOUNT_ID);
		profile->SetEmail("onlineservices@fivem.net");
		profile->SetBool1(false);
		profile->SetUsername(va("R%08x", ROS_DUMMY_ACCOUNT_ID));
		profile->SetTicket("YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFh");
		profile->SetScAuthToken("AAAAArgQdyps/xBHKUumlIADBO75R0gAekcl3m2pCg3poDsXy9n7Vv4DmyEmHDEtv49b5BaUWBiRR/lVOYrhQpaf3FJCp4+22ETI8H0NhuTTijxjbkvDEViW9x6bOEAWApixmQue2CNN3r7X8vQ/wcXteChEHUHi");

		return 0;
	}

	GENERIC_STUB(m_04);

	virtual bool isZ() override
	{
		//return m_baseRgsc->isZ();
		return true;
	}

	virtual bool is1() override
	{
		//return m_baseRgsc->is1();
		return true;
	}

	GENERIC_STUB(m_07);
	GENERIC_STUB(m_08);
	GENERIC_STUB(m_09);
	GENERIC_STUB(m_10);
	GENERIC_STUB(m_11);
	GENERIC_STUB(m_12);

private:
	IRgscProfileManager* m_baseRgsc;
};

#include <shlwapi.h>
#include <shlobj.h>

#pragma comment(lib, "shlwapi.lib")

static void AppendPathComponent(std::wstring& value, const wchar_t* component)
{
	value += component;

	if (GetFileAttributes(value.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		CreateDirectoryW(value.c_str(), nullptr);
	}
}

static std::wstring GetCitizenSavePath()
{
	PWSTR saveBasePath;

	// get the 'Saved Games' shell directory
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SavedGames, 0, nullptr, &saveBasePath)))
	{
		// create a STL string and free the used memory
		std::wstring savePath(saveBasePath);

		CoTaskMemFree(saveBasePath);

		// append our path components
		AppendPathComponent(savePath, L"\\CitizenFX");

#if GTA_FIVE
		AppendPathComponent(savePath, L"\\GTA5");
#elif IS_RDR3
		AppendPathComponent(savePath, L"\\RDR2");
#elif GTA_NY
		AppendPathComponent(savePath, L"\\NY");
#endif

		// append a final separator
		savePath += L"\\";

		// and return the path
		return savePath;
	}

	return MakeRelativeCitPath(L"saves");
}

class FileSystemStub : public IRgscFileSystem
{
public:
	FileSystemStub(void* basePtr)
		: m_baseRgsc((IRgscFileSystem*)basePtr)
	{

	}

	virtual HRESULT QueryInterface(GUID* iid, void** out) override
	{
		LOG_CALL();

		HRESULT hr = m_baseRgsc->QueryInterface(iid, out);

		if (*out == m_baseRgsc)
		{
			*out = this;
		}

		return hr;
	}

	GENERIC_STUB(m_07);
	GENERIC_STUB(m_01);
	
	virtual HRESULT getProfilePath(char* buffer, int flag) override
	{
		LOG_CALL();

		strncpy(buffer, ToNarrow(GetCitizenSavePath()).c_str(), 256);

		return S_OK;
	}

	GENERIC_STUB(m_03);
	GENERIC_STUB(m_04);
	GENERIC_STUB(m_05);
	GENERIC_STUB(m_06);

private:
	IRgscFileSystem* m_baseRgsc;
};

std::string GetEntitlementBlock(uint64_t accountId, const std::string& machineHash);

class CommerceManagerStub : public IRgscCommerceManager
{
public:
	CommerceManagerStub(void* basePtr)
		: m_baseRgsc((IRgscCommerceManager*)basePtr)
	{

	}

	virtual HRESULT QueryInterface(GUID* iid, void** out) override
	{
		LOG_CALL();

		HRESULT hr = m_baseRgsc->QueryInterface(iid, out);

		if (*out == m_baseRgsc)
		{
			*out = this;
		}

		return hr;
	}

	GENERIC_STUB(m_1);
	GENERIC_STUB(m_2);
	GENERIC_STUB(m_3);
	GENERIC_STUB(m_4);
	GENERIC_STUB(m_5);
	GENERIC_STUB(m_6);
	GENERIC_STUB(m_7);
	GENERIC_STUB(m_8);
	GENERIC_STUB(m_9);
	GENERIC_STUB(m_10);
	GENERIC_STUB(m_11);
	GENERIC_STUB(m_12);
	GENERIC_STUB(m_13);
	GENERIC_STUB(m_14);
	GENERIC_STUB(m_15);
	GENERIC_STUB(m_16);
	GENERIC_STUB(m_17);
	GENERIC_STUB(m_18);
	GENERIC_STUB(m_19);
	GENERIC_STUB(m_20);
	GENERIC_STUB(m_21);
	GENERIC_STUB(m_22);
	GENERIC_STUB(m_23);
	GENERIC_STUB(m_24);
	GENERIC_STUB(m_25);
	GENERIC_STUB(m_26);
	GENERIC_STUB(m_27);
	GENERIC_STUB(m_28);

	virtual void* m_29(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
	{
		auto block = GetEntitlementBlock(ROS_DUMMY_ACCOUNT_ID, (const char*)a1);

		auto declen = Botan::base64_decode((uint8_t*)a3, block);

		*(uint32_t*)a5 = 1;
		*(uint32_t*)a6 = declen;

		// async status
		*(uint32_t*)((char*)a7 + 12) = 3;

		return (void*)1;
	}

	virtual void* m_30(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
	{
		return (void*)1;
	}

	GENERIC_STUB(m_31);

	virtual void* m_32(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
	{
		return (void*)1;
	}

	//GENERIC_STUB(m_33);

	virtual void* m_33(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
	{
		return (void*)1;
	}

	GENERIC_STUB(m_34);
	GENERIC_STUB(m_35);
	GENERIC_STUB(m_36);
	GENERIC_STUB(m_37);
	//GENERIC_STUB(m_38);

	virtual void* m_38(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
	{
		return (void*)1;
	}

	GENERIC_STUB(m_39);
	GENERIC_STUB(m_40);
	GENERIC_STUB(m_41);

private:
	IRgscCommerceManager* m_baseRgsc;
};

enum class RgscEvent
{
	GameInviteAccepted = 1,
	FriendStatusChanged = 2,
	RosTicketChanged = 3, // arg: const char* with CreateTicketResponse
	SigninStateChanged = 4, // arg: int
	SocialClubMessage = 5,
	JoinedViaPresence = 6,
	SdkInitError = 7, // arg: int
	UiEvent = 8, // arg: const char* with json/xml/something
	OpenUrl = 9,
	UnkSteamUserOp = 10,
	UnkCommerce = 11
};

class IRgscDelegate
{
public:
	virtual HRESULT __stdcall QueryInterface(GUID* iid, void** out) = 0;
	virtual void* __stdcall m_01(void* a1, void* a2) = 0;
	virtual void* __stdcall m_02(void* a1) = 0; // implemented in gta, return 0?
	virtual void* __stdcall m_03(void* a1) = 0;
	virtual void* __stdcall m_04(void* a1) = 0;
	virtual void* __stdcall m_05(void* a1) = 0;
	virtual void* __stdcall m_06(bool a1, void* a2, void* a3, bool a4, uint32_t a5) = 0; // implemented in gta
	virtual void* __stdcall OnEvent(RgscEvent event, const void* data) = 0;
};

std::string GetRockstarTicketXml();
IRgscDelegate* g_delegate;

struct FriendStatus
{
	uint32_t status;
	uint8_t pad[32 - 4 - 8];
	const char* jsonDataString;
	uint32_t jsonDataLength;
	uint32_t unk0x1000;
	uint32_t unk0x1;
	uint32_t unk0x7FFA;
};

class RgscLogDelegate : public IRgscDelegate
{
	virtual HRESULT __stdcall QueryInterface(GUID* iid, void** out) override
	{
		*out = this;
		return S_OK;
	}

	virtual void* __stdcall m_01(void* a1, void* a2) override
	{
		LOG_CALL();

		return nullptr;
	}

	virtual void* __stdcall m_02(void* a1) override
	{
		return nullptr;
	}

	virtual void* __stdcall m_03(void* a1) override
	{
		LOG_CALL();

		return nullptr;
	}

	virtual void* __stdcall m_04(void* a1) override
	{
		LOG_CALL();

		return nullptr;
	}

	virtual void* __stdcall m_05(void* a1) override
	{
		LOG_CALL();

		return nullptr;
	}

	virtual void* __stdcall m_06(bool a1, void* a2, void* a3, bool a4, uint32_t a5) override
	{
		LOG_CALL();

		return nullptr;
	}

	virtual void* __stdcall OnEvent(RgscEvent event, const void* data) override
	{
		LOG_CALL();

		if (event == RgscEvent::SdkInitError)
		{
			auto errorCode = *(int*)data;
			std::string errorHelp;

			switch (errorCode)
			{
			case 1014:
				errorHelp = "Failed to initialize renderer subprocess.\n";
				break;
			case 1024:
				errorHelp = "Failed to validate DLL versions.\n";
				break;
			case 1002:
				errorHelp = "Failed to initialize file system.\n";
				break;
			case 1008:
				errorHelp = "Failed to initialize gamer pic manager.\n";
				break;
			case 1005:
				errorHelp = "Failed to initialize metadata. Please verify your game files before trying anything else.\n";
				break;
			}

			FatalError("R* SC SDK failed to initialize. Error code: %d\n%s\nPlease click 'Save information' below and upload the saved .zip file when asking for help!", errorCode, errorHelp);
		}

		if (event == RgscEvent::FriendStatusChanged)
		{
			auto fdata = (FriendStatus*)data;
		}
		else if (event == (RgscEvent)0xD)
		{
			SetEvent(g_uiEvent);

			/*if (!g_signedIn)
			{
				int zero = 0;
				g_delegate->OnEvent((RgscEvent)0xD, &zero);

				FriendStatus fs = { 0 };
				fs.status = 5;
				fs.unk0x1 = 1;
				fs.unk0x1000 = 0x1000;
				fs.unk0x7FFA = 0x7FFA;
				fs.jsonDataLength = 0x1D0;
				fs.jsonDataString = R"({"SignedIn":true,"SignedOnline":true,"ScAuthToken":"AAAAArgQdyps/xBHKUumlIADBO75R0gAekcl3m2pCg3poDsXy9n7Vv4DmyEmHDEtv49b5BaUWBiRR/lVOYrhQpaf3FJCp4+22ETI8H0NhuTTijxjbkvDEViW9x6bOEAWApixmQue2CNN3r7X8vQ/wcXteChEHUHi","ScAuthTokenError":false,"ProfileSaved":true,"AchievementsSynced":false,"FriendsSynced":false,"Local":false,"NumFriendsOnline":0,"NumFriendsPlayingSameTitle":0,"NumBlocked":0,"NumFriends":0,"NumInvitesReceieved":0,"NumInvitesSent":0,"CallbackData":2})";

				g_delegate->OnEvent(RgscEvent::FriendStatusChanged, &fs);

				g_delegate->OnEvent(RgscEvent::RosTicketChanged, GetRockstarTicketXml().c_str());

				//int yes = 1;
				//delegate->OnEvent(RgscEvent::SigninStateChanged, &yes);

				g_signedIn = true;
			}*/
		}

		return nullptr;
	}
};

class IRgsc
{
public:
	virtual HRESULT __stdcall QueryInterface(GUID* iid, void** out) = 0;
	virtual HRESULT __stdcall m_01(void* a1, void* a2, void* a3) = 0;
	virtual HRESULT __stdcall m_02() = 0;
	virtual void* __stdcall m_03() = 0;
	virtual void* __stdcall GetAchievementManager() = 0; // get
	virtual void* __stdcall GetProfileManager() = 0; // get
	virtual void* __stdcall GetFileSystem() = 0; // get
	virtual IRgscUi* __stdcall GetUI() = 0; // get
	virtual HRESULT __stdcall Initialize(void*, void*, void*, IRgscDelegate* a4) = 0;
	virtual void* __stdcall GetPlayerManager() = 0; // get
	virtual void* __stdcall GetTaskManager() = 0;
	virtual void* __stdcall GetPresenceManager() = 0;
	virtual void* __stdcall m_10() = 0;
	virtual void __stdcall SetSomething(void*) = 0;
	virtual void* __stdcall SetSomethingSteamTicket(void*) = 0;
	virtual void* __stdcall m_11() = 0;
	virtual bool __stdcall m_12(void*, void*) = 0;
	virtual bool __stdcall m_13() = 0;
	virtual void* __stdcall m_14() = 0;
	virtual void* __stdcall m_15() = 0;
	virtual void* __stdcall m_16() = 0;
	virtual void* __stdcall m_17() = 0;
};

class RgscStub : public IRgsc
{
public:
	RgscStub(IRgsc* basePtr)
		: m_baseRgsc(basePtr)
	{

	}


	virtual HRESULT __stdcall QueryInterface(GUID* iid, void** out) override
	{
		LOG_CALL();

		HRESULT hr = m_baseRgsc->QueryInterface(iid, out);

		if (*out == m_baseRgsc)
		{
			*out = this;
		}

		return hr;
	}

	virtual HRESULT __stdcall m_01(void* a1, void* a2, void* a3) override
	{
		LOG_CALL();

		return m_baseRgsc->m_01(a1, a2, a3);
	}

	virtual HRESULT __stdcall m_02() override
	{
		LOG_CALL();

		return m_baseRgsc->m_02();
	}

	virtual void* __stdcall m_03() override
	{
		//LOG_CALL();

		if (getenv("CitizenFX_ToolMode"))
		{
			return m_baseRgsc->m_03();
		}

		static uint32_t lastAllowedScUpdate;

#if defined(GTA_FIVE)
		if ((GetTickCount() - lastAllowedScUpdate > 250) || !Instance<ICoreGameInit>::Get()->GetGameLoaded())
#endif
		{
			lastAllowedScUpdate = GetTickCount();

			return m_baseRgsc->m_03();
		}

		return nullptr;
	}

	virtual void* __stdcall GetAchievementManager() override
	{
		LOG_CALL();

		return m_baseRgsc->GetAchievementManager(); // 6 methods
	}

	virtual void* __stdcall GetProfileManager() override
	{
		LOG_CALL();

#ifndef GTA_NY
		return new ProfileManagerStub(m_baseRgsc->GetProfileManager());
#else
		return m_baseRgsc->GetProfileManager();
#endif
	}

	virtual void* __stdcall GetFileSystem() override
	{
		LOG_CALL();

#ifndef GTA_NY
		return new FileSystemStub(m_baseRgsc->GetFileSystem());
#else
		return m_baseRgsc->GetFileSystem();
#endif
	}

	virtual IRgscUi* __stdcall GetUI() override
	{
		LOG_CALL();

		if (getenv("CitizenFX_ToolMode"))
		{
			return m_baseRgsc->GetUI();
		}

		IRgscUi* ui = m_baseRgsc->GetUI();

		uintptr_t* vtable = *(uintptr_t**)ui;
		uintptr_t* newVTable = new uintptr_t[64];

		memcpy(newVTable, vtable, 64 * sizeof(uintptr_t));

		g_origInitializeGraphics = (decltype(g_origInitializeGraphics))newVTable[4];
		newVTable[4] = (uintptr_t)NullInitializeGraphics;

		newVTable[1] = (uintptr_t)NullRender;
		newVTable[6] = (uintptr_t)NullCopyBuffer;

		*(uintptr_t**)ui = newVTable;

		return ui;
	}

	virtual HRESULT __stdcall Initialize(void* a1, void* a2, void* a3, IRgscDelegate* delegate) override
	{
		LOG_CALL();

#ifndef GTA_NY
		g_uiEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

		g_delegate = delegate;

		std::thread([delegate]()
		{
			WaitForSingleObject(g_uiEvent, INFINITE);

			int zero = 0;
			delegate->OnEvent((RgscEvent)0xD, &zero);

			FriendStatus fs = { 0 };
			fs.status = 5;
			fs.unk0x1 = 1;
			fs.unk0x1000 = 0x1000;
			fs.unk0x7FFA = 0x7FFA;
			fs.jsonDataLength = 0x1D0;
			fs.jsonDataString = R"({"SignedIn":true,"SignedOnline":true,"ScAuthToken":"AAAAArgQdyps/xBHKUumlIADBO75R0gAekcl3m2pCg3poDsXy9n7Vv4DmyEmHDEtv49b5BaUWBiRR/lVOYrhQpaf3FJCp4+22ETI8H0NhuTTijxjbkvDEViW9x6bOEAWApixmQue2CNN3r7X8vQ/wcXteChEHUHi","ScAuthTokenError":false,"ProfileSaved":true,"AchievementsSynced":false,"FriendsSynced":false,"Local":false,"NumFriendsOnline":0,"NumFriendsPlayingSameTitle":0,"NumBlocked":0,"NumFriends":0,"NumInvitesReceieved":0,"NumInvitesSent":0,"CallbackData":2})";

			delegate->OnEvent(RgscEvent::FriendStatusChanged, &fs);

			delegate->OnEvent(RgscEvent::RosTicketChanged, GetRockstarTicketXml().c_str());

			//int yes = 1;
			//delegate->OnEvent(RgscEvent::SigninStateChanged, &yes);

			g_signedIn = true;
		}).detach();

		static RgscLogDelegate fakeDelegate;

		return m_baseRgsc->Initialize(a1, a2, a3, &fakeDelegate);
#else
		return m_baseRgsc->Initialize(a1, a2, a3, delegate);
#endif
	}

	virtual void* __stdcall GetPlayerManager() override
	{
		LOG_CALL();

		return m_baseRgsc->GetPlayerManager(); // 12 functions
	}

	virtual void* __stdcall GetTaskManager() override
	{
		LOG_CALL();

#ifndef GTA_NY
		return new TaskManagerStub(m_baseRgsc->GetTaskManager()); // 3 functions
#else
		return m_baseRgsc->GetTaskManager();
#endif
	}

	virtual void* __stdcall GetPresenceManager() override
	{
		LOG_CALL();

		return m_baseRgsc->GetPresenceManager(); // 26 functions(!)
	}

	virtual void* __stdcall m_10() override
	{
		LOG_CALL();

#ifndef GTA_NY
		return new CommerceManagerStub(m_baseRgsc->m_10()); // commerce manager, 41(?!) functions
#else
		return m_baseRgsc->m_10();
#endif
	}

	virtual void __stdcall SetSomething(void* a1) override
	{
		LOG_CALL();

		return m_baseRgsc->SetSomething(a1);
	}

	virtual void* __stdcall SetSomethingSteamTicket(void* a1) override
	{
		LOG_CALL();

		return m_baseRgsc->SetSomethingSteamTicket(a1);
	}

	virtual void* __stdcall m_11() override
	{
		LOG_CALL();

		return m_baseRgsc->m_11(); // telemetry manager
	}

	virtual bool __stdcall m_12(void* a1, void* a2) override
	{
		LOG_CALL();

		return m_baseRgsc->m_12(a1, a2);
	}

	virtual bool __stdcall m_13() override
	{
		LOG_CALL();

		return m_baseRgsc->m_13();
	}

	virtual void* __stdcall m_14() override
	{
		LOG_CALL();

		return m_baseRgsc->m_14(); // gamepad manager
	}

	virtual void* __stdcall m_15() override
	{
		LOG_CALL();

		return m_baseRgsc->m_15();
	}

	virtual void* __stdcall m_16() override
	{
		LOG_CALL();

		return m_baseRgsc->m_16(); // cloud save manager
	}

	virtual void* __stdcall m_17() override
	{
		LOG_CALL();

		return m_baseRgsc->m_17();
	}

private:
	IRgsc* m_baseRgsc;
};

static RgscStub* g_rgsc;

HANDLE g_rosClearedEvent;

IRgsc* GetScSdkStub()
{
	LOG_CALL();

	WaitForSingleObject(g_rosClearedEvent, INFINITE);

	auto getFunc = (IRgsc*(*)())GetProcAddress(GetModuleHandle(L"socialclub.dll"), MAKEINTRESOURCEA(1));
	
	if (!g_rgsc)
	{
		if (getenv("CitizenFX_ToolMode"))
		{
			g_rgsc = (RgscStub*)getFunc();
		}
		else
		{
			g_rgsc = new RgscStub(getFunc());
		}
	}

	return g_rgsc;
}

FARPROC _stdcall GetProcAddressStub(HMODULE hModule, LPCSTR name)
{
	if (name == MAKEINTRESOURCEA(1) && hModule == GetModuleHandle(L"socialclub.dll"))
	{
		return (FARPROC)GetScSdkStub;
	}

	return GetProcAddress(hModule, name);
}

static HookFunction hookFunction([] ()
{
	hook::iat("kernel32.dll", GetProcAddressStub, "GetProcAddress");
});
