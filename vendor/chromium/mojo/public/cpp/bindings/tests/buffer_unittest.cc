// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include <limits>

#include "mojo/public/cpp/bindings/lib/fixed_buffer.h"
#include "mojo/public/cpp/bindings/lib/serialization_util.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace mojo {
namespace test {
namespace {

// Tests that FixedBuffer allocates memory aligned to 8 byte boundaries.
TEST(FixedBufferTest, Alignment) {
  internal::FixedBufferForTesting buf(internal::Align(10) * 2);
  ASSERT_EQ(buf.size(), 16u * 2);

  size_t a = buf.Allocate(10);
  EXPECT_EQ(0u, a);

  size_t b = buf.Allocate(10);
  ASSERT_EQ(16u, b);

  // Any more allocations would result in an assert, but we can't test that.
}

}  // namespace
}  // namespace test
}  // namespace mojo
