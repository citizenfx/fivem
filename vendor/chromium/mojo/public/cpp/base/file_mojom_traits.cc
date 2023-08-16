// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/base/file_mojom_traits.h"
#include "base/files/file.h"
#include "mojo/public/cpp/system/platform_handle.h"

namespace mojo {

mojo::ScopedHandle StructTraits<mojo_base::mojom::FileDataView, base::File>::fd(
    base::File& file) {
  DCHECK(file.IsValid());

  return mojo::WrapPlatformFile(file.TakePlatformFile());
}

bool StructTraits<mojo_base::mojom::FileDataView, base::File>::Read(
    mojo_base::mojom::FileDataView data,
    base::File* file) {
  base::PlatformFile platform_handle = base::kInvalidPlatformFile;
  if (mojo::UnwrapPlatformFile(data.TakeFd(), &platform_handle) !=
      MOJO_RESULT_OK) {
    return false;
  }
  *file = base::File(platform_handle, data.async());
  return true;
}

}  // namespace mojo
