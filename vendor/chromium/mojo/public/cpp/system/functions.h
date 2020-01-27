// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file provides a C++ wrapping around the standalone functions of the Mojo
// C API, replacing the prefix of "Mojo" with a "mojo" namespace.
//
// Please see "mojo/public/c/system/functions.h" for complete documentation of
// the API.

#ifndef MOJO_PUBLIC_CPP_SYSTEM_FUNCTIONS_H_
#define MOJO_PUBLIC_CPP_SYSTEM_FUNCTIONS_H_

#include "mojo/public/c/system/functions.h"

namespace mojo {

// Returns the current |MojoTimeTicks| value. See |MojoGetTimeTicksNow()| for
// complete documentation.
inline MojoTimeTicks GetTimeTicksNow() {
  return MojoGetTimeTicksNow();
}

}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_SYSTEM_FUNCTIONS_H_
