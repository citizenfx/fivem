#pragma once

#include <string_view>

namespace fx
{
class ModalWindow
{
public:
	// Displays an error prompt window to the user with the given title and body.
	static void DisplayError(std::wstring_view title, std::wstring_view body);
	
	// Displays an error prompt window from a CURL request, will add extra information and potential solutions for known issues.
	static void DisplayCurlError(std::wstring_view title, std::wstring_view body, int errorCode, std::wstring_view url);
};
}
