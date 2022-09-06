import { DomPortalOutlet, TemplatePortal } from '@angular/cdk/portal';
import { AfterViewInit, ApplicationRef, Component,
	ComponentFactoryResolver, ElementRef, EmbeddedViewRef, Inject, Injector, Input, OnDestroy,
	OnInit, QueryList, TemplateRef, ViewChild, ViewChildren, ViewContainerRef } from '@angular/core';
import { L10nLocale, L10nTranslationService, L10N_LOCALE } from 'angular-l10n';
import { DiscourseService } from 'app/discourse.service';
import { VisibilityService } from 'app/servers/visibility.service';
import { intervalToDuration, Locale } from 'date-fns';
import { Observable } from 'rxjs';
import { filter, startWith, take, tap } from 'rxjs/operators';
import { Post, Reaction, Site, Type } from './discourse-models';
import { TopicEntry } from './server-reviews.component';

const formatDistanceLocale = { xSeconds: '{{count}} sec', xMinutes: '{{count}} min', xHours: '{{count}} h', xDays: '{{count}} d' };
const shortEnLocale: Locale = { formatDistance: (token, count) => formatDistanceLocale[token].replace('{{count}}', count) };

@Component({
	moduleId: module.id,
	selector: 'app-server-review',
	templateUrl: 'server-review.component.html',
	styleUrls: ['server-review.component.scss']
})
export class ServerReviewComponent implements AfterViewInit, OnInit, OnDestroy {
	@Input()
	review: TopicEntry;

	@Input()
	playtimes: Record<string, number> = {};

	@Input()
	isFirst = false;

	@ViewChild('flagModal')
	modal: TemplateRef<any>;

	@ViewChild('flagModalSelf')
	modalSelf: ElementRef<any>;

	@ViewChild('outer')
	outer: ElementRef<any>;

	private embeddedViewRef: EmbeddedViewRef<any>;

	acting = false;
	actedReaction: string = null;
	fipo: Post = null;

	flagPost: Post = null;
	flagPostFlags: Observable<Type[]> = null;
	modalActive = false;

	flagActionKey: string;
	flagMessage = '';

	hasMore = false;
	isMore = false;

	unhelpfulMapping = {
		'=1': '@Review_UserUnhelpful',
		'other': '@Review_UsersUnhelpful',
	};

	helpfulMapping = {
		'=1': '@Review_UserHelpful',
		'other': '@Review_UsersHelpful',
	};

	funnyMapping = {
		'=1': '@Review_UserFunny',
		'other': '@Review_UsersFunny',
	};

	shortEnLocale = shortEnLocale;

	@ViewChildren('more')
	more: QueryList<ElementRef<HTMLElement>>;

	siteData: Site;
	visible: Observable<boolean>;

	constructor(private discourse: DiscourseService,
		private translation: L10nTranslationService,
		@Inject(L10N_LOCALE) public locale: L10nLocale,
		private cfr: ComponentFactoryResolver,
		private ar: ApplicationRef,
		private injector: Injector,
		private vcr: ViewContainerRef,
		private visibility: VisibilityService) {

	}

	ngAfterViewInit() {
		this.visible = this.visibility
			.elementInSight(this.outer)
			.pipe(filter(visible => visible), take(1));

		this.embeddedViewRef = new DomPortalOutlet(
			document.getElementById('overlays'),
			this.cfr,
			this.ar,
			this.injector,
		).attach(new TemplatePortal(this.modal, this.vcr));

		this.more.changes.pipe(startWith(0)).subscribe(_ => {
			const el = this.more.first?.nativeElement;

			setTimeout(() => {
				this.hasMore = el?.clientHeight < el?.scrollHeight;
			}, 0);
		})
	}

	canFlag(post: Post) {
		// 2 seems to be 'like', explicitly ignore before we try to fetch /site
		return post.actions_summary.filter(a => a.can_act && a.id !== 2).length > 0;
	}

	getFlags(post: Post) {
		return this
			.discourse
			.siteData
			.pipe(tap(sd => this.siteData = sd))
			.map(d =>
				d.post_action_types
					.filter(a => post.actions_summary.find(b => b.id === a.id)?.can_act)
					.filter(a => a.is_flag)
					.filter(a => a.name_key !== 'notify_user')
			);
	}

	openFlagModal(post: Post) {
		this.flagPost = post;
		this.flagPostFlags = this.getFlags(post);
		this.modalActive = true;
	}

	toggleModalVisiblityOuter(event: MouseEvent) {
        if (event.target === this.modalSelf?.nativeElement) {
            this.modalActive = false;
        }
    }

	ngOnInit() {
	}

	ngOnDestroy() {
		this.embeddedViewRef?.destroy();
	}

	readMore(event: MouseEvent) {
		this.isMore = true;
		event.preventDefault();
	}

	get myId() {
		return this.discourse.currentUser?.id ?? 0;
	}

	get flagType() {
		return this.siteData?.post_action_types.find(a => a.name_key === this.flagActionKey);
	}

	get canSubmitFlag() {
		if (!this.flagActionKey) {
			return false;
		}

		if (this.flagType?.is_custom_flag && this.flagMessage.length < 10) {
			return false;
		}

		return true;
	}

	async submitFlag() {
		this.modalActive = false;

		await this.discourse.apiCallObservable('/post_actions', 'POST', {
			id: this.flagPost.id,
			post_action_type_id: this.flagType.id,
			flag_topic: false,
			message: this.flagMessage
		}).toPromise();

		this.flagMessage = '';
		this.flagActionKey = '';
	}

	duration(time?: number): Duration {
		return intervalToDuration({
			start: 0,
			end: (time ?? 0) * 1000
		});
	}

	date(value: string | Date) {
		if (value instanceof Date) {
			return value;
		}

		return new Date(value);
	}

	getFipo(post: Post) {
		return this.fipo ?? post;
	}

	getAvatar(template: string) {
		return DiscourseService.getAvatarUrlForUser(template);
	}

	getCount(reactions: Reaction[], type: string) {
		return reactions
			.filter(a => a.id === type)
			.map(a => a.count)
			.reduce((c, r) => c + r, 0);
	}

	getLikes(reactions: Reaction[]) {
		return this.getCount(reactions, 'heart') - this.getCount(reactions, 'angry');
	}

	getLaughs(reactions: Reaction[]) {
		return this.getCount(reactions, 'laughing');
	}

	abs(x: number) {
		return Math.abs(x);
	}

	canAct(post: Post, type: string) {
		if (post.current_user_reaction && !post.current_user_reaction.can_undo) {
			return false;
		}

		return !this.hasActed(post, type) && !this.acting && !this.hasActed(post, type);
	}

	hasActed(post: Post, type: string) {
		return this.actedReaction === type || (this.actedReaction === null && post.current_user_reaction?.id === type);
	}

	async act(postId: number, type: string) {
		if (this.acting) {
			return;
		}

		this.acting = true;

		try {
			const newPost = await this.discourse.apiCallObservable<Post>(
				`/discourse-reactions/posts/${postId}/custom-reactions/${type}/toggle.json`,
				'PUT',
				''
			).toPromise();

			this.actedReaction = newPost.current_user_reaction?.id ?? '';
			this.fipo = newPost;
		} finally {
			this.acting = false;
		}
	}
}
