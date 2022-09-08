import { IServerView } from "cfx/common/services/servers/types";

export function showServerPremiumBadge(premium: IServerView['premium']): premium is NonNullable<IServerView['premium']> {
  return !!premium;
}

export function showServerCountryFlag(localeCountry: IServerView['localeCountry']): localeCountry is NonNullable<IServerView['localeCountry']> {
  const lclocaleCountry = localeCountry.toLowerCase();

  return lclocaleCountry !== 'aq' && lclocaleCountry !== '001';
}
