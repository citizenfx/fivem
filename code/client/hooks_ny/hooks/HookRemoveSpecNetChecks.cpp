// removes specification-bound checks from IV networking

#include "StdInc.h"

static HookFunction hookFunction([] ()
{
	// function which maintains counters related to MP_WARNING_5/MP_WARNING_6 messages; which are 'too slowly' and 'run out of streaming memory' disconnects
	hook::return_function(0x460310);
});