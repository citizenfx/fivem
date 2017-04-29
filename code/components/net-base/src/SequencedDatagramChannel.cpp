/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "SequencedDatagramChannel.h"

namespace net
{
SequencedDatagramChannel::SequencedDatagramChannel()
{
	m_sequence = 0;
}

SequencedDatagramChannel::~SequencedDatagramChannel()
{

}
}