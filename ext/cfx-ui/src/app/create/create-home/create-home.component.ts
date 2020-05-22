import { Component, OnInit } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';
import { GameService } from 'app/game.service';

@Component({
  selector: 'app-create-home',
  templateUrl: './create-home.component.html',
  styleUrls: ['./create-home.component.scss']
})
export class CreateHomeComponent implements OnInit {
  welcomeMessage: any;

  constructor(private domSanitizer: DomSanitizer,
    private gameService: GameService) { }

  ngOnInit(): void {
    this.fetchEditor();
  }

  fetchEditor() {
    window.fetch('https://runtime.fivem.net/create_home.html')
          .then(async res => {
              if (res.ok) {
                  this.welcomeMessage = this.domSanitizer.bypassSecurityTrustHtml(await res.text());
              }
          });
  }

  clickContent(event: MouseEvent) {
    const srcElement = event.srcElement as HTMLElement;

    for (const el of [srcElement, srcElement.parentElement]) {
      if (el.localName === 'div' && el.dataset.href) {
          this.gameService.openUrl(el.dataset.href);

          event.preventDefault();
      }
    }
  }

}
