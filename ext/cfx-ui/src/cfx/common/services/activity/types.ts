import React from 'react';

export namespace IRawTweetTypes {
  export type Indices = [number, number];

  export interface HashtagEntity {
    indices: Indices;
    text: string;
  }

  export interface SymbolEntity {
    indices: Indices;
    text: string;
  }

  export interface UrlEntity {
    indices: Indices;
    url: string;
    display_url: string;
    expanded_url: string;
  }

  export interface UserMentionEntity {
    indices: Indices;
    id: number;
    id_str: string;
    name: string;
    screen_name: string;
  }

  export interface MediaEntitySize {
    w: number;
    h: number;
    resize: 'fit' | 'crop' | string;
  }

  export interface MediaEntitySizes {
    thumb: MediaEntitySize;
    small: MediaEntitySize;
    medium: MediaEntitySize;
    large: MediaEntitySize;
  }

  export interface VideoInfoVariant {
    bitrate: number;
    content_type: string;
    url: string;
  }

  export interface VideoInfo {
    aspect_ratio: [number, number];
    variants: VideoInfoVariant[];
  }

  export interface BaseMediaEntity {
    type: string;
    indices: Indices;
    id: number;
    id_str: string;
    url: string;
    display_url: string;
    expanded_url: string;
    media_url: string;
    media_url_https: string;
    sizes: MediaEntitySizes;
  }

  export interface PhotoMediaEntity extends BaseMediaEntity {
    type: 'photo';
  }

  export interface AnimatedMediaEntity extends BaseMediaEntity {
    type: 'animated_gif' | 'video';
    video_info: VideoInfo;
  }

  export type MediaEntity = PhotoMediaEntity | AnimatedMediaEntity;

  export interface Entities {
    hashtags: HashtagEntity[];
    symbols: SymbolEntity[];
    user_mentions: UserMentionEntity[];
    urls: UrlEntity[];
    media: MediaEntity[];
  }

  export interface ExtendedEntities {
    media: MediaEntity[];
  }

  export interface User {
    id: number;
    id_str: string;
    name: string;
    screen_name: string;
    location: string;

    url: string;

    entities?: {
      description?: Partial<IRawTweetTypes.Entities>;
      url?: Partial<IRawTweetTypes.Entities>;
    };

    profile_background_title: string;
    profile_background_color: string;
    profile_background_image_url: string;
    profile_background_image_url_https: string;
    profile_banner_url: string;
    profile_image_url: string;
    profile_image_url_https: string;
    profile_link_color: string;
    profile_sidebar_border_color: string;
    profile_sidebar_fill_color: string;
    profile_text_color: string;
    profile_use_background_image: boolean;

    created_at: string;
    default_profile: boolean;
    default_profile_image: boolean;
    contributors_enabled: boolean;

    description: string;
    favourites_count: number;
    followers_count: number;
    statuses_count: number;
    friends_count: number;
    listed_count: number;
    has_extended_profile: boolean;
    verified: boolean;
    protected: boolean;
  }
}

export interface IRawTweet {
  id: number;
  id_str: string;

  rt_displayname?: string;

  user: IRawTweetTypes.User;

  entities: Partial<IRawTweetTypes.Entities>;
  extended_entities?: {
    media: IRawTweetTypes.MediaEntity[];
  };

  lang: string;
  place: null | unknown;
  possibly_sensetive: boolean;

  retweeted: boolean;
  retweet_count: number;

  favorited: boolean;
  favorite_count: number;

  text: string;
  display_text_range: IRawTweetTypes.Indices;
  truncated: boolean;

  source: string;
  created_at: string;

  retweeted_status?: IRawTweet;

  is_quote_status: boolean;

  in_reply_to_screen_name: null | string;
  in_reply_to_status_id: null | number;
  in_reply_to_status_id_str: null | string;
  in_reply_to_user_id: null | number;
  in_reply_to_user_id_str: null | string;
}

export type IActivityDataItemLinkIndices = [number, number];
export interface IActivityDataItemLink {
  indices: IActivityDataItemLinkIndices;
  url: string;
}

export interface IActivityItemData {
  id: string;
  url: string;
  date: Date;

  userDisplayName: string;
  userScreenName: string;
  userAvatarUrl: string;

  content: Map<number, string>;
  links: IActivityDataItemLink[];

  media: IActivityItemMedia[];

  repostedBy?: string | undefined;
}

export interface IActivityItemMedia {
  id: string;
  type: 'photo' | 'animated_gif' | 'video' | 'youtube';

  blurhash?: string;
  previewUrl?: string;
  previewAspectRatio: number;

  fullAspectRatio: number;
  fullUrl: string;
}

export interface IActivityItem {
  id: string;
  url: string;
  date: Date;

  userDisplayName: string;
  userScreenName: string;
  userAvatarUrl: string;

  content: React.ReactNode;

  media: IActivityItemMedia[];

  repostedBy?: string | undefined;
}
