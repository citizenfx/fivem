/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ProfileManagerImpl.h"
#include "SteamComponentAPI.h"

#include <base64.h>
#include <GfxUtil.h>

#include <shlwapi.h>
#include <wincodec.h>
#include <wrl.h>

using namespace Microsoft::WRL;

struct AvatarImageLoaded_t
{
	enum { k_iCallback = 300 + 34 };
	uint64_t m_steamID; // steamid the avatar has been loaded for
	int m_iImage; // the image index of the now loaded image
	int m_iWide; // width of the loaded image
	int m_iTall; // height of the loaded image
};

class SteamSuggestionProvider : public ProfileSuggestionProvider
{
private:
	char* EncodeImageAsPNG(const char* inData, int width, int height, size_t* outLength);

public:
	virtual void GetProfiles(std::function<void(fwRefContainer<Profile>)> receiver) override;
};

void SteamSuggestionProvider::GetProfiles(std::function<void(fwRefContainer<Profile>)> receiver)
{
	auto steamComponent = Instance<ISteamComponent>::Get();

	if (steamComponent->IsSteamRunning())
	{
		IClientEngine* steamClient = steamComponent->GetPrivateClient();

		// check if the private client class exists; as it might be the case that a 'cracked' steamclient.dll is used which only implements ISteam*
		if (steamClient)
		{
			InterfaceMapper steamUser(steamClient->GetIClientUser(steamComponent->GetHSteamUser(), steamComponent->GetHSteamPipe(), "CLIENTUSER_INTERFACE_VERSION001"));
			InterfaceMapper steamFriends(steamClient->GetIClientFriends(steamComponent->GetHSteamUser(), steamComponent->GetHSteamPipe(), "CLIENTFRIENDS_INTERFACE_VERSION001"));

			if (steamUser.IsValid())
			{
				// NOTE: this may be different on AMD64
				uint64_t steamID;
				steamUser.Invoke<void>("GetSteamID", &steamID);

				int largeAvatar = steamFriends.Invoke<int>("GetLargeFriendAvatar", steamID);
				std::string personaName = steamFriends.Invoke<const char*>("GetPersonaName");

				auto continueCode = [=] (int avatarId)
				{
					InterfaceMapper steamUtils(steamClient->GetIClientUtils(steamComponent->GetHSteamPipe(), "CLIENTUTILS_INTERFACE_VERSION001"));
					std::string tileURI;

					uint32_t width;
					uint32_t height;
					if (steamUtils.Invoke<bool>("GetImageSize", avatarId, &width, &height))
					{
						std::vector<char> imageData;
						imageData.resize(width * height * 4);

						if (steamUtils.Invoke<bool>("GetImageRGBA", avatarId, &imageData[0], imageData.size()))
						{
							// convert to PNG
							size_t pngLength = 0;
							char* pngData = EncodeImageAsPNG(&imageData[0], width, height, &pngLength);

							// encode as base64
							size_t base64Length = 0;

							char* base64 = base64_encode(reinterpret_cast<uint8_t*>(pngData), pngLength, &base64Length);

							// free the input buffer
							delete[] pngData;

							// make a funny string
							tileURI = "data:image/png;base64," + std::string(base64, base64Length);

							// and free the base64
							free(base64);
						}
					}

					fwRefContainer<ProfileImpl> profile = new ProfileImpl();
					std::vector<ProfileIdentifier> identifiers;
					identifiers.push_back(std::make_pair("steam", va("%llx", steamID)));

					profile->SetDisplayName(personaName);
					profile->SetIdentifiers(identifiers);
					profile->SetTileURI(tileURI);

					receiver(profile);
				};

				if (largeAvatar == -1)
				{
					int steamCallbackRef = steamComponent->RegisterSteamCallback<AvatarImageLoaded_t>([=] (AvatarImageLoaded_t* callbackData)
					{
						continueCode(callbackData->m_iImage);

						steamComponent->RemoveSteamCallback(steamCallbackRef);
					});
				}
				else
				{
					continueCode(largeAvatar);
				}
			}
		}
	}
}

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "windowscodecs.lib")

char* SteamSuggestionProvider::EncodeImageAsPNG(const char* inData, int width, int height, size_t* outLength)
{
	ComPtr<IWICImagingFactory> imagingFactory;

	// initialize COM
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	// create a WIC context
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)imagingFactory.ReleaseAndGetAddressOf());

	if (FAILED(hr))
	{
		return nullptr;
	}

	// create an output stream
	ComPtr<IStream> comStream;

	hr = CreateStreamOnHGlobal(nullptr, TRUE, comStream.ReleaseAndGetAddressOf());

	// create an encoder for PNG
	ComPtr<IWICBitmapEncoder> encoder;

	hr = imagingFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, encoder.ReleaseAndGetAddressOf());

	// initialize the encoder
	hr = encoder->Initialize(comStream.Get(), WICBitmapEncoderNoCache);

	// create an encoder frame
	ComPtr<IWICBitmapFrameEncode> frame;
	
	hr = encoder->CreateNewFrame(frame.GetAddressOf(), nullptr);

	// initialize the frame
	hr = frame->Initialize(nullptr);

	// set the frame size
	hr = frame->SetSize(width, height);

	// set the pixel format
	WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppBGRA;
	hr = frame->SetPixelFormat(&pixelFormat);

	{
		// swap the pixels before writing (as they're RGBA and the PNG encoder only wants BGRA)
		std::vector<uint8_t> inDataConverted;
		inDataConverted.resize(width * height * 4);

		ConvertImageDataRGBA_BGRA(0, 0, width, height, width * 4, inData, width * 4, &inDataConverted[0]);

		// write the pixel data
		hr = frame->WritePixels(height, width * 4, width * height * 4, &inDataConverted[0]);
	}

	// commit data to the stream
	hr = frame->Commit();
	hr = encoder->Commit();

	// get the HGLOBAL from the COM stream
	HGLOBAL hGlobal;
	hr = GetHGlobalFromStream(comStream.Get(), &hGlobal);

	// lock the HGLOBAL (for good measure - I doubt this is even needed in Win32)
	LPVOID dataPtr = GlobalLock(hGlobal);

	// get the stream size
	ULARGE_INTEGER streamSize;
	hr = IStream_Size(comStream.Get(), &streamSize);

	// copy it to a new buffer
	char* retval = new char[streamSize.QuadPart];

	memcpy(retval, dataPtr, streamSize.QuadPart);

	// store the size
	*outLength = streamSize.QuadPart;

	// unlock the HGLOBAL (it'll get released later on)
	GlobalUnlock(hGlobal);

	// and return
	return retval;
}

static InitFunction initFunction([] ()
{
	ProfileManagerImpl* ourProfileManager = static_cast<ProfileManagerImpl*>(Instance<ProfileManager>::Get());

	ourProfileManager->AddSuggestionProvider(new SteamSuggestionProvider());
});