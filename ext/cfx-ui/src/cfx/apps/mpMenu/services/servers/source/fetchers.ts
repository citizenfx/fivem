import { GameName } from "cfx/base/game";
import { fetcher } from "cfx/utils/fetcher";
import { mpMenu } from "cfx/apps/mpMenu/mpMenu";
import { resolveOrTimeout } from "cfx/utils/async";
import { CurrentGameName } from "cfx/base/gameRuntime";
import { IServerView } from "cfx/common/services/servers/types";
import { getMasterListServer } from "cfx/common/services/servers/source/utils/fetchers";
import { HostServerAddress, IParsedServerAddress, isIpServerAddress, isJoinServerAddress, parseServerAddress } from "cfx/common/services/servers/serverAddressParser";
import { IDynamicServerData, IQueriedServerData } from "./types";
import { dynamicServerData2ServerView, queriedServerData2ServerView } from "./transformers";

/**
 * No-brainer to get IServerView from virtually any string that can point to the server: joinId, joinId link, IP or host server address
 */
export async function getServerForAddress(address: string, gameName: GameName = CurrentGameName): Promise<IServerView | null> {
  const parsedAddress = parseServerAddress(address);
  if (!parsedAddress) {
    return null;
  }

  // Try fetching server by it's joinId
  const joinId = await getOrInferServerJoinId(parsedAddress);
  if (joinId) {
    const server = await getMasterListServer(gameName, joinId);
    if (server) {
      return server;
    }
  }

  // Can't do much with joinId at this point :c
  if (isJoinServerAddress(parsedAddress)) {
    return null;
  }

  if (isIpServerAddress(parsedAddress)) {
    return getServerViewFromQueriedOrDynamicServerData(gameName, joinId, parsedAddress.address);
  }

  // Resolve host server address as there could have been multiple address candidates
  const resolvedAddressAndDynamicServerData = await resolveServerHostAndGetDynamicServerData(parsedAddress);
  if (!resolvedAddressAndDynamicServerData) {
    return null;
  }

  const { resolvedAddress, dynamicServerData } = resolvedAddressAndDynamicServerData;

  if (__CFXUI_DEV__) {
    console.log('getServerForAddress result', {
      address,
      joinId,
      resolvedAddress,
      parseServerAddress,
      dynamicServerData,
    });
  }

  return getServerViewFromQueriedOrDynamicServerData(gameName, joinId, resolvedAddress, dynamicServerData);
}

async function getServerViewFromQueriedOrDynamicServerData(
  gameName: GameName,
  joinId: string | null,
  address: string,
  dynamicServerData: IDynamicServerData | null = null,
): Promise<IServerView | null> {
  const queriedServerData = await getQueriedDataForAddress(address);
  if (queriedServerData) {
    return saveHistoricalAddressAndDoGameNameCheck(
      address,
      joinId,
      gameName,
      queriedServerData2ServerView(address, queriedServerData),
    );
  }

  // If no dynamic server data provided - try to load it as a last resort
  dynamicServerData ??= await getDynamicServerDataForAddress(address);
  if (!dynamicServerData) {
    return null;
  }

  return saveHistoricalAddressAndDoGameNameCheck(
    address,
    joinId,
    gameName,
    dynamicServerData2ServerView(address, dynamicServerData),
  );
}

function doGameNameCheck(gameName: GameName, server: IServerView): IServerView | null {
  if (server.gamename) {
    if (server.gamename !== gameName) {
      return null;
    }
  }

  return server;
}

function saveHistoricalAddressAndDoGameNameCheck(address: string, joinId: string | null, gameName: GameName, server: IServerView): IServerView | null {
  if (!doGameNameCheck(gameName, server)) {
    return null;
  }

  if (joinId) {
    server.joinId = joinId;
  }

  server.historicalAddress = address;

  return server;
}

async function getOrInferServerJoinId(parsedAddress: IParsedServerAddress): Promise<string | null> {
  if (isJoinServerAddress(parsedAddress)) {
    return parsedAddress.address;
  }

  return getJoinIdForAddress(parsedAddress.address);
}

async function getJoinIdForAddress(address: string): Promise<string | null> {
  try {
    const serverID = await resolveOrTimeout(
      5000,
      'https://nui-internal/gsclient/url timed out',
      fetcher.text('https://nui-internal/gsclient/url', {
        method: 'POST',
        body: `url=${address}`,
      }),
    );

    if (serverID) {
      return serverID;
    }

    return null;
  } catch (e) {
    console.warn(e);

    return null;
  }
}

export async function getDynamicServerDataForAddress(address: string): Promise<IDynamicServerData | null> {
  // cpp side appends `/dynamic.json` so remove any trailing slash
  if (address.endsWith('/')) {
    address = address.substring(0, address.length - 1);
  }

  try {
    const res = await resolveOrTimeout(
      5000,
      'https://nui-internal/gsclient/dynamic timed out',
      fetcher.json('https://nui-internal/gsclient/dynamic', {
        method: 'POST',
        body: `url=${address}`,
      }),
    );

    if (res) {
      return res;
    }

    return null;
  } catch (e) {
    console.warn(e);

    return null;
  }
}

export async function getQueriedDataForAddress(address: string): Promise<IQueriedServerData | null> {
  try {
    return await mpMenu.queryServer(address);
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

async function resolveServerHostAndGetDynamicServerData(host: HostServerAddress) {
  if (!host.addressCandidates) {
    return fetchDynamicServerDataPair(host.address);
  }

  const shouldThrow = true;

  const httpAddresses: string[] = [];
  const httpsAddresses: string[] = [];

  for (const address of host.addressCandidates) {
    if (address.startsWith('https:')) {
      httpsAddresses.push(address);
    } else {
      httpAddresses.push(address);
    }
  }

  // Prioritize HTTPS addresses to make resolver stable as server may support both HTTP and HTTPS
  if (httpsAddresses.length) {
    try {
      const pair = await Promise.any(httpsAddresses.map((address) => fetchDynamicServerDataPair(address, shouldThrow)));

      // Continue to HTTP addresses if no pair
      if (pair) {
        return pair;
      }
    } catch (e) {
      // no-op
    }
  }

  if (httpAddresses.length) {
    try {
      return await Promise.any(host.addressCandidates.map((address) => fetchDynamicServerDataPair(address, shouldThrow)));
    } catch (e) {
      // no-op
    }
  }

  return null;
}

async function fetchDynamicServerDataPair(address: string, shouldThrow = false) {
  const dynamicServerData = await getDynamicServerDataForAddress(address);
  if (!dynamicServerData) {
    if (shouldThrow) {
      throw new Error();
    }

    return null;
  }

  return {
    resolvedAddress: address,
    dynamicServerData,
  };
}

try {
  (window as any).__getServerForAddress = getServerForAddress;
  (window as any).__getJoinIdForAddress = getJoinIdForAddress;
  (window as any).__getDynamicServerDataForAddress = getDynamicServerDataForAddress;
  (window as any).__getLocalhostServerInfo = getLocalhostServerInfo;
} catch (e) { }
