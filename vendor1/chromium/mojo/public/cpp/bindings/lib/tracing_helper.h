// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BINDINGS_LIB_TRACING_HELPER_H_
#define MOJO_PUBLIC_CPP_BINDINGS_LIB_TRACING_HELPER_H_

#define MANGLE_MESSAGE_ID(id) (id ^ ::mojo::internal::kMojoMessageMangleMask)

namespace mojo {
namespace internal {

// Mojo message id is 32-bit, but for tracing we ensure that mojo messages
// don't collide with other trace events.
constexpr uint64_t kMojoMessageMangleMask = 0x655b2a8e8efdf27f;

}  // namespace internal
}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BINDINGS_LIB_TRACING_HELPER_H_
