import { injectable } from 'inversify';

import { LogProvider } from '../logService.extensions';

@injectable()
export class ConsoleLogProvider implements LogProvider {
  log(...args) {
    console.log(...args);
  }

  error(error, extra) {
    console.error(error, extra);
  }
}
