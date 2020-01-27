// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BINDINGS_INTERFACE_ID_H_
#define MOJO_PUBLIC_CPP_BINDINGS_INTERFACE_ID_H_

#include <stdint.h>

namespace mojo {

// The size of the type matters because it is directly used in messages.
using InterfaceId = uint32_t;

// IDs of associated interface can be generated at both sides of the message
// pipe. In order to avoid collision, the highest bit is used as namespace bit:
// at the side where the client-side of the master interface lives, IDs are
// generated with the namespace bit set to 1; at the opposite side IDs are
// generated with the namespace bit set to 0.
const uint32_t kInterfaceIdNamespaceMask = 0x80000000;

const InterfaceId kMasterInterfaceId = 0x00000000;
const InterfaceId kInvalidInterfaceId = 0xFFFFFFFF;

inline bool IsMasterInterfaceId(InterfaceId id) {
  return id == kMasterInterfaceId;
}

inline bool IsValidInterfaceId(InterfaceId id) {
  return id != kInvalidInterfaceId;
}

inline bool HasInterfaceIdNamespaceBitSet(InterfaceId id) {
  return (id & kInterfaceIdNamespaceMask) != 0;
}

}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BINDINGS_INTERFACE_ID_H_
