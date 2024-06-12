import { makeAutoObservable, runInAction } from 'mobx';

import { getAvatarURL } from 'cfx/base/avatar';
import { formatCFXID, TCFXID } from 'cfx/base/identifiers';
import {
  IServerReviewReport,
  IServerReviewReportOption,
  IServerReviewItem,
  IServerReviewItemReactions,
  ServerReviewSentiment,
} from 'cfx/common/services/servers/reviews/types';
import { html2react } from 'cfx/utils/html2react';
import { ObservableAsyncValue } from 'cfx/utils/observable';

import { DiscourseServerReviewItemReactions } from './reviewItemReactions';
import { DiscourseServerReviewReport } from './reviewItemReport';
import { IDiscourseService } from '../../discourse/discourse.service';
import { IDiscourse } from '../../discourse/types';

export enum RecognizedTopicTags {
  Recommend = 'recommended',
  NotRecommend = 'unrecommended',
}

export interface IUserStub {
  id: number;
  cfxId: string;
  name: string;
  avatar: string;
}

export class DiscourseServerReviewItem implements IServerReviewItem {
  readonly id: string;

  readonly sentiment: ServerReviewSentiment;

  readonly createdAt: Date;

  authorId: number = -1;

  authorCfxId: TCFXID = '';

  authorName: string = 'loading...';

  authorAvatarURL: string = '';

  readonly title: string;

  content: React.ReactNode = null;

  hidden = false;

  report?: IServerReviewReport;

  reactions?: IServerReviewItemReactions;

  private _loaded: boolean = false;
  public get loaded(): boolean {
    return this._loaded;
  }
  private set loaded(loaded: boolean) {
    this._loaded = loaded;
  }

  constructor(
    protected readonly discourseService: IDiscourseService,
    protected readonly flagOptions: ObservableAsyncValue<IServerReviewReportOption[]>,
    protected readonly topic: IDiscourse.Topic,
    protected readonly user: IUserStub | null,
    protected readonly loadPost: () => Promise<IDiscourse.Post>,
  ) {
    makeAutoObservable(this);

    this.id = topic.id.toString();
    this.title = topic.title;
    this.content = null;
    this.createdAt = new Date(topic.created_at);

    if (user) {
      this.authorId = user.id;
      this.authorCfxId = user.cfxId;
      this.authorName = user.name;
      this.authorAvatarURL = getAvatarURL(user.avatar);
    }

    this.sentiment = ServerReviewSentiment.Undecided;

    if (topic.tags.includes(RecognizedTopicTags.Recommend)) {
      this.sentiment = ServerReviewSentiment.Recommend;
    } else if (topic.tags.includes(RecognizedTopicTags.NotRecommend)) {
      this.sentiment = ServerReviewSentiment.NotRecommend;
    }
  }

  async load() {
    if (this.loaded) {
      return;
    }

    try {
      const post = await this.loadPost();

      runInAction(() => {
        this.hidden = post.hidden;
        this.content = html2react(post.cooked);

        this.report = new DiscourseServerReviewReport(this.discourseService, this.flagOptions, post);
        this.reactions = new DiscourseServerReviewItemReactions(this.discourseService, post);

        this.authorId = post.user_id;
        this.authorCfxId = formatCFXID(post.user_id);
        this.authorName = post.username;
        this.authorAvatarURL = getAvatarURL(post.avatar_template);
      });
    } catch (e) {
      console.error('Failed to load post', e);
    } finally {
      this.loaded = true;
    }
  }
}
