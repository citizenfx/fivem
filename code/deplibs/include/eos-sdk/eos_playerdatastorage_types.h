// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

#pragma pack(push, 8)

/** Maximum File Name Length in bytes */
#define EOS_PLAYERDATASTORAGE_FILENAME_MAX_LENGTH_BYTES 64

/** Maximum File size in bytes*/
#define EOS_PLAYERDATASTORAGE_FILE_MAX_SIZE_BYTES (64 * 1024 * 1024)

EXTERN_C typedef struct EOS_PlayerDataStorageHandle* EOS_HPlayerDataStorage;

#define EOS_PLAYERDATASTORAGE_FILEMETADATA_API_LATEST 1

/**
 * Metadata information for a specific file
 */
EOS_STRUCT(EOS_PlayerDataStorage_FileMetadata, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_FILEMETADATA_API_LATEST. */
	int32_t ApiVersion;
	/** The total size of the file in bytes (Includes file header in addition to file contents) */
	uint32_t FileSizeBytes;
	/** The MD5 Hash of the entire file (including additional file header), in hex digits */
	const char* MD5Hash;
	/** The file's name */
	const char* Filename;
));

/**
 * Free the memory used by the file metadata
 */
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorage_FileMetadata_Release(EOS_PlayerDataStorage_FileMetadata* FileMetadata);


#define EOS_PLAYERDATASTORAGE_QUERYFILEOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_PlayerDataStorage_QueryFile function
 */
EOS_STRUCT(EOS_PlayerDataStorage_QueryFileOptions, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_QUERYFILEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user requesting file metadata */
	EOS_ProductUserId LocalUserId;
	/** The name of the file being queried */
	const char* Filename;
));

/**
 * Data containing information about a query file request
 */
EOS_STRUCT(EOS_PlayerDataStorage_QueryFileCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the file query request */
	void* ClientData;
	/** The Product User ID of the local user who initiated this request */
	EOS_ProductUserId LocalUserId;
));
/**
 * Callback for when EOS_PlayerDataStorage_QueryFile completes
 */
EOS_DECLARE_CALLBACK(EOS_PlayerDataStorage_OnQueryFileCompleteCallback, const EOS_PlayerDataStorage_QueryFileCallbackInfo* Data);


#define EOS_PLAYERDATASTORAGE_QUERYFILELISTOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_PlayerDataStorage_QueryFileList function
 */
EOS_STRUCT(EOS_PlayerDataStorage_QueryFileListOptions, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_QUERYFILELISTOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who requested file metadata */
	EOS_ProductUserId LocalUserId;
));

/**
 * Data containing information about a query file list request
 */
EOS_STRUCT(EOS_PlayerDataStorage_QueryFileListCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the file query request */
	void* ClientData;
	/** The Product User ID of the local user who initiated this request */
	EOS_ProductUserId LocalUserId;
	/** A count of files that were found, if successful */
	uint32_t FileCount;
));
/**
 * Callback for when EOS_PlayerDataStorage_QueryFileList completes
 */
EOS_DECLARE_CALLBACK(EOS_PlayerDataStorage_OnQueryFileListCompleteCallback, const EOS_PlayerDataStorage_QueryFileListCallbackInfo* Data);


#define EOS_PLAYERDATASTORAGE_GETFILEMETADATACOUNTOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_PlayerDataStorage_GetFileMetadataCount function
 */
EOS_STRUCT(EOS_PlayerDataStorage_GetFileMetadataCountOptions, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_GETFILEMETADATACOUNTOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who is requesting file metadata */
	EOS_ProductUserId LocalUserId;
));


#define EOS_PLAYERDATASTORAGE_COPYFILEMETADATAATINDEXOPTIONS_API_LATEST 1

/**
 * Input data for the CopyFileMetadataAtIndex function
 */
EOS_STRUCT(EOS_PlayerDataStorage_CopyFileMetadataAtIndexOptions, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_COPYFILEMETADATAATINDEXOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who is requesting file metadata */
	EOS_ProductUserId LocalUserId;
	/** The index to get data for */
	uint32_t Index;
));


#define EOS_PLAYERDATASTORAGE_COPYFILEMETADATABYFILENAMEOPTIONS_API_LATEST 1

/**
 * Input data for the CopyFileMetadataByFilename function
 */
EOS_STRUCT(EOS_PlayerDataStorage_CopyFileMetadataByFilenameOptions, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_COPYFILEMETADATABYFILENAMEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who is requesting file metadata */
	EOS_ProductUserId LocalUserId;
	/** The file's name to get data for */
	const char* Filename;
));


#define EOS_PLAYERDATASTORAGE_DUPLICATEFILEOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_PlayerDataStorage_DuplicateFile function
 */
EOS_STRUCT(EOS_PlayerDataStorage_DuplicateFileOptions, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_DUPLICATEFILEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who authorized the duplication of the requested file; must be the original file's owner */
	EOS_ProductUserId LocalUserId;
	/** The name of the existing file to duplicate */
	const char* SourceFilename;
	/** The name of the new file */
	const char* DestinationFilename;
));

/**
 * Data containing the result information for a duplicate file request
 */
EOS_STRUCT(EOS_PlayerDataStorage_DuplicateFileCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the file duplicate request */
	void* ClientData;
	/** The Product User ID of the local user who initiated this request */
	EOS_ProductUserId LocalUserId;
));

/**
 * Callback for when EOS_PlayerDataStorage_DuplicateFile completes
 */
EOS_DECLARE_CALLBACK(EOS_PlayerDataStorage_OnDuplicateFileCompleteCallback, const EOS_PlayerDataStorage_DuplicateFileCallbackInfo* Data);


#define EOS_PLAYERDATASTORAGE_DELETEFILEOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_PlayerDataStorage_DeleteFile function
 */
EOS_STRUCT(EOS_PlayerDataStorage_DeleteFileOptions, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_DELETEFILEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who authorizes deletion of the file; must be the file's owner */
	EOS_ProductUserId LocalUserId;
	/** The name of the file to delete */
	const char* Filename;
));

/**
 * Data containing the result information for a delete file request
 */
EOS_STRUCT(EOS_PlayerDataStorage_DeleteFileCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the file deletion request */
	void* ClientData;
	/** The Product User ID of the local user who initiated this request */
	EOS_ProductUserId LocalUserId;
));

/**
 * Callback for when EOS_PlayerDataStorage_DeleteFile completes
 */
EOS_DECLARE_CALLBACK(EOS_PlayerDataStorage_OnDeleteFileCompleteCallback, const EOS_PlayerDataStorage_DeleteFileCallbackInfo* Data);

/**
 * Handle type to a File Request
 */
EXTERN_C typedef struct EOS_PlayerDataStorageFileTransferRequestHandle* EOS_HPlayerDataStorageFileTransferRequest;

/**
 * Free the memory used by a cloud-storage file request handle. This will not cancel a request in progress.
 */
EOS_DECLARE_FUNC(void) EOS_PlayerDataStorageFileTransferRequest_Release(EOS_HPlayerDataStorageFileTransferRequest PlayerDataStorageFileTransferHandle);


/**
 * Data containing the information about a file transfer in progress (or one that has completed)
 */
EOS_STRUCT(EOS_PlayerDataStorage_FileTransferProgressCallbackInfo, (
	/** Client-specified data passed into the file request */
	void* ClientData;
	/** The Product User ID of the local user who initiated this request */
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
EOS_DECLARE_CALLBACK(EOS_PlayerDataStorage_OnFileTransferProgressCallback, const EOS_PlayerDataStorage_FileTransferProgressCallbackInfo* Data);


/**
 * Return results for EOS_PlayerDataStorage_OnReadFileDataCallback callbacks to return
 */
EOS_ENUM(EOS_PlayerDataStorage_EReadResult,
	/** Signifies the data was read successfully, and we should continue to the next chunk if possible */
	EOS_RR_ContinueReading = 1,
	/** Signifies there was a failure reading the data, and the request should end */
	EOS_RR_FailRequest = 2,
	/** Signifies the request should be cancelled, but not due to an error */
	EOS_RR_CancelRequest = 3
);

/**
 * Data containing data for a chunk of a file being read
 */
EOS_STRUCT(EOS_PlayerDataStorage_ReadFileDataCallbackInfo, (
	/** Client-specified data passed into the file request */
	void* ClientData;
	/** The Product User ID of the local user who initiated this request */
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
 * @return The result of the read operation. If this value is not EOS_RR_ContinueReading, this callback will not be called again for the same request
 */
EOS_DECLARE_CALLBACK_RETVALUE(EOS_PlayerDataStorage_EReadResult, EOS_PlayerDataStorage_OnReadFileDataCallback, const EOS_PlayerDataStorage_ReadFileDataCallbackInfo* Data);

#define EOS_PLAYERDATASTORAGE_READFILEOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_PlayerDataStorage_ReadFile function
 */
EOS_STRUCT(EOS_PlayerDataStorage_ReadFileOptions, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_READFILEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who is reading the requested file */
	EOS_ProductUserId LocalUserId;
	/** The file name to read; this file must already exist */
	const char* Filename;
	/** The maximum amount of data in bytes should be available to read in a single EOS_PlayerDataStorage_OnReadFileDataCallback call */
	uint32_t ReadChunkLengthBytes;
	/** Callback function that handles data as it comes in, and can stop the transfer early */
	EOS_PlayerDataStorage_OnReadFileDataCallback ReadFileDataCallback;
	/** Optional callback function to be informed of download progress, if the file is not already locally cached; if provided, this will be called at least once before completion if the request is successfully started */
	EOS_PlayerDataStorage_OnFileTransferProgressCallback FileTransferProgressCallback;
));


/**
 * Data containing the result of a read file request
 */
EOS_STRUCT(EOS_PlayerDataStorage_ReadFileCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the file read request */
	void* ClientData;
	/** The Product User ID of the local user who initiated this request */
	EOS_ProductUserId LocalUserId;
	/** The filename of the file that has been finished reading */
	const char* Filename;
));

/**
 * Callback for when EOS_PlayerDataStorage_ReadFile completes
 */
EOS_DECLARE_CALLBACK(EOS_PlayerDataStorage_OnReadFileCompleteCallback, const EOS_PlayerDataStorage_ReadFileCallbackInfo* Data);


/**
 * Return results for EOS_PlayerDataStorage_OnWriteFileDataCallback callbacks to return
 */
EOS_ENUM(EOS_PlayerDataStorage_EWriteResult,
	/** Signifies the data was written successfully, and we should write the data the file */
	EOS_WR_ContinueWriting = 1,
	/** Signifies all data has now been written successfully, and we should upload the data to the cloud */
	EOS_WR_CompleteRequest = 2,
	/** Signifies there was a failure writing the data, and the request should end */
	EOS_WR_FailRequest = 3,
	/** Signifies the request should be cancelled, but not due to an error */
	EOS_WR_CancelRequest = 4
);

/**
 * Data containing data for a chunk of a file being written
 */
EOS_STRUCT(EOS_PlayerDataStorage_WriteFileDataCallbackInfo, (
	/** Client-specified data passed into the file write request */
	void* ClientData;
	/** The Product User ID of the local user who initiated this request */
	EOS_ProductUserId LocalUserId;
	/** The file name that is being written to */
	const char* Filename;
	/** The maximum amount of data in bytes that can be written safely to DataBuffer */
	uint32_t DataBufferLengthBytes;
));

/**
 * Callback for when we are ready to get more data to be written into the requested file. It is undefined how often this will be called during a single tick.
 *
 * @param Data Struct containing metadata for the file being written to, as well as the max length in bytes that can be safely written to DataBuffer
 * @param OutDataBuffer A buffer to write data into, to be appended to the end of the file that is being written to. The maximum length of this value is provided in the Info parameter. The number of bytes written to this buffer should be set in OutDataWritten.
 * @param OutDataWritten The length of the data written to OutDataBuffer. This must be less than or equal than the DataBufferLengthBytes provided in the Info parameter
 * @return The result of the write operation. If this value is not EOS_WR_ContinueWriting, this callback will not be called again for the same request. If this is set to EOS_WR_FailRequest or EOS_WR_CancelRequest, all data written during the request will not be saved
 */
EOS_DECLARE_CALLBACK_RETVALUE(EOS_PlayerDataStorage_EWriteResult, EOS_PlayerDataStorage_OnWriteFileDataCallback, const EOS_PlayerDataStorage_WriteFileDataCallbackInfo* Data, void* OutDataBuffer, uint32_t* OutDataWritten);

#define EOS_PLAYERDATASTORAGE_WRITEFILEOPTIONS_API_LATEST 1

/**
 * Input data for the EOS_PlayerDataStorage_WriteFile function
 */
EOS_STRUCT(EOS_PlayerDataStorage_WriteFileOptions, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_WRITEFILEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who is writing the requested file to the cloud */
	EOS_ProductUserId LocalUserId;
	/** The name of the file to write; if this file already exists, the contents will be replaced if the write request completes successfully */
	const char* Filename;
	/** Requested maximum amount of data (in bytes) that can be written to the file per tick */
	uint32_t ChunkLengthBytes;
	/** Callback function that provides chunks of data to be written into the requested file */
	EOS_PlayerDataStorage_OnWriteFileDataCallback WriteFileDataCallback;
	/** Optional callback function to inform the application of upload progress; will be called at least once if set */
	EOS_PlayerDataStorage_OnFileTransferProgressCallback FileTransferProgressCallback;
));

/**
 * The result information for a request to write data to a file
 */
EOS_STRUCT(EOS_PlayerDataStorage_WriteFileCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the file write request */
	void* ClientData;
	/** The Product User ID of the local user who initiated this request */
	EOS_ProductUserId LocalUserId;
	/** The file name that is being written to */
	const char* Filename;
));

/**
 * Callback for when EOS_PlayerDataStorage_WriteFile completes
 */
EOS_DECLARE_CALLBACK(EOS_PlayerDataStorage_OnWriteFileCompleteCallback, const EOS_PlayerDataStorage_WriteFileCallbackInfo* Data);

/** The most recent version of the EOS_PlayerDataStorage_DeleteCacheOptions API. */
#define EOS_PLAYERDATASTORAGE_DELETECACHEOPTIONS_API_LATEST 1
/**
 * Input data for the EOS_TitleStorage_DeleteCache function
 */
EOS_STRUCT(EOS_PlayerDataStorage_DeleteCacheOptions, (
	/** API Version: Set this to EOS_PLAYERDATASTORAGE_DELETECACHEOPTIONS_API_LATEST. */
	int32_t ApiVersion;
	/** Product User ID of the local user who is deleting his cache */
	EOS_ProductUserId LocalUserId;
));
/**
 * Structure containing the result of a delete cache operation
 */
EOS_STRUCT(EOS_PlayerDataStorage_DeleteCacheCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into the delete cache request */
	void* ClientData;
	/** Product User ID of the local user who initiated this request */
	EOS_ProductUserId LocalUserId;
));
/**
 * Callback for when EOS_PlayerDataStorage_DeleteCache completes
 */
EOS_DECLARE_CALLBACK(EOS_PlayerDataStorage_OnDeleteCacheCompleteCallback, const EOS_PlayerDataStorage_DeleteCacheCallbackInfo* Data);


#pragma pack(pop)