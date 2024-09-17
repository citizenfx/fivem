import { GameName } from 'cfx/base/game';
import { Deferred } from 'cfx/utils/async';
import { fetcher } from 'cfx/utils/fetcher';

import { FrameReader } from './frameReader';
import { masterListFullServerData2ServerView, masterListServerData2ServerView } from '../../transformers';
import { IFullServerData, IServerView } from '../../types';
import { decodeServer } from '../api/api';

const BASE_URL = 'https://servers-frontend.fivem.net/api/servers';
const ALL_SERVERS_URL = `${BASE_URL}/streamRedir/`;
const SINGLE_SERVER_URL = `${BASE_URL}/single/`;
const TOP_SERVER_URL = `${BASE_URL}/top/`;

async function readBodyToServers(
  gameName: GameName,
  onServer: (server: IServerView) => void,
  body: ReadableStream<Uint8Array>,
): Promise<void> {
  const deferred = new Deferred<void>();

  let decodeTime = 0;
  let transformTime = 0;
  let onServerTime = 0;

  const frameReader = new FrameReader(
    body,
    (frame) => {
      let timestamp = performance.now();
      const srv = decodeServer(frame);
      decodeTime += performance.now() - timestamp;

      if (srv.EndPoint && srv.Data) {
        const serverGameName = srv.Data?.vars?.gamename || GameName.FiveM;

        if (gameName === serverGameName) {
          timestamp = performance.now();
          const serverView = masterListServerData2ServerView(srv.EndPoint, srv.Data);
          transformTime += performance.now() - timestamp;

          timestamp = performance.now();
          onServer(serverView);
          onServerTime += performance.now() - timestamp;
        }
      }

      decodeTime += performance.now() - timestamp;
    },
    deferred.resolve,
  );

  frameReader.read();

  await deferred.promise;

  console.log('Times: decode', decodeTime, 'ms, transform', transformTime, 'ms, onServer', onServerTime, 'ms');
}

export async function getAllMasterListServers(
  gameName: GameName,
  onServer: (server: IServerView) => void,
): Promise<void> {
  console.time('Total getAllServers');

  const {
    body,
  } = await fetcher.fetch(new Request(ALL_SERVERS_URL));

  if (!body) {
    console.timeEnd('Total getAllServers');
    throw new Error('Empty body of all servers stream');
  }

  await readBodyToServers(gameName, onServer, body);

  console.timeEnd('Total getAllServers');
}

export async function getMasterListServer(gameName: GameName, address: string): Promise<IServerView | null> {
  try {
    const srv: IFullServerData = await fetcher.json(SINGLE_SERVER_URL + address);

    if (srv.EndPoint && srv.Data) {
      const serverGameName = srv.Data?.vars?.gamename || GameName.FiveM;

      if (gameName === serverGameName) {
        return masterListFullServerData2ServerView(srv.EndPoint, srv.Data);
      }
    }

    return null;
  } catch (e) {
    return null;
  }
}

export interface TopServerConfig {
  language: string;
  gameName?: GameName;
}
interface TopServerResponse {
  EP: string;
  Data: IFullServerData;
}
export async function getTopServer(config: TopServerConfig): Promise<IServerView | null> {
  try {
    const response: TopServerResponse = await fetcher.json(TOP_SERVER_URL + config.language);
    const srv = response.Data;

    if (srv.EndPoint && srv.Data) {
      const serverGameName = srv.Data?.vars?.gamename || GameName.FiveM;

      if (config.gameName) {
        if (serverGameName !== config.gameName) {
          return null;
        }
      }

      return masterListFullServerData2ServerView(srv.EndPoint, srv.Data);
    }

    return null;
  } catch (e) {
    console.error(e);

    return null;
  }
}

try {
  (window as any).__getSingleServer = getMasterListServer;
} catch (e) {
  // Do nothing
}
