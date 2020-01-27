# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest

from mojom_bindings_generator import MakeImportStackMessage
from mojom_bindings_generator import ScrambleMethodOrdinals


class FakeIface(object):
  def __init__( self ):
    self.name = None
    self.methods = None


class FakeMethod(object):
  def __init__( self ):
    self.ordinal = None
    self.ordinal_comment = None


class MojoBindingsGeneratorTest(unittest.TestCase):
  """Tests mojo_bindings_generator."""

  def testMakeImportStackMessage(self):
    """Tests MakeImportStackMessage()."""
    self.assertEquals(MakeImportStackMessage(["x"]), "")
    self.assertEquals(MakeImportStackMessage(["x", "y"]),
        "\n  y was imported by x")
    self.assertEquals(MakeImportStackMessage(["x", "y", "z"]),
        "\n  z was imported by y\n  y was imported by x")

  def testScrambleMethodOrdinals(self):
    """Tests ScrambleMethodOrdinals()."""
    interface = FakeIface()
    interface.name = 'RendererConfiguration'
    interface.methods = [FakeMethod(), FakeMethod(), FakeMethod()]
    ScrambleMethodOrdinals([interface], "foo")
    # These next three values are hard-coded. If the generation algorithm
    # changes from being based on sha256(seed + interface.name + str(i)) then
    # these numbers will obviously need to change too.
    #
    # Note that hashlib.sha256('fooRendererConfiguration1').digest()[:4] is
    # '\xa5\xbc\xf9\xca' and that hex(1257880741) = '0x4af9bca5'. The
    # difference in 0x4a vs 0xca is because we only take 31 bits.
    self.assertEquals(interface.methods[0].ordinal, 1257880741)
    self.assertEquals(interface.methods[1].ordinal, 631133653)
    self.assertEquals(interface.methods[2].ordinal, 549336076)


if __name__ == "__main__":
  unittest.main()
