import { IServerReviewReport, IServerReviewReportOption } from "cfx/common/services/servers/reviews/types";
import { ObservableAsyncValue } from "cfx/utils/observable";
import { makeAutoObservable } from "mobx";
import { IDiscourseService } from "../../discourse/discourse.service";
import { IDiscourse } from "../../discourse/types";

export class DiscourseServerReviewReport implements IServerReviewReport {
  public get canReport(): boolean {
		// 2 seems to be 'like', explicitly ignore
    return this.post.actions_summary.filter((action) => action.can_act && action.id !== 2).length > 0;
  }

  private _reportInProgress: boolean = false;
  public get reportInProgress(): boolean { return this._reportInProgress }
  private set reportInProgress(flagInProgress: boolean) { this._reportInProgress = flagInProgress }

  private _options: IServerReviewReportOption[] = [];
  public get options(): IServerReviewReportOption[] { return this._options }
  private set options(options: IServerReviewReportOption[]) { this._options = options }

  constructor(
    protected readonly discourseService: IDiscourseService,
    flagOptions: ObservableAsyncValue<IServerReviewReportOption[]>,
    protected post: IDiscourse.Post,
  ) {
    makeAutoObservable(this, {
      // @ts-expect-error protected
      post: false,
    });

    this.setOptions(flagOptions);
  }

  private async setOptions(flagOptions: ObservableAsyncValue<IServerReviewReportOption[]>) {
    try {
      this.options = (await flagOptions.waitGet()).filter((option) => {
        return this.post.actions_summary.find(({ id }) => id === option.id)?.can_act;
      });
    } catch (e) {
      // noop
    }
  }

  canSubmit(option: IServerReviewReportOption, message?: string | undefined): boolean {
    if (option.withMessage) {
      return (message?.length || 0) >= 10;
    }

    return true;
  }

  async submit(option: IServerReviewReportOption, message?: string | undefined) {
    await this.discourseService.makeApiCall('/post_actions', 'POST', {
      id: this.post.id,
      post_action_type_id: option.id,
      flag_topic: false,
      message,
    });
  }
}
