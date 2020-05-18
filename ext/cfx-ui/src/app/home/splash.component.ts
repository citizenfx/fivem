import { Component, OnInit } from "@angular/core";
import { ServersService } from "app/servers/servers.service";
import { GameService } from "app/game.service";
import { Router } from "@angular/router";

@Component({
    moduleId: module.id,
    selector: 'app-splash',
    templateUrl: 'splash.component.html',
    styleUrls: ['./splash.component.scss']
})

export class SplashComponent implements OnInit {
    // TODO: unify
	brandingName = 'CitizenFX';
    gameName = 'gta5';
    
    // TODO: REALLY UNIFY
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

    constructor(private serversService: ServersService, private gameService: GameService,
        private router: Router) {
		this.brandingName = gameService.brandingName;
        this.gameName = gameService.gameName;

        this.randomGreeting = this.randomGreetings[Math.floor(Math.random() * this.randomGreetings.length)];
    }

    ngOnInit() {
        const settle = () => {
            setTimeout(() => {
                this.serversService.onInitialized();

                (<HTMLDivElement>document.querySelector('.spinny')).style.display = 'none';
                (<HTMLDivElement>document.querySelector('app-root')).style.opacity = '1';
            }, 50);
        };

        settle();
    }

    goThere(event: MouseEvent) {
        this.router.navigate(['/home']);
        event.preventDefault();
    }
}
