/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "DllGameComponent.h"

#include <dlfcn.h>

Component* DllGameComponent::CreateComponent()
{
	void* hModule = dlopen(m_path.c_str(), RTLD_NOW);

	if (!hModule)
	{
		return nullptr;
	}

	auto createComponent = (Component*(__cdecl*)())dlsym(hModule, "CreateComponent");

	return createComponent ? createComponent() : nullptr;
}

void DllGameComponent::ReadManifest()
{
	FILE* file = _pfopen(va(_P("%s.json"), m_path.c_str()), _P("r"));
	
	if (!file)
	{
		return;
	}

	// seek to the file end
	fseek(file, 0, SEEK_END);
	long length = ftell(file);

	// allocate a buffer and seek to the start
	std::vector<char> data(length);
	fseek(file, 0, SEEK_SET);

	// read into the buffer
	fread(&data[0], 1, length, file);

	// close the file
	fclose(file);

	// copy into a zero-terminated std::string and parse
	std::string resourceString(&data[0], data.size());

	m_document.Parse(resourceString.c_str());
}