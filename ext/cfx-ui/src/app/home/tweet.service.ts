import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { DomSanitizer } from '@angular/platform-browser';

import * as unicodeSubstring from 'unicode-substring';

import 'rxjs/add/operator/toPromise';

export class Tweet {
    readonly user_displayname: string;
    readonly user_screenname: string;
    readonly rt_displayname: string;
    readonly rt_screenname: string;
    readonly content: string;
    readonly date: Date;
    readonly avatar: string;
    readonly id: string;

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
    constructor(private http: HttpClient) { }

    public getTweets(uri: string): Promise<Tweet[]> {
        return this.http.get(uri)
            .toPromise()
            .then((result: any) => result
                .filter(t => !t.in_reply_to_user_id)
                .map(t => new Tweet(t)));
    }
}