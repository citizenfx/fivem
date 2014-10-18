/*
* An algorithm cache (used by Algorithm_Factory)
* (C) 2008-2009,2011 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_ALGORITHM_CACHE_TEMPLATE_H__
#define BOTAN_ALGORITHM_CACHE_TEMPLATE_H__

#include <botan/types.h>
#include <botan/internal/stl_util.h>
#include <mutex>
#include <string>
#include <vector>
#include <map>

namespace Botan {

/**
* @param prov_name a provider name
* @return weight for this provider
*/
size_t static_provider_weight(const std::string& prov_name);

/**
* Algorithm_Cache (used by Algorithm_Factory)
*/
template<typename T>
class Algorithm_Cache
   {
   public:
      /**
      * @param algo_spec names the requested algorithm
      * @param pref_provider suggests a preferred provider
      * @return prototype object, or NULL
      */
      const T* get(const std::string& algo_spec,
                   const std::string& pref_provider);

      /**
      * Add a new algorithm implementation to the cache
      * @param algo the algorithm prototype object
      * @param requested_name how this name will be requested
      * @param provider_name is the name of the provider of this prototype
      */
      void add(T* algo,
               const std::string& requested_name,
               const std::string& provider_name);

      /**
      * Set the preferred provider
      * @param algo_spec names the algorithm
      * @param provider names the preferred provider
      */
      void set_preferred_provider(const std::string& algo_spec,
                                  const std::string& provider);

      /**
      * Return the list of providers of this algorithm
      * @param algo_name names the algorithm
      * @return list of providers of this algorithm
      */
      std::vector<std::string> providers_of(const std::string& algo_name);

      /**
      * Clear the cache
      */
      void clear_cache();

      ~Algorithm_Cache() { clear_cache(); }
   private:
      typename std::map<std::string, std::map<std::string, T*> >::const_iterator
         find_algorithm(const std::string& algo_spec);

      std::mutex mutex;
      std::map<std::string, std::string> aliases;
      std::map<std::string, std::string> pref_providers;
      std::map<std::string, std::map<std::string, T*> > algorithms;
   };

/*
* Look for an algorithm implementation in the cache, also checking aliases
* Assumes object lock is held
*/
template<typename T>
typename std::map<std::string, std::map<std::string, T*> >::const_iterator
Algorithm_Cache<T>::find_algorithm(const std::string& algo_spec)
   {
   auto algo = algorithms.find(algo_spec);

   // Not found? Check if a known alias
   if(algo == algorithms.end())
      {
      auto alias = aliases.find(algo_spec);

      if(alias != aliases.end())
         algo = algorithms.find(alias->second);
      }

   return algo;
   }

/*
* Look for an algorithm implementation by a particular provider
*/
template<typename T>
const T* Algorithm_Cache<T>::get(const std::string& algo_spec,
                                 const std::string& requested_provider)
   {
   std::lock_guard<std::mutex> lock(mutex);

   auto algo = find_algorithm(algo_spec);
   if(algo == algorithms.end()) // algo not found at all (no providers)
      return nullptr;

   // If a provider is requested specifically, return it or fail entirely
   if(requested_provider != "")
      {
      auto prov = algo->second.find(requested_provider);
      if(prov != algo->second.end())
         return prov->second;
      return nullptr;
      }

   const T* prototype = nullptr;
   std::string prototype_provider;
   size_t prototype_prov_weight = 0;

   const std::string pref_provider = search_map(pref_providers, algo_spec);

   for(auto i = algo->second.begin(); i != algo->second.end(); ++i)
      {
      // preferred prov exists, return immediately
      if(i->first == pref_provider)
         return i->second;

      const size_t prov_weight = static_provider_weight(i->first);

      if(prototype == nullptr || prov_weight > prototype_prov_weight)
         {
         prototype = i->second;
         prototype_provider = i->first;
         prototype_prov_weight = prov_weight;
         }
      }

   return prototype;
   }

/*
* Add an implementation to the cache
*/
template<typename T>
void Algorithm_Cache<T>::add(T* algo,
                             const std::string& requested_name,
                             const std::string& provider)
   {
   if(!algo)
      return;

   std::lock_guard<std::mutex> lock(mutex);

   if(algo->name() != requested_name &&
      aliases.find(requested_name) == aliases.end())
      {
      aliases[requested_name] = algo->name();
      }

   if(!algorithms[algo->name()][provider])
      algorithms[algo->name()][provider] = algo;
   else
      delete algo;
   }

/*
* Find the providers of this algo (if any)
*/
template<typename T> std::vector<std::string>
Algorithm_Cache<T>::providers_of(const std::string& algo_name)
   {
   std::lock_guard<std::mutex> lock(mutex);

   std::vector<std::string> providers;

   auto algo = find_algorithm(algo_name);
   if(algo != algorithms.end())
      {
      auto provider = algo->second.begin();

      while(provider != algo->second.end())
         {
         providers.push_back(provider->first);
         ++provider;
         }
      }

   return providers;
   }

/*
* Set the preferred provider for an algorithm
*/
template<typename T>
void Algorithm_Cache<T>::set_preferred_provider(const std::string& algo_spec,
                                                const std::string& provider)
   {
   std::lock_guard<std::mutex> lock(mutex);

   pref_providers[algo_spec] = provider;
   }

/*
* Clear out the cache
*/
template<typename T>
void Algorithm_Cache<T>::clear_cache()
   {
   auto algo = algorithms.begin();

   while(algo != algorithms.end())
      {
      auto provider = algo->second.begin();

      while(provider != algo->second.end())
         {
         delete provider->second;
         ++provider;
         }

      ++algo;
      }

   algorithms.clear();
   }

}

#endif
