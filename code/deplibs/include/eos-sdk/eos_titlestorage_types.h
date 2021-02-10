// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

#pragma pack(push, 8)

/** Maximum File Name Length in bytes */
#define EOS_TITLESTORAGE_FILENAME_MAX_LENGTH_BYTES 64

EXTERN_C typedef struct EOS_TitleStorageHandle* EOS_HTitleStorage;

/** The most recent version of the EOS_TitleStorage_FileMetadata API. */
#define EOS_TITLESTORAGE_FILEMETADATA_API_LATEST 1

/**
 * Metadata information for a specific file
 */
EOS_STRUCT(EOS_TitleStorage_FileMetadata, (
	/** API Version: Set this to EOS_TITLESTORAGE_FILEMETADATA_API_LATEST. */
	int32_t ApiVersion;
	/** The total size of the file in bytes (Includes file header in addition to file contents). */
	uint32_t FileSizeBytes;
	/** The MD5 Hash of the entire file (including additional file header), in hex digits */
	const char* MD5Hash;
	/** The file's name */
	const char* Filename;
));

/**
 * Free the memory used by the file metadata
 */
EOS_DECLARE_FUNC(void) EOS_TitleStorage_FileMetadata_Release(EOS_TitleStorage_FileMetadata* FileMetadata);

/** The most recent version of the EOS_TitleStorage_QueryFileOptions API. */
#define EOS_TITLESTORAGE_QUERYFILEOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_TitleStorage_QueryFile function
 */
EOS_STRUCT(EOS_TitleStorage_QueryFileOptions, (
	/** API Version: Set this to EOS_TITLESTORAGE_QUERYFILEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** Product User ID of the local user requesting file metadata (optional) */
	EOS_ProductUserId LocalUserId;
	/** The requested file's name */
	const char* Filename;
));

/**
 * Structure containing information about a query file request
 */
EOS_STRUCT(EOS_TitleStorage_QueryFileCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the file query request */
	void* ClientData;
	/** Product User ID of the local user who initiated this request (optional, will only be present in case it was provided during operation start) */
	EOS_ProductUserId LocalUserId;
));

/**
 * Callback for when EOS_TitleStorage_QueryFile completes
 */
EOS_DECLARE_CALLBACK(EOS_TitleStorage_OnQueryFileCompleteCallback, const EOS_TitleStorage_QueryFileCallbackInfo* Data);

/** The most recent version of the EOS_TitleStorage_QueryFileListOptions API. */
#define EOS_TITLESTORAGE_QUERYFILELISTOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_TitleStorage_QueryFileList function
 */
EOS_STRUCT(EOS_TitleStorage_QueryFileListOptions, (
	/** API Version: Set this to EOS_TITLESTORAGE_QUERYFILELISTOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** Product User ID of the local user who requested file metadata (optional) */
	EOS_ProductUserId LocalUserId;
	/** List of tags to use for lookup. */
	const char* const* ListOfTags;
	/** Number of tags specified in ListOfTags */
	uint32_t ListOfTagsCount;
));

/**
 * Structure containing information about a query file list request
 */
EOS_STRUCT(EOS_TitleStorage_QueryFileListCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the file query request */
	void* ClientData;
	/** Product User ID of the local user who initiated this request (optional, will only be present in case it was provided during operation start) */
	EOS_ProductUserId LocalUserId;
	/** A count of files that were found, if successful */
	uint32_t FileCount;
));

/**
 * Callback for when EOS_TitleStorage_QueryFileList completes
 */
EOS_DECLARE_CALLBACK(EOS_TitleStorage_OnQueryFileListCompleteCallback, const EOS_TitleStorage_QueryFileListCallbackInfo* Data);

/** The most recent version of the EOS_TitleStorage_GetFileMetadataCountOptions API. */
#define EOS_TITLESTORAGE_GETFILEMETADATACOUNTOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_TitleStorage_GetFileMetadataCount function
 */
EOS_STRUCT(EOS_TitleStorage_GetFileMetadataCountOptions, (
	/** API Version: Set this to EOS_TITLESTORAGE_GETFILEMETADATACOUNTOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** Product User ID of the local user who is requesting file metadata (optional) */
	EOS_ProductUserId LocalUserId;
));

/** The most recent version of the EOS_TitleStorage_CopyFileMetadataAtIndexOptions API. */
#define EOS_TITLESTORAGE_COPYFILEMETADATAATINDEXOPTIONS_API_LATEST 1

/**
 * Input data for the CopyFileMetadataAtIndex function
 */
EOS_STRUCT(EOS_TitleStorage_CopyFileMetadataAtIndexOptions, (
	/** API Version: Set this to EOS_TITLESTORAGE_COPYFILEMETADATAATINDEXOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** Product User ID of the local user who is requesting file metadata (optional) */
	EOS_ProductUserId LocalUserId;
	/** The index to get data for */
	uint32_t Index;
));

/** The most recent version of the EOS_TitleStorage_CopyFileMetadataByFilenameOptions API. */
#define EOS_TITLESTORAGE_COPYFILEMETADATABYFILENAMEOPTIONS_API_LATEST 1

/**
 * Input data for the CopyFileMetadataByFilename function
 */
EOS_STRUCT(EOS_TitleStorage_CopyFileMetadataByFilenameOptions, (
	/** API Version: Set this to EOS_TITLESTORAGE_COPYFILEMETADATABYFILENAMEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** Product User ID of the local user who is requesting file metadata (optional) */
	EOS_ProductUserId LocalUserId;
	/** The file's name to get data for */
	const char* Filename;
));

/**
 * Handle type to a File Request
 */
EXTERN_C typedef struct EOS_TitleStorageFileTransferRequestHandle* EOS_HTitleStorageFileTransferRequest;

/**
 * Free the memory used by a cloud-storage file request handle. This will not cancel a request in progress.
 */
EOS_DECLARE_FUNC(void) EOS_TitleStorageFileTransferRequest_Release(EOS_HTitleStorageFileTransferRequest TitleStorageFileTransferHandle);

/**
 * Structure containing the information about a file transfer in progress (or one that has completed)
 */
EOS_STRUCT(EOS_TitleStorage_FileTransferProgressCallbackInfo, (
	/** Client-specified data passed into the file request */
	void* ClientData;
	/** Product User ID of the local user who initiated this request (optional, will only be present in case it was provided during operation start) */
	EOS_ProductUserId LocalUserId;
	/** The file name of the file being transferred */
	const char* Filename;
	/** Amount of bytes transferred so far in this request, out of TotalFileSizeBytes */
	uint32_t BytesTransferred;
	/** The total size of the file being transferred (Includes file header in addition to file contents, can be slightly more than expected) */
	uint32_t TotalFileSizeBytes;
));

/**
 * Callback for when there is a progress update for a file transfer in progress
 */
EOS_DECLARE_CALLBACK(EOS_TitleStorage_OnFileTransferProgressCallback, const EOS_TitleStorage_FileTransferProgressCallbackInfo* Data);

/**
 * Return results for EOS_TitleStorage_OnReadFileDataCallback callbacks
 */
EOS_ENUM(EOS_TitleStorage_EReadResult,
	/** Signifies the data was read successfully, and we should continue to the next chunk if possible */
	EOS_TS_RR_ContinueReading = 1,
	/** Signifies there was a failure reading the data, and the request should end */
	EOS_TS_RR_FailRequest = 2,
	/** Signifies the request should be cancelled, but not due to an error */
	EOS_TS_RR_CancelRequest = 3
);

/**
 * Structure containing data for a chunk of a file being read
 */
EOS_STRUCT(EOS_TitleStorage_ReadFileDataCallbackInfo, (
	/** Client-specified data passed into the file request */
	void* ClientData;
	/** Product User ID of the local user who initiated this request (optional, will only be present in case it was provided during operation start) */
	EOS_ProductUserId LocalUserId;
	/** The file name being read */
	const char* Filename;
	/** The total file size of the file being read */
	uint32_t TotalFileSizeBytes;
	/** Is this chunk the last chunk of data? */
	EOS_Bool bIsLastChunk;
	/** The length of DataChunk in bytes that can be safely read */
	uint32_t DataChunkLengthBytes;
	/** Pointer to the start of data to be read */
	const void* DataChunk;
));

/**
 * Callback for when we have data ready to be read from the requested file. It is undefined how often this will be called during a single tick.
 *
 * @param Data Struct containing a chunk of data to read, as well as some metadata for the file being read
 * @return The result of the read operation. If this value is not EOS_TS_RR_ContinueReading, this callback will not be called again for the same request
 */
EOS_DECLARE_CALLBACK_RETVALUE(EOS_TitleStorage_EReadResult, EOS_TitleStorage_OnReadFileDataCallback, const EOS_TitleStorage_ReadFileDataCallbackInfo* Data);

/** The most recent version of the EOS_TitleStorage_ReadFileOptions API. */
#define EOS_TITLESTORAGE_READFILEOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_TitleStorage_ReadFile function
 */
EOS_STRUCT(EOS_TitleStorage_ReadFileOptions, (
	/** API Version: Set this to EOS_TITLESTORAGE_READFILEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** Product User ID of the local user who is reading the requested file (optional) */
	EOS_ProductUserId LocalUserId;
	/** The file name to read; this file must already exist */
	const char* Filename;
	/** The maximum amount of data in bytes should be available to read in a single EOS_TitleStorage_OnReadFileDataCallback call */
	uint32_t ReadChunkLengthBytes;
	/** Callback function to handle copying read data */
	EOS_TitleStorage_OnReadFileDataCallback ReadFileDataCallback;
	/** Optional callback function to be informed of download progress, if the file is not already locally cached. If set, this will be called at least once before completion if the request is successfully started */
	EOS_TitleStorage_OnFileTransferProgressCallback FileTransferProgressCallback;
));

/**
 * Structure containing the result of a read file request
 */
EOS_STRUCT(EOS_TitleStorage_ReadFileCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the file read request */
	void* ClientData;
	/** Product User ID of the local user who initiated this request (optional, will only be present in case it was provided during operation start) */
	EOS_ProductUserId LocalUserId;
	/** The filename of the file that has been finished reading */
	const char* Filename;
));

/**
 * Callback for when EOS_TitleStorage_ReadFile completes
 */
EOS_DECLARE_CALLBACK(EOS_TitleStorage_OnReadFileCompleteCallback, const EOS_TitleStorage_ReadFileCallbackInfo* Data);

/** The most recent version of the EOS_TitleStorage_DeleteCacheOptions API. */
#define EOS_TITLESTORAGE_DELETECACHEOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_TitleStorage_DeleteCache function
 */
EOS_STRUCT(EOS_TitleStorage_DeleteCacheOptions, (
	/** API Version: Set this to EOS_TITLESTORAGE_DELETECACHEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** Product User ID of the local user who is deleting his cache (optional) */
	EOS_ProductUserId LocalUserId;
));

/**
 * Structure containing the result of a delete cache operation
 */
EOS_STRUCT(EOS_TitleStorage_DeleteCacheCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the delete cache request */
	void* ClientData;
	/** Product User ID of the local user who initiated this request (optional, will only be present in case it was provided during operation start) */
	EOS_ProductUserId LocalUserId;
));

/** 
 * Callback for when EOS_TitleStorage_DeleteCache completes
 */
EOS_DECLARE_CALLBACK(EOS_TitleStorage_OnDeleteCacheCompleteCallback, const EOS_TitleStorage_DeleteCacheCallbackInfo* Data);

#pragma pack(pop)