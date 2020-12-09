import { injectable } from 'inversify';

import { DiskFileSystemProvider } from '@theia/filesystem/lib/node/disk-file-system-provider';
import URI from '@theia/core/lib/common/uri';
import { createFileSystemProviderError, FileSystemProviderError, FileSystemProviderErrorCode, FileUpdateOptions, FileUpdateResult, toFileSystemProviderErrorCode } from '@theia/filesystem/lib/common/files';
import { TextDocumentContentChangeEvent } from 'vscode-languageserver-protocol';

function toFileSystemProviderError(error: NodeJS.ErrnoException): FileSystemProviderError {
  if (error instanceof FileSystemProviderError) {
      return error; // avoid double conversion
  }

  let code: FileSystemProviderErrorCode;
  switch (error.code) {
      case 'ENOENT':
          code = FileSystemProviderErrorCode.FileNotFound;
          break;
      case 'EISDIR':
          code = FileSystemProviderErrorCode.FileIsADirectory;
          break;
      case 'ENOTDIR':
          code = FileSystemProviderErrorCode.FileNotADirectory;
          break;
      case 'EEXIST':
          code = FileSystemProviderErrorCode.FileExists;
          break;
      case 'EPERM':
      case 'EACCES':
          code = FileSystemProviderErrorCode.NoPermissions;
          break;
      default:
          code = FileSystemProviderErrorCode.Unknown;
  }

  return createFileSystemProviderError(error, code);
}

@injectable()
export class FxdkDiskFileSystemProvider extends DiskFileSystemProvider {
  async updateFile(resource: URI, changes: TextDocumentContentChangeEvent[], opts: FileUpdateOptions): Promise<FileUpdateResult> {
    if (changes.length === 0) {
      try {
        const stats = await this.stat(resource);

        return {
          ...stats,
          encoding: opts.readEncoding || opts.writeEncoding || 'utf8',
        };
      } catch (e) {
        throw toFileSystemProviderError(e);
      }
    }

    return super.updateFile(resource, changes, opts);
  }
}
