export interface IDynamicServerData {
  clients: number;
  gametype: string;
  hostname: string;
  iv: string;
  mapname: string;
  sv_maxclients: string;
}

export interface IQueriedServerData {
  name: string;
  mapname: string;
  gametype: string;
  clients: string;
  maxclients: string;
  ping: 42; // yes, it is always 42 in the cpp code, lol
  addr: string; // queried address, exactly how it was queried
  queryCorrelation: string; // also queried address, exactly how it was queried

  infoBlob: {
    server: string; // server software description
    enhancedHostSupport: boolean; // now it is hardcoded to be true, though
    resources: string[];
    icon?: string; // yes, data string of icon, as in "data:image/png;base64,..." or so
    vars: Record<string, string>;
    requestSteamTicket: 'off' | 'unset' | 'on';
    version: number; // this infoBlob joaat64 hash & 0x7fFFffFF
  };
}
