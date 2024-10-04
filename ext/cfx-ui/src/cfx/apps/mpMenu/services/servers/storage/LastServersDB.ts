import * as IDB from 'idb';

import { Deferred } from 'cfx/utils/async';
import { preventDefault } from 'cfx/utils/domEvents';

export class LastServersDB {
  private initError: Error | null = null;

  private initDeferred = new Deferred<boolean>();

  /**
   * @deprecated use getConnection()
   */
  private __db__: IDB.IDBPDatabase | null = null;

  private async getConnection(): Promise<IDB.IDBPDatabase | null> {
    if (await this.initFailed()) {
      return null;
    }

    return this.__db__;
  }

  private dbDidNotExist = false;

  constructor() {
    this.init2();
  }

  // private async trx<T>(mode: 'readonly' | 'readwrite', cb: (store: IDBObjectStore, trx: IDBTransaction) => Promise<T>): Promise<T> {
  //   const connection = await this.getConnection();
  //   if (!connection) {
  //     throw new Error('No connection');
  //   }

  //   const deferred = new Deferred<T>();

  //   try {
  //     const trx = connection.transaction(Db.STORE_NAME, mode);
  //     const objectStore = trx.objectStore(Db.STORE_NAME);

  //     trx.addEventListener('abort', (event) => {
  //       const error = trx.error || new Error('Transaction aborted');

  //       console.warn(error);

  //       event.preventDefault();
  //       event.stopPropagation();

  //       deferred.reject(error);
  //     });

  //     trx.addEventListener('error', (event) => {
  //       const error = trx.error || new Error('Transaction failed');

  //       console.warn(error);

  //       event.preventDefault();
  //       event.stopPropagation();

  //       deferred.reject(error);
  //     });

  //     let retval: T;

  //     trx.addEventListener('complete', () => {
  //       deferred.resolve(retval);
  //     });

  //     try {
  //       retval = await cb(objectStore, trx);
  //       trx.commit();
  //     } catch (e) {
  //       console.warn(e);
  //       deferred.reject(e);
  //     }

  //     return deferred.promise;
  //   } catch (e) {
  //     // This can be caused by either transaction() or trx.objectStore() and if it's the latter - uh huh,
  //     // because it means that object store is not in a scope of transaction, but the way we request it makes it impossible
  //     if (e instanceof DOMException && e.name === 'NotFoundError') {
  //       this.setInitError(e);
  //     } else {
  //       console.warn(e);
  //     }

  //     deferred.reject(e);
  //     return deferred.promise;
  //   }
  // }

  private async init2() {
    try {
      this.__db__ = await IDB.openDB(Db.NAME, Db.VERSION, {
        upgrade: (database, oldVersion) => {
          this.dbDidNotExist = oldVersion < 1;

          // Create list object store
          const createListObjectStore = () => database.createObjectStore(Db.STORE_NAME, {
            keyPath: 'address',
            autoIncrement: false,
          });

          let objectStore: ReturnType<typeof createListObjectStore>;

          try {
            objectStore = createListObjectStore();
          } catch (createError) {
            if (
              createError instanceof DOMException
              && createError.name === 'ConstraintError'
              && database.objectStoreNames.contains(Db.STORE_NAME)
            ) {
              // Try recreating
              try {
                database.deleteObjectStore(Db.STORE_NAME);

                objectStore = createListObjectStore();
              } catch (e) {
                this.setInitError(e);

                return;
              }
            } else {
              this.setInitError(createError);

              return;
            }
          }

          // Create time index
          const createIndex = () => objectStore.createIndex(Db.TIME_INDEX_NAME, 'time', {
            unique: false,
            multiEntry: false,
          });

          try {
            createIndex();
          } catch (createError) {
            if (
              createError instanceof DOMException
              && createError.name === 'ConstraintError'
              && objectStore.indexNames.contains(Db.TIME_INDEX_NAME)
            ) {
              try {
                objectStore.deleteIndex(Db.TIME_INDEX_NAME);
                createIndex();
              } catch (e) {
                this.setInitError(e);
              }
            } else {
              this.setInitError(createError);
            }
          }
        },
      });
    } catch (e) {
      this.setInitError(e);
    }
  }

  private setInitError(error: Error) {
    if (this.initError) {
      return;
    }

    this.initError = error;
    this.initDeferred.resolve(false);

    console.warn(`Locally fatal error of ${Db.LOG_CLUE}, can not proceed, error:`, error);
  }

  private async initFailed(): Promise<boolean> {
    if (this.initError) {
      return true;
    }

    const success = await this.initDeferred.promise;

    return !success;
  }

  private readonly __dbErrorHandler = preventDefault(
    (event: any) => console.warn(`Unhandled ${Db.LOG_CLUE}.IDBDatabase.error event`, { event }),
  );

  private readonly __dbAbortHandler = preventDefault(
    (event: any) => console.warn(`Unhandled ${Db.LOG_CLUE}.IDBDatabase.abort event`, { event }),
  );

  private readonly __dbCloseHandler = preventDefault(
    () => this.setInitError(new Error(`${Db.LOG_CLUE} was closed`)),
  );

  private readonly __dbVersionChangeHandler = preventDefault(
    () => this.setInitError(new Error(`${Db.LOG_CLUE} version changed elsewhere`)),
  );

  // eslint-disable-next-line no-empty-function
  private async performUpgrades() {}
}

namespace Db {
  export const NAME = 'HistoryServers';
  export const VERSION = 1;
  export const STORE_NAME = 'list';
  export const TIME_INDEX_NAME = 'time';
  export const LOG_CLUE = `IDB[${Db.NAME}#${Db.VERSION}]`;
}
