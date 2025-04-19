#include "StdInc.h"

#include <filesystem>
#include <lua.hpp>
#include <lua_cmsgpacklib.h>

#include "FilesystemPermissions.h"
#include "LuaFXLib.h"

#include "LuaScriptRuntime.h"
#include "VFSManager.h"

namespace
{
// flockfile
#define l_lockfile(f)		((void)0)
// funlockfile
#define l_unlockfile(f)		((void)0)
typedef luaL_Stream LStream;
#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))
#define isclosed(p)	((p)->closef == NULL)

int LuaIOFileResultError(lua_State* L, int stat, const char* fname)
{
	lua_pushnil(L);
	if (fname)
	{
		lua_pushfstring(L, "%s: %s", fname, strerror(stat));
	}
	else
	{
		lua_pushstring(L, strerror(stat));
	}

	lua_pushinteger(L, stat);
	return 3;
}

int io_type(lua_State* L)
{
	LStream* p;
	luaL_checkany(L, 1);
	p = static_cast<LStream*>(luaL_testudata(L, 1, LUA_FILEHANDLE));
	if (p == nullptr)
	{
		lua_pushnil(L); /* not a file */
	}
	else if (isclosed(p))
	{
		lua_pushliteral(L, "closed file");
	}
	else
	{
		lua_pushliteral(L, "file");
	}

	return 1;
}

int f_tostring(lua_State* L)
{
	LStream* p = tolstream(L);
	if (isclosed(p))
	{
		lua_pushliteral(L, "file (closed)");
	}
	else
	{
		lua_pushfstring(L, "file (%p)", p->f);
	}

	return 1;
}

vfs::Stream* LuaIOToFile(lua_State* L)
{
	const LStream* p = tolstream(L);
	if (isclosed(p))
	{
		luaL_error(L, "attempt to use a closed file");
	}

	lua_assert(p->f);
	return reinterpret_cast<vfs::Stream*>(p->f);
}

#define LUA_FX_DIRECTORY_HANDLE          "DIRECTORY*"

struct LuaIODirectoryFile
{
	size_t length;
	char* fileName;
};

struct LuaIODirectory
{
	size_t size;
	LuaIODirectoryFile* directory;
};

LuaIODirectory* LuaIONewDirectoryContent(lua_State* L)
{
	LuaIODirectory* p = static_cast<LuaIODirectory*>(lua_newuserdata(L, sizeof(LuaIODirectory)));
	luaL_setmetatable(L, LUA_FX_DIRECTORY_HANDLE);
	return p;
}

int LuaIODirectoryGC(lua_State* L)
{
	LuaIODirectory* p = static_cast<LuaIODirectory*>(luaL_checkudata(L, 1, LUA_FX_DIRECTORY_HANDLE));
	if (!p)
	{
		return 0;
	}

	if (p->directory)
	{
		for (size_t i = 0; i < p->size; i++)
		{
			delete[] p->directory[i].fileName;
		}

		free(p->directory);
		p->directory = nullptr;
	}

	return 0;
}

int LuaIODirectoryClose(lua_State* L)
{
	LuaIODirectory* p = static_cast<LuaIODirectory*>(luaL_checkudata(L, 1, LUA_FX_DIRECTORY_HANDLE));
	if (p && p->directory)
	{
		for (size_t i = 0; i < p->size; i++)
		{
			delete[] p->directory[i].fileName;
		}

		free(p->directory);
		p->directory = nullptr;
	}

	return luaL_fileresult(L, true, nullptr);
}

int LuaIODirectoryToString(lua_State* L)
{
	LuaIODirectory* p = static_cast<LuaIODirectory*>(luaL_checkudata(L, 1, LUA_FX_DIRECTORY_HANDLE));
	if (p && p->directory)
	{
		std::string directoryString{};
		for (size_t i = 0; i < p->size; i++)
		{
			directoryString += std::string_view{p->directory[i].fileName, p->directory[i].length};
			directoryString += "\n";
		}

		lua_pushstring(L, directoryString.c_str());
		return 1;
	}

	lua_pushliteral(L, "directory (closed)");
	return 1;
}

int LuaIODirectoryIterator(lua_State* L)
{
	LuaIODirectory* p = static_cast<LuaIODirectory*>(lua_touserdata(L, lua_upvalueindex(1)));
	size_t* index = static_cast<size_t*>(lua_touserdata(L, lua_upvalueindex(2)));

	if (*index < p->size)
	{
		lua_pushstring(L, p->directory[*index].fileName);
		(*index)++;
		return 1;
	}

	// No more lines
	return 0;
}

int LuaIODirectoryLines(lua_State* L)
{
	LuaIODirectory* p = static_cast<LuaIODirectory*>(luaL_checkudata(L, 1, LUA_FX_DIRECTORY_HANDLE));
	luaL_argcheck(L, p != nullptr && p->directory != nullptr, 1, "directory is closed");

	lua_pushvalue(L, 1); // Push the directory userdata
	size_t* index = static_cast<size_t*>(lua_newuserdata(L, sizeof(size_t)));
	*index = 0; // Initialize the index

	lua_pushcclosure(L, LuaIODirectoryIterator, 2); // Create closure with 2 upvalues
	return 1;
}

/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
int aux_close(lua_State* L)
{
	LStream* p = tolstream(L);
	volatile lua_CFunction cf = p->closef;
	p->closef = nullptr; /* mark stream as closed */
	return (*cf)(L); /* close it */
}

int LuaIOClose(lua_State* L)
{
	LuaIOToFile(L); /* make sure argument is an open stream */
	return aux_close(L);
}

int f_gc(lua_State* L)
{
	LStream* p = tolstream(L);
	if (!isclosed(p) && p->f != nullptr)
	{
		aux_close(L); /* ignore closed and incompletely open files */
	}

	return 0;
}

int LuaVFSStreamClose(lua_State* L)
{
	LStream* p = tolstream(L);
	int res = fx::Lua_EACCES;
	if (p->f)
	{
		fwRefContainer<vfs::Stream> stream = reinterpret_cast<vfs::Stream*>(p->f);
		stream->Close();
		// should always reach zero here
		stream->Release();
		res = 0;
	}

	return luaL_fileresult(L, (res == 0), nullptr);
}

/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** file is not left opened.
*/
LStream* LuaNewPreFile(lua_State* L)
{
	LStream* p = static_cast<LStream*>(lua_newuserdata(L, sizeof(LStream)));
	p->closef = nullptr; /* mark file handle as 'closed' */
	luaL_setmetatable(L, LUA_FILEHANDLE);
	return p;
}

int LuaIOOpen(lua_State* L)
{
	const char* file = luaL_optstring(L, 1, nullptr);
	std::string fileName = file;
	const char* modeStr = luaL_optstring(L, 2, nullptr);
	std::string mode = "r";
	if (modeStr)
	{
		mode = modeStr;
	}

	bool write = false;
	bool append = false;
	bool create = false;

	if (mode.find('a') != std::string::npos)
	{
		append = true;
		write = true;
	}

	if (mode.find('+') != std::string::npos)
	{
		create = true;
	}

	if (mode.find('w') != std::string::npos)
	{
		write = true;
	}

	fwRefContainer<vfs::Device> device = !fileName.empty() && fileName[0] == '@' ? vfs::GetDevice(fileName) : nullptr;
	std::string path = fileName;
	if (!device.GetRef())
	{
		device = vfs::FindDevice(fileName, path);
		if (!device.GetRef())
		{
			return LuaIOFileResultError(L, ENOENT, file);
		}
	}

	if ((write || create || append) && !fx::ScriptingFilesystemAllowWrite(path))
	{
		write = false;
		create = false;
		append = false;
	}

	uintptr_t handle;
	if (create)
	{
		handle = device->Create(path, false, append);
		if (handle == INVALID_DEVICE_HANDLE)
		{
			// open in case already exists
			handle = device->Open(path, false, append);
		}
	}
	else
	{
		handle = device->Open(path, !write, append);
		// try creation if its write mode and the file could not be opened
		if (write && handle == INVALID_DEVICE_HANDLE)
		{
			handle = device->Create(path, false, append);
		}
	}

	if (handle != INVALID_DEVICE_HANDLE)
	{
		fwRefContainer<vfs::Stream> deviceStream = new vfs::Stream(device, handle);
		deviceStream->AddRef();

		LStream* p = LuaNewPreFile(L);
		p->closef = &LuaVFSStreamClose;
		p->f = reinterpret_cast<FILE*>(deviceStream.GetRef());
		return 1;
	}

	return LuaIOFileResultError(L, ENOENT, file);
}

int LuaIOPOpen(lua_State* L)
{
	const char* commandString = luaL_checkstring(L, 1);
	std::string_view command = commandString;
	const char* mode = luaL_optstring(L, 2, "r");

	if (command.find("dir", 0) == 0)
	{
		command.remove_prefix(3);
	}
	else if (command.find("ls", 0) == 0)
	{
		command.remove_prefix(2);
	}
	else
	{
		return luaL_fileresult(L, 0, commandString);
	}

	const size_t pathStart = command.find('\"', 0);
	if (pathStart == std::string::npos)
	{
		return luaL_fileresult(L, 0, commandString);
	}

	const size_t pathEnd = command.find('\"', pathStart + 1);
	if (pathEnd == std::string::npos)
	{
		return luaL_fileresult(L, 0, commandString);
	}

	std::string_view commandPath = std::string_view{command.data() + pathStart + 1, pathEnd - pathStart - 1};
	std::string commandAbsolutePath = std::filesystem::absolute(std::filesystem::path(commandPath)).string();

	std::string transformedPath;
	fwRefContainer<vfs::Device> device = vfs::FindDevice(commandAbsolutePath, transformedPath);
	if (!device.GetRef())
	{
		return luaL_fileresult(L, 0, commandString);
	}

	vfs::FindData fd;
	auto handle = device->FindFirst(transformedPath, &fd);
	std::list<std::string> files{};
	if (handle != INVALID_DEVICE_HANDLE)
	{
		do
		{
			files.emplace_back(fd.name);
		}
		while (device->FindNext(handle, &fd));

		device->FindClose(handle);
	}

	LuaIODirectory* luaDirectory = LuaIONewDirectoryContent(L);
	luaDirectory->size = files.size();
	luaDirectory->directory = static_cast<LuaIODirectoryFile*>(calloc(files.size(), sizeof(LuaIODirectoryFile)));
	size_t fileIndex{0};
	for (std::string& fileName : files)
	{
		LuaIODirectoryFile& directoryFile = luaDirectory->directory[fileIndex++];
		directoryFile.fileName = new char[fileName.size() + 1];
		memcpy(directoryFile.fileName, fileName.c_str(), fileName.size() + 1);
		directoryFile.length = fileName.size();
	}

	return 1;
}

int LuaIOReadDir(lua_State* L)
{
	const char* directoryPathString = luaL_checkstring(L, 1);
	std::filesystem::path directoryPath = directoryPathString;
	// ensure that the path ends with a path seperator
	directoryPath /= "";

	fwRefContainer<vfs::Device> device = vfs::GetDevice(directoryPath.generic_string());
	if (!device.GetRef())
	{
		std::string directoryAbsolutePath = std::filesystem::absolute(std::filesystem::path(directoryPath)).string();
		std::string transformedPath;
		device = vfs::FindDevice(directoryAbsolutePath, transformedPath);
		if (device.GetRef())
		{
			directoryPath = transformedPath;
		}
	}

	if (!device.GetRef())
	{
		return luaL_fileresult(L, 0, directoryPathString);
	}

	vfs::FindData fd;
	auto handle = device->FindFirst(directoryPath.generic_string(), &fd);
	std::list<std::string> files{};
	if (handle != INVALID_DEVICE_HANDLE)
	{
		do
		{
			if (fd.name == "." || fd.name == "..")
			{
				continue;
			}

			files.emplace_back(fd.name);
		}
		while (device->FindNext(handle, &fd));

		device->FindClose(handle);
	}

	LuaIODirectory* luaDirectory = LuaIONewDirectoryContent(L);
	luaDirectory->size = files.size();
	luaDirectory->directory = static_cast<LuaIODirectoryFile*>(calloc(files.size(), sizeof(LuaIODirectoryFile)));
	size_t fileIndex{0};
	for (std::string& fileName : files)
	{
		LuaIODirectoryFile& directoryFile = luaDirectory->directory[fileIndex++];
		directoryFile.fileName = new char[fileName.size() + 1];
		memcpy(directoryFile.fileName, fileName.c_str(), fileName.size() + 1);
		directoryFile.length = fileName.size();
	}

	return 1;
}

int LuaIOTempFile(lua_State* L)
{
	LStream* p = LuaNewPreFile(L);
	p->f = nullptr;
	return p->f == nullptr ? luaL_fileresult(L, 0, nullptr) : 1;
}

int LuaIOGetC(const fwRefContainer<vfs::Stream>& f)
{
	unsigned char c;
	if (f->Read(&c, sizeof(char)) != sizeof(char))
	{
		return EOF;
	}

	return c;
}

size_t LuaIOUnGetC(int c, const fwRefContainer<vfs::Stream>& f)
{
	// no way to push it back in because we don't use buffered read atm
	return f->Seek(-1, SEEK_CUR);
}

constexpr size_t kLineBufferSize = 64 * 1024;

int LuaIOFileLineIterator(lua_State* L)
{
	const LStream* p = static_cast<LStream*>(lua_touserdata(L, lua_upvalueindex(1)));
	fwRefContainer<vfs::Stream> f = reinterpret_cast<vfs::Stream*>(p->f);
	uint8_t* buffer = static_cast<uint8_t*>(lua_touserdata(L, lua_upvalueindex(2)));
	size_t& bufferPos = *static_cast<size_t*>(lua_touserdata(L, lua_upvalueindex(3)));
	size_t& bufferEnd = *static_cast<size_t*>(lua_touserdata(L, lua_upvalueindex(4)));
	uint64_t length = f->GetLength();
	size_t currentPosition = f->Seek(0, SEEK_CUR);
	if (currentPosition >= length && bufferPos >= bufferEnd)
	{
		return 0;
	}

	luaL_Buffer lineBuffer;
	luaL_buffinit(L, &lineBuffer);

	while (true)
	{
		if (bufferPos >= bufferEnd)
		{
			bufferEnd = f->Read(buffer, kLineBufferSize);
			bufferPos = 0;

			if (bufferEnd == 0)
			{
				break;
			}
		}

		size_t lineStart = bufferPos;
		bool foundLineEnd = false;
		while (bufferPos < bufferEnd)
		{
			if (buffer[bufferPos] == '\n' || buffer[bufferPos] == '\r')
			{
				foundLineEnd = true;
				break;
			}

			bufferPos++;
		}

		luaL_addlstring(&lineBuffer, reinterpret_cast<const char*>(buffer + lineStart), bufferPos - lineStart);

		if (foundLineEnd)
		{
			// skip line ends when one was found
			while (bufferPos < bufferEnd && (buffer[bufferPos] == '\n' || buffer[bufferPos] == '\r'))
			{
				bufferPos++;
			}

			break;
		}
	}

	luaL_pushresult(&lineBuffer);
	return 1;
}

int LuaIOFileLines(lua_State* L)
{
	fwRefContainer<vfs::Stream> f = LuaIOToFile(L);

	lua_pushvalue(L, 1); // Push the file userdata
	uint8_t* buffer = static_cast<uint8_t*>(lua_newuserdata(L, kLineBufferSize));
	size_t* bufferPos = static_cast<size_t*>(lua_newuserdata(L, sizeof(size_t)));
	size_t* bufferEnd = static_cast<size_t*>(lua_newuserdata(L, sizeof(size_t)));

	(void) buffer;
	*bufferPos = 0;
	*bufferEnd = 0;

	lua_pushcclosure(L, LuaIOFileLineIterator, 4); // Create closure with 4 upvalues
	return 1;
}

int LuaIOFileEmptyLineIterator(lua_State* L)
{
	return 0;
}

int LuaIOFileEmptyLines(lua_State* L)
{
	lua_pushcclosure(L, LuaIOFileEmptyLineIterator, 0);
	return 1;
}

/* maximum length of a numeral */
#define MAXRN		200

/* auxiliary structure used by 'read_number' */
typedef struct
{
	fwRefContainer<vfs::Stream> f; /* file being read */
	int c; /* current character (look ahead) */
	int n; /* number of elements in buffer 'buff' */
	char buff[MAXRN + 1]; /* +1 for ending '\0' */
} RN;


/*
** Add current char to buffer (if not out of space) and read next one
*/
int LuaIONextChar(RN* rn)
{
	if (rn->n >= MAXRN)
	{
		/* buffer overflow? */
		rn->buff[0] = '\0'; /* invalidate result */
		return 0; /* fail */
	}

	rn->buff[rn->n++] = rn->c; /* save current char */
	rn->c = LuaIOGetC(rn->f); /* read next one */
	return 1;
}


/*
** Accept current char if it is in 'set' (of size 1 or 2)
*/
int LuaIOTestIfOneOfTwo(RN* rn, const char* set)
{
	if (rn->c == set[0] || (rn->c == set[1] && rn->c != '\0'))
	{
		return LuaIONextChar(rn);
	}

	return 0;
}


/*
** Read a sequence of (hex)digits
*/
int LuaIOReadDigits(RN* rn, const int hex)
{
	int count = 0;
	while ((hex ? std::isxdigit(rn->c) : std::isdigit(rn->c)) && LuaIONextChar(rn))
	{
		count++;
	}

	return count;
}


/*
** Read a number: first reads a valid prefix of a numeral into a buffer.
** Then it calls 'lua_stringtonumber' to check whether the format is
** correct and to convert it to a Lua number
*/
int LuaIOReadNumber(lua_State* L, const fwRefContainer<vfs::Stream>& f)
{
	RN rn;
	int count = 0;
	int hex = 0;
	char decp[2];
	rn.f = f;
	rn.n = 0;
	decp[0] = lua_getlocaledecpoint(); /* get decimal point from locale */
	decp[1] = '\0';
	l_lockfile(rn.f);
	do
	{
		rn.c = LuaIOGetC(rn.f);
	}
	while (std::isspace(rn.c)); /* skip spaces */

	LuaIOTestIfOneOfTwo(&rn, "-+"); /* optional signal */
	if (LuaIOTestIfOneOfTwo(&rn, "0"))
	{
		if (LuaIOTestIfOneOfTwo(&rn, "xX")) hex = 1; /* numeral is hexadecimal */
		else count = 1; /* count initial '0' as a valid digit */
	}

	count += LuaIOReadDigits(&rn, hex); /* integral part */
	if (LuaIOTestIfOneOfTwo(&rn, decp))
	{
		/* decimal point? */
		count += LuaIOReadDigits(&rn, hex); /* fractional part */
	}

	if (count > 0 && LuaIOTestIfOneOfTwo(&rn, (hex ? "pP" : "eE")))
	{
		/* exponent mark? */
		LuaIOTestIfOneOfTwo(&rn, "-+"); /* exponent signal */
		LuaIOReadDigits(&rn, 0); /* exponent digits */
	}

	LuaIOUnGetC(rn.c, rn.f); /* unread look-ahead char */
	l_unlockfile(rn.f);
	rn.buff[rn.n] = '\0'; /* finish string */
	if (lua_stringtonumber(L, rn.buff))
	{
		/* is this a valid number? */
		return 1; /* ok */
	}

	/* invalid format */
	lua_pushnil(L); /* "result" to be removed */
	return 0; /* read fails */
}

int LuaIOTestEOF(const fwRefContainer<vfs::Stream>& f)
{
	size_t currOffset = f->Seek(0, SEEK_CUR);
	size_t max = f->Seek(0, SEEK_END);
	f->Seek(static_cast<intptr_t>(currOffset), SEEK_SET);

	if (currOffset == max)
	{
		return false;
	}

	return true;
}

int LuaIOReadLine(lua_State* L, const fwRefContainer<vfs::Stream>& f, int chop)
{
	luaL_Buffer b;
	int c = '\0';
	luaL_buffinit(L, &b);
	while (c != EOF && c != '\n')
	{
		/* repeat until end of line */
		char* buff = luaL_prepbuffer(&b); /* pre-allocate buffer */
		int i = 0;
		l_lockfile(f); /* no memory errors can happen inside the lock */
		while (i < LUAL_BUFFERSIZE && (c = LuaIOGetC(f)) != EOF && c != '\n')
		{
			buff[i++] = static_cast<char>(c);
		}

		l_unlockfile(f);
		luaL_addsize(&b, i);
	}

	if (!chop && c == '\n')
	{
		/* want a newline and have one? */
		luaL_addchar(&b, c); /* add ending newline to result */
	}

	luaL_pushresult(&b); /* close buffer */
	/* return ok if read something (either a newline or something else) */
	return (c == '\n' || lua_rawlen(L, -1) > 0);
}

void LuaIOReadAll(lua_State* L, const fwRefContainer<vfs::Stream>& f)
{
	size_t nr;
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	do
	{
		/* read file in chunks of LUAL_BUFFERSIZE bytes */
		char* p = luaL_prepbuffsize(&b, LUAL_BUFFERSIZE);
		nr = f->Read(p, sizeof(char) * LUAL_BUFFERSIZE);
		luaL_addsize(&b, nr);
	}
	while (nr == LUAL_BUFFERSIZE);

	luaL_pushresult(&b); /* close buffer */
}

int LuaIOReadChars(lua_State* L, const fwRefContainer<vfs::Stream>& f, size_t n)
{
	size_t nr; /* number of chars actually read */
	char* p;
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	p = luaL_prepbuffsize(&b, n); /* prepare buffer to read whole block */
	nr = f->Read(p, sizeof(char) * n); /* try to read 'n' chars */
	luaL_addsize(&b, nr);
	luaL_pushresult(&b); /* close buffer */
	return nr > 0; /* true iff read something */
}

int LuaIORead(lua_State* L, const fwRefContainer<vfs::Stream>& f, int first)
{
	int nargs = lua_gettop(L) - 1;
	int success;
	int n;

	f->Seek(0, SEEK_CUR);
	if (nargs == 0)
	{
		/* no arguments? */
		success = LuaIOReadLine(L, f, 1);
		n = first + 1; /* to return 1 result */
	}
	else
	{
		/* ensure stack space for all results and for auxlib's buffer */
		luaL_checkstack(L, nargs + LUA_MINSTACK, "too many arguments");
		success = 1;
		for (n = first; nargs-- && success; n++)
		{
			if (lua_type(L, n) == LUA_TNUMBER)
			{
				size_t l = static_cast<size_t>(luaL_checkinteger(L, n));
				success = (l == 0) ? LuaIOTestEOF(f) : LuaIOReadChars(L, f, l);
			}
			else
			{
				const char* p = luaL_checkstring(L, n);
				if (*p == '*')
				{
					p++; /* skip optional '*' (for compatibility) */
				}
				switch (*p)
				{
				case 'n': /* number */
					success = LuaIOReadNumber(L, f);
					break;
				case 'l': /* line */
					success = LuaIOReadLine(L, f, 1);
					break;
				case 'L': /* line with end-of-line */
					success = LuaIOReadLine(L, f, 0);
					break;
				case 'a': /* file */
					LuaIOReadAll(L, f); /* read entire file */
					success = 1; /* always success */
					break;
				default:
					return luaL_argerror(L, n, "invalid format");
				}
			}
		}
	}

	if (!success)
	{
		lua_pop(L, 1); /* remove last result */
		lua_pushnil(L); /* push nil instead */
	}

	return n - first;
}

int LuaIOFileRead(lua_State* L)
{
	fwRefContainer<vfs::Stream> f = LuaIOToFile(L);
	return LuaIORead(L, f, 2);
}

int LuaIOFileWrite(lua_State* L)
{
	const fwRefContainer<vfs::Stream> f = LuaIOToFile(L);
	// push file at the stack top (to be returned)
	lua_pushvalue(L, 1);
	int argumentOffset = 2;
	int numberOfArguments = lua_gettop(L) - argumentOffset;
	int status = 1;
	for (; numberOfArguments--; argumentOffset++)
	{
		if (lua_type(L, argumentOffset) == LUA_TNUMBER)
		{
			thread_local char buffer[64];
			int len;
			if (lua_isinteger(L, argumentOffset))
			{
				len = snprintf(buffer, sizeof(buffer), LUA_INTEGER_FMT, lua_tointeger(L, argumentOffset));
			}
			else
			{
				len = snprintf(buffer, sizeof(buffer), LUA_NUMBER_FMT, lua_tonumber(L, argumentOffset));
			}

			if (len > 0)
			{
				status = status && f->Write(buffer, len) == static_cast<size_t>(len);
			}
		}
		else
		{
			size_t stringLength;
			const char* stringBuffer = luaL_checklstring(L, argumentOffset, &stringLength);
			status = status && f->Write(stringBuffer, sizeof(char) * stringLength) == sizeof(char) * stringLength;
		}
	}

	if (status)
	{
		// file handle already on stack top
		return 1;
	}

	return luaL_fileresult(L, status, nullptr);
}

int LuaIOFileSeek(lua_State* L)
{
	static constexpr int mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
	static const char* modeNames[] = {"set", "cur", "end", nullptr};

	const fwRefContainer<vfs::Stream> f = LuaIOToFile(L);
	const int op = luaL_checkoption(L, 2, "cur", modeNames);
	const intptr_t offset = luaL_optinteger(L, 3, 0);
	const size_t result = f->Seek(offset, mode[op]);

	// check if seek returned an error
	if (result == static_cast<uint64_t>(-1))
	{
		return luaL_fileresult(L, 0, nullptr);
	}

	lua_pushinteger(L, static_cast<lua_Integer>(result));
	return 1;
}

int LuaIOFileSetVBuf(lua_State* L)
{
	static constexpr int mode[] = {_IONBF, _IOFBF, _IOLBF};
	static const char* modeNames[] = {"no", "full", "line", nullptr};

	const fwRefContainer<vfs::Stream> f = LuaIOToFile(L);
	const int op = luaL_checkoption(L, 2, nullptr, modeNames);
	const lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
	// set buffer mode is not emulated, because we do not need to use the system buffer anymore
	//const int res = setvbuf(f, nullptr, mode[op], static_cast<size_t>(sz));
	return luaL_fileresult(L, true, nullptr);
}

int LuaIOFileFlush(lua_State* L)
{
	const fwRefContainer<vfs::Stream> f = LuaIOToFile(L);
	return luaL_fileresult(L, f->Flush() == true, nullptr);
}

int LuaIOEmptyFlush(lua_State* L)
{
	return luaL_fileresult(L, false, nullptr);
}

int LuaIOWrite(lua_State* L)
{
	return fx::Lua_Print(L);
}

/*
** functions for 'io' library
*/
const luaL_Reg iolib[] = {
	{"close", LuaIOClose},
	{"flush", LuaIOEmptyFlush},
	//{"input", io_input},
	{"lines", LuaIOFileEmptyLines},
	{"open", LuaIOOpen},
	//{"output", io_output},
	{"popen", LuaIOPOpen},
	//{"read", io_read},
	{"tmpfile", LuaIOTempFile},
	{"readdir", LuaIOReadDir},
	{"type", io_type},
	{"write", LuaIOWrite},
	{nullptr, nullptr}
};

/*
** methods for file handles
*/
const luaL_Reg flib[] = {
	{"close", LuaIOClose},
	{"flush", LuaIOFileFlush},
	{"lines", LuaIOFileLines},
	{"read", LuaIOFileRead},
	{"seek", LuaIOFileSeek},
	{"setvbuf", LuaIOFileSetVBuf},
	{"write", LuaIOFileWrite},
	{"__gc", f_gc},
	{"__tostring", f_tostring},
	{nullptr, nullptr}
};

/*
** methods for directory
*/
const luaL_Reg directoryLib[] = {
	{"close", LuaIODirectoryClose},
	{"lines", LuaIODirectoryLines},
	//{"read", LuaIOFileRead},
	//{"seek", LuaIOFileSeek},
	{"__gc", LuaIODirectoryGC},
	{"__tostring", LuaIODirectoryToString},
	{nullptr, nullptr}
};

void createFileMeta(lua_State* L)
{
	luaL_newmetatable(L, LUA_FILEHANDLE); /* create metatable for file handles */
	lua_pushvalue(L, -1); /* push metatable */
	lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */
	luaL_setfuncs(L, flib, 0); /* add file methods to new metatable */
	lua_pop(L, 1); /* pop new metatable */
}

void createDirectoryMeta(lua_State* L)
{
	luaL_newmetatable(L, LUA_FX_DIRECTORY_HANDLE); /* create metatable for file handles */
	lua_pushvalue(L, -1); /* push metatable */
	lua_setfield(L, -2, "__index"); /* metatable.__index = metatable */
	luaL_setfuncs(L, directoryLib, 0); /* add file methods to new metatable */
	lua_pop(L, 1); /* pop new metatable */
}

void createStdEmptyFile(lua_State* L, const char* k,
                   const char* fname)
{
	LStream* p = LuaNewPreFile(L);
	p->closef = nullptr;
	p->f = nullptr;
	if (k != nullptr)
	{
		lua_pushvalue(L, -1);
		lua_setfield(L, LUA_REGISTRYINDEX, k); /* add file to registry */
	}

	lua_setfield(L, -2, fname); /* add file to module */
}
}

namespace fx
{
int lua_fx_openio(lua_State* L)
{
	luaL_newlib(L, iolib);
	createFileMeta(L);
	createDirectoryMeta(L);
	// emulate stdin, stdout and stderr
	createStdEmptyFile(L, "_IO_input", "stdin");
	createStdEmptyFile(L, "_IO_output", "stdout");
	createStdEmptyFile(L, nullptr, "stderr");
	return 1;
}
}
