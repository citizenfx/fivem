// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/base/shared_memory_utils.h"

#include "base/memory/shared_memory_mapping.h"
#include "mojo/public/cpp/system/buffer.h"
#include "mojo/public/cpp/system/platform_handle.h"

namespace mojo {

base::MappedReadOnlyRegion CreateReadOnlySharedMemoryRegion(size_t size) {
  auto writable_region = CreateWritableSharedMemoryRegion(size);
  if (!writable_region.IsValid())
    return {};

  base::WritableSharedMemoryMapping mapping = writable_region.Map();
  return {base::WritableSharedMemoryRegion::ConvertToReadOnly(
              std::move(writable_region)),
          std::move(mapping)};
}

base::UnsafeSharedMemoryRegion CreateUnsafeSharedMemoryRegion(size_t size) {
  auto writable_region = CreateWritableSharedMemoryRegion(size);
  if (!writable_region.IsValid())
    return base::UnsafeSharedMemoryRegion();

  return base::WritableSharedMemoryRegion::ConvertToUnsafe(
      std::move(writable_region));
}

base::WritableSharedMemoryRegion CreateWritableSharedMemoryRegion(size_t size) {
  mojo::ScopedSharedBufferHandle handle =
      mojo::SharedBufferHandle::Create(size);
  if (!handle.is_valid())
    return base::WritableSharedMemoryRegion();

  return mojo::UnwrapWritableSharedMemoryRegion(std::move(handle));
}

}  // namespace mojo
