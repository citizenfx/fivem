#pragma once

#include <chrono>
#include <sstream>

class CfxDiagnosticsOutputBuffer
{
public:
	void AppendText(const std::string& data);

	bool AppendCommand(const std::string& commandLine, std::chrono::milliseconds timeout = std::chrono::milliseconds(15000));

	std::string ToString();

private:
	std::stringstream m_outputStream;
};

using TDiagnosticsFunction = std::function<void(const std::shared_ptr<CfxDiagnosticsOutputBuffer>&)>;

void RegisterCfxDiagnosticsCommand(const std::string& label, const TDiagnosticsFunction& function);
