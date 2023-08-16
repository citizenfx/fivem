// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/bindings/lib/fixed_buffer.h"

#include <stdlib.h>

#include "mojo/public/cpp/bindings/lib/bindings_internal.h"

namespace mojo {
namespace internal {

FixedBufferForTesting::FixedBufferForTesting(size_t size)
    : Buffer(calloc(Align(size), 1), Align(size), 0) {}

FixedBufferForTesting::~FixedBufferForTesting() {
  free(data());
}

}  // namespace internal
}  // namespace mojo
