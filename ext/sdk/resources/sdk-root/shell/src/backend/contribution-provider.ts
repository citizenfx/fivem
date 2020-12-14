import { interfaces } from "inversify";

export const ContributionProvider = Symbol('ContributionProvider');
export interface ContributionProvider<T> {
  getAll(): T[],
  getTagged(tag: string, value: any): T,
}

class ContributionProviderImpl<T> implements ContributionProvider<T> {
  protected services: T[] | null = null;

  constructor(
    protected serviceId: interfaces.ServiceIdentifier<T>,
    protected container: interfaces.Container,
  ) {

  }

  getAll(): T[] {
    if (this.services === null) {
      this.services = this.container.getAll(this.serviceId);
    }

    return this.services;
  }

  getTagged(tag: string, value: any): T {
    return this.container.getTagged(this.serviceId, tag, value);
  }
}

/**
 * Then you use it like
 *
 * @inject(ContributionProvider) @named(YourContribution)
 * protected readonly yourContributionProvider: ContributionProvider<YourContribution>;
 */
export const bindContributionProvider = (container: interfaces.Container, serviceId: symbol | string) => {
  container.bind(ContributionProvider)
    .toDynamicValue((ctx: interfaces.Context) => new ContributionProviderImpl(serviceId, ctx.container))
    .inSingletonScope()
    .whenTargetNamed(serviceId);
};
