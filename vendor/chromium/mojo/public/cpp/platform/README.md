# Mojo C++ Platform API
This document is a subset of the [Mojo documentation](/mojo/README.md).

[TOC]

## Overview
The Mojo C++ Platform API provides a lightweight set of abstractions around
stable platform primitive APIs like UNIX domain sockets and Windows named pipes.
This API is primarily useful in conjunction with Mojo
[Invitations](/mojo/public/cpp/system/README.md#Invitations) to bootstrap Mojo
IPC between two processes.

## Platform Handles
The `PlatformHandle` type provides a move-only wrapper around an owned,
platform-specific primitive handle types. The type of primitive it holds can be
any of the following:

  * Windows HANDLE (Windows only)
  * Fuchsia zx_handle_t (Fuchsia only)
  * Mach send right (OSX only)
  * POSIX file descriptor (POSIX systems only)

See the
[header](https://cs.chromium.org/src/mojo/public/cpp/platform/platform_handle.h)
for more details.

## Platform Channels
The `PlatformChannel` type abstracts a platform-specific IPC FIFO primitive
primarily for use with the Mojo
[Invitations](/mojo/public/cpp/system/README.md#Invitations) API. Constructing
a `PlatformChannel` instance creates the underlying system primitive with two
transferrable `PlatformHandle` instances, each thinly wrapped as a
`PlatformChannelEndpoint` for additional type-safety. One endpoint is designated
as **local** and the other **remote**, the intention being that the remote
endpoint will be transferred to another process in the system.

See the
[header](https://cs.chromium.org/src/mojo/public/cpp/platform/platform_channel.h)
for more details. See the
[Invitations](/mojo/public/cpp/system/README.md#Invitations) documentation for
an example of using `PlatformChannel` with an invitation to bootstrap IPC
between a process and one of its newly launched child processes.

## Named Platform Channels
For cases where it is not feasible to transfer a `PlatformHandle` from one
running process to another, the Platform API also provides
`NamedPlatformChannel`, which abstracts a named system resource that can
facilitate communication similarly to `PlatformChannel`.

A `NamedPlatformChannel` upon construction will begin listening on a
platform-specific primitive (a named pipe server on Windows, a domain socket
server on POSIX, *etc.*). The globally reachable name of the server (*e.g.* the
socket path) can be specified at construction time via
`NamedPlatformChannel::Options::server_name`, but if no name is given, a
suitably random one is generated and used.

``` cpp
// In one process
mojo::NamedPlatformChannel::Options options;
mojo::NamedPlatformChannel named_channel(options);
OutgoingInvitation::Send(std::move(invitation),
                         named_channel.TakeServerEndpoint());
SendServerNameToRemoteProcessSomehow(named_channel.GetServerName());

// In the other process
void OnGotServerName(const mojo::NamedPlatformChannel::ServerName& name) {
  // Connect to the server.
  mojo::PlatformChannelEndpoint endpoint =
      mojo::NamedPlatformChannel::ConnectToServer(name);

  // Proceed normally with invitation acceptance.
  auto invitation = mojo::IncomingInvitation::Accept(std::move(endpoint));
  // ...
}
```
