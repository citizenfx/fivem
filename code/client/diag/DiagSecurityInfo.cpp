#include <StdInc.h>
#include "DiagLocal.h"

#include <wscapi.h>
#include <iwscapi.h>

#include <OleAuto.h>

// Function declaration
static std::string GetSecurityCenterInfo(WSC_SECURITY_PROVIDER provider);

// Initialization function
static InitFunction initFunction([]() {
    RegisterCfxDiagnosticsCommand("SecurityCenter", [](const std::shared_ptr<CfxDiagnosticsOutputBuffer>& buffer) {
        buffer->AppendText(GetSecurityCenterInfo(WSC_SECURITY_PROVIDER_FIREWALL));
        buffer->AppendText(GetSecurityCenterInfo(WSC_SECURITY_PROVIDER_ANTIVIRUS));
        buffer->AppendText(GetSecurityCenterInfo(WSC_SECURITY_PROVIDER_ANTISPYWARE));
    });
});

// Function definition
static std::string GetSecurityCenterInfo(WSC_SECURITY_PROVIDER provider) {
    std::stringstream outStream;

    try {
        Microsoft::WRL::ComPtr<IWSCProductList> productList;
        HRESULT hr = CoCreateInstance(__uuidof(WSCProductList), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWSCProductList), productList.GetAddressOf());

        if (FAILED(hr)) {
            throw std::runtime_error(fmt::sprintf("CoCreateInstance returned error: 0x%x", hr));
        }

        hr = productList->Initialize(provider);
        if (FAILED(hr)) {
            throw std::runtime_error(fmt::sprintf("Initialize failed with error: 0x%x", hr));
        }

        LONG productCount = 0;
        hr = productList->get_Count(&productCount);
        if (FAILED(hr)) {
            throw std::runtime_error(fmt::sprintf("get_Count failed with error: 0x%x", hr));
        }

        // ... (rest of the code)

    } catch (const std::exception& e) {
        outStream << "Error: " << e.what() << std::endl;
    }

    return outStream.str();
}
