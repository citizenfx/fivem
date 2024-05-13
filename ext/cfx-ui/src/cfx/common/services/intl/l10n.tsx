/* eslint-disable react/jsx-pascal-case */
/* eslint-disable camelcase */
import { observer } from 'mobx-react-lite';

import { nl2brx } from 'cfx/utils/nl2br';

import { useIntlService } from './intl.service';
import { LocaleKeyOrString, LocaleKeyOrString_nl2br, PluralKeys } from './types';

export interface LocalizeProps<T extends string> {
  children?: undefined | void;

  id: LocaleKeyOrString<T>;
  args?: Record<string, any>;
  fallbackString?: string;
}
export const Localize = observer(function Localize<T extends string>(props: LocalizeProps<T>) {
  return useL10n(props.id, props.args, props.fallbackString) as any;
});

export interface Localize_nl2brProps<T> {
  children?: undefined | void;

  id: LocaleKeyOrString_nl2br<T>;
  args?: Record<string, any>;
}
export const Localize_nl2br = observer(function Localize_nl2br<T>(props: Localize_nl2brProps<T>) {
  return nl2brx(useL10n(props.id, props.args)) as any;
});

export interface LocalizePluralProps {
  children?: undefined | void;

  ids: PluralKeys;
  count: number;
  args?: Record<string, any>;
}
export const LocalizePlural = observer(function LocalizePlural(props: LocalizePluralProps) {
  return useL10nPlural(props.count, props.ids, props.args) as any;
});

/**
 * If you see that `key` is of type `never` - then key does not exist in `assets/locales/locale-en.json` file
 */
export function $L<T>(key: LocaleKeyOrString<T>, args?: Record<string, any>, fallbackString?: string) {
  return (
    <Localize id={key} args={args} fallbackString={fallbackString} />
  );
}

/**
 * Only accepts keys prefixed with `&` dedicated for nl2brx
 *
 * If you see that `key` is of type `never` - then key does not exist in `assets/locales/locale-en.json` file
 *
 * TODO: make `ts-loader` transformer to use this fn automagically
 */
export function $L_nl2br<T>(key: LocaleKeyOrString_nl2br<T>, args?: Record<string, any>) {
  return (
    <Localize_nl2br id={key} args={args} />
  );
}

/**
 * If you see that `key` is of type `never` - then key does not exist in `assets/locales/locale-en.json` file
 */
export function useL10n<T>(
  key: LocaleKeyOrString<T> | LocaleKeyOrString_nl2br<T>,
  args?: Record<string, any>,
  fallbackString?: string,
): string {
  return useIntlService().translate(key, args, fallbackString);
}

export function $LP(count: number, keys: PluralKeys, args?: Record<string, unknown>) {
  return (
    <LocalizePlural count={count} ids={keys} args={args} />
  );
}
export function useL10nPlural(count: number, keys: PluralKeys, args?: Record<string, unknown>): string {
  return useIntlService().translatePlural(count, keys, args);
}
