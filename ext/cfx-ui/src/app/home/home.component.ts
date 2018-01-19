import { Component, OnInit } from '@angular/core';

import { Tweet, TweetService } from './tweet.service';

import { GameService } from '../game.service';

@Component({
    moduleId: module.id,
    selector: 'app-home',
    templateUrl: 'home.component.html',
    styleUrls: ['./home.component.scss']
})

export class HomeComponent implements OnInit {
    tweets: Tweet[];

    constructor(private tweetService: TweetService, private gameService: GameService) { }

    ngOnInit() {
        this.fetchTweets();
    }

    fetchTweets() {
        this.tweetService
            .getTweets('https://runtime.fivem.net/tweets.json')
            .then(tweets => this.tweets = tweets);
    }

    openTweet(id) {
        this.gameService.openUrl('https://twitter.com/_FiveM/status/' + id);
    }
}