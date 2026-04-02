// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

// From: https://github.com/chromiumembedded/cef/blob/master/tests/cefclient/browser/bytes_write_handler.h
#pragma once

#include "include/cef_stream.h"

class BytesWriteHandler : public CefWriteHandler
{
public:
	explicit BytesWriteHandler(size_t grow);
	~BytesWriteHandler() override;

	BytesWriteHandler(const BytesWriteHandler&) = delete;
	BytesWriteHandler& operator=(const BytesWriteHandler&) = delete;

	size_t Write(const void* ptr, size_t size, size_t n) override;
	int Seek(int64_t offset, int whence) override;
	int64_t Tell() override;
	int Flush() override;
	bool MayBlock() override
	{
		return false;
	}

	void* GetData()
	{
		return data_;
	}
	int64_t GetDataSize()
	{
		return offset_;
	}

private:
	size_t Grow(size_t size);

	size_t grow_;
	void* data_;
	int64_t datasize_;
	int64_t offset_ = 0;

	base::Lock lock_;

	IMPLEMENT_REFCOUNTING(BytesWriteHandler);
};
