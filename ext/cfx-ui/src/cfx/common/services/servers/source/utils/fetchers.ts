import { Deferred } from "cfx/utils/async";
import { decodeServer } from "../api/api";
import { fullServerData2ServerView, listServerData2ServerView } from "../../transformers";
import { IFullServerData, IServerView } from "../../types";
import { FrameReader } from "./frameReader";
import { fetcher } from "cfx/utils/fetcher";
import { GameName } from "cfx/base/game";

const BASE_URL = 'https://servers-frontend.fivem.net/api/servers';
const ALL_SERVERS_URL = `${BASE_URL}/streamRedir/`;
const SINGLE_SERVER_URL = `${BASE_URL}/single/`;
const TOP_SERVER_URL = `${BASE_URL}/top/`;

async function readBodyToServers(gameName: GameName, onServer: (server: IServerView) => void, body: ReadableStream<Uint8Array>): Promise<void> {
  const deferred = new Deferred<void>();

  let decodeTime = 0;
  let transformTime = 0;
  let onServerTime = 0;

  const frameReader = new FrameReader(
    body,
    (frame) => {
      let s = performance.now();
      const srv = decodeServer(frame);
      decodeTime += performance.now() - s;

      if (srv.EndPoint && srv.Data) {
        const serverGameName = srv.Data?.vars?.gamename || GameName.FiveM;

        if (gameName === serverGameName) {
          s = performance.now();
          const aaa = listServerData2ServerView(srv.EndPoint, srv.Data);
          transformTime += performance.now() - s;

          s = performance.now();
          onServer(aaa);
          onServerTime += performance.now() - s;
        }
      }

      decodeTime += performance.now() - s;
    },
    deferred.resolve,
  );

  frameReader.read();

  await deferred.promise;

  console.log('Times: decode', decodeTime, 'ms, transform', transformTime, 'ms, onServer', onServerTime, 'ms');
}

export async function getAllServers(gameName: GameName, onServer: (server: IServerView) => void): Promise<void> {
  console.time('Total getAllServers');

  const { body } = await fetcher.fetch(new Request(ALL_SERVERS_URL));
  if (!body) {
    console.timeEnd('Total getAllServers');
    throw new Error('Empty body of all servers stream');
  }

  await readBodyToServers(gameName, onServer, body);

  console.timeEnd('Total getAllServers');
}

export async function getSingleServer(gameName: GameName, address: string): Promise<IServerView | null> {
  try {
    const srv: IFullServerData = await fetcher.json(SINGLE_SERVER_URL + address);

    if (srv.EndPoint && srv.Data) {
      const serverGameName = srv.Data?.vars?.gamename || GameName.FiveM;

      if (gameName === serverGameName) {
        return fullServerData2ServerView(srv.EndPoint, srv.Data);
      }
    }

    return null;
  } catch (e) {
    return null;
  }
}

export interface TopServerConfig {
  language: string,
  gameName?: GameName,
}
interface TopServerResponse {
  EP: string,
  Data: IFullServerData,
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

      return fullServerData2ServerView(srv.EndPoint, srv.Data);
    }

    return null;
  } catch (e) {
    console.error(e);

    return null;
  }
}


try {
  (window as any).__getSingleServer = getSingleServer;
} catch (e) {}
