import { defineService } from 'cfx/base/servicesContainer';
import { ObservableAsyncValue } from 'cfx/utils/observable';

import { IServerReviewReportOption, IServerReviews } from './types';

export const IServersReviewsService = defineService<IServersReviewsService>('ServersReviewsService');
export interface IServersReviewsService {
  readonly flagOptions: ObservableAsyncValue<IServerReviewReportOption[]>;

  getForServer(serverId: string): IServerReviews;
}
