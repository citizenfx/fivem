/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include "InstallerExtraction.h"

#define restrict
#define LZMA_API_STATIC
#include <lzma.h>
#undef restrict

#include <sstream>

class LzmaStreamWrapper
{
private:
	FILE* m_file;

	lzma_stream m_stream;

	std::vector<uint8_t> m_buffer;
	std::vector<uint8_t> m_inBuffer;

	size_t m_cursor;

public:
	LzmaStreamWrapper(FILE* baseFile);

	~LzmaStreamWrapper();

	size_t Tell();

	void SeekAhead(size_t bytes);

	size_t Read(void* buffer, size_t bytes);
};

LzmaStreamWrapper::LzmaStreamWrapper(FILE* baseFile)
	: m_file(baseFile), m_buffer(32768), m_inBuffer(32768), m_cursor(0)
{
	memset(&m_stream, 0, sizeof(m_stream));
	lzma_alone_decoder(&m_stream, UINT32_MAX);

	m_stream.next_out = &m_buffer[0];
	m_stream.avail_out = m_buffer.size();

	m_stream.next_in = &m_inBuffer[0];

	// decode the base file header + a fake uncompressed size (as NSIS doesn't have one)
	uint8_t lzmaHeader[5 + 8];
	fread(&lzmaHeader[0], 1, 5, baseFile);
	
	*(uint64_t*)&lzmaHeader[5] = UINT64_MAX;

	// 'decode' it
	m_stream.avail_in = sizeof(lzmaHeader);
	m_stream.next_in = &lzmaHeader[0];

	assert(lzma_code(&m_stream, LZMA_RUN) <= 1);
}

LzmaStreamWrapper::~LzmaStreamWrapper()
{
	lzma_end(&m_stream);
}

size_t LzmaStreamWrapper::Tell()
{
	return m_cursor;
}

size_t LzmaStreamWrapper::Read(void* buffer, size_t bytes)
{
	if (bytes == 0)
	{
		return 0;
	}

	m_stream.next_out = static_cast<uint8_t*>(buffer);
	m_stream.avail_out = bytes;

	while (m_stream.avail_out > 0)
	{
		if (m_stream.avail_in == 0)
		{
			m_stream.next_in = &m_inBuffer[0];
			m_stream.avail_in = fread(&m_inBuffer[0], 1, m_inBuffer.size(), m_file);

			if (m_stream.avail_in == 0)
			{
				break;
			}
		}

		int res = lzma_code(&m_stream, LZMA_RUN);

		assert(res <= 1);
	}

	m_cursor += (bytes - m_stream.avail_out);

	return (bytes - m_stream.avail_out);
}

void LzmaStreamWrapper::SeekAhead(size_t bytes)
{
	// first, drain the buffer's outness
	size_t toDrain = min(m_stream.avail_out, bytes);

	m_cursor += toDrain;
	bytes -= toDrain;
	m_stream.avail_out -= toDrain;
	m_stream.next_out += toDrain;

	// if there's still bytes to drain, continue decompressing
	while (bytes > 0)
	{
		size_t initialOut = min(m_buffer.size(), bytes);

		m_stream.next_out = &m_buffer[0];
		m_stream.avail_out = initialOut;

		if (m_stream.avail_in == 0)
		{
			m_stream.next_in = &m_inBuffer[0];
			m_stream.avail_in = fread(&m_inBuffer[0], 1, m_inBuffer.size(), m_file);
		}

		int res = lzma_code(&m_stream, LZMA_RUN);

		assert(res <= 1);

		bytes -= (initialOut - m_stream.avail_out);
		m_cursor += (initialOut - m_stream.avail_out);

		// poll the Windows message loop if needed
		MSG msg;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

bool ExtractInstallerFile(const std::wstring& installerFile, const std::function<void(const InstallerInterface&)>& fileFindCallback)
{
	// open the installer file
	FILE* f = _wfopen(installerFile.c_str(), L"rb");

	if (!f)
	{
		return false;
	}

	// update UI
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	std::wstring entryPathWide = installerFile;

	UI_UpdateText(1, va(L"Extracting %s (scanning)", entryPathWide.c_str()));
	UI_UpdateProgress(0.0);

	// try finding the position of the firstheader structure
	char byteBuffer[32768];
	firstheader fh;

	bool found = false;
	
	while (!found)
	{
		// if the file ended, return false
		if (feof(f))
		{
			fclose(f);
			return false;
		}

		// read from the file
		int firstPos = ftell(f);
		int left = fread(byteBuffer, 1, sizeof(byteBuffer), f);

		// compare through the buffer to see if we found firstheader
		for (int i = 0; i < sizeof(byteBuffer) - sizeof(firstheader); i++)
		{
			memcpy(&fh, &byteBuffer[i], sizeof(firstheader));

			// validation from NSIS code
			if ((fh.flags & (~FH_FLAGS_MASK)) == 0 &&
				fh.siginfo == FH_SIG &&
				fh.nsinst[2] == FH_INT3 &&
				fh.nsinst[1] == FH_INT2 &&
				fh.nsinst[0] == FH_INT1)
			{
				// seek back to the end of firstheader in the file
				fseek(f, firstPos + i + sizeof(firstheader), SEEK_SET);

				// we found it.
				found = true;
				break;
			}
		}
	}

	// pull message loop
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// V's NSIS installers are NSIS_COMPRESS_WHOLE, so we'll need to pack our bags into a meta-LZMA stream...
	LzmaStreamWrapper stream(f);

	uint32_t headerLength;
	stream.Read(&headerLength, sizeof(headerLength));

	header h;
	stream.Read(&h, sizeof(h));

	// read the header blocks sequentially
	std::vector<page> pages(h.blocks[NB_PAGES].num);
	std::vector<section> sections(h.blocks[NB_SECTIONS].num);;
	std::vector<entry> entries(h.blocks[NB_ENTRIES].num);;
	std::vector<wchar_t> strings;

	// string getting function
	auto getString = [&] (int index) -> std::wstring
	{
		const wchar_t* str = &strings[index];
		std::wstringstream ss;
		
		while (*str)
		{
			if (*str == 3)
			{
				ss << '$';

				str += 2;
			}
			else
			{
				ss << *str;
				str++;
			}
		}

		return ss.str();
	};

	// seek to the pages block and read it
	stream.SeekAhead(h.blocks[NB_PAGES].offset + 4 - stream.Tell());
	stream.Read(&pages[0], pages.size() * sizeof(page));

	// same for sections
	stream.SeekAhead(h.blocks[NB_SECTIONS].offset + 4 - stream.Tell());
	stream.Read(&sections[0], sections.size() * sizeof(section));

	// and entries
	stream.SeekAhead(h.blocks[NB_ENTRIES].offset + 4 - stream.Tell());
	stream.Read(&entries[0], entries.size() * sizeof(entry));

	// read strings 'manually'
	size_t stringsLength = h.blocks[NB_STRINGS + 1].offset - h.blocks[NB_STRINGS].offset;
	strings.resize(stringsLength);

	stream.SeekAhead(h.blocks[NB_STRINGS].offset + 4 - stream.Tell());
	stream.Read(&strings[0], stringsLength);

	// find a list of files to 'extract'
	std::vector<std::pair<entry, std::wstring>> filesToExtract;

	// set up the installer interface
	InstallerInterface interface;
	interface.addFile = [&] (const entry& entry, const std::wstring& outPath)
	{
		filesToExtract.push_back({ entry, outPath });
	};

	interface.getSections = [&] ()
	{
		return sections;
	};

	interface.processSection = [&] (section section, std::function<void(const entry&)> processEntry)
	{
		int entryIdx = section.code;
		int entrySize = section.code_size;

		for (int i = entryIdx; i < (entryIdx + entrySize); i++)
		{
			if (entries[i].which == EW_RET)
			{
				break;
			}

			processEntry(entries[i]);
		}
	};

	interface.getString = getString;

	// get code for the section we'll scan for
#if 0
	int entryIdx = 0;
	int entrySize = 0;

	for (const auto& section : sections)
	{
		entryIdx = section.code;
		entrySize = section.code_size;

		if (entrySize != 0)
		{
			break;
		}
	}

	// look for any 'extractfile' entries with the path we use (checking for any 'createdir's with flag [1] set to specify the directory name)
	std::wstring currentDirectory;

	int dataIdx = -1;

	for (int i = entryIdx; i < (entryIdx + entrySize); i++)
	{
		auto& entry = entries[i];

		if (entry.which == EW_CREATEDIR)
		{
			if (entry.offsets[1] != 0)
			{
				// update instdir
				currentDirectory = getString(entry.offsets[0]);

				std::replace(currentDirectory.begin(), currentDirectory.end(), L'\\', L'/');
			}
		}
		// extract file
		else if (entry.which == EW_EXTRACTFILE)
		{
			// get the base filename
			std::wstring fileName = currentDirectory + L"/" + getString(entry.offsets[1]);

			// append a new filename without double slashes
			{
				std::wstringstream newFileName;
				bool wasSlash = false;

				for (int j = 0; j < fileName.length(); j++)
				{
					wchar_t c = fileName[j];

					if (c == L'/')
					{
						if (!wasSlash)
						{
							newFileName << c;

							wasSlash = true;
						}
					}
					else
					{
						newFileName << c;

						wasSlash = false;
					}
				}

				fileName = newFileName.str();
			}

			// check for equality, and if so, specify extraction data
			if (_wcsicmp(entryPathWide.c_str(), fileName.c_str()) == 0)
			{
				dataIdx = entry.offsets[2];
				break;
			}
		}
	}

	if (dataIdx == -1)
	{
		fclose(f);
		return false;
	}
#endif

	// run the callback
	fileFindCallback(interface);

	if (filesToExtract.empty())
	{
		fclose(f);
		return false;
	}

	// sort by compressed offset
	std::sort(filesToExtract.begin(), filesToExtract.end(), [] (const auto& left, const auto& right)
	{
		return (left.first.offsets[2] < right.first.offsets[2]);
	});

	// extract files - seek to the offset first and get length of the file
	uint32_t fileLength;

	// for all files...
	for (auto& data : filesToExtract)
	{
		stream.SeekAhead(headerLength + data.first.offsets[2] + 4 - stream.Tell());
		stream.Read(&fileLength, sizeof(fileLength));

		// now, extract the file to disk. we'll probably want to update UI for this, as well...
		auto nameStr = getString(data.first.offsets[1]);
		int lastSlash = nameStr.find_last_of(L'/');

		UI_UpdateText(1, va(L"Extracting %s", nameStr.substr(lastSlash + 1).c_str()));
		UI_UpdateProgress(0.0);

		// open the output file
		std::wstring tmpFile = data.second + L".tmp";

		FILE* of = _wfopen(tmpFile.c_str(), L"wb");

		if (!of)
		{
			fclose(f);
			return false;
		}

		char buffer[8192];
		uint32_t bytesWritten = 0;

		while (bytesWritten < fileLength)
		{
			// write to the output file
			size_t toRead = fileLength - bytesWritten;
			toRead = min(sizeof(buffer), toRead);

			toRead = stream.Read(buffer, toRead);
			bytesWritten += fwrite(buffer, 1, toRead, of);

			UI_UpdateProgress((bytesWritten / (double)fileLength) * 100.0);

			// poll message loop
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (UI_IsCanceled())
			{
				fclose(of);
				fclose(f);

				return false;
			}
		}

		// we're done? wow!
		fclose(of);

		// rename the file tooooo
		_wunlink(data.second.c_str());
		_wrename(tmpFile.c_str(), data.second.c_str());
	}

	fclose(f);

	return true;
}