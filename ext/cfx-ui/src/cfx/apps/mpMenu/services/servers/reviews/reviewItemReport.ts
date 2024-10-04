import { makeAutoObservable } from 'mobx';

import { IServerReviewReport, IServerReviewReportOption } from 'cfx/common/services/servers/reviews/types';
import { ObservableAsyncValue } from 'cfx/utils/observable';

import { IDiscourseService } from '../../discourse/discourse.service';
import { IDiscourse } from '../../discourse/types';

export class DiscourseServerReviewReport implements IServerReviewReport {
  public get canReport(): boolean {
    // 2 seems to be 'like', explicitly ignore
    return this.post.actions_summary.filter((action) => action.can_act && action.id !== 2).length > 0;
  }

  private _reportInProgress: boolean = false;
  public get reportInProgress(): boolean {
    return this._reportInProgress;
  }
  private set reportInProgress(flagInProgress: boolean) {
    this._reportInProgress = flagInProgress;
  }

  private _options: IServerReviewReportOption[] = [];
  public get options(): IServerReviewReportOption[] {
    return this._options;
  }
  private set options(options: IServerReviewReportOption[]) {
    this._options = options;
  }

  private _optionsLoading: boolean = true;
  public get optionsLoading(): boolean {
    return this._optionsLoading;
  }
  private set optionsLoading(optionsLoading: boolean) {
    this._optionsLoading = optionsLoading;
  }

  private _optionsError: string | null = null;
  public get optionsError(): string | null {
    return this._optionsError;
  }
  private set optionsError(optionsError: string | null) {
    this._optionsError = optionsError;
  }

  private optionsInitialized = false;

  constructor(
    protected readonly discourseService: IDiscourseService,
    protected readonly flagOptions: ObservableAsyncValue<IServerReviewReportOption[]>,
    protected post: IDiscourse.Post,
  ) {
    makeAutoObservable(this, {
      // @ts-expect-error protected
      post: false,
    });
  }

  async ensureOptions() {
    if (this.optionsInitialized) {
      return;
    }

    this.optionsInitialized = true;

    try {
      this.options = (await this.flagOptions.waitGet())
        .filter((option) => this.post.actions_summary.find(({
          id,
        }) => id === option.id)?.can_act);
    } catch (e) {
      console.warn(e);

      this.optionsError = e.message || 'Failed to load flag options';
    } finally {
      this.optionsLoading = false;
    }
  }

  canSubmit(option: IServerReviewReportOption, message?: string | undefined): boolean {
    if (this.optionsLoading) {
      return false;
    }

    if (this.optionsError) {
      return false;
    }

    if (option.withMessage) {
      return (message?.length || 0) >= 10;
    }

    return true;
  }

  async submit(option: IServerReviewReportOption, message?: string | undefined) {
    try {
      await this.discourseService.makeApiCall('/post_actions', 'POST', {
        id: this.post.id,
        post_action_type_id: option.id,
        flag_topic: false,
        message,
      });
    } catch (e) {
      console.warn(e);
    }
  }
}
