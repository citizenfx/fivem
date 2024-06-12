import { inject, injectable } from 'inversify';

import { ServicesContainer } from 'cfx/base/servicesContainer';
import { IServersReviewsService } from 'cfx/common/services/servers/reviews/serversReviews.service';
import { IServerReviewReportOption, IServerReviews } from 'cfx/common/services/servers/reviews/types';
import { proxyInvariant } from 'cfx/utils/invariant';
import { ObservableAsyncValue } from 'cfx/utils/observable';

import { DiscourseServerReviews } from './reviews/DiscourseServerReviews';
import { IDiscourseService } from '../discourse/discourse.service';
import { IDiscourse } from '../discourse/types';

export function registerMpMenuServersReviewsService(container: ServicesContainer) {
  container.registerImpl(IServersReviewsService, MpMenuServersReviewsService);
}

@injectable()
export class MpMenuServersReviewsService implements IServersReviewsService {
  public readonly flagOptions: ObservableAsyncValue<IServerReviewReportOption[]>;

  private _cache: Record<string, DiscourseServerReviews> = {};

  constructor(
    @inject(IDiscourseService)
    protected readonly discourseService: IDiscourseService,
  ) {
    this.flagOptions = discourseService.siteData.map((siteData) => {
      const options: IServerReviewReportOption[] = [];

      for (const postActionType of siteData.post_action_types) {
        if (!suitablePostActionType(postActionType)) {
          continue;
        }

        options.push({
          id: proxyInvariant(postActionType.id, 'Invalid siteDate.post_action_type.id'),

          value: proxyInvariant(postActionType.name_key, 'Invalid siteDate.post_action_type.name_key'),
          label: postActionType.name,

          description: postActionType.description,
          withMessage: postActionType.is_custom_flag,
          messagePlaceholder: postActionType.is_custom_flag
            ? postActionType.short_description
            : undefined,
        });
      }

      return options;
    });
  }

  getForServer(serverId: string): IServerReviews {
    if (!this._cache[serverId]) {
      this._cache[serverId] = new DiscourseServerReviews(this.discourseService, this.flagOptions, serverId);
    }

    return this._cache[serverId];
  }
}

function suitablePostActionType(postActionType: IDiscourse.Type): boolean {
  if (postActionType.id === null) {
    return false;
  }

  if (!postActionType.name_key || postActionType.name_key === 'notify_user') {
    return false;
  }

  if (!postActionType.is_flag) {
    return false;
  }

  return true;
}
