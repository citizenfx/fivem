#pragma once

#include <HttpClient.h>

class ExtDownloader
{
public:
	virtual std::string StartDownload(const std::string& url, const std::string& filePath, const HttpRequestOptions& options, const std::function<void()>& doneCb) = 0;

	virtual void SetRequestWeight(const std::string& gid, int newWeight) = 0;

	virtual ~ExtDownloader() = default;
};

std::shared_ptr<ExtDownloader> CreateExtDownloader();
