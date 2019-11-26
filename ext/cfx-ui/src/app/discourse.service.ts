import { Injectable, EventEmitter } from '@angular/core';
import { Server } from './servers/server';
import { ServersService } from './servers/servers.service';

import * as forge from 'node-forge';
import * as query from 'query-string';
import { BehaviorSubject } from 'rxjs';
import { GameService } from './game.service';

class RSAKeyCollection {
    public: string;
    private: string;
}

const randomBytes = function(length) {
    return Array(length + 1)
        .join('x')
        .replace(/x/g, c => {
            return Math.floor(Math.random() * 16).toString(16);
        });
};


export class BoostData {
    power: number;
    source: string;
    server: Server;
    address: string;
}

@Injectable()
export class DiscourseService {
    private static BASE_URL = 'https://forum.fivem.net';

    private rsaKeys: RSAKeyCollection;
    private clientId: string;
    private nonce: string;

    private ownershipTicket: string;

    private authToken: string;
    private computerName = 'UNKNOWN';

    public messageEvent = new EventEmitter<string>();
    public signinChange = new BehaviorSubject<any>(null);

    public currentUser: any;

    public currentBoost: BoostData;
    public noCurrentBoost = false;

    public constructor(private serversService: ServersService, private gameService: GameService) {
        this.authToken = window.localStorage.getItem('discourseAuthToken');

        if (this.authToken && this.authToken.length > 0) {
            this.getCurrentUser().then(user => {
                this.signinChange.next(user);

                this.currentUser = user;
            });
        }

        this.signinChange.subscribe(user => {
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
                        .filter(server => server != null && server.address === result.data.address)
                        .subscribe(a => {
                            this.currentBoost.server = a;
                        });
                } else if (result.status === 404) {
                    this.noCurrentBoost = true;
                }
            });
        });

		this.signinChange.subscribe(identity => {
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
    }

    private setComputerName(computerName: string) {
        this.computerName = computerName;
    }

    private setOwnershipTicket(ticket: string) {
        this.ownershipTicket = ticket;
    }

    public async apiCall(path: string, method?: string, data?: any) {
        console.log(`calling ${DiscourseService.BASE_URL}${path}`);

        const clientId = await this.getClientId();

        const finalMethod = method || 'GET';

        const headers = {
            'User-Agent': 'CitizenFX/Five',
            'Content-Type': 'application/json',
            'User-Api-Client-Id': clientId,
            'User-Api-Key': this.authToken
        };

        const finalData = (data) ? JSON.stringify(data) : undefined;

        const req = new Request(DiscourseService.BASE_URL + path, {
            headers,
            method: finalMethod,
            body: finalData
        })

        const res = await window.fetch(req);

        if (res.ok) {
            return await res.json();
        }

        if (res.status === 403) {
            window.localStorage.setItem('discourseAuthToken', '');
            this.authToken = '';

            this.signinChange.next(null);
            this.currentUser = null;

            throw new Error('User was logged out.');
            return;
        }

        throw new Error('Failed to fetch API, status code: ' + res.status);
    }

    public async externalCall(url: string, method?: string, data?: any) {
        const clientId = await this.getClientId();

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

        const res = await window.fetch(req);

        return {
            status: res.status,
            data: await res.json()
        };
    }

    public async getCurrentUser() {
        if (!this.authToken || this.authToken.length === 0) {
            return null;
        }

        const apiResponse = await this.apiCall('/session/current.json');
        const userResponse = await this.apiCall(`/u/${apiResponse.current_user.username}.json`);

        return {
            userId: apiResponse.current_user.id,
            username: apiResponse.current_user.username,
            isStaff: (apiResponse.current_user.admin || apiResponse.current_user.moderator),
            isPremium: (userResponse.user.groups.filter(group => group.name.startsWith('premium_')).length > 0)
        };
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

            this.messageEvent.emit(`Thanks for linking your FiveM user account, ${userInfo.username}.`);
            this.signinChange.next(userInfo);

            this.currentUser = userInfo;
        } catch (e) {
            console.log(e);

            this.messageEvent.emit('Authentication failure connecting to FiveM account.');
        }
    }

    public async generateAuthURL() {
        const rsaKeys = await this.ensureRSAKeys();
        const clientId = await this.getClientId();
        const nonce = await this.generateNonce();

        const deviceName = `FiveM client on ${await this.getComputerName()}`;

        const params = {
            scopes: 'session_info,read',
            client_id: clientId,
            nonce: nonce,
            auth_redirect: 'fivem://accept-auth',
            application_name: deviceName,
            public_key: rsaKeys.public
        };

        return `${DiscourseService.BASE_URL}/user-api-key/new?${this.serializeParams(params)}`;
    }

    public getToken() {
        return this.authToken;
    }

    public getExtClientId() {
        return this.clientId;
    }

    private async generateNonce() {
        this.nonce = randomBytes(16);
        window.localStorage.setItem('lastAuthNonce', this.nonce);

        return this.nonce;
    }

    private async getClientId() {
        if (this.clientId) {
            return this.clientId;
        }

        const clientId = window.localStorage.getItem('clientId');

        if (clientId && clientId.length > 0) {
            this.clientId = clientId;
            return clientId;
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
                workers: -1
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

    private serializeParams(obj: {[key: string]: string}) {
        return Object.entries(obj)
            .map(([k, v]) => `${encodeURIComponent(k)}=${encodeURIComponent(v)}`)
            .join('&');
    }

    private async getComputerName() {
        return this.computerName;
    }
}
