import { injectable, interfaces } from "inversify";

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
