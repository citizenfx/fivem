import { getAvatarURL } from "cfx/base/avatar";
import { formatCFXID, TCFXID } from "cfx/base/identifiers";
import { IServerReviewReport, IServerReviewReportOption, IServerReviewItem, IServerReviewItemReactions, ServerReviewSentiment } from "cfx/common/services/servers/reviews/types";
import { html2react } from "cfx/utils/html2react";
import { ObservableAsyncValue } from "cfx/utils/observable";
import { makeAutoObservable } from "mobx";
import { IDiscourseService } from "../../discourse/discourse.service";
import { IDiscourse } from "../../discourse/types";
import { DiscourseServerReviewReport } from "./reviewItemReport";
import { DiscourseServerReviewItemReactions } from "./reviewItemReactions";

export enum RecognizedTopicTags {
  Recommend = 'recommended',
  NotRecommend = 'unrecommended',
}

export class DiscourseServerReviewItem implements IServerReviewItem {
  readonly id: string;
  readonly sentiment: ServerReviewSentiment;
  readonly createdAt: Date;
  readonly authorId: number;
  readonly authorCfxId: TCFXID;
  readonly authorName: string;
  readonly authorAvatarURL: string;

  readonly title: string;
  readonly content: React.ReactNode;

  readonly report: IServerReviewReport;
  readonly reactions: IServerReviewItemReactions;

  constructor(
    discourseService: IDiscourseService,
    flagOptions: ObservableAsyncValue<IServerReviewReportOption[]>,
    topic: IDiscourse.Topic,
    post: IDiscourse.Post,
  ) {
    makeAutoObservable(this);

    this.id = topic.id.toString();
    this.title = topic.title;
    this.content = html2react(post.cooked);
    this.createdAt = new Date(topic.created_at);

    this.authorId = post.user_id;
    this.authorCfxId = formatCFXID(post.user_id);
    this.authorName = post.username;
    this.authorAvatarURL = getAvatarURL(post.avatar_template);

    this.sentiment = ServerReviewSentiment.Undecided;
    if (topic.tags.includes(RecognizedTopicTags.Recommend)) {
      this.sentiment = ServerReviewSentiment.Recommend;
    } else if (topic.tags.includes(RecognizedTopicTags.NotRecommend)) {
      this.sentiment = ServerReviewSentiment.NotRecommend;
    }

    this.report = new DiscourseServerReviewReport(discourseService, flagOptions, post);
    this.reactions = new DiscourseServerReviewItemReactions(discourseService, post);
  }
}
