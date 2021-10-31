import { Injectable, EventEmitter } from '@angular/core';
import { Server } from './servers/server';
import { ServersService } from './servers/servers.service';

import * as forge from 'node-forge';
import * as query from 'query-string';
import { BehaviorSubject, Observable } from 'rxjs';
import { GameService } from './game.service';
import { environment } from 'environments/environment';
import { HttpClient, HttpHeaders, HttpRequest } from '@angular/common/http';
import { Site } from './servers/components/reviews/discourse-models';
import { ForumSignoutInterceptorState } from './http-interceptors';

class RSAKeyCollection {
	public: string;
	private: string;
}

const randomBytes = function (length) {
	return Array(length + 1)
		.join('x')
		.replace(/x/g, c => {
			return Math.floor(Math.random() * 16).toString(16);
		});
};

export class DiscourseUser {
	id: number;
	username: string;
	avatarTemplate: string;

	isStaff: boolean;
	isPremium: boolean;

	getAvatarUrl(size = 250): string {
		return DiscourseService.getAvatarUrlForUser(this.avatarTemplate, size);
	}

	getAvatarUrlForCss(size = 250): string {
		return `url(${this.getAvatarUrl()})`;
	}

	public static fromApi(apiRes, userRes): DiscourseUser {
		const user = new DiscourseUser();

		user.id = apiRes.current_user.id;
		user.username = apiRes.current_user.username;
		user.avatarTemplate = apiRes.current_user.avatar_template;
		user.isStaff = (apiRes.current_user.admin || apiRes.current_user.moderator);
		user.isPremium = (userRes.user.groups.filter(group => group.name.startsWith('premium_')).length > 0);

		return user;
	}
}

export class BoostData {
	power: number;
	source: string;
	server: Server;
	address: string;
}

export enum DiscourseAuthModalState {
    INITIAL = 'initial',
    SHOWN = 'shown',
    IGNORE = 'ignore',
}

const DISCOURSE_AUTH_MODAL_STATE = 'discourseAuthModalState';

@Injectable()
export class DiscourseService {
	private static BASE_URL = 'https://forum.cfx.re';

	private rsaKeys: RSAKeyCollection;
	private clientId: string;
	private nonce: string;

	private ownershipTicket: string;

	private authToken: string;
	private computerName = 'UNKNOWN';

	public messageEvent = new EventEmitter<string>();
	public signinChange = new BehaviorSubject<any>(null);
    public initialAuthComplete = new BehaviorSubject(false);

	public currentUser: DiscourseUser;

	public currentBoost: BoostData;
	public noCurrentBoost = false;

    public authModalState: BehaviorSubject<DiscourseAuthModalState>;
    public authModalOpenChange = new BehaviorSubject<boolean>(true);
    public authModalClosedEvent = new EventEmitter<{ where?: string, ignored?: boolean }>();

	public siteData: Observable<Site>;

	static getAvatarUrlForUser(template: string, size = 250): string {
		const prefix = template[0] === '/' ? 'https://forum.cfx.re' : '';
		return prefix + template.replace('{size}', size.toString());
	}

	public constructor(private serversService: ServersService, private gameService: GameService,
		private signoutInterceptor: ForumSignoutInterceptorState, private http: HttpClient) {
        const storedAuthModalState = window.localStorage.getItem(DISCOURSE_AUTH_MODAL_STATE) as any;

        this.authModalState = new BehaviorSubject(
            Object.values(DiscourseAuthModalState).includes(storedAuthModalState)
                ? storedAuthModalState
                : DiscourseAuthModalState.INITIAL,
        );

        if (this.authModalState.getValue() === DiscourseAuthModalState.IGNORE) {
            this.authModalOpenChange.next(false);
        }

		this.authToken = window.localStorage.getItem('discourseAuthToken');

		if (this.authToken && this.authToken.length > 0) {
			this.getCurrentUser().then(user => {
				this.signinChange.next(user);

				this.currentUser = user;

                if (user) {
                    this.closeAuthModal();
                }

                this.initialAuthComplete.next(true);
			}).catch(() => {
                // failed, perhaps as user revoked token?
                this.initialAuthComplete.next(true);
            });
		} else {
            this.initialAuthComplete.next(true);
        }

		this.initialAuthComplete.subscribe(complete => {
			if (!complete) {
				return;
			}

			if (environment.web) {
				return;
			}

			this.siteData = this.apiCallObservable<Site>('/site');

			if (!this.currentUser) {
				return;
			}

			this.externalCall('https://servers-frontend.fivem.net/api/upvote/', 'GET').then(result => {
				if (result.status < 400) {
					this.currentBoost = {
						address: result.data.address,
						power: result.data.power,
						source: result.data.source,
						server: null
					};

					this.noCurrentBoost = false;

					this.serversService
						.getReplayedServers()
						.filter(server => server != null && server.EndPoint === result.data.address)
						.subscribe(a => {
							this.currentBoost.server = this.serversService.getMaterializedServer(a);
						});
				} else if (result.status === 404) {
					this.noCurrentBoost = true;
				}
			});
		});

		this.signinChange.subscribe(() => {
			this.gameService.setDiscourseIdentity(this.getToken(), this.getExtClientId());
		});

		this.messageEvent.subscribe((msg: string) => {
			this.gameService.invokeInformational(msg);
		});

		this.gameService.computerNameChange.subscribe((data: string) => {
			this.setComputerName(data);
		});

		this.gameService.ownershipTicketChange.subscribe((ticket: string) => {
			this.setOwnershipTicket(ticket);
		});

		this.gameService.authPayloadSet.subscribe((payload: string) => {
			this.handleAuthPayload(payload);
		})

		this.signoutInterceptor.onSignout.subscribe(() => {
			this.handleSignout();
		});
	}

	handleSignout() {
		window.localStorage.removeItem('discourseAuthToken');
		this.authToken = null;

		this.signinChange.next(null);
		this.currentUser = null;
	}

	private setComputerName(computerName: string) {
		this.computerName = computerName;
	}

	private setOwnershipTicket(ticket: string) {
		this.ownershipTicket = ticket;
	}

	public apiCallObservable<T>(path: string, method?: string, data?: any) {
		const clientId = this.getClientId();

		const finalMethod = method || 'GET';

		const headers: Record<string, string> = {
			'Content-Type': 'application/json',
			'Accept': 'application/json',
		};

		if (clientId && this.authToken) {
			headers['User-Api-Client-Id'] = clientId;
			headers['User-Api-Key'] = this.authToken;
		}

		const finalData = (data) ? JSON.stringify(data) : undefined;

		return this.http.request<T>(
			finalMethod,
			DiscourseService.BASE_URL + path,
			{
				headers: new HttpHeaders(headers),
				responseType: 'json',
				body: finalData,
			}
		);
	}

	public async apiCall(path: string, method?: string, data?: any) {
		const call = this.apiCallObservable<any>(path, method, data);
		const response = (await call.toPromise());

		return response;
	}

	public async externalCall(url: string, method?: string, data?: any) {
		const clientId = this.getClientId();

		const finalMethod = method || 'GET';

		const headers = {
			'User-Agent': 'CitizenFX/Five',
			'Content-Type': 'application/json',
			'User-Api-Client-Id': clientId,
			'User-Api-Key': this.authToken,
			'Cfx-Entitlement-Ticket': this.ownershipTicket,
		};

		const finalData = (data) ? JSON.stringify(data) : undefined;

		const req = new Request(url, {
			headers,
			method: finalMethod,
			body: finalData
		})

		try {
			const res = await window.fetch(req);

			let json = null;

			try {
				json = await res.json();
			} catch { }

			return {
				status: res.status,
				data: json || null
			};
		} catch {
			return {
				status: 503,
				data: null
			};
		}
	}

	public async getCurrentUser() {
		if (!this.authToken || this.authToken.length === 0) {
			return null;
		}

		const apiResponse = await this.apiCall('/session/current.json');
		const userResponse = await this.apiCall(`/u/${apiResponse.current_user.username}.json`);

		return DiscourseUser.fromApi(apiResponse, userResponse);
	}

	private async handleAuthPayload(queryString: string) {
		const parts = query.parse(queryString);
		const payload = parts['payload'] as string;

		const rsaKeys = await this.ensureRSAKeys();
		const privateKey = forge.pki.privateKeyFromPem(rsaKeys.private) as forge.pki.rsa.PrivateKey;
		const decrypted = JSON.parse(privateKey.decrypt(forge.util.decode64(payload)));

		this.nonce = window.localStorage.getItem('lastAuthNonce');

		if (decrypted.nonce !== this.nonce) {
			this.messageEvent.emit('We were not expecting this reply - please try connecting your account again.');
			return;
		}

		this.authToken = decrypted.key;

		window.localStorage.setItem('discourseAuthToken', this.authToken);

		try {
			const userInfo = await this.getCurrentUser();

			// this.messageEvent.emit(`Thanks for linking your FiveM user account, ${userInfo.username}.`);
			this.signinChange.next(userInfo);
			this.initialAuthComplete.next(true);

			this.currentUser = userInfo;
		} catch (e) {
			console.log(e);

			this.messageEvent.emit('Authentication failure connecting to FiveM account.');
		}
	}

	public async generateAuthURL() {
		const rsaKeys = await this.ensureRSAKeys();
		const clientId = this.getClientId(true);
		const nonce = await this.generateNonce();

		const deviceName = `FiveM client on ${await this.getComputerName()}`;

		const params = {
			scopes: 'session_info,read,write',
			client_id: clientId,
			nonce: nonce,
			auth_redirect: 'fivem://accept-auth',
			application_name: deviceName,
			public_key: rsaKeys.public
		};

		return `${DiscourseService.BASE_URL}/user-api-key/new?${this.serializeParams(params)}`;
	}

    private async createCSRFToken() {
		const csrfRes = await window.fetch('https://forum.cfx.re/session/csrf.json', {
			method: 'GET',
			headers: {
				"x-requested-with": 'XMLHttpRequest',
				"discourse-present": 'true',
				'Cfx-Entitlement-Ticket': this.ownershipTicket,
			},
			credentials: 'include',
		});

		return await csrfRes.json();

	}

	public async login(login: string, pw: string, totp?: string) {
		if (login === '' || pw === '') {
			return { error: 'Enter a valid email and password' };
		}

		const csrf = await this.createCSRFToken();

		const loggingInFormData = new FormData();
		loggingInFormData.append('login', login);
		loggingInFormData.append('password', pw);
        loggingInFormData.append('second_factor_method', '1');

        if (totp && totp !== '') {
            loggingInFormData.append('second_factor_token', totp);
        }

		const loggingUserIn = await window.fetch('https://forum.cfx.re/session', {
			method: 'POST',
			headers: {
				"x-requested-with": 'XMLHttpRequest',
				"discourse-present": 'true',
				"x-csrf-token": csrf.csrf,
				'Cfx-Entitlement-Ticket': this.ownershipTicket,
			},
			credentials: 'include',
			body: loggingInFormData
		});

		let user = await loggingUserIn.json();

		if (user.error) {
			return user;
		} else {

			try {
				const authURL = await this.generateAuthURL();
				const authRes = await window.fetch(authURL, {
					method: 'GET',
					headers: {
						"x-requested-with": 'XMLHttpRequest',
						"discourse-present": 'true',
						'Cfx-Entitlement-Ticket': this.ownershipTicket,
					},
					credentials: 'include',
				});

				const authHTML = await authRes.text();
				const parser = new DOMParser();
				const authDoc = parser.parseFromString(authHTML, 'text/html');
				const authFormData = new FormData();
				authFormData.append('authenticity_token', (<HTMLInputElement>authDoc.querySelector('input[name="authenticity_token"]')).value);
				authFormData.append('application_name', (<HTMLInputElement>authDoc.querySelector('input[name="application_name"]')).value);
				authFormData.append('nonce', (<HTMLInputElement>authDoc.querySelector('input[name="nonce"]')).value);
				authFormData.append('client_id', (<HTMLInputElement>authDoc.querySelector('input[name="client_id"]')).value);
				authFormData.append('auth_redirect', 'https://nui-game-internal/ui/app/index.html');
				authFormData.append('public_key', (<HTMLInputElement>authDoc.querySelector('input[name="public_key"]')).value);
				authFormData.append('scopes', (<HTMLInputElement>authDoc.querySelector('input[name="scopes"]')).value);
				authFormData.append('commit', 'Authorize');

				const authKeyRes = await fetch('https://forum.cfx.re/user-api-key', {
					method: 'POST',
					headers: {
						"x-requested-with": 'XMLHttpRequest',
						"discourse-present": 'true',
						'Cfx-Entitlement-Ticket': this.ownershipTicket,
					},
					body: authFormData
				});

				const payload = authKeyRes.url.split("?").pop();
				this.handleAuthPayload(payload);

				return user;

			} catch (err) {
				return { error: 'Failed to Authenticate - Try Again Later' };
			}

		}

	}

	public async registerNewUser(newUserInfo) {
		const { email, username, password } = newUserInfo;
		const secrectStrings = await (await window.fetch('https://forum.cfx.re/session/hp.json', {
			method: 'GET',
			headers: {
				"x-requested-with": 'XMLHttpRequest',
				"discourse-present": 'true',
				'Cfx-Entitlement-Ticket': this.ownershipTicket,
			},
			credentials: 'include',
		})).json();

		const newAccountFormData = new FormData();
		newAccountFormData.append('email', email);
		newAccountFormData.append('password', password);
		newAccountFormData.append('username', username);
		newAccountFormData.append('password_confirmation', secrectStrings.value);
		newAccountFormData.append('challenge', [...secrectStrings.challenge].reverse().join(''));

		const csrf = await this.createCSRFToken();

		const newUser = await window.fetch('https://forum.cfx.re/u', {
			method: 'POST',
			headers: {
				'Cfx-Entitlement-Ticket': this.ownershipTicket,
				"x-requested-with": 'XMLHttpRequest',
				"discourse-present": 'true',
				"x-csrf-token": csrf.csrf,
			},
			credentials: 'include',
			body: newAccountFormData
		});

		return await newUser.json();
	}

	public async resendActivationEmail(username: string) {
		const csrf = await this.createCSRFToken();

		const newUserForm = new FormData();
		newUserForm.append('username', username);

		await window.fetch('https://forum.cfx.re/u/action/send_activation_email', {
			method: 'POST',
			headers: {
				'Cfx-Entitlement-Ticket': this.ownershipTicket,
				"x-requested-with": 'XMLHttpRequest',
				"discourse-present": 'true',
				"x-csrf-token": csrf.csrf,
			},
			credentials: 'include',
			body: newUserForm
		});

		return;

	}

	public async resetPassword(email: string) {
		try {
			const csrf = await this.createCSRFToken();
			const passwordResetForm = new FormData();
			passwordResetForm.append('login', email);

			await window.fetch('https://forum.cfx.re/session/forgot_password', {
				method: 'POST',
				headers: {
					'Cfx-Entitlement-Ticket': this.ownershipTicket,
					"x-requested-with": 'XMLHttpRequest',
					"discourse-present": 'true',
					"x-csrf-token": csrf.csrf,
				},
				credentials: 'include',
				body: passwordResetForm
			});

			return;

		} catch (err) {
			return { error: true };
		}
	}

	public async checkValidEmail(email: string) {
		return await (await window.fetch(`https://forum.cfx.re/u/check_email.json?email=${email}`)).json();
	}

	public async checkValidUsername(username: string) {
		return await (await window.fetch(`https://forum.cfx.re/u/check_username.json?username=${username}`)).json();
	}

	public getToken() {
		return this.authToken;
	}

	public getExtClientId() {
		return this.clientId;
	}

    public openAuthModal() {
        this.authModalOpenChange.next(true);
    }

    public closeAuthModal(analyticsName?: string) {
        this.authModalOpenChange.next(false);

        if (this.authModalState.getValue() === DiscourseAuthModalState.INITIAL) {
            this.setAuthModalState(DiscourseAuthModalState.SHOWN);
        }

        if (analyticsName) {
            this.authModalClosedEvent.next({
                where: analyticsName,
            });
        }
    }

    public closeAuthModalAndIgnore() {
        this.authModalOpenChange.next(false);
        this.setAuthModalState(DiscourseAuthModalState.IGNORE);

        this.authModalClosedEvent.next({
            ignored: true,
        });
    }

    private setAuthModalState(state: DiscourseAuthModalState) {
        this.authModalState.next(state);
        window.localStorage.setItem(DISCOURSE_AUTH_MODAL_STATE, state);
    }

	private async generateNonce() {
		this.nonce = randomBytes(16);
		window.localStorage.setItem('lastAuthNonce', this.nonce);

		return this.nonce;
	}

	private getClientId(regenerate?: boolean) {
		if (!regenerate) {
			if (this.clientId) {
				return this.clientId;
			}

			const clientId = window.localStorage.getItem('clientId');

			if (clientId && clientId.length > 0) {
				this.clientId = clientId;
				return clientId;
			}
		}

		this.clientId = randomBytes(32);
		window.localStorage.setItem('clientId', this.clientId);

		return this.clientId;
	}

	private async ensureRSAKeys() {
		if (this.rsaKeys) {
			return this.rsaKeys;
		}

		const keys = window.localStorage.getItem('rsaKeys');

		if (keys) {
			this.rsaKeys = JSON.parse(keys);
			return this.rsaKeys;
		}

		const promise = new Promise<RSAKeyCollection>((resolve, reject) => {
			forge.pki.rsa.generateKeyPair({
				bits: 2048,
				workers: -1,
				workerScript: require('file-loader!node-forge/dist/prime.worker.min.js').default
			}, (err, keypair) => {
				if (err) {
					reject(err);
					return;
				}

				const rkc = new RSAKeyCollection();
				rkc.private = forge.pki.privateKeyToPem(keypair.privateKey);
				rkc.public = forge.pki.publicKeyToPem(keypair.publicKey);
				resolve(rkc);
			});
		});

		const pair = await promise;
		this.rsaKeys = pair;

		window.localStorage.setItem('rsaKeys', JSON.stringify(pair));

		return pair;
	}

	private serializeParams(obj: { [key: string]: string }) {
		return Object.entries(obj)
			.map(([k, v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v)}`)
			.join('&');
	}

	private async getComputerName() {
		return this.computerName;
	}
}
