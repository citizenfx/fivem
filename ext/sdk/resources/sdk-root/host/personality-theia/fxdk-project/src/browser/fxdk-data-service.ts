import { injectable } from 'inversify';


@injectable()
export class FxdkDataService {
  public data: { [key: string]: any } = {};
}
