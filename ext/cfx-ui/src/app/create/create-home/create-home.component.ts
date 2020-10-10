import { Component, OnInit } from '@angular/core';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { GameService } from 'app/game.service';

let cachedCreate: SafeHtml;

@Component({
  selector: 'app-create-home',
  templateUrl: './create-home.component.html',
  styleUrls: ['./create-home.component.scss']
})
export class CreateHomeComponent implements OnInit {
  create: SafeHtml;

  constructor(private domSanitizer: DomSanitizer,
    private gameService: GameService) {
		this.create = cachedCreate;
	}

  ngOnInit(): void {
    this.fetchEditor();
  }

  fetchEditor() {
    window.fetch('https://runtime.fivem.net/create_home.html')
          .then(async res => {
              if (res.ok) {
                  this.create = cachedCreate = this.domSanitizer.bypassSecurityTrustHtml(await res.text());
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
