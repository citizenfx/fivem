#pragma once

namespace net
{
	inline constexpr std::pair<uint32_t, const char*> PacketNames[]
	{
		// comment above each element is the hash of the packet
		// 0976e783
		{ HashRageString("msgArrayUpdate"), "msgArrayUpdate" },
		// 6acbd583
		{ HashRageString("msgConVars"), "msgConVars" },
		// ba96192a
		{ HashRageString("msgConfirm"), "msgConfirm" },
		// ca569e63
		{ HashRageString("msgEnd"), "msgEnd" },
		// 0f216a2a
		{ HashRageString("msgEntityCreate"), "msgEntityCreate" },
		// 53fffa3f
		{ HashRageString("msgFrame"), "msgFrame" },
		// 86e9f87b
		{ HashRageString("msgHeHost"), "msgHeHost" },
		// b3ea30de
		{ HashRageString("msgIHost"), "msgIHost" },
		// 522cadd1
		{ HashRageString("msgIQuit"), "msgIQuit" },
		// 7337fd7a
		{ HashRageString("msgNetEvent"), "msgNetEvent" },
		// 100d66a8
		{ HashRageString("msgNetGameEvent"), "msgNetGameEvent" },
		// 48e39581j
		{ HashRageString("msgObjectIds"), "msgObjectIds" },
		// 258dfdb4
		{ HashRageString("msgPackedAcks"), "msgPackedAcks" },
		// 81e1c835
		{ HashRageString("msgPackedClones"), "msgPackedClones" },
		// 073b065b
		{ HashRageString("msgPaymentRequest"), "msgPaymentRequest" },
		// b8e611cf
		{ HashRageString("msgRequestObjectIds"), "msgRequestObjectIds" },
		// afe4cd4a
		{ HashRageString("msgResStart"), "msgResStart" },
		// 45e855d7
		{ HashRageString("msgResStop"), "msgResStop" },
		// e938445b
		{ HashRageString("msgRoute"), "msgRoute" },
		// 211cab17
		{ HashRageString("msgRpcNative"), "msgRpcNative" },
		// b18d4fc4
		{ HashRageString("msgServerCommand"), "msgServerCommand" },
		// fa776e18
		{ HashRageString("msgServerEvent"), "msgServerEvent" },
		// de3d1a59
		{ HashRageString("msgStateBag"), "msgStateBag" },
		// e56e37ed
		{ HashRageString("msgTimeSync"), "msgTimeSync" },
		// 1c1303f8
		{ HashRageString("msgTimeSyncReq"), "msgTimeSyncReq" },
		// 852c1561
		{ HashRageString("msgWorldGrid3"), "msgWorldGrid3" },
		// a5d4e2bc
		{ HashRageString("gameStateAck"), "gameStateAck" },
		// d2f86a6e
		{ HashRageString("gameStateNAck"), "gameStateNAck" },
	};

}


