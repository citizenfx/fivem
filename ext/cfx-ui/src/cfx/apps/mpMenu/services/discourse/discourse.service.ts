import { inject, injectable, named } from 'inversify';
import { makeAutoObservable } from 'mobx';

import { CurrentGameBrand, CurrentGameProtocol } from 'cfx/base/gameRuntime';
import { defineService, ServicesContainer } from 'cfx/base/servicesContainer';
import { IAccountService } from 'cfx/common/services/account/account.service';
import { AccountChangeEvent, ExternalAuthCompleteEvent } from 'cfx/common/services/account/events';
import { IAccount } from 'cfx/common/services/account/types';
import { AppContribution, registerAppContribution } from 'cfx/common/services/app/app.extensions';
import { ScopedLogger } from 'cfx/common/services/log/scopedLogger';
import { Deferred } from 'cfx/utils/async';
import { fetcher } from 'cfx/utils/fetcher';
import { invariant } from 'cfx/utils/invariant';
import { ObservableAsyncValue } from 'cfx/utils/observable';
import { randomBytes } from 'cfx/utils/random';
import { SingleEventEmitter } from 'cfx/utils/singleEventEmitter';
import { serializeQueryString } from 'cfx/utils/url';

import { decryptBase64, getOrCreateRSAKeys } from './rsaKeys';
import { IDiscourse } from './types';
import { mpMenu } from '../../mpMenu';
import { ConvarService } from '../convars/convars.service';

export const IDiscourseService = defineService<IDiscourseService>('DiscourseService');
export type IDiscourseService = DiscourseService;

export function registerDiscourseService(container: ServicesContainer) {
  container.registerImpl(IAccountService, DiscourseService);
  container.registerImpl(IDiscourseService, DiscourseService);

  registerAppContribution(container, DiscourseService);
}

@injectable()
class DiscourseService implements IAccountService, AppContribution {
  @inject(ConvarService)
  protected readonly convarsService: ConvarService;

  @inject(ScopedLogger)
  @named('DiscourseService')
  protected readonly logService: ScopedLogger;

  public readonly siteData: ObservableAsyncValue<IDiscourse.Site>;

  readonly accountChange = new SingleEventEmitter<AccountChangeEvent>();

  readonly ExternalAuthProcessing = new SingleEventEmitter<void>();
  readonly ExternalAuthComplete = new SingleEventEmitter<ExternalAuthCompleteEvent>();

  private initialAuthCompleteDeferred = new Deferred<boolean>();

  private _account: Account | null = null;
  get account(): Account | null {
    return this._account;
  }
  private set account(account: Account | null) {
    this._account = account;
    this.accountChange.emit({ account });
  }

  private _accountLoadError: string | null = null;
  public get accountLoadError(): string | null {
    return this._accountLoadError;
  }
  private set accountLoadError(accountLoadError: string | null) {
    this._accountLoadError = accountLoadError;
  }

  private _accountLoadComplete = false;
  get accountLoadComplete(): boolean {
    return this._accountLoadComplete;
  }
  private set accountLoadComplete(loaded: boolean) {
    this._accountLoadComplete = loaded;
  }

  private _authToken: string = '';
  public get authToken(): string {
    return this._authToken;
  }
  private set authToken(authToken: string) {
    this._authToken = authToken;

    if (authToken) {
      window.localStorage.setItem(LSKeys.DISCOURSE_AUTH_TOKEN, authToken);
    } else {
      window.localStorage.removeItem(LSKeys.DISCOURSE_AUTH_TOKEN);
    }

    this.updateDiscourseIdentity();
  }

  private _clientId: string = '';
  public get clientId(): string {
    if (!this._clientId) {
      this.recreateClientId();
    }

    return this._clientId;
  }
  private set clientId(clientID: string) {
    this._clientId = clientID;

    window.localStorage.setItem(LSKeys.CLIENT_ID, clientID);
  }

  private recreateClientId() {
    this.clientId = randomBytes(32);
  }

  constructor() {
    makeAutoObservable(this);

    this.preinit();

    mpMenu.on('authPayload', this.handleExternalAuthPayload);

    this.siteData = ObservableAsyncValue.from(() => this.makeApiCall('/site'), {
      lazy: true,
    });
  }

  init() {
    this.loadCurrentAccount();
  }

  private preinit() {
    const savedAuthToken = window.localStorage.getItem(LSKeys.DISCOURSE_AUTH_TOKEN) || '';
    const savedClientId = window.localStorage.getItem(LSKeys.CLIENT_ID) || '';

    this.authToken = savedAuthToken;
    this.clientId = savedClientId;
  }

  initialAuthCompletePromise(): Promise<boolean> {
    return this.initialAuthCompleteDeferred.promise;
  }

  private async loadCurrentAccount(shouldThrowOnError = false) {
    try {
      if (this.authToken) {
        const apiResponse: ICurrentSessionResponse = await this.makeApiCall('/session/current.json');
        const userResponse: IUserResponse = await this.makeApiCall(`/u/${apiResponse.current_user.username}.json`);

        this.account = new Account(this, apiResponse, userResponse);

        if (this.accountLoadError) {
          this.accountLoadError = '';
        }
      }
    } catch (e) {
      let culprit = 'Unknown auth error';

      if (fetcher.HttpError.is(e)) {
        culprit = 'Discourse auth service unavailable';

        this.logService.error(e, {
          culprit,
          statusCode: e.status,
        });
      }

      this.accountLoadError = culprit;

      console.warn(culprit, e);

      if (shouldThrowOnError) {
        throw new Error(culprit);
      }
    } finally {
      if (!this.accountLoadComplete) {
        this.accountLoadComplete = true;
        this.initialAuthCompleteDeferred.resolve(!!this.account);
      }
    }
  }

  async initializeExternalAuth() {
    mpMenu.openUrl(await this.createAuthURL());
  }

  getAvatarUrlForUser(template: string, size = 250): string {
    const prefix = template[0] === '/'
      ? BASE_URL
      : '';

    return prefix + template.replace('{size}', size.toString());
  }

  getSignUpURL(): string {
    return `${BASE_URL}/signup`;
  }

  readonly signout = () => {
    this.account = null;
    this.authToken = '';
  };

  // eslint-disable-next-line default-param-last
  async makeApiCall<TRequest, TResponse>(path: string, method = 'GET', data?: TRequest): Promise<TResponse> {
    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
      Accept: 'application/json',
    };

    if (this.clientId && this.authToken) {
      headers['User-Api-Client-Id'] = this.clientId;
      headers['User-Api-Key'] = this.authToken;
    }

    try {
      return await fetcher.json(BASE_URL + path, {
        headers,
        method,
        body: data
          ? JSON.stringify(data)
          : undefined,
      });
    } catch (e) {
      if (fetcher.HttpError.is(e)) {
        if (e.status === 403) {
          this.signout();
        }
      }

      throw e;
    }
  }

  // eslint-disable-next-line default-param-last
  async makeExternalCall<TRequest, TResponse>(url: string, method = 'GET', data?: TRequest): Promise<TResponse> {
    const request = new Request(url, {
      method,
      headers: {
        'User-Agent': 'CitizenFX/Five',
        'Content-Type': 'application/json',
        'User-Api-Client-Id': this.clientId,
        'User-Api-Key': this.authToken,
        'Cfx-Entitlement-Ticket': await this.getOwnershipTicket(),
      },
      body: data
        ? JSON.stringify(data)
        : undefined,
    });

    try {
      return await fetcher.json<TResponse>(request);
    } catch (e) {
      if (fetcher.HttpError.is(e)) {
        if (e.status === 403 && isSignOutInducingUrl(url)) {
          this.signout();
        }
      }

      throw e;
    }
  }

  async createAuthURL(): Promise<string> {
    invariant(CurrentGameProtocol, 'Current game does not support link protocol');

    const rsaKeys = await getOrCreateRSAKeys();

    this.recreateClientId();

    const params = {
      scopes: 'session_info,read,write',
      client_id: this.clientId,
      nonce: this.createNonce(),
      auth_redirect: `${CurrentGameProtocol}://accept-auth`,
      application_name: `${CurrentGameBrand} client`,
      public_key: rsaKeys.public,
    };

    return `${BASE_URL}/user-api-key/new?${serializeQueryString(params)}`;
  }

  async applyExternalAuthPayload(payload: string): Promise<void> {
    const {
      key,
      nonce,
    } = JSON.parse(await decryptBase64(payload));

    if (nonce !== this.getLastNonce()) {
      throw new Error('We were not expecting this reply - please try connecting your account again.');
    }

    this.authToken = key;
  }

  private async updateDiscourseIdentity() {
    mpMenu.invokeNative(
      'setDiscourseIdentity',
      JSON.stringify({
        token: this.authToken,
        clientId: this.clientId,
      }),
    );
  }

  private createNonce(): string {
    return this.setLastNonce(randomBytes(16));
  }

  private setLastNonce(nonce: string): string {
    window.localStorage.setItem(LSKeys.LAST_AUTH_NONCE, nonce);

    return nonce;
  }

  private getLastNonce(): string {
    return window.localStorage.getItem(LSKeys.LAST_AUTH_NONCE) || '';
  }

  private async getOwnershipTicket(): Promise<string> {
    await this.convarsService.whenPopulated();

    return this.convarsService.get('cl_ownershipTicket');
  }

  private readonly handleExternalAuthPayload = async (data: { data: string }) => {
    const payload = new URL(`http://dummy/?${data.data}`).searchParams.get('payload');

    if (!payload) {
      this.ExternalAuthComplete.emit(ExternalAuthCompleteEvent.error(
        'Failed to authenticate: invalid payload, please try again',
      ));

      return;
    }

    this.ExternalAuthProcessing.emit();

    try {
      await this.applyExternalAuthPayload(payload);

      const loadCurrentAccountShouldThrowOnError = true;
      await this.loadCurrentAccount(loadCurrentAccountShouldThrowOnError);

      this.ExternalAuthComplete.emit(ExternalAuthCompleteEvent.success());
    } catch (e) {
      this.ExternalAuthComplete.emit(ExternalAuthCompleteEvent.error(e.message));
    }
  };
}

enum LSKeys {
  DISCOURSE_AUTH_TOKEN = 'discourseAuthToken',
  LAST_AUTH_NONCE = 'lastAuthNonce',
  CLIENT_ID = 'clientId',
}

const BASE_URL = 'https://forum.cfx.re';
const SIGN_OUT_INDUCING_URL_PARTS = ['forum.cfx.re', 'forum.fivem.net'];
function isSignOutInducingUrl(url: string): boolean {
  return SIGN_OUT_INDUCING_URL_PARTS.some((part) => url.includes(part));
}

type ICurrentSessionResponse = {
  current_user: IDiscourse.User;
};
type IUserResponse = {
  user: {
    groups: IDiscourse.Group[];
  };
};

export class Account implements IAccount {
  public readonly id: number;

  public readonly username: string;

  public readonly avatarTemplate: string;

  public readonly isStaff: boolean;

  public readonly isPremium: boolean;

  constructor(
    protected readonly accountService: IAccountService,
    apiResponse: ICurrentSessionResponse,
    userResponse: IUserResponse,
  ) {
    const {
      id,
      username,
      // eslint-disable-next-line camelcase
      avatar_template,
    } = apiResponse.current_user;

    this.id = id;
    this.username = username;
    // eslint-disable-next-line camelcase
    this.avatarTemplate = avatar_template;

    this.isStaff = Boolean(userResponse.user.groups.find((group) => group.name === 'staff'));
    this.isPremium = Boolean(userResponse.user.groups.find((group) => group.name.startsWith('premium_')));
  }

  getAvatarUrl(size = 250): string {
    return this.accountService.getAvatarUrlForUser(this.avatarTemplate, size);
  }

  getAvatarUrlForCss(size = 250): string {
    return `url(${this.getAvatarUrl(size)})`;
  }
}
