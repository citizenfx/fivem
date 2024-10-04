import { IServerView, ServerViewDetailsLevel } from './types';

export function mergeServers(from: IServerView, to: IServerView | null): IServerView {
  const fromServerIsNotLive = from.detailsLevel < ServerViewDetailsLevel.Live;
  const toServerIsNotLive = to && to.detailsLevel < ServerViewDetailsLevel.Live;
  const toServerIsLive = !toServerIsNotLive;

  // If both servers are not live - prefer `to` version
  if (fromServerIsNotLive && toServerIsNotLive) {
    return to;
  }

  if (!to || toServerIsNotLive) {
    return {
      ...from,
      offline: true,
    };
  }

  let server: IServerView;

  if (from.detailsLevel >= to.detailsLevel) {
    server = { ...from };

    // If `to` server is live - only update players counters
    if (toServerIsLive) {
      server.playersMax = to.playersMax;
      server.playersCurrent = to.playersCurrent;
    }
  } else {
    server = { ...to };
  }

  if (server.offline) {
    delete server.offline;
  }

  server.joinId = to.joinId || from.joinId;
  server.historicalAddress = to.historicalAddress || from.historicalAddress;
  server.historicalIconURL = to.historicalIconURL || from.historicalIconURL;

  return server;
}
