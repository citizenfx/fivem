import { DOCUMENT } from '@angular/common';
import { ElementRef, Inject, Injectable } from '@angular/core';
import { combineLatest, concat, defer, fromEvent, Observable, of } from 'rxjs';
import { distinctUntilChanged, flatMap, map } from 'rxjs/operators';

// copied from a medium dot com blog: `angular-in-depth/improve-performance-with-lazy-components`
@Injectable()
export class VisibilityService {

	private pageVisible$: Observable<boolean>;

	constructor(@Inject(DOCUMENT) document: any) {
		this.pageVisible$ = concat(
			defer(() => of(!document.hidden)),
			fromEvent(document, 'visibilitychange')
				.pipe(
					map(e => !document.hidden)
				)
		);
	}

	elementInSight(element: ElementRef): Observable<boolean> {
		const elementVisible$ = new Observable(observer => {
			const intersectionObserver = new IntersectionObserver(entries => {
				observer.next(entries);
			});

			intersectionObserver.observe(element.nativeElement);

			return () => { intersectionObserver.disconnect(); };
		})
		.pipe(
			flatMap((entries: any[]) => entries),
			map(entry => entry.isIntersecting as boolean),
			distinctUntilChanged()
		);

		const elementInSight$ = combineLatest([
			this.pageVisible$,
			elementVisible$
		])
		.pipe(
			map(([pageVisible, elementVisible]) => pageVisible && elementVisible),
			distinctUntilChanged()
		);

		return elementInSight$;
	}

}
