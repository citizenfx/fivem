import { Component, Input, Output, HostListener, EventEmitter, OnInit, Inject } from '@angular/core';
import { L10nLocale, L10N_LOCALE } from 'angular-l10n';

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
export class ServersListHeaderComponent implements OnInit {
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

    constructor(@Inject(L10N_LOCALE) public locale: L10nLocale) {

    }

    ngOnInit(): void { }

    isSorted(column: ServersListHeadingColumn, type: String) {
        return (this.sortOrder[0] == column.column && this.sortOrder[1] == type);
    }

    isSortable(column: ServersListHeadingColumn) {
        return !!column.sortable;
    }
}
