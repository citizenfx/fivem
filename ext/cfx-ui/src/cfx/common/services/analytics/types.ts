export interface IAnalyticsEvent {
  action: string;
  properties: any;
}

export interface EventParamsBuilder<
  T extends string,
  P,
> extends IAnalyticsEvent {
  action: T;
  properties: P;
}

export enum EventActionNames {
  PageViews = 'trackPageview',
  SiteNavClick = 'nav_click',
  SiteLinkClick = 'site_link_click',
  PlayCTA = 'cta_play',
  ServerConnectCTA = 'cta_server_connect',
  ForumCTA = 'cta_forum',
  StartCTA = 'cta_start',
  AccountInfoCTA = 'cta_account_info',
  FilterCTA = 'cta_filter',
  HostCTA = 'cta_host',
  CTAOther = 'cta_other',
  ServerJoinFailed = 'server_join_fail',
  ServerSelect = 'server_select',
  BoostCTA = 'cta_boost',
}

export enum ElementPlacements {
  Nav = 'Nav',
  Footer = 'Footer',
  Settings = 'Settings',
  Unknown = 'Unknown',
  TopServers = 'TopServers',
  ServerExtraDetails = 'ServerExtraDetails',
  Continuity = 'Continuity',
  ServerPage = 'ServerPage',
  LastConnected = 'LastConnected',
  FeaturedServerTile = 'FeaturedServerTile',
  ServerFiltersWithDirectConnect = 'ServerFiltersWithDirectConnect',
  ServerSelectorModal = 'ServerSelectorModal',
  ServerList = 'ServerList',
  ServerFeaturedList = 'ServerFeaturedList',
  AuthModal = 'AuthModal',
  ServerConnect = 'ServerConnect',
}

type ServerType = 'featured' | undefined;

interface BaseProperties {
  view_name?: string;
}

export interface PageViewsProperties extends BaseProperties {
  position: number | undefined;
}
export interface SiteNavClickProperties extends BaseProperties {
  element_placement: ElementPlacements;
  position: number;
  text: string;
  link_url: string;
}
export interface SiteLinkClickProperties extends BaseProperties {
  element_placement: ElementPlacements;
  text: string;
  link_url: string;
}
export interface PlayCTAProperties extends BaseProperties {
  element_placement: ElementPlacements;
  text: string;
  link_url: string;
}
export interface ServerConnectCTAProperties extends BaseProperties {
  element_placement: ElementPlacements;
  server_id: string;
  server_name: string;
  server_type: ServerType;
  text: string;
  link_url: string;
}
export interface ForumCTAProperties extends BaseProperties {
  element_placement: ElementPlacements;
  text: string;
  link_url: string;
}
export interface StartCTAProperties extends BaseProperties {
  element_placement: ElementPlacements;
  text: string;
  link_url: string;
}
export interface AccountInfoCTAProperties extends BaseProperties {
  element_placement: ElementPlacements;
  text: string;
  link_url: string;
}
export interface FilterCTAProperties extends BaseProperties {
  element_placement: ElementPlacements;
  text: string;
  link_url: string;
}
export interface HostCTAProperties extends BaseProperties {
  element_placement: ElementPlacements;
  text: string;
  link_url: string;
}
export interface CTAOtherProperties extends BaseProperties {
  element_placement: ElementPlacements;
  text: string;
  link_url: string;
}
export interface ServerJoinFailedProperties extends BaseProperties {
  element_placement: ElementPlacements;
  server_id: string;
  server_name: string;
  server_type: ServerType;
  text: string;
  link_url: string;
}
export interface ServerSelectProperties extends BaseProperties {
  element_placement: ElementPlacements;
  server_id: string;
  server_name: string;
  server_type: ServerType;
  text: string;
  link_url: string;
}
export interface BoostCTAProperties extends BaseProperties {
  server_id: string;
  server_name: string;
  server_type: ServerType;
  text: string;
}

export type TrackEventParams =
  | EventParamsBuilder<EventActionNames.PageViews, PageViewsProperties>
  | EventParamsBuilder<EventActionNames.SiteNavClick, SiteNavClickProperties>
  | EventParamsBuilder<EventActionNames.SiteLinkClick, SiteLinkClickProperties>
  | EventParamsBuilder<EventActionNames.PlayCTA, PlayCTAProperties>
  | EventParamsBuilder<EventActionNames.ServerConnectCTA, ServerConnectCTAProperties>
  | EventParamsBuilder<EventActionNames.ForumCTA, ForumCTAProperties>
  | EventParamsBuilder<EventActionNames.StartCTA, StartCTAProperties>
  | EventParamsBuilder<EventActionNames.AccountInfoCTA, AccountInfoCTAProperties>
  | EventParamsBuilder<EventActionNames.FilterCTA, FilterCTAProperties>
  | EventParamsBuilder<EventActionNames.HostCTA, HostCTAProperties>
  | EventParamsBuilder<EventActionNames.CTAOther, CTAOtherProperties>
  | EventParamsBuilder<EventActionNames.ServerJoinFailed, ServerJoinFailedProperties>
  | EventParamsBuilder<EventActionNames.ServerSelect, ServerSelectProperties>
  | EventParamsBuilder<EventActionNames.BoostCTA, BoostCTAProperties>;

export const isFeaturedElementPlacement = (elementPlacement: ElementPlacements): boolean => {
  switch (elementPlacement) {
    case ElementPlacements.FeaturedServerTile:
    case ElementPlacements.ServerFeaturedList:
      return true;
    default:
      return false;
  }
};
