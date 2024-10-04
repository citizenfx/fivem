import { injectable } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';

import { defineService, ServicesContainer, useService } from 'cfx/base/servicesContainer';
import { fetcher } from 'cfx/utils/fetcher';

import { ActiveActivityPubFeed } from './ActiveActivityPubFeed';
import { compileActivityItem } from './compilers';
import { rawTweetToActivityDataItem } from './twitter';
import { IActivityItem, IRawTweet } from './types';

export const IActivityService = defineService<IActivityService>('ActivityService');
export type IActivityService = ActivityService;

export function registerActivityService(container: ServicesContainer) {
  container.registerImpl(IActivityService, ActivityService);
}

export function useActivityService() {
  return useService(IActivityService);
}

@injectable()
class ActivityService {
  private _officialItems: Record<string, IActivityItem> = {};
  get officialItems(): IActivityItem[] {
    return Object.values(this._officialItems).sort(activityItemSorter);
  }

  private _communityItems: Record<string, IActivityItem> = {};
  get communityItems(): IActivityItem[] {
    return Object.values(this._communityItems).sort(activityItemSorter);
  }

  private activeActivityPubFeeds: Record<string, ActiveActivityPubFeed> = {};

  constructor() {
    makeAutoObservable(this, {
      // @ts-expect-error
      _officialItems: observable.shallow,
      _communityItems: observable.shallow,
      getActiveAcitivityPubFeed: false,
    });

    this.fetchOfficial();
  }

  public getActiveActivityPubFeed(pub: string): ActiveActivityPubFeed {
    if (!this.activeActivityPubFeeds[pub]) {
      this.activeActivityPubFeeds[pub] = new ActiveActivityPubFeed(pub);

      this.activeActivityPubFeeds[pub].init();
    }

    return this.activeActivityPubFeeds[pub];
  }

  private async fetchOfficial() {
    try {
      const tweets = await fetcher.json('https://runtime.fivem.net/tweets.json');

      if (!Array.isArray(tweets)) {
        console.warn('Expected array of tweets, got', typeof tweets, 'from https://runtime.fivem.net/tweets.json');

        return;
      }

      this.populateFromTweets(tweets);
    } catch (e) {
      console.error('Failed to load official tweets', e);
    }
  }

  private populateFromTweets(tweets: IRawTweet[]) {
    for (const tweet of tweets) {
      if (!tweet) {
        continue;
      }

      if (tweet.in_reply_to_user_id) {
        continue;
      }

      const dataItem = rawTweetToActivityDataItem(tweet);

      if (!dataItem) {
        continue;
      }

      const isOfficial = !dataItem.repostedBy;

      const activityItem = compileActivityItem(dataItem);

      if (isOfficial) {
        this._officialItems[activityItem.id] = activityItem;
      } else {
        this._communityItems[activityItem.id] = activityItem;
      }
    }
  }
}

function activityItemSorter(a: IActivityItem, b: IActivityItem) {
  return b.date.valueOf() - a.date.valueOf();
}
