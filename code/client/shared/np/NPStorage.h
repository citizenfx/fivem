// ==========================================================
// alterIWnet project
// 
// Component: xnp
// Sub-component: libnp
// Purpose: definitions for the NP storage service
//
// Initial author: NTAuthority
// Started: 2011-07-17
// ==========================================================

enum EGetFileResult
{
	GetFileResultOK = 0,
	GetFileResultNotFound = 1,
	GetFileResultNotAllowed = 2,
	GetFileResultServiceError = 3
};

enum EWriteFileResult
{
	WriteFileResultOK = 0,
	WriteFileResultNotAllowed = 1,
	WriteFileResultServiceError = 2
};

class NPGetPublisherFileResult
{
public:
	// the request result
	EGetFileResult result;

	// the amount of bytes written to the buffer
	uint32_t fileSize;

	// the buffer passed to NP_GetPublisherFile()
	uint8_t* buffer;
};

class NPGetUserFileResult
{
public:
	// the request result
	EGetFileResult result;

	// the amount of bytes written to the buffer
	uint32_t fileSize;

	// the buffer passed to NP_GetUserFile()
	uint8_t* buffer;
};

class NPWriteUserFileResult
{
public:
	// the request result
	EWriteFileResult result;
};

class NPSendRandomStringResult
{
public:
	// the string buffer passed in
	const char* result;
};