import { master } from './source/api/master';

/**
 * Describes information completeness level available for given IServerView
 */
export enum ServerViewDetailsLevel {
  /**
   * Only address is known
   */
  Address = 0,

  /**
   * Populated from history entry
   *
   * Server should not stay in this details level for long,
   * in other words, must be resolved to a higher details level
   */
  Historical = 50,

  /**
   * Meta details level for any data that is relatively live
   */
  Live = 100,

  /**
   * Data from /dynamic.json endpoint of the server
   */
  DynamicDataJson = 101,

  /**
   * Data from from /dynamic.json and /info.json endpoints of the server
   */
  InfoAndDynamicDataJson = 199,

  /**
   * Data populated from servers list
   */
  MasterList = 201,

  /**
   * Data populated from complete server data
   */
  MasterListFull = 299,
}

export interface IServerView {
  // MANDATORY FIELDS
  // Unique id for internal use within cfx ui
  id: string;

  // Represents the very minimum set of information app needs to know about particular server
  detailsLevel: ServerViewDetailsLevel;

  locale: string;
  localeCountry: string;
  hostname: string;

  projectName: string;

  rawVariables: Record<string, string>;
  // /MANDATORY FIELDS

  joinId?: string;

  historicalAddress?: string;
  historicalIconURL?: string | null;

  connectEndPoints?: string[];
  projectDescription?: string;

  upvotePower?: number;
  burstPower?: number;

  offline?: true;

  iconVersion?: number | null;

  licenseKeyToken?: string | null;
  mapname?: string | null;
  gametype?: string | null;
  gamename?: string | null;
  fallback?: any;
  private?: boolean;
  scriptHookAllowed?: boolean;
  enforceGameBuild?: string;
  pureLevel?: ServerPureLevel;

  premium?: null | 'pt' | 'au' | 'ag' | string;

  bannerConnecting?: string;
  bannerDetail?: string;

  canReview?: boolean;

  ownerID?: string;
  ownerName?: string;
  ownerAvatar?: string;
  ownerProfile?: string;

  activitypubFeed?: string;
  onesyncEnabled?: boolean;
  server?: string | null;
  supportStatus?: 'supported' | 'end_of_support' | 'end_of_life' | 'unknown';

  playersMax?: number;
  playersCurrent?: number;

  tags?: string[];
  players?: IServerViewPlayer[];
  resources?: string[];

  variables?: Record<string, string>;
}

export enum ServerPureLevel {
  None = '0',
  AudioAndGraphicsAllowed = '1',
  NoModsAllowed = '2',
}

export interface IPinnedServersCollection {
  title: string;
  ids: string[];
}

export type IFeaturedServer = { type: 'id'; id: string } | { type: 'collection'; collection: IPinnedServersCollection };

/**
 * This is not a reflection of `/pins.json` file schema,
 * but internal representation of such
 */
export interface IPinnedServersConfig {
  featuredServer?: IFeaturedServer;
  pinIfEmpty?: boolean;
  pinnedServers: string[];
}

export interface IServerBoost {
  address: string;
  burst: number;
  power: number;
  source: string;
  user: number;
}

export interface IHistoryServer {
  // fields used in native code for setLastServers
  address: string;
  hostname: string;
  rawIcon: string;
  vars: Record<string, string>;
  // ---------------------------------------------

  title: string;
  time: Date;
  token: string;
}

export interface IServer {
  address: string;
  data: master.IServerData;
}

export interface IFullServerData {
  EndPoint: string;
  Data: {
    clients?: number;
    selfReportedClients?: number;

    server?: string;

    support_status?: string;

    svMaxclients?: number;
    sv_maxclients?: number;

    burstPower?: number;
    upvotePower: number;

    connectEndPoints: string[];

    enhancedHostSupport?: boolean;
    fallback?: boolean;
    private?: boolean;
    valid?: false; // only present if is `false`

    gametype?: string;
    hostname?: string;
    iconVersion?: number;
    lastSeen: string;
    mapname?: string;

    ownerID?: string;
    ownerName?: string;
    ownerAvatar?: string;
    ownerProfile?: string;

    players: IServerViewPlayer[];
    resources?: string[];

    vars?: Record<string, string>;
  };
}

export interface IServerViewPlayer {
  endpoint: string;
  id: number;
  identifiers: string[];
  name: string;
  ping: number;
}
