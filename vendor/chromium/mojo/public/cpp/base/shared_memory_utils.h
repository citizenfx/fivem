// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BASE_SHARED_MEMORY_UTILS_H_
#define MOJO_PUBLIC_CPP_BASE_SHARED_MEMORY_UTILS_H_

#include "base/component_export.h"
#include "base/memory/read_only_shared_memory_region.h"
#include "base/memory/unsafe_shared_memory_region.h"
#include "base/memory/writable_shared_memory_region.h"

namespace mojo {

// These creation methods are parallel to the base::*SharedMemoryRegion::Create
// methods. These methods should be used instead of the base:: ones to create
// shared memory in an unprivileged context, in which case a broker in a
// privileged process will be used to create the region.
//
// IsValid() should be checked on the return value of the following methods to
// determine if the creation was successful.
COMPONENT_EXPORT(MOJO_BASE)
base::MappedReadOnlyRegion CreateReadOnlySharedMemoryRegion(size_t size);
COMPONENT_EXPORT(MOJO_BASE)
base::UnsafeSharedMemoryRegion CreateUnsafeSharedMemoryRegion(size_t size);
COMPONENT_EXPORT(MOJO_BASE)
base::WritableSharedMemoryRegion CreateWritableSharedMemoryRegion(size_t size);

}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BASE_SHARED_MEMORY_UTILS_H_
