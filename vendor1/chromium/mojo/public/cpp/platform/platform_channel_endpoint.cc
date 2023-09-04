// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mojo/public/cpp/platform/platform_channel_endpoint.h"

namespace mojo {

PlatformChannelEndpoint::PlatformChannelEndpoint() = default;

PlatformChannelEndpoint::PlatformChannelEndpoint(
    PlatformChannelEndpoint&& other) = default;

PlatformChannelEndpoint::PlatformChannelEndpoint(PlatformHandle handle)
    : handle_(std::move(handle)) {}

PlatformChannelEndpoint::~PlatformChannelEndpoint() = default;

PlatformChannelEndpoint& PlatformChannelEndpoint::operator=(
    PlatformChannelEndpoint&& other) = default;

void PlatformChannelEndpoint::reset() {
  handle_.reset();
}

PlatformChannelEndpoint PlatformChannelEndpoint::Clone() const {
  return PlatformChannelEndpoint(handle_.Clone());
}

}  // namespace mojo
