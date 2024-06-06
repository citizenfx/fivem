import { shouldPrioritizePinnedServers } from 'cfx/base/serverUtils';

import { IListableServerView } from './types';
import { IServerListConfig, ServerListSortDir, ServersListSortBy } from '../lists/types';
import { IPinnedServersConfig } from '../types';

export function sortList(
  servers: Record<string, IListableServerView>,
  pinConfig: IPinnedServersConfig | null,
  config: IServerListConfig,
): string[] {
  const {
    sortBy,
    sortDir,
  } = config;

  const sortByName = sortBy === ServersListSortBy.Name
    ? sortByProperty.bind(null, servers, 'sortableName', sortDir)
    : sortByProperty.bind(null, servers, 'sortableName', ServerListSortDir.Asc);

  const sortByUpvotePower = sortBy === ServersListSortBy.Boosts
    ? sortByProperty.bind(null, servers, 'upvotePower', sortDir)
    : sortByProperty.bind(null, servers, 'upvotePower', ServerListSortDir.Desc);

  const sortByPlayers = sortBy === ServersListSortBy.Players
    ? sortByProperty.bind(null, servers, 'players', sortDir)
    : sortByProperty.bind(null, servers, 'players', ServerListSortDir.Desc);

  const sorters: Array<(a: string, b: string) => number> = [];

  if (pinConfig && shouldPrioritizePinnedServers(config)) {
    sorters.push(sortByPinConfig.bind(null, pinConfig));
  }

  switch (sortBy) {
    case ServersListSortBy.Name: {
      sorters.push(sortByName, sortByUpvotePower, sortByPlayers);
      break;
    }
    case ServersListSortBy.Players: {
      sorters.push(sortByPlayers, sortByUpvotePower, sortByName);
      break;
    }

    case ServersListSortBy.Boosts:
    default: {
      sorters.push(sortByUpvotePower, sortByPlayers, sortByName);
      break;
    }
  }

  return Object.keys(servers).sort((a, b) => {
    for (const sorter of sorters) {
      const retval = sorter(a, b);

      if (retval !== 0) {
        return retval;
      }
    }

    // Make it stable by ID comparison
    if (a < b) {
      return -1;
    }

    if (a > b) {
      return 1;
    }

    return 0;
  });
}

function sortByPinConfig(pinConfig: IPinnedServersConfig, a: string, b: string): number {
  const aPinned = pinConfig.pinnedServers.includes(a);
  const bPinned = pinConfig.pinnedServers.includes(b);

  if (aPinned === bPinned) {
    return 0;
  }

  if (aPinned && !bPinned) {
    return -1;
  }

  return 1;
}

function sortByProperty(
  servers: Record<string, IListableServerView>,
  property: keyof IListableServerView,
  dir: number,
  a: string,
  b: string,
): number {
  const aPropertyValue = servers[a][property] || 0;
  const bPropertyValue = servers[b][property] || 0;

  if (aPropertyValue === bPropertyValue) {
    return 0;
  }

  if (aPropertyValue > bPropertyValue) {
    return dir;
  }

  return -dir;
}
