import { ISettings } from './types';

export const ISettingsServiceInit = Symbol('SettingsServiceInit');
export interface ISettingsServiceInit {
  settings: ISettings;
  defaultSettingsCategoryId: string;
}
