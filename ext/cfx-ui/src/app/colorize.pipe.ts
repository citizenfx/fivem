import { Pipe, PipeTransform } from '@angular/core';

@Pipe({
    name: 'colorize'
})

export class ColorizePipe implements PipeTransform {
    transform(value: any, ...args: any[]): any {
        const s = "<span>" + (String(value).replace(/\^([0-9])/g, (str, color) => `</span><span class="color-${color}">`)) + "</span>";
        return s.replace(/<span[^>]*><\/span[^>]*>/g, '');
    }
}