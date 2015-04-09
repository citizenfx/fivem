/*
* PBKDF
* (C) 1999-2007,2012,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PBKDF_H__
#define BOTAN_PBKDF_H__

#include <botan/symkey.h>
#include <botan/scan_name.h>
#include <chrono>

namespace Botan {

/**
* Base class for PBKDF (password based key derivation function)
* implementations. Converts a password into a key using a salt
* and iterated hashing to make brute force attacks harder.
*/
class BOTAN_DLL PBKDF
   {
   public:

      virtual ~PBKDF() {}

      typedef SCAN_Name Spec;

      /**
      * @return new instance of this same algorithm
      */
      virtual PBKDF* clone() const = 0;

      virtual std::string name() const = 0;

      /**
      * Derive a key from a passphrase for a number of iterations
      * specified by either iterations or if iterations == 0 then
      * running until seconds time has elapsed.
      *
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      * @param msec if iterations is zero, then instead the PBKDF is
      *        run until msec milliseconds has passed.
      * @return the number of iterations performed
      */
      virtual size_t pbkdf(byte out[], size_t out_len,
                           const std::string& passphrase,
                           const byte salt[], size_t salt_len,
                           size_t iterations,
                           std::chrono::milliseconds msec) const = 0;

      void pbkdf_iterations(byte out[], size_t out_len,
                            const std::string& passphrase,
                            const byte salt[], size_t salt_len,
                            size_t iterations) const;

      void pbkdf_timed(byte out[], size_t out_len,
                         const std::string& passphrase,
                         const byte salt[], size_t salt_len,
                         std::chrono::milliseconds msec,
                         size_t& iterations) const;

      secure_vector<byte> pbkdf_iterations(size_t out_len,
                                           const std::string& passphrase,
                                           const byte salt[], size_t salt_len,
                                           size_t iterations) const;

      secure_vector<byte> pbkdf_timed(size_t out_len,
                                      const std::string& passphrase,
                                      const byte salt[], size_t salt_len,
                                      std::chrono::milliseconds msec,
                                      size_t& iterations) const;

      // Following kept for compat with 1.10:

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      */
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const byte salt[], size_t salt_len,
                             size_t iterations) const
         {
         return pbkdf_iterations(out_len, passphrase, salt, salt_len, iterations);
         }

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param iterations the number of iterations to use (use 10K or more)
      */
      template<typename Alloc>
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const std::vector<byte, Alloc>& salt,
                             size_t iterations) const
         {
         return pbkdf_iterations(out_len, passphrase, &salt[0], salt.size(), iterations);
         }

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param msec is how long to run the PBKDF
      * @param iterations is set to the number of iterations used
      */
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const byte salt[], size_t salt_len,
                             std::chrono::milliseconds msec,
                             size_t& iterations) const
         {
         return pbkdf_timed(out_len, passphrase, salt, salt_len, msec, iterations);
         }

      /**
      * Derive a key from a passphrase using a certain amount of time
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param msec is how long to run the PBKDF
      * @param iterations is set to the number of iterations used
      */
      template<typename Alloc>
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const std::vector<byte, Alloc>& salt,
                             std::chrono::milliseconds msec,
                             size_t& iterations) const
         {
         return pbkdf_timed(out_len, passphrase, &salt[0], salt.size(), msec, iterations);
         }
   };

}

#endif
