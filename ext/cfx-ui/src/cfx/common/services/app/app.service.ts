import { inject, injectable, multiInject, named, optional } from 'inversify';

import { AppContribution } from './app.extensions';
import { ServicesContainer } from '../../../base/servicesContainer';
import { ScopedLogger } from '../log/scopedLogger';

@injectable()
class AppService {
  @inject(ScopedLogger)
  @named('AppService')
  protected logService: ScopedLogger;

  @multiInject(AppContribution)
  @optional()
  protected readonly appContributions: AppContribution[];

  async beforeRender() {
    this.logService.log('Initing');
    await Promise.all(this.appContributions.map((contribution) => contribution.init?.()));

    this.logService.log('Before render');
    await Promise.all(this.appContributions.map((contribution) => contribution.beforeRender?.()));
  }

  async afterRender() {
    this.logService.log('After render');
    await Promise.all(this.appContributions.map((contribution) => contribution.afterRender?.()));
  }
}

export function registerAppService(container: ServicesContainer) {
  container.register(AppService);
}

export function getAppService(container: ServicesContainer): AppService {
  return container.get(AppService);
}
