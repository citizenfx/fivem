#!/usr/bin/env python
#
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# TODO(brettw) bug 582594: merge this with build/android/gn/zip.py and update
# callers to use the existing template rather than invoking this directly.

"""Archives a set of files.
"""

import optparse
import os
import sys
import zipfile

sys.path.append(os.path.join(os.path.dirname(__file__),
                             os.pardir, os.pardir, os.pardir, os.pardir,
                             "build"))
import gn_helpers

sys.path.append(os.path.join(os.path.dirname(__file__),
                             os.pardir, os.pardir, os.pardir, os.pardir,
                             'build', 'android', 'gyp'))
from util import build_utils


def DoZip(inputs, link_inputs, zip_inputs, output, base_dir):
  files = []
  with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as outfile:
    for f in inputs:
      file_name = os.path.relpath(f, base_dir)
      files.append(file_name)
      build_utils.AddToZipHermetic(outfile, file_name, f)
    for f in link_inputs:
      realf = os.path.realpath(f)  # Resolve symlinks.
      file_name = os.path.relpath(realf, base_dir)
      files.append(file_name)
      build_utils.AddToZipHermetic(outfile, file_name, realf)
    for zf_name in zip_inputs:
      with zipfile.ZipFile(zf_name, 'r') as zf:
        for f in zf.namelist():
          if f not in files:
            files.append(f)
            build_utils.AddToZipHermetic(outfile, f, data=zf.read(f))


def main():
  parser = optparse.OptionParser()

  parser.add_option('--inputs',
      help='GN format list of files to archive.')
  parser.add_option('--link-inputs',
      help='GN-format list of files to archive. Symbolic links are resolved.')
  parser.add_option('--zip-inputs',
      help='GN-format list of zip files to re-archive.')
  parser.add_option('--output', help='Path to output archive.')
  parser.add_option('--base-dir',
                    help='If provided, the paths in the archive will be '
                    'relative to this directory', default='.')

  options, _ = parser.parse_args()

  inputs = []
  if (options.inputs):
    parser = gn_helpers.GNValueParser(options.inputs)
    inputs = parser.ParseList()

  link_inputs = []
  if options.link_inputs:
    parser = gn_helpers.GNValueParser(options.link_inputs)
    link_inputs = parser.ParseList()

  zip_inputs = []
  if options.zip_inputs:
    parser = gn_helpers.GNValueParser(options.zip_inputs)
    zip_inputs = parser.ParseList()

  output = options.output
  base_dir = options.base_dir

  DoZip(inputs, link_inputs, zip_inputs, output, base_dir)

if __name__ == '__main__':
  sys.exit(main())
