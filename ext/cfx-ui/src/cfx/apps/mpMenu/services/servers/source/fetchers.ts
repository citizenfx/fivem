import { mpMenu } from "cfx/apps/mpMenu/mpMenu";
import { CurrentGameName } from "cfx/base/gameRuntime";
import { GameName } from "cfx/base/game";
import { getSingleServer } from "cfx/common/services/servers/source/utils/fetchers";
import { IServerView } from "cfx/common/services/servers/types";
import { isJoinServerAddress, parseServerAddress } from "cfx/common/services/servers/serverAddressParser";
import { resolveOrTimeout } from "cfx/utils/async";
import { fetcher } from "cfx/utils/fetcher";
import { dynamicServerData2ServerView, queriedServerData2ServerView } from "./transformers";
import { IDynamicServerData, IQueriedServerData } from "./types";


export async function getServerByAnyMean(gameName: GameName, address: string): Promise<IServerView | null> {
  // First parse address
  const parsedAddress = parseServerAddress(address);
  if (!parsedAddress) {
    console.log('[getServerByAnyMean] failed to parse address', address);
    return null;
  }

  // Best-case scenario, it's a listed server
  if (isJoinServerAddress(parsedAddress)) {
    console.log('[getServerByAnyMean] loading single server', address);
    return getSingleServer(gameName, parsedAddress.address);
  }

  const joinId = await getJoinIdFromEndpoint(parsedAddress.address);
  if (joinId) {
    console.log('[getServerByAnyMean] resolved server id', joinId);
    const server = await getSingleServer(gameName, joinId);
    if (server) {
      console.log('[getServerByAnyMean] loaded server by id', server);

      server.manuallyEnteredEndPoint = parsedAddress.address;
      return server;
    }
  }

  const queriedServerData = await getQueriedDataFromEndpoint(parsedAddress.address);
  if (queriedServerData) {
    console.log('[getServerByAnyMean] loaded queried server data', queriedServerData);
    return queriedServerData2ServerView(parsedAddress.address, queriedServerData);
  }

  const dynamicServerData = await getDynamicServerDataFromEndpoint(parsedAddress.address);
  if (!dynamicServerData) {
    console.log('[getServerByAnyMean] failed to load dynamic server data', address);
    return null;
  }

  console.log('[getServerByAnyMean] loaded server by dynamic data', dynamicServerData);
  const server = dynamicServerData2ServerView(parsedAddress.address, dynamicServerData);

  server.manuallyEnteredEndPoint = parsedAddress.address;
  return server;
}

async function getJoinIdFromEndpoint(endpoint: string): Promise<string | null> {
  try {
    const serverID = await resolveOrTimeout(
      5000,
      'https://nui-internal/gsclient/url timed out',
      fetcher.text('https://nui-internal/gsclient/url', {
        method: 'POST',
        body: `url=${endpoint}`,
      }),
    );

    if (serverID) {
      return serverID;
    }

    return null;
  } catch (e) {
    console.error(e);

    return null;
  }
}

export async function getDynamicServerDataFromEndpoint(endpoint: string): Promise<IDynamicServerData | null> {
  try {
    const res = await resolveOrTimeout(
      5000,
      'https://nui-internal/gsclient/dynamic timed out',
      fetcher.json('https://nui-internal/gsclient/dynamic', {
        method: 'POST',
        body: `url=${endpoint}`,
      }),
    );

    if (res) {
      return res;
    }

    return null;
  } catch (e) {
    console.error(e);

    return null;
  }
}

export async function getQueriedDataFromEndpoint(endpoint: string): Promise<IQueriedServerData | null> {
  try {
    return await mpMenu.queryServer(endpoint);
  } catch (e) {
    return null;
  }
}

export async function getLocalhostServerInfo(port: string): Promise<IQueriedServerData & { address: string } | null> {
  const response = await mpMenu.queryServer(`localhost_sentinel:${port}`) as IQueriedServerData & { address: string };

  const serverGameName = response.infoBlob?.vars?.gamename;
  if (serverGameName && serverGameName !== CurrentGameName) {
    return null;
  }

  return response;
}

try {
  (window as any).__getServerByAnyMean = getServerByAnyMean;
  (window as any).__getServerIDFromEndpoint = getJoinIdFromEndpoint;
  (window as any).__getDynamicServerDataFromEndpoint = getDynamicServerDataFromEndpoint;
  (window as any).__getLocalhostServer = getLocalhostServerInfo;
} catch (e) {}
