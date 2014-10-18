/*
* Runtime assertion checking
* (C) 2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_ASSERTION_CHECKING_H__
#define BOTAN_ASSERTION_CHECKING_H__

#include <botan/build.h>

namespace Botan {

/**
* Called when an assertion fails
*/
void BOTAN_DLL assertion_failure(const char* expr_str,
                                 const char* assertion_made,
                                 const char* func,
                                 const char* file,
                                 int line);

/**
* Make an assertion
*/
#define BOTAN_ASSERT(expr, assertion_made)                \
   do {                                                   \
      if(!(expr))                                         \
         Botan::assertion_failure(#expr,                  \
                                  assertion_made,         \
                                  BOTAN_CURRENT_FUNCTION, \
                                  __FILE__,               \
                                  __LINE__);              \
   } while(0)

/**
* Assert that value1 == value2
*/
#define BOTAN_ASSERT_EQUAL(expr1, expr2, assertion_made)   \
   do {                                                    \
     if((expr1) != (expr2))                                \
       Botan::assertion_failure(#expr1 " == " #expr2,      \
                                  assertion_made,          \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Assert that expr1 (if true) implies expr2 is also true
*/
#define BOTAN_ASSERT_IMPLICATION(expr1, expr2, msg)        \
   do {                                                    \
     if((expr1) && !(expr2))                               \
       Botan::assertion_failure(#expr1 " implies " #expr2, \
                                msg,                       \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Assert that a pointer is not null
*/
#define BOTAN_ASSERT_NONNULL(ptr)                          \
   do {                                                    \
      if(static_cast<bool>(ptr) == false)                  \
         Botan::assertion_failure(#ptr " is not null",     \
                                  "",                      \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Mark variable as unused
*/
#define BOTAN_UNUSED(v) static_cast<void>(v)

}

#endif
