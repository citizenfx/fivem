// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/system/data_pipe.h"

namespace mojo {

namespace {

NOINLINE void CrashMojoResourceExhausted() {
  LOG(FATAL)
      << "Failed to create data pipe due to MOJO_RESULT_RESOURCE_EXHAUSTED.";
}

void CrashIfResultNotOk(MojoResult result) {
  if (LIKELY(result == MOJO_RESULT_OK))
    return;

  // Include some extra information for resource exhausted failures.
  if (result == MOJO_RESULT_RESOURCE_EXHAUSTED)
    CrashMojoResourceExhausted();

  LOG(FATAL) << "Failed to create data pipe; result=" << result;
}

}  // namespace

DataPipe::DataPipe() {
  MojoResult result =
      CreateDataPipe(nullptr, &producer_handle, &consumer_handle);
  CrashIfResultNotOk(result);
}

DataPipe::DataPipe(uint32_t capacity_num_bytes) {
  MojoCreateDataPipeOptions options;
  options.struct_size = sizeof(MojoCreateDataPipeOptions);
  options.flags = MOJO_CREATE_DATA_PIPE_FLAG_NONE;
  options.element_num_bytes = 1;
  options.capacity_num_bytes = capacity_num_bytes;
  MojoResult result =
      CreateDataPipe(&options, &producer_handle, &consumer_handle);
  CrashIfResultNotOk(result);
}

DataPipe::DataPipe(const MojoCreateDataPipeOptions& options) {
  MojoResult result =
      CreateDataPipe(&options, &producer_handle, &consumer_handle);
  CrashIfResultNotOk(result);
}

DataPipe::~DataPipe() {}

}  // namespace mojo
