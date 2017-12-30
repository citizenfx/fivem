import { Component, Input, Output, HostListener, EventEmitter, OnInit } from '@angular/core';
import { Language, Translation, TranslationService } from 'angular-l10n';

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
export class ServersListHeaderComponent extends Translation implements OnInit {
    @Input()
    columns: ServersListHeadingColumn[];

    @Input()
    sortOrder: string[];

    @Output()
    sort = new EventEmitter<string>();

    @Language() lang: string;

    @HostListener('click', ['$event.target'])
    private onClick(target: Element) {
        const nameAttribute: Attr = target.attributes['sortName'] || target.parentElement.attributes['sortName'];

        if (!nameAttribute) {
            return;
        }

        this.sort.emit(nameAttribute.value);
    }

    constructor(public translation: TranslationService) {
        super();
    }

    ngOnInit(): void { }

    isSorted(column: ServersListHeadingColumn, type: String) {
        return (this.sortOrder[0] == column.column && this.sortOrder[1] == type);
    }

    isSortable(column: ServersListHeadingColumn) {
        return !!column.sortable;
    }
}
