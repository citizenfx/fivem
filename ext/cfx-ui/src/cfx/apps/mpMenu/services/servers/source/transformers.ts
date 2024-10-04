import { DEFAULT_SERVER_LOCALE, DEFAULT_SERVER_LOCALE_COUNTRY, filterServerProjectName } from 'cfx/base/serverUtils';
import { processServerDataVariables } from 'cfx/common/services/servers/transformers';
import { IServerView, ServerViewDetailsLevel } from 'cfx/common/services/servers/types';

import { IDynamicServerData, IQueriedServerData } from './types';

export function dynamicServerData2ServerView(endpoint: string, data: IDynamicServerData): IServerView {
  return {
    id: endpoint,
    detailsLevel: ServerViewDetailsLevel.DynamicDataJson,
    locale: DEFAULT_SERVER_LOCALE,
    localeCountry: DEFAULT_SERVER_LOCALE_COUNTRY,
    connectEndPoints: [endpoint],
    gametype: data.gametype,
    mapname: data.mapname,
    hostname: data.hostname || '',
    projectName: filterServerProjectName(data.hostname),
    playersCurrent: data.clients,
    playersMax: parseInt(data.sv_maxclients, 10),
    upvotePower: 0,
    burstPower: 0,
    rawVariables: {},
  };
}

export function queriedServerData2ServerView(endpoint: string, data: IQueriedServerData): IServerView {
  return {
    id: endpoint,
    detailsLevel: ServerViewDetailsLevel.InfoAndDynamicDataJson,
    server: data.infoBlob.server,
    mapname: data.mapname,
    hostname: data.name || '',
    projectName: filterServerProjectName(data.name),
    gametype: data.gametype,
    locale: DEFAULT_SERVER_LOCALE,
    localeCountry: DEFAULT_SERVER_LOCALE_COUNTRY,
    connectEndPoints: [data.addr],
    rawVariables: data.infoBlob.vars,
    playersCurrent: parseInt(data.clients, 10) || 0,
    playersMax: parseInt(data.maxclients, 10) || 0,
    resources: data.infoBlob.resources,
    historicalIconURL: data.infoBlob.icon
      ? `data:image/png;base64,${data.infoBlob.icon}`
      : undefined,
    ...processServerDataVariables(data.infoBlob.vars),
  };
}
