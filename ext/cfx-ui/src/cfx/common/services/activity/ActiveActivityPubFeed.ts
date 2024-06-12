import { makeAutoObservable, observable } from 'mobx';

import { fetcher } from 'cfx/utils/fetcher';
import { html2react } from 'cfx/utils/html2react';
import { HTTP_STATUS_CODE_TEXT } from 'cfx/utils/http';
import { isObject } from 'cfx/utils/object';

import { IActivityItem, IActivityItemMedia } from './types';

export enum ActiveActivityPubFeedState {
  ERROR,
  UNINIT,
  INITING,
  IDLE,
  LOADING_MORE,
}

export class ActiveActivityPubFeed {
  private _state: ActiveActivityPubFeedState = ActiveActivityPubFeedState.UNINIT;
  public get state(): ActiveActivityPubFeedState {
    return this._state;
  }
  private set state(state: ActiveActivityPubFeedState) {
    this._state = state;
  }

  private _initError: string = '';
  public get initError(): string {
    return this._initError;
  }
  private set initError(initError: string) {
    this._initError = initError;
  }

  private _items: IActivityItem[] = [];
  public get items(): IActivityItem[] {
    return this._items.slice().sort(activityItemSorter);
  }
  private set items(items: IActivityItem[]) {
    this._items = items;
  }

  private _itemsTotal: number = 0;
  public get itemsTotal(): number {
    return this._itemsTotal;
  }
  private set itemsTotal(itemsTotal: number) {
    this._itemsTotal = itemsTotal;
  }

  private _account: IActivityPubAccount | null = null;
  private get account(): IActivityPubAccount | null {
    return this._account;
  }
  private set account(account: IActivityPubAccount | null) {
    this._account = account;
  }

  protected nextOrderedCollectionPageURL: string | null = null;

  protected loadMoreAttemptsFailed: number = 0;

  public get hasMoreItemsToLoad(): boolean {
    if (this._items.length >= this.itemsTotal) {
      return false;
    }

    if (!this.nextOrderedCollectionPageURL) {
      return false;
    }

    return !this.loadingInit && !this.loadingMore;
  }

  public get loadingInit(): boolean {
    return this.state === ActiveActivityPubFeedState.INITING;
  }

  public get loadingMore(): boolean {
    return this.state === ActiveActivityPubFeedState.LOADING_MORE;
  }

  public get hasInitError(): boolean {
    return this.state === ActiveActivityPubFeedState.ERROR;
  }

  constructor(public readonly id: string) {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _items: observable.shallow,
      _account: observable.ref,
      loadMoreAttemptsFailed: false,
    });
  }

  async init() {
    if (this.state !== ActiveActivityPubFeedState.UNINIT) {
      return;
    }

    this.state = ActiveActivityPubFeedState.INITING;

    try {
      await this.loadAccount();
      await this.loadOrderedCollectionPage(this.account!.outbox);

      this.state = ActiveActivityPubFeedState.IDLE;
    } catch (e) {
      if (e instanceof fetcher.HttpError) {
        this.initError = HTTP_STATUS_CODE_TEXT[e.status] || '';
      }

      this.state = ActiveActivityPubFeedState.ERROR;
      console.error(e);
    }
  }

  readonly loadAll = async () => {
    while (this.hasMoreItemsToLoad && this.loadMoreAttemptsFailed < 5) {
      // eslint-disable-next-line no-await-in-loop
      await this.loadMore();
    }
  };

  readonly loadMore = async () => {
    if (!this.hasMoreItemsToLoad || !this.nextOrderedCollectionPageURL) {
      return;
    }

    this.state = ActiveActivityPubFeedState.LOADING_MORE;

    const nextPage = this.nextOrderedCollectionPageURL;
    this.nextOrderedCollectionPageURL = null;

    try {
      await this.loadOrderedCollectionPage(nextPage);
      this.loadMoreAttemptsFailed = 0;
    } catch (e) {
      // Preserve in case of error
      this.nextOrderedCollectionPageURL = nextPage;
      this.loadMoreAttemptsFailed++;
    } finally {
      this.state = ActiveActivityPubFeedState.IDLE;
    }
  };

  private async loadAccount() {
    const [user, host] = this.id.split('@');

    if (!user || !host) {
      throw new Error(`Invalid activitypub identifier: ${this.id}`);
    }

    const webfinger = await fetcher.json(`https://${host}/.well-known/webfinger?resource=${this.id}`);

    if (!isObject<{ links: Array<{ type: string; href: string }> }>(webfinger)) {
      throw new Error(`Invalid webfinger response for pub ${this.id}`);
    }

    const accountURL = webfinger.links.find(({
      type,
    }) => type === 'application/activity+json');

    if (!accountURL) {
      throw new Error(`No account link for pub ${this.id}`);
    }

    const account = await fetcher.json(accountURL.href, {
      headers: {
        Accept: 'application/activity+json',
      },
    });

    if (!isObject<IActivityPubAccount>(account) || account.type !== 'Person') {
      throw new Error(`Unknown or invalid account for pub ${this.id}`);
    }

    if (!account.outbox) {
      throw new Error(`Unknown or invalid account for pub ${this.id}, no outbox link`);
    }

    this.account = account;
  }

  private async loadOrderedCollectionPage(url: string) {
    const items = await fetcher.json(url);

    if (
      !isObject<{
        type?: 'OrderedCollection' | 'OrderedColelctionPage' | string;
        orderedItems?: any[];
        first?: string;
        prev?: string;
        totalItems?: number;
      }>(items)
    ) {
      console.warn(`Invalid response for OrderedCollectionPage url: ${url} of pub ${this.id}`);

      return;
    }

    if (items.type === 'OrderedCollection' && items.totalItems) {
      this.itemsTotal = items.totalItems;
    }

    if (items.orderedItems) {
      this.addOrderedItems(items.orderedItems);
    }

    if (items.first) {
      await this.loadOrderedCollectionPage(items.first);

      return;
    }

    if (items.prev) {
      this.nextOrderedCollectionPageURL = items.prev;
    }
  }

  private addOrderedItems(items: nay[]) {
    for (const item of items) {
      if (item.type !== 'Create') {
        continue;
      }

      const {
        object,
      } = item;

      if (object.type !== 'Note') {
        continue;
      }

      const activityItem: IActivityItem = {
        id: object.id,
        url: object.url,
        date: new Date(object.published),
        content: html2react(object.content),
        media: [],

        userAvatarUrl: this.account!.icon?.url || 'https://avatars.discourse.org/v4/letter/_/7993a0/96.png',
        userDisplayName: this.account!.name || this.account!.preferredUsername || this.id,
        userScreenName: this.id,
      };

      populateActivityPubActivityItemMedia(activityItem, object);

      this._items.push(activityItem);
    }
  }
}

type nay = any;

export interface IActivityPubAccount {
  type: 'Person' | string;

  outbox: string;

  name?: string | undefined;
  preferredUsername?: string;

  icon?: {
    url: string;
  };

  first?: string;
  prev?: string;
}

function activityItemSorter(a: IActivityItem, b: IActivityItem) {
  return b.date.valueOf() - a.date.valueOf();
}

function populateActivityPubActivityItemMedia(activityItem: IActivityItem, object: any) {
  if (!Array.isArray(object.attachment)) {
    return;
  }

  for (const attachment of object.attachment) {
    if (!isObject<{ mediaType: string; url: string; width: number; height: number; blurhash?: string }>(attachment)) {
      continue;
    }

    const type: IActivityItemMedia['type'] = { image: 'photo',
      video: 'video' }[
      attachment.mediaType.split('/')[0]
    ] as any;

    activityItem.media.push({
      id: attachment.url,
      type,
      blurhash: attachment.blurhash,
      previewUrl: type === 'video'
        ? undefined
        : attachment.url,
      previewAspectRatio: attachment.width / attachment.height,
      fullUrl: attachment.url,
      fullAspectRatio: attachment.width / attachment.height,
    });
  }
}
