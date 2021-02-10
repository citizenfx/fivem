// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_types.h"

#pragma pack(push, 8)

/**
 * Function prototype type definition for functions that allocate memory.
 *
 * Functions passed to EOS_Initialize to serve as memory allocators should return a pointer to the allocated memory.
 *
 * The returned pointer should have at least SizeInBytes available capacity and the memory address should be a multiple of Alignment.
 * The SDK will always call the provided function with an Alignment that is a power of 2.
 * Allocation failures should return a null pointer.
 */
EXTERN_C typedef void* (EOS_MEMORY_CALL * EOS_AllocateMemoryFunc)(size_t SizeInBytes, size_t Alignment);

/**
 * Function prototype type definition for functions that reallocate memory.
 *
 * Functions passed to EOS_Initialize to serve as memory reallocators should return a pointer to the reallocated memory.
 * The returned pointer should have at least SizeInBytes available capacity and the memory address should be a multiple of alignment.
 * The SDK will always call the provided function with an Alignment that is a power of 2.
 * Reallocation failures should return a null pointer.
 */
EXTERN_C typedef void* (EOS_MEMORY_CALL * EOS_ReallocateMemoryFunc)(void* Pointer, size_t SizeInBytes, size_t Alignment);

/**
 * Function prototype type definition for functions that release memory.
 *
 * When the SDK is done with memory that has been allocated by a custom allocator passed to EOS_Initialize, it will call the corresponding memory release function.
 */
EXTERN_C typedef void (EOS_MEMORY_CALL * EOS_ReleaseMemoryFunc)(void* Pointer);

/** The most recent version of the EOS_Initialize_ThreadAffinity API. */
#define EOS_INITIALIZE_THREADAFFINITY_API_LATEST 1

/**
 * Options for initializing defining thread affinity for use by Epic Online Services SDK.
 * Set the affinity to 0 to allow EOS SDK to use a platform specific default value.
 */
EOS_STRUCT(EOS_Initialize_ThreadAffinity, (
	/** API Version: Set this to EOS_INITIALIZE_THREADAFFINITY_API_LATEST. */
	int32_t ApiVersion;
	/** Any thread related to network management that is not IO. */
	uint64_t NetworkWork;
	/** Any thread that will interact with a storage device. */
	uint64_t StorageIo;
	/** Any thread that will generate web socket IO. */
	uint64_t WebSocketIo;
	/** Any thread that will generate IO related to P2P traffic and mangement. */
	uint64_t P2PIo;
	/** Any thread that will generate http request IO. */
	uint64_t HttpRequestIo;
));

/** The most recent version of the EOS_Initialize API. */
#define EOS_INITIALIZE_API_LATEST 4

/**
 * Options for initializing the Epic Online Services SDK.
 */
EOS_STRUCT(EOS_InitializeOptions, (
	/** API Version: Set this to EOS_INITIALIZE_API_LATEST. */
	int32_t ApiVersion;
	/** A custom memory allocator, if desired. */
	EOS_AllocateMemoryFunc AllocateMemoryFunction;
	/** A corresponding memory reallocator. If the AllocateMemoryFunction is nulled, then this field must also be nulled. */
	EOS_ReallocateMemoryFunc ReallocateMemoryFunction;
	/** A corresponding memory releaser. If the AllocateMemoryFunction is nulled, then this field must also be nulled. */
	EOS_ReleaseMemoryFunc ReleaseMemoryFunction;
	/**
	 * The name of the product using the Epic Online Services SDK.
	 *
	 * The name string is required to be non-empty and at maximum of 64 characters long.
	 * The string buffer can consist of the following characters:
	 * A-Z, a-z, 0-9, dot, underscore, space, exclamation mark, question mark, and sign, hyphen, parenthesis, plus, minus, colon.
	 */
	const char* ProductName;
	/**
	 * Product version of the running application.
	 *
	 * The name string has same requirements as the ProductName string.
	 */
	const char* ProductVersion;
	/** A reserved field that should always be nulled. */
	void* Reserved;
	/** 
	 * This field is for system specific initialization if any.
	 *
	 * If provided then the structure will be located in <System>/eos_<system>.h.
	 * The structure will be named EOS_<System>_InitializeOptions.
	 */
	void* SystemInitializeOptions;
	/** The thread affinity override values for each category of thread. */
	EOS_Initialize_ThreadAffinity* OverrideThreadAffinity;
));

/**
 * Initialize the Epic Online Services SDK.
 *
 * Before calling any other function in the SDK, clients must call this function.
 *
 * This function must only be called one time and must have a corresponding EOS_Shutdown call.
 *
 * @param Options - The initialization options to use for the SDK.
 * @return An EOS_EResult is returned to indicate success or an error. 
 *
 * EOS_Success is returned if the SDK successfully initializes.
 * EOS_AlreadyConfigured is returned if the function has already been called.
 * EOS_InvalidParameters is returned if the provided options are invalid.
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Initialize(const EOS_InitializeOptions* Options);

/**
 * Tear down the Epic Online Services SDK.
 *
 * Once this function has been called, no more SDK calls are permitted; calling anything after EOS_Shutdown will result in undefined behavior.
 * @return An EOS_EResult is returned to indicate success or an error.
 * EOS_Success is returned if the SDK is successfully torn down.
 * EOS_NotConfigured is returned if a successful call to EOS_Initialize has not been made.
 * EOS_UnexpectedError is returned if EOS_Shutdown has already been called.
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Shutdown();

/**
 * Create a single Epic Online Services Platform Instance.
 *
 * The platform instance is used to gain access to the various Epic Online Services.
 *
 * This function returns an opaque handle to the platform instance, and that handle must be passed to EOS_Platform_Release to release the instance.
 *
 * @return An opaque handle to the platform instance.
 */
EOS_DECLARE_FUNC(EOS_HPlatform) EOS_Platform_Create(const EOS_Platform_Options* Options);

/**
 * Release an Epic Online Services platform instance previously returned from EOS_Platform_Create.
 *
 * This function should only be called once per instance returned by EOS_Platform_Create. Undefined behavior will result in calling it with a single instance more than once.
 * Typically only a single platform instance needs to be created during the lifetime of a game.
 * You should release each platform instance before calling the EOS_Shutdown function.
 */
EOS_DECLARE_FUNC(void) EOS_Platform_Release(EOS_HPlatform Handle);

#pragma pack(pop)
