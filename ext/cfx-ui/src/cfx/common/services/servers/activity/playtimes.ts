import { TCFXID } from 'cfx/base/identifiers';
import { fetcher } from 'cfx/utils/fetcher';
import { isObject } from 'cfx/utils/object';

import { IServerActivityUserPlaytime } from './types';

type TResponsePlaytime = {
  id: TCFXID;
  seconds: number;
};

export function formatPlaytime(seconds: number): string {
  return `${(seconds / 3600).toFixed(1)} hours on record`;
}

export async function loadPlaytimes(
  serverId: string,
  cfxIds: TCFXID[],
): Promise<Record<TCFXID, IServerActivityUserPlaytime>> {
  try {
    const searchParams = new URLSearchParams(cfxIds.map((cfxId) => ['identifiers[]', cfxId]));

    const response: Array<unknown> = await fetcher.json(
      `${__CFXUI_CNL_ENDPOINT__}api/ticket/playtimes/${serverId}?${searchParams}`,
    );

    if (!Array.isArray(response)) {
      return {};
    }

    const serverPlaytimes = {};

    for (const playtime of response) {
      if (!isObject<TResponsePlaytime>(playtime)) {
        continue;
      }

      serverPlaytimes[playtime.id] = {
        cfxId: playtime.id,
        seconds: playtime.seconds,

        formattedSeconds: playtime.seconds > 0
          ? formatPlaytime(playtime.seconds)
          : 'never played',
      };
    }

    return serverPlaytimes;
  } catch (e) {
    console.error(e);

    return {};
  }
}
