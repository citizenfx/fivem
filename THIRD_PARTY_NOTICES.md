# Third-Party Notices

This repository includes third-party software distributed under its own license terms. Those components are not covered by the license in [LICENSE](LICENSE); each remains subject to the license listed below. 

Each entry lists the component, the license it is distributed under, and (where applicable) its copyright notice. The full text of each distinct license is reproduced in [Section 4](#4-full-license-texts) and in the [`LICENSES/`](LICENSES/) directory; the authoritative per-component license file is also retained in the source tree. 

Take-Two Interactive Software, Inc. hereby disclaims all copyright interest in the Files described above written by its contributors.

---

## 1. Embedded third-party code

| Component | Location | License |
|-----------|----------|---------|
| Fast DXT (`dxt.h`) | `code/components/font-renderer/src/3rdparty/dxt.h` | [LGPL-2.1](LICENSES/LGPL-2.1.txt) |
| ImFileDialog (dfranx) | `code/components/glue/src/imfd/` | [MIT](LICENSES/MIT.txt) |
| FFmpeg / libav headers | `code/components/voip-mumble/include/` | [LGPL-2.1-or-later](LICENSES/LGPL-2.1.txt) |
| Mumble | `code/components/voip-server-mumble/src/` | [BSD-3-Clause](LICENSES/BSD-3-Clause.txt) |
| Mumble `GlobalInput` | game-interface components | [BSD-3-Clause](LICENSES/BSD-3-Clause.txt) |
| Microsoft PDB | `code/tools/dbg/msft_pdb/` | [MIT](LICENSES/MIT.txt) |
| Microsoft `dxerr` | game-interface components | [MIT](LICENSES/MIT.txt) |
| textselect | `code/components/conhost-v2/src/` | [MIT](LICENSES/MIT.txt) |
| Aru's GTA IV script hook | `code/components/rage-scripting-{five,ny,rdr3}` | [Zlib](LICENSES/Zlib.txt) |
| RageLib (Arushan/Aru) | `code/components/rage-formats-x` | [Zlib](LICENSES/Zlib.txt) |

---

## 2. Vendored dependencies

### [MIT License](LICENSES/MIT.txt)

| Component | Copyright |
|-----------|-----------|
| cpr | Copyright (c) 2017-2021 Huu Nguyen |
| directxtex | Copyright (c) 2011-2020 Microsoft Corp |
| dspfilters | Copyright (c) 2009 Vinnie Falco |
| fmtlib | Copyright (c) 2012-present, Victor Zverovich |
| fx11 | Copyright (c) 2009-2021 Microsoft Corp |
| glm | Copyright (c) 2005 G-Truc Creation |
| hdiffpatch | Copyright (c) 2012-2022 housisong |
| imgui | Copyright (c) 2014-2022 Omar Cornut |
| jwt-cpp | Copyright (c) 2018 Dominik Thalhammer |
| lua-cmsgpack | Copyright (c) 2012 Salvatore Sanfilippo |
| lua-rapidjson | Copyright (c) 2015 Xpol Wan |
| nghttp2 | Copyright (c) 2012-2016 Tatsuhiro Tsujikawa |
| nljson (JSON for Modern C++) | Copyright (c) 2013-2022 Niels Lohmann |
| nng / nngpp | Copyright (c) nanomsg contributors; nngpp (c) 2018 Chris Welshman |
| picohttpparser | Copyright (c) 2009-2014 Kazuho Oku et al. |
| prometheus-cpp | Copyright (c) 2016-2019 Jupp Mueller |
| rapidjson | Copyright (c) 2015 THL A29 Limited (Tencent) and Milo Yip |
| thread-pool-cpp | Copyright (c) 2017 Andrey Kubarkov |
| tinygltf | Copyright (c) 2017 Syoyo Fujita, Aurélien Chatelain, and contributors |
| uvw | Copyright (c) 2016-2021 Michele Caini |
| wil (Windows Implementation Library) | Copyright (c) Microsoft Corporation |
| xenium | Copyright (c) 2018 Manuel Pöter |
| lua (LuaGLM fork, github.com/citizenfx/lua) | Copyright (c) 1994-2025 Lua.org, PUC-Rio |

### [Apache License 2.0](LICENSES/Apache-2.0.txt)

| Component | Copyright |
|-----------|-----------|
| ben-demystifier | Copyright (c) Ben Adams and contributors |
| cpu_features | Copyright (c) Google LLC |
| folly | Copyright (c) Meta Platforms, Inc. (Facebook, Inc.) |
| mbedtls | Copyright (c) Arm Limited |
| tbb / oneTBB | Copyright (c) Intel Corporation |
| uSockets | Copyright (c) Alex Hultman and contributors |
| uws / uWebSockets | Copyright (c) Alex Hultman and contributors |
| vulkan-headers | Copyright (c) The Khronos Group Inc. |
| rocksdb \* | Copyright (c) Meta Platforms, Inc. and affiliates |

\* **rocksdb** is dual-licensed Apache-2.0 / GPLv2 (plus a BSD-3 `LICENSE.leveldb`);
the **Apache-2.0 option is elected.**

### [BSD-3-Clause](LICENSES/BSD-3-Clause.txt)

| Component | Copyright |
|-----------|-----------|
| bgfx / bimg / bx | Copyright 2010-2020 Branimir Karadžić |
| breakpad | Copyright (c) 2006 Google Inc. |
| citizen_util (VTIL Project) | Copyright (c) 2020 Can Boluk and VTIL contributors |
| eabase / eastl | Copyright (c) Electronic Arts Inc. |
| jitasm | Copyright (c) 2009-2011 Hikaru Inoue, Akihiro Yamasaki |
| leveldb | Copyright (c) 2011 The LevelDB Authors |
| libfvad | Copyright (c) 2011 The WebRTC project authors |
| libnyquist | Copyright (c) 2010 The Cinder Project |
| libopus | Copyright 2001-2011 Xiph.Org, Skype Limited, Octasic, et al. |
| lss (linux-syscall-support) | Copyright (c) 2005-2011 Google Inc. |
| minhook | Copyright (c) 2009-2017 Tsuda Kageyu |
| protobuf | Copyright 2008 Google Inc. |
| replxx | Copyright (c) 2017-2018 Marcin Konarski |
| rnnoise | Copyright (c) 2017 Mozilla |
| speexdsp | Copyright 2002-2008 Xiph.Org Foundation |
| udis86 | Copyright (c) 2002-2012 Vivek Thampi |
| webrtc-audio-processing | Copyright (c) The WebRTC project authors |
| websocketpp | Copyright (c) 2014 Peter Thorson |

### [BSD-2-Clause](LICENSES/BSD-2-Clause.txt)

| Component | Copyright |
|-----------|-----------|
| labsound | BSD-2 / BSD-3; per-file copyrights: Google Inc., Apple Inc., Intel Corp. |
| lz4 (library) | Copyright (c) 2011-2020 Yann Collet; see [§3](#3-notes-on-specific-components) |
| zstd (library) | Copyright (c) 2016-present Facebook, Inc.; see [§3](#3-notes-on-specific-components) |

### [Boost Software License 1.0](LICENSES/BSL-1.0.txt)

| Component | Copyright |
|-----------|-----------|
| cpp-url | Copyright (c) Glyn Matthews and contributors |
| msgpack-c / msgpack-cpp | Copyright (c) 2008-2015 FURUHASHI Sadayuki |
| range-v3 | Copyright (c) 2009-2014 Eric Niebler and contributors |
| utfcpp | Copyright (c) Nemanja Trifunovic |

### [Zlib / libpng License](LICENSES/Zlib.txt)

| Component | Copyright |
|-----------|-----------|
| concurrentqueue | Copyright (c) 2013-2016 Cameron Desrochers |
| minizip | Copyright (c) 1998-2010 Gilles Vollant |
| tinyxml2 | Original code by Lee Thomason |
| toojpeg | Copyright (c) 2011-2016 Stephan Brumme |
| zlib | Copyright (c) 1995-2022 Jean-loup Gailly and Mark Adler |

### [ISC License](LICENSES/ISC.txt)

| Component | Copyright |
|-----------|-----------|
| openssl/boringssl | Copyright (c) The OpenSSL Project Authors / Google Inc. |

### [curl License](LICENSES/curl.txt)

| Component | Copyright |
|-----------|-----------|
| curl | Copyright (c) 1996-2022 Daniel Stenberg `<daniel@haxx.se>` and many contributors |

### [MIT / X11-style permission notice](LICENSES/MIT.txt)

| Component | Copyright |
|-----------|-----------|
| AntiLag2-SDK | Copyright (c) 2024 Advanced Micro Devices, Inc. |
| citizen_enet (Cfx fork of ENet) | Copyright (c) 2002-2020 Lee Salzman and contributors |
| discord-rpc | Copyright 2017 Discord, Inc. |
| im3d | Copyright (c) 2016-2020 John Chapman |
| node | Copyright (c) Node.js contributors |

### [The FreeType Project License (FTL)](LICENSES/FTL.txt)

| Component | Copyright |
|-----------|-----------|
| freetype \* | Copyright (c) The FreeType Project (www.freetype.org) |

\* **freetype** is dual-licensed FTL / GPLv2; **the FTL option is elected.** The FTL
requires crediting the FreeType Project in product documentation:
*"Portions of this software are copyright © The FreeType Project
(www.freetype.org). All rights reserved."*

### Public Domain / equivalent

| Component | Notes |
|-----------|-------|
| rpmalloc | Public domain / Unlicense (Mattias Jansson) |
| lmprof | Public domain / MIT-style (Lua profiler) |
| xz / liblzma | Public domain core (Lasse Collin, based on code by Igor Pavlov) |

---

## 3. Notes

- **lz4 / zstd**: each also ships a GPLv2 `COPYING` that applies **only to the
  command-line programs**, not the library. Only the BSD-licensed library
  (`lz4/lib/LICENSE`, `zstd/LICENSE`) is linked. See the
  [BSD-2-Clause](#bsd-2-clause) table above.

- **Prebuilt binary distributions**: **cef**, **cef32**, **chromium**, **v8**,
  and **node/libnode** are distributed here as prebuilt binaries under
  [BSD-3-Clause](LICENSES/BSD-3-Clause.txt) (and their own bundled third-party
  notices). They are not source components of this repository.

- **libuv**: `vendor/libuv/` here is a build-artifact placeholder, **not** libuv
  source. libuv itself ([MIT](LICENSES/MIT.txt), Copyright (c) libuv contributors)
  is obtained during the build; its notice belongs with the built distribution,
  not this directory.

---

## 4. Full license texts

The complete text of each distinct license used above is reproduced in the
[LICENSES/](LICENSES/) directory as canonical, verbatim SPDX-named files (as
published on the [SPDX License List](https://spdx.org/licenses/)).

| License | Reference text |
|---------|----------------|
| MIT | [LICENSES/MIT.txt](LICENSES/MIT.txt) |
| Apache-2.0 | [LICENSES/Apache-2.0.txt](LICENSES/Apache-2.0.txt) |
| BSD-2-Clause | [LICENSES/BSD-2-Clause.txt](LICENSES/BSD-2-Clause.txt) |
| BSD-3-Clause | [LICENSES/BSD-3-Clause.txt](LICENSES/BSD-3-Clause.txt) |
| ISC | [LICENSES/ISC.txt](LICENSES/ISC.txt) |
| Boost-1.0 | [LICENSES/BSL-1.0.txt](LICENSES/BSL-1.0.txt) |
| Zlib | [LICENSES/Zlib.txt](LICENSES/Zlib.txt) |
| curl | [LICENSES/curl.txt](LICENSES/curl.txt) |
| FTL | [LICENSES/FTL.txt](LICENSES/FTL.txt) |
| LGPL-2.1 | [LICENSES/LGPL-2.1.txt](LICENSES/LGPL-2.1.txt) |
