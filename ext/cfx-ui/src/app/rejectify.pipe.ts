import { Pipe, PipeTransform } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';

@Pipe({
    name: 'rejectify'
})
export class RejectifyPipe implements PipeTransform {
    constructor(protected sanitizer: DomSanitizer) {

    }

    transform(value: any, args?: any): any {
        if (args?.server?.ownerName) {
			if (value.startsWith('Connection rejected by server:') && value.indexOf('CitizenFX ticket was specified') < 0) {
				const ownerName = args?.server?.ownerName.replace(/</g, '&lt;');
				const ownerAvatar = args?.server?.ownerAvatar.replace(/</g, '&lt;');

				return this.sanitizer.bypassSecurityTrustHtml(
					value.replace('Connection rejected by server:', `Connection rejected by <img class="avatar" src="${ownerAvatar}"> ${ownerName}'s server: `) + '<br><br><strong>This is <em>not</em> a message from Cfx.re/FiveM.</strong>'
				);
			}
		}

		return value;
    }
}
