// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_titlestorage_types.h"

/**
 * The following EOS_TitleStorage_* functions allow you to query metadata for available files from title storage as well as download and read their contents.
 */

/**
 * Query a specific file's metadata, such as file names, size, and a MD5 hash of the data. This is not required before a file may be opened. Once a file has
 * been queried, its metadata will be available by the EOS_TitleStorage_CopyFileMetadataAtIndex and EOS_TitleStorage_CopyFileMetadataByFilename functions.
 *
 * @param Options Object containing properties related to which user is querying files, and what file is being queried
 * @param ClientData Optional pointer to help clients track this request, that is returned in the completion callback
 * @param CompletionCallback This function is called when the query operation completes
 *
 * @see EOS_TitleStorage_GetFileMetadataCount
 * @see EOS_TitleStorage_CopyFileMetadataAtIndex
 * @see EOS_TitleStorage_CopyFileMetadataByFilename
 */
EOS_DECLARE_FUNC(void) EOS_TitleStorage_QueryFile(EOS_HTitleStorage Handle, const EOS_TitleStorage_QueryFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileCompleteCallback CompletionCallback);

/**
 * Query the file metadata, such as file names, size, and a MD5 hash of the data, for all files available for current user based on their settings (such as game role) and tags provided.
 * This is not required before a file can be downloaded by name.
 *
 * @param Options Object containing properties related to which user is querying files and the list of tags
 * @param ClientData Optional pointer to help clients track this request, that is returned in the completion callback
 * @param CompletionCallback This function is called when the query operation completes
 *
 */
EOS_DECLARE_FUNC(void) EOS_TitleStorage_QueryFileList(EOS_HTitleStorage Handle, const EOS_TitleStorage_QueryFileListOptions* Options, void* ClientData, const EOS_TitleStorage_OnQueryFileListCompleteCallback CompletionCallback);

/**
 * Create a cached copy of a file's metadata by filename. The metadata will be for the last retrieved or successfully saved version, and will not include any changes that have not
 * completed writing. The returned pointer must be released by the user when no longer needed.
 *
 * @param Options Object containing properties related to which user is requesting metadata, and for which filename
 * @param OutMetadata A copy of the FileMetadata structure will be set if successful.  This data must be released by calling EOS_TitleStorage_FileMetadata_Release.
 * @return EOS_Success if the metadata is currently cached, otherwise an error result explaining what went wrong
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_CopyFileMetadataByFilename(EOS_HTitleStorage Handle, const EOS_TitleStorage_CopyFileMetadataByFilenameOptions* Options, EOS_TitleStorage_FileMetadata ** OutMetadata);

/**
 * Get the count of files we have previously queried information for and files we have previously read from / written to.
 *
 * @param Options Object containing properties related to which user is requesting the metadata count
 * @return If successful, the count of metadata currently cached. Returns 0 on failure.
 *
 * @see EOS_TitleStorage_CopyFileMetadataAtIndex
 */
EOS_DECLARE_FUNC(uint32_t) EOS_TitleStorage_GetFileMetadataCount(EOS_HTitleStorage Handle, const EOS_TitleStorage_GetFileMetadataCountOptions* Options);

/**
 * Get the cached copy of a file's metadata by index. The metadata will be for the last retrieved version. The returned pointer must be released by the user when no longer needed.
 *
 * @param Options Object containing properties related to which user is requesting metadata, and at what index
 * @param OutMetadata A copy of the FileMetadata structure will be set if successful.  This data must be released by calling EOS_TitleStorage_FileMetadata_Release.
 * @return EOS_Success if the requested metadata is currently cached, otherwise an error result explaining what went wrong.
 *
 * @see EOS_TitleStorage_GetFileMetadataCount
 * @see EOS_TitleStorage_FileMetadata_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_CopyFileMetadataAtIndex(EOS_HTitleStorage Handle, const EOS_TitleStorage_CopyFileMetadataAtIndexOptions* Options, EOS_TitleStorage_FileMetadata ** OutMetadata);

/**
 * Retrieve the contents of a specific file, potentially downloading the contents if we do not have a local copy, from the cloud. This request will occur asynchronously, potentially over
 * multiple frames. All callbacks for this function will come from the same thread that the SDK is ticked from. If specified, the FileTransferProgressCallback will always be called at
 * least once if the request is started successfully.
 *
 * @param Options Object containing properties related to which user is opening the file, what the file's name is, and related mechanisms for copying the data
 * @param ClientData Optional pointer to help clients track this request, that is returned in associated callbacks
 * @param CompletionCallback This function is called when the read operation completes
 * @return A valid Title Storage File Request handle if successful, or NULL otherwise. Data contained in the completion callback will have more detailed information about issues with the request in failure cases. This handle must be released when it is no longer needed
 *
 * @see EOS_TitleStorageFileTransferRequest_Release
 */
EOS_DECLARE_FUNC(EOS_HTitleStorageFileTransferRequest) EOS_TitleStorage_ReadFile(EOS_HTitleStorage Handle, const EOS_TitleStorage_ReadFileOptions* Options, void* ClientData, const EOS_TitleStorage_OnReadFileCompleteCallback CompletionCallback);

/**
 * Clear previously cached file data. This operation will be done asynchronously. All cached files except those corresponding to the transfers in progress will be removed.
 * Warning: Use this with care. Cache system generally tries to clear old and unused cached files from time to time. Unnecessarily clearing cache can degrade performance as SDK will have to re-download data.
 *
 * @param Options Object containing properties related to which user is deleting cache
 * @param ClientData Optional pointer to help clients track this request, that is returned in associated callbacks
 * @param CompletionCallback This function is called when the delete cache operation completes
 * @return EOS_Success if the operation was started correctly, otherwise an error result explaining what went wrong
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorage_DeleteCache(EOS_HTitleStorage Handle, const EOS_TitleStorage_DeleteCacheOptions* Options, void* ClientData, const EOS_TitleStorage_OnDeleteCacheCompleteCallback CompletionCallback);

/**
 * Below are helper functions to retrieve information about a file request handle, or to attempt to cancel a request in progress.
 */

/**
 * Get the current state of a file request.
 *
 * @return EOS_Success if complete and successful, EOS_RequestInProgress if the request is still in progress, or another state for failure.
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_GetFileRequestState(EOS_HTitleStorageFileTransferRequest Handle);

/**
 * Get the file name of the file this request is for. OutStringLength will always be set to the string length of the file name if it is not NULL.
 *
 * @param FilenameStringBufferSizeBytes The maximum number of bytes that can be written to OutStringBuffer
 * @param OutStringBuffer The buffer to write the NULL-terminated utf8 file name into, if successful
 * @param OutStringLength How long the file name is (not including null terminator)
 * @return EOS_Success if the file name was successfully written to OutFilenameBuffer, a failure result otherwise
 *
 * @see EOS_TITLESTORAGE_FILENAME_MAX_LENGTH_BYTES
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_GetFilename(EOS_HTitleStorageFileTransferRequest Handle, uint32_t FilenameStringBufferSizeBytes, char* OutStringBuffer, int32_t* OutStringLength);

/**
 * Attempt to cancel this file request in progress. This is a best-effort command and is not guaranteed to be successful if the request has completed before this function is called.
 *
 * @return EOS_Success if cancel is successful, EOS_NoChange if request had already completed (can't be canceled), EOS_AlreadyPending if it's already been canceled before (this is a final state for canceled request and won't change over time).
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_TitleStorageFileTransferRequest_CancelRequest(EOS_HTitleStorageFileTransferRequest Handle);
