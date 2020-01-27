// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BINDINGS_STRONG_BINDING_SET_H_
#define MOJO_PUBLIC_CPP_BINDINGS_STRONG_BINDING_SET_H_

#include "mojo/public/cpp/bindings/binding_set.h"
#include "mojo/public/cpp/bindings/unique_ptr_impl_ref_traits.h"

namespace mojo {

// This class manages a set of bindings. When the pipe a binding is bound to is
// disconnected, the binding is automatically destroyed and removed from the
// set, and the interface implementation is deleted. When the StrongBindingSet
// is destructed, all outstanding bindings in the set are destroyed and all the
// bound interface implementations are automatically deleted.
template <typename Interface,
          typename ContextType = void,
          typename Deleter = std::default_delete<Interface>>
using StrongBindingSet = BindingSetBase<
    Interface,
    Binding<Interface, UniquePtrImplRefTraits<Interface, Deleter>>,
    ContextType>;

}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BINDINGS_STRONG_BINDING_SET_H_
