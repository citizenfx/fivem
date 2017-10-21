/*
* Exceptions
* (C) 1999-2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_EXCEPTION_H_
#define BOTAN_EXCEPTION_H_

#include <botan/types.h>
#include <exception>
#include <string>

namespace Botan {

/**
* Base class for all exceptions thrown by the library
*/
class BOTAN_PUBLIC_API(2,0) Exception : public std::exception
   {
   public:
      explicit Exception(const std::string& msg) : m_msg(msg) {}
      Exception(const char* prefix, const std::string& msg) : m_msg(std::string(prefix) + " " + msg) {}
      const char* what() const BOTAN_NOEXCEPT override { return m_msg.c_str(); }
   private:
      std::string m_msg;
   };

/**
* An invalid argument
*/
class BOTAN_PUBLIC_API(2,0) Invalid_Argument : public Exception
   {
   public:
      explicit Invalid_Argument(const std::string& msg) :
         Exception("Invalid argument", msg) {}

      explicit Invalid_Argument(const std::string& msg, const std::string& where) :
         Exception("Invalid argument", msg + " in " + where) {}
};

#define BOTAN_ARG_CHECK(expr) \
   do { if(!(expr)) throw Invalid_Argument(#expr, BOTAN_CURRENT_FUNCTION); } while(0)

/**
* Unsupported_Argument Exception
*
* An argument that is invalid because it is not supported by Botan.
* It might or might not be valid in another context like a standard.
*/
class BOTAN_PUBLIC_API(2,0) Unsupported_Argument final : public Invalid_Argument
   {
   public:
      explicit Unsupported_Argument(const std::string& msg) : Invalid_Argument(msg) {}
   };

/**
* Invalid_State Exception
*/
class BOTAN_PUBLIC_API(2,0) Invalid_State : public Exception
   {
   public:
      explicit Invalid_State(const std::string& err) : Exception(err) {}
   };

/**
* Lookup_Error Exception
*/
class BOTAN_PUBLIC_API(2,0) Lookup_Error : public Exception
   {
   public:
      explicit Lookup_Error(const std::string& err) : Exception(err) {}

      Lookup_Error(const std::string& type,
                   const std::string& algo,
                   const std::string& provider) :
         Exception("Unavailable " + type + " " + algo +
                   (provider.empty() ? std::string("") : (" for provider " + provider)))
         {}
   };

/**
* Internal_Error Exception
*/
class BOTAN_PUBLIC_API(2,0) Internal_Error : public Exception
   {
   public:
      explicit Internal_Error(const std::string& err) :
         Exception("Internal error: " + err)
         {}
   };

/**
* Invalid_Key_Length Exception
*/
class BOTAN_PUBLIC_API(2,0) Invalid_Key_Length final : public Invalid_Argument
   {
   public:
      Invalid_Key_Length(const std::string& name, size_t length) :
         Invalid_Argument(name + " cannot accept a key of length " +
                          std::to_string(length))
         {}
   };

/**
* Invalid_IV_Length Exception
*/
class BOTAN_PUBLIC_API(2,0) Invalid_IV_Length final : public Invalid_Argument
   {
   public:
      Invalid_IV_Length(const std::string& mode, size_t bad_len) :
         Invalid_Argument("IV length " + std::to_string(bad_len) +
                          " is invalid for " + mode)
         {}
   };

/**
* PRNG_Unseeded Exception
*/
class BOTAN_PUBLIC_API(2,0) PRNG_Unseeded final : public Invalid_State
   {
   public:
      explicit PRNG_Unseeded(const std::string& algo) :
         Invalid_State("PRNG not seeded: " + algo)
         {}
   };

/**
* Policy_Violation Exception
*/
class BOTAN_PUBLIC_API(2,0) Policy_Violation final : public Invalid_State
   {
   public:
      explicit Policy_Violation(const std::string& err) :
         Invalid_State("Policy violation: " + err)
         {}
   };

/**
* Algorithm_Not_Found Exception
*/
class BOTAN_PUBLIC_API(2,0) Algorithm_Not_Found final : public Lookup_Error
   {
   public:
      explicit Algorithm_Not_Found(const std::string& name) :
         Lookup_Error("Could not find any algorithm named \"" + name + "\"")
         {}
   };

/**
* No_Provider_Found Exception
*/
class BOTAN_PUBLIC_API(2,0) No_Provider_Found final : public Exception
   {
   public:
      explicit No_Provider_Found(const std::string& name) :
         Exception("Could not find any provider for algorithm named \"" + name + "\"")
         {}
   };

/**
* Provider_Not_Found is thrown when a specific provider was requested
* but that provider is not available.
*/
class BOTAN_PUBLIC_API(2,0) Provider_Not_Found final : public Lookup_Error
   {
   public:
      Provider_Not_Found(const std::string& algo, const std::string& provider) :
         Lookup_Error("Could not find provider '" + provider + "' for " + algo) {}
   };

/**
* Invalid_Algorithm_Name Exception
*/
class BOTAN_PUBLIC_API(2,0) Invalid_Algorithm_Name final : public Invalid_Argument
   {
   public:
      explicit Invalid_Algorithm_Name(const std::string& name):
         Invalid_Argument("Invalid algorithm name: " + name)
         {}
   };

/**
* Encoding_Error Exception
*/
class BOTAN_PUBLIC_API(2,0) Encoding_Error final : public Invalid_Argument
   {
   public:
      explicit Encoding_Error(const std::string& name) :
         Invalid_Argument("Encoding error: " + name) {}
   };

/**
* Decoding_Error Exception
*/
class BOTAN_PUBLIC_API(2,0) Decoding_Error : public Invalid_Argument
   {
   public:
      explicit Decoding_Error(const std::string& name) :
         Invalid_Argument("Decoding error: " + name) {}
   };

/**
* Integrity_Failure Exception
*/
class BOTAN_PUBLIC_API(2,0) Integrity_Failure final : public Exception
   {
   public:
      explicit Integrity_Failure(const std::string& msg) :
         Exception("Integrity failure: " + msg) {}
   };

/**
* Invalid_OID Exception
*/
class BOTAN_PUBLIC_API(2,0) Invalid_OID final : public Decoding_Error
   {
   public:
      explicit Invalid_OID(const std::string& oid) :
         Decoding_Error("Invalid ASN.1 OID: " + oid) {}
   };

/**
* Stream_IO_Error Exception
*/
class BOTAN_PUBLIC_API(2,0) Stream_IO_Error final : public Exception
   {
   public:
      explicit Stream_IO_Error(const std::string& err) :
         Exception("I/O error: " + err)
         {}
   };

/**
* No_Filesystem_Access Exception
*/
class BOTAN_PUBLIC_API(2,0) No_Filesystem_Access final : public Exception
   {
   public:
      No_Filesystem_Access() : Exception("No filesystem access enabled.") {}
   };

/**
* Self Test Failure Exception
*/
class BOTAN_PUBLIC_API(2,0) Self_Test_Failure final : public Internal_Error
   {
   public:
      explicit Self_Test_Failure(const std::string& err) :
         Internal_Error("Self test failed: " + err)
         {}
   };

/**
* Not Implemented Exception
*/
class BOTAN_PUBLIC_API(2,0) Not_Implemented final : public Exception
   {
   public:
      explicit Not_Implemented(const std::string& err) :
         Exception("Not implemented", err)
         {}
   };

}

#endif
