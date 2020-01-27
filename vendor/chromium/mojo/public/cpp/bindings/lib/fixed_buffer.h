// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BINDINGS_LIB_FIXED_BUFFER_H_
#define MOJO_PUBLIC_CPP_BINDINGS_LIB_FIXED_BUFFER_H_

#include <cstddef>

#include "base/component_export.h"
#include "base/macros.h"
#include "mojo/public/cpp/bindings/lib/buffer.h"

namespace mojo {
namespace internal {

// FixedBufferForTesting owns its buffer. The Leak method may be used to steal
// the underlying memory.
class COMPONENT_EXPORT(MOJO_CPP_BINDINGS_BASE) FixedBufferForTesting
    : public Buffer {
 public:
  explicit FixedBufferForTesting(size_t size);
  ~FixedBufferForTesting();

 private:
  DISALLOW_COPY_AND_ASSIGN(FixedBufferForTesting);
};

}  // namespace internal
}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BINDINGS_LIB_FIXED_BUFFER_H_
