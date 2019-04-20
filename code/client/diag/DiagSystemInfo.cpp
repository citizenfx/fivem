#include <StdInc.h>
#include "DiagLocal.h"

static InitFunction initFunction([]()
{
	RegisterCfxDiagnosticsCommand("ComputerName", [](const std::shared_ptr<CfxDiagnosticsOutputBuffer>& buffer)
	{
		buffer->AppendText(ToNarrow(_wgetenv(L"COMPUTERNAME")));
	});

	RegisterCfxDiagnosticsCommand("WhoAmI", [](const std::shared_ptr<CfxDiagnosticsOutputBuffer>& buffer)
	{
		buffer->AppendCommand("whoami");
	});
}, -1000);
