import { useInstance } from '@cfx-dev/ui-components';
import { makeAutoObservable } from 'mobx';

import { IServerReviews } from 'cfx/common/services/servers/reviews/types';

export function useServerReviewFormState() {
  return useInstance(() => new ServerReviewFormState());
}

class ServerReviewFormState {
  public serverReviews: IServerReviews;

  private _title: string = '';
  public get title(): string {
    return this._title;
  }
  private set title(title: string) {
    this._title = title;
  }

  private _content: string = '';
  public get content(): string {
    return this._content;
  }
  private set content(content: string) {
    this._content = content;
  }

  private _recommend: boolean | null = null;
  public get recommend(): boolean | null {
    return this._recommend;
  }
  private set recommend(recommend: boolean | null) {
    this._recommend = recommend;
  }

  private _submitting: boolean = false;
  public get submitting(): boolean {
    return this._submitting;
  }
  private set submitting(submitting: boolean) {
    this._submitting = submitting;
  }

  private _error: string | null = null;
  public get error(): string | null {
    return this._error;
  }
  private set error(error: string | null) {
    this._error = error;
  }

  public get canSubmit(): boolean {
    if (this.submitting) {
      return false;
    }

    if (!this.titleValid) {
      return false;
    }

    if (!this.contentValid) {
      return false;
    }

    if (!this.recommendValid) {
      return false;
    }

    return true;
  }

  public get disabled(): boolean {
    return this.submitting;
  }

  constructor() {
    makeAutoObservable(this);
  }

  get titleValid(): boolean {
    return this.title.length >= 5;
  }

  readonly setTitle = (title: string) => {
    this.title = title;
  };

  get contentValid(): boolean {
    return this.content.length >= 5;
  }

  readonly setContent = (content: string) => {
    this.content = content;
  };

  get recommendValid(): boolean {
    return this.recommend !== null;
  }

  readonly setRecommend = () => {
    this.recommend = true;
  };

  readonly setNotRecommend = () => {
    this.recommend = false;
  };

  readonly submit = async () => {
    this.submitting = true;

    this.error = await this.serverReviews.submitReview({
      title: this.title,
      content: this.content,
      recommend: this.recommend!,
    });

    this.submitting = false;
  };
}
