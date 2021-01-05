export enum ResourceManifestKind {
  none,
  __resource,
  fxmanifest,
}

export type ResourceManifestRecord = [string, unknown][];

export const ResourceManifestGame = {
  gta5: 'gta5',
  rdr3: 'rdr3',
  common: 'common',
};

export const ResourceManifestVersion = {
  /**
   * Requires `game` to be specified, and is mandatory for RedM.
   */
  adamant: 'adamant',

  /**
   * Implies `clr_disable_task_scheduler` being specified for server library compatibility.
   * Does not define window in JS contexts for library compatibility.
   */
  bodacious: 'bodacious',

  /**
   * Requires `https://` callbacks in NUI/DUI.
   * Adds WASM and `fetch` support.
   */
  cerulean: 'cerulean',
};

export class ResourceManifest {
  fxVersion: string = ResourceManifestVersion.bodacious;
  games: string[] = [ResourceManifestGame.gta5];

  files: string[] = [];

  clientScripts: string[] = [];
  serverScripts: string[] = [];
  sharedScripts: string[] = [];

  author: string = 'You';
  version: string = '1.0.0';
  description: string = '';

  uiPage: string = '';
  loadScreen: string = '';

  /**
   * Marks the current resource as a replacement for the specified resource.
   * This means it'll start instead of the specified resource,
   * if another resource requires it,
   * and will act as if it is said resource if started.
   */
  provide: string = '';

  exports: string[] = [];
  serverExports: string[] = [];

  /**
   * @deprecated use data files instead
   */
  beforeLevelMeta: string = '';

  /**
   * @deprecated use data files instead
   */
  afterLevelMeta: string = '';

  replaceLevelMeta: string = '';

  dataFiles: ResourceManifestRecord = [];

  /**
   * Marks this resource as being a GTA map, and reloads the map storage when the resource gets loaded.
   */
  isMap: boolean = false;

  /**
   * Marks the resource as being server-only. This stops clients from downloading anything of this resource.
   */
  serverOnly: boolean = false;

  /**
   * Requires the specified resource to load before the current resource.
   */
  dependencies: string[] = [];

  /**
   * Enables Lua 5.4
   *
   * @see http://www.lua.org/manual/5.4/manual.html
   */
  lua54: boolean = false;

  /**
   * By default, lazy loading of native functions is enabled to drastically reduce resource memory usage.
   * While not recommended, you can set this option to any value to disable lazy loading.
   */
  disableLazyNatives: boolean = false;

  /**
   * When present, disables the custom C# task scheduler on the server.
   * This will increase compatibility with third-party libraries using the .NET TPL,
   * but make it more likely you'll have to await Delay(0); to end up back on the main thread.
   */
  clrDisableTaskScheduler: boolean = false;

  fxdkWatchCommands: ResourceManifestRecord = [];
  fxdkBuildCommands: ResourceManifestRecord = [];

  getFiles(): string[] {
    return [...new Set([
      ...this.files,
      this.uiPage,
      this.loadScreen,
    ].filter(Boolean))];
  }

  getAllScripts(): string[] {
    return [
      ...this.clientScripts,
      ...this.serverScripts,
      ...this.sharedScripts,
    ];
  }

  toString(): string {
    const nl = '';

    const lines = [
      metaFromArrayOrString('fx_version', this.fxVersion),
      metaFromArrayOrString('game', this.games),
      nl,
      metaFromArrayOrString('dependency', this.dependencies),
      metaFromArrayOrString('provide', this.provide),
      nl,
      metaFromArrayOrString('author', this.author),
      metaFromArrayOrString('version', this.version),
      metaFromArrayOrString('description', this.description),
      nl,
      ...metaFromRecord('fxdk_watch_command', this.fxdkWatchCommands),
      ...metaFromRecord('fxdk_build_command', this.fxdkBuildCommands),
      nl,
      metaFromArrayOrString('export', this.exports),
      metaFromArrayOrString('server_export', this.serverExports),
      nl,
      metaFromArrayOrString('file', this.getFiles()),
      nl,
      metaFromArrayOrString('client_script', this.clientScripts),
      metaFromArrayOrString('server_script', this.serverScripts),
      metaFromArrayOrString('shared_script', this.sharedScripts),
      nl,
      metaFromArrayOrString('ui_page', this.uiPage),
      metaFromArrayOrString('loadscreen', this.loadScreen),
      nl,
      metaFromArrayOrString('before_level_meta', this.beforeLevelMeta),
      metaFromArrayOrString('after_level_meta', this.afterLevelMeta),
      metaFromArrayOrString('replace_level_meta', this.replaceLevelMeta),
      nl,
      ...metaFromRecord('data_file', this.dataFiles),
      nl,
      metaFromBoolean('lua54', this.lua54),
      metaFromBoolean('server_only', this.serverOnly),
      metaFromBoolean('this_is_a_map', this.isMap),
      metaFromBoolean('disable_lazy_natives', this.disableLazyNatives),
      metaFromBoolean('clr_disable_task_scheduler', this.clrDisableTaskScheduler),
    ];

    return lines
      .filter((line) => line !== null)
      .filter((line, i, a) => {
        if (i > 0 && line === nl && a[i-1] === nl) {
          return false;
        }

        return true;
      })
      .join('\n');
  }

  fromObject(obj: any) {
    this.fxVersion = stringFromMeta(obj, 'fx_version', ResourceManifestVersion.bodacious);
    this.games = arrayFromMeta(obj, 'game', [ResourceManifestGame.gta5]);

    this.dependencies = arrayFromMeta(obj, 'dependency', []);
    this.provide = stringFromMeta(obj, 'provide', '');

    this.author = stringFromMeta(obj, 'author', 'you');
    this.version = stringFromMeta(obj, 'version', '1.0.0');
    this.description = stringFromMeta(obj, 'description', '');

    this.fxdkWatchCommands = recordFromMeta(obj, 'fxdk_watch_command', []);
    this.fxdkBuildCommands = recordFromMeta(obj, 'fxdk_build_command', []);

    this.exports = arrayFromMeta(obj, 'export', []);
    this.serverExports = arrayFromMeta(obj, 'server_export', []);

    this.files = arrayFromMeta(obj, 'file', []);

    this.clientScripts = arrayFromMeta(obj, 'client_script', []);
    this.serverScripts = arrayFromMeta(obj, 'server_script', []);
    this.sharedScripts = arrayFromMeta(obj, 'shared_script', []);

    this.uiPage = stringFromMeta(obj, 'ui_page', '');
    this.loadScreen = stringFromMeta(obj, 'loadscreen', '');

    this.beforeLevelMeta = stringFromMeta(obj, 'before_level_meta', '');
    this.afterLevelMeta = stringFromMeta(obj, 'after_level_meta', '');
    this.replaceLevelMeta = stringFromMeta(obj, 'replace_level_meta', '');

    this.dataFiles = recordFromMeta(obj, 'data_file', []);

    this.lua54 = booleanFromMeta(obj, 'lua54', false);
    this.serverOnly = booleanFromMeta(obj, 'server_only', false);
    this.isMap = booleanFromMeta(obj, 'this_is_a_map', false);
    this.disableLazyNatives = booleanFromMeta(obj, 'disable_lazy_natives', false);
    this.clrDisableTaskScheduler = booleanFromMeta(obj, 'clr_disable_task_scheduler', false);
  }
}

function booleanFromMeta(meta: any, key: string, defaultValue: boolean): boolean {
  if (typeof meta[key] === undefined) {
    return defaultValue;
  }

  return !!meta[key]?.[0];
}

function stringFromMeta(meta: any, key: string, defaultValue: string): string {
  return meta[key]?.[0] || defaultValue;
}

function arrayFromMeta(meta: any, key: string, defaultValue: string[]): string[] {
  return meta[key] || defaultValue;
}

function recordFromMeta(meta: any, key: string, defaultValue: ResourceManifestRecord): ResourceManifestRecord {
  if (!meta[key]) {
    return defaultValue;
  }

  return meta[key].map((vkey, index) => {
    return [vkey, JSON.parse(meta[`${key}_extra`]?.[index] || `''`)];
  });
}

function pluralKey(key: string): string {
  return key === 'dependency'
    ? 'dependencies'
    : key + 's';
}

function metaFromRecord(key: string, record: ResourceManifestRecord): string[] {
  return record.map(([subKey, value]) => `${key} '${subKey}' ${serializeToLua(value)}`);
}

function metaFromBoolean<T>(key: string, value: T) {
  if (value) {
    return `${key} 'yes'`;
  }

  return null;
}

function metaFromArrayOrString(key: string, entries: string[] | string) {
  if (entries === '') {
    return null;
  }

  if (!Array.isArray(entries)) {
    return `${key} '${entries}'`;
  }

  if (entries.length === 0) {
    return null;
  }

  if (entries.length === 1) {
    return `${key} '${entries[0]}'`;
  }

  const entriesString = entries.filter(Boolean).map((entry) => `  '${entry}'`).join(',\n');

  return `${pluralKey(key)} {\n${entriesString}\n}`;
}

function serializeToLua(val: unknown, indent = 2): string {
  switch (typeof val) {
    case 'number':
    case 'boolean':
    case 'symbol':
    case 'bigint': return val.toString();

    case 'string': return `'${val}'`;
    case 'function': return '';
    case 'undefined': 'nil';

    case 'object': {
      if (val === null) {
        return 'nil';
      }

      if (Array.isArray(val)) {
        return `{${val.map((subVal) => serializeToLua(subVal)).join(', ')}}`;
      }

      const indentString = Array(indent).fill(' ').join('');
      const indentString2 = Array(indent - 2).fill(' ').join('');

      const objEntries = Object.entries(val).map(([key, subVal]) => {
        return `${indentString}['${key}'] = ${serializeToLua(subVal, indent + 2)}`;
      }).join(',\n');

      return `{\n${objEntries}\n${indentString2}}`;
    }
  }
}
