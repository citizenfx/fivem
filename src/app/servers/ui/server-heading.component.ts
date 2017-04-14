import { Component, Input, Output, HostListener, EventEmitter } from '@angular/core';
import { Translation, TranslationService } from 'angular-l10n';

export class ServerHeadingColumn {
    public column: string;
    public label: string;
}

@Component({
    moduleId: module.id,
    selector: 'app-server-heading',
    template: `
    <div class="ui-row heading">
        <div *ngFor="let column of columns" class="{{column.column}}" [attr.sortName]="column.column">
            {{column.label | translate}}
        </div>
    </div>
`,

    styleUrls: ['./server-heading.component.scss']
})
export class ServerHeadingComponent extends Translation {
    @Input()
    private columns: ServerHeadingColumn[];

    @Input()
    private sortOrder: string[];

    @Output()
    private sort = new EventEmitter<string>();

    @HostListener('click', ['$event.target'])
    private onClick(target: Element) {
        const nameAttribute: Attr = target.attributes['sortName'];

        if (!nameAttribute) {
            return;
        }

        this.sort.emit(nameAttribute.value);
    }

    constructor(public translation: TranslationService) {
        super(translation);
    }
}