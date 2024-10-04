import { defineService, useService } from 'cfx/base/servicesContainer';

import { PluralKeys } from './types';

export function useIntlService(): IIntlService {
  return useService(IIntlService);
}

export const IIntlService = defineService<IIntlService>('IntlService');

export interface IIntlService {
  readonly systemLocale: string;
  readonly systemLocaleCountry: string;

  readonly localeCode: string;
  readonly localeTranslations: Record<string, Function>;

  readonly localesOptions: { label: string; value: string }[];

  readonly defaultDisplayNames: Intl.DisplayNames;
  readonly displayNames: Intl.DisplayNames;

  readonly defaultPluralRules: Intl.PluralRules;
  readonly pluralRules: Intl.PluralRules;

  setLocale(locale: string): void;

  translate(key: string, args?: Record<string, unknown>, fallbackString?: string): string;
  translatePlural(count: number, keys: PluralKeys, args?: Record<string, unknown>): string;
}
