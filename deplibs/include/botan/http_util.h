/*
* HTTP utilities
* (C) 2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_UTILS_URLGET_H__
#define BOTAN_UTILS_URLGET_H__

#include <botan/types.h>
#include <future>
#include <vector>
#include <map>
#include <chrono>
#include <string>

namespace Botan {

namespace HTTP {

struct Response
   {
   public:
      Response() : m_status_code(0), m_status_message("Uninitialized") {}

      Response(unsigned int status_code, const std::string& status_message,
               const std::vector<byte>& body,
               const std::map<std::string, std::string>& headers) :
         m_status_code(status_code),
         m_status_message(status_message),
         m_body(body),
         m_headers(headers) {}

      unsigned int status_code() const { return m_status_code; }

      const std::vector<byte>& body() const { return m_body; }

      const std::map<std::string, std::string>& headers() const { return m_headers; }

      std::string status_message() const { return m_status_message; }

      void throw_unless_ok()
         {
         if(status_code() != 200)
            throw std::runtime_error("HTTP error: " + status_message());
         }

   private:
      unsigned int m_status_code;
      std::string m_status_message;
      std::vector<byte> m_body;
      std::map<std::string, std::string> m_headers;
   };

BOTAN_DLL std::ostream& operator<<(std::ostream& o, const Response& resp);

typedef std::function<std::string (const std::string&, const std::string&)> http_exch_fn;

#if defined(BOTAN_HAS_BOOST_ASIO)
std::string BOTAN_DLL http_transact_asio(const std::string& hostname,
                                         const std::string& message);
#endif

std::string BOTAN_DLL http_transact_fail(const std::string& hostname,
                                         const std::string& message);


BOTAN_DLL Response http_sync(http_exch_fn fn,
                             const std::string& verb,
                             const std::string& url,
                             const std::string& content_type,
                             const std::vector<byte>& body,
                             size_t allowable_redirects);

BOTAN_DLL Response http_sync(const std::string& verb,
                             const std::string& url,
                             const std::string& content_type,
                             const std::vector<byte>& body,
                             size_t allowable_redirects);

BOTAN_DLL Response GET_sync(const std::string& url,
                            size_t allowable_redirects = 1);

BOTAN_DLL Response POST_sync(const std::string& url,
                             const std::string& content_type,
                             const std::vector<byte>& body,
                             size_t allowable_redirects = 1);

std::future<Response> BOTAN_DLL GET_async(const std::string& url,
                                                    size_t allowable_redirects = 1);

BOTAN_DLL std::string url_encode(const std::string& url);

}

}

#endif
