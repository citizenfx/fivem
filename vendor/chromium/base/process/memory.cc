// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/memory.h"
#include "base/debug/alias.h"
#include "base/logging.h"
#include "base/partition_alloc_buildflags.h"
#if BUILDFLAG(USE_PARTITION_ALLOC)
#include "base/allocator/partition_allocator/page_allocator.h"
#endif
#include "build/build_config.h"

namespace base {

// Defined in memory_win.cc for Windows.
#if !defined(OS_WIN)

namespace {

// Breakpad server classifies base::`anonymous namespace'::OnNoMemory as
// out-of-memory crash.
NOINLINE void OnNoMemory(size_t size) {
  size_t tmp_size = size;
  base::debug::Alias(&tmp_size);
  LOG(FATAL) << "Out of memory. size=" << tmp_size;
}

}  // namespace

void TerminateBecauseOutOfMemory(size_t size) {
  OnNoMemory(size);
}

#endif  // !defined(OS_WIN)

// Defined in memory_mac.mm for Mac.
#if !defined(OS_MACOSX)

bool UncheckedCalloc(size_t num_items, size_t size, void** result) {
  const size_t alloc_size = num_items * size;

  // Overflow check
  if (size && ((alloc_size / size) != num_items)) {
    *result = nullptr;
    return false;
  }

  if (!UncheckedMalloc(alloc_size, result))
    return false;

  memset(*result, 0, alloc_size);
  return true;
}

#endif  // defined(OS_MACOSX)

namespace internal {
bool ReleaseAddressSpaceReservation() {
#if BUILDFLAG(USE_PARTITION_ALLOC)
  return ReleaseReservation();
#else
  return false;
#endif
}
}  // namespace internal

}  // namespace base
