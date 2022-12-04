// @ts-expect-error
import __cldr from 'cfxui-cldr-loader!./__internal/__cldr';

const cldr: {
  validLocales: Set<string>,
  languageMap: Record<string, string>,
  countryMap: Record<string, string>,
  localeDisplayName: Record<string, string>,
} = __cldr;

export const countryMap = cldr.countryMap;
export const languageMap = cldr.languageMap;
export const validLocales = cldr.validLocales;

{
  (globalThis as any).__countryMap = countryMap;
  (globalThis as any).__languageMap = languageMap;
  (globalThis as any).__validLocales = validLocales;
}
