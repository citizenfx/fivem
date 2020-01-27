// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/base_switches.h"
#include "base/bind.h"
#include "base/test/launcher/unit_test_launcher.h"
#include "base/test/test_suite.h"
#include "build/build_config.h"
#include "mojo/public/c/system/core.h"

int main(int argc, char** argv) {
  base::TestSuite test_suite(argc, argv);

  MojoInitializeOptions options;
  options.struct_size = sizeof(options);
  options.flags = MOJO_INITIALIZE_FLAG_NONE;
  options.mojo_core_path = NULL;
  options.mojo_core_path_length = 0;
  if (!base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kTestChildProcess)) {
    options.flags = MOJO_INITIALIZE_FLAG_AS_BROKER;
  }

  CHECK_EQ(MOJO_RESULT_OK, MojoInitialize(&options));
  int result = base::LaunchUnitTests(
      argc, argv,
      base::BindOnce(&base::TestSuite::Run, base::Unretained(&test_suite)));

  CHECK_EQ(MOJO_RESULT_OK, MojoShutdown(nullptr));
  return result;
}
