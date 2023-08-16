// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/bindings/associated_interface_ptr.h"

namespace mojo {

void AssociateWithDisconnectedPipe(ScopedInterfaceEndpointHandle handle) {
  MessagePipe pipe;
  scoped_refptr<internal::MultiplexRouter> router =
      new internal::MultiplexRouter(
          std::move(pipe.handle0), internal::MultiplexRouter::MULTI_INTERFACE,
          false, base::SequencedTaskRunnerHandle::Get());
  router->AssociateInterface(std::move(handle));
}

}  // namespace mojo
