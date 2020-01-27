// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_PUBLIC_CPP_TEST_SUPPORT_TEST_UTILS_H_
#define MOJO_PUBLIC_CPP_TEST_SUPPORT_TEST_UTILS_H_

#include <string>
#include <utility>

#include "base/macros.h"
#include "base/run_loop.h"
#include "mojo/public/cpp/bindings/message.h"
#include "mojo/public/cpp/system/core.h"

namespace mojo {
namespace test {

template <typename MojomType, typename UserType>
bool SerializeAndDeserialize(UserType* input, UserType* output) {
  mojo::Message message = MojomType::SerializeAsMessage(input);

  // This accurately simulates full serialization to ensure that all attached
  // handles are serialized as well. Necessary for DeserializeFromMessage to
  // work properly.
  mojo::ScopedMessageHandle handle = message.TakeMojoMessage();
  message = mojo::Message::CreateFromMessageHandle(&handle);
  DCHECK(!message.IsNull());

  return MojomType::DeserializeFromMessage(std::move(message), output);
}

// Writes a message to |handle| with message data |text|. Returns true on
// success.
bool WriteTextMessage(const MessagePipeHandle& handle, const std::string& text);

// Reads a message from |handle|, putting its contents into |*text|. Returns
// true on success. (This blocks if necessary and will call |MojoReadMessage()|
// multiple times, e.g., to query the size of the message.)
bool ReadTextMessage(const MessagePipeHandle& handle, std::string* text);

// Discards a message from |handle|. Returns true on success. (This does not
// block. It will fail if no message is available to discard.)
bool DiscardMessage(const MessagePipeHandle& handle);

// Run |single_iteration| an appropriate number of times and report its
// performance appropriately. (This actually runs |single_iteration| for a fixed
// amount of time and reports the number of iterations per unit time.)
typedef void (*PerfTestSingleIteration)(void* closure);
void IterateAndReportPerf(const char* test_name,
                          const char* sub_test_name,
                          PerfTestSingleIteration single_iteration,
                          void* closure);

// Intercepts a single bad message (reported via mojo::ReportBadMessage or
// mojo::GetBadMessageCallback) that would be associated with the global bad
// message handler (typically when the messages originate from a test
// implementation of an interface hosted in the test process).
class BadMessageObserver {
 public:
  BadMessageObserver();
  ~BadMessageObserver();

  // Waits for the bad message and returns the error string.
  std::string WaitForBadMessage();

  // Returns true iff a bad message was already received.
  bool got_bad_message() const { return got_bad_message_; }

 private:
  void OnReportBadMessage(const std::string& message);

  std::string last_error_for_bad_message_;
  bool got_bad_message_;
  base::RunLoop run_loop_;

  DISALLOW_COPY_AND_ASSIGN(BadMessageObserver);
};

}  // namespace test
}  // namespace mojo

#endif  // MOJO_PUBLIC_CPP_TEST_SUPPORT_TEST_UTILS_H_
