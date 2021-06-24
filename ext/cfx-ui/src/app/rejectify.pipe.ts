import { Pipe, PipeTransform } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';

@Pipe({
    name: 'rejectify'
})
export class RejectifyPipe implements PipeTransform {
    private static OWNER_RE = /\[OWNER\]/g;

    constructor(protected sanitizer: DomSanitizer) {

    }

    transform(value: any, args?: any): any {
        if (value.indexOf('[STATUSPAGE]') >= 0) {
            return this.sanitizer.bypassSecurityTrustHtml(
                value.replace(/\[STATUSPAGE\]/g, `<a href="https://status.cfx.re">Cfx.re Status</a>`)
                    .replace(/\[SUPPORT\]/g, `<a href="https://aka.cfx.re/support">Cfx.re Support</a>`)
            );
        }

        if (args?.server?.ownerName) {
            const ownerName = args?.server?.ownerName.replace(/</g, '&lt;');
            const ownerAvatar = args?.server?.ownerAvatar.replace(/</g, '&lt;');

            if (value.startsWith('Connection rejected by server:') && value.indexOf('CitizenFX ticket was specified') < 0) {
				return this.sanitizer.bypassSecurityTrustHtml(
					value.replace('Connection rejected by server:', `Connection rejected by <img class="avatar" src="${ownerAvatar}"> ${ownerName}'s server: `) + '<br><br><strong>This is <em>not</em> a message from Cfx.re/FiveM.</strong>'
				);
			}

            if (RejectifyPipe.OWNER_RE.test(value)) {
				return this.sanitizer.bypassSecurityTrustHtml(
					value.replace(RejectifyPipe.OWNER_RE, `<a href="https://forum.cfx.re/u/${ownerName}"><img class="avatar" src="${ownerAvatar}"> ${ownerName}</a>`)
				);
            }
		} else {
            if (RejectifyPipe.OWNER_RE.test(value)) {
                // #TODO: localization!
                value = value.replace(RejectifyPipe.OWNER_RE, 'the server owner');
            }
        }

		return value;
    }
}
