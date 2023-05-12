import { ServicesContainer } from "cfx/base/servicesContainer";
import { IIntlService } from "cfx/common/services/intl/intl.service";
import { defaultLocale, locales } from "cfx/common/services/intl/resources/translations";
import { PluralRule } from "cfx/common/services/intl/types";
import { injectable } from "inversify";
import { makeAutoObservable } from "mobx";
import { mpMenu } from "../../mpMenu";

const defaultDisplayNames = new Intl.DisplayNames('en', {
  type: 'language',
});
const defaultPluralRules = new Intl.PluralRules('en');

export function registerMpMenuIntlService(container: ServicesContainer) {
  container.registerImpl(IIntlService, MpMenuIntlService);
}

@injectable()
class MpMenuIntlService implements IIntlService {
  readonly systemLocale = (() => {
    const systemLocale = mpMenu.systemLanguages[0] || 'en-US';
    const [language, country] = systemLocale.split('-');

    if (!country) {
      // Windows has some locales such as `pl` which should expand to `pl-PL`
      return `${language.toLowerCase()}-${language.toUpperCase()}`;
    }

    return systemLocale;
  })();
  readonly systemLocaleCountry = (() => {
    const [_, country] = this.systemLocale.split('-');

    return country.toUpperCase();
  })();

  readonly localesOptions = Object.keys(locales).map((locale) => ({
    label: defaultDisplayNames.of(locale) || 'en',
    value: locale,
  }));

  readonly defaultDisplayNames = defaultDisplayNames;
  readonly defaultPluralRules = defaultPluralRules;

  private _localeCode: string = getInitialLocaleCode();
  public get localeCode(): string { return this._localeCode }
  private set localeCode(locale: string) { this._localeCode = locale }

  private _localeTranslations: Record<string, Function> = locales[this.localeCode];
  public get localeTranslations(): Record<string, Function> { return this._localeTranslations }
  private set localeTranslations(localeTranslations: Record<string, Function>) { this._localeTranslations = localeTranslations }

  private _displayNames: Intl.DisplayNames = defaultDisplayNames;
  public get displayNames(): Intl.DisplayNames { return this._displayNames }
  private set displayNames(displayNames: Intl.DisplayNames) { this._displayNames = displayNames }

  private _pluralRules: Intl.PluralRules = defaultPluralRules;
  public get pluralRules(): Intl.PluralRules { return this._pluralRules }
  private set pluralRules(pluralRules: Intl.PluralRules) { this._pluralRules = pluralRules }

  constructor() {
    makeAutoObservable(this);
  }

  readonly setLocale = (locale: string) => {
    if (!locales[locale]) {
      return;
    }

    this.localeCode = locale;
    this.localeTranslations = locales[locale];
    this.pluralRules = getPluralRules(locale);
    this.displayNames = getDisplayNames(locale);

    window.localStorage.setItem('language', locale);
  };

  translate(key: string, args?: Record<string, unknown> | undefined): string {
    return this.getKeyInLocale(key)(args);
  }

  translatePlural(count: number, keys: Partial<Record<PluralRule, string>>, args?: Record<string, unknown> | undefined): string {
    let pickedKey = keys[this.pluralRules.select(count)] || keys[PluralRule.Other] || Object.values(keys)[0];

    return this.translate(pickedKey, {
      ...args,
      count,
    });
  }

  private getKeyInLocale(key: string): Function {
    if (this.localeTranslations[key]) {
      return this.localeTranslations[key];
    }

    if (defaultLocale[key]) {
      return defaultLocale[key];
    }

    return () => key;
  }
}

function getInitialLocaleCode(): string {
  let locale = window.localStorage.getItem('language')?.toLowerCase();
  if (locale && locales[locale]) {
    return locale;
  }

  const [mainLanguage] = mpMenu.systemLanguages || [];
  if (!mainLanguage || typeof mainLanguage !== 'string') {
    return 'en';
  }

  const [mainLanguageCode] = mainLanguage.toLowerCase().split('-');
  if (locales[mainLanguageCode]) {
    return mainLanguageCode;
  }

  return 'en';
}

function getDisplayNames(lang: string): Intl.DisplayNames {
  try {
    return new Intl.DisplayNames(lang, {
      type: 'language',
    });
  } catch (e) {
    console.error(e);
    return defaultDisplayNames;
  }
}

function getPluralRules(lang: string): Intl.PluralRules {
  try {
    return new Intl.PluralRules(lang);
  } catch (e) {
    console.error(e);
    return defaultPluralRules;
  }
}
