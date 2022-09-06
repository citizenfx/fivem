import { TCFXID } from "cfx/base/identifiers";
import { IServerActivityUserPlaytime } from "../activity/types";

export interface IServerReviewSubmitData {
  title: string,
  content: string,

  recommend: boolean,
}

export interface IServerReviews {
  readonly playtimes: Record<TCFXID, IServerActivityUserPlaytime>;

  readonly ownReview: IServerReviewItem | null;
  readonly ownPlaytime: IServerActivityUserPlaytime | null;

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
  readonly id: string,

  readonly sentiment: ServerReviewSentiment,

  readonly createdAt: Date,

  readonly authorId: number,
  readonly authorCfxId: TCFXID,
  readonly authorName: string,
  readonly authorAvatarURL: string,

  readonly title: string,
  readonly content: React.ReactNode,

  report?: IServerReviewReport,
  reactions?: IServerReviewItemReactions,
}

export interface IServerReviewReport {
  readonly canReport: boolean;
  readonly reportInProgress: boolean;

  readonly options: IServerReviewReportOption[];

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
