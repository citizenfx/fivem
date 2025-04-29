import { defineService } from "../../../base/servicesContainer";
import type { TSettingsService, TSettingsUIService } from "./settings.common";

export const ISettingsService = defineService<ISettingsService>('SettingsService');
export type ISettingsService = TSettingsService;

export const ISettingsUIService = defineService<ISettingsUIService>('SettingsUIService');
export type ISettingsUIService = TSettingsUIService;
