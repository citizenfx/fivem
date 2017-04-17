import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
    name: 'escape'
})

export class EscapePipe implements PipeTransform {
    entityMap = {
        '&': '&amp;',
        '<': '&lt;',
        '>': '&gt;',
        '"': '&quot;',
        "'": '&#39;',
        '/': '&#x2F;',
        '`': '&#x60;',
        '=': '&#x3D;'
    };

    transform(value: any, ...args: any[]): any {
        return String(value).replace(/[&<>"'`=\/]/g, s => this.entityMap[s]);
    }
}