import { setCurrentGameBuild, setCurrentGameName, setCurrentGamePureLevel } from 'cfx/base/gameRuntime';
import { GameName } from 'cfx/base/game';
import { Deferred, timeout } from 'cfx/utils/async';
import { IDisposable } from 'cfx/utils/disposable';
import { MultiEventEmitter } from "cfx/utils/eventEmitter";
import { AwaitableValue } from 'cfx/utils/observable';
import { RichEvent } from 'cfx/utils/types';
import { makeAutoObservable } from 'mobx';
import { IQueriedServerData } from './services/servers/source/types';

const nuiWindow: (typeof window) & {
  nuiTargetGame: string;
  nuiTargetGameBuild: number;
  nuiTargetGamePureLevel: number;
  nuiSetAudioCategory(category: string): void;
  nuiSystemLanguages: string[];

  invokeNative(native: string, arg: string): void;
} = window as any;

// Set this as early as possible
setCurrentGameName((nuiWindow.nuiTargetGame as GameName) || GameName.RedM);
if (typeof nuiWindow.nuiTargetGameBuild === 'number') {
  setCurrentGameBuild(nuiWindow.nuiTargetGameBuild.toString());
}
if (typeof nuiWindow.nuiTargetGamePureLevel === 'number') {
  setCurrentGamePureLevel(nuiWindow.nuiTargetGamePureLevel.toString());
}

const events = new MultiEventEmitter<TPayload>();
const fileSelectRequests: Record<string, (filePath: string) => void> = {};
const serverQueryRequests: Record<string, Deferred<any>> = {};

class NicknameStore {
  private _nickname = window.localStorage.getItem('nickOverride') || '';

  get nickname(): string {
    return this._nickname;
  }
  set nickname(newNickname: string) {
    this._nickname = newNickname;

    window.localStorage.setItem('nickOverride', newNickname);

    this.propagateNicknameChange();
  }

  constructor() {
    makeAutoObservable(this);

    this.propagateNicknameChange();
  }

  private propagateNicknameChange() {
    if (this.nickname) {
      mpMenu.invokeNative('checkNickname', this.nickname);
    }
  }
}

export namespace mpMenu {
  export function invokeNative(native: string, arg = '') {
    console.log('Invoking native', native, JSON.stringify(arg));

    nuiWindow.invokeNative(native, arg);
  }

  export function onRich<Name extends string, Payload extends TPayload>(
    eventDescriptor: RichEvent.Descriptor<Name, Payload>,
    cb: (data: Payload) => void,
  ): IDisposable {
    return events.addListener(eventDescriptor, cb);
  }
  export function on<T extends TPayload>(eventName: string, cb: (data: T) => void): IDisposable {
    return events.addListener(eventName, cb);
  }

  export function openUrl(url: string) {
    invokeNative('openUrl', url);
  }

  export const systemLanguages = [...new Set(nuiWindow.nuiSystemLanguages || ['en-us'])];

  export const computerName = new AwaitableValue('');

  export const discordIdZ = new AwaitableValue<number>(0);

  
  export const playerNickname = new NicknameStore();
  export function getPlayerNickname() { return playerNickname.nickname }
  export function setPlayerNickname(nickname: string) { playerNickname.nickname = nickname }
  
  export const memberCounbt = new AwaitableValue<number>(0);
  export function getPlayerCount() { return memberCounbt.value }
  export function setPlayerCount(amount: number) { memberCounbt.value = amount }

  export async function selectFile(key: string): Promise<string> {
    return new Promise<string>((resolve) => {
      fileSelectRequests[key] = resolve;

      invokeNative('openFileDialog', key);
    });
  }

  export function queryServer(fullAddress: string): Promise<IQueriedServerData> {
    if (serverQueryRequests[fullAddress]) {
      return serverQueryRequests[fullAddress].promise;
    }

    const deferred = new Deferred<IQueriedServerData>();

    serverQueryRequests[fullAddress] = deferred;

    invokeNative('queryServer', fullAddress);

    timeout(7500).then(() => {
      if (!serverQueryRequests[fullAddress]) {
        return;
      }

      serverQueryRequests[fullAddress].reject(new Error('Server query timed out after 7.5s'));
      delete serverQueryRequests[fullAddress];
    });

    return deferred.promise;
  }

  export function submitAdaptiveCardResponse(data: object) {
    invokeNative('submitCardResponse', JSON.stringify({ data }));
  }

  // Subscribe to wrapped events
  on('setComputerName', (data: { data: string }) => computerName.value = data.data);

  on('discordIdZ', (data: { data: number }) =>  discordIdZ.value = data.data);
    
  on('fileDialogResult', ({ dialogKey, result }: { dialogKey: string, result: string }) => fileSelectRequests[dialogKey]?.(result));

  on('serverQueried', (data: any) => {
    const deferred = serverQueryRequests[data.queryCorrelation];
    if (!deferred) {
      return;
    }

    delete serverQueryRequests[data.queryCorrelation];

    deferred.resolve(data);
  });

  on('queryFailed', (data: any) => {
    const deferred = serverQueryRequests[data.arg];
    if (!deferred) {
      return;
    }

    delete serverQueryRequests[data.arg];

    deferred.reject(new Error('Query failed'));
  });
}



window.addEventListener('message', (event: MessageEvent) => {
  console.log('[WNDMSG]', event.data);

  const { data } = event;

  if (typeof data !== 'object') {
    return;
  }

  if (data === null) {
    return;
  }

  if (!data.type) {
    return;
  }

  events.emit(data.type, data);
});

type TPayload = Record<string, any>;
