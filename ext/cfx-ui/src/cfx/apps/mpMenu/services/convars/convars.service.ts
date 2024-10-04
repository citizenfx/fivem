import { injectable } from 'inversify';
import { makeAutoObservable } from 'mobx';

import { defineService, ServicesContainer, useService } from 'cfx/base/servicesContainer';
import { Deferred } from 'cfx/utils/async';

import { IConvar } from './types';
import { mpMenu } from '../../mpMenu';

export const IConvarService = defineService<IConvarService>('ConvarService');
export type IConvarService = ConvarService;

export function useConvarService() {
  return useService(IConvarService);
}

export function useStreamerMode(): boolean {
  return useConvarService().getBoolean(KnownConvars.streamerMode);
}

export function registerConvarService(container: ServicesContainer) {
  container.registerImpl(IConvarService, ConvarService);
}

@injectable()
export class ConvarService {
  private _convars: Record<IConvar, string> = {};

  private _convarsPopulatedDeferred = new Deferred<void>();

  constructor() {
    makeAutoObservable(this);

    mpMenu.on('convarSet', this.handleConvarSet);
    mpMenu.on('convarsSet', this.handleInitialConvarsSet);

    mpMenu.invokeNative('getConvars');

    this.runLocalStorageMigrations();
  }

  whenPopulated(): Promise<void> {
    return this._convarsPopulatedDeferred.promise;
  }

  getAll(): Record<IConvar, string> {
    return this._convars;
  }

  get(convar: IConvar): string {
    return this._convars[convar] || '';
  }

  getBoolean(convar: IConvar): boolean {
    return this.get(convar) === 'true';
  }

  set(convar: IConvar, value: string) {
    const payload = JSON.stringify({
      name: convar,
      value,
    });

    if (ARCHIVED_CONVARS[convar]) {
      mpMenu.invokeNative('setArchivedConvar', payload);
    } else {
      mpMenu.invokeNative('setConvar', payload);
    }
  }

  setBoolean(convar: IConvar, value: boolean) {
    this.set(convar, value
      ? 'true'
      : 'false');
  }

  private readonly handleConvarSet = (data: { name?: string; value?: string }) => {
    const {
      name,
      value,
    } = data;

    if (!name) {
      return;
    }

    this.setConvar(name, value || '');
  };

  private readonly handleInitialConvarsSet = (data: { vars: Array<{ key: string; value: string }> }) => {
    const {
      vars,
    } = data;

    if (!Array.isArray(vars)) {
      return;
    }

    for (const {
      key,
      value,
    } of vars) {
      this._convars[key] = value || '';
      this.setConvar(key, value);
    }

    this._convarsPopulatedDeferred.resolve();
  };

  private setConvar(convar: IConvar, value: string) {
    this._convars[convar] = value;
  }

  private runLocalStorageMigrations() {
    // Dark color scheme
    if (window.localStorage.getItem('darkThemeNew') !== null) {
      try {
        const useLightColorScheme = window.localStorage.getItem('darkThemeNew') === 'no';

        window.localStorage.removeItem('darkThemeNew');

        this.setBoolean('ui_preferLightColorScheme', useLightColorScheme);
      } catch (e) {
        // Do nothing
      }
    }
  }
}

export namespace KnownConvars {
  export const streamerMode = 'ui_streamerMode';
  export const localhostPort = 'ui_quickAccessLocalhostPort';
  export const updateChannel = 'ui_updateChannel';
  export const customBackdrop = 'ui_customBackdrop';
  export const preferLightColorScheme = 'ui_preferLightColorScheme';
  export const preferBlurredBackdrop = 'ui_preferBlurredBackdrop';
}

const ARCHIVED_CONVARS: Record<IConvar, boolean> = [
  KnownConvars.streamerMode,
  KnownConvars.customBackdrop,
  KnownConvars.preferLightColorScheme,
  KnownConvars.preferBlurredBackdrop,
].reduce((acc, convar) => ({
  ...acc,
  [convar]: true,
}), {});
