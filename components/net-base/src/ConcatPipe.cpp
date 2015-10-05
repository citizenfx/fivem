/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ConcatPipe.h"

namespace net
{
ConcatInputPipe::ConcatInputPipe(const fwRefContainer<NetPipe>& p1, const fwRefContainer<NetPipe>& p2)
	: m_pipe1(p1), m_pipe2(p2)
{

}

void ConcatInputPipe::Reset()
{
	m_pipe1->Reset();
	m_pipe2->Reset();
}

void ConcatInputPipe::PassPacket(Buffer data)
{
	// get the length of the first element
	uint16_t firstLength = data.Read<uint16_t>();
	
	// verify length
	if (data.GetRemainingBytes() >= firstLength)
	{
		// create arrays for both first and second
		Buffer firstData(firstLength);
		Buffer secondData(data.GetRemainingBytes() - firstLength);

		// write data to both arrays
		data.ReadTo(firstData, firstData.GetLength());
		data.ReadTo(secondData, secondData.GetLength());

		// pass to both pipes
		m_pipe1->PassPacket(firstData);
		m_pipe2->PassPacket(secondData);
	}
}

ConcatOutputPipe::ConcatOutputPipe(const fwRefContainer<NetPipe>& p)
	: m_pipe(p), m_tickTock(false)
{

}

void ConcatOutputPipe::Reset()
{
	m_tickTock = false;

	m_pipe->Reset();
}

void ConcatOutputPipe::PassPacket(Buffer data)
{
	if (!m_tickTock)
	{
		m_savedFirst = data;
	}
	else
	{
		Buffer outPacket(m_savedFirst.GetLength() + data.GetLength() + 2);

		outPacket.Write<uint16_t>(m_savedFirst.GetLength());

		m_savedFirst.ReadTo(outPacket, m_savedFirst.GetLength());
		data.ReadTo(outPacket, data.GetLength());
		
		m_pipe->PassPacket(outPacket);
	}

	m_tickTock = !m_tickTock;
}

class DummyOutputPipe : public NetPipe
{
private:
	Buffer m_packet;

public:
	void Reset() override
	{

	}

	void PassPacket(Buffer data) override
	{
		m_packet = data;
	}

	inline Buffer& GetPacket()
	{
		return m_packet;
	}
};
}

using namespace net;

#include <NetUdpSocket.h>

static InitFunction initFunction([] ()
{
	/*
	PeerAddress addr1 = PeerAddress::FromString("127.0.0.1:0").get();
	PeerAddress addr2 = PeerAddress::FromString("localhost:0").get();

	fwRefContainer<UdpSocket> udpSocket1 = new UdpSocket(AddressFamily::IPv4);
	udpSocket1->Bind(addr1);

	fwRefContainer<UdpSocket> udpSocket2 = new UdpSocket(AddressFamily::IPv4);
	udpSocket2->Bind(addr2);

	Buffer data1;
	data1.Write(0xDEADBDEF);

	Buffer data2;
	data2.Write(0xDEADBEEF);

	fwRefContainer<DummyOutputPipe> toSendPipe = new DummyOutputPipe();

	fwRefContainer<ConcatOutputPipe> outPipe = new ConcatOutputPipe(toSendPipe);

	outPipe->Reset();
	outPipe->PassPacket(data1);
	outPipe->PassPacket(data2);

	udpSocket1->SendTo(toSendPipe->GetPacket().GetData(), udpSocket2->GetLocalAddress());

	PeerAddress inAddress;
	Buffer recvBit = udpSocket2->ReceiveFrom(2048, &inAddress).get();

	fwRefContainer<DummyOutputPipe> splitOutputPipe1 = new DummyOutputPipe();
	fwRefContainer<DummyOutputPipe> splitOutputPipe2 = new DummyOutputPipe();

	fwRefContainer<ConcatInputPipe> splitPipe = new ConcatInputPipe(splitOutputPipe1, splitOutputPipe2);

	splitPipe->PassPacket(recvBit);

	Buffer outPacket = splitOutputPipe2->GetPacket();

	uint32_t bit = outPacket.Read<uint32_t>();

	auto localAddress = udpSocket2->GetLocalAddress();

	trace("bit: %x\n", bit);
	trace("local addr: %s\n", localAddress.ToString().c_str());
	*/

	//__debugbreak();
});