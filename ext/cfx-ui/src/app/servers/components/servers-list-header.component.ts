import { Component, Input, Output, HostListener, EventEmitter } from '@angular/core';
import { Translation, TranslationService } from 'angular-l10n';

export class ServersListHeadingColumn {
    public column: string;
    public label: string;
    public sortable?: boolean = false;
}

@Component({
    moduleId: module.id,
    selector: 'servers-list-header',
    templateUrl: 'servers-list-header.component.html',

    styleUrls: ['./servers-list-header.component.scss']
})
export class ServersListHeaderComponent extends Translation {
    @Input()
    columns: ServersListHeadingColumn[];

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

    isSorted(column: ServersListHeadingColumn, type: String) {
        return (this.sortOrder[0] == column.column && this.sortOrder[1] == type);
    }

    isSortable(column: ServersListHeadingColumn) {
        return !!column.sortable;
    }
}
