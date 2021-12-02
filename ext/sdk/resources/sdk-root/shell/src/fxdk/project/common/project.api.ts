import { defineApiEndpoints } from "shared/api.protocol";
import { ProjectVariableSetter } from "./project.types";

export namespace ProjectApi {
  export const LoaderEndpoints = defineApiEndpoints('projectLoader', [
    'checkCreateRequest',
    'checkCreateResult',
    'checkOpenRequest',
    'create',
    'open',
    'close',

    'getRecents',
    'recents',
    'removeRecent',
  ]);

  export const ManifestEndpoints = defineApiEndpoints('projectManifest', [
    'update',

    'setServerUpdateChannel',
    'setSystemResources',

    'setVariable',
    'deleteVariable',
  ]);
  export namespace ManifestRequests {
    export interface SetVariable {
      variable: string,
      setter: ProjectVariableSetter,
      value: string | number,
    }
  }

  export const BuilderEndpoints = defineApiEndpoints('projectBuilder', [
    'build',
    'error',
  ]);

  export const FsEndpoints = defineApiEndpoints('projectFs', [
    'update',
    'shallowScanChildren',

    'createFile',
    'createDirectory',
    'renameEntry',
    'entryRenamed',
    'moveEntry',
    'deleteEntry',
    'entryDeleted',
    'copyEntry',
    'copyEntries',

    'pathsStateUpdate',
    'setPathsStatePatch',
  ]);

  export const AssetEndpoints = defineApiEndpoints('projectAssets', [
    'setAssetConfig',

    'setAssetRuntimeData',
    'deleteAssetRuntimeData',
  ]);

  export const ServerEndpoints = defineApiEndpoints('projectServer', [
    'start',
    'stop',
  ]);
}
