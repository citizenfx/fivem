import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { DomSanitizer } from '@angular/platform-browser';

import * as unicodeSubstring from 'unicode-substring';

import 'rxjs/add/operator/toPromise';
import { Observable, Subject } from 'rxjs';

export class Tweet {
    readonly user_displayname: string;
    readonly user_screenname: string;
    readonly rt_displayname: string;
    readonly rt_screenname: string;
    readonly content: string;
    readonly date: Date;
    readonly avatar: string;
    readonly id: string;
    readonly url: string;

    image: string;

    constructor(json: any) {
        if (json.retweeted_status) {
            this.rt_displayname = json.user.name;
            this.rt_screenname = json.user.screen_name;

            json = json.retweeted_status;
        }

        this.user_displayname = json.user.name;
        this.user_screenname = json.user.screen_name;

        this.content = this.parseEntities(json.text, json.entities);

        this.date = new Date(json.created_at);
        this.avatar = json.user.profile_image_url_https;
        this.id = json.id_str;
        this.url = json.url || ('https://twitter.com/_FiveM/status/' + this.id);
    }

    // based on https://gist.github.com/darul75/88fc42a21f6113708a0b
    // * Copyright 2010, Wade Simmons
    // * Licensed under the MIT license
    private parseEntities(content: string, entities: any): string {
        if (!entities) {
            return content;
        }

        const index_map = {};

        entities.urls.forEach(entry => index_map[entry.indices[0]] = [entry.indices[1], text => entry.expanded_url]);

        (entities.media || []).forEach(entry => {
            if (entry.type === 'photo') {
                this.image = entry.media_url_https || entry.media_url;
            }

            index_map[entry.indices[0]] = [entry.indices[1], text => ''];
        });

        let result = '';
        let last_i = 0;
        let i = 0;

        // iterate through the string looking for matches in the index_map
        for (i = 0; i < content.length; ++i) {
            const ind = index_map[i];
            if (ind) {
                const end = ind[0];
                const func = ind[1];
                if (i > last_i) {
                    result += unicodeSubstring(content, last_i, i);
                }
                result += func(unicodeSubstring(content, i, end));
                i = end - 1;
                last_i = end;
            }
        }

        if (i > last_i) {
            result += unicodeSubstring(content, last_i, i);
        }

        return result;
    }
}

@Injectable()
export class TweetService {
    private sentMap = new Set<string>();

    constructor(private http: HttpClient) { }

    public getTweets(uri: string): Promise<Tweet[]> {
        return this.http.get(uri)
            .toPromise()
            .then((result: any) => result
                .filter(t => !t.in_reply_to_user_id)
                .map(t => new Tweet(t)));
    }

    public getActivityTweets(pubs: string[]): Observable<Tweet> {
        const subject = new Subject<Tweet>();
        this.sentMap.clear();

        for (const pub of pubs) {
            this.fetchPub(pub, subject);
        }

        return subject;
    }

    private async fetchPub(pub: string, subject: Subject<Tweet>) {
        const domainPart = pub.match(/@(.*?)$/);

        if (!domainPart || !domainPart[1]) {
            return;
        }

        const part = domainPart[1];

        const response: any = await this.http.get(`https://${part}/.well-known/webfinger?resource=${pub}`, {
            responseType: 'json'
        }).toPromise();

        const activityDesc = response.links.find((a: any) => a.rel === 'self' && a.type === 'application/activity+json');

        if (!activityDesc) {
            return;
        }

        const actResponse: any = await this.http.get(activityDesc.href, {
            responseType: 'json',
            headers: {
                Accept: 'application/activity+json'
            }
        }).toPromise();

        if (!actResponse.type || actResponse.type !== 'Person') {
            return;
        }

        actResponse._pub = pub;

        const outbox = actResponse.outbox;

        await this.fetchOutbox(actResponse, outbox, subject);
    }

    private async fetchOutbox(account: any, url: string, subject: Subject<Tweet>) {
        const actResponse: any = await this.http.get(url, {
            responseType: 'json',
            headers: {
                Accept: 'application/activity+json'
            }
        }).toPromise();

        if (actResponse.orderedItems) {
            for (const item of actResponse.orderedItems) {
                if (item.type === 'Create') {
                    const object = item.object;

                    if (object.type === 'Note') {
                        const t = new Tweet({
                            user: {
                                name: account.name || account.preferredUsername,
                                screen_name: account._pub,
                                profile_image_url_https: account.icon
                                    ? account.icon.url : 'https://avatars.discourse.org/v4/letter/_/7993a0/96.png'
                            },
                            text: object.content.replace(/<[^>]>/g, ''),
                            created_at: object.published,
                            entities: {
                                urls: [],
                                media: []
                            },
                            id_str: object.id,
                            url: object.url
                        });

                        if (object.attachment && object.attachment[0] &&
                            object.attachment[0].mediaType === 'image/png') {
                            t.image = object.attachment[0].url;
                        }

                        if (!this.sentMap.has(t.id)) {
                            subject.next(t);

                            this.sentMap.add(t.id);
                        }
                    }
                }
            }
        }

        if (actResponse.first) {
            await this.fetchOutbox(account, actResponse.first, subject);
        }
    }
}
