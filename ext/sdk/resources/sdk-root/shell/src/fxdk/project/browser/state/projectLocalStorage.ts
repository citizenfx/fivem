import { dispose, Disposer, IDisposableObject } from "fxdk/base/disposable";
import { makeAutoObservable, reaction } from "mobx";

export interface IProjectLocalStorage {
  build?: {
    path?: string,
    useVersioning?: boolean,
    useArtifact?: boolean,
    useTxAdmin?: boolean,
  },
  steamWebApiKey?: string,
  tebexSecret?: string,
}

export class ProjectLocalStorage implements IDisposableObject {
  private storageKey: string;

  private storage: IProjectLocalStorage;

  private toDispose: Disposer;

  constructor(private path: string) {
    this.storageKey = `project://${path}`;

    try {
      this.storage = JSON.parse(localStorage.getItem(this.storageKey) || '');
    } catch (e) {
      this.storage = this.maybeMigrate();
    }

    makeAutoObservable(this);

    this.toDispose = new Disposer();

    this.toDispose.register(reaction(
      () => JSON.stringify(this.storage),
      (storageString) => {
        localStorage.setItem(this.storageKey, storageString);
      },
      {
        delay: 250,
      },
    ));
  }

  dispose() {
    dispose(this.toDispose);
  }

  //#region accessors
  get buildPath(): string {
    return this.storage.build?.path || '';
  }
  set buildPath(buildPath: string) {
    this.storage.build ??= {};
    this.storage.build.path = buildPath;
  }

  get buildUseVersioning(): boolean {
    return toBoolean(this.storage.build?.useVersioning, true);
  }
  set buildUseVersioning(useIt: boolean) {
    this.storage.build ??= {};
    this.storage.build.useVersioning = useIt;
  }

  get buildUseArtifact(): boolean {
    return toBoolean(this.storage.build?.useArtifact, true);
  }
  set buildUseArtifact(useIt: boolean) {
    this.storage.build ??= {};
    this.storage.build.useArtifact = useIt;
  }

  get buildUseTxAdmin(): boolean {
    return toBoolean(this.storage.build?.useTxAdmin, false);
  }
  set buildUseTxAdmin(useIt: boolean) {
    this.storage.build ??= {};
    this.storage.build.useTxAdmin = useIt;
  }

  get steamWebApiKey(): string {
    return this.storage.steamWebApiKey || '';
  }
  set steamWebApiKey(key: string) {
    this.storage.steamWebApiKey = key;
  }

  get tebexSecret(): string {
    return this.storage.tebexSecret || '';
  }
  set tebexSecret(key: string) {
    this.storage.tebexSecret = key;
  }
  //#endregion accessors

  private maybeMigrate(): IProjectLocalStorage {
    const getDeleting = <T>(key: string): T | undefined => {
      const composedKey = `${this.path}:${key}`;

      const value = localStorage.getItem(composedKey);
      if (value !== undefined) {
        localStorage.removeItem(composedKey);

        if (!value) {
          return undefined;
        }

        try {
          return JSON.parse(value);
        } catch (e) {
          return undefined;
        }
      }

      return value;
    };

    const storage: IProjectLocalStorage = {
      build: {
        path: getDeleting('buildPath') || '',
        useVersioning: toBoolean(getDeleting('useVersioning'), true),
        useArtifact: toBoolean(getDeleting('deployArtifact'), true),
        useTxAdmin: toBoolean(getDeleting('useTxAdmin'), false),
      },
      steamWebApiKey: getDeleting('steamWebApiKey') || '',
      tebexSecret: getDeleting('tebexSecret') || '',
    };

    localStorage.setItem(this.storageKey, JSON.stringify(storage));

    return storage;
  }
}

function toBoolean(value: boolean | undefined, defaultValue: boolean): boolean {
  if (typeof value === 'undefined') {
    return defaultValue;
  }

  return value;
}
