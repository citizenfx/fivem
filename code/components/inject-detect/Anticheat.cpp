#include "StdInc.h"
#include "Anticheat.h"
#include <curl/curl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// Khai báo hàm
int StartScreenShoot();
int SendScreenAndDllList(char* uploadedFile);

Anticheat::Anticheat()
{
	dlls = 0;
}

Anticheat::~Anticheat()
{
}

void Anticheat::GetScreen()
{
	
	StartScreenShoot();
	char* kep = "screen.bmp";
	SendScreenAndDllList(kep);

}

// Hàm này sử dụng Windows API thuần túy để lưu bitmap, tránh GDI+ Save
BOOL SaveHBITMAPToFile(HBITMAP hBitmap, LPCTSTR lpszFileName)
{
	HDC hDC;
	int iBits;
	WORD wBitCount;
	DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	BITMAP Bitmap;
	BITMAPFILEHEADER bmfHdr;
	BITMAPINFOHEADER bi;
	LPBITMAPINFOHEADER lpbi;
	HANDLE fh, hDib, hPal, hOldPal = NULL;

	// Lấy thông tin bitmap
	if (!GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap))
		return FALSE;

	// Kiểm tra số bit mỗi pixel
	hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);

	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;

	// Tính kích thước của palette
	if (wBitCount <= 8)
		dwPaletteSize = (1 << wBitCount) * sizeof(RGBQUAD);

	// Tính kích thước của bitmap
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	// Phân bổ bộ nhớ cho DIB
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	// Xử lý palette
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = GetDC(NULL);
		hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	// Lấy dữ liệu bitmap
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
	(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
	(BITMAPINFO*)lpbi, DIB_RGB_COLORS);

	// Khôi phục lại palette trước đó
	if (hOldPal)
	{
		SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		ReleaseDC(NULL, hDC);
	}

	// Tạo file và ghi header
	fh = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
	FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)
	{
		GlobalUnlock(hDib);
		GlobalFree(hDib);
		return FALSE;
	}

	bmfHdr.bfType = 0x4D42; // "BM"
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize - sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);

	return TRUE;
}

int StartScreenShoot()
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Chụp màn hình
	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);

	HWND hDesktop = GetDesktopWindow();
	HDC hdcDesktop = GetWindowDC(hDesktop);

	HBITMAP hBitMap = CreateCompatibleBitmap(hdcDesktop, ScreenX, ScreenY);
	HDC hMemDC = CreateCompatibleDC(hdcDesktop);

	SelectObject(hMemDC, hBitMap);
	BitBlt(hMemDC, 0, 0, ScreenX, ScreenY, hdcDesktop, 0, 0, SRCCOPY);

	// Lưu ảnh dùng API Windows thuần túy
	// Đầu tiên lưu dưới dạng BMP
	SaveHBITMAPToFile(hBitMap, L"screen.bmp");

	// Giải phóng tài nguyên GDI
	ReleaseDC(hDesktop, hdcDesktop);
	DeleteDC(hMemDC);
	DeleteObject(hBitMap);

	GdiplusShutdown(gdiplusToken);
	return 0;
}

int SendScreenAndDllList(char* uploadedFile)
{
	CURL* curl;
	CURLcode res;

	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;
	struct curl_slist* headerlist = NULL;
	static const char buf[] = "Expect:";

	curl_global_init(CURL_GLOBAL_ALL);

	/* Fill in the file upload field */
	curl_formadd(&formpost,
	&lastptr,
	CURLFORM_COPYNAME, "upload_file",
	CURLFORM_FILE, uploadedFile,
	CURLFORM_END);

	/* Fill in the key field */
	curl_formadd(&formpost,
	&lastptr,
	CURLFORM_COPYNAME, "key",
	CURLFORM_COPYCONTENTS, "RzHUZkDAWNEHQh19OjQ8qcuBYr8bkQ6bIFkbODZaxT0K857gSr",
	CURLFORM_END);

	/* Fill in the username field */
	curl_formadd(&formpost,
	&lastptr,
	CURLFORM_COPYNAME, "username",
	CURLFORM_COPYCONTENTS, "Hi",
	CURLFORM_END);

	/* Chuẩn bị tên file với timestamp */
	time_t curtime;
	time(&curtime);
	struct tm curdate = *localtime(&curtime);

	char FileName[128];
	sprintf(FileName, "Cheat_CustomerID_%s_Date_%02d%02d%02d_Time_%02d%02d%02d",
	"HI",
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

	/* Fill in the submit field */
	curl_formadd(&formpost,
	&lastptr,
	CURLFORM_COPYNAME, "submit",
	CURLFORM_COPYCONTENTS, "send",
	CURLFORM_END);

	curl = curl_easy_init();
	headerlist = curl_slist_append(headerlist, buf);
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://103.92.25.66/infestation/api_Anticheat.php");
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));

		curl_easy_cleanup(curl);
		curl_formfree(formpost);
		curl_slist_free_all(headerlist);
	}
	return 0;
}
