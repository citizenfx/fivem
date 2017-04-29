#pragma once

#define HASH_LENGTH 20
#define BLOCK_LENGTH 64

typedef struct sha1nfo
{
	uint32_t buffer[BLOCK_LENGTH / 4];
	uint32_t state[HASH_LENGTH / 4];
	uint32_t byteCount;
	uint8_t bufferOffset;
	uint8_t keyBuffer[BLOCK_LENGTH];
	uint8_t innerHash[HASH_LENGTH];
} sha1nfo;

/* public API - prototypes - TODO: doxygen*/

/**
*/
void sha1_init(sha1nfo *s);
/**
*/
void sha1_writebyte(sha1nfo *s, uint8_t data);
/**
*/
void sha1_write(sha1nfo *s, const char *data, size_t len);
/**
*/
uint8_t* sha1_result(sha1nfo *s);
/**
*/
void sha1_initHmac(sha1nfo *s, const uint8_t* key, int keyLength);
/**
*/
uint8_t* sha1_resultHmac(sha1nfo *s);