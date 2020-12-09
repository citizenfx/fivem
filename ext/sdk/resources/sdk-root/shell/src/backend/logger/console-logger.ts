import { injectable } from "inversify";
import { LogProvider } from "./log-provider";

@injectable()
export class ConsoleLogger implements LogProvider {
  constructor() {
    console.log('Started ConsoleLogger');
  }

  log(...args) {
    console.log(...args);
  }
}
