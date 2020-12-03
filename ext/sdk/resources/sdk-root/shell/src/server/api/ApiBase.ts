export abstract class ApiBase {
  protected bind<T extends Function>(fn: T): T {
    return fn.bind(this);
  }
}
