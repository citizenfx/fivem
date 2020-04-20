#pragma once

inline int evutil_socketpair(int, int, int, intptr_t*) { return -1; }