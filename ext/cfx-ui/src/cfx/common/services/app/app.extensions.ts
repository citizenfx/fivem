import { interfaces } from 'inversify';

import { ServicesContainer } from '../../../base/servicesContainer';

export const AppContribution = Symbol('AppContribution');
export interface AppContribution {
  init?(): any;

  beforeRender?(): any;

  afterRender?(): any;
}

export function registerAppContribution<T extends AppContribution>(
  container: ServicesContainer,
  service: interfaces.Newable<T>,
) {
  container.registerImpl(AppContribution, service);
}
