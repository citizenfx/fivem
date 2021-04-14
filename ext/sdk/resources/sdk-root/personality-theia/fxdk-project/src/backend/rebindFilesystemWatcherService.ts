import { interfaces } from 'inversify';
import { FileSystemWatcherService } from '@theia/filesystem/lib/common/filesystem-watcher-protocol';
import { createNsfwFileSystemWatcherService } from '@theia/filesystem/lib/node/filesystem-backend-module';

export const rebindFilesystemWatcherService = (bind: interfaces.Bind, rebind: interfaces.Rebind) => {
  rebind<FileSystemWatcherService>(FileSystemWatcherService).toDynamicValue((ctx) => {
    return createNsfwFileSystemWatcherService(ctx);
  }).inSingletonScope();
};
