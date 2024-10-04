import { makeAutoObservable, observable } from 'mobx';

import { IServerReviewItemReactions, ServerReviewReaction } from 'cfx/common/services/servers/reviews/types';
import { fetcher } from 'cfx/utils/fetcher';

import { IDiscourseService } from '../../discourse/discourse.service';
import { IDiscourse } from '../../discourse/types';

export class DiscourseServerReviewItemReactions implements IServerReviewItemReactions {
  private _count: Record<ServerReviewReaction, number> = {
    [ServerReviewReaction.Funny]: 0,
    [ServerReviewReaction.Helpful]: 0,
    [ServerReviewReaction.Unhelpful]: 0,
  };
  public get count(): Record<ServerReviewReaction, number> {
    return this._count;
  }
  private set count(count: Record<ServerReviewReaction, number>) {
    this._count = count;
  }

  private _reactionError: string | null = null;
  public get reactionError(): string | null {
    return this._reactionError;
  }
  private set reactionError(reactionError: string | null) {
    this._reactionError = reactionError;
  }

  private _reactionInProgress: boolean = false;
  public get reactionInProgress(): boolean {
    return this._reactionInProgress;
  }
  private set reactionInProgress(reactionInProgress: boolean) {
    this._reactionInProgress = reactionInProgress;
  }

  private get post(): IDiscourse.Post {
    return this._post;
  }
  private set post(post: IDiscourse.Post) {
    this._post = post;
  }

  constructor(
    protected readonly discourseService: IDiscourseService,
    protected _post: IDiscourse.Post,
  ) {
    this.populateCounts();

    makeAutoObservable(this, {
      // @ts-expect-error protected prop
      _post: observable.ref,
    });
  }

  canReact(reaction: ServerReviewReaction): boolean {
    if (this.reactionInProgress) {
      return false;
    }

    if (this.post.current_user_reaction && !this.post.current_user_reaction.can_undo) {
      return false;
    }

    if (this.hasReaction(reaction)) {
      return false;
    }

    return true;
  }

  hasReaction(reaction: ServerReviewReaction): boolean {
    return this.post.current_user_reaction?.id === reaction;
  }

  async react(reaction: ServerReviewReaction) {
    if (this.reactionInProgress) {
      return;
    }

    this.reactionInProgress = true;

    try {
      this.post = await this.discourseService.makeApiCall(
        `/discourse-reactions/posts/${this.post.id}/custom-reactions/${reaction}/toggle.json`,
        'PUT',
        '',
      );

      this.populateCounts();
    } catch (e) {
      if (e instanceof fetcher.HttpError) {
        const body: {
          error_type: string;
          errors: string[];
          extras: Record<string, unknown>;
        } | null = await e.readJsonBody();

        this.reactionError = body?.errors?.[0] || null;
      }

      // FIXME: Show this error somewhere may be?
      this.reactionError = this.reactionError || 'Failed to add reaction, may be try again later? :c';
    } finally {
      this.reactionInProgress = false;
    }
  }

  protected populateCounts() {
    for (const reaction of this.post.reactions) {
      this.count[reaction.id] = Number(reaction.count) || 0;
    }
  }
}
