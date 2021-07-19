#include <StdInc.h>
#include <SIEnput.h>

#include <hidclass.h>
#include <hidsdi.h>
#include <setupapi.h>

#include <botan/crc32.h>
#include <wil/resource.h>

#include <concurrent_queue.h>

#include <array>
#include <variant>

#include <CoreConsole.h>
#include <IteratorView.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

constexpr const uint16_t kVendorIDSony = 0x054C;
constexpr const uint16_t kProductIDJedi = 0x05C4;
constexpr const uint16_t kProductIDJediPlus = 0x09CC;
constexpr const uint16_t kProductIDBond = 0x0CE6;

#ifdef _DEBUG
#define SIE_TRACE(...) \
	trace(__VA_ARGS__)
#else
#define SIE_TRACE(...) \
	console::DPrintf("SIEnput", __VA_ARGS__)
#endif

class SIEPadInternal;

using SIEPadInternalHolder = std::variant<std::shared_ptr<SIEPadInternal>, std::nullptr_t>;

class SIEPadManagerInternal : public io::SIEPadManager
{
public:
	SIEPadManagerInternal()
	{
		m_outputQueueTrip.create();

		m_deviceDetectionThread = std::thread([this]()
		{
			DeviceDetectionMain();
		});

		m_outputQueueThread = std::thread([this]()
		{
			OutputQueueMain();
		});
	}

	void QueueOutputEvent(std::function<void()>&& fn, bool critical = false)
	{
		if (m_outputQueue.unsafe_size() < 16 || critical)
		{
			m_outputQueue.push(std::move(fn));
			m_outputQueueTrip.SetEvent();
		}
	}

	void DestroyDevice(SIEPadInternal* self);

private:
	void DeviceDetectionMain();

	void OutputQueueMain();

	bool IsSIEPad(const HIDD_ATTRIBUTES& attributes);

	std::shared_ptr<SIEPadInternalHolder> FindDevice(std::wstring_view devicePath);

	void AddFailedDevice(std::wstring_view devicePath);

	void StartDevice(std::wstring_view devicePath, std::array<uint8_t, 6> btAddr, wil::unique_hfile&& deviceHandle);

private:
	std::map<std::wstring, std::shared_ptr<SIEPadInternalHolder>, std::less<>> m_devices;
	std::multimap<std::array<uint8_t, 6>, std::weak_ptr<SIEPadInternal>> m_devicesByBtAddr;
	std::vector<std::array<uint8_t, 6>> m_devicesByIndex;

	std::thread m_deviceDetectionThread;
	std::thread m_outputQueueThread;

	wil::unique_event m_outputQueueTrip;
	concurrency::concurrent_queue<std::function<void()>> m_outputQueue;

	std::shared_mutex m_managerLock;

	volatile bool m_run = true;

public:
	virtual int GetNumPads() override
	{
		std::shared_lock _(m_managerLock);

		return m_devicesByIndex.size();
	}

	virtual std::shared_ptr<io::SIEPad> GetPad(int index) override;
};

class SIEPadInternal : public io::SIEPad
{
public:
	enum class DeviceType
	{
		Unknown,
		Jedi,
		Bond
	};

	SIEPadInternal(SIEPadManagerInternal* mgr, std::array<uint8_t, 6> btAddr, wil::unique_hfile&& deviceHandle)
		: m_mgr(mgr), m_btAddr(btAddr)
	{
		m_deviceHandle = std::move(deviceHandle);

		Initialize();

		m_inputThread = std::thread([this]()
		{
			InputMain();

			m_mgr->QueueOutputEvent([this]()
			{
				m_mgr->DestroyDevice(this);
			}, true);
		});
	}

	~SIEPadInternal()
	{
		if (m_inputThread.joinable())
		{
			m_inputThread.join();
		}
	}

	uint32_t GetButtons() override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

	virtual void SetLightBar(uint8_t r, uint8_t g, uint8_t b) override;

	virtual void SetVibration(uint8_t l, uint8_t r) override;

	void SetIndex(int playerIdx);

	DeviceType GetDeviceType()
	{
		return m_deviceType;
	}

	void SendOutputReportAsync(const void* report, uint16_t length, bool critical = false);

	auto GetBtAddr()
	{
		return m_btAddr;
	}

	auto IsBluetooth()
	{
		return m_isBluetooth;
	}

private:
	void Initialize();

	void InputMain();

	bool HasCrcErrorOnBtHid(uint8_t crcType, const void* buffer, size_t bufferLength);

private:
	SIEPadManagerInternal* m_mgr;

	std::array<uint8_t, 6> m_btAddr;

	std::thread m_inputThread;

	wil::unique_hfile m_deviceHandle;

	DeviceType m_deviceType = DeviceType::Unknown;

	bool m_isBluetooth = false;

	HIDP_CAPS m_hidCaps = { 0 };

	uint8_t m_lightbarColor[3] = { 0, 0, 0 };
	uint8_t m_userColor[3] = { 0xFF, 0x92, 0x43 };
	uint8_t m_seq = 0;

	std::map<std::string, std::string> m_firmwareInfo;
};

std::shared_ptr<io::SIEPad> SIEPadManagerInternal::GetPad(int index)
{
	std::shared_lock _(m_managerLock);

	if (index >= 0 && index < m_devicesByIndex.size())
	{
		auto btAddr = m_devicesByIndex[index];
		std::shared_ptr<io::SIEPad> btDevice;
		std::shared_ptr<io::SIEPad> usbDevice;

		for (auto& device : fx::GetIteratorView(m_devicesByBtAddr.equal_range(btAddr)))
		{
			auto d = device.second.lock();

			if (d)
			{
				if (d->IsBluetooth())
				{
					btDevice = d;
				}
				else
				{
					usbDevice = d;
				}
			}
		}

		return (usbDevice) ? usbDevice : btDevice;
	}

	return {};
}

void SIEPadManagerInternal::DeviceDetectionMain()
{
	GUID hidGuid;
	HidD_GetHidGuid(&hidGuid);

	while (m_run)
	{
		// get the device info set
		auto deviceInfoSet = SetupDiGetClassDevsW(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);
		if (deviceInfoSet)
		{
			size_t deviceIdx = 0;

			// while we've got valid devices
			SP_DEVICE_INTERFACE_DATA did = { 0 };
			did.cbSize = sizeof(did);

			while (SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &hidGuid, deviceIdx, &did))
			{
				// get the expected size
				DWORD size = 0;
				SetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &did, NULL, 0, &size, NULL);

				if (size)
				{
					std::vector<uint8_t> diddBuffer(size);
					auto didd = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)&diddBuffer[0];
					didd->cbSize = sizeof(*didd);

					// get actual detail
					if (SetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &did, didd, size, &size, NULL))
					{
						// do we already know you?
						auto existingDevice = FindDevice(didd->DevicePath);

						// 
						if (!existingDevice)
						{
							wil::unique_hfile deviceHandle{
								CreateFileW(didd->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0)
							};

							if (deviceHandle)
							{
								HIDD_ATTRIBUTES hidAttributes = { 0 };
								hidAttributes.Size = sizeof(hidAttributes);

								if (HidD_GetAttributes(deviceHandle.get(), &hidAttributes))
								{
									if (IsSIEPad(hidAttributes))
									{
										auto getFeatureReport = [&deviceHandle](uint8_t reportId, void* outBuffer, size_t length)
										{
											*(uint8_t*)outBuffer = reportId;
											auto f = HidD_GetFeature(deviceHandle.get(), outBuffer, length);
											auto e = GetLastError();

											return (f && *(uint8_t*)outBuffer == reportId);
										};

										std::array<uint8_t, 6> btAddr;
										memset(btAddr.data(), 0, btAddr.size());

										bool gotAddress = false;

										if (hidAttributes.ProductID == kProductIDBond)
										{
											char btPairingInfo[20];
											if (getFeatureReport(9, btPairingInfo, sizeof(btPairingInfo)))
											{
												memcpy(btAddr.data(), &btPairingInfo[1], 6);
												std::reverse(btAddr.begin(), btAddr.end());

												gotAddress = true;
											}
										}
										else
										{
											char btPairingInfo[16];
											if (getFeatureReport(18, btPairingInfo, sizeof(btPairingInfo)))
											{
												memcpy(btAddr.data(), &btPairingInfo[1], 6);
												std::reverse(btAddr.begin(), btAddr.end());

												gotAddress = true;
											}
										}

										if (!gotAddress)
										{
											wchar_t serialNumber[257];
											if (HidD_GetSerialNumberString(deviceHandle.get(), serialNumber, std::size(serialNumber)))
											{
												swscanf(serialNumber, L"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx", &btAddr[0], &btAddr[1], &btAddr[2], &btAddr[3], &btAddr[4], &btAddr[5]);
											}
										}

										StartDevice(didd->DevicePath, btAddr, std::move(deviceHandle));
									}
									else
									{
										AddFailedDevice(didd->DevicePath);
									}
								}
								else
								{
									AddFailedDevice(didd->DevicePath);
								}
							}
						}
					}
				}

				deviceIdx++;
			}
		}

		Sleep(500);
	}
}

void SIEPadManagerInternal::OutputQueueMain()
{
	while (m_run)
	{
		m_outputQueueTrip.wait();

		std::function<void()> fn;
		while (m_outputQueue.try_pop(fn))
		{
			fn();
		}
	}
}

void SIEPadManagerInternal::StartDevice(std::wstring_view devicePath, std::array<uint8_t, 6> btAddr, wil::unique_hfile&& deviceHandle)
{
	auto device = std::make_shared<SIEPadInternal>(this, btAddr, std::move(deviceHandle));

	std::unique_lock _(m_managerLock);

	m_devices[std::wstring{ devicePath }] = std::make_shared<SIEPadInternalHolder>(device);
	m_devicesByBtAddr.emplace(btAddr, device);
	
	if (std::find(m_devicesByIndex.begin(), m_devicesByIndex.end(), btAddr) == m_devicesByIndex.end())
	{
		m_devicesByIndex.push_back(btAddr);

		device->SetIndex(m_devicesByIndex.size() - 1);
	}
	else
	{
		device->SetIndex(std::find(m_devicesByIndex.begin(), m_devicesByIndex.end(), btAddr) - m_devicesByIndex.begin());
	}
}

void SIEPadManagerInternal::AddFailedDevice(std::wstring_view devicePath)
{
	std::unique_lock _(m_managerLock);
	m_devices[std::wstring{ devicePath }] = std::make_shared<SIEPadInternalHolder>(nullptr);
}

void SIEPadManagerInternal::DestroyDevice(SIEPadInternal* self)
{
	std::unique_lock _(m_managerLock);
	auto devAddr = self->GetBtAddr();

	auto findPair = m_devicesByBtAddr.equal_range(devAddr);

	for (auto it = findPair.first; it != findPair.second;)
	{
		if (auto ptr = it->second.lock(); ptr && ptr.get() == self)
		{
			m_devicesByBtAddr.erase(it);
			break;
		}

		++it;
	}

	for (auto it = m_devices.begin(); it != m_devices.end();)
	{
		if (it->second->index() == 0 && std::get<0>(*it->second).get() == self)
		{
			it = m_devices.erase(it);
		}
		else
		{
			++it;
		}
	}

	// last device removed?
	if (auto it = m_devicesByBtAddr.find(devAddr); it != m_devicesByBtAddr.end())
	{
		// still there! tell it about the index
		if (auto dev = it->second.lock())
		{
			dev->SetIndex(std::find(m_devicesByIndex.begin(), m_devicesByIndex.end(), devAddr) - m_devicesByIndex.begin());
		}
	}
	else
	{
		m_devicesByIndex.erase(std::remove(m_devicesByIndex.begin(), m_devicesByIndex.end(), devAddr));
	}
}

std::shared_ptr<SIEPadInternalHolder> SIEPadManagerInternal::FindDevice(std::wstring_view devicePath)
{
	std::shared_lock _(m_managerLock);

	if (auto it = m_devices.find(devicePath); it != m_devices.end())
	{
		return it->second;
	}

	return {};
}

using unique_preparsed_data = wil::unique_any<PHIDP_PREPARSED_DATA, decltype(&HidD_FreePreparsedData), &HidD_FreePreparsedData>;

void SIEPadInternal::Initialize()
{
	HIDD_ATTRIBUTES hidAttributes = { 0 };
	hidAttributes.Size = sizeof(hidAttributes);

	if (HidD_GetAttributes(m_deviceHandle.get(), &hidAttributes))
	{
		m_deviceType = (hidAttributes.ProductID == kProductIDBond) ? DeviceType::Bond : DeviceType::Jedi;
	}

	unique_preparsed_data preparsedData;
	if (HidD_GetPreparsedData(m_deviceHandle.get(), &preparsedData))
	{
		HIDP_CAPS caps = { 0 };

		if (HidP_GetCaps(preparsedData.get(), &caps) == HIDP_STATUS_SUCCESS)
		{
			m_hidCaps = caps;
		}
	}
}

static const constexpr uint8_t kReportIdInputReportBond = 1;
static const constexpr uint8_t kReportIdInputReportBtBond = 17;
static const constexpr uint8_t kReportIdGetFirmInfo = 32;
static const constexpr uint8_t kReportIdGetFirmInfoJedi = 0xA3;

#pragma pack(push, 1)
struct TouchDataBond
{
	uint8_t contact;
	uint8_t x_lo;
	uint8_t x_hi : 4, y_lo : 4;
	uint8_t y_hi;
};

struct InputReportBond
{
	uint8_t reportId;
	uint8_t analogStickLX;
	uint8_t analogStickLY;
	uint8_t analogStickRX;
	uint8_t analogStickRY;
	uint8_t analogTriggerL;
	uint8_t analogTriggerR;
	uint8_t sequenceNum;
	uint8_t digitalKeys[6];
	uint16_t randomNum;
	int16_t gyroPitch;
	int16_t gyroYaw;
	int16_t gyroRoll;
	int16_t accelX;
	int16_t accelY;
	int16_t accelZ;
	uint32_t motionTimestamp;
	uint8_t motionTemperature;
	TouchDataBond touchData[2];
	uint8_t pad;
	uint8_t waltherStatus0;
	uint8_t waltherStatus1;
	uint32_t hostTimestamp;
	uint8_t waltherStatus2;
	uint32_t deviceTimestamp;
	uint8_t status0;
	uint8_t status1;
	uint8_t status2;
	uint8_t aesCmac;
};

struct InputReportJedi
{
	uint8_t reportId;
	uint8_t analogStickLX;
	uint8_t analogStickLY;
	uint8_t analogStickRX;
	uint8_t analogStickRY;
	uint8_t digitalKeys[2];
	uint8_t digitalKeys2 : 2;
	uint8_t sequenceNum : 6;
	uint8_t analogTriggerL;
	uint8_t analogTriggerR;
	uint16_t timestamp;
	uint8_t batteryLevel;
	int16_t gyroPitch;
	int16_t gyroYaw;
	int16_t gyroRoll;
	int16_t accelX;
	int16_t accelY;
	int16_t accelZ;
	// some touch/other data
};

struct TriggerDataBond
{
	uint8_t motorMode;
	uint8_t startResistance;
	uint8_t effectForce;
	uint8_t rangeForce;
	uint8_t nearReleaseStrength;
	uint8_t nearMiddleStrength;
	uint8_t pressedStrength;
	uint8_t pad[2];
	uint8_t actuationFrequency;
	uint8_t pad2;
};

struct OutputReportJedi
{
	uint8_t setFlags;
	uint8_t pad[2];

	uint8_t vibrationRight;
	uint8_t vibrationLeft;

	uint8_t lightbarRed;
	uint8_t lightbarGreen;
	uint8_t lightbarBlue;

	uint8_t ledCycleOn;
	uint8_t ledCycleOff;
};

struct OutputReportBond
{
	uint8_t setFlags0;
	uint8_t setFlags1;

	uint8_t vibrationRight;
	uint8_t vibrationLeft;

	uint8_t headphoneVolume;
	uint8_t speakerVolume;
	uint8_t micVolume;
	uint8_t audioFlags : 4;
	uint8_t audioPath : 4;

	uint8_t muteButton;

	uint8_t powerSaveControl;
	
	TriggerDataBond triggerRight;
	TriggerDataBond triggerLeft;

	uint8_t pad[4];

	uint8_t triggerIntensity;
	uint8_t pad2;

	uint8_t setFlagsLed;
	uint8_t reserved3[2];
	uint8_t lightbarSetup;
	uint8_t ledBrightness;
	uint8_t playerLeds;
	uint8_t lightbarRed;
	uint8_t lightbarGreen;
	uint8_t lightbarBlue;

	~OutputReportBond()
	{
		static_assert(offsetof(OutputReportBond, triggerLeft) == 21, "triggerLeft");
		static_assert(offsetof(OutputReportBond, setFlagsLed) == 38, "setFlagsLed");
	}
};
#pragma pack(pop)

void SIEPadInternal::InputMain()
{
	auto initializePad = [this]()
	{
		SetLightBar(m_userColor[0], m_userColor[1], m_userColor[2]);
		SetVibration(0, 0);
	};

	auto getFeatureReport = [this](uint8_t reportId, void* outBuffer, size_t length)
	{
		if (m_isBluetooth)
		{
			if (reportId == kReportIdGetFirmInfoJedi)
			{
				reportId = 6;
				length += 4;
			}
		}

		*(uint8_t*)outBuffer = reportId;
		auto f = HidD_GetFeature(m_deviceHandle.get(), outBuffer, length);
		auto e = GetLastError();

		return (f && *(uint8_t*)outBuffer == reportId && !HasCrcErrorOnBtHid(0xA3, outBuffer, length));
	};

	auto parseInputReport = [this](const std::vector<uint8_t>& inputReport)
	{
		uint8_t reportId = inputReport[0];

		if (m_deviceType == DeviceType::Bond)
		{
			if (reportId == kReportIdInputReportBond || reportId == kReportIdInputReportBtBond)
			{
				if (m_isBluetooth)
				{
					if (HasCrcErrorOnBtHid(0xA1, &inputReport[0], inputReport.size()))
					{
						return;
					}
				}

				auto report = (InputReportBond*)&inputReport[0];
				// #TODO: parse and handle report
			}
		}
		else
		{
			if (reportId == kReportIdInputReportBond || reportId == kReportIdInputReportBtBond)
			{
				size_t off = 0;

				if (m_isBluetooth)
				{
					if (HasCrcErrorOnBtHid(0xA1, &inputReport[0], 78))
					{
						return;
					}

					off = 2;
				}

				auto report = (InputReportJedi*)&inputReport[off];
				// #TODO: parse and handle report
			}
		}
	};

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	unique_preparsed_data preparsedData;
	if (HidD_GetPreparsedData(m_deviceHandle.get(), &preparsedData))
	{
		HIDP_CAPS caps = { 0 };

		if (HidP_GetCaps(preparsedData.get(), &caps) == HIDP_STATUS_SUCCESS)
		{
			std::vector<uint8_t> fvcapBuf(caps.NumberFeatureValueCaps * sizeof(HIDP_VALUE_CAPS));
			auto fvcaps = (PHIDP_VALUE_CAPS)&fvcapBuf[0];
			USHORT fvcapsLength = fvcapBuf.size();

			// it's bluetooth if there's a CRC at the end of a feature report
			m_isBluetooth = caps.InputReportByteLength > 64;

			if (HidP_GetValueCaps(HidP_Feature, fvcaps, &fvcapsLength, preparsedData.get()) == HIDP_STATUS_SUCCESS)
			{
				if (caps.FeatureReportByteLength >= 0x35 && caps.InputReportByteLength >= 0x40)
				{
					// #TODO: check if fvcaps support feature reports we request
					std::vector<uint8_t> featureReportBuffer(caps.FeatureReportByteLength);
					std::vector<uint8_t> inputReportBuffer(caps.InputReportByteLength);

					// get firmware info
					std::string deviceType;
					std::string updateInfo = "";

					if (GetDeviceType() == DeviceType::Bond)
					{
						if (getFeatureReport(kReportIdGetFirmInfo, &featureReportBuffer[0], featureReportBuffer.size()))
						{
							m_firmwareInfo = {
								{ "buildDate", std::string{ (const char*)&featureReportBuffer[1], 11 } },
								{ "buildTime", std::string{ (const char*)&featureReportBuffer[12], 8 } },
								{ "fwType", fmt::sprintf("%x", *(uint16_t*)&featureReportBuffer[20]) },
								{ "swSeries", fmt::sprintf("%x", *(uint16_t*)&featureReportBuffer[22]) },
								{ "hwInfo", fmt::sprintf("%04x", *(uint32_t*)&featureReportBuffer[24]) },
								{ "mainFwVersion", fmt::sprintf("%x", *(uint32_t*)&featureReportBuffer[28]) },
								{ "deviceInfo", std::string{ (const char*)&featureReportBuffer[32], 12 } },
								{ "updateVersion", fmt::sprintf("%04x", *(uint16_t*)&featureReportBuffer[44]) },
								{ "updateImageInfo", fmt::sprintf("%x", featureReportBuffer[46]) },
								{ "sblFwVersion", fmt::sprintf("%x", *(uint32_t*)&featureReportBuffer[48]) },
								{ "venomFwVersion", fmt::sprintf("%x", *(uint32_t*)&featureReportBuffer[52]) },
								{ "spiderDspFwVersion", fmt::sprintf("%x", *(uint32_t*)&featureReportBuffer[56]) },
							};

							deviceType = "BOND " + m_firmwareInfo["hwInfo"];
							updateInfo = fmt::sprintf("update %s, ", m_firmwareInfo["updateVersion"]);
						}
					}
					else // jedi
					{
						if (getFeatureReport(kReportIdGetFirmInfoJedi, &featureReportBuffer[0], 0x31))
						{
							m_firmwareInfo = {
								{ "bcdDevRel", fmt::sprintf("%x", *(uint16_t*)&featureReportBuffer[33]) },
								{ "hwVersion", fmt::sprintf("%04x", *(uint16_t*)&featureReportBuffer[35]) },
								{ "swVersion", fmt::sprintf("%x", *(uint32_t*)&featureReportBuffer[37]) },
								{ "subSwVersion", fmt::sprintf("%x", *(uint16_t*)&featureReportBuffer[41]) },
								{ "swSeries", fmt::sprintf("%x", *(uint16_t*)&featureReportBuffer[43]) },
								{ "codeSize", fmt::sprintf("%x", *(uint32_t*)&featureReportBuffer[45]) },
								{ "buildDate", std::string{ (const char*)&featureReportBuffer[1], /*16*/11 } },
								{ "buildTime", std::string{ (const char*)&featureReportBuffer[17], /*17*/8 } },
							};

							deviceType = "JEDI " + m_firmwareInfo["hwVersion"];
						}
					}
					
					std::string btAddr = fmt::sprintf("%02x:%02x:%02x:%02x:%02x:%02x", m_btAddr[0], m_btAddr[1], m_btAddr[2], m_btAddr[3], m_btAddr[4], m_btAddr[5]);
					SIE_TRACE("Connected%s to %s %s (%sfirmware built at %s %s)\n", m_isBluetooth ? " via Bluetooth" : "", deviceType, btAddr, updateInfo, m_firmwareInfo["buildDate"], m_firmwareInfo["buildTime"]);					
					
					initializePad();

					while (true)
					{
						DWORD bytesRead = 0;
						if (!ReadFile(m_deviceHandle.get(), &inputReportBuffer[0], inputReportBuffer.size(), &bytesRead, NULL))
						{
							break;
						}

						parseInputReport(inputReportBuffer);
					}
				}
			}
		}
	}
}

void SIEPadInternal::SendOutputReportAsync(const void* report, uint16_t length, bool critical)
{
	std::vector<uint8_t> reportBytes(length);
	memcpy(&reportBytes[0], report, length);

	// #TODO: keep a ref
	m_mgr->QueueOutputEvent([this, reportBytes]()
	{
		if (m_hidCaps.OutputReportByteLength >= reportBytes.size())
		{
			auto outBytes = reportBytes;

			if (m_isBluetooth)
			{
				if (GetDeviceType() == DeviceType::Jedi)
				{
					// is this the Jedi's '5'? if so, wrap in a BT frame
					if (outBytes[0] == 5)
					{
						outBytes[0] = 0x11;
						outBytes.insert(outBytes.begin() + 1, 0xC0);
						outBytes.insert(outBytes.begin() + 2, 0x00);
					}
				}
				else // bond
				{
					if (outBytes[0] == 2)
					{
						outBytes[0] = 0x31;
						outBytes.insert(outBytes.begin() + 1, m_seq << 4);
						outBytes.insert(outBytes.begin() + 2, 0x10);

						m_seq++;
						if (m_seq >= 16)
						{
							m_seq = 0;
						}
					}
				}

				// both types of bluetooth reports expect to be padded to 74b
				outBytes.resize(74);

				Botan::CRC32 crc;
				crc.update(0xA2); // output seed
				crc.update(outBytes);

				// push in little endian
				auto crcBytes = crc.final();
				outBytes.push_back(crcBytes[3]);
				outBytes.push_back(crcBytes[2]);
				outBytes.push_back(crcBytes[1]);
				outBytes.push_back(crcBytes[0]);
			}

			DWORD bytesWritten = 0;
			WriteFile(m_deviceHandle.get(), &outBytes[0], outBytes.size(), &bytesWritten, NULL);
		}
	}, critical);
}

void SIEPadInternal::SetIndex(int playerIdx)
{
	if (m_deviceType != DeviceType::Bond)
	{
		return;
	}

	static const int ledMask[] = {
		(1 << 2),
		(1 << 3) | (1 << 1),
		(1 << 4) | (1 << 2) | (1 << 0),
		(1 << 4) | (1 << 3) | (1 << 1) | (1 << 0),
		(1 << 4) | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0),
	};

	uint8_t outputReport[48] = { 0 };
	outputReport[0] = 2;

	OutputReportBond& output = *(OutputReportBond*)(&outputReport[1]);
	output.setFlags1 |= (1 << 4); // index control
	output.playerLeds = ledMask[playerIdx % std::size(ledMask)];

	SendOutputReportAsync(outputReport, sizeof(outputReport), true);
}

void SIEPadInternal::SetLightBar(uint8_t r, uint8_t g, uint8_t b)
{
	if (m_lightbarColor[0] != r || m_lightbarColor[1] != g || m_lightbarColor[2] != b)
	{
		if (m_deviceType == DeviceType::Bond)
		{
			uint8_t outputReport[48] = { 0 };
			outputReport[0] = 2;

			OutputReportBond& output = *(OutputReportBond*)(&outputReport[1]);
			output.setFlags1 |= (1 << 2); // lightbar control
			output.setFlagsLed |= (1 << 1);

			output.lightbarSetup = 1 << 1;
			output.lightbarRed = (r / 255.0) * 255.0;
			output.lightbarGreen = (g / 255.0) * 255.0;
			output.lightbarBlue = (b / 255.0) * 255.0;

			SendOutputReportAsync(outputReport, sizeof(outputReport));
		}
		else
		{
			uint8_t outputReport[32] = { 0 };
			outputReport[0] = 5;

			OutputReportJedi& output = *(OutputReportJedi*)(&outputReport[1]);
			output.setFlags |= (1 << 1);
			output.lightbarRed = std::max(13.0, (r / 255.0) * 128.0);
			output.lightbarGreen = std::max(13.0, (g / 255.0) * 128.0);
			output.lightbarBlue = std::max(13.0, (b / 255.0) * 128.0);

			SendOutputReportAsync(outputReport, sizeof(outputReport));
		}

		m_lightbarColor[0] = r;
		m_lightbarColor[1] = g;
		m_lightbarColor[2] = b;
	}
}

void SIEPadInternal::SetVibration(uint8_t l, uint8_t r)
{
	// #TODO: set vibration
}

bool SIEPadManagerInternal::IsSIEPad(const HIDD_ATTRIBUTES& attributes)
{
	// it's a Sony(R)! (TM)
	if (attributes.VendorID == kVendorIDSony)
	{
		return (attributes.ProductID == kProductIDBond || attributes.ProductID == kProductIDJedi || attributes.ProductID == kProductIDJediPlus);
	}

	return false;
}

bool SIEPadInternal::HasCrcErrorOnBtHid(uint8_t crcType, const void* buffer, size_t bufferLength)
{
	if (!m_isBluetooth)
	{
		return false;
	}

	auto byteBuffer = reinterpret_cast<const uint8_t*>(buffer);
	auto reportCRC = *(uint32_t*)(byteBuffer + bufferLength - 4);

	Botan::CRC32 crc;
	crc.update(crcType);
	crc.update(byteBuffer, bufferLength - 4);

	return (_byteswap_ulong(*(uint32_t*)crc.final().data())) != reportCRC;
}

#ifdef TEST_SIENPUT
#define S(o, n) r[t[int(h[0]) / 60 * 3 + o] + o - 2] = (n + h[2] - c / 2) * 255;
void C(float* h, int* r)
{
	float g = 2 * h[2] - 1, c = (g < 0 ? 1 + g : 1 - g) * h[1], a = int(h[0]) % 120 / 60.f - 1;
	int t[] = { 2, 2, 2, 3, 1, 2, 3, 3, 0, 4, 2, 0, 4, 1, 1, 2, 3, 1 };
	S(0, c)
	S(1, c * (a < 0 ? 1 + a : 1 - a))
	S(2, 0)
}

static InitFunction initFunction([]()
{
	auto manager = std::make_unique<SIEPadManagerInternal>();

	while (true)
	{
		Sleep(50);

		float hsv[3];
		int rgb[3];

		hsv[0] = (sinf(GetTickCount() / 1500.0) + 1.0f) * 179.4f;
		hsv[1] = 1.0f;
		hsv[2] = 0.5f;

		C(hsv, rgb);

		auto pad = manager->GetPad(0);
		auto pad2 = manager->GetPad(1);

		if (pad)
		{
			pad->SetLightBar(rgb[0], rgb[1], rgb[2]);
		}

		if (pad2)
		{
			pad2->SetLightBar(rgb[0], rgb[1], rgb[2]);
		}
	}
});
#endif
