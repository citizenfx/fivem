import { Component, OnInit, Input, HostBinding, HostListener } from '@angular/core';
import { Translation, TranslationService } from 'angular-l10n';

@Component({
    moduleId: module.id,
    selector: 'app-server-heading-column',
    template: `
    {{label | translate}}
`,
    styles: [`
    :host {
        display: block;
    }
    `]
})

export class ServerHeadingColumnComponent extends Translation {
    @Input()
    @HostBinding('class')
    private column: string;

    @Input()
    private label: string;

    @HostListener('click')
    onClick() {
        
    }

    constructor(public translation: TranslationService) {
        super(translation);
        this.label = '';
    }
}