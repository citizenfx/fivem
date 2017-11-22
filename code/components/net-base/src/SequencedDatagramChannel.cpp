/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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