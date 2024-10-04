import { injectable } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';

import { defineService, ServicesContainer } from 'cfx/base/servicesContainer';

import { IUiMessage } from './types';
import { mpMenu } from '../../mpMenu';
import { maybeParseFormattedMessage } from '../../utils/messageFormatting';

export const IUiMessageService = defineService<IUiMessageService>('UiMessageService');
export type IUiMessageService = UiMessageService;

export function registerUiMessageService(container: ServicesContainer) {
  container.registerImpl(IUiMessageService, UiMessageService);
}

@injectable()
class UiMessageService {
  private _message: IUiMessage | null = null;
  public get message(): IUiMessage | null {
    return this._message;
  }
  private set message(message: IUiMessage | null) {
    this._message = message;
  }

  constructor() {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _message: observable.ref,
    });

    mpMenu.on('setWarningMessage', (event) => this.showWarningMessage(event.message));
  }

  readonly closeMessage = () => {
    this.message = null;
  };

  showInfoMessage(message: string) {
    this.message = {
      type: 'info',
      ...maybeParseFormattedMessage(message),
    };
  }

  showWarningMessage(message: string) {
    this.message = {
      type: 'warning',
      ...maybeParseFormattedMessage(message),
    };
  }
}
