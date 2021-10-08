import { HttpErrorResponse, HttpHandler, HttpInterceptor, HttpRequest, HTTP_INTERCEPTORS } from '@angular/common/http';
import { EventEmitter, Injectable } from '@angular/core';
import { of } from 'rxjs';
import { catchError } from 'rxjs/operators';

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
		return next.handle(req).pipe(catchError(event => this.handleForumResponseError(event)));
	}

	private handleForumResponseError(error: any) {
		if (error instanceof HttpErrorResponse && error.status === 403) {
			this.state.onSignout.emit();
		}

		return of(error);
	}
}

export const httpInterceptorProviders = [
	{ provide: HTTP_INTERCEPTORS, useClass: ForumSignoutInterceptor, multi: true }
];
