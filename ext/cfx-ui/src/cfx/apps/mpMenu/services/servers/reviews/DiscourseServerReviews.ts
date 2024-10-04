import { makeAutoObservable, observable } from 'mobx';

import { formatCFXID, TCFXID } from 'cfx/base/identifiers';
import { loadPlaytimes } from 'cfx/common/services/servers/activity/playtimes';
import { IServerActivityUserPlaytime } from 'cfx/common/services/servers/activity/types';
import {
  IServerReviewReportOption,
  IServerReviewItem,
  IServerReviews,
  IServerReviewSubmitData,
} from 'cfx/common/services/servers/reviews/types';
import { fetcher } from 'cfx/utils/fetcher';
import { ObservableAsyncValue } from 'cfx/utils/observable';

import { DiscourseServerReviewItem, IUserStub, RecognizedTopicTags } from './reviewItem';
import { IDiscourseService } from '../../discourse/discourse.service';
import { IDiscourse } from '../../discourse/types';

// discourse' server reviews category id
const REVIEWS_CATEGORY_ID = 76;

// the required play time (in seconds) to be allowed to post a review
const HOURS = 3600;
const REVIEW_REQUIRED_PLAYTIME = 2 * HOURS;

enum OwnReviewState {
  Loading,
  LoadingError,
  None,
  Exists,
  ApprovePending,
}

export class DiscourseServerReviews implements IServerReviews {
  private _initialLoading: boolean = true;
  public get initialLoading(): boolean {
    return this._initialLoading;
  }
  private set initialLoading(initialLoading: boolean) {
    this._initialLoading = initialLoading;
  }

  private _items: Record<string, IServerReviewItem> = {};
  public get items(): Record<string, IServerReviewItem> {
    return this._items;
  }
  private set items(items: Record<string, IServerReviewItem>) {
    this._items = items;
  }

  private _itemsSequence: string[] = [];
  public get itemsSequence(): string[] {
    return this._itemsSequence;
  }
  private set itemsSequence(itemsSequence: string[]) {
    this._itemsSequence = itemsSequence;
  }

  private _playtimes: Record<TCFXID, IServerActivityUserPlaytime> = {};
  public get playtimes(): Record<TCFXID, IServerActivityUserPlaytime> {
    return this._playtimes;
  }
  private set playtimes(playtimes: Record<TCFXID, IServerActivityUserPlaytime>) {
    this._playtimes = playtimes;
  }

  private _loadingMoreItems: boolean = false;
  public get loadingMoreItems(): boolean {
    return this._loadingMoreItems;
  }
  private set loadingMoreItems(loadingMoreItems: boolean) {
    this._loadingMoreItems = loadingMoreItems;
  }

  private _hasMoreItemsToLoad: boolean = false;
  public get hasMoreItemsToLoad(): boolean {
    return this._hasMoreItemsToLoad;
  }
  private set hasMoreItemsToLoad(hasMoreItemsToLoad: boolean) {
    this._hasMoreItemsToLoad = hasMoreItemsToLoad;
  }

  private _ownReview: IServerReviewItem | null = null;
  public get ownReview(): IServerReviewItem | null {
    return this._ownReview;
  }
  private set ownReview(ownReview: IServerReviewItem | null) {
    this._ownReview = ownReview;
  }

  private _ownReviewState: OwnReviewState = OwnReviewState.Loading;
  public get ownReviewState(): OwnReviewState {
    return this._ownReviewState;
  }
  private set ownReviewState(ownReviewState: OwnReviewState) {
    this._ownReviewState = ownReviewState;
  }

  private _ownPlaytime: IServerActivityUserPlaytime | null = null;
  public get ownPlaytime(): IServerActivityUserPlaytime | null {
    return this._ownPlaytime;
  }
  private set ownPlaytime(ownPlaytime: IServerActivityUserPlaytime | null) {
    this._ownPlaytime = ownPlaytime;
  }

  public get canSubmitReview(): boolean {
    if (!this.discourseService.account) {
      return false;
    }

    if (!this.ownPlaytime || this.ownPlaytime.seconds < REVIEW_REQUIRED_PLAYTIME) {
      return false;
    }

    switch (this.ownReviewState) {
      case OwnReviewState.Exists:
      case OwnReviewState.Loading:
      case OwnReviewState.LoadingError:
      case OwnReviewState.ApprovePending:
        return false;
    }

    return true;
  }

  public get ownReviewApprovePending(): boolean {
    return this.ownReviewState === OwnReviewState.ApprovePending;
  }

  private _currentReviewsPage = 0;

  constructor(
    protected readonly discourseService: IDiscourseService,
    protected readonly flagOptions: ObservableAsyncValue<IServerReviewReportOption[]>,
    public readonly serverId: string,
  ) {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _items: observable.shallow,
      _playtimes: observable.shallow,
    });

    this.initialLoad();
  }

  private async initialLoad() {
    await this.discourseService.initialAuthCompletePromise();

    await this.loadOwnReview();

    await this.loadReviews();

    this.initialLoading = false;
  }

  public async submitReview(data: IServerReviewSubmitData): Promise<string | null> {
    try {
      const response: IDiscourse.PostCreateResponse = await this.discourseService.makeApiCall('/posts', 'POST', {
        title: data.title,
        raw: data.content,
        category: REVIEWS_CATEGORY_ID,
        tags: [this.serverId, data.recommend
          ? RecognizedTopicTags.Recommend
          : RecognizedTopicTags.NotRecommend],
        archetype: 'regular',

        typing_duration_msecs: 3000, // TODO
        composer_open_duration_msecs: 5000, // TODO?
        nested_post: true, // use the new api
      });

      if (!response.success && response.errors) {
        return response.errors.join('\n');
      }

      if (response.action === 'enqueued') {
        // TODO: flow to say the post is pending-review

        this.ownReviewState = OwnReviewState.ApprovePending;

        return null;
      }

      await this.loadOwnReview();
    } catch (e) {
      return e.message;
    }

    return null;
  }

  public async loadMoreItems() {
    if (!this.hasMoreItemsToLoad) {
      return;
    }

    this.loadingMoreItems = true;

    await this.loadReviews(++this._currentReviewsPage);

    this.loadingMoreItems = false;
  }

  private async loadReviews(page = 0) {
    try {
      const params = new URLSearchParams({
        page,
        ascending: false,
        order: 'likes',
      } as any);

      const response = await this.discourseService.makeApiCall<void, IDiscourse.TagTopicsResponse>(
        `/tags/c/${REVIEWS_CATEGORY_ID}/${this.serverId}.json?${params}`,
      );

      const users: Record<number, IUserStub> = {};

      if (response.users) {
        const userCfxIds: string[] = [];

        for (const user of response.users) {
          if (user.id < 0) {
            continue;
          }

          const userCfxId = formatCFXID(user.id);

          userCfxIds.push(userCfxId);

          users[user.id] = {
            id: user.id,
            name: user.username,
            cfxId: userCfxId,
            avatar: user.avatar_template,
          };
        }

        await this.loadAndAddPlaytimes(userCfxIds);
      }

      // Yeah would be nice, but Discourse reports nonsense in there
      //
      // Let's also get the total items count here since it's the very firts page, right
      // if (page === 0) {
      //   this.totalItemsCount = response.topic_list.tags.find((tag) => tag.name === this.serverId)?.topic_count || 0;
      // }

      for (const topic of response.topic_list.topics) {
        const userId = topic.posters?.find(
          (poster) => poster.description === 'Original Poster' && poster.user_id > 0,
        )?.user_id || -1;

        if (userId === this.discourseService.account?.id) {
          continue;
        }

        const user = users[userId] || null;

        this.addReviewItem(this.loadDetailsAndTransformToServerReviewItem(topic, user));
      }

      // Infer if we just loaded the very last page of review topics
      this.hasMoreItemsToLoad = !!response.topic_list.more_topics_url;
    } catch (e) {
      console.error(e);
    }
  }

  private async loadOwnReview() {
    if (!this.discourseService.account) {
      return;
    }

    // Load own playtime
    const ownCfxId = formatCFXID(this.discourseService.account.id);
    this.loadAndAddPlaytimes([ownCfxId]).then(() => {
      this.ownPlaytime = this.playtimes[ownCfxId] || null;
    });

    try {
      const response = await this.discourseService.makeApiCall<void, IDiscourse.TagTopicsResponse>(
        `/tags/c/${REVIEWS_CATEGORY_ID}/${this.serverId}/l/posted.json`,
      );

      const [topic] = response.topic_list.topics;

      if (!topic) {
        this.ownReviewState = OwnReviewState.None;

        return;
      }

      this.ownReview = this.loadDetailsAndTransformToServerReviewItem(topic, {
        id: this.discourseService.account.id,
        cfxId: formatCFXID(this.discourseService.account.id),
        name: this.discourseService.account.username,
        avatar: this.discourseService.account.avatarTemplate,
      });
      this.ownReviewState = OwnReviewState.Exists;
    } catch (e) {
      if (fetcher.HttpError.is(e)) {
        // No reviews - can post still, right
        if (e.status === 404) {
          this.ownReviewState = OwnReviewState.None;

          return;
        }
      }

      console.error(e);

      // Need to mark as loaded so we won't show review form
      // as it's impossible to tell whether or not user have left a review already
      this.ownReviewState = OwnReviewState.LoadingError;
    }
  }

  private async loadAndAddPlaytimes(cfxIds: TCFXID[]) {
    try {
      const playtimes = await loadPlaytimes(this.serverId, cfxIds);

      this.playtimes = {
        ...this.playtimes,
        ...playtimes,
      };
    } catch (e) {
      // no-op
    }
  }

  private addReviewItem(reviewItem: IServerReviewItem) {
    if (this.items[reviewItem.id]) {
      return;
    }

    // Do not add own review(s)
    if (this.ownReview?.authorId === reviewItem.authorId) {
      return;
    }

    this.itemsSequence.push(reviewItem.id);
    this.items[reviewItem.id] = reviewItem;
  }

  private loadDetailsAndTransformToServerReviewItem(
    topic: IDiscourse.Topic,
    user: IUserStub | null,
  ): IServerReviewItem {
    const loadPost = async () => {
      const postsResponse = await this.discourseService.makeApiCall<void, IDiscourse.TopicPostsResponse>(
        `/t/${topic.id}/posts.json`,
      );

      if (!Array.isArray(postsResponse.post_stream.posts)) {
        throw new Error('Invalid response');
      }

      const [post] = postsResponse.post_stream.posts;

      if (!post) {
        throw new Error('Invalid response: no first post');
      }

      return post;
    };

    return new DiscourseServerReviewItem(this.discourseService, this.flagOptions, topic, user, loadPost);
  }
}
