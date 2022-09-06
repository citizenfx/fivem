// @ts-expect-error
import __locales from 'cfxui-locales-loader!./__internal/__translations';

export const locales: typeof import('./__internal/__translations') = __locales;
export const defaultLocale = __locales['en'];
