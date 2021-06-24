import { DisposableContainer, DisposableObject } from "backend/disposable-container";
import { inject, injectable } from "inversify";
import { getScopedEventName } from "utils/apiScope";
import { ApiClient, ApiEventListener } from "./api-client";
import { ApiContribution } from "./api-contribution";
import { getClientCallbackEventHandlers, getClientEventHandlers } from "./api-decorators";

@injectable()
export class ApiClientScoped implements DisposableObject {
  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  private scope = '';
  private scopeDisposer = new DisposableContainer();

  private eventBindings: Record<string, ApiEventListener> = Object.create(null);
  private callbackBindings: Record<string, ApiEventListener> = Object.create(null);

  init(contribution: ApiContribution) {
    const eventBindings = getClientEventHandlers(contribution);
    for (const { propKey, eventName } of eventBindings) {
      if (!contribution[propKey]) {
        continue;
      }

      this.eventBindings[eventName] = contribution[propKey].bind(contribution);
    }

    const callbackBindings = getClientCallbackEventHandlers(contribution);
    for (const { propKey, eventName } of callbackBindings) {
      if (!contribution[propKey]) {
        continue;
      }

      this.callbackBindings[eventName] = contribution[propKey].bind(contribution);
    }
  }

  setScope(scope: string) {
    this.scope = scope;

    if (!this.scopeDisposer.empty()) {
      this.scopeDisposer.dispose();

      this.scopeDisposer = new DisposableContainer();
    }

    for (const [eventName, eventHandler] of Object.entries(this.eventBindings)) {
      this.scopeDisposer.add(this.apiClient.on(this.getEventName(eventName), eventHandler));
    }

    for (const [eventName, eventHandler] of Object.entries(this.callbackBindings)) {
      this.scopeDisposer.add(this.apiClient.onCallback(this.getEventName(eventName), eventHandler));
    }
  }

  dispose() {
    this.scopeDisposer.dispose();
  }

  private getEventName(eventName: string): string {
    return getScopedEventName(eventName, this.scope);
  }
}
