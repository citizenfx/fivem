import { Component } from '@angular/core';

import { Translation, LocaleService, TranslationService } from 'angular-l10n';

import localeEn from './locale-en.json';

@Component({
  selector: 'app-root',
  template: `
    <router-outlet></router-outlet>
    <app-bottom-nav></app-bottom-nav>
  `
})
export class AppComponent extends Translation {
  constructor(public locale: LocaleService, public translation: TranslationService) {
    super(translation);

    this.locale.addConfiguration()
      .addLanguages(['en'])
      .defineLanguage('en');

    this.locale.init();

    this.translation.addConfiguration()
      .addTranslation('en', localeEn);
    this.translation.init();
  }
}
