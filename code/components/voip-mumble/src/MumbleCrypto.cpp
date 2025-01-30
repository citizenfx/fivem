#include "StdInc.h"
#include "MumbleClientImpl.h"
#include "MumbleMessageHandler.h"

// we can't use the Botan::OCB_Mode class since this expects full tags, whereas Mumble uses 'truncated' 3-byte tags
//#include <botan/aead.h>
#include <botan/block_cipher.h>

#include <array>


void MumbleCrypto::Encrypt(const uint8_t* plain, uint8_t* cipher, size_t length)
{
	// increase the IV
	for (size_t i = 0; i < m_clientNonce.size(); i++)
	{
		if (++m_clientNonce[i])
		{
			break;
		}
	}

	// encrypt
	uint8_t tag[16];
	OCBEncrypt(plain, cipher + 4, length, m_clientNonce.data(), tag);

	cipher[0] = m_clientNonce[0];
	cipher[1] = tag[0];
	cipher[2] = tag[1];
	cipher[3] = tag[2];
}

bool MumbleCrypto::Decrypt(const uint8_t* cipher, uint8_t* plain, size_t length)
{
	if (length < 4)
	{
		console::DPrintf("mumble", "Tried to decrypt packet but it didn't have headers\n");
		return false;
	}

	unsigned int plain_length = length - 4;

	unsigned char saveiv[16];
	unsigned char ivbyte = cipher[0];
	bool restore = false;
	unsigned char tag[16];

	int lost = 0;
	int late = 0;

	memcpy(saveiv, m_serverNonce.data(), 16);

	if (((m_serverNonce[0] + 1) & 0xFF) == ivbyte)
	{
		// In order as expected.
		if (ivbyte > m_serverNonce[0])
		{
			m_serverNonce[0] = ivbyte;
		}
		else if (ivbyte < m_serverNonce[0])
		{
			int i;
			m_serverNonce[0] = ivbyte;
			for (i = 1; i < 16; i++)
				if (++m_serverNonce[i])
					break;
		}
		else
		{
			return false;
		}
	}
	else
	{
		// This is either out of order or a repeat.

		int diff = ivbyte - m_serverNonce[0];
		if (diff > 128)
			diff = diff - 256;
		else if (diff < -128)
			diff = diff + 256;

		if ((ivbyte < m_serverNonce[0]) && (diff > -30) && (diff < 0))
		{
			// Late packet, but no wraparound.
			late = 1;
			lost = -1;
			m_serverNonce[0] = ivbyte;
			restore = true;
		}
		else if ((ivbyte > m_serverNonce[0]) && (diff > -30) && (diff < 0))
		{
			int i;
			// Last was 0x02, here comes 0xff from last round
			late = 1;
			lost = -1;
			m_serverNonce[0] = ivbyte;
			for (i = 1; i < 16; i++)
				if (m_serverNonce[i]--)
					break;
			restore = true;
		}
		else if ((ivbyte > m_serverNonce[0]) && (diff > 0))
		{
			// Lost a few packets, but beyond that we're good.
			lost = ivbyte - m_serverNonce[0] - 1;
			m_serverNonce[0] = ivbyte;
		}
		else if ((ivbyte < m_serverNonce[0]) && (diff > 0))
		{
			int i;
			// Lost a few packets, and wrapped around
			lost = 256 - m_serverNonce[0] + ivbyte - 1;
			m_serverNonce[0] = ivbyte;
			for (i = 1; i < 16; i++)
				if (++m_serverNonce[i])
					break;
		}
		else
		{
			return false;
		}

		if (m_decryptHistory[m_serverNonce[0]] == m_serverNonce[1])
		{
			memcpy(m_serverNonce.data(), saveiv, 16);
			return false;
		}
	}

	OCBDecrypt(cipher + 4, plain, plain_length, m_serverNonce.data(), tag);

	if (memcmp(tag, cipher + 1, 3) != 0)
	{
		memcpy(m_serverNonce.data(), saveiv, 16);
		return false;
	}

	m_decryptHistory[m_serverNonce[0]] = m_serverNonce[1];

	if (restore)
		memcpy(m_serverNonce.data(), saveiv, 16);

	m_localGood++;
	m_localLate += late;
	m_localLost += lost;

	return true;
}

std::string MumbleCrypto::GetClientNonce()
{
	return std::string{ m_clientNonce.begin(), m_clientNonce.end() };
}


bool MumbleCrypto::SetServerNonce(const std::string& serverNonce)
{
	if (serverNonce.length() != AES_BLOCK_SIZE) { return false; }
	memcpy(m_serverNonce.data(), serverNonce.c_str(), AES_BLOCK_SIZE);
	return true;
}

bool MumbleCrypto::SetKey(const std::string& key, const std::string& clientNonce, const std::string& serverNonce)
{
	if (key.length() != AES_BLOCK_SIZE || clientNonce.length() != AES_BLOCK_SIZE)
	{
		return false;
	}

	memcpy(m_key.data(), key.c_str(), AES_BLOCK_SIZE);
	memcpy(m_clientNonce.data(), clientNonce.c_str(), AES_BLOCK_SIZE);
	if (!SetServerNonce(serverNonce))
	{
		return false;
	}
	const Botan::SymmetricKey keyData(m_key.data(), m_key.size());

	m_cipher = Botan::BlockCipher::create("AES-128");
	m_cipher->set_key(keyData);
	m_init = true;
	return true;
}

// if crypto has been initialized
bool MumbleCrypto::IsInitialized() const
{
	return m_init;
}

DEFINE_HANDLER(CryptSetup)
{
	auto client = MumbleClient::GetCurrent();

	if (data.has_key() && data.has_client_nonce() && data.has_server_nonce())
	{
		const auto& key = data.key();
		const auto& clientNonce = data.client_nonce();
		const auto& serverNonce = data.server_nonce();
		
		if (!client->m_crypto.SetKey(key, clientNonce, serverNonce))
		{
			console::PrintError("mumble", "Server sent invalid data for mumble crypto setup.\n");
		}
	}
	else if (data.has_server_nonce())
	{
		const auto& serverNonce = data.server_nonce();
		if (client->m_crypto.SetServerNonce(serverNonce))
		{
			client->m_crypto.m_localResync++;
		}
		else
		{
			console::PrintWarning("mumble", "Server sent an invalid size server nonce.\n");
		}
	}
	else 
	{
		MumbleProto::CryptSetup cryptSetup;
		cryptSetup.set_client_nonce(client->m_crypto.GetClientNonce());
		client->Send(MumbleMessageType::CryptSetup, cryptSetup);
	}
});

#include <stdint.h>

#ifdef _WIN32
#define SWAPPED(x) _byteswap_ulong(x)
#else
#define SWAPPED(x) htonl(x)
#endif

#define BLOCKSIZE 4
#define SHIFTBITS 31
#define AES_BLOCK_SIZE 16
typedef uint32_t subblock;

#define HIGHBIT (1<<SHIFTBITS);

static void inline XOR(subblock *dst, const subblock *a, const subblock *b) {
	int i;
	for (i = 0; i < BLOCKSIZE; i++) {
		dst[i] = a[i] ^ b[i];
	}
}

static void inline S2(subblock *block) {
	subblock carry = SWAPPED(block[0]) >> SHIFTBITS;
	int i;
	for (i = 0; i < BLOCKSIZE - 1; i++)
		block[i] = SWAPPED((SWAPPED(block[i]) << 1) | (SWAPPED(block[i + 1]) >> SHIFTBITS));
	block[BLOCKSIZE - 1] = SWAPPED((SWAPPED(block[BLOCKSIZE - 1]) << 1) ^ (carry * 0x87));
}

static void inline S3(subblock *block) {
	subblock carry = SWAPPED(block[0]) >> SHIFTBITS;
	int i;
	for (i = 0; i < BLOCKSIZE - 1; i++)
		block[i] ^= SWAPPED((SWAPPED(block[i]) << 1) | (SWAPPED(block[i + 1]) >> SHIFTBITS));
	block[BLOCKSIZE - 1] ^= SWAPPED((SWAPPED(block[BLOCKSIZE - 1]) << 1) ^ (carry * 0x87));
}

static void inline ZERO(subblock *block) {
	int i;
	for (i = 0; i < BLOCKSIZE; i++)
		block[i] = 0;
}

void MumbleCrypto::OCBEncrypt(const unsigned char *plain, unsigned char *encrypted, unsigned int len, const unsigned char *nonce, unsigned char *tag)
{
	subblock checksum[BLOCKSIZE], delta[BLOCKSIZE], tmp[BLOCKSIZE], pad[BLOCKSIZE];

	// Initialize
	//CRYPT_AES_ENCRYPT(nonce, delta, cs);
	m_cipher->encrypt((uint8_t*)nonce, (uint8_t*)delta);
	ZERO(checksum);

	while (len > AES_BLOCK_SIZE) {
		S2(delta);
		XOR(tmp, delta, (const subblock *)(plain));
		m_cipher->encrypt((uint8_t*)tmp);
		XOR((subblock *)(encrypted), delta, tmp);
		XOR(checksum, checksum, (subblock *)(plain));
		len -= AES_BLOCK_SIZE;
		plain += AES_BLOCK_SIZE;
		encrypted += AES_BLOCK_SIZE;
	}

	S2(delta);
	ZERO(tmp);
	tmp[BLOCKSIZE - 1] = SWAPPED(len * 8);
	XOR(tmp, tmp, delta);
	m_cipher->encrypt((uint8_t*)tmp, (uint8_t*)pad);
	memcpy(tmp, plain, len);
	memcpy((unsigned char *)tmp + len, (unsigned char *)pad + len, AES_BLOCK_SIZE - len);
	XOR(checksum, checksum, tmp);
	XOR(tmp, pad, tmp);
	memcpy(encrypted, tmp, len);

	S3(delta);
	XOR(tmp, delta, checksum);
	m_cipher->encrypt((uint8_t*)tmp, (uint8_t*)tag);
}

void MumbleCrypto::OCBDecrypt(const unsigned char *encrypted, unsigned char *plain, unsigned int len, const unsigned char *nonce, unsigned char *tag) {
	subblock checksum[BLOCKSIZE], delta[BLOCKSIZE], tmp[BLOCKSIZE], pad[BLOCKSIZE];
	// Initialize
	m_cipher->encrypt((uint8_t*)nonce, (uint8_t*)delta);
	ZERO(checksum);

	while (len > AES_BLOCK_SIZE) {
		S2(delta);
		XOR(tmp, delta, (const subblock *)(encrypted));
		m_cipher->decrypt((uint8_t*)tmp);
		XOR((subblock *)(plain), delta, tmp);
		XOR(checksum, checksum, (const subblock *)(plain));
		len -= AES_BLOCK_SIZE;
		plain += AES_BLOCK_SIZE;
		encrypted += AES_BLOCK_SIZE;
	}

	S2(delta);
	ZERO(tmp);
	tmp[BLOCKSIZE - 1] = SWAPPED(len * 8);
	XOR(tmp, tmp, delta);
	m_cipher->encrypt((uint8_t*)tmp, (uint8_t*)pad);
	memset(tmp, 0, AES_BLOCK_SIZE);
	memcpy(tmp, encrypted, len);
	XOR(tmp, tmp, pad);
	XOR(checksum, checksum, tmp);
	memcpy(plain, tmp, len);

	S3(delta);
	XOR(tmp, delta, checksum);
	m_cipher->encrypt((uint8_t*)tmp, (uint8_t*)tag);
}

