import { inject, injectable } from "inversify";
import { ApiClient } from "backend/api/api-client";
import { LogProvider } from "backend/logger/log-provider";

@injectable()
export class ApiClientLogger implements LogProvider {
  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  constructor() {
    console.log('Started ApiClientLogger');
  }

  error(error, extra) {
    this.apiClient.emit('@@log', ['Backend error:', error.toString(), extra]);
  }

  log(...args) {
    this.apiClient.emit('@@log', args);
  }
}
