import { Container, injectable, interfaces } from "inversify";
import getDecorators from 'inversify-inject-decorators';

@injectable()
export class ContainerAccess {
  constructor(
    private readonly container: interfaces.Container,
  ) { }

  get<T>(service: interfaces.ServiceIdentifier<T>): T {
    return this.container.get(service);
  }

  resolve<T>(service: interfaces.Newable<T>): T {
    return this.container.resolve(service);
  }

  withChildContainer<T>(task: (container: interfaces.Container) => T): T {
    const container = this.container.createChild();

    return task(container);
  }
}

let container = new Container();
let decorators = getDecorators(container);

export function getContainer(): interfaces.Container {
  return container;
}

export const lazyInject = (...args: Parameters<(typeof decorators)['lazyInject']>) => decorators!.lazyInject(...args);

export function registerSingleton<T>(service: interfaces.Newable<T>) {
  container.bind(service).toSelf().inSingletonScope();

  return service;
}

export function registerDynamic<T>(service: interfaces.ServiceIdentifier<T>, factory: (context: interfaces.Context) => T) {
  container.bind(service).toDynamicValue(factory);
}

export function registerFactory<T>(service: interfaces.ServiceIdentifier<T>, factory: interfaces.FactoryCreator<T>) {
  container.bind(service).toFactory(factory);
}
