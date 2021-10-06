import { HttpEvent, HttpHandler, HttpInterceptor, HttpRequest, HttpResponse, HTTP_INTERCEPTORS } from '@angular/common/http';
import { EventEmitter, Injectable } from '@angular/core';
import { map } from 'rxjs/operators';

@Injectable()
export class ForumSignoutInterceptorState {
	onSignout = new EventEmitter();
}

@Injectable()
export class ForumSignoutInterceptor implements HttpInterceptor {
	constructor(private state: ForumSignoutInterceptorState) {
	}

	intercept(req: HttpRequest<any>, next: HttpHandler) {
		if (req.url.includes('forum.cfx.re') || req.url.includes('forum.fivem.net')) {
			return this.handleForumRequest(req, next);
		}

		return next.handle(req);
	}

	private handleForumRequest(req: HttpRequest<any>, next: HttpHandler) {
		return next.handle(req).pipe(map(event => this.handleForumResponse(event)));
	}

	private handleForumResponse(event: HttpEvent<any>) {
		if (event instanceof HttpResponse && event.status === 403) {
			this.state.onSignout.emit();
		}

		return event;
	}
}

export const httpInterceptorProviders = [
	{ provide: HTTP_INTERCEPTORS, useClass: ForumSignoutInterceptor, multi: true }
];
