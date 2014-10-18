/*
* Global State Management
* (C) 2010 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_GLOBAL_STATE_H__
#define BOTAN_GLOBAL_STATE_H__

#include <botan/build.h>

namespace Botan {

/*
* Forward declare to avoid recursive dependency between this header
* and libstate.h
*/
class Library_State;

/**
* Namespace for management of the global state
*/
namespace Global_State_Management {

/**
* Access the global library state
* @return reference to the global library state
*/
BOTAN_DLL Library_State& global_state();

/**
* Set the global state object
* @param state the new global state to use
*/
BOTAN_DLL void set_global_state(Library_State* state);

/**
* Set the global state object unless it is already set
* @param state the new global state to use
* @return true if the state parameter is now being used as the global
*         state, or false if one was already set, in which case the
*         parameter was deleted immediately
*/
BOTAN_DLL bool set_global_state_unless_set(Library_State* state);

/**
* Swap the current state for another
* @param new_state the new state object to use
* @return previous state (or NULL if none)
*/
BOTAN_DLL Library_State* swap_global_state(Library_State* new_state);

/**
* Query if the library is currently initialized
* @return true iff the library is initialized
*/
BOTAN_DLL bool global_state_exists();

}

/*
* Insert into Botan ns for convenience/backwards compatability
*/
using Global_State_Management::global_state;

}

#endif
