import { Component, OnInit } from '@angular/core';

import { Tweet, TweetService } from './tweet.service';

import { GameService } from '../game.service';
import { DiscourseService } from '../discourse.service';

import { DomSanitizer } from '@angular/platform-browser';

@Component({
    moduleId: module.id,
    selector: 'app-home',
    templateUrl: 'home.component.html',
    styleUrls: ['./home.component.scss']
})

export class HomeComponent implements OnInit {
    officialTweets: Tweet[];
    communityTweets: Tweet[];

    accountBeg: any;

    currentAccount: any;

    randomGreeting = '';

    randomGreetings = [
        'It is like the Five Eyes, but mmm...',
        'The "M" stands for "Memes".',
        'Play your role moderately.',
        'What is a crash?',
        'Warning: may contain traces of snails.',
        'Broken since 2015.',
        'Open source? More like "open meme".',
        'Don\'t dare to click "Servers".',
        'Idot! What were you thinking?',
        'RIP :thinkingsnail:',
        'Still getting there, still worth the wait.',
        'Hint: it wasn\'t worth the wait.',
        'Red, dead, but not redeemed. We have cars.',
        'Did the "M" mean "Multiplayer" in the end?',
        '63 slots in the game, fill one up, 62 slots in the game...',
        'Q4 2017? H1 2018? 2018? More like _2020_.',
        'Proudly made of stolen code and hijacked helicopters.',
        '32 slots ought to be enough for anyone.',
        'Any signs of low FPS should be reported to the Performance Optimization Department.',
        'What is that mumbling in the background?',
        'Recommended dessert: session split.',
        'Our stones come pre-decrypted for your convenience.',
        'Sudo removed. VME disengaged. Wait, what does that mean?',
        'The fact is, we are right. And if you think we are wrong, *you* are wrong!',
        'Whose idea was it to put bad jokes in here, anyway?',
        'Anger, city names, alternatives and fruits do not stand a chance.',
        'Undocumented. Practically, a puzzle you have to solve by yourself.',
        'Actually open. You know, as in *open*. Not "open".',
        'We do not do big-O notation around here.',
        'Where are the hiests?',
        'Do not be like this. Be like that.',
        'One sync, two sync, three sync, four sync.',
        'Forced memes, yes.',
        'Three things are certain in life: death, taxes, and FiveM breaking your game.',
        '3 years and still no point at button.',
        'But.. I thought FiveM had an office?',
        'Eww!',
        'Five meters. That is a distance.'
    ];

    welcomeMessage: any;

    brandingName: string;

    constructor(private tweetService: TweetService, private gameService: GameService,
        private discourseService: DiscourseService, private domSanitizer: DomSanitizer) {
        discourseService.signinChange.subscribe(user => this.currentAccount = user);
    }

    ngOnInit() {
        this.brandingName = this.gameService.brandingName;

        if (this.gameService.gameName === 'rdr3') {
            // references from RDR2 (that is, Redemption 1) executable
            this.randomGreetings = [
                'You\'re running a PRE-RELEASE build, pilgrim!',
                'Howdy partner. You\'re playing with an executable built *someday*.',
            ];
        }

        this.randomGreeting = this.randomGreetings[Math.floor(Math.random() * this.randomGreetings.length)];

        this.fetchTweets();
        this.fetchWelcome();

        if (this.gameService.gameName !== 'rdr3') {
            this.fetchBeg();
        }

        this.currentAccount = this.discourseService.currentUser;
    }

    fetchWelcome() {
        window.fetch((this.gameService.gameName === 'gta5') ?
            'https://runtime.fivem.net/welcome.html' :
            `https://runtime.fivem.net/welcome_${this.gameService.gameName}.html`)
              .then(async res => {
                  if (res.ok) {
                      this.welcomeMessage = this.domSanitizer.bypassSecurityTrustHtml(await res.text());
                  }
              });
    }

    fetchBeg() {
        window.fetch('https://runtime.fivem.net/account_beg.html')
              .then(async res => {
                  if (res.ok) {
                      this.accountBeg = this.domSanitizer.bypassSecurityTrustHtml(await res.text());
                  }
              });
    }

    fetchTweets() {
        this.tweetService
            .getTweets('https://runtime.fivem.net/tweets.json')
            .then(tweets => {
                this.officialTweets = tweets.filter(a => !a.rt_displayname);
                this.communityTweets = tweets.filter(a => a.rt_displayname);
            });
    }

    clickContent(event: MouseEvent) {
        const srcElement = event.srcElement as HTMLElement;

        if (srcElement.localName === 'a') {
            this.gameService.openUrl(srcElement.getAttribute('href'));

            event.preventDefault();
        }
    }
}