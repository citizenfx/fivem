// @ts-expect-error
// eslint-disable-next-line import/no-webpack-loader-syntax, import/no-unresolved
import __cldr from 'cfxui-cldr-loader!./__internal/__cldr';

const cldr: {
  validLocales: Set<string>;
  languageMap: Record<string, string>;
  countryMap: Record<string, string>;
  localeDisplayName: Record<string, string>;
} = __cldr;

export const {
  countryMap,
} = cldr;
export const {
  languageMap,
} = cldr;
export const {
  validLocales,
} = cldr;

// eslint-disable-next-line no-lone-blocks
{
  (globalThis as any).__countryMap = countryMap;
  (globalThis as any).__languageMap = languageMap;
  (globalThis as any).__validLocales = validLocales;
}
