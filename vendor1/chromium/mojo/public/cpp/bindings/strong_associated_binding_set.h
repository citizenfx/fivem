// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BINDINGS_STRONG_ASSOCIATED_BINDING_SET_H_
#define MOJO_PUBLIC_CPP_BINDINGS_STRONG_ASSOCIATED_BINDING_SET_H_

#include "mojo/public/cpp/bindings/associated_binding.h"
#include "mojo/public/cpp/bindings/associated_binding_set.h"
#include "mojo/public/cpp/bindings/associated_interface_ptr.h"
#include "mojo/public/cpp/bindings/associated_interface_request.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "mojo/public/cpp/bindings/unique_ptr_impl_ref_traits.h"

namespace mojo {

template <typename Interface, typename ContextType = void>
using StrongAssociatedBindingSet = BindingSetBase<
    Interface,
    AssociatedBinding<Interface, UniquePtrImplRefTraits<Interface>>,
    ContextType>;

}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BINDINGS_STRONG_ASSOCIATED_BINDING_SET_H_
