#pragma once

#define PAIRED_BUILD 4

static_assert(PAIRED_BUILD > 0, "a zero build would let an unpatched peer through the check");
