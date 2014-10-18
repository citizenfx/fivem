/*
* SSL/TLS Protocol Constants
* (C) 2004-2010 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_PROTOCOL_MAGIC_H__
#define BOTAN_TLS_PROTOCOL_MAGIC_H__

namespace Botan {

namespace TLS {

/**
* Protocol Constants for SSL/TLS
*/
enum Size_Limits {
   TLS_HEADER_SIZE    = 5,
   DTLS_HEADER_SIZE   = TLS_HEADER_SIZE + 8,

   MAX_PLAINTEXT_SIZE = 16*1024,
   MAX_COMPRESSED_SIZE = MAX_PLAINTEXT_SIZE + 1024,
   MAX_CIPHERTEXT_SIZE = MAX_COMPRESSED_SIZE + 1024,
};

enum Connection_Side { CLIENT = 1, SERVER = 2 };

enum Record_Type {
   NO_RECORD          = 0,

   CHANGE_CIPHER_SPEC = 20,
   ALERT              = 21,
   HANDSHAKE          = 22,
   APPLICATION_DATA   = 23,
   HEARTBEAT          = 24,
};

enum Handshake_Type {
   HELLO_REQUEST        = 0,
   CLIENT_HELLO         = 1,
   CLIENT_HELLO_SSLV2   = 253, // Not a wire value
   SERVER_HELLO         = 2,
   HELLO_VERIFY_REQUEST = 3,
   NEW_SESSION_TICKET   = 4, // RFC 5077
   CERTIFICATE          = 11,
   SERVER_KEX           = 12,
   CERTIFICATE_REQUEST  = 13,
   SERVER_HELLO_DONE    = 14,
   CERTIFICATE_VERIFY   = 15,
   CLIENT_KEX           = 16,
   FINISHED             = 20,

   CERTIFICATE_URL      = 21,
   CERTIFICATE_STATUS   = 22,

   NEXT_PROTOCOL        = 67,

   HANDSHAKE_CCS        = 254, // Not a wire value
   HANDSHAKE_NONE       = 255  // Null value
};

enum Compression_Method {
   NO_COMPRESSION       = 0x00,
   DEFLATE_COMPRESSION  = 0x01
};

}

}

#endif
