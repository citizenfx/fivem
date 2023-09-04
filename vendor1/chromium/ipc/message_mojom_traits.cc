// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ipc/message_mojom_traits.h"

#include "mojo/public/cpp/base/big_buffer_mojom_traits.h"

namespace mojo {

// static
mojo_base::BigBufferView
StructTraits<IPC::mojom::MessageDataView, IPC::MessageView>::buffer(
    IPC::MessageView& view) {
  return view.TakeBufferView();
}

// static
base::Optional<std::vector<mojo::native::SerializedHandlePtr>>
StructTraits<IPC::mojom::MessageDataView, IPC::MessageView>::handles(
    IPC::MessageView& view) {
  return view.TakeHandles();
}

// static
bool StructTraits<IPC::mojom::MessageDataView, IPC::MessageView>::Read(
    IPC::mojom::MessageDataView data,
    IPC::MessageView* out) {
  mojo_base::BigBufferView buffer_view;
  if (!data.ReadBuffer(&buffer_view))
    return false;
  base::Optional<std::vector<mojo::native::SerializedHandlePtr>> handles;
  if (!data.ReadHandles(&handles))
    return false;

  *out = IPC::MessageView(std::move(buffer_view), std::move(handles));
  return true;
}

}  // namespace mojo
