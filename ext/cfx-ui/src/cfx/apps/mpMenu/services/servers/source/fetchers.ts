import { mpMenu } from 'cfx/apps/mpMenu/mpMenu';
import { GameName } from 'cfx/base/game';
import { CurrentGameName } from 'cfx/base/gameRuntime';
import {
  HostServerAddress,
  IParsedServerAddress,
  isHostServerAddress,
  isIpServerAddress,
  isJoinOrHostServerAddress,
  isJoinServerAddress,
  JoinOrHostServerAddress,
  parseServerAddress,
} from 'cfx/common/services/servers/serverAddressParser';
import { getMasterListServer } from 'cfx/common/services/servers/source/utils/fetchers';
import { IServerView } from 'cfx/common/services/servers/types';
import { resolveOrTimeout } from 'cfx/utils/async';
import { fetcher } from 'cfx/utils/fetcher';

import { dynamicServerData2ServerView, queriedServerData2ServerView } from './transformers';
import { IDynamicServerData, IQueriedServerData } from './types';

/**
 * No-brainer to get IServerView from virtually any string that can point to the server: joinId, joinId link, IP or host server address
 */
export async function getServerForAddress(
  address: string,
  gameName: GameName = CurrentGameName,
): Promise<IServerView | null> {
  const parsedAddress = parseServerAddress(address);

  if (!parsedAddress) {
    return null;
  }

  // Infer joinId and try fetching server by it
  let joinId = await getOrInferServerJoinId(parsedAddress);

  if (joinId) {
    const server = await getMasterListServer(gameName, joinId);

    if (server) {
      if (!isJoinServerAddress(parsedAddress)) {
        return saveHistoricalAddressAndDoGameNameCheck(parsedAddress.address, joinId, gameName, server);
      }

      return server;
    }
  }

  if (isIpServerAddress(parsedAddress)) {
    return getServerViewFromQueriedOrDynamicServerData(gameName, joinId, parsedAddress.address);
  }

  // Can't do much with joinId at this point :c
  if (isJoinServerAddress(parsedAddress)) {
    return null;
  }

  // If parsed address is either joinId or host - set joinId to null,
  // because fetching server from the master list failed for this joinId
  if (isJoinOrHostServerAddress(parsedAddress)) {
    joinId = null;
  }

  // Resolve host server address as there could have been multiple address candidates
  const resolvedAddressAndDynamicServerData = await resolveServerHostAndGetDynamicServerData(parsedAddress);

  if (!resolvedAddressAndDynamicServerData) {
    return null;
  }

  const {
    resolvedAddress,
    dynamicServerData,
  } = resolvedAddressAndDynamicServerData;

  return getServerViewFromQueriedOrDynamicServerData(gameName, joinId, resolvedAddress, dynamicServerData);
}

async function getServerViewFromQueriedOrDynamicServerData(
  gameName: GameName,
  joinId: string | null,
  address: string,
  dynamicServerDataRaw: IDynamicServerData | null = null,
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
  let dynamicServerData = dynamicServerDataRaw;
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

function saveHistoricalAddressAndDoGameNameCheck(
  address: string,
  joinId: string | null,
  gameName: GameName,
  server: IServerView,
): IServerView | null {
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
  if (isJoinServerAddress(parsedAddress) || isJoinOrHostServerAddress(parsedAddress)) {
    return parsedAddress.address;
  }

  return getJoinIdForAddress(parsedAddress.address);
}

async function getJoinIdForAddress(address: string): Promise<string | null> {
  try {
    const joinId = await resolveOrTimeout(
      5000,
      'https://nui-internal/gsclient/url timed out',
      fetcher.text('https://nui-internal/gsclient/url', {
        method: 'POST',
        body: `url=${address}`,
      }),
    );

    if (joinId) {
      return joinId;
    }

    return null;
  } catch (e) {
    console.warn(e);

    return null;
  }
}

export async function getDynamicServerDataForAddress(addressRaw: string): Promise<IDynamicServerData | null> {
  let address = addressRaw;

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

export async function getLocalhostServerInfo(port: string): Promise<(IQueriedServerData & { address: string }) | null> {
  const response = (await mpMenu.queryServer(`localhost_sentinel:${port}`)) as IQueriedServerData & { address: string };

  const serverGameName = response.infoBlob?.vars?.gamename;

  if (serverGameName && serverGameName !== CurrentGameName) {
    return null;
  }

  return response;
}

async function resolveServerHostAndGetDynamicServerData(parsedAddress: HostServerAddress | JoinOrHostServerAddress) {
  const candidates = isHostServerAddress(parsedAddress)
    ? parsedAddress.addressCandidates || [parsedAddress.address]
    : parsedAddress.addressCandidates;

  if (candidates.length === 1) {
    return fetchDynamicServerDataPair(candidates[0]);
  }

  const shouldThrow = true;

  const httpCandidates: string[] = [];
  const httpsCandidates: string[] = [];

  for (const address of candidates) {
    if (address.startsWith('https:')) {
      httpsCandidates.push(address);
    } else {
      httpCandidates.push(address);
    }
  }

  // Prioritize HTTPS addresses to make resolver stable as server may support both HTTP and HTTPS
  if (httpsCandidates.length) {
    try {
      const pair = await Promise.any(
        httpsCandidates.map((address) => fetchDynamicServerDataPair(address, shouldThrow)),
      );

      // Continue to HTTP addresses if no pair
      if (pair) {
        return pair;
      }
    } catch (e) {
      // no-op
    }
  }

  if (httpCandidates.length) {
    try {
      return await Promise.any(httpCandidates.map((address) => fetchDynamicServerDataPair(address, shouldThrow)));
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
} catch (e) {
  // Do nothing
}
