// @ts-expect-error
// eslint-disable-next-line import/no-webpack-loader-syntax, import/no-unresolved
import __locales from 'cfxui-locales-loader!./__internal/__translations';

export const locales: typeof import('./__internal/__translations') = __locales;
// eslint-disable-next-line dot-notation
export const defaultLocale = __locales['en'];
