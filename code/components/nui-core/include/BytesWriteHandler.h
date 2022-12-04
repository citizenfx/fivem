// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#pragma once

#include "include/cef_stream.h"

#include <mutex>

class BytesWriteHandler : public CefWriteHandler {
public:
	explicit BytesWriteHandler(size_t grow);
	~BytesWriteHandler();

	size_t Write(const void* ptr, size_t size, size_t n) override;
	int Seek(int64 offset, int whence) override;
	int64 Tell() override;
	int Flush() override;
	bool MayBlock() override
	{
		return false;
	}

	void* GetData() { return data_; }
	int64 GetDataSize() { return offset_; }

private:
	size_t Grow(size_t size);

	size_t grow_;
	void* data_;
	int64 datasize_;
	int64 offset_;

	std::recursive_mutex lock_;

	IMPLEMENT_REFCOUNTING(BytesWriteHandler);
	DISALLOW_COPY_AND_ASSIGN(BytesWriteHandler);
};
