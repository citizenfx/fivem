import { inject, injectable, named } from 'inversify';
import { makeAutoObservable } from 'mobx';

import { CurrentGameBrand, CurrentGameProtocol } from 'cfx/base/gameRuntime';
import { defineService, ServicesContainer } from 'cfx/base/servicesContainer';
import { IAccountService } from 'cfx/common/services/account/account.service';
import { AccountChangeEvent, SSOAuthCompleteEvent } from 'cfx/common/services/account/events';
import {
  IAccount,
  ILoginCredentials,
  ILoginResponse,
  IRegisterCredentials,
  IRegisterResponse,
  LoginStatus,
  RegisterStatus,
} from 'cfx/common/services/account/types';
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

  readonly SSOAuthComplete = new SingleEventEmitter<SSOAuthCompleteEvent>();

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

  async initializeSSOAuth() {
    mpMenu.openUrl(await this.createAuthURL());
  }

  getAvatarUrlForUser(template: string, size = 250): string {
    const prefix = template[0] === '/'
      ? BASE_URL
      : '';

    return prefix + template.replace('{size}', size.toString());
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

  async getEmailError(email: string, onlineCheck: boolean): Promise<string | null> {
    if (!email) {
      return 'Email can not be empty';
    }

    if (!EMAIL_REGEXP.test(email)) {
      return 'Email is invalid';
    }

    if (!onlineCheck) {
      return null;
    }

    try {
      const json = await fetcher.json(`${BASE_URL}/u/check_email.json?email=${email}`);

      if (json.success) {
        return null;
      }

      return json.errors[0];
    } catch (e) {
      if (fetcher.HttpError.is(e)) {
        return 'Unable to verify email';
      }

      console.error(e);

      return 'Unable to verify email, forum service is unavailable';
    }
  }

  async getUsernameError(username: string): Promise<string | null> {
    if (!username) {
      return 'Can\'t be empty';
    }

    if (username.length < 3) {
      return 'Too short';
    }

    try {
      const json = await fetcher.json(`https://forum.cfx.re/u/check_username.json?username=${username}`);

      if (json.available) {
        return null;
      }

      const addition = json.suggestion
        ? `, try ${json.suggestion}`
        : '';

      return `Unavailable${addition}`;
    } catch (e) {
      if (fetcher.HttpError.is(e)) {
        return 'Unable to verify username';
      }

      console.error(e);

      return 'Unable to verify username, forum service is unavailable';
    }
  }

  async resetPassword(email: string) {
    const csrf = await this.createCSRFToken();
    const ownershipTicket = await this.getOwnershipTicket();

    const passwordResetForm = new FormData();
    passwordResetForm.append('login', email);

    await fetcher.fetch(`${BASE_URL}/session/forgot_password`, {
      method: 'POST',
      headers: {
        'Cfx-Entitlement-Ticket': ownershipTicket,
        'x-requested-with': 'XMLHttpRequest',
        'discourse-present': 'true',
        'x-csrf-token': csrf,
      },
      credentials: 'include',
      body: passwordResetForm,
    });
  }

  async login(credentials: ILoginCredentials): Promise<ILoginResponse> {
    const {
      email,
      password,
      totp,
    } = credentials;

    if (!email || !password) {
      return {
        status: LoginStatus.Error,
        error: 'Enter valid email and password',
      };
    }

    try {
      const session = await this.createSession(credentials);

      if (session.error) {
        if (session.reason === 'invalid_second_factor' && !totp) {
          return {
            status: LoginStatus.TOTPRequest,
          };
        }

        return {
          status: LoginStatus.Error,
          error: session.error,
        };
      }

      await this.syntheticAuth();
      await this.loadCurrentAccount();

      return {
        status: LoginStatus.Success,
      };
    } catch (e) {
      console.warn(e);

      return {
        status: LoginStatus.Error,
        error: 'Failed to Authenticate - Try Again Later',
      };
    }
  }

  async register(credentials: IRegisterCredentials): Promise<IRegisterResponse> {
    const ownershipTicket = await this.getOwnershipTicket();

    try {
      const {
        success,
        message,
      } = await this.performRegistration(credentials, ownershipTicket);

      if (success) {
        return {
          status: RegisterStatus.Success,
        };
      }

      const response: IRegisterResponse = {
        status: RegisterStatus.Error,
        error: 'Failed to register account, please try again later',
      };

      if (success === false) {
        response.error = message;
      }

      return response;
    } catch (e) {
      return {
        status: RegisterStatus.Error,
        error: 'Failed to register account, please try again later',
      };
    }
  }

  async resendActivationEmail(username: string) {
    const formData = new FormData();

    formData.append('username', username);

    await window.fetch(`${BASE_URL}/u/action/send_activation_email`, {
      method: 'POST',
      headers: {
        'Cfx-Entitlement-Ticket': await this.getOwnershipTicket(),
        'x-requested-with': 'XMLHttpRequest',
        'discourse-present': 'true',
        'x-csrf-token': await this.createCSRFToken(),
      },
      credentials: 'include',
      body: formData,
    });
  }

  async createAuthURL(): Promise<string> {
    invariant(CurrentGameProtocol, 'Current game does not support link protocol');

    const [rsaKeys] = await Promise.all([getOrCreateRSAKeys(), mpMenu.computerName.resolved()]);

    this.recreateClientId();

    const params = {
      scopes: 'session_info,read,write',
      client_id: this.clientId,
      nonce: this.createNonce(),
      auth_redirect: `${CurrentGameProtocol}://accept-auth`,
      application_name: `${CurrentGameBrand} client on ${mpMenu.computerName.value}`,
      public_key: rsaKeys.public,
    };

    return `${BASE_URL}/user-api-key/new?${serializeQueryString(params)}`;
  }

  async applyAuthPayload(payload: string): Promise<void> {
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

  private async syntheticAuth() {
    const ownershipTicket = await this.getOwnershipTicket();

    const authKeyResponse = await fetcher.fetch(`${BASE_URL}/user-api-key`, {
      method: 'POST',
      headers: {
        'x-requested-with': 'XMLHttpRequest',
        'discourse-present': 'true',
        'Cfx-Entitlement-Ticket': ownershipTicket,
      },
      body: await parseAuthFormDataFromURL(await this.createAuthURL(), ownershipTicket),
    });

    const callbackURL = new URL(authKeyResponse.url);
    const payload = callbackURL.searchParams.get('payload');

    if (!payload) {
      throw new Error('Invalid auth payload');
    }

    await this.applyAuthPayload(payload);
  }

  private async createCSRFToken(): Promise<string> {
    const {
      csrf,
    } = await fetcher.json(`${BASE_URL}/session/csrf.json`, {
      method: 'GET',
      headers: {
        'x-requested-with': 'XMLHttpRequest',
        'discourse-present': 'true',
        'Cfx-Entitlement-Ticket': await this.getOwnershipTicket(),
      },
      credentials: 'include',
    });

    return csrf;
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

  private async performRegistration(
    credentials: IRegisterCredentials,
    ownershipTicket: string,
  ): Promise<{ success: boolean; message: string }> {
    const {
      email,
      password,
      username,
    } = credentials;

    const secrets = await getRegistrationSecrets(ownershipTicket);

    const formData = new FormData();
    formData.append('email', email);
    formData.append('username', username);
    formData.append('password', password);
    formData.append('password_confirmation', secrets.value);
    formData.append('challenge', [...secrets.challenge].reverse().join(''));

    try {
      return await fetcher.json(`${BASE_URL}/u`, {
        method: 'POST',
        headers: {
          'Cfx-Entitlement-Ticket': ownershipTicket,
          'x-requested-with': 'XMLHttpRequest',
          'discourse-present': 'true',
          'x-csrf-token': await this.createCSRFToken(),
        },
        credentials: 'include',
        body: formData,
      });
    } catch (e) {
      console.error('Failed to register new account', e);

      throw e;
    }
  }

  private async createSession(credentials: ILoginCredentials) {
    const {
      email,
      password,
      totp,
    } = credentials;

    const body = new FormData();
    body.append('login', email);
    body.append('password', password);
    body.append('second_factor_method', '1');

    if (totp) {
      body.append('second_factor_token', totp);
    }

    // eslint-disable-next-line no-return-await
    return await fetcher.json(`${BASE_URL}/session`, {
      method: 'POST',
      headers: {
        'x-requested-with': 'XMLHttpRequest',
        'discourse-present': 'true',
        'x-csrf-token': await this.createCSRFToken(),
        'Cfx-Entitlement-Ticket': await this.getOwnershipTicket(),
      },
      credentials: 'include',
      body,
    });
  }

  private async getOwnershipTicket(): Promise<string> {
    await this.convarsService.whenPopulated();

    return this.convarsService.get('cl_ownershipTicket');
  }

  private readonly handleExternalAuthPayload = async (data: { data: string }) => {
    const payload = new URL(`http://dummy/?${data.data}`).searchParams.get('payload');

    if (!payload) {
      this.handleExternalAuthFail('Failed to authenticate: invalid payload, please try again');

      return;
    }

    try {
      await this.applyAuthPayload(payload);

      const loadCurrentAccountShouldThrowOnError = true;
      await this.loadCurrentAccount(loadCurrentAccountShouldThrowOnError);

      this.SSOAuthComplete.emit(SSOAuthCompleteEvent.success());
    } catch (e) {
      this.handleExternalAuthFail(e.message);
    }
  };

  private handleExternalAuthFail(error: string) {
    this.SSOAuthComplete.emit(SSOAuthCompleteEvent.error(error));
  }
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

// eslint-disable-next-line no-useless-escape, @stylistic/max-len
const EMAIL_REGEXP = /^[a-zA-Z0-9!#\$%&'*+\/=?\^_`{|}~\-]+(?:\.[a-zA-Z0-9!#\$%&'\*+\/=?\^_`{|}~\-]+)*@(?:[a-zA-Z0-9](?:[a-zA-Z0-9\-]*[a-zA-Z0-9])?\.)+[a-zA-Z0-9](?:[a-zA-Z0-9\-]*[a-zA-Z0-9])?$/;

async function parseAuthFormDataFromURL(authUrl: string, ownershipTicket: string): Promise<FormData> {
  const authPageString = await fetcher.text(authUrl, {
    method: 'GET',
    headers: {
      'x-requested-with': 'XMLHttpRequest',
      'discourse-present': 'true',
      'Cfx-Entitlement-Ticket': ownershipTicket,
    },
    credentials: 'include',
  });

  const authPage = new DOMParser().parseFromString(authPageString, 'text/html');
  const authFormData = new FormData();

  authFormData.append(
    'authenticity_token',
    (<HTMLInputElement>authPage.querySelector('input[name="authenticity_token"]')).value,
  );
  authFormData.append(
    'application_name',
    (<HTMLInputElement>authPage.querySelector('input[name="application_name"]')).value,
  );
  authFormData.append('nonce', (<HTMLInputElement>authPage.querySelector('input[name="nonce"]')).value);
  authFormData.append('client_id', (<HTMLInputElement>authPage.querySelector('input[name="client_id"]')).value);
  authFormData.append('auth_redirect', 'https://nui-game-internal/ui/app/index.html');
  authFormData.append('public_key', (<HTMLInputElement>authPage.querySelector('input[name="public_key"]')).value);
  authFormData.append('scopes', (<HTMLInputElement>authPage.querySelector('input[name="scopes"]')).value);
  authFormData.append('commit', 'Authorize');

  return authFormData;
}

async function getRegistrationSecrets(
  ownershipTicket: string,
): Promise<{ value: string; challenge: string | string[] }> {
  try {
    const secrets = await fetcher.json(`${BASE_URL}/session/hp.json`, {
      method: 'GET',
      headers: {
        'x-requested-with': 'XMLHttpRequest',
        'discourse-present': 'true',
        'Cfx-Entitlement-Ticket': ownershipTicket,
      },
      credentials: 'include',
    });

    if (typeof secrets !== 'object' || secrets === null) {
      throw new TypeError(`Invalid secrets response: "${JSON.stringify(secrets)}"`);
    }

    if (typeof secrets.value !== 'string') {
      throw new TypeError(`Invalid secrets response, .value must be a string: "${JSON.stringify(secrets)}"`);
    }

    if (!Array.isArray(secrets.challenge) && typeof secrets.challenge !== 'string') {
      throw new TypeError(
        `Invalid secrets response, .challenge must be a string or array: "${JSON.stringify(secrets)}"`,
      );
    }

    return secrets;
  } catch (e) {
    console.error('Failed to get secrets for account registration', e);

    throw e;
  }
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
