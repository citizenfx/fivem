/*
* Exceptions
* (C) 2004-2006 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_EXCEPTION_H__
#define BOTAN_TLS_EXCEPTION_H__

#include <botan/exceptn.h>
#include <botan/tls_alert.h>

namespace Botan {

namespace TLS {

/**
* Exception Base Class
*/
class BOTAN_DLL TLS_Exception : public Exception
   {
   public:
      Alert::Type type() const BOTAN_NOEXCEPT { return alert_type; }

      TLS_Exception(Alert::Type type,
                    const std::string& err_msg = "Unknown error") :
         Exception(err_msg), alert_type(type) {}

   private:
      Alert::Type alert_type;
   };

/**
* Unexpected_Message Exception
*/
struct BOTAN_DLL Unexpected_Message : public TLS_Exception
   {
   Unexpected_Message(const std::string& err) :
      TLS_Exception(Alert::UNEXPECTED_MESSAGE, err) {}
   };

}

}

#endif
