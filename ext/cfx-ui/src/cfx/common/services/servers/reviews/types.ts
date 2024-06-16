import { TCFXID } from 'cfx/base/identifiers';

import { IServerActivityUserPlaytime } from '../activity/types';

export interface IServerReviewSubmitData {
  title: string;
  content: string;

  recommend: boolean;
}

export interface IServerReviews {
  readonly playtimes: Record<TCFXID, IServerActivityUserPlaytime>;
  readonly ownPlaytime: IServerActivityUserPlaytime | null;

  readonly ownReview: IServerReviewItem | null;
  readonly ownReviewApprovePending: boolean;

  readonly canSubmitReview: boolean;
  submitReview(data: IServerReviewSubmitData): Promise<string | null>;

  readonly initialLoading: boolean;

  readonly items: Record<string, IServerReviewItem>;
  readonly itemsSequence: string[];

  loadMoreItems(): void;
  readonly loadingMoreItems: boolean;
  readonly hasMoreItemsToLoad: boolean;
}

export enum ServerReviewSentiment {
  Undecided,
  Recommend,
  NotRecommend,
}

export enum ServerReviewReaction {
  Helpful = 'heart',
  Unhelpful = 'angry',
  Funny = 'laughing',
}

export enum ServerReviewReactionsSentiment {
  Helpful,
  Unhelpful,
}

export interface IServerReviewItem {
  readonly id: string;

  loaded: boolean;

  readonly sentiment: ServerReviewSentiment;

  readonly createdAt: Date;

  readonly authorId: number;
  readonly authorCfxId: TCFXID;
  readonly authorName: string;
  readonly authorAvatarURL: string;

  readonly title: string;
  readonly content: React.ReactNode;

  readonly hidden: boolean;

  report?: IServerReviewReport;
  reactions?: IServerReviewItemReactions;

  load(): Promise<void>;
}

export interface IServerReviewReport {
  readonly canReport: boolean;
  readonly reportInProgress: boolean;

  readonly options: IServerReviewReportOption[];
  readonly optionsLoading: boolean;
  readonly optionsError: string | null;

  ensureOptions(): void;

  canSubmit(option: IServerReviewReportOption, message?: string): boolean;
  submit(option: IServerReviewReportOption, message?: string): Promise<void>;
}

export interface IServerReviewReportOption {
  id: number;
  value: string;
  label: string;
  description: string;

  withMessage: boolean;
  messagePlaceholder?: string;
}

export interface IServerReviewItemReactions {
  readonly count: Record<ServerReviewReaction, number>;

  readonly reactionError: string | null;
  readonly reactionInProgress: boolean;

  hasReaction(reaction: ServerReviewReaction): boolean;
  canReact(reaction: ServerReviewReaction): boolean;
  react(reaction: ServerReviewReaction): void;
}
