import { makeAutoObservable, runInAction } from "mobx"
import { logger } from "utils/logger";
import { onLoadingEvent } from "utils/windowMessages";

const log = (...args) => {};
// const log = logger('GameLoadingState');

export enum InitFunctionType {
  INIT_UNKNOWN = 0,
  INIT_CORE = 1,
  INIT_BEFORE_MAP_LOADED = 2,
  INIT_AFTER_MAP_LOADED = 4,
  INIT_SESSION = 8
}

export const GameLoadingState = new class GameLoadingState {
  public loadingProgress = 0;

  constructor() {
    makeAutoObservable(this);

    onLoadingEvent('loadProgress', ({ loadFraction }) => {
      log('loadProgress', loadFraction);

      runInAction(() => {
        this.loadingProgress = loadFraction;
      });
    });

    onLoadingEvent('startInitFunction', ({ type }) => {
      log('startInitFunction', type);
    });

    onLoadingEvent('startInitFunctionOrder', ({ type, order, count }) => {
      log('startInitFunctionOrder', {
        type,
        order,
        count,
      });
    });

    onLoadingEvent('initFunctionInvoking', ({ type, name, idx }) => {
      log('initFunctionInvoking', {
        type,
        name,
        idx,
      });
    });

    onLoadingEvent('initFunctionInvoked', ({ type, name }) => {
      log('initFunctionInvoked', {
        type,
        name,
      });
    });

    onLoadingEvent('endInitFunction', ({ type }) => {
      log('endInitFunction', type);
    });

    onLoadingEvent('startDataFileEntries', ({ count }) => {
      log('startDataFileEntries', count);
    });

    onLoadingEvent('performMapLoadFunction', ({ idx }) => {
      log('performMapLoadFunction', idx);
    });

    onLoadingEvent('onDataFileEntry', ({ type, isNew, name }) => {
      log('onDataFileEntry', {
        type,
        isNew,
        name,
      });
    });

    onLoadingEvent('onLogLine', ({ message }) => {
      log('onLogLine', message);
    });
  }

  resetLoadingProgress() {
    this.loadingProgress = 0;
  }
}
