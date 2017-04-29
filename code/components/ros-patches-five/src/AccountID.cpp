/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#pragma comment(lib, "crypt32.lib")

#include <wincrypt.h>
#include <shlobj.h>

#include <mutex>

#include <Error.h>

uint64_t ROSGetDummyAccountID()
{
	static std::once_flag gotAccountId;
	static uint32_t accountId;

	std::call_once(gotAccountId, [] ()
	{
		PWSTR appdataPath = nullptr;
		SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdataPath);

		CreateDirectory(va(L"%s\\CitizenFX", appdataPath), nullptr);

		FILE* f = _wfopen(va(L"%s\\CitizenFX\\ros_id.dat", appdataPath), L"rb");

		auto generateNewId = [&] ()
		{
			// generate a random id
			HCRYPTPROV provider;
			if (!CryptAcquireContext(&provider, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
			{
				FatalError("CryptAcquireContext failed (ros:five ID generation)");
			}

			// do call
			if (!CryptGenRandom(provider, sizeof(accountId), reinterpret_cast<BYTE*>(&accountId)))
			{
				FatalError("CryptGenRandom failed (ros:five ID generation)");
			}

			// release
			CryptReleaseContext(provider, 0);

			// remove top bit
			accountId &= 0x7FFFFFFF;

			// verify if ID isn't null
			if (accountId == 0)
			{
				FatalError("ros:five ID generation generated a null ID!");
			}

			// write id
			f = _wfopen(va(L"%s\\CitizenFX\\ros_id.dat", appdataPath), L"wb");

			if (!f)
			{
				FatalError("Could not open AppData\\CitizenFX\\ros_id.dat for writing!");
			}

			fwrite(&accountId, 1, sizeof(accountId), f);
		};

		if (!f)
		{
			generateNewId();
		}
		else
		{
			fread(&accountId, 1, sizeof(accountId), f);

			if (accountId == 0)
			{
				fclose(f);
				f = nullptr;

				_wunlink(va(L"%s\\CitizenFX\\ros_id.dat", appdataPath));

				generateNewId();
			}
		}

		if (f)
		{
			fclose(f);
		}

		CoTaskMemFree(appdataPath);
	});

	return accountId;
}

#include <shlobj.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <sstream>

bool LoadAccountData(std::string& str)
{
    // make path
    wchar_t* appdataPath;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdataPath);

    CreateDirectory(va(L"%s\\CitizenFX", appdataPath), nullptr);

    // open?
    FILE* f = _wfopen(va(L"%s\\CitizenFX\\ros_auth.dat", appdataPath), L"rb");

    if (!f)
    {
        CoTaskMemFree(appdataPath);
        return false;
    }

    // seek
    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    fseek(f, 0, SEEK_SET);

    // read
    std::vector<char> data(length + 1);
    fread(&data[0], 1, length, f);

    fclose(f);

    // hm
    str = &data[0];

    CoTaskMemFree(appdataPath);

    return true;
}

bool LoadAccountData(boost::property_tree::ptree& tree)
{
    std::string str;

    if (!LoadAccountData(str))
    {
        return false;
    }

    std::stringstream stream(str);

    boost::property_tree::read_xml(stream, tree);

    if (tree.get("Response.Status", 0) == 0)
    {
        return false;
    }
    else
    {
        std::string ticket = tree.get<std::string>("Response.Ticket");
        int posixTime = tree.get<int>("Response.PosixTime");
        int secsUntilExpiration = tree.get<int>("Response.SecsUntilExpiration");

        if (time(nullptr) < (posixTime + secsUntilExpiration))
        {
            return true;
        }
    }

    return false;
}

void SaveAccountData(const std::string& data)
{
    // make path
    wchar_t* appdataPath;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdataPath);

    CreateDirectory(va(L"%s\\CitizenFX", appdataPath), nullptr);

    // open?
    FILE* f = _wfopen(va(L"%s\\CitizenFX\\ros_auth.dat", appdataPath), L"wb");

    CoTaskMemFree(appdataPath);

    if (!f)
    {
        return;
    }

    fwrite(data.c_str(), 1, data.size(), f);
    fclose(f);
}