// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_BINDINGS_ASSOCIATED_BINDING_SET_H_
#define MOJO_PUBLIC_CPP_BINDINGS_ASSOCIATED_BINDING_SET_H_

#include "mojo/public/cpp/bindings/associated_binding.h"
#include "mojo/public/cpp/bindings/associated_interface_ptr.h"
#include "mojo/public/cpp/bindings/associated_interface_request.h"
#include "mojo/public/cpp/bindings/binding_set.h"

namespace mojo {

template <typename Interface, typename ImplRefTraits>
struct BindingSetTraits<AssociatedBinding<Interface, ImplRefTraits>> {
  using ProxyType = AssociatedInterfacePtr<Interface>;
  using RequestType = AssociatedInterfaceRequest<Interface>;
  using BindingType = AssociatedBinding<Interface, ImplRefTraits>;
  using ImplPointerType = typename BindingType::ImplPointerType;
};

template <typename Interface, typename ContextType = void>
using AssociatedBindingSet =
    BindingSetBase<Interface, AssociatedBinding<Interface>, ContextType>;

}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_BINDINGS_ASSOCIATED_BINDING_SET_H_
