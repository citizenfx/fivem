import { Component, Input, Output, HostListener, EventEmitter } from '@angular/core';
import { Translation, TranslationService } from 'angular-l10n';

export class ServersTableHeadingColumn {
    public column: string;
    public label: string;
}

@Component({
    moduleId: module.id,
    selector: 'servers-table-header',
    templateUrl: 'servers-table-header.component.html',

    styleUrls: ['./servers-table-header.component.scss']
})
export class ServersTableHeaderComponent extends Translation {
    @Input()
    columns: ServersTableHeadingColumn[];

    @Input()
    sortOrder: string[];

    @Output()
    sort = new EventEmitter<string>();

    @HostListener('click', ['$event.target'])
    private onClick(target: Element) {
        const nameAttribute: Attr = target.attributes['sortName'] || target.parentElement.attributes['sortName'];

        if (!nameAttribute) {
            return;
        }

        this.sort.emit(nameAttribute.value);
    }

    constructor(public translation: TranslationService) {
        super(translation);
    }

    isSorted(column: ServersTableHeadingColumn, type: String) {
        return (this.sortOrder[0] == column.column && this.sortOrder[1] == type);
    }
}
