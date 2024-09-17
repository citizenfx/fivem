import { GameName } from 'cfx/base/game';
import { currentGameNameIs } from 'cfx/base/gameRuntime';

import { IServerView, ServerViewDetailsLevel } from './types';

const southKoreaGRACFiveM = new URL('assets/images/GRAC-GTAV.png', import.meta.url).toString();
const southKoreaGRACRedM = new URL('assets/images/GRAC-RDR2.png', import.meta.url).toString();

export function showServerPremiumBadge(
  premium: IServerView['premium'],
): premium is NonNullable<IServerView['premium']> {
  return !!premium;
}

export function showServerCountryFlag(
  localeCountry: IServerView['localeCountry'],
): localeCountry is NonNullable<IServerView['localeCountry']> {
  const lclocaleCountry = localeCountry.toLowerCase();

  return lclocaleCountry !== 'aq' && lclocaleCountry !== '001';
}

export function getServerLegalRatingImageURL(server: IServerView): string | null {
  if (server.localeCountry === 'KR') {
    if (currentGameNameIs(GameName.RedM)) {
      return southKoreaGRACRedM;
    }
  
    return southKoreaGRACFiveM;
  }

  return null;
}

export function showServerPowers(server: IServerView): boolean {
  if (!isServerBoostable(server)) {
    return false;
  }

  return Boolean(server.upvotePower || server.burstPower);
}

export function isServerBoostable(server: IServerView): boolean {
  return !!server.joinId;
}

export function getServerDetailsLink(server: IServerView): string {
  return `/servers/detail/${server.id}`;
}

export function isServerLiveLoading(server: IServerView): boolean {
  if (server.offline) {
    return false;
  }

  return server.detailsLevel < ServerViewDetailsLevel.Live;
}

export function isServerOffline(server: IServerView): boolean {
  if (!server.connectEndPoints?.length) {
    return true;
  }

  if (server.offline) {
    return true;
  }

  if (server.fallback) {
    return true;
  }

  return false;
}
