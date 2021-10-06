import {
    ElementRef, Output, Directive, AfterViewInit, OnDestroy, EventEmitter
} from '@angular/core';
import { Subscription } from 'rxjs';
import { fromEvent } from 'rxjs';
import { startWith } from 'rxjs/operators';

@Directive({
    selector: '[appHorizAppear]'
})
export class AppearDirective implements AfterViewInit, OnDestroy {
    @Output() appear: EventEmitter<void>;

    elementPos: number;
    elementWidth: number;

    scrollPos: number;
    windowWidth: number;

    subscriptionScroll: Subscription;
    subscriptionResize: Subscription;

    constructor(private element: ElementRef<HTMLElement>) {
        this.appear = new EventEmitter<void>();
    }

    saveDimensions() {
        this.elementPos = this.getOffsetLeft(this.element.nativeElement);
        this.elementWidth = this.element.nativeElement.offsetWidth;
        this.windowWidth = this.scrollParent(this.element.nativeElement).clientWidth;
    }
    saveScrollPos() {
        this.scrollPos = this.scrollParent(this.element.nativeElement).scrollLeft;
    }
    getOffsetLeft(element: HTMLElement) {
        let offsetLeft = element.offsetLeft || 0;
		const sp = this.scrollParent(element);
		while (element !== sp) {
			offsetLeft += element.offsetLeft;
			element = element.parentElement;
		}
        return offsetLeft;
    }
    checkVisibility() {
        if (this.isVisible()) {
            this.saveDimensions();
            if (this.isVisible()) {
                this.unsubscribe();
                this.appear.emit();
            }
        }
    }
    isVisible() {
        return this.scrollPos >= this.elementPos || (this.scrollPos + this.windowWidth) >= (this.elementPos + this.elementWidth);
    }

    subscribe() {
        this.subscriptionScroll = fromEvent(this.scrollParent(this.element.nativeElement), 'scroll').pipe(startWith(null))
            .subscribe(() => {
                this.saveScrollPos();
                this.checkVisibility();
            });
        this.subscriptionResize = fromEvent(this.scrollParent(this.element.nativeElement), 'resize').pipe(startWith(null))
            .subscribe(() => {
                this.saveDimensions();
                this.checkVisibility();
            });
    }
    unsubscribe() {
        if (this.subscriptionScroll) {
            this.subscriptionScroll.unsubscribe();
        }
        if (this.subscriptionResize) {
            this.subscriptionResize.unsubscribe();
        }
    }

    ngAfterViewInit() {
        this.subscribe();
    }
    ngOnDestroy() {
        this.unsubscribe();
    }

	scrollParent(element: HTMLElement) {
		while (element.scrollWidth === element.clientWidth) {
			element = element.parentElement;
		}

		return element;
	}
}
