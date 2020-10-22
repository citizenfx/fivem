import { DiskFileSystemProvider } from '@theia/filesystem/lib/node/disk-file-system-provider';
import URI from '@theia/core/lib/common/uri';
import { FileUpdateOptions, FileUpdateResult } from '@theia/filesystem/lib/common/files';
import { TextDocumentContentChangeEvent } from 'vscode-languageserver-protocol';
export declare class FxdkDiskFileSystemProvider extends DiskFileSystemProvider {
    updateFile(resource: URI, changes: TextDocumentContentChangeEvent[], opts: FileUpdateOptions): Promise<FileUpdateResult>;
}
//# sourceMappingURL=fxdk-disk-file-system-provider.d.ts.map