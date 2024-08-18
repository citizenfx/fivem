import type DEFAULT_LOCALE from 'assets/languages/locale-en.json';

export type LocaleKey = keyof typeof DEFAULT_LOCALE;

// eslint-disable-next-line @stylistic/max-len
// @see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/PluralRules/select#return_value
export enum PluralRule {
  Zero = 'zero',
  One = 'one',
  Two = 'two',
  Few = 'few',
  Many = 'many',
  Other = 'other',
}

export type PluralKeys = Partial<Record<PluralRule, Extract<LocaleKey, `@${string}`>>>;

export type LocaleKeyOrString<T> = T extends `#${string}`
  ? Extract<LocaleKey, T>
  : T extends '#DUMMY' // exclude #DUMMY key
    ? never
    : T extends `@${string}` // exclude keys for plural
      ? never
      : T extends `&${string}` // exclude keys for nl2brx version
        ? never
        : string;

export type LocaleKeyOrString_nl2br<T> = T extends `&${string}`
  ? Extract<LocaleKey, T>
  : T extends `#${string}` // exclude normal keys
    ? never
    : T extends `@${string}` // exclude keys for plural
      ? never
      : string;
