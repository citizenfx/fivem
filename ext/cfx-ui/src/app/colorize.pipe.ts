import { Pipe, PipeTransform } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';

@Pipe({
    name: 'colorize'
})

export class ColorizePipe implements PipeTransform {
    constructor(protected sanitizer: DomSanitizer) {

    }

    transform(value: any, args: any): any {
        const s = '<span>' + (String(value).replace(/\^([0-9])/g, (str, color) => `</span><span class="color-${color}">`)) + '</span>';
        const rep = s.replace(/<span[^>]*><\/span[^>]*>/g, '');

        if (args.trust) {
            return this.sanitizer.bypassSecurityTrustHtml(rep);
        }

        return rep;
    }
}