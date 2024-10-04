import { NavList } from '@cfx-dev/ui-components';
import { inject, injectable } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';

import { ISettingsService, ISettingsUIService } from './settings.service';
import { ISettingsServiceInit } from './settingsInit';
import { ICategory, ISettings } from './types';
import { ServicesContainer } from '../../../base/servicesContainer';

type NavListItems = React.ComponentProps<typeof NavList>['items'];

export type TSettingsService = SettingsService;
export type TSettingsUIService = SettingsUIService;

export function registerSettingsService(container: ServicesContainer, init: ISettingsServiceInit) {
  container.registerConstant(ISettingsServiceInit, init);

  container.registerImpl(ISettingsService, SettingsService);
  container.registerImpl(ISettingsUIService, SettingsUIService);
}

@injectable()
class SettingsService {
  private _settings: ISettings = new Map();
  public get settings(): ISettings {
    return this._settings;
  }
  private set settings(settings: ISettings) {
    this._settings = settings;
  }

  public readonly defaultSettingsCategoryId: string;

  constructor(
    @inject(ISettingsServiceInit)
    initObject: ISettingsServiceInit,
  ) {
    this.settings = initObject.settings;
    this.defaultSettingsCategoryId = initObject.defaultSettingsCategoryId;

    makeAutoObservable(this, {
      // @ts-expect-error
      _settings: observable.ref,
    });
  }
}

@injectable()
class SettingsUIService {
  private _visible: boolean = false;
  public get visible(): boolean {
    return this._visible;
  }
  private set visible(visible: boolean) {
    this._visible = visible;
  }

  private _categoryID: string = '';
  public get categoryID(): string {
    return this._categoryID;
  }
  private set categoryID(category: string) {
    this._categoryID = category;
  }

  public get category(): ICategory | undefined {
    return this.settingsService.settings.get(this.categoryID);
  }

  public get navListItems(): NavListItems {
    return [...this.settingsService.settings.entries()].map(([id, category]) => ({
      id,
      icon: category.icon,
      label: category.label,
    }));
  }

  constructor(
    @inject(ISettingsService)
    protected readonly settingsService: ISettingsService,
  ) {
    this.categoryID = settingsService.defaultSettingsCategoryId;

    makeAutoObservable(this);
  }

  readonly open = (category?: string) => {
    if (category) {
      this.categoryID = category;
    }

    this.visible = true;
  };

  readonly close = () => {
    this.visible = false;
  };

  readonly selectCategory = (category: string) => {
    this.categoryID = category;
  };
}
