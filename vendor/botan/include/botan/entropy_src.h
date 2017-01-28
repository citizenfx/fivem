/*
* EntropySource
* (C) 2008,2009,2014,2015,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_ENTROPY_H__
#define BOTAN_ENTROPY_H__

#include <botan/secmem.h>
#include <botan/rng.h>
#include <string>
#include <chrono>
#include <memory>
#include <vector>

namespace Botan {

class RandomNumberGenerator;

/**
* Abstract interface to a source of entropy
*/
class BOTAN_DLL Entropy_Source
   {
   public:
      /*
      * Return a new entropy source of a particular type, or null
      * Each entropy source may require substantial resources (eg, a file handle
      * or socket instance), so try to share them among multiple RNGs, or just
      * use the preconfigured global list accessed by global_entropy_sources()
      */
      static std::unique_ptr<Entropy_Source> create(const std::string& type);

      /**
      * @return name identifying this entropy source
      */
      virtual std::string name() const = 0;

      /**
      * Perform an entropy gathering poll
      * @param rng will be provided with entropy via calls to add_entropy
      @ @return conservative estimate of actual entropy added to rng during poll
      */
      virtual size_t poll(RandomNumberGenerator& rng) = 0;

      virtual ~Entropy_Source() {}
   };

class BOTAN_DLL Entropy_Sources final
   {
   public:
      static Entropy_Sources& global_sources();

      void add_source(std::unique_ptr<Entropy_Source> src);

      std::vector<std::string> enabled_sources() const;

      size_t poll(RandomNumberGenerator& rng,
                  size_t bits,
                  std::chrono::milliseconds timeout);

      /**
      * Poll just a single named source. Ordinally only used for testing
      */
      size_t poll_just(RandomNumberGenerator& rng, const std::string& src);

      Entropy_Sources() {}
      explicit Entropy_Sources(const std::vector<std::string>& sources);

      ~Entropy_Sources();
   private:
      std::vector<Entropy_Source*> m_srcs;
   };

}

#endif
